#ifndef PS5_RELAY_H
#define PS5_RELAY_H

/* LAN-only H.264 receiver for the companion Chiaki-ng relay.
 *
 * The companion, not the PS3, owns Remote Play registration/session state.
 * The PS3 receives only a constrained 1280x720 progressive H.264 Annex-B
 * stream over the documented P3RP/1 transport.
 */

int ps5_relay_start(const char *relay_ipv4);
void ps5_relay_stop(void);
void ps5_relay_poll(void);
int ps5_relay_is_active(void);

#endif /* PS5_RELAY_H */
