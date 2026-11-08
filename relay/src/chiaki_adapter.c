#include "chiaki_adapter.h"

/*
 * The caller is responsible for using Chiaki-ng's documented registration and
 * session setup. This adapter intentionally has no code that extracts, forges,
 * or bypasses PSN/Remote Play authentication material.
 */
void chiaki_ps3_relay_set_720p_profile(ChiakiConnectInfo *connect_info)
{
    if (!connect_info) {
        return;
    }

    chiaki_connect_video_profile_preset(&connect_info->video_profile,
                                        CHIAKI_VIDEO_RESOLUTION_PRESET_720p,
                                        CHIAKI_VIDEO_FPS_PRESET_60);
    connect_info->video_profile.codec = CHIAKI_CODEC_H264;
    connect_info->video_profile_auto_downgrade = false;
}

bool chiaki_ps3_relay_video_sample(uint8_t *buf, size_t buf_size,
                                   int32_t frames_lost, bool frame_recovered,
                                   void *user)
{
    struct ps3_relay_sender *sender = user;
    (void)frames_lost;
    (void)frame_recovered;

    if (!sender || !buf || buf_size == 0) {
        return false;
    }

    /* Chiaki-ng delivers the assembled H.264 sample through this callback.
     * It is forwarded unchanged; the PS3 VDEC path consumes Annex-B. */
    return ps3_relay_sender_send_access_unit(sender, buf, buf_size, false) == 0;
}
