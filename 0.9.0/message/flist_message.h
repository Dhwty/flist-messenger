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

#include<QTime>
#include<QStringList>
#include"../flist_parser.h"
#include"../flist_character.h"
#include"../flist_channel.h"
class flist_messenger;
class FChannel;
class FCharacter;

class FMessage
{
public:
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
    static void initClass(bool ping, bool alwaysPing, flist_messenger* mainprog);
	static bool doPing;
    static bool doAlwaysPing;
    static flist_messenger* mainprog;
    static BBCodeParser* bbparser;
    static QStringList pingList;
    static QString selfName;
	static QHash<MessageType, QString> cssClassNames;

	bool pings() {return doesPing;}
	QString getOutput() { return done ? output : "" ; }

    FMessage(FChannel* channel, FCharacter* character, QString& msg);
	~FMessage();
protected:
    virtual void parse() = 0;
    virtual bool checkPing() = 0;
    bool checkSysPing();
	QString charNameToHtml();
	MessageType type;
	QString message;
	FCharacter* sender;
	FChannel* channel;
	bool doesPing;
	bool done;
    QString output;
	QString toString();
};

#endif // FLIST_MESSAGE_H
