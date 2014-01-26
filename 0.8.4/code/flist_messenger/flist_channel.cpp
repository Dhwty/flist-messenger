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

#include "flist_channel.h"
#include <iostream>
#include <fstream>
#include <QDir>
#include <QtCore/QStringList>
#include <QtCore/QSettings>
#include <QDateTime>

BBCodeParser* FChannel::bbparser = 0;
QColor FChannel::colorInactive(255, 255, 255);
QColor FChannel::colorHighlighted(0, 255, 0);
QColor FChannel::colorNewMessages(204, 255, 153);
QColor FChannel::colorTyping(255, 153, 0);
QColor FChannel::colorPaused(128, 128, 255);
QString FChannel::cssStyle;

void FChannel::initClass()
{
	FChannel::bbparser = new BBCodeParser();

	QFile stylefile("default.css");
	stylefile.open(QFile::ReadOnly);
	cssStyle = QString(stylefile.readAll());

	QSettings colset("./colors.ini", QSettings::IniFormat);
	if(colset.status() != QSettings::NoError)
	{
		return;
	}
	QStringList colstr = colset.value("buttons/inactive").toStringList();
	colorInactive = QColor( colstr[0].toInt(), colstr[1].toInt(), colstr[2].toInt() );
	colstr = colset.value("buttons/highlighted").toStringList();
	colorHighlighted = QColor( colstr[0].toInt(), colstr[1].toInt(), colstr[2].toInt() );
	colstr = colset.value("buttons/newmessages").toStringList();
	colorNewMessages = QColor( colstr[0].toInt(), colstr[1].toInt(), colstr[2].toInt() );
	colstr = colset.value("buttons/typing").toStringList();
	colorTyping = QColor( colstr[0].toInt(), colstr[1].toInt(), colstr[2].toInt() );
	colstr = colset.value("buttons/paused").toStringList();
	colorPaused = QColor( colstr[0].toInt(), colstr[1].toInt(), colstr[2].toInt() );
}

FChannel::FChannel ( QString name, channelType type )
{
    mode = CHANMODE_BOTH;
	chanName = name;
	creationTime = time ( 0 );
	chanType = type;
	chanTitle = name;
	active = true;
	typing = TYPINGSTATUS_CLEAR;
	typingSelf = TYPINGSTATUS_CLEAR;
	input = "";
	if (type == CHANTYPE_NORMAL || type == CHANTYPE_ADHOC)
	{
		QString setting = chanName;
		setting += "/alwaysping";
		QSettings settings(flist_messenger::getSettingsPath(), QSettings::IniFormat);
		alwaysPing = settings.value(setting) == "true" ? true : false;
	} else {
		alwaysPing = false;
	}
}

void FChannel::setDescription ( QString& desc )
{
	chanDesc = desc;
}

void FChannel::setName ( QString& name )
{
	chanName = name;
}

void FChannel::setTitle ( QString& title )
{
	chanTitle = title;
}

void FChannel::setTyping ( typingStatus status )
{
	typing = status;
}

void FChannel::emptyCharList()
{
	while ( chanChars.size() )
		chanChars.pop_back();
}

void FChannel::addChar ( FCharacter* character, bool sort_list )
{
	if ( character == 0 )
	{
		std::cout << "Received null pointer character." << std::endl;
		return;
	}

	if ( chanChars.count ( character ) == 0 )
	{
		chanChars.append ( character );

		if ( sort_list )
		{
			sortChars();
		}
	}
	else
	{
		std::cout << "[SERVER BUG] Server gave us a person joining a channel who was already in the channel." << character->name().toStdString() << std::endl;
	}
}

void FChannel::remChar ( FCharacter* character )
{
	if ( chanChars.count ( character ) != 0 )
	{
		chanChars.removeAll ( character );
	}
}

void FChannel::sortChars()
{
	// Gnome Sort. :3
	int i = 0;

	while ( i < chanChars.count() )
	{
		if ( i == 0 )
			i++;
		else
		{
			FCharacter* a = chanChars[i - 1];
			FCharacter* b = chanChars[i];
			int aLevel = ( a->getFriend() ? 1 : 0 ) + ( isOp ( a ) ? 2 : 0 ) + ( isOwner ( a ) ? 4 : 0 ) + ( a->isChatOp() ? 8 : 0 );
			int bLevel = ( b->getFriend() ? 1 : 0 ) + ( isOp ( b ) ? 2 : 0 ) + ( isOwner ( b ) ? 4 : 0 ) + ( b->isChatOp() ? 8 : 0 );

			if ( aLevel < bLevel || ( ( aLevel == bLevel ) && a->name().toLower() > b->name().toLower() ) )
			{
				chanChars.swap ( i - 1, i );
				i--;
			}
			else
			{
				i++;
			}
		}
	}

	//std::cout << "Gnome sorted userlist~." << std::endl;
}

const bool FChannel::isOp ( FCharacter* character )
{
	if ( character == 0 )
	{
		std::cout << "Received null pointer character." << std::endl;
		return false;
	}

	for ( int i = 0; i < this->chanOps.count(); i++ )
	{
		if ( this->chanOps[i].toLower() == character->name().toLower() )
			return true;
	}

	return false;
}

const bool FChannel::isOwner ( FCharacter* character )
{
	if ( character == 0 )
	{
		std::cout << "Received null pointer character." << std::endl;
		return false;
	}

	if ( this->chanOps.count() > 0 && this->chanOps[0].toLower() == character->name().toLower() )
	{
		return true;
	}

	return false;
}

void FChannel::setType ( FChannel::channelType type )
{
	if ( type > 0 && type < CHANTYPE_MAX )
	{
		chanType = type;
	}
	else
	{
		std::cout << "Tried to set an invalid channel type." << ( int ) type << std::endl;
	}
}

void FChannel::setOps ( QStringList& oplist )
{
	chanOps.clear();

	for ( int i = 0;i < oplist.length();++i )
	{
		chanOps.append ( oplist[i] );
	}
}

void FChannel::addLine(QString chanLine, bool log, bool parse)
{
	// if (parse) chanLine = bbparser->parse(chanLine);
	char timebuf[64];
	time_t now = time(0);
	size_t len = strftime(&timebuf[0], 64, "[%T] ", localtime(&now));
	timebuf[len] = 0;
	chanLine = &timebuf[0] + chanLine;
	chanLines << chanLine;

	while ( chanLines.count() > 256 )
		chanLines.pop_front();
	if (log)
		logLine	( chanLine );
}

void FChannel::logLine ( QString &chanLine )
{

	QString logName, dirName;

    logName = chanName + "-" + QDateTime::currentDateTime().toString("yyyy-MM-dd") + ".html";

	// To ensure filename compatibility:
	logName.remove ( QRegExp ( "[^\\d\\w\\.\\_\\-]" ) );

	switch ( chanType )
	{

	case CHANTYPE_NORMAL:
		dirName += "public";
		break;

	case CHANTYPE_ADHOC:
		dirName += "private";
		break;

	case CHANTYPE_PM:
		dirName += "pm";
		break;

	case CHANTYPE_CONSOLE:
		dirName += "console";
		break;

	default:
		break;
	}

	dirName = "./logs/"  + dirName + "/";

	QDir::toNativeSeparators ( logName );

	if ( !QDir().exists ( dirName ) )
	{
		QDir().mkpath ( dirName );
	}

	logName = dirName + logName;

	QFile logfile(logName);
	if(!logfile.open(QFile::WriteOnly | QFile::Append))
	{
		QMessageBox::critical(NULL, "Failed to open log file.", QString("Could not open a log file. This could be caused by bad file permissions, or windows zone protection preventing the write of files.\nLog File: ") + logName);
		qApp->exit(1);
	}
    chanLine.append("<br />\n");
    chanLine.replace("#LNK-", "");
    chanLine.replace("#USR-", "https://www.f-list.net/c/");
	logfile.write(chanLine.toUtf8());
	logfile.close();
}
void FChannel::updateButtonColor()
{
	QString rv;

	if (pushButton->isChecked())
	{
		highlighted = false;
		hasNewMessages = false;
	}
	rv = "background-color: ";
	if ( highlighted )
		rv = rv + colorHighlighted.name() + ";";
	else if ( hasNewMessages )
		rv = rv + colorNewMessages.name() + ";";
	else
	{
		switch ( typing )
		{
		case TYPINGSTATUS_TYPING:
			rv = rv + colorTyping.name() + ";";
			break;
		case TYPINGSTATUS_PAUSED:
			rv = rv + colorPaused.name() + ";";
			break;
		default:
			rv = rv + colorInactive.name() + ";";
			break;
		}
	}
	pushButton->setStyleSheet( rv );
}

void FChannel::printChannel ( QTextBrowser* textEdit )
{
	if ( textEdit == 0 )
	{
		return;
	}

	bool doScroll = false;

	QString chanLine;
	QString html = cssStyle;

	if ( textEdit->verticalScrollBar()->value() == textEdit->verticalScrollBar()->maximum() )
	{
		doScroll = true;
	}
	QString lines = "";
	foreach ( chanLine, chanLines )
	{
		lines += "<br />" + chanLine;
	}
	html+=lines.mid(6);

	//html += "</font></qt>";
	textEdit->setHtml ( html );

	if ( doScroll )
	{
		textEdit->verticalScrollBar()->setSliderPosition ( textEdit->verticalScrollBar()->maximum() );
	}
}

JSONNode* FChannel::toJSON()
{
	JSONNode* rv = new JSONNode(JSON_ARRAY);
	JSONNode node;
	JSONNode typeNode("type", "chat");
	JSONNode byNode("by", "");
	JSONNode htmlNode;
	QString chanLine;
	foreach(chanLine, chanLines)
	{
		node = JSONNode(JSON_NODE);
		node.push_back(typeNode);
		node.push_back(byNode);
		htmlNode = JSONNode("html", chanLine.toStdString());
		node.push_back(htmlNode);
		rv->push_back(node);
	}
	return rv;
}

QString* FChannel::toString()
{
	QString* rv = new QString("Channel: ");
	*rv += chanTitle;
	*rv += "\nName: ";
	*rv += chanName;
	*rv += "\nType: ";
	if (chanType == CHANTYPE_NORMAL) *rv += "Normal";
	else if (chanType == CHANTYPE_PM) { *rv += "PM to: "; *rv += recipientName; }
	else if (chanType == CHANTYPE_ADHOC) *rv += "Adhoc";
	else if (chanType == CHANTYPE_CONSOLE) *rv += "CONSOLE";
	else *rv += "INVALID TYPE";
	*rv += "\nLines: ";
	*rv += QString::number(chanLines.count());
	*rv += "\nActive: ";
	*rv += active ? "Yes" : "No";
	*rv += "\nMode: ";
	if (mode == CHANMODE_CHAT) *rv += "Chat";
	else if (mode == CHANMODE_ADS) *rv += "Ads";
	else if (mode == CHANMODE_BOTH) *rv += "Both";
	else *rv += "INVALID MODE";
	return rv;
}
