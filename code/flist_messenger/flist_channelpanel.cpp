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
#include <QtCore/QStringList>
#include <QtCore/QSettings>
#include <QDateTime>

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

FChannelPanel::FChannelPanel (QString panelname, QString channelname, channelType type)
{
	mode = CHANMODE_BOTH;
	this->panelname = panelname;
        chanName = channelname;
        creationTime = time ( 0 );
        chanType = type;
        chanTitle = channelname;
        active = true;
        typing = TYPING_STATUS_CLEAR;
        typingSelf = TYPING_STATUS_CLEAR;
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

bool FChannelPanel::isOp ( FCharacter* character )
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

bool FChannelPanel::isOwner ( FCharacter* character )
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

void FChannelPanel::setType ( FChannelPanel::channelType type )
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

void FChannelPanel::setOps ( QStringList& oplist )
{
        chanOps.clear();

        for ( int i = 0;i < oplist.length();++i )
        {
                chanOps.append ( oplist[i] );
        }
}
void FChannelPanel::addOp(QString &charactername)
{
	if(chanOps.contains(charactername)) {
		return;
	}
	chanOps.append(charactername);
}
void FChannelPanel::removeOp(QString &charactername)
{
	chanOps.removeAll(charactername);
}

void FChannelPanel::addLine(QString chanLine, bool log, bool parse)
{
        (void) parse; //todo: check what this was intended for
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
                logLine        ( chanLine );
}

void FChannelPanel::logLine ( QString &chanLine )
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
    chanLine.replace("#LNK-", "");
    chanLine.replace("#USR-", "https://www.f-list.net/c/");
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
