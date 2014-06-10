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

#include "flist_channelpanel.h"
#include <iostream>
#include <fstream>
#include <QDir>
#include <QStringList>
#include <QSettings>
#include <QDateTime>

#include "flist_global.h"
#include "flist_session.h"
#include "flist_iuserinterface.h"
#include "flist_settings.h"

BBCodeParser* FChannelPanel::bbparser = 0;
QColor FChannelPanel::colorInactive(255, 255, 255);
QColor FChannelPanel::colorHighlighted(0, 255, 0);
QColor FChannelPanel::colorNewMessages(204, 255, 153);
QColor FChannelPanel::colorTyping(255, 153, 0);
QColor FChannelPanel::colorPaused(128, 128, 255);
QString FChannelPanel::cssStyle;

void FChannelPanel::initClass()
{
        FChannelPanel::bbparser = new BBCodeParser();

        QFile stylefile("default.css");
        stylefile.open(QFile::ReadOnly);
        cssStyle = QString(stylefile.readAll());

        QSettings colset("./colors.ini", QSettings::IniFormat);
        if(colset.status() != QSettings::NoError)
        {
                return;
        }
        QStringList colstr;
        colstr = colset.value("buttons/inactive").toStringList();
	std::cout << colset.value("buttons/inactive").toString().toUtf8().constData() << std::endl;
        if(colstr.size() >= 2) {
            colorInactive = QColor( colstr[0].toInt(), colstr[1].toInt(), colstr[2].toInt() );
        }
        colstr = colset.value("buttons/highlighted").toStringList();
        if(colstr.size() >= 2) {
            colorHighlighted = QColor( colstr[0].toInt(), colstr[1].toInt(), colstr[2].toInt() );
        }
        colstr = colset.value("buttons/newmessages").toStringList();
        if(colstr.size() >= 2) {
            colorNewMessages = QColor( colstr[0].toInt(), colstr[1].toInt(), colstr[2].toInt() );
        }
        colstr = colset.value("buttons/typing").toStringList();
        if(colstr.size() >= 2) {
            colorTyping = QColor( colstr[0].toInt(), colstr[1].toInt(), colstr[2].toInt() );
        }
        colstr = colset.value("buttons/paused").toStringList();
        if(colstr.size() >= 2) {
            colorPaused = QColor( colstr[0].toInt(), colstr[1].toInt(), colstr[2].toInt() );
        }
}

FChannelPanel::FChannelPanel (iUserInterface *ui, QString sessionid, QString panelname, QString channelname, FChannel::ChannelType type) :
	ui(ui),
	sessionid(sessionid),
	panelname(panelname)
{
	mode = CHANNEL_MODE_BOTH;
        chanName = channelname;
        creationTime = time ( 0 );
        chanType = type;
        chanTitle = channelname;
        active = true;
        typing = TYPING_STATUS_CLEAR;
        typingSelf = TYPING_STATUS_CLEAR;
        input = "";
	loadSettings();
}

void FChannelPanel::setDescription ( QString& desc )
{
        chanDesc = desc;
}

void FChannelPanel::setName ( QString& name )
{
        chanName = name;
}

void FChannelPanel::setTitle ( QString& title )
{
        chanTitle = title;
}

void FChannelPanel::setTyping ( TypingStatus status )
{
        typing = status;
}

void FChannelPanel::emptyCharList()
{
        while ( chanChars.size() )
                chanChars.pop_back();
}

void FChannelPanel::addChar ( FCharacter* character, bool sort_list )
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

void FChannelPanel::remChar ( FCharacter* character )
{
        if ( chanChars.count ( character ) != 0 )
        {
                chanChars.removeAll ( character );
        }
}

void FChannelPanel::sortChars()
{
        // Gnome Sort. :3
        int i;

	QList<int> chancharlevels;

	chancharlevels.reserve(chanChars.count());
	for(i = 0; i < chanChars.count(); i++) {
		FCharacter* ch = chanChars[i];
		chancharlevels.append((ch->getFriend() ? 1 : 0) + (isOp(ch) ? 2 : 0) + (isOwner(ch) ? 4 : 0) + (ch->isChatOp() ? 8 : 0));
	}
	i = 0;
        while ( i < chanChars.count() )
        {
                if ( i == 0 )
                        i++;
                else
                {
                        FCharacter* a = chanChars[i - 1];
                        FCharacter* b = chanChars[i];
			int aLevel = chancharlevels[i - 1];
			int bLevel = chancharlevels[i];

                        if ( aLevel < bLevel || ( ( aLevel == bLevel ) && a->name().toLower() > b->name().toLower() ) )
                        {
                                chanChars.swap ( i - 1, i );
                                chancharlevels.swap ( i - 1, i );
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

bool FChannelPanel::isOp ( FCharacter* character )
{
        if ( character == 0 )
        {
                std::cout << "Received null pointer character." << std::endl;
                return false;
        }

	return chanOps.contains(character->name().toLower());
}

bool FChannelPanel::isOwner ( FCharacter* character )
{
        if ( character == 0 )
        {
                std::cout << "Received null pointer character." << std::endl;
                return false;
        }

	return character->name().toLower() == chanowner.toLower();
}

void FChannelPanel::setType ( FChannel::ChannelType type )
{
        if ( type > 0 && type < FChannel::CHANTYPE_MAX )
        {
                chanType = type;
        }
        else
        {
                std::cout << "Tried to set an invalid channel type." << ( int ) type << std::endl;
        }
}

void FChannelPanel::setOps ( QStringList& oplist )
{
        chanOps.clear();

	chanowner = (oplist.length() > 0) ? oplist[0] : "";

        for ( int i = 0;i < oplist.length();++i )
        {
		chanOps[oplist[i].toLower()] = oplist[i];
        }
}
void FChannelPanel::addOp(QString &charactername)
{
	chanOps[charactername.toLower()] = charactername;
}
void FChannelPanel::removeOp(QString &charactername)
{
	chanOps.remove(charactername.toLower());
}

void FChannelPanel::addLine(QString chanLine, bool log)
{
	chanLines.append(chanLine);
	//todo: make this configurable
	while(chanLines.count() > 256) {
		chanLines.pop_front();
	}
	if(log) {
		logLine(chanLine);
	}
}

void FChannelPanel::clearLines()
{
	chanLines.clear();
}

void FChannelPanel::logLine ( QString &chanLine )
{
	QString logName, dirName;

	FSession *session = ui->getSession(sessionid);
	if(session) {
		logName = escapeFileName(session->character) + "~";
	}

        switch ( chanType )
        {

        case FChannel::CHANTYPE_NORMAL:
                dirName += "public";
		logName += escapeFileName(chanName);
                break;
        case FChannel::CHANTYPE_ADHOC:
                dirName += "private";
		logName += QString("%1~%2").arg(escapeFileName(chanName), escapeFileName(chanTitle));
                break;

        case FChannel::CHANTYPE_PM:
                dirName += "pm";
		logName += escapeFileName(chanName);
                break;

        case FChannel::CHANTYPE_CONSOLE:
		dirName += "console";
		logName += escapeFileName(chanName);
		break;
	default:
		logName += escapeFileName(chanName);
		break;
	}
	logName += "~" + QDateTime::currentDateTime().toString("yyyy-MM-dd") + ".html";
        // To ensure filename compatibility:

        dirName = "./logs/" + dirName + "/";

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
        logfile.write(chanLine.toUtf8());
        logfile.close();
}
void FChannelPanel::updateButtonColor()
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
                case TYPING_STATUS_TYPING:
                        rv = rv + colorTyping.name() + ";";
                        break;
                case TYPING_STATUS_PAUSED:
                        rv = rv + colorPaused.name() + ";";
                        break;
                default:
                        rv = rv + colorInactive.name() + ";";
                        break;
                }
        }
        pushButton->setStyleSheet( rv );
}

void FChannelPanel::printChannel ( QTextBrowser* textEdit )
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
	int size = chanLines.size() * 6;
        foreach(chanLine, chanLines) {
		size += chanLine.length();
	}
	lines.reserve(size);

	foreach ( chanLine, chanLines )  {
		lines += chanLine;
		lines += "<br />";
	}
	html += lines.left(lines.length() - 6);

        //html += "</font></qt>";
        textEdit->setHtml ( html );

        if ( doScroll )
        {
                textEdit->verticalScrollBar()->setSliderPosition ( textEdit->verticalScrollBar()->maximum() );
        }
}

JSONNode* FChannelPanel::toJSON()
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

QString* FChannelPanel::toString()
{
        QString* rv = new QString("Channel: ");
        *rv += chanTitle;
        *rv += "\nName: ";
        *rv += chanName;
        *rv += "\nType: ";
        if (chanType == FChannel::CHANTYPE_NORMAL) *rv += "Normal";
        else if (chanType == FChannel::CHANTYPE_PM) { *rv += "PM to: "; *rv += recipientName; }
        else if (chanType == FChannel::CHANTYPE_ADHOC) *rv += "Adhoc";
        else if (chanType == FChannel::CHANTYPE_CONSOLE) *rv += "CONSOLE";
        else *rv += "INVALID TYPE";
        *rv += "\nLines: ";
        *rv += QString::number(chanLines.count());
        *rv += "\nActive: ";
        *rv += active ? "Yes" : "No";
        *rv += "\nMode: ";
        if (mode == CHANNEL_MODE_CHAT) *rv += "Chat";
        else if (mode == CHANNEL_MODE_ADS) *rv += "Ads";
        else if (mode == CHANNEL_MODE_BOTH) *rv += "Both";
        else *rv += "INVALID MODE";
        return rv;
}

/**
 */
void FChannelPanel::loadSettings()
{
	QString keyprefix;
	if(type() == FChannel::CHANTYPE_PM) {
		keyprefix = QString("Character/%1/").arg(escapeFileName(recipient()));
	} else {
		keyprefix = QString("Channel/%1/").arg(escapeFileName(getChannelName()));
	}
	//load and parse keywords
	keywordlist = settings->qsettings->value(keyprefix + "keywords").toString().split(",", QString::SkipEmptyParts);
	for(int i = 0; i < keywordlist.size(); i++) {
		keywordlist[i] = keywordlist[i].trimmed();
		if(keywordlist[i].isEmpty()) {
			keywordlist.removeAt(i);
			i--;
		}
	}
	
}
