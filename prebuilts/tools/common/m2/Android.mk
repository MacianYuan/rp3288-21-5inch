LOCAL_PATH:= $(call my-dir)

###########################################

# trove prebuilt. Module stem is chosen so it can be used as a static library.

include $(CLEAR_VARS)

LOCAL_MODULE := trove-prebuilt
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := repository/net/sf/trove4j/trove4j/1.1/trove4j-1.1.jar
LOCAL_IS_HOST_MODULE := true
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_BUILT_MODULE_STEM := javalib.jar
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)

include $(BUILD_PREBUILT)

###########################################

# net.bytebuddy prebuilt.
# org.mockito prebuilt.
# org.objenesis prebuilt.
# com.squareup.haha prebuilt.
# com.google.truth prebuilt.

include $(CLEAR_VARS)

LOCAL_PREBUILT_STATIC_JAVA_LIBRARIES := \
    byte-buddy-prebuilt-jar:repository/net/bytebuddy/byte-buddy/1.6.5/byte-buddy-1.6.5.jar \
    mockito2-prebuilt-jar:repository/org/mockito/mockito-core/2.7.6/mockito-core-2.7.6.jar \
    objenesis-prebuilt-jar:repository/org/objenesis/objenesis/2.5/objenesis-2.5.jar \
    squareup-haha-prebuilt:repository/com/squareup/haha/haha/2.0.2/haha-2.0.2.jar \
    truth-prebuilt-jar:repository/com/google/truth/truth/0.28/truth-0.28.jar

include $(BUILD_MULTI_PREBUILT)

###########################################
# org.mockito prebuilt for Robolectric
###########################################

include $(CLEAR_VARS)

LOCAL_MODULE := mockito-robolectric-prebuilt

LOCAL_STATIC_JAVA_LIBRARIES := \
    byte-buddy-prebuilt-jar \
    mockito2-prebuilt-jar \
    objenesis-prebuilt-jar

include $(BUILD_STATIC_JAVA_LIBRARY)

###########################################
# com.google.truth prebuilt
###########################################

include $(CLEAR_VARS)

LOCAL_MODULE := truth-prebuilt

LOCAL_STATIC_JAVA_LIBRARIES := \
    truth-prebuilt-jar \
    guava

include $(BUILD_STATIC_JAVA_LIBRARY)

include $(CLEAR_VARS)

LOCAL_PREBUILT_JAVA_LIBRARIES := \
    truth-prebuilt-host-jar:repository/com/google/truth/truth/0.28/truth-0.28.jar

include $(BUILD_HOST_PREBUILT)

include $(CLEAR_VARS)

LOCAL_MODULE := truth-host-prebuilt

LOCAL_STATIC_JAVA_LIBRARIES := \
    truth-prebuilt-host-jar \
    guavalib

include $(BUILD_HOST_JAVA_LIBRARY)

