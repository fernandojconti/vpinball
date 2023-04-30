#!/bin/bash

export SDL2_VERSION=2.26.5
export SDL2_TTF_VERSION=2.20.2
export SDL2_IMAGE_VERSION=2.6.3

export SERUM_SHA=c4feee2bba6faead153d9b47f28cea02d8b04fce

rm -rf external
mkdir external
mkdir external/framework
mkdir external/include
mkdir external/lib

rm -rf tmp
mkdir tmp
cd tmp

#
# build freeimage and copy to external
#

curl -sL https://downloads.sourceforge.net/project/freeimage/Source%20Distribution/3.18.0/FreeImage3180.zip -o FreeImage3180.zip
unzip FreeImage3180.zip
cd FreeImage
patch -i ../../freeimage/FreeImage3180.patch
cp ../../freeimage/Makefile.tvos.arm64 .
make -f Makefile.tvos.arm64
cp Dist/libfreeimage.a ../../external/lib
cd ..

#
# copy bass24 framework to external
# 
# Note: The BASS library needs to be replaced with the tvOS version.
# Leaving the iOS version here so we can test the compile. It will fail when linking. 
#

curl -s https://www.un4seen.com/files/bass24-ios.zip -o bass.zip
unzip bass.zip 
mv bass.xcframework/ios-arm64_armv7_armv7s/bass.framework ../external/framework

#
# build SDL2 and copy to external
#

curl -sL https://github.com/libsdl-org/SDL/releases/download/release-${SDL2_VERSION}/SDL2-${SDL2_VERSION}.zip -o SDL2-${SDL2_VERSION}.zip
unzip SDL2-${SDL2_VERSION}.zip
cp -r SDL2-${SDL2_VERSION}/include ../external/include/SDL2

xcrun xcodebuild -project "SDL2-${SDL2_VERSION}/Xcode/SDL/SDL.xcodeproj" -target "Static Library-tvOS" -sdk appletvos -configuration Release clean build CONFIGURATION_BUILD_DIR="$(pwd)/sdl-build"
cp sdl-build/libSDL2.a ../external/lib

#
# build SDL2_image and copy to external
#

curl -sL https://github.com/libsdl-org/SDL_image/releases/download/release-${SDL2_IMAGE_VERSION}/SDL2_image-${SDL2_IMAGE_VERSION}.zip -o SDL2_image-${SDL2_IMAGE_VERSION}.zip
unzip SDL2_image-${SDL2_IMAGE_VERSION}.zip
cp -r SDL2_image-${SDL2_IMAGE_VERSION}/SDL_image.h ../external/include/SDL2

xcrun xcodebuild -project "SDL2_image-${SDL2_IMAGE_VERSION}/Xcode/SDL_image.xcodeproj" -target "Static Library" -sdk appletvos -configuration Release clean build CONFIGURATION_BUILD_DIR="$(pwd)/sdl_image-build"
cp sdl_image-build/libSDL2_image.a ../external/lib

#
# build SDL2_ttf and copy to external
#

curl -sL https://github.com/libsdl-org/SDL_ttf/releases/download/release-${SDL2_TTF_VERSION}/SDL2_ttf-${SDL2_TTF_VERSION}.zip -o SDL2_ttf-${SDL2_TTF_VERSION}.zip
unzip SDL2_ttf-${SDL2_TTF_VERSION}.zip
cp -r SDL2_ttf-${SDL2_TTF_VERSION}/SDL_ttf.h ../external/include/SDL2

xcrun xcodebuild -project "SDL2_ttf-${SDL2_TTF_VERSION}/Xcode/SDL_ttf.xcodeproj" -target "Static Library" -sdk appletvos -configuration Release clean build CONFIGURATION_BUILD_DIR="$(pwd)/sdl_ttf-build"
cp sdl_ttf-build/libSDL2_ttf.a ../external/lib

#
# build libpinmame and copy to external
#

curl -sL https://github.com/vpinball/pinmame/archive/refs/heads/master.zip -o pinmame.zip
unzip pinmame.zip
cd pinmame-master
cp src/libpinmame/libpinmame.h ../../external/include
cp cmake/libpinmame/CMakeLists_tvos-arm64.txt CMakeLists.txt
cmake -DCMAKE_BUILD_TYPE=Release -B build/Release
cmake --build build/Release
cp build/Release/libpinmame.3.6.a ../../external/lib
cd ..

#
# build libserum and copy to external
#

curl -sL https://github.com/zesinger/libserum/archive/${SERUM_SHA}.zip -o libserum.zip
unzip libserum.zip
cd libserum-$SERUM_SHA
cp src/serum-decode.h ../../external/include
cmake -DCMAKE_BUILD_TYPE=Release -DUSE_TVOS=ON -B build
cmake --build build
cp build/libserum.a ../../external/lib
cd ..
