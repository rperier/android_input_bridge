LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES:=    \
    common.c         \
    aibd.c
LOCAL_MODULE := aibd
LOCAL_CFLAGS += -fno-strict-aliasing -DAPPNAME=\"aidb\"
include $(BUILD_EXECUTABLE)
