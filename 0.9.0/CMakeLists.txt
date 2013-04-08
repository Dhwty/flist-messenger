project(flist_messenger)
cmake_minimum_required(VERSION 2.8.8)
find_package(Qt5Core)
find_package(Qt5Gui)
find_package(Qt5Widgets)
find_package(Qt5Declarative)
find_package(Qt5Network)
find_package(Qt5Multimedia)
include_directories(
        ${Qt5Widgets_INCLUDE_DIRS}
        ${Qt5Declarative_INCLUDE_DIRS}
        ${Qt5Widgets_LIBRARIES}
        ${Qt5Declarative_LIBRARIES}
        ${Qt5Core_LIBRARIES}
        ${Qt5Core_INCLUDE_DIRS}

	${CMAKE_CURRENT_BINARY_DIR} 
	"../libjson/Source"
)
set(CMAKE_AUTOMOC ON)
set(flist_messenger_SRCS
	../libjson/Source/internalJSONNode.cpp
	../libjson/Source/JSONNode.cpp
	../libjson/Source/JSONChildren.cpp
	../libjson/Source/JSONDebug.cpp
	../libjson/Source/JSONMemory.cpp
	../libjson/Source/JSONWorker.cpp
	../libjson/Source/JSONWriter.cpp
	flist_character.cpp
	flist_sound.cpp
	main.cpp
	flist_channel.cpp
	flist_messenger.cpp
	flist_parser.cpp
	flist_settings.cpp
	./gui/flist_avatar.cpp
	./gui/flist_gui.cpp
	./network/flist_network.cpp
	./message/flist_message.cpp
	./message/flist_systemmessage.cpp
	./message/flist_chatmessage.cpp
	./message/flist_advertmessage.cpp
	./message/flist_broadcastmessage.cpp
	./message/flist_reportmessage.cpp
)
SET(flist_messenger_HEADERS
#        flist_character.h
#        flist_sound.h
#        flist_channel.h
#        flist_messenger.h
#        flist_parser.h
#        flist_settings.h
#        ./gui/flist_avatar.h
#        ./gui/flist_gui.h
#        ./network/flist_network.h
#        ./message/flist_message.h
#        ./message/flist_systemmessage.h
#        ./message/flist_chatmessage.h
#        ./message/flist_advertmessage.h
#        ./message/flist_broadcastmessage.h
#        ./message/flist_reportmessage.h
)
set(QtApp_RCCS resources.qrc)


QT5_WRAP_CPP(flist_messenger_HEADERS_MOC ${flist_messenger_HEADERS})
QT5_ADD_RESOURCES(QtApp_RCC_SRCS ${QtApp_RCCS})
#qt5_automoc(${flist_messenger_SRCS})
add_executable(flist_messenger WIN32 ${flist_messenger_SRCS} ${QtApp_RCC_SRCS}   ${flist_messenger_HEADERS_MOC} resource.rc)
#target_link_libraries(flist_messenger ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTNETWORK_LIBRARY})
target_link_libraries(flist_messenger ${Qt5Widgets_LIBRARIES} ${Qt5Multimedia_LIBRARIES} ${Qt5Network_LIBRARIES} ${Qt5Core_LIBRARIES} ${Qt5Declarative_LIBRARIES})
