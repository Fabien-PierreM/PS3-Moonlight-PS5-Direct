#include <arpa/inet.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include "packetizer.h"
#include "ps3_relay_protocol.h"

int main(void)
{
    int receiver = -1;
    struct sockaddr_in bind_addr;
    socklen_t bind_addr_len = sizeof(bind_addr);
    struct timeval timeout = {.tv_sec = 2, .tv_usec = 0};
    struct ps3_relay_sender sender;
    uint8_t source[PS3_RELAY_MAX_PAYLOAD * 2 + 37];
    uint8_t reconstructed[sizeof(source)];
    size_t reconstructed_len = 0;
    size_t expected_fragments;

    for (size_t i = 0; i < sizeof(source); i++) {
        source[i] = (uint8_t)(i * 17u + 3u);
    }

    receiver = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (receiver < 0) {
        perror("socket");
        return 1;
    }
    setsockopt(receiver, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    memset(&bind_addr, 0, sizeof(bind_addr));
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    bind_addr.sin_port = 0;
    if (bind(receiver, (const struct sockaddr *)&bind_addr, sizeof(bind_addr)) != 0 ||
        getsockname(receiver, (struct sockaddr *)&bind_addr, &bind_addr_len) != 0) {
        perror("bind/getsockname");
        close(receiver);
        return 1;
    }

    if (ps3_relay_sender_init(&sender, "127.0.0.1", ntohs(bind_addr.sin_port)) != 0 ||
        ps3_relay_sender_send_access_unit(&sender, source, sizeof(source), true) != 0) {
        perror("sender");
        ps3_relay_sender_fini(&sender);
        close(receiver);
        return 1;
    }
    ps3_relay_sender_fini(&sender);

    expected_fragments = (sizeof(source) + PS3_RELAY_MAX_PAYLOAD - 1) / PS3_RELAY_MAX_PAYLOAD;
    for (size_t expected_index = 0; expected_index < expected_fragments; expected_index++) {
        uint8_t packet[PS3_RELAY_MAX_DATAGRAM];
        ssize_t received = recv(receiver, packet, sizeof(packet), 0);
        uint16_t payload_len;

        if (received < (ssize_t)PS3_RELAY_HEADER_SIZE ||
            memcmp(packet, "P3RP", 4) != 0 || packet[4] != PS3_RELAY_VERSION ||
            !(packet[5] & PS3_RELAY_FLAG_IDR) ||
            ps3_relay_read_be16(packet + 10) != expected_index ||
            ps3_relay_read_be16(packet + 12) != expected_fragments) {
            fprintf(stderr, "invalid fragment %zu\n", expected_index);
            close(receiver);
            return 1;
        }

        payload_len = ps3_relay_read_be16(packet + 14);
        if (received != (ssize_t)(PS3_RELAY_HEADER_SIZE + payload_len) ||
            reconstructed_len + payload_len > sizeof(reconstructed)) {
            fprintf(stderr, "invalid payload length\n");
            close(receiver);
            return 1;
        }
        memcpy(reconstructed + reconstructed_len, packet + PS3_RELAY_HEADER_SIZE, payload_len);
        reconstructed_len += payload_len;
    }

    close(receiver);
    if (reconstructed_len != sizeof(source) || memcmp(source, reconstructed, sizeof(source)) != 0) {
        fprintf(stderr, "round trip mismatch\n");
        return 1;
    }

    puts("packetizer loopback test: PASS");
    return 0;
}
