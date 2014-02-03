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

#ifndef flist_channel_H
#define flist_channel_H

#include <QString>
#include <QList>
#include <QVector>
#include <QVectorIterator>
#include "flist_messenger.h"
#include "flist_character.h"
#include "flist_parser.h"
#include "../libjson/libJSON.h"
#include "../libjson/Source/NumberToString.h"

#include <iostream>
#include <QDesktopWidget>
#include <QApplication>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QMessageBox>
#include <QComboBox>
#include <QPushButton>
#include <QGroupBox>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QListWidget>
#include <QScrollArea>
#include <QStatusBar>
#include <QTextEdit>
#include <QLineEdit>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QPlainTextEdit>
#include <QTextBrowser>

#include <time.h>

class QStringList;

class FChannel
{

public:
	enum typingStatus
	{
		TYPINGSTATUS_CLEAR,
		TYPINGSTATUS_TYPING,
		TYPINGSTATUS_PAUSED,
		TYPINGSTATUS_MAX
	};

	enum channelType
	{
		CHANTYPE_NORMAL,
		CHANTYPE_ADHOC,
		CHANTYPE_PM,
		CHANTYPE_CONSOLE,
		CHANTYPE_MAX
	};

	enum channelMode // ads, etc
	{
		CHANMODE_CHAT,
		CHANMODE_ADS,
		CHANMODE_BOTH,
		CHANMODE_MAX
	};

	FChannel ( QString name, channelType type );
	~FChannel() {}
	static void initClass();
	void setRecipient ( QString& name ){recipientName = name;}
	const QString& recipient() const{return recipientName;}
	void setName ( QString& name );
	const QString& name() const{return chanName;}
	void setInput ( QString& input ){this->input = input;}
	const QString& getInput(){return input;}
	void setType ( channelType type );
	const channelType type() const{return chanType;}
	void setDescription ( QString& desc );
	const QString& description() const{return chanDesc;}
	void setTitle ( QString& title );
	const QString& title() const{return chanTitle;}
	void updateButtonColor();
	void setOps ( QStringList& oplist );
	QList<QString> opList() const{return chanOps;}
	void setTyping ( typingStatus status );
	typingStatus getTyping() const{return typing;}
	void setTypingSelf ( typingStatus status ){typingSelf = status;}
	typingStatus getTypingSelf() const{return typingSelf;}
	void addChar ( FCharacter* character, bool sort_list = true );
	void remChar ( FCharacter* character );
	QList<FCharacter*> charList() const{return chanChars;}
	void sortChars();
	const bool isOp ( FCharacter* character );
	const bool isOwner ( FCharacter* character );
	void setActive ( bool o ){active = o;}
	const bool getActive() const{return active;}
	void setHighlighted ( bool o ){highlighted = o;}
	const bool getHighlighted(){return highlighted;}
	void setHasNewMessages ( bool o ){hasNewMessages = o;}
	const bool getHasNewMessages(){return hasNewMessages;}
	void setMode ( channelMode o ){mode = o;}
	const channelMode getMode(){return mode;}
	void setAlwaysPing ( bool b ) {alwaysPing = b;}
	const bool getAlwaysPing(){return alwaysPing;}
	JSONNode* toJSON();
	QString* toString();

	void addLine ( QString chanLine, bool log, bool parse = true );
	void emptyCharList();
	void logLine ( QString& chanLine );
	void printChannel ( QTextBrowser* textEdit );
	QPushButton*			pushButton;
	static BBCodeParser* 	bbparser;
private:
	bool					active;				// Will be no when the user leaves. This way, logs are kept.~
	QString					recipientName;		// For PM tabs.
	typingStatus			typing;				// For PM tabs: Whether the other is typing.
	typingStatus			typingSelf;			// For PM tabs: Whether the user is typing
	channelMode				mode;
	QString					input;
	bool					highlighted;
	bool					hasNewMessages;
	bool					alwaysPing;			// This channel pings on every message.
	QString					chanName;
	QString					chanTitle;
	QString					chanDesc;
	QList<FCharacter*>  	chanChars;
	QList<QString>      	chanOps;
	channelType         	chanType;
	QVector<QString>    	chanLines;
	quint64					chanLastActivity;
	time_t					creationTime;

	static QColor			colorInactive;
	static QColor			colorHighlighted;
	static QColor			colorNewMessages;
	static QColor			colorTyping;
	static QColor			colorPaused;

	static QString			cssStyle;
};

#endif //flist_character_H
