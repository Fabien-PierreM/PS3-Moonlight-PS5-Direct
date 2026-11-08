#include <chiaki/remote/holepunch.h>
#include <string.h>

ChiakiHolepunchRegistInfo chiaki_get_regist_info(ChiakiHolepunchSession s) { (void)s; ChiakiHolepunchRegistInfo i; memset(&i,0,sizeof(i)); return i; }
void chiaki_get_ps_selected_addr(ChiakiHolepunchSession s, char *out) { (void)s; if(out) out[0]=0; }
uint16_t chiaki_get_ps_ctrl_port(ChiakiHolepunchSession s) { (void)s; return 0; }
chiaki_socket_t *chiaki_get_holepunch_sock(ChiakiHolepunchSession s, ChiakiHolepunchPortType t) { (void)s; (void)t; return NULL; }
void chiaki_holepunch_session_fini(ChiakiHolepunchSession s) { (void)s; }
ChiakiErrorCode holepunch_session_create_offer(ChiakiHolepunchSession s) { (void)s; return CHIAKI_ERR_UNKNOWN; }
ChiakiErrorCode chiaki_holepunch_session_punch_hole(ChiakiHolepunchSession s, ChiakiHolepunchPortType t) { (void)s; (void)t; return CHIAKI_ERR_UNKNOWN; }
