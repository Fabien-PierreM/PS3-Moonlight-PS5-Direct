#include "packetizer.h"

#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "ps3_relay_protocol.h"

static struct sockaddr_in *sender_address(struct ps3_relay_sender *sender)
{
    return (struct sockaddr_in *)(void *)sender->destination;
}

int ps3_relay_sender_init(struct ps3_relay_sender *sender,
                          const char *destination_ipv4, uint16_t destination_port)
{
    struct sockaddr_in *destination;

    if (!sender || !destination_ipv4 || destination_port == 0) {
        errno = EINVAL;
        return -1;
    }

    memset(sender, 0, sizeof(*sender));
    sender->socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sender->socket_fd < 0) {
        return -1;
    }

    destination = sender_address(sender);
    memset(destination, 0, sizeof(*destination));
    destination->sin_family = AF_INET;
    destination->sin_port = htons(destination_port);
    if (inet_pton(AF_INET, destination_ipv4, &destination->sin_addr) != 1) {
        ps3_relay_sender_fini(sender);
        errno = EINVAL;
        return -1;
    }

    sender->destination_port = destination_port;
    sender->next_frame_id = 1;
    return 0;
}

void ps3_relay_sender_fini(struct ps3_relay_sender *sender)
{
    if (sender && sender->socket_fd >= 0) {
        close(sender->socket_fd);
        sender->socket_fd = -1;
    }
}

int ps3_relay_sender_send_access_unit(struct ps3_relay_sender *sender,
                                      const uint8_t *annexb, size_t annexb_len,
                                      bool is_idr)
{
    uint8_t packet[PS3_RELAY_MAX_DATAGRAM];
    size_t fragments;
    uint32_t frame_id;

    if (!sender || sender->socket_fd < 0 || !annexb || annexb_len == 0 ||
        annexb_len > PS3_RELAY_MAX_ACCESS_UNIT) {
        errno = EINVAL;
        return -1;
    }

    fragments = (annexb_len + PS3_RELAY_MAX_PAYLOAD - 1) / PS3_RELAY_MAX_PAYLOAD;
    if (fragments == 0 || fragments > UINT16_MAX) {
        errno = EMSGSIZE;
        return -1;
    }

    frame_id = sender->next_frame_id++;
    for (size_t fragment = 0; fragment < fragments; fragment++) {
        size_t offset = fragment * PS3_RELAY_MAX_PAYLOAD;
        size_t payload_len = annexb_len - offset;
        ssize_t sent;

        if (payload_len > PS3_RELAY_MAX_PAYLOAD) {
            payload_len = PS3_RELAY_MAX_PAYLOAD;
        }

        packet[0] = PS3_RELAY_MAGIC_0;
        packet[1] = PS3_RELAY_MAGIC_1;
        packet[2] = PS3_RELAY_MAGIC_2;
        packet[3] = PS3_RELAY_MAGIC_3;
        packet[4] = PS3_RELAY_VERSION;
        packet[5] = is_idr ? PS3_RELAY_FLAG_IDR : 0;
        ps3_relay_write_be32(packet + 6, frame_id);
        ps3_relay_write_be16(packet + 10, (uint16_t)fragment);
        ps3_relay_write_be16(packet + 12, (uint16_t)fragments);
        ps3_relay_write_be16(packet + 14, (uint16_t)payload_len);
        memcpy(packet + PS3_RELAY_HEADER_SIZE, annexb + offset, payload_len);

        sent = sendto(sender->socket_fd, packet, PS3_RELAY_HEADER_SIZE + payload_len, 0,
                      (const struct sockaddr *)sender_address(sender), sizeof(struct sockaddr_in));
        if (sent != (ssize_t)(PS3_RELAY_HEADER_SIZE + payload_len)) {
            return -1;
        }
    }

    return 0;
}
