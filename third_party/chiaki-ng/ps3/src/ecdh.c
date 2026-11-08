// SPDX-License-Identifier: LicenseRef-AGPL-3.0-only-OpenSSL
// PS3 mbedTLS ECDH backend. It obtains entropy from LV2 instead of a host OS.

#include <chiaki/ecdh.h>

#ifndef CHIAKI_HANDSHAKE_KEY_SIZE
#define CHIAKI_HANDSHAKE_KEY_SIZE 0x10
#endif

#include <mbedtls/ecdh.h>
#include <mbedtls/md.h>

#include <lv2/system.h>

#include <stddef.h>
#include <stdint.h>
#include <string.h>

static int chiaki_ps3_rng(void *user, unsigned char *buf, size_t buf_size)
{
	(void)user;
	while(buf_size > 0)
	{
		size_t chunk_size = buf_size > RANDOM_NUMBER_MAX_SIZE ? RANDOM_NUMBER_MAX_SIZE : buf_size;
		if(sysGetRandomNumber(buf, (u64)chunk_size) != 0)
			return -1;
		buf += chunk_size;
		buf_size -= chunk_size;
	}
	return 0;
}

CHIAKI_EXPORT ChiakiErrorCode chiaki_ecdh_init(ChiakiECDH *ecdh)
{
	const char personalization[] = "chiaki-ps3-ecdh";
	memset(ecdh, 0, sizeof(*ecdh));
	mbedtls_ecdh_init(&ecdh->ctx);
	mbedtls_ctr_drbg_init(&ecdh->drbg);

	if(mbedtls_ctr_drbg_seed(&ecdh->drbg, chiaki_ps3_rng, NULL,
			(const unsigned char *)personalization, sizeof(personalization) - 1) != 0)
		goto error;
	if(mbedtls_ecp_group_load(&ecdh->ctx.grp, MBEDTLS_ECP_DP_SECP256K1) != 0)
		goto error;
	if(mbedtls_ecdh_gen_public(&ecdh->ctx.grp, &ecdh->ctx.d, &ecdh->ctx.Q,
			mbedtls_ctr_drbg_random, &ecdh->drbg) != 0)
		goto error;

	return CHIAKI_ERR_SUCCESS;

error:
	chiaki_ecdh_fini(ecdh);
	return CHIAKI_ERR_UNKNOWN;
}

CHIAKI_EXPORT void chiaki_ecdh_fini(ChiakiECDH *ecdh)
{
	mbedtls_ecdh_free(&ecdh->ctx);
	mbedtls_ctr_drbg_free(&ecdh->drbg);
}

CHIAKI_EXPORT ChiakiErrorCode chiaki_ecdh_set_local_key(ChiakiECDH *ecdh,
		const uint8_t *private_key, size_t private_key_size,
		const uint8_t *public_key, size_t public_key_size)
{
	if(mbedtls_ecp_point_read_binary(&ecdh->ctx.grp, &ecdh->ctx.Q,
			public_key, public_key_size) != 0)
		return CHIAKI_ERR_UNKNOWN;
	if(mbedtls_mpi_read_binary(&ecdh->ctx.d, private_key, private_key_size) != 0)
		return CHIAKI_ERR_UNKNOWN;
	return CHIAKI_ERR_SUCCESS;
}

CHIAKI_EXPORT ChiakiErrorCode chiaki_ecdh_get_local_pub_key(ChiakiECDH *ecdh,
		uint8_t *key_out, size_t *key_out_size, const uint8_t *handshake_key,
		uint8_t *sig_out, size_t *sig_out_size)
{
	if(mbedtls_ecp_point_write_binary(&ecdh->ctx.grp, &ecdh->ctx.Q,
			MBEDTLS_ECP_PF_UNCOMPRESSED, key_out_size, key_out, *key_out_size) != 0)
		return CHIAKI_ERR_UNKNOWN;

	mbedtls_md_context_t md;
	mbedtls_md_init(&md);
	if(mbedtls_md_setup(&md, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1) != 0
			|| mbedtls_md_hmac_starts(&md, handshake_key, CHIAKI_HANDSHAKE_KEY_SIZE) != 0
			|| mbedtls_md_hmac_update(&md, key_out, *key_out_size) != 0
			|| mbedtls_md_hmac_finish(&md, sig_out) != 0)
	{
		mbedtls_md_free(&md);
		return CHIAKI_ERR_UNKNOWN;
	}
	mbedtls_md_free(&md);
	*sig_out_size = 32;
	return CHIAKI_ERR_SUCCESS;
}

CHIAKI_EXPORT ChiakiErrorCode chiaki_ecdh_derive_secret(ChiakiECDH *ecdh,
		uint8_t *secret_out, const uint8_t *remote_key, size_t remote_key_size,
		const uint8_t *handshake_key, const uint8_t *remote_sig, size_t remote_sig_size)
{
	(void)handshake_key;
	(void)remote_sig;
	(void)remote_sig_size;

	if(mbedtls_mpi_lset(&ecdh->ctx.Qp.Z, 1) != 0
			|| mbedtls_ecp_point_read_binary(&ecdh->ctx.grp, &ecdh->ctx.Qp,
				remote_key, remote_key_size) != 0
			|| mbedtls_ecdh_compute_shared(&ecdh->ctx.grp, &ecdh->ctx.z, &ecdh->ctx.Qp,
				&ecdh->ctx.d, mbedtls_ctr_drbg_random, &ecdh->drbg) != 0
			|| mbedtls_mpi_write_binary(&ecdh->ctx.z, secret_out,
				CHIAKI_ECDH_SECRET_SIZE) != 0)
		return CHIAKI_ERR_UNKNOWN;

	return CHIAKI_ERR_SUCCESS;
}
