flist-messenger
===============

A multi-platform desktop client for the F-Chat protocol. If you have any questions, feel free to contact Viona/Bastogne on F-chat, or post in the desktop client's group forums.

THE LATEST CODE IS THE 0.9.0 FOLDER. 
- If you would like to work on this client, please work with 0.9.0. 
- If you would like to compile and use this client, I suggest sticking to 0.8.4, for now.

To build 0.8.4, you will need:
- Cmake
- Qt4 (not Qt5! Qt5 declines to build 0.8.4!)

To build 0.9.0, you will need:
- the libjson directory from 0.8.4
- Cmake 2.8.8 or higher
- Qt5

If make panics about 'reduce relocations', add the following line to CMakeLists.txt, after 'set(CMAKE_AUTOMOC ON)':
  set(CMAKE_CXX_FLAGS "${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")
  
If you are compiling on Windows, and QtCreator panics about icon.ico being an 'Invalid Argument', in CMakeLists.txt, remove ' resources.rc' from line 69, save the file, and try again.

More about this client, including detailed instructions for building under linux, can be found here: http://wiki.f-list.net/index.php/F-Chat_Desktop_Client

Most user bug reports will show up here: http://www.f-list.net/forum.php?forum=1698

Updated bugs for 0.9.0 will be added here, in the issues tracker, as soon as the Deer gets this thing to compile. 

Versions for Other Platforms
==============
0.8.4 for Open Pandora: http://repo.openpandora.org/?page=detail&app=fchat-001
