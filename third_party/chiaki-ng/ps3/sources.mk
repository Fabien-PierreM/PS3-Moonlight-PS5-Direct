# Chiaki-ng sources for the first PS3 direct-LAN client build.
# GUI, CLI, Internet Remote Play (RUDP/holepunch) and decoders are excluded.
CHIAKI_PS3_SOURCES := \
	lib/src/common.c \
	lib/src/sock.c \
	lib/src/session.c \
	lib/src/thread.c \
	lib/src/base64.c \
	lib/src/http.c \
	lib/src/log.c \
	lib/src/ctrl.c \
	lib/src/rpcrypt.c \
	lib/src/takion.c \
	lib/src/senkusha.c \
	lib/src/streamconnection.c \
	lib/src/ecdh.c \
	lib/src/launchspec.c \
	lib/src/random.c \
	lib/src/gkcrypt.c \
	lib/src/audio.c \
	lib/src/audioreceiver.c \
	lib/src/audiosender.c \
	lib/src/videoreceiver.c \
	lib/src/frameprocessor.c \
	lib/src/packetstats.c \
	lib/src/congestioncontrol.c \
	lib/src/stoppipe.c \
	lib/src/reorderqueue.c \
	lib/src/feedback.c \
	lib/src/feedbacksender.c \
	lib/src/controller.c \
	lib/src/takionsendbuffer.c \
	lib/src/time.c \
	lib/src/fec.c \
	lib/src/orientation.c \
	lib/src/bitstream.c \
	lib/src/aia.c

# These files are intentionally outside the LAN-first milestone:
# lib/src/discovery.c lib/src/discoveryservice.c lib/src/regist.c
# lib/src/remote/holepunch.c lib/src/remote/rudp.c lib/src/remote/rudpsendbuffer.c
