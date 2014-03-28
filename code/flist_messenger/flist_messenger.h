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

#include "flist_common.h"

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QTcpSocket>
#include <QSslSocket>
#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif
#include <QUrl>
#include <QString>
#include <QSignalMapper>
#include <QDesktopServices>

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
#include <QTextBrowser>
#include <QLineEdit>
#include <QScrollBar>
#include <QSpacerItem>
#include <QVBoxLayout>
#include <QPlainTextEdit>
#include <QListWidgetItem>
#include <QDialog>
#include <QTabWidget>
#include <QCheckBox>
#include <QIcon>
#include <QSystemTrayIcon>
#include <QTextBrowser>
#include <QSettings>

#include <QUrl>
#include <QtNetwork>
#include <QTcpSocket>

#include "../libjson/libJSON.h"
#include "../libjson/Source/NumberToString.h"

#include "flist_character.h"
#include "flist_channelpanel.h"
#include "flist_sound.h"
#include "flist_avatar.h"
#include "flist_message.h"
//#include "flist_account.h"
#include "flist_channeltab.h"
#include "flist_iuserinterface.h"

class FAccount;
class FServer;

#ifndef USERETURN_CLASS
#define USERETURN_CLASS


class UseReturn : public QObject
{
	Q_OBJECT

public:
	UseReturn ( QObject* parent )
	{
		setParent ( parent );
	}

protected:
	bool eventFilter ( QObject *obj, QEvent *event );
};

class ReturnLogin : public QObject
{
	Q_OBJECT
public:
	ReturnLogin ( QObject* parent) {setParent(parent);}
protected:
	bool eventFilter(QObject *obj, QEvent *event);
};

#endif

#ifndef CHANNELLISTITEM_CLASS
#define CHANNELLISTITEM_CLASS

class ChannelListItem : public QListWidgetItem
{

public:
	ChannelListItem ( QString& name, int chars );
	ChannelListItem ( QString& name, QString& title, int chars );

	void setName ( QString& n )
	{
		name = n;
	}

	const QString& getName()
	{
		return name;
	}

	void setTitle ( QString& t )
	{
		title = t;
	}

	const QString& getTitle()
	{
		return title;
	}

	void setChars ( int i )
	{
		chars = i;
	}

	int getChars()
	{
		return chars;
	}

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

// This is a complete mess, login should be pulled out into another class somehow, and decoupled from the UI.

class FChannelPanel;

class flist_messenger : public QMainWindow, iUserInterface
{
	Q_OBJECT

public:
	static void init();
	static const QString getSettingsPath() { return settingsPath; }
	flist_messenger(bool d);
	~flist_messenger();
public:
	virtual void setChatOperator(FSession *session, QString characteroperator, bool opstatus);
	virtual void addChannel(FSession *session, QString name, QString title);
	virtual void removeChannel(FSession *session, QString name);
	virtual void addChannelCharacter(FSession *session, QString channelname, QString charactername, bool notify);
	virtual void removeChannelCharacter(FSession *session, QString channelname, QString charactername);
	virtual void setChannelOperator(FSession *session, QString channelname, QString charactername, bool opstatus);
	virtual void joinChannel(FSession *session, QString channelname);
	virtual void leaveChannel(FSession *session, QString channelname);
	virtual void setChannelDescription(FSession *session, QString channelname, QString description);
	virtual void notifyCharacterOnline(FSession *session, QString charactername, bool online);
	virtual void notifyIgnoreUpdate(FSession *session);
	virtual void setIgnoreCharacter(FSession *session, QString charactername, bool ignore);
	virtual void messageMany(FSession *session, QList<QString> &channels, QList<QString> &characters, bool system, QString message, MessageType messagetype);
	virtual void messageAll(FSession *session, QString message, MessageType messagetype);
	virtual void messageChannel(FSession *session, QString channelname, QString message, MessageType messagetype, bool console = false, bool notify = false);
	virtual void messageCharacter(FSession *session, QString charactername, QString message, MessageType messagetype);
	virtual void messageSystem(FSession *session, QString message, MessageType messagetype);

	void messageMany(QList<QString> &panelnames, QString message, MessageType messagetype);

public:
	QPushButton* pushButton;
	FChannelTab* channelTab;
	QLabel* label;
	QComboBox* comboBox;
	QGroupBox* groupBox;
	//=================================
	QMenuBar *menubar;
	QAction *actionDisconnect;
	QAction *actionQuit;
	QAction *actionHelp;
	QAction *actionAbout;
	QAction *actionColours;
	QAction *actionCommands;
	//=================================
	QWidget *horizontalLayoutWidget;
	QWidget *verticalLayoutWidget;
	QHBoxLayout *horizontalLayout;
	QVBoxLayout *verticalLayout;
	QGridLayout *gridLayout;
	QScrollArea *activePanels;
	QVBoxLayout *activePanelsContents;
	QVBoxLayout *centralStuff;
	QHBoxLayout *centralButtons;
	QWidget *centralButtonsWidget;
	QPushButton *btnSettings;
	QPushButton *btnChannels;
	QPushButton *btnMakeRoom;
	QPushButton *btnSetStatus;
	QPushButton *btnFriends;
	QPushButton *btnReport;
	QPushButton *btnSendChat;
	QPushButton *btnSendAdv;
	QPushButton *btnConnect;
	QLabel *lblCheckingVersion;
	QLabel *lblChannelName;
	QTextBrowser *textEdit;
	QLineEdit *lineEdit;
	QPlainTextEdit *plainTextEdit;
	QListWidget *listWidget;
	QMenu *menuHelp;
	QMenu *menuFile;
	UseReturn* returnFilter;
	FAvatar avatarFetcher;
	QSpacerItem* activePanelsSpacer;

public slots:
	void anchorClicked ( QUrl link );	// Handles anchor clicks in the main text field.
	void insertLineBreak();				// Called when shift+enter is pressed while typing.
	void connectClicked();			// Called by the connect button during account login
	void closeEvent(QCloseEvent *event);
	void iconActivated(QSystemTrayIcon::ActivationReason reason);
	void enterPressed();

private slots:
	//void parseCommand ( std::string input );			// Parses messages received from the server.
	void processCommand(std::string input, std::string cmd, JSONNode &nodes);
	void recvMessage(QString type, QString session, QString chan, QString sender, QString message);
	void prepareLogin ( QString username, QString password );
	void handleLogin();
	void loginError(FAccount *account, QString errortitle, QString errorstring);
	void loginComplete(FAccount *account);
	void handleSslErrors( QList<QSslError> sslerrors );
	void setupConnectBox();			// The connect box is used for account login
	void setupLoginBox();			// The login box is used for character selection
	void clearLoginBox();			// Destroys login box
	void clearConnectBox();			// Destroys connect box
	void loginClicked();			// Called by the login button during character selection
	void setupRealUI();				// Creation of the chat environment GUI
	void setupSettingsDialog();
	void setupTimeoutDialog();
	void timeoutDialogRequested();
	void setupChannelsUI();
	void setupMakeRoomUI();
	void setupSetStatusUI();
	void setupCharacterInfoUI();
	void setupFriendsDialog();
	void setupAddIgnoreDialog();
	void setupReportDialog();
	void setupHelpDialog();
	bool setupChannelSettingsDialog();
	void settingsDialogRequested();
	void friendsDialogRequested();
	void refreshFriendLists();
	void addIgnoreDialogRequested();
	void channelsDialogRequested();
	void makeRoomDialogRequested();
	void setStatusDialogRequested();
	void characterInfoDialogRequested();
	void reportDialogRequested();
	void helpDialogRequested();
	void channelSettingsDialogRequested();
	void destroyMenu();
	void destroyChanMenu();
	void socketError ( QAbstractSocket::SocketError socketError );
        void socketSslError ( QList<QSslError> sslerrors );
	void quitApp();
	void aboutApp();
	void channelButtonMenuRequested();
	void channelButtonClicked();	// Called when channel button is clicked. This should switch panels, and do other necessary things.
	void switchTab ( QString& tabname );
	void inputChanged();
	void userListContextMenuRequested ( const QPoint& point );
	void submitReport();
	void handleReportFinished();
	void reportTicketFinished();
	void versionInfoReceived();
	void btnSendAdvClicked();
	void btnSendChatClicked();
	void cd_btnJoinClicked();
	void cd_btnProomsJoinClicked();
	void cd_btnCancelClicked();
	void mr_btnSubmitClicked();
	void mr_btnCancelClicked();
	void ss_btnSubmitClicked();
	void ss_btnCancelClicked();
	void ul_pmRequested();
	void ul_infoRequested();
	void ul_ignoreAdd();
	void ul_ignoreRemove();
	void ul_channelBan();
	void ul_channelKick();
	void ul_chatBan();
	void ul_chatKick();
	void ul_chatTimeout();
	void ul_channelOpAdd();
	void ul_channelOpRemove();
	void ul_profileRequested();
	void ul_chatOpAdd();
	void ul_chatOpRemove();
	void to_btnSubmitClicked();
	void to_btnCancelClicked();
	void ci_btnCloseClicked();
	void fr_btnFriendsPMClicked();
	void fr_btnIgnoreRemoveClicked();
	void fr_btnCloseClicked();
	void fr_btnIgnoreAddClicked();
	void fr_friendsContextMenuRequested ( const QPoint& point );
	void ai_btnSubmitClicked();
	void ai_btnCancelClicked();
	void re_btnSubmitClicked();
	void re_btnCancelClicked();
	void se_btnSubmitClicked();
	void se_btnCancelClicked();
	void tb_channelRightClicked ( const QPoint & point );
	void tb_closeClicked();
	void tb_settingsClicked();
	void cs_chbEditDescriptionToggled(bool state);
	void cs_btnCancelClicked();
	void cs_btnSaveClicked();
	void openPMTab();
	void openPMTab ( QString& character );
	void displayCharacterContextMenu ( FCharacter* ch );
	void displayChannelContextMenu ( FChannelPanel* ch );


public:
	void parseInput();
	void flashApp(QString& reason);
	static const int BUFFERPUB  = 4096; // Buffer limit in public
	static const int BUFFERPRIV = 50000; // Buffer limit in private

private:
	FAccount *account;
	FServer *server;

	bool debugging;
	bool versionIsOkay;
	bool notificationsAreaMessageShown;
	void printDebugInfo(std::string s);
	void createTrayIcon();
	void setupConsole();								// Makes the console channel.
	void sendWS ( std::string& input );					// Sends messages to the server
	FChannelTab* addToActivePanels ( QString& channel, QString &channelname, QString& tooltip );	// Adds the newly joined channel to the displayed list of channels
	void leaveChannel(QString &panelname, QString &channelname, bool toServer = true);		// Leaves the channel
	void messageChannel ( std::string& channel, std::string& message );	// Sends a message to the specified channel
	void messagePrivate ( std::string& character, std::string& message );	// Sends a message to the specified character
	void advertiseChannel(std::string& channel, std::string& message);
	void sendIgnoreAdd ( QString& character );			// Sends an ignore request to the server
	void sendIgnoreDelete ( QString& character );		// Sends an ignore request to the server
	void changeStatus ( std::string& status, std::string& statusmsg );
	void refreshUserlist();								// Refreshes the GUI's userlist, based on what the current panel is
	void refreshChatLines();							// Refreshes the GUI's chat lines, based on what the current panel is
	void usersCommand();								// Does the /users thing.
	void receivePM ( QString& message, QString& character );
	void typingPaused ( FChannelPanel* channel );
	void typingContinued ( FChannelPanel* channel );
	void typingCleared ( FChannelPanel* channel );
	QString* usernameToHtml ( QString& name );
	void addToChannelsDialogList ( ChannelListItem* cli );
	void addToProomsDialogList ( ChannelListItem* cli );
	void addToFriendsList ( QListWidgetItem* lwi );
	void addToIgnoreList ( QListWidgetItem* lwi );
	void saveSettings();
	void loadSettings();
	void loadDefaultSettings();
	void parseSettingsLine(QString line);
	QNetworkAccessManager qnam;
	QNetworkReply* lreply;
	QUrl lurl;
	QUrlQuery lparam;

	unsigned int loginStep;
	FChannelPanel* console;	// We could just put this into the channel list, but the console needs to be accessed quite often. So here we go...
	FChannelPanel* currentPanel;
	FSound soundPlayer;
	BBCodeParser bbparser;
	QStringList selfPingList;
	QStringList defaultChannels;
	QString charName;
	QString selfStatus;
	QString selfStatusMessage;
	bool disconnected;
	static QString settingsPath;
	bool doingWS;
	//QList<QString> opList;
	//QHash<QString, FCharacter*> characterList;
	QHash<QString, FChannelPanel*> channelList;
	FCharacter* ul_recent;
	FChannelPanel* tb_recent;
	QMenu* recentCharMenu;
	QMenu* recentChannelMenu;
	//========================================
	QSystemTrayIcon* trayIcon;
	QMenu* trayIconMenu;

	QDialog* channelsDialog;
	QTabWidget* cd_twOverview;
	QVBoxLayout* cd_vblOverview;
	QVBoxLayout* cd_vblChannels; // cd stands for channels dialog
	QVBoxLayout* cd_vblProoms;
	QGroupBox* cd_gbChannels;
	QHBoxLayout* cd_hblChannelsSouthButtons;
	QHBoxLayout* cd_hblChannelsCenter;
	QPushButton* cd_btnChannelsCancel;
	QPushButton* cd_btnChannelsJoin;
	QListWidget* cd_channelsList;
	QGroupBox* cd_gbProoms;
	QHBoxLayout* cd_hblProomsSouthButtons;
	QHBoxLayout* cd_hblProomsCenter;
	QPushButton* cd_btnProomsCancel;
	QPushButton* cd_btnProomsJoin;
	QListWidget* cd_proomsList;

	QDialog* makeRoomDialog; // mr stands for makeroom
	QGroupBox* mr_gbOverview;
	QLabel* mr_lblNameRequest;
	QLabel* mr_lblInstructions;
	QLabel* mr_lblWarning;
	QLineEdit* mr_leName;
	QPushButton* mr_btnCancel;
	QPushButton* mr_btnSubmit;
	QVBoxLayout* mr_vblOverview;
	QVBoxLayout* mr_vblContents;
	QHBoxLayout* mr_hblButtons;

	QDialog* setStatusDialog; // ss stands for setstatus
	QLineEdit* ss_leMessage;
	QComboBox* ss_cbStatus;

	QDialog* settingsDialog; // se stands for settings
	QCheckBox* se_chbLeaveJoin;
	QCheckBox* se_chbOnlineOffline;
	QCheckBox* se_chbEnableChatLogs;
	QCheckBox* se_chbMute;
	QCheckBox* se_chbAlwaysPing;
	QCheckBox* se_chbEnablePing;
	QLineEdit* se_lePingList;
	QCheckBox* se_chbHelpdesk;
	bool se_leaveJoin;
	bool se_onlineOffline;
	bool se_chatLogs;
	bool se_sounds;
	bool se_alwaysPing;
	bool se_ping;
	bool se_helpdesk;

	QDialog* characterInfoDialog; // ci stands for character info
	QLabel* ci_lblName;
	QLabel* ci_lblStatusMessage;
	QTextEdit* ci_teKinks;
	QTextEdit* ci_teProfile;

	QDialog* friendsDialog; // fr stands for friends
	QVBoxLayout* fr_vblOverview;
	QTabWidget* fr_twOverview;
	QHBoxLayout* fr_hblSouthButtons;
	QListWidget* fr_lwFriends;
	QListWidget* fr_lwIgnore;
	QHBoxLayout* fr_hblIgnoreButtons;
	QHBoxLayout* fr_hblFriendsButtons;
	QPushButton* fr_btnFriendsPM;
	QPushButton* fr_btnIgnoreRemove;
	QPushButton* fr_btnIgnoreAdd;
	QPushButton* fr_btnClose;
	QVBoxLayout* fr_vblFriends;
	QVBoxLayout* fr_vblIgnore;
	QGroupBox* fr_gbIgnore;
	QGroupBox* fr_gbFriends;
	QLabel* fr_lblFriendsInstructions;
	QLabel* fr_lblIgnoreInstructions;

	QDialog* addIgnoreDialog; //ai stands for add ignore
	QVBoxLayout* ai_vblOverview;
	QHBoxLayout* ai_hblSouthButtons;
	QPushButton* ai_btnSubmit;
	QPushButton* ai_btnCancel;
	QLineEdit* ai_leName;
	QLabel* ai_lblInstructions;

	QDialog* reportDialog; // re stands for report
	QVBoxLayout* re_vblOverview;
	QLabel* re_lblWho;
	QLabel* re_lblProblem;
	QLabel* re_lblInstructions;
	QLineEdit* re_leWho;
	QTextEdit* re_teProblem;
	QHBoxLayout* re_hblButtons;
	QPushButton* re_btnCancel;
	QPushButton* re_btnSubmit;

	QDialog* helpDialog; // he stands for help

	QDialog* timeoutDialog; // to stands for timeout
	QLineEdit* to_leWho;
	QLineEdit* to_leLength;
	QLineEdit* to_leReason;

	QDialog* channelSettingsDialog; // cs stands for channel settings
	QTextEdit* cs_teDescription;
	QTextBrowser* cs_tbDescription;
	QCheckBox* cs_chbEditDescription;
	QVBoxLayout* cs_vblOverview;
	QVBoxLayout* cs_vblDescription;
	QGroupBox* cs_gbDescription;
	QHBoxLayout* cs_hblButtons;
	QPushButton* cs_btnCancel;
	QPushButton* cs_btnSave;
	QString cs_qsPlainDescription;
	QCheckBox* cs_chbAlwaysPing;
	FChannelPanel* cs_chanCurrent;

	/* The following GUIs still need to be made:
	QDialog* kinkSearchDialog;
	 */

	/* Stuff to do:
		Kink search:
		[14:27 PM]>>FKS {"kinks":["77","239"],"genders":["Female","Transgender"],"orientations":["Straight","Gay"],"languages":["English"],"furryprefs":["Furs and / or humans"]}

	 */
};
#endif // flist_messenger_H
