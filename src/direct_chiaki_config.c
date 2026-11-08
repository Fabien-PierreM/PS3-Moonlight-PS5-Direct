#include "direct_chiaki_config.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#define FIELD_HOST (1u << 0)
#define FIELD_REGIST_KEY (1u << 1)
#define FIELD_MORNING (1u << 2)
#define FIELD_ACCOUNT_ID (1u << 3)
#define FIELD_ALL (FIELD_HOST | FIELD_REGIST_KEY | FIELD_MORNING | FIELD_ACCOUNT_ID)

static bool is_space(char value)
{
    return value == ' ' || value == '\t' || value == '\r';
}

static char *trim_left(char *value)
{
    while (is_space(*value)) {
        value++;
    }
    return value;
}

static void trim_right(char *value)
{
    size_t length = strlen(value);
    while (length > 0 && is_space(value[length - 1])) {
        value[--length] = '\0';
    }
}

static int hex_value(char value)
{
    if (value >= '0' && value <= '9') {
        return value - '0';
    }
    if (value >= 'a' && value <= 'f') {
        return value - 'a' + 10;
    }
    if (value >= 'A' && value <= 'F') {
        return value - 'A' + 10;
    }
    return -1;
}

static bool decode_hex_exact(const char *text, uint8_t *output, size_t output_size)
{
    size_t expected_length = output_size * 2;
    if (strlen(text) != expected_length) {
        return false;
    }
    for (size_t i = 0; i < output_size; i++) {
        int hi = hex_value(text[i * 2]);
        int lo = hex_value(text[i * 2 + 1]);
        if (hi < 0 || lo < 0) {
            return false;
        }
        output[i] = (uint8_t)((hi << 4) | lo);
    }
    return true;
}

static enum ps3_direct_chiaki_config_result parse_line(
    char *line, struct ps3_direct_chiaki_config *config, unsigned int *fields)
{
    char *key = trim_left(line);
    char *equals;
    char *value;

    trim_right(key);
    if (*key == '\0' || *key == '#') {
        return PS3_DIRECT_CHIAKI_CONFIG_OK;
    }

    equals = strchr(key, '=');
    if (!equals) {
        return PS3_DIRECT_CHIAKI_CONFIG_INVALID_VALUE;
    }
    *equals = '\0';
    trim_right(key);
    value = trim_left(equals + 1);
    trim_right(value);

    if (strcmp(key, "host") == 0) {
        size_t length = strlen(value);
        if (length == 0 || length > PS3_DIRECT_CHIAKI_HOST_MAX) {
            return PS3_DIRECT_CHIAKI_CONFIG_INVALID_VALUE;
        }
        memcpy(config->host, value, length + 1);
        *fields |= FIELD_HOST;
        return PS3_DIRECT_CHIAKI_CONFIG_OK;
    }
    if (strcmp(key, "regist_key_hex") == 0) {
        if (!decode_hex_exact(value, config->regist_key, sizeof(config->regist_key))) {
            return PS3_DIRECT_CHIAKI_CONFIG_INVALID_VALUE;
        }
        *fields |= FIELD_REGIST_KEY;
        return PS3_DIRECT_CHIAKI_CONFIG_OK;
    }
    if (strcmp(key, "morning_hex") == 0) {
        if (!decode_hex_exact(value, config->morning, sizeof(config->morning))) {
            return PS3_DIRECT_CHIAKI_CONFIG_INVALID_VALUE;
        }
        *fields |= FIELD_MORNING;
        return PS3_DIRECT_CHIAKI_CONFIG_OK;
    }
    if (strcmp(key, "account_id_hex") == 0) {
        if (!decode_hex_exact(value, config->account_id, sizeof(config->account_id))) {
            return PS3_DIRECT_CHIAKI_CONFIG_INVALID_VALUE;
        }
        *fields |= FIELD_ACCOUNT_ID;
        return PS3_DIRECT_CHIAKI_CONFIG_OK;
    }
    return PS3_DIRECT_CHIAKI_CONFIG_INVALID_VALUE;
}

enum ps3_direct_chiaki_config_result ps3_direct_chiaki_config_parse(
    const char *text, struct ps3_direct_chiaki_config *config)
{
    char line[512];
    size_t line_length = 0;
    unsigned int fields = 0;

    if (!text || !config) {
        return PS3_DIRECT_CHIAKI_CONFIG_INVALID_ARGUMENT;
    }
    memset(config, 0, sizeof(*config));

    for (const char *cursor = text;; cursor++) {
        char value = *cursor;
        if (value != '\n' && value != '\0') {
            if (line_length + 1 >= sizeof(line)) {
                return PS3_DIRECT_CHIAKI_CONFIG_LINE_TOO_LONG;
            }
            line[line_length++] = value;
            continue;
        }

        line[line_length] = '\0';
        enum ps3_direct_chiaki_config_result result = parse_line(line, config, &fields);
        if (result != PS3_DIRECT_CHIAKI_CONFIG_OK) {
            return result;
        }
        line_length = 0;
        if (value == '\0') {
            break;
        }
    }

    return fields == FIELD_ALL ? PS3_DIRECT_CHIAKI_CONFIG_OK
                               : PS3_DIRECT_CHIAKI_CONFIG_MISSING_FIELD;
}

const char *ps3_direct_chiaki_config_result_string(
    enum ps3_direct_chiaki_config_result result)
{
    switch (result) {
    case PS3_DIRECT_CHIAKI_CONFIG_OK:
        return "ok";
    case PS3_DIRECT_CHIAKI_CONFIG_INVALID_ARGUMENT:
        return "invalid argument";
    case PS3_DIRECT_CHIAKI_CONFIG_INVALID_VALUE:
        return "invalid profile value";
    case PS3_DIRECT_CHIAKI_CONFIG_MISSING_FIELD:
        return "missing required profile field";
    case PS3_DIRECT_CHIAKI_CONFIG_LINE_TOO_LONG:
        return "profile line too long";
    default:
        return "unknown profile error";
    }
}
