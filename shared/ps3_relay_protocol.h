#ifndef PS3_RELAY_PROTOCOL_H
#define PS3_RELAY_PROTOCOL_H

/*
 * PS3 Remote Play Relay Protocol (P3RP/1)
 *
 * This is a deliberately small, LAN-only transport between a companion relay
 * and the PS3. It carries one H.264 Annex-B access unit split into UDP
 * datagrams. It is not the PlayStation Remote Play protocol and carries no
 * PSN credential, registration key, or account identifier.
 */

#include <stddef.h>
#include <stdint.h>

#define PS3_RELAY_MAGIC_0 'P'
#define PS3_RELAY_MAGIC_1 '3'
#define PS3_RELAY_MAGIC_2 'R'
#define PS3_RELAY_MAGIC_3 'P'
#define PS3_RELAY_VERSION 1u

#define PS3_RELAY_VIDEO_PORT 9295u
#define PS3_RELAY_CONTROL_PORT 9296u
#define PS3_RELAY_HEADER_SIZE 16u
#define PS3_RELAY_MAX_DATAGRAM 1200u
#define PS3_RELAY_MAX_PAYLOAD (PS3_RELAY_MAX_DATAGRAM - PS3_RELAY_HEADER_SIZE)
#define PS3_RELAY_MAX_ACCESS_UNIT (4u * 1024u * 1024u)

/* Header byte layout; every multi-byte field is unsigned big endian.
 *  0..3   magic: "P3RP"
 *  4      version: 1
 *  5      flags
 *  6..9   frame_id
 * 10..11  fragment_index
 * 12..13  fragment_count
 * 14..15  payload_len
 */
#define PS3_RELAY_FLAG_IDR 0x01u

static inline uint16_t ps3_relay_read_be16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

static inline uint32_t ps3_relay_read_be32(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

static inline void ps3_relay_write_be16(uint8_t *p, uint16_t value)
{
    p[0] = (uint8_t)(value >> 8);
    p[1] = (uint8_t)value;
}

static inline void ps3_relay_write_be32(uint8_t *p, uint32_t value)
{
    p[0] = (uint8_t)(value >> 24);
    p[1] = (uint8_t)(value >> 16);
    p[2] = (uint8_t)(value >> 8);
    p[3] = (uint8_t)value;
}

#endif /* PS3_RELAY_PROTOCOL_H */
