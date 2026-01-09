cmake_minimum_required(VERSION 3.16)
project(UAFXController LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Core)

# RtMidi 찾기 (시스템에 설치되어 있다고 가정, 예: brew install rtmidi)
# pkg-config를 사용하거나 라이브러리를 직접 링크합니다.
find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(RTMIDI rtmidi)
endif()

set(PROJECT_SOURCES
    src/main.cpp
    src/MainWindow.cpp src/MainWindow.h
    src/MidiController.cpp src/MidiController.h
    src/CustomWidgets.cpp src/CustomWidgets.h
)

add_executable(UAFXController ${PROJECT_SOURCES})

target_link_libraries(UAFXController PRIVATE Qt6::Widgets Qt6::Core)
target_include_directories(UAFXController PRIVATE src)

if(RTMIDI_FOUND)
    target_include_directories(UAFXController PRIVATE ${RTMIDI_INCLUDE_DIRS})
    target_link_libraries(UAFXController PRIVATE ${RTMIDI_LIBRARIES})
else()
    # Fallback: macOS Homebrew 및 표준 경로 수동 검색
    find_path(RTMIDI_INCLUDE_DIR NAMES rtmidi/RtMidi.h
        PATHS /opt/homebrew/include /usr/local/include
    )
    find_library(RTMIDI_LIBRARY NAMES rtmidi
        PATHS /opt/homebrew/lib /usr/local/lib
    )

    if(RTMIDI_INCLUDE_DIR AND RTMIDI_LIBRARY)
        message(STATUS "Found RtMidi manually: ${RTMIDI_LIBRARY}")
        target_include_directories(UAFXController PRIVATE ${RTMIDI_INCLUDE_DIR})
        target_link_libraries(UAFXController PRIVATE ${RTMIDI_LIBRARY})
    else()
        message(WARNING "RtMidi not found. Build might fail.")
        target_link_libraries(UAFXController PRIVATE rtmidi)
    endif()
endif()

# 실행 시 설정 파일을 찾을 수 있도록 복사 (선택 사항)
file(COPY controls.json DESTINATION ${CMAKE_BINARY_DIR})