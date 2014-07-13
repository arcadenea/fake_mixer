# KallistiOS ##version##
#
# libmp3 Makefile

TARGET = libFAKE_mixer.a
OBJS = FAKE_mixer.o
LOCAL_CLEAN = *.o *.a

KOS_CFLAGS += -I../include/mikmod

include $(KOS_BASE)/addons/Makefile.prefab
