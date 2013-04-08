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

#include "flist_message.h"


bool						FMessage::doPing = true;
bool						FMessage::doAlwaysPing = true;
BBCodeParser*				FMessage::bbparser = 0;
QStringList					FMessage::pingList;
QString						FMessage::selfName = "";
flist_messenger*			FMessage::mainprog = 0;
QHash<FMessage::MessageType, QString> FMessage::cssClassNames;

void FMessage::initClass(bool ping, bool alwaysPing, flist_messenger *mainprog)
{
    FMessage::mainprog = mainprog;
    bbparser = new BBCodeParser;
	doAlwaysPing = alwaysPing;
    doPing = ping;

	MESSAGETYPE_CHANMESSAGE,
	MESSAGETYPE_PRIVMESSAGE,
	MESSAGETYPE_ROLEPLAYAD,
	MESSAGETYPE_BROADCAST,
	MESSAGETYPE_SYSTEM,
	MESSAGETYPE_REPORT;

	cssClassNames[MESSAGETYPE_CHANMESSAGE] = "ChannelMessage ";
	cssClassNames[MESSAGETYPE_PRIVMESSAGE] = "PrivateMessage ";
	cssClassNames[MESSAGETYPE_ROLEPLAYAD] = "RolePlayAd ";
	cssClassNames[MESSAGETYPE_BROADCAST] = "Broadcast ";
	cssClassNames[MESSAGETYPE_SYSTEM] = "SystemMessage ";
	cssClassNames[MESSAGETYPE_REPORT] = "Report";
}

FMessage::FMessage(FChannel* channel, FCharacter* character, QString& msg) :
    sender(character),
    message(msg),
    channel(channel)
{
}

FMessage::~FMessage(){}


QString FMessage::charNameToHtml()
{
	QString rv="";
	QString ch=sender->name();
	QString chop = ch;
	QString genderColor = sender->genderColor().name();
	bool isOp = sender->isChatOp() || channel->isOp(sender);
	if ( isOp )
		chop = "<img src=\":/images/auction-hammer.png\" />" + ch;
	if ( message.startsWith ( "/me" ) )
		rv = "<i>*<b><a style=\"color: " + genderColor + "\" href=\"#USR-" + ch + "\">" + chop + "</a></b> " + message.mid ( 4, -1 ) + "</i>";
	else if ( message.startsWith ( "/me 's" ) )
		rv = "<i>*<b><a style=\"color: " + genderColor + "\" href=\"#USR-" + ch + "\">" + chop + "'s</a></b> " + message.mid ( 5, -1 ) + "</i>";
	else if ( message.startsWith ( "/warn" ) && isOp)
		rv = "<b><a style=\"color: " + genderColor + "\" href=\"#USR-" + ch + "\">" + chop + "</a></b>: <span id=\"warning\"><font color=\"white\">" + message.mid( 6, -1) + "</font></span>";
	else
		rv = "<b><a style=\"color: " + genderColor + "\" href=\"#USR-" + ch + "\">" + chop + "</a></b>: " + message;

	return rv;
}

QString FMessage::toString()
{
	QString rv = "";
    rv += (doesPing) ? "YES\n\t" : "NO\n\t";
	rv += "\n\tSender: ";
	rv += (sender) ? sender->name() : "None.";
    if (channel)
    {
        rv += "\n\tPanel: ";
        rv += channel->title();
    }
	rv += "\n\tMessage: ";
	rv += output;
	return rv;
}
