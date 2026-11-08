#include "ps5_relay.h"

#include <arpa/inet.h>
#include <errno.h>
#include <malloc.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "net_logger.h"
#include "ps3_relay_protocol.h"
#include "video.h"

/*
 * P3RP/1 receiver
 * ---------------
 * This deliberately accepts only a small, testable LAN transport. Remote Play
 * authentication stays in the companion relay: this code never contains a
 * PSN token, account ID, registration key, or a Remote Play cipher bypass.
 *
 * The first prototype is intentionally strict: fragments must arrive in-order
 * and a lost fragment drops its access unit. That keeps bad/corrupt H.264 out
 * of libvdec and gives the relay a deterministic point to request an IDR.
 */

#define PS5_RELAY_WIDTH  1280
#define PS5_RELAY_HEIGHT 720
#define PS5_RELAY_FPS    60
#define PS5_RELAY_MAX_DRAIN 96

static int g_socket = -1;
static int g_active = 0;
static uint8_t *g_access_unit = NULL;
static size_t g_access_unit_len = 0;
static uint32_t g_frame_id = 0;
static uint16_t g_fragment_count = 0;
static uint16_t g_next_fragment = 0;

static void reset_assembly(void)
{
    g_access_unit_len = 0;
    g_frame_id = 0;
    g_fragment_count = 0;
    g_next_fragment = 0;
}

static int packet_header_is_valid(const uint8_t *packet, int packet_len,
                                  uint32_t *frame_id, uint16_t *fragment_index,
                                  uint16_t *fragment_count, uint16_t *payload_len)
{
    if (!packet || packet_len < (int)PS3_RELAY_HEADER_SIZE) {
        return 0;
    }

    if (packet[0] != PS3_RELAY_MAGIC_0 || packet[1] != PS3_RELAY_MAGIC_1 ||
        packet[2] != PS3_RELAY_MAGIC_2 || packet[3] != PS3_RELAY_MAGIC_3 ||
        packet[4] != PS3_RELAY_VERSION) {
        return 0;
    }

    *frame_id = ps3_relay_read_be32(packet + 6);
    *fragment_index = ps3_relay_read_be16(packet + 10);
    *fragment_count = ps3_relay_read_be16(packet + 12);
    *payload_len = ps3_relay_read_be16(packet + 14);

    if (*fragment_count == 0 || *fragment_index >= *fragment_count ||
        *payload_len == 0 || *payload_len > PS3_RELAY_MAX_PAYLOAD ||
        (int)(PS3_RELAY_HEADER_SIZE + *payload_len) != packet_len) {
        return 0;
    }

    return 1;
}

static void submit_complete_access_unit(void)
{
    int rc;

    if (!g_access_unit || g_access_unit_len == 0) {
        reset_assembly();
        return;
    }

    rc = ps3video_submit_annexb_access_unit(g_access_unit, g_access_unit_len);
    if (rc != DR_OK) {
        NLOG("[P3RP] libvdec queue rejected frame %u; relay should send a new IDR", g_frame_id);
    }
    reset_assembly();
}

int ps5_relay_start(const char *relay_ipv4)
{
    struct sockaddr_in local_addr;
    struct sockaddr_in control_addr;
    int nonblocking = 1;
    int receive_buffer = 512 * 1024;
    static const char hello[] = "P3RP/1 HELLO 720P60 H264";

    if (!relay_ipv4 || !*relay_ipv4) {
        NLOG("[P3RP] Relay IPv4 address is empty");
        return -1;
    }

    ps5_relay_stop();

    if (ps3video_setup_h264(PS5_RELAY_WIDTH, PS5_RELAY_HEIGHT, PS5_RELAY_FPS) != 0) {
        NLOG("[P3RP] H.264 decoder initialization failed");
        return -1;
    }

    g_access_unit = memalign(128, PS3_RELAY_MAX_ACCESS_UNIT);
    if (!g_access_unit) {
        NLOG("[P3RP] Cannot allocate %u-byte H.264 access-unit buffer", (unsigned)PS3_RELAY_MAX_ACCESS_UNIT);
        ps3video_cleanup_h264();
        return -1;
    }

    g_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (g_socket < 0) {
        NLOG("[P3RP] UDP socket() failed: errno=%d", errno);
        free(g_access_unit);
        g_access_unit = NULL;
        ps3video_cleanup_h264();
        return -1;
    }

    setsockopt(g_socket, SOL_SOCKET, SO_RCVBUF, &receive_buffer, sizeof(receive_buffer));
    setsockopt(g_socket, SOL_SOCKET, SO_NBIO, &nonblocking, sizeof(nonblocking));

    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    local_addr.sin_port = htons(PS3_RELAY_VIDEO_PORT);
    if (bind(g_socket, (struct sockaddr *)&local_addr, sizeof(local_addr)) != 0) {
        NLOG("[P3RP] bind(UDP/%u) failed: errno=%d", PS3_RELAY_VIDEO_PORT, errno);
        close(g_socket);
        g_socket = -1;
        free(g_access_unit);
        g_access_unit = NULL;
        ps3video_cleanup_h264();
        return -1;
    }

    memset(&control_addr, 0, sizeof(control_addr));
    control_addr.sin_family = AF_INET;
    control_addr.sin_port = htons(PS3_RELAY_CONTROL_PORT);
    if (inet_aton(relay_ipv4, &control_addr.sin_addr) == 0) {
        NLOG("[P3RP] Invalid IPv4 relay address: %s", relay_ipv4);
        close(g_socket);
        g_socket = -1;
        free(g_access_unit);
        g_access_unit = NULL;
        ps3video_cleanup_h264();
        return -1;
    }

    /* Best-effort discovery/control signal. The companion relay may also be
     * configured to start streaming without a HELLO. */
    sendto(g_socket, hello, sizeof(hello) - 1, 0,
           (struct sockaddr *)&control_addr, sizeof(control_addr));

    reset_assembly();
    g_active = 1;
    NLOG("[P3RP] Listening on UDP/%u for %s; fixed profile is 1280x720p60 H.264",
         PS3_RELAY_VIDEO_PORT, relay_ipv4);
    return 0;
}

void ps5_relay_poll(void)
{
    uint8_t packet[PS3_RELAY_MAX_DATAGRAM];

    if (!g_active || g_socket < 0 || !g_access_unit) {
        return;
    }

    for (int drained = 0; drained < PS5_RELAY_MAX_DRAIN; drained++) {
        int packet_len;
        uint32_t frame_id;
        uint16_t fragment_index;
        uint16_t fragment_count;
        uint16_t payload_len;

        packet_len = recvfrom(g_socket, packet, sizeof(packet), MSG_DONTWAIT, NULL, NULL);
        if (packet_len <= 0) {
            break;
        }

        if (!packet_header_is_valid(packet, packet_len, &frame_id, &fragment_index,
                                    &fragment_count, &payload_len)) {
            continue;
        }

        if (fragment_index == 0) {
            /* A new access unit always supersedes an incomplete old one. */
            g_frame_id = frame_id;
            g_fragment_count = fragment_count;
            g_next_fragment = 0;
            g_access_unit_len = 0;
        }

        if (g_fragment_count == 0 || frame_id != g_frame_id ||
            fragment_count != g_fragment_count || fragment_index != g_next_fragment ||
            g_access_unit_len + payload_len > PS3_RELAY_MAX_ACCESS_UNIT) {
            reset_assembly();
            continue;
        }

        memcpy(g_access_unit + g_access_unit_len, packet + PS3_RELAY_HEADER_SIZE, payload_len);
        g_access_unit_len += payload_len;
        g_next_fragment++;

        if (g_next_fragment == g_fragment_count) {
            submit_complete_access_unit();
        }
    }
}

void ps5_relay_stop(void)
{
    if (g_socket >= 0) {
        close(g_socket);
        g_socket = -1;
    }
    if (g_access_unit) {
        free(g_access_unit);
        g_access_unit = NULL;
    }
    if (g_active) {
        ps3video_cleanup_h264();
    }
    g_active = 0;
    reset_assembly();
}

int ps5_relay_is_active(void)
{
    return g_active;
}
