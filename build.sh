#!/bin/sh

BUILD_DIR="./build"
RELEASE_DIR="."

# 设置架构，默认为 x86
ARCH=${ARCH:-x86}

# 根据 ARCH 设置编译器
case ${ARCH} in
	x86|X86)
		CC="gcc"
		CXX="g++"
		echo "Building for x86 (using gcc)"
		;;
	ARM64|arm64|aarch64)
		CC="aarch64-linux-gnu-gcc"
		CXX="aarch64-linux-gnu-g++"
		echo "Building for ARM64 (using aarch64-linux-gnu-gcc)"
		;;
	*)
		echo "Error: Unknown ARCH=${ARCH}. Supported: x86, ARM64"
		exit 1
		;;
esac

case ${1} in
	clean)
		echo "Clean..."
		rm -vrf ${BUILD_DIR}
		;;
	debug)
		rm -vrf ${BUILD_DIR}
		cmake -S . -B ${BUILD_DIR} -D CMAKE_C_COMPILER=${CC} -D CMAKE_CXX_COMPILER=${CXX} -D CMAKE_BUILD_TYPE=Debug
		cmake --build ${BUILD_DIR}
		cmake --install ${BUILD_DIR} --prefix ${RELEASE_DIR}
		;;
	*)
		rm -vrf ${BUILD_DIR}
		cmake -S . -B ${BUILD_DIR} -D CMAKE_C_COMPILER=${CC} -D CMAKE_CXX_COMPILER=${CXX}
		cmake --build ${BUILD_DIR}
		cmake --install ${BUILD_DIR} --prefix ${RELEASE_DIR}
		;;
esac
