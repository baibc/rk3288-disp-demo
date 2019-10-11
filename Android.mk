LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := disp 
LOCAL_SRC_FILES := \
				disp.cpp

LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)
