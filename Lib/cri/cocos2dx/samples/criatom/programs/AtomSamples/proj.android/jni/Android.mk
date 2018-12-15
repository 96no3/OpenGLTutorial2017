LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := cri_ware_cocos2dx
LOCAL_SRC_FILES := ../../../../../../../../cri/cocos2dx/libs/android/$(TARGET_ARCH_ABI)/libcri_ware_android_le.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
$(call import-add-path,$(LOCAL_PATH)/../../cocos2d)
$(call import-add-path,$(LOCAL_PATH)/../../cocos2d/external)
$(call import-add-path,$(LOCAL_PATH)/../../cocos2d/cocos)
LOCAL_MODULE := cocos2dcpp_shared
LOCAL_MODULE_FILENAME := libcocos2dcpp

LOCAL_SRC_FILES := hellocpp/main.cpp \
                   ../../Classes/AppDelegate.cpp \
				   ../../Classes/AtomUtil.cpp \
				   ../../Classes/Scenes/AtomBasicScene.cpp \
				   ../../Classes/Scenes/AtomAisacScene.cpp \
				   ../../Classes/Scenes/MenuScene.cpp
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../Classes \
                    $(LOCAL_PATH)/../../../../../../../../cri/cocos2dx/include/android
LOCAL_STATIC_LIBRARIES := cocos2dx_static

LOCAL_SHARED_LIBRARIES := libcri_ware_cocos2dx
LOCAL_LDLIBS += \
        -llog \
        -lOpenSLES
include $(BUILD_SHARED_LIBRARY)


$(call import-module,.)
