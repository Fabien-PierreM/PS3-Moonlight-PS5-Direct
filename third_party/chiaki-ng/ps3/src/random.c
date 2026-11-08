// SPDX-License-Identifier: LicenseRef-AGPL-3.0-only-OpenSSL
// PS3 platform RNG for the direct LAN client.

#include <chiaki/random.h>

#include <lv2/system.h>

#include <stddef.h>
#include <stdint.h>

CHIAKI_EXPORT ChiakiErrorCode chiaki_random_bytes_crypt(uint8_t *buf, size_t buf_size)
{
	while(buf_size > 0)
	{
		size_t chunk_size = buf_size > RANDOM_NUMBER_MAX_SIZE ? RANDOM_NUMBER_MAX_SIZE : buf_size;
		if(sysGetRandomNumber(buf, (u64)chunk_size) != 0)
			return CHIAKI_ERR_UNKNOWN;
		buf += chunk_size;
		buf_size -= chunk_size;
	}
	return CHIAKI_ERR_SUCCESS;
}

CHIAKI_EXPORT uint32_t chiaki_random_32(void)
{
	uint32_t random_number = 0;
	if(chiaki_random_bytes_crypt((uint8_t *)&random_number, sizeof(random_number)) != CHIAKI_ERR_SUCCESS)
		return 0;
	return random_number;
}
