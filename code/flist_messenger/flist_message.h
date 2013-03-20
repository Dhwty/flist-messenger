/*
 * Copyright (c) 2011, F-list.net
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 * the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the
 * following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
 * following disclaimer in the documentation and/or other materials provided with the distribution.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

#ifndef FLIST_MESSAGE_H
#define FLIST_MESSAGE_H

#include"flist_parser.h"
#include"flist_channel.h"
#include"flist_character.h"
#include"flist_messenger.h"
#include"flist_sound.h"
#include<string>
#include<QApplication>
#include<QHash>
#include<QTime>
#include<QTextBrowser>

class flist_messenger;
class FChannel;
class FMessage
{
public:
	static void initClass(bool ping, bool alwaysPing, flist_messenger* window);
	static bool doPing;
	static bool doAlwaysPing;
	static bool doSounds;
	static flist_messenger* window;
	static BBCodeParser* bbparser;
	static FSound* sound;
	static QStringList pingList;
	static FChannel* console;
	static QHash<QString, FChannel*> channelList;
	static FChannel* currentPanel;
	static QString selfName;
	static QTextBrowser* textField;

	bool pings() {return doesPing;}
	QString getOutput() { return done ? output : "" ; }

	enum MessageType
	{
		MESSAGETYPE_CHANMESSAGE,
		MESSAGETYPE_PRIVMESSAGE,
		MESSAGETYPE_ROLEPLAYAD,
		MESSAGETYPE_BROADCAST,
		MESSAGETYPE_SYSTEM,
		MESSAGETYPE_REPORT,
		MESSAGETYPE_MAX
	};
	enum SystemMessageType
	{
		SYSTYPE_JOIN, // Leave/join messages
		SYSTYPE_ONLINE, // online/offline/status messages
		SYSTYPE_KICKBAN, // Kicks or bans
		SYSTYPE_ADDOP, // Adding/removing OPs, channel OPs, etc.
		SYSTYPE_FEEDBACK, // Feedback about the user having done something.
		SYSTYPE_DICE, // Rolling dice, etc.
		SYSTYPE_MAX
	};
	FMessage(MessageType msgType,		FChannel* channel, FCharacter* character, QString& msg, FChannel* currentPanel);
	FMessage(MessageType msgType,		FChannel* channel, FCharacter* character, QString& msg, FChannel* currentPanel, QHash<QString, FChannel*>& channelList);
	FMessage(SystemMessageType sysType, FChannel* channel, FCharacter* character, QString& msg, FChannel* currentPanel);
	~FMessage();
private:
	void parse();
	bool checkPing();
	bool checkSysPing();
	void checkButtonColor();
	void doPings();
	QString charNameToHtml();
	MessageType type;
	SystemMessageType systemType;
	QString message;
	FCharacter* sender;
	FChannel* channel;
	bool doesPing;
	bool done;
	QString output;
	QString pingReason;

	QString toString();
};

#endif // FLIST_MESSAGE_H
