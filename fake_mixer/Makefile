# Fake lib, can be compiled with Mikmod or Modplug support.
# Please notice that Mikmod only supports PCM Mono (no ADPCM).

TARGET = libFAKE_mixer.a

OBJS = FAKE_mixer.o mm.o

LOCAL_CLEAN = *.o *.a

KOS_CFLAGS += -I../libmikmod-3.3.8/include -I../SDL-1.2.13/include

include $(KOS_BASE)/addons/Makefile.prefab
