project(flist_messenger)
cmake_minimum_required(VERSION 2.8.8)
find_package(Qt5Core)
find_package(Qt5Gui)
find_package(Qt5Widgets)
find_package(Qt5Declarative)
find_package(Qt5Network)
find_package(Qt5Multimedia)
include_directories(
        ${Qt5_INCLUDE_DIRS}
        ${Qt5Widgets_INCLUDE_DIRS}
        ${Qt5Declarative_INCLUDE_DIRS}
        ${Qt5Widgets_LIBRARIES}
        ${Qt5Declarative_LIBRARIES}
        ${Qt5Core_LIBRARIES}
        ${Qt5Core_INCLUDE_DIRS}
        ${Qt5Multimedia_LIBRARIES}
        ${Qt5Multimedia_INCLUDE_DIRS}
        ${Qt5Network_LIBRARIES}
        ${Qt5Network_INCLUDE_DIRS}

        ${CMAKE_CURRENT_BINARY_DIR}
        "../libjson/Source"
)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_FLAGS "${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

#include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} "../libjson/Source")

set(flist_messenger_SRCS
        ../libjson/Source/internalJSONNode.cpp
        ../libjson/Source/JSONNode.cpp
        ../libjson/Source/JSONChildren.cpp
        ../libjson/Source/JSONDebug.cpp
        ../libjson/Source/JSONMemory.cpp
        ../libjson/Source/JSONWorker.cpp
        ../libjson/Source/JSONWriter.cpp
        flist_avatar.cpp
        flist_character.cpp
        flist_sound.cpp
        main.cpp
        flist_channel.cpp
        flist_messenger.cpp
        flist_message.cpp
        flist_parser.cpp
    )
SET(flist_messenger_HEADERS
    )
set(QtApp_RCCS resources.qrc)

QT5_WRAP_CPP(flist_messenger_HEADERS_MOC ${flist_messenger_HEADERS})
QT5_ADD_RESOURCES(QtApp_RCC_SRCS ${QtApp_RCCS})
add_executable(flist_messenger WIN32 ${flist_messenger_SRCS} ${QtApp_RCC_SRCS}   ${flist_messenger_HEADERS_MOC} resource.rc)
#target_link_libraries(flist_messenger ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTNETWORK_LIBRARY})
target_link_libraries(flist_messenger ${Qt5Widgets_LIBRARIES} ${Qt5Multimedia_LIBRARIES} ${Qt5Network_LIBRARIES} ${Qt5Core_LIBRARIES} ${Qt5Declarative_LIBRARIES})
