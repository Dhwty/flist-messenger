flist-messenger
===============

A multi-platform desktop client for the F-Chat protocol. If you have any questions, feel free to contact Alicia Sprig on F-chat, or post in the [desktop client's group forums](https://www.f-list.net/group.php?group=f-chat%20desktop%20client). More about this client can be found here: http://wiki.f-list.net/index.php/F-Chat_Desktop_Client

**Changes between 0.8.5 and 0.9.1:**
* Internally flist-messenger has under gone a very heavy overhaul to improve code quality. While a great deal of effort has been spent trying to keep behaviour bug free and working, it's possible that unintended bugs have crept in.
* Project files have been switched from CMake to QMake. This provides better support from Qt Creator and removes the dependance on CMake.
* bugfix: Switching between tabs should no longer put double time stamps on every line.
* bugfix: The current character selection in channels should no longer deselect when characters join or leave the channel.
* bugfix: The character list in channels should refresh correctly with people joining, leaving and changing status.
* bugfix: RP ads from ignored users are now hidden.
* bugfix: Unignoring characters should now always work.

**Known issues:**
* Sound functionality is currently reduced when compared to version 0.8.5.
* The code only implements a small subset of HyBi WebSockets. This is enough to work with F-Chat but may cause problems in the future if the server starts using more features.
* SSL connections are not verified. I had problems with F-List's SSL certificate verifying correctly. Because my attention was focused on get the HyBi WebSockets working, I've set it so that it currently ignores any errors in the certificate. The initial login will display a popup listing the errors encountered while verifying the SSL connection but it should continue as normal.

---------------

Compiling from Source
==============

For the latest stable source visit the Github repository here:
  https://github.com/Dhwty/flist-messenger

Bleeding edge development can be found here: 
  https://github.com/aliciasprig/flist-messenger

Compiling flist-messenger requires Qt4 or Qt5. The easiest way to get this on Windows or Mac is to download Qt5 from http://qt-project.org/downloads . For GNU/Linux systems, installing Qt Creator from your distro's package manager should be easiest.

With Qt Creator installed you should be able to run it and open 'flist_messenger.pro'. Setting up and compiling the project should be straight forward from there.

By default, the project will compile with the options '-Wall' and '-Werror' on. This will enable all useful warnings and will also treat all warnings as errors. It's possible that if you're using a newer compiler it will generate newer warnings that need to be fixed. If this happens, remove or comment out the following line from 'flist_messenger.pro':

    QMAKE_CXXFLAGS_DEBUG += -Werror

---------------

Versions for Other Platforms
==============
0.8.5 for Open Pandora: http://repo.openpandora.org/?page=detail&app=fchat-001
