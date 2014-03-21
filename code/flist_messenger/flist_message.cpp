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
bool						FMessage::doSounds = true;
BBCodeParser*				FMessage::bbparser = 0;
FSound*						FMessage::sound = 0;
QStringList					FMessage::pingList;
FChannelPanel*					FMessage::console = 0;
QHash<QString, FChannelPanel*>	FMessage::channelList;
FChannelPanel*					FMessage::currentPanel = 0;
QString						FMessage::selfName = "";
QTextBrowser*				FMessage::textField = 0;
flist_messenger*			FMessage::window = 0;

void FMessage::initClass(bool ping, bool alwaysPing, flist_messenger *window)
{
	FMessage::window = window;
	bbparser = new BBCodeParser;
	sound = new FSound;
	doPing = ping;
	doAlwaysPing = alwaysPing;
}

FMessage::FMessage(MessageType msgType, FChannelPanel* channel, FCharacter* character, QString& msg, FChannelPanel* currentPanel)
{
	type = msgType;
	sender = character;
	message = msg;
	this->channel = channel;
	this->currentPanel = currentPanel;

	parse();
}
FMessage::FMessage(MessageType msgType, FChannelPanel *channel, FCharacter *character, QString &msg, FChannelPanel *currentPanel, QHash<QString, FChannelPanel *> &channelList)
{
	FMessage::channelList = channelList;
	type = msgType;
	sender = character;
	this->channel = channel;
	message = msg;
	this->currentPanel = currentPanel;

	parse();
}
FMessage::FMessage(SystemMessageType sysType, FChannelPanel* channel, FCharacter* character, QString& msg, FChannelPanel* currentPanel)
{
	systemType = sysType;
	type = MESSAGETYPE_SYSTEM;
	sender = character;
	this->channel = channel;
	message = msg;
	this->currentPanel = currentPanel;

	parse();
}

FMessage::~FMessage(){}

void FMessage::parse()
{
	doesPing = checkPing();
	QString timestr = QTime::currentTime().toString("h:mm AP");

	//prepare message
	switch(type)
	{
	case MESSAGETYPE_PRIVMESSAGE:
		channel->setHighlighted(true);
	case MESSAGETYPE_CHANMESSAGE:
		channel->setHasNewMessages(true);
		output = "<small><i>[" + timestr + "]</i></small> " + charNameToHtml();
		output = bbparser->parse(output);
		break;
	case MESSAGETYPE_ROLEPLAYAD:
		output = "<small><i>[" + timestr + "]</i></small> <font color=\"green\"><b>Roleplay ad by</font> " + charNameToHtml();
		output = bbparser->parse(output);
		break;
	default:
		output = "<small><i>[" + timestr + "]</i></small> " + message;
		output = bbparser->parse(output);
		break;
	}

	//find out what channels it should be sent to
	FChannelPanel* outputChannel = 0;
	switch(type)
	{
	case MESSAGETYPE_BROADCAST:
	case MESSAGETYPE_REPORT:
		foreach(FChannelPanel* c, channelList)
			if (c->getActive())
				c->addLine(output, true);
		textField->append(output);
		break;
	case MESSAGETYPE_SYSTEM:
		if (systemType == SYSTYPE_DICE || systemType == SYSTYPE_JOIN)
		{
			if (channel)
			{
				outputChannel=channel;
				outputChannel->addLine(output, true);
				if (outputChannel == currentPanel) {
					textField->append(output);
				}
			}
		}
		else
		{
			outputChannel = ((channel) ? channel : currentPanel);
			outputChannel->addLine(output, true);
			if (console != outputChannel)
				console->addLine(output, true);
			if (outputChannel == currentPanel || console == currentPanel)
				textField->append(output);
		}
		break;
	default:
		channel->addLine(output, true);
		if (channel == currentPanel)
			textField->append(output);
	}

	// play sounds
	doPings();
	if (channel)
		channel->updateButtonColor();
}

bool FMessage::checkPing()
{
	if (sender && sender->name() == selfName) // Don't ping if user is the sender
	{
		pingReason = "User is sender.";
		return false;
	}
	if (channel == currentPanel && doAlwaysPing == false) // Else, don't ping if currentpanel
	{
		pingReason = "Panel is currentpanel.";
		return false;
	}
	if (type == MESSAGETYPE_PRIVMESSAGE) // Else, ping if PM
	{
		pingReason = "PM tab.";
		return true;
	}
	if (channel && channel->getAlwaysPing())
	{
		pingReason = "Channel always pings.";
		return true;
	}
	if (doPing == false) // Else, don't ping if settings say don't
	{
		pingReason = "Settings are disabled.";
		return false;
	}
	if (type == MESSAGETYPE_SYSTEM) // Else, check system message for username mention
	{
		pingReason = "System message.";
		return checkSysPing();
	}
	if (message.contains(selfName, Qt::CaseInsensitive)) // Else, ping if name is mentioned
	{
		pingReason = "Name is mentioned.";
		return true;
	}
	else foreach (QString s, pingList) // Else, check pinglist
		if (message.contains(s, Qt::CaseInsensitive)) // Ping if item found
		{
			pingReason = "Pinglist found: " + s;
			return true;
		}

	/* else */ return false;
}

bool FMessage::checkSysPing()
{
	if (systemType == SYSTYPE_JOIN || systemType == SYSTYPE_ONLINE || systemType == SYSTYPE_FEEDBACK)
		return false;													// Don't ping on leave/join or online/offline messages
	if (message.contains(selfName, Qt::CaseInsensitive)) return true;	// Else, ping if name is mentioned (Even if settings say don't. This is for bans, kicks, etc. Quite important~)

	/* else */ return false;
}
void FMessage::checkButtonColor()
{
	if (channel == currentPanel
			|| (type != MESSAGETYPE_PRIVMESSAGE
			&& type != MESSAGETYPE_CHANMESSAGE)
	   )
		return;
	if (doesPing) channel->setHighlighted(true);
	channel->setHasNewMessages(true);
	channel->updateButtonColor();
}

void FMessage::doPings()
{
	if (!doPing) return;
	bool flash = false;
	if (type == MESSAGETYPE_BROADCAST)
	{
		flash = true;
		if (doSounds) sound->play(FSound::SOUND_ATTENTION);
	}
	else if (type == MESSAGETYPE_REPORT)
	{
		flash = true;
		if (doSounds) sound->play(FSound::SOUND_MODALERT);
	}
	else if (doesPing)
	{
		flash = true;
		if (doSounds) sound->play(FSound::SOUND_ATTENTION);
	}

	if (flash)
	{
		if (channel && channel != currentPanel)
		{
			channel->setHighlighted(true);
			channel->updateButtonColor();
		}
		QString reason = toString();
		window->flashApp(reason);
	}
}

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
	rv += pingReason;
	rv += "\n\tSender: ";
	rv += (sender) ? sender->name() : "None.";
	rv += "\n\tPanel: ";
	rv += (channel && channel != currentPanel) ? channel->title() : "Current panel.";
	rv += "\n\tMessage: ";
	rv += output;
	return rv;
}
