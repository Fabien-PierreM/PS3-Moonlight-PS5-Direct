#ifndef PS3_RELAY_PACKETIZER_H
#define PS3_RELAY_PACKETIZER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct ps3_relay_sender {
    int socket_fd;
    uint8_t destination[16];
    uint16_t destination_port;
    uint32_t next_frame_id;
};

/* Initializes a UDP sender for a P3RP/1 receiver. destination_ipv4 must be
 * dotted decimal. The caller owns the socket lifetime via ps3_relay_sender_fini. */
int ps3_relay_sender_init(struct ps3_relay_sender *sender,
                          const char *destination_ipv4, uint16_t destination_port);
void ps3_relay_sender_fini(struct ps3_relay_sender *sender);

/* Sends exactly one H.264 Annex-B access unit. is_idr only labels telemetry;
 * the PS3 receiver accepts either flag. Returns 0 on success. */
int ps3_relay_sender_send_access_unit(struct ps3_relay_sender *sender,
                                      const uint8_t *annexb, size_t annexb_len,
                                      bool is_idr);

#endif /* PS3_RELAY_PACKETIZER_H */
