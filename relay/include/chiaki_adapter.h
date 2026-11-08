#ifndef CHIAKI_PS3_RELAY_ADAPTER_H
#define CHIAKI_PS3_RELAY_ADAPTER_H

#include <chiaki/session.h>

#include "packetizer.h"

/* Force the only codec/profile the PS3 prototype accepts. */
void chiaki_ps3_relay_set_720p_profile(ChiakiConnectInfo *connect_info);

/* Attach this callback after chiaki_session_init() and before session_start(). */
bool chiaki_ps3_relay_video_sample(uint8_t *buf, size_t buf_size,
                                   int32_t frames_lost, bool frame_recovered,
                                   void *user);

#endif /* CHIAKI_PS3_RELAY_ADAPTER_H */
