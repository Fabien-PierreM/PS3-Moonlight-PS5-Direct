// SPDX-License-Identifier: LicenseRef-AGPL-3.0-only-OpenSSL

#include <chiaki/time.h>

#include <lv2/systime.h>

/* PSL1GHT exposes the LV2 system timer in microseconds. It is suitable for
 * latency accounting and the monotonic-time contract used by libchiaki. */
CHIAKI_EXPORT uint64_t chiaki_time_now_monotonic_us(void)
{
    return (uint64_t)sysGetSystemTime();
}
