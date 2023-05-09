flist-messenger
===============

A fork of the original multi-platform desktop client for the F-Chat protocol.

If you have any questions about *this fork specifically*, feel free to contact Greyhoof on F-List via Note.

If you have questions about *the original version of this project*, please refer to the readme of the original repo: https://github.com/Dhwty/flist-messenger

-----
Please do not bother the original devs of this project with questions about this fork, as they are not involved with it.
-----

-----


**Changes between 0.9.1 and 0.9.5:**
* flist-messenger was migrated to Qt 6.5
* Handling of stylesheets was improved, the app will now provide default files if none exist in the app folder
* libjson was removed in favor of Qt's built-in JSON support
* WebSocket connections are now properly handled by QWebSocket

**Changes between 0.8.5 and 0.9.1:**
* Internally flist-messenger has undergone a very heavy overhaul to improve code quality. While a great deal of effort has been spent trying to keep behaviour bug free and working, it's possible that unintended bugs have crept in.
* Project files have been switched from CMake to QMake. This provides better support from Qt Creator and removes the dependance on CMake.
* '/close' has been added.
* Right-clicking URLs in chat view will now bring up a smarter context menu based upon the URL.
* The user list can be resized to see more of character names, or to show more of the chat view.
* The channel list dialog has been improved to allow sorting and filtering. (Thanks to Kythyria.)
* bugfix: Regular URLs in the chat view and logs will no longer have wacky prefixes.
* bugfix: Switching between tabs should no longer put double time stamps on every line.
* bugfix: The current character selection in channels should no longer deselect when characters join or leave the channel.
* bugfix: The character list in channels should refresh correctly with people joining, leaving and changing status.
* bugfix: RP ads from ignored users are now hidden.
* bugfix: Unignoring characters should now always work.
* bugfix: '/setdescription' will no longer convert line breaks into spaces.

**Known issues:**
* The code only implements a small subset of HyBi WebSockets. This is enough to work with F-Chat but may cause problems in the future if the server starts using more features.
* SSL connections are not verified. I had problems with F-List's SSL certificate verifying correctly. Because my attention was focused on get the HyBi WebSockets working, I've set it so that it currently ignores any errors in the certificate. The initial login will display a popup listing the errors encountered while verifying the SSL connection but it should continue as normal.

---------------

Compiling from Source
==============

* Pull the master branch for the latest "stable" version.
* Or pull the develop branch for the latest "in development" version.

For the original source, visit the Github repository here:
  https://github.com/Dhwty/flist-messenger

Compiling flist-messenger requires Qt6. The easiest way to get this on Windows or Mac is to download Qt6 from http://qt-project.org/downloads . For GNU/Linux systems, installing Qt Creator from your distro's package manager should be easiest.

With Qt Creator installed you should be able to run it and open 'flist_messenger.pro'. Setting up and compiling the project should be straight forward from there.

By default, the project will compile with the options '-Wall' and '-Werror' on. This will enable all useful warnings and will also treat all warnings as errors. It's possible that if you're using a newer compiler it will generate newer warnings that need to be fixed. If this happens, remove or comment out the following line from 'flist_messenger.pro':

    QMAKE_CXXFLAGS_DEBUG += -Werror

---------------

Code Style
==========
Please use the provided "_clang-format" file.

It can be applied using the "Beautifier" plugin within Qt Creator. (May need to be enabled and set up manually.)
