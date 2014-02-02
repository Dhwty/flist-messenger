project(flist_messenger)
cmake_minimum_required(VERSION 2.6)
find_package(Qt4 REQUIRED)

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} "../libjson/Source")

set(flist_messenger_SRCS ../libjson/Source/internalJSONNode.cpp ../libjson/Source/JSONNode.cpp ../libjson/Source/JSONChildren.cpp ../libjson/Source/JSONDebug.cpp
		../libjson/Source/JSONMemory.cpp ../libjson/Source/JSONWorker.cpp ../libjson/Source/JSONWriter.cpp
flist_avatar.cpp   flist_character.cpp  flist_sound.cpp     main.cpp
flist_channel.cpp  flist_messenger.cpp  flist_message.cpp   flist_parser.cpp)
SET(flist_messenger_HEADERS flist_avatar.h)
set(QtApp_RCCS resources.qrc)

QT4_WRAP_CPP(flist_messenger_HEADERS_MOC ${flist_messenger_HEADERS})
QT4_ADD_RESOURCES(QtApp_RCC_SRCS ${QtApp_RCCS})
qt4_automoc(${flist_messenger_SRCS})
add_executable(flist_messenger WIN32 ${flist_messenger_SRCS} ${QtApp_RCC_SRCS}   ${flist_messenger_HEADERS_MOC} resource.rc)
target_link_libraries(flist_messenger ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTNETWORK_LIBRARY})
