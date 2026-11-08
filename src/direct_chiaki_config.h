#ifndef DIRECT_CHIAKI_CONFIG_H
#define DIRECT_CHIAKI_CONFIG_H

#include <stdint.h>

#define PS3_DIRECT_CHIAKI_HOST_MAX 255
#define PS3_DIRECT_CHIAKI_REGIST_KEY_SIZE 16
#define PS3_DIRECT_CHIAKI_MORNING_SIZE 16
#define PS3_DIRECT_CHIAKI_ACCOUNT_ID_SIZE 8

enum ps3_direct_chiaki_config_result {
    PS3_DIRECT_CHIAKI_CONFIG_OK = 0,
    PS3_DIRECT_CHIAKI_CONFIG_INVALID_ARGUMENT,
    PS3_DIRECT_CHIAKI_CONFIG_INVALID_VALUE,
    PS3_DIRECT_CHIAKI_CONFIG_MISSING_FIELD,
    PS3_DIRECT_CHIAKI_CONFIG_LINE_TOO_LONG,
};

struct ps3_direct_chiaki_config {
    char host[PS3_DIRECT_CHIAKI_HOST_MAX + 1];
    uint8_t regist_key[PS3_DIRECT_CHIAKI_REGIST_KEY_SIZE];
    uint8_t morning[PS3_DIRECT_CHIAKI_MORNING_SIZE];
    uint8_t account_id[PS3_DIRECT_CHIAKI_ACCOUNT_ID_SIZE];
};

/*
 * Parses a PS3-local, owner-supplied profile. The input has fields host,
 * regist_key_hex, morning_hex and account_id_hex. It never logs secret fields.
 */
enum ps3_direct_chiaki_config_result ps3_direct_chiaki_config_parse(
    const char *text, struct ps3_direct_chiaki_config *config);

const char *ps3_direct_chiaki_config_result_string(
    enum ps3_direct_chiaki_config_result result);

#endif
