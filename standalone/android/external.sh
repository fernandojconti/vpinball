#!/bin/bash

export SDL2_VERSION=2.26.5
export SDL2_TTF_VERSION=2.20.2
export SDL2_IMAGE_VERSION=2.6.3

export SERUM_SHA=c4feee2bba6faead153d9b47f28cea02d8b04fce

rm -rf external
mkdir external
mkdir external/include
mkdir external/lib

rm -rf tmp
mkdir tmp
cd tmp

#
# build freeimage, and copy to external
#

curl -sL https://downloads.sourceforge.net/project/freeimage/Source%20Distribution/3.18.0/FreeImage3180.zip -o FreeImage3180.zip
unzip FreeImage3180.zip
cd FreeImage
cp ../../freeimage/Android.mk Android.mk
cd ..

$ANDROID_NDK_HOME/ndk-build -C FreeImage \
        NDK_PROJECT_PATH=$ANDROID_NDK_HOME \
        APP_BUILD_SCRIPT=$(pwd)/FreeImage/Android.mk \
	APP_STL="c++_static" \
        APP_PLATFORM=android-19 \
        APP_ABI=arm64-v8a \
        NDK_OUT=$(pwd)/obj \
        NDK_LIBS_OUT=$(pwd)/libs

cp libs/arm64-v8a/libFreeImage.so ../external/lib

#
# download bass24 and copy to external
#

curl -sL https://www.un4seen.com/files/bass24-android.zip -o bass.zip
unzip bass.zip 
mv libs/arm64-v8a/libbass.so ../external/lib

#
# build SDL2 and copy to external
# (derived from: https://github.com/AlexanderAgd/SDL2-Android)
#

curl -sL https://github.com/libsdl-org/SDL/releases/download/release-${SDL2_VERSION}/SDL2-${SDL2_VERSION}.zip -o SDL2-${SDL2_VERSION}.zip
unzip SDL2-${SDL2_VERSION}.zip
cp -r SDL2-${SDL2_VERSION}/include ../external/include/SDL2

$ANDROID_NDK_HOME/ndk-build -C SDL2-${SDL2_VERSION} \
	NDK_PROJECT_PATH=$ANDROID_NDK_HOME \
	APP_BUILD_SCRIPT=$(pwd)/SDL2-${SDL2_VERSION}/Android.mk \
	APP_PLATFORM=android-19 \
	APP_ABI=arm64-v8a \
	NDK_OUT=$(pwd)/obj \
	NDK_LIBS_OUT=$(pwd)/libs

cp libs/arm64-v8a/libSDL2.so ../external/lib

#
# build SDL2_image and copy to external
# (derived from: https://github.com/AlexanderAgd/SDL2-Android)
#

curl -sL https://github.com/libsdl-org/SDL_image/releases/download/release-${SDL2_IMAGE_VERSION}/SDL2_image-${SDL2_IMAGE_VERSION}.zip -o SDL2_image-${SDL2_IMAGE_VERSION}.zip
unzip SDL2_image-${SDL2_IMAGE_VERSION}.zip
cp -r SDL2_image-${SDL2_IMAGE_VERSION}/SDL_image.h ../external/include/SDL2

MK_ADDON=$'include $(CLEAR_VARS)\\\n'
MK_ADDON+=$'LOCAL_MODULE := SDL2\\\n'
MK_ADDON+=$'LOCAL_SRC_FILES := '"$(pwd)/libs/arm64-v8a"$'/libSDL2.so\\\n'
MK_ADDON+=$'LOCAL_EXPORT_C_INCLUDES += '"$(pwd)/SDL2-${SDL2_VERSION}/include"$'\\\n'
MK_ADDON+="include \$(PREBUILT_SHARED_LIBRARY)"

sed -e $'/(call my-dir)/a\\\n'"$MK_ADDON" SDL2_image-${SDL2_IMAGE_VERSION}/Android.mk > tmp_mk
cp tmp_mk SDL2_image-${SDL2_IMAGE_VERSION}/Android.mk

$ANDROID_NDK_HOME/ndk-build -C SDL2_image-${SDL2_IMAGE_VERSION} \
	NDK_PROJECT_PATH=$ANDROID_NDK_HOME \
	APP_BUILD_SCRIPT="$(pwd)/SDL2_image-${SDL2_IMAGE_VERSION}/Android.mk" \
	APP_PLATFORM=android-19 \
	APP_ABI=arm64-v8a \
	APP_ALLOW_MISSING_DEPS=true \
	NDK_OUT=$(pwd)/obj \
	NDK_LIBS_OUT=$(pwd)/libs

cp libs/arm64-v8a/libSDL2_image.so ../external/lib

#
# build SDL2_ttf and copy to external
# (derived from: https://github.com/AlexanderAgd/SDL2-Android)
#

curl -sL https://github.com/libsdl-org/SDL_ttf/releases/download/release-${SDL2_TTF_VERSION}/SDL2_ttf-${SDL2_TTF_VERSION}.zip -o SDL2_ttf-${SDL2_TTF_VERSION}.zip
unzip SDL2_ttf-${SDL2_TTF_VERSION}.zip
cp -r SDL2_ttf-${SDL2_TTF_VERSION}/SDL_ttf.h ../external/include/SDL2

MK_ADDON=$'include $(CLEAR_VARS)\\\n'
MK_ADDON+=$'LOCAL_MODULE := SDL2\\\n'
MK_ADDON+=$'LOCAL_SRC_FILES := '"$(pwd)/libs/arm64-v8a"$'/libSDL2.so\\\n'
MK_ADDON+=$'LOCAL_EXPORT_C_INCLUDES += '"$(pwd)/SDL2-${SDL2_VERSION}/include"$'\\\n'
MK_ADDON+="include \$(PREBUILT_SHARED_LIBRARY)"

sed -e $'/(call my-dir)/a\\\n'"$MK_ADDON" SDL2_ttf-${SDL2_TTF_VERSION}/Android.mk > tmp_mk
cp tmp_mk SDL2_ttf-${SDL2_TTF_VERSION}/Android.mk

$ANDROID_NDK_HOME/ndk-build -C SDL2_ttf-${SDL2_TTF_VERSION} \
	NDK_PROJECT_PATH=$ANDROID_NDK_HOME \
	APP_BUILD_SCRIPT="$(pwd)/SDL2_ttf-${SDL2_TTF_VERSION}/Android.mk" \
	APP_PLATFORM=android-19 \
	APP_ABI=arm64-v8a \
	APP_ALLOW_MISSING_DEPS=true \
	NDK_OUT=$(pwd)/obj \
	NDK_LIBS_OUT=$(pwd)/libs

cp libs/arm64-v8a/libSDL2_ttf.so ../external/lib

#
# build libpinmame and copy to external
#

curl -sL https://github.com/vpinball/pinmame/archive/refs/heads/master.zip -o pinmame.zip
unzip pinmame.zip
cd pinmame-master
cp src/libpinmame/libpinmame.h ../../external/include
cp cmake/libpinmame/CMakeLists_android-arm64-v8a.txt CMakeLists.txt
cmake -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build
cp build/libpinmame.3.6.so ../../external/lib
cd ..

#
# build libserum and copy to external
#

curl -sL https://github.com/zesinger/libserum/archive/${SERUM_SHA}.zip -o libserum.zip
unzip libserum.zip
cd libserum-$SERUM_SHA
cp src/serum-decode.h ../../external/include
cmake -DCMAKE_BUILD_TYPE=Release -DUSE_ANDROID=ON -B build
cmake --build build
cp build/libserum.so ../../external/lib
cd ..
