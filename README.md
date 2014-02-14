flist-messenger
===============

**0.8.5 WORKS, THANKS TO ALICIA SPRIG! And it's compiled for Windows and everything!**


**Known issues:**

The code only implements a small subset of HyBi WebSockets. This is enough to work with F-Chat but may cause problems in the future if the server starts using more features.

SSL connections are not verified. I had problems with F-List's SSL certificate verifying correctly. Because my attention was focused on get the HyBi WebSockets working, I've set it so that it currently ignores any errors in the certificate. The initial login will display a popup listing the errors encountered while verifying the SSL connection but it should continue as normal.


---------------



A multi-platform desktop client for the F-Chat protocol. If you have any questions, feel free to contact Viona/Bastogne on F-chat, or post in the desktop client's group forums.

THE LATEST CODE IS THE 0.9.0 BRANCH. (But, 0.9.0 hasn't been updated for SSL or HyBi.)
- If you would like to work on this client, please work with 0.9.0. 
- If you would like to compile and use this client, I suggest sticking to 0.8.x, for now.

To build 0.8.x, you will need:
- Cmake 2.6 or newer
- Qt4 (On Windows, installing Qt SDK 1.2.1 is the easiest way to get this.)
   or
- Cmake 2.8.8 or newer
- Qt5 (Replace 'CMakeLists.txt' with the one from the 'qt5' directory.)

To build 0.9.0, you will need:
- the libjson directory from 0.8.x
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
0.8.5 for Open Pandora: http://repo.openpandora.org/?page=detail&app=fchat-001
