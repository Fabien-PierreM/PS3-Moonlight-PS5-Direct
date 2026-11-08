#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "direct_chiaki_config.h"

static int expect(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        return 1;
    }
    return 0;
}

int main(void)
{
    static const char profile[] =
        "# PS3 Direct Chiaki profile\n"
        "host = 192.168.1.50\n"
        "regist_key_hex = 000102030405060708090a0b0c0d0e0f\n"
        "morning_hex = f0e0d0c0b0a090807060504030201000\n"
        "account_id_hex = 0123456789abcdef\n";
    struct ps3_direct_chiaki_config config;

    if (ps3_direct_chiaki_config_parse(profile, &config) != PS3_DIRECT_CHIAKI_CONFIG_OK) {
        fprintf(stderr, "FAIL: valid profile was rejected\n");
        return 1;
    }
    if (expect(strcmp(config.host, "192.168.1.50") == 0, "host is retained") ||
        expect(config.regist_key[0] == 0x00 && config.regist_key[15] == 0x0f,
               "registration key is decoded") ||
        expect(config.morning[0] == 0xf0 && config.morning[15] == 0x00,
               "morning key is decoded") ||
        expect(config.account_id[0] == 0x01 && config.account_id[7] == 0xef,
               "account id is decoded")) {
        return 1;
    }

    puts("direct Chiaki profile parser: PASS");
    return 0;
}
