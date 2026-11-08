#include <chiaki/session.h>
#include <chiaki/log.h>
#include <chiaki/common.h>
#include <chiaki/controller.h>
#include <net/net.h>
#include <net/netctl.h>
#include <sysmodule/sysmodule.h>
#include <sys/process.h>
#include <sysutil/sysutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

SYS_PROCESS_PARAM(1001, 0x100000)

static volatile int running = 1;
static void log_cb(ChiakiLogLevel level, const char *msg, void *user) { (void)user; printf("[Chiaki:%c] %s\n", chiaki_log_level_char(level), msg); }
static void event_cb(ChiakiEvent *event, void *user) {
    ChiakiSession *session = (ChiakiSession *)user;
    if(event->type == CHIAKI_EVENT_LOGIN_PIN_REQUEST) {
        const char *pin = getenv("CHIaki_LOGIN_PIN");
        if(pin) chiaki_session_set_login_pin(session, (const uint8_t *)pin, strlen(pin));
    } else if(event->type == CHIAKI_EVENT_QUIT) {
        running = 0;
    }
}
static bool video_cb(uint8_t *buf, size_t size, int32_t lost, bool recovered, void *user) {
    (void)buf; (void)size; (void)lost; (void)recovered; (void)user;
    return true;
}
static int hex(const char *s, uint8_t *out, size_t n) {
    for(size_t i=0;i<n;i++) { unsigned v; if(sscanf(s+i*2, "%2x", &v)!=1) return -1; out[i]=(uint8_t)v; }
    return s[n*2] == '\0' ? 0 : -1;
}
static char *read_file(const char *path) {
    FILE *f=fopen(path,"rb"); if(!f) return NULL; fseek(f,0,SEEK_END); long n=ftell(f); fseek(f,0,SEEK_SET); char *p=calloc(1,(size_t)n+1); if(p) fread(p,1,(size_t)n,f); fclose(f); return p;
}
static const char *value(const char *text, const char *key) {
    static char out[512]; const char *p=strstr(text,key); if(!p) return NULL; p=strchr(p,'='); if(!p) return NULL; p++; while(*p==' '||*p=='\t') p++; size_t n=0; while(p[n] && p[n]!='\r' && p[n]!='\n' && n<sizeof(out)-1) n++; memcpy(out,p,n); out[n]=0; return out;
}
int main(void) {
    sysModuleLoad(SYSMODULE_NET); if(netInitialize()!=0) return 1; netCtlInit();
    char *cfg=read_file("/dev_hdd0/game/PS5RL0001/USRDIR/chiaki.conf");
    if(!cfg) { printf("Missing chiaki.conf\n"); netCtlTerm(); netDeinitialize(); sysModuleUnload(SYSMODULE_NET); return 2; }
    const char *host=value(cfg,"host"), *rk=value(cfg,"regist_key_hex"), *morning=value(cfg,"morning_hex");
    if(!host || !rk || !morning) { free(cfg); return 3; }
    ChiakiConnectInfo ci; memset(&ci,0,sizeof(ci)); ci.ps5=true; ci.host=host; ci.packet_loss_max=0.1; ci.audio_video_disabled=CHIAKI_AUDIO_DISABLED; ci.enable_idr_on_fec_failure=true;
    if(hex(rk, (uint8_t*)ci.regist_key, sizeof(ci.regist_key)) || hex(morning,ci.morning,sizeof(ci.morning))) { free(cfg); return 4; }
    chiaki_connect_video_profile_preset(&ci.video_profile, CHIAKI_VIDEO_RESOLUTION_PRESET_720p, CHIAKI_VIDEO_FPS_PRESET_60);
    ChiakiLog log; chiaki_log_init(&log, CHIAKI_LOG_ALL, log_cb, NULL); ChiakiSession session;
    if(chiaki_session_init(&session,&ci,&log)!=CHIAKI_ERR_SUCCESS) { free(cfg); return 5; }
    chiaki_session_set_event_cb(&session,event_cb,&session); chiaki_session_set_video_sample_cb(&session,video_cb,NULL);
    if(chiaki_session_start(&session)!=CHIAKI_ERR_SUCCESS) { chiaki_session_fini(&session); free(cfg); return 6; }
    while(running) { sysUtilCheckCallback(); usleep(20000); }
    chiaki_session_stop(&session); chiaki_session_join(&session); chiaki_session_fini(&session); free(cfg);
    netCtlTerm(); netDeinitialize(); sysModuleUnload(SYSMODULE_NET); return 0;
}
