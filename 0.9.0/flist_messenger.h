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

#ifndef flist_messenger_H
#define flist_messenger_H

#define VERSION "F-List Messenger [Beta] 0.9.0"
#define VERSIONNUM "0.9.0"
#define CLIENTID "F-List Desktop Client"

#include <QtWidgets/QMainWindow>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QTextCodec>
#include <QUrlQuery>
#include <QUrl>
#include <QString>
#include <QSignalMapper>
#include <QDesktopServices>

#include <iostream>
#include <QSettings>

#include "../libjson/libJSON.h"
#include "../libjson/Source/NumberToString.h"

#include "flist_character.h"
#include "flist_channel.h"
#include "flist_sound.h"
#include "./gui/flist_avatar.h"
#include "./message/flist_advertmessage.h"
#include "./message/flist_broadcastmessage.h"
#include "./message/flist_chatmessage.h"
#include "./message/flist_reportmessage.h"
#include "./message/flist_systemmessage.h"
#include "gui/flist_gui.h"
#include "network/flist_network.h"
#include "flist_settings.h"

#ifndef CHANNELLISTITEM_CLASS
#define CHANNELLISTITEM_CLASS

class ChannelListItem : public QListWidgetItem
{

public:
	ChannelListItem ( QString& name, int chars );
	ChannelListItem ( QString& name, QString& title, int chars );
    void setName ( QString& n ) {name = n;}
    const QString& getName(){return name;}
	void setTitle ( QString& t ){title = t;}
    const QString& getTitle(){return title;}
    void setChars ( int i ){chars = i;}
    const int getChars(){return chars;}
	bool operator > ( ChannelListItem* cli );

private:
	QString name;
	QString title;
	int chars;
};

#endif

#ifndef USERLISTROW_CLASS
#define USERLISTROW_CLASS

class UserListRow : public QObject
{
public:
	UserListRow ( QString name, QWidget *parent = 0 );
	~UserListRow() {}

	QLabel* icon;
	QLabel* userName;
	QHBoxLayout* hbox;
};

#endif

class FChannel;
class FGui;
class FNetwork;

class flist_messenger : public QObject
{
    Q_OBJECT

public:
	static void init();
	static const QString getSettingsPath() { return settingsPath; }
	flist_messenger(bool d);
	~flist_messenger();

    void setupSocket(QString& charName);

    // GETTERS AND SETTERS
    bool getDisconnected() {return disconnected;}
    FCharacter* getCharacter(QString name) { return characterList[name]; }
    FChannel* getChannel(QString name) { return channelList[name]; }
    FChannel* getCurrentPanel() {return currentPanel;}
    bool isIgnored(QString name) { return selfIgnoreList.contains(name); }
    FCharacter* me() {return characterList[charName];}
    const QString* getStatus() { return &selfStatus; }
    const QString* getStatusMessage() { return &selfStatusMessage; }
    BBCodeParser* getParser() { return &bbparser; }
    FSettings* getSettings() { return settings; }
    void submitReport(QString& problem, QString& who);

public slots:
	void anchorClicked ( QUrl link );	// Handles anchor clicks in the main text field.
    void prepareLogin ( QString& username, QString& password );
    void friendsDialogRequested();
    void channelsDialogRequested();
    void characterDialogRequested(QString& name);

signals:
    void tabSwitched(FChannel* from, FChannel* to);

private slots:
    void setupConsole();								// Makes the console channel.
    void handleLogin();
    void setupLoginBox();			// The login box is used for character selection
	void loginClicked();			// Called by the login button during character selection
    void setupRealUI();				// Creation of the chat environment GUI
	void connectedToSocket();
    void readReadyOnSocket();
	void channelButtonClicked();	// Called when channel button is clicked. This should switch panels, and do other necessary things.
	void switchTab ( QString& tabname );
    void inputChanged();
	void handleReportFinished();
	void reportTicketFinished();
	void versionInfoReceived();
	void btnSendAdvClicked();
    void btnSendChatClicked();
public:
    void parseInput(QString& input);
	static const int BUFFERPUB  = 4096; // Buffer limit in public
	static const int BUFFERPRIV = 50000; // Buffer limit in private
    static std::string WSConnect;
    void parseCommand ( std::string& input );			// Parses messages received from the server.
    void socketError ( QAbstractSocket::SocketError socketError );

    // The following functions are meant to be called by the network interface, when receiving certain commands from the server.
    void setOpList(QList<QString>& opList);
    void addOp(QString& op);
    void removeOp(QString& op);
    void receiveBroadcast(QString& msg);
    void printDebugInfo(std::string s);
    void openPMTab ( QString& character );
    void deleteIgnore(QString& name);
    void addIgnore(QString& name);
    void _channelBan(QString& name);
    void _channelKick(QString& name);
    void _chatBan(QString& name);
    void _chatKick(QString& name);
    void _addOp(QString& name);
    void _deleteOp(QString& name);
    void _addChanOp(QString& name);
    void _deleteChanOp(QString& name);
    void _timeout(QString& name, int time, QString& reason);
    void _makeRoom(QString& name);
    void _setDescription(QString& channel, QString& description);
    void changeStatus ( QString& status, QString& statusmsg );
    void leaveChannel ( QString& channel, bool toServer = true );		// Leaves the channel
    void joinChannel(QString& channel, bool toServer = true);

private:
    std::string escape(std::string& s);
    FGui* gui;
    FNetwork* network;
    FSettings* settings;
	bool debugging;
	bool versionIsOkay;
    bool notificationsAreaMessageShown;
    void joinedChannel(QString& name, QString& title);               // Creates/activates the channel (after the server says we joined it.)
	void refreshUserlist();								// Refreshes the GUI's userlist, based on what the current panel is
    void receivePM ( QString& message, QString& character );
	void typingPaused ( FChannel* channel );
	void typingContinued ( FChannel* channel );
	void typingCleared ( FChannel* channel );
    void _submitReport(QString& );
    QString* usernameToHtml ( QString& name );
	QNetworkAccessManager qnam;
	QNetworkReply* lreply;
	QUrl lurl;

    void postAdvertMessage(FChannel* channel, FCharacter* character, QString& msg);
    void postBroadcastMessage(QString& msg);
    void postChatMessage(FChannel* channel, FCharacter* character, QString& msg);
    void postReportMessage(FCharacter* character, QString& msg);
    void postSystemMessage(FSystemMessage::SystemMessageType sysType, FChannel* channel, QString& msg);

	std::string networkBuffer;
	unsigned int loginStep;
	FChannel* console;	// We could just put this into the channel list, but the console needs to be accessed quite often. So here we go...
	FChannel* currentPanel;
	FSound soundPlayer;
	BBCodeParser bbparser;
	QString sessionID;
	QList<QString> selfCharacterList;
	QList<QString> selfFriendsList;
	QList<QString> selfIgnoreList;
	QStringList selfPingList;
	QStringList defaultChannels;
	QString username;
    QString loginTicket;
    QString defaultCharacter;
	QString password;
	QString sessionHash;
	QString accountID;
	QString charName;
	QString selfStatus;
	QString selfStatusMessage;
	QString defaultChar;
	bool disconnected;
	static QString settingsPath;
	bool doingWS;
	QList<QString> opList;
	QHash<QString, FCharacter*> characterList;
	QHash<QString, FChannel*> channelList;
    QHash<QString, QString> serverVariables;

    QString re_problem;
    QString re_who;

    bool se_leaveJoin;
    bool se_onlineOffline;
    bool se_chatLogs;
    bool se_sounds;
    bool se_alwaysPing;
    bool se_ping;
    bool se_helpdesk;
	/* The following GUIs still need to be made:
	QDialog* kinkSearchDialog;
	 */

	/* Stuff to do:
		Kink search:
		[14:27 PM]>>FKS {"kinks":["77","239"],"genders":["Female","Transgender"],"orientations":["Straight","Gay"],"languages":["English"],"furryprefs":["Furs and / or humans"]}

	 */
};
#endif // flist_messenger_H
