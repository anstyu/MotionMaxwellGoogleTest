cmake_minimum_required(VERSION 3.14)

#set(CMAKE_C_COMPILER "/opt/rh/devtoolset-8/root/usr/bin/gcc")
#set(CMAKE_CXX_COMPILER "/opt/rh/devtoolset-8/root/usr/bin/g++")

# GoogleTest requires at least C++14
set(CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)


project(MotionMaxwellGoogleTest)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG f8d7d77c06936315286eb55f8de22cd23c188571
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# conan
IF(WIN32)	# window conan path 
	IF(EXISTS $ENV{ANSYS_CONAN_VENV_PATH} ) 
		MESSAGE("Add ANSYS_CONAN_VENV_PATH to Path")
		SET(ENV{PATH} "$ENV{PATH};$ENV{ANSYS_CONAN_VENV_PATH}/Scripts")
	ELSEIF()
		MESSAGE("ANSYS_CONAN_VENV_PATH is not defined")
	ENDIF()
ELSE()
# linux conan path 
	find_program(CONAN_CMD conan HINTS $ENV{HOME}/ANSYSconan/ConanVenv/bin)
	message("conan_cmd : ${CONAN_CMD}")
	if(NOT CONAN_CMD)
		message("warning :: conan programe not founded")
	else()
		SET(ENV{PATH} "$ENV{PATH}:$ENV{HOME}/ANSYSconan/ConanVenv/bin")
		message("path : $ENV{PATH}")
	endif()

	find_path(CONAN_PATH conan_lib HINTS $ENV{HOME})
	if(NOT CONAN_PATH)
	 	message("warning :: conan library path not founded")
	else()
		SET(ENV{LD_LIBRARY_PATH} "$ENV{LD_LIBRARY_PATH}:${CONAN_PATH}/conan_lib")
	endif()
  SET(ENV{LD_LIBRARY_PATH} "$ENV{LD_LIBRARY_PATH}:$ENV{HOME}/Conan/Installation/CPython/3_7/linx64/Release/python/lib")
	message("lib path : $ENV{LD_LIBRARY_PATH}")
ENDIF()

# Download automatically, you can also just copy the conan.cmake file
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
   message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
   file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/master/conan.cmake"
                  "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

message("CONAN_CMAKE_INSTALL START")

INCLUDE("${CMAKE_BINARY_DIR}/conan.cmake")

SET(CONAN_DISABLE_CHECK_COMPILER TRUE)

IF(WIN32)
    CONAN_CMAKE_RUN(
        REQUIRES cpython/3.10.12@thirdparty/stable
        #REQUIRES googletest/1.10.0@thirdparty/stable
        PROFILE windows_x86_64_vs2019 release
	)


ELSE()

	CONAN_CMAKE_RUN(
        REQUIRES cpython/3.10.12@thirdparty/stable
	      REQUIRES boost/1.71.0@thirdparty/stable
        PROFILE linux_x86_64_gcc_8.2.0_CXX03_ABI release
		)

ENDIF()

# this does not change the output directory
CONAN_BASIC_SETUP(NO_OUTPUT_DIRS)

#find_package(Boost 1.63.0 REQUIRED COMPONENTS filesystem )
add_executable(
  MotionMaxwellGoogleTest
  test.cpp 
  pch.cpp
  )

target_link_libraries(
  MotionMaxwellGoogleTest
  GTest::gtest_main
  stdc++fs
  ${CMAKE_DL_LIBS}
  )

include(GoogleTest)

add_subdirectory(../MotionMaxwellCPython lib)

