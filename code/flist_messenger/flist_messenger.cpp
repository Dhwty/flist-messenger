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

#include "flist_messenger.h"
#include <QString>

#define VERSION "F-List Messenger [Beta] 0.8.3"
#define VERSIONNUM "0.8.3"
#define CLIENTID "F-List Desktop Client"

// Bool to string macro
#define BOOLSTR(b) ( (b) ? "true" : "false" )
// String to bool macro
#define STRBOOL(s) ( (s=="true") ? true : false )

// Some day we may implement a proper websocket connection system. Today is not that day.
std::string flist_messenger::WSConnect = "GET / HTTP/1.1\r\nUpgrade: WebSocket\r\nConnection: Upgrade\r\nHost: f-list.net:9722\r\nOrigin: http://www.f-list.net\r\nSec-WebSocket-Key1: Z+t 6` H  XM%31   7T 5=7 330 r@\r\nSec-WebSocket-Key2: 267 0 4 0  \\ K36 3 2\r\n\r\nabcdefgh";
QString flist_messenger::settingsPath = "./settings.ini";

//get ticket, get characters, get friends list, get default character
void flist_messenger::prepareLogin ( QString& username, QString& password )
{
	lurl = QString ( "http://www.f-list.net/json/getApiTicket.json" );
	lurl.addQueryItem("secure", "no");
	lurl.addQueryItem("account", username);
	lurl.addQueryItem("password", password);
	lreply = qnam.get ( QNetworkRequest ( lurl ) ); //using lreply since this will replace the existing login system.
	connect ( lreply, SIGNAL ( finished() ), this, SLOT ( handleLogin() ) );
}
void flist_messenger::handleLogin()
{
	QMessageBox msgbox;

	if ( lreply->error() != QNetworkReply::NoError )
	{
		QString message = "Response error during login step ";
		message.append ( NumberToString::_uitoa<unsigned int> ( loginStep ).c_str() );
		message.append ( " of type " );
		message.append ( NumberToString::_uitoa<unsigned int> ( ( unsigned int ) lreply->error() ).c_str() );
		msgbox.critical ( this, "FATAL ERROR DURING LOGIN!", message );
		if (btnConnect) btnConnect->setEnabled(true);
		return;
	}

	QByteArray respbytes = lreply->readAll();

	lreply->deleteLater();
	std::string response ( respbytes.begin(), respbytes.end() );
	JSONNode respnode = libJSON::parse ( response );
	JSONNode childnode = respnode.at ( "error" );

	if ( childnode.as_string() != "" )
	{
		if (btnConnect) btnConnect->setEnabled(true);
		std::string message = "Error from server: " + childnode.as_string();
		QMessageBox::critical ( this, "Error", message.c_str() );
		return;
	}

	JSONNode subnode = respnode.at ( "ticket" );

	loginTicket = subnode.as_string();
	subnode = respnode.at ( "default_character" );
	defaultCharacter = subnode.as_string();
	childnode = respnode.at ( "characters" );
	int children = childnode.size();

	for ( int i = 0; i < children; ++i )
	{
		QString addchar = childnode[i].as_string().c_str();
		selfCharacterList.append ( addchar );
	}
	setupLoginBox();
}
void flist_messenger::versionInfoReceived()
{
}

void flist_messenger::init()
{
	settingsPath = QApplication::applicationDirPath() + "/settings.ini";
}

flist_messenger::flist_messenger(bool d)
{
	versionIsOkay = true;
	doingWS = true;
	notificationsAreaMessageShown = false;
	console = 0;
	textEdit = 0;
	tcpSock = 0;
	debugging = d;
	disconnected = true;
	friendsDialog = 0;
	addIgnoreDialog = 0;
	channelsDialog = 0;
	makeRoomDialog = 0;
	setStatusDialog = 0;
	characterInfoDialog = 0;
	ul_recent = 0;
	tb_recent = 0;
	recentCharMenu = 0;
	recentChannelMenu = 0;
	reportDialog = 0;
	helpDialog = 0;
	timeoutDialog = 0;
	settingsDialog = 0;
	trayIcon = 0;
	trayIconMenu = 0;
	channelSettingsDialog = 0;
	createTrayIcon();
	loadSettings();
	setupConnectBox();
	FCharacter::initClass();
	FChannel::initClass();
	FMessage::initClass(se_ping, se_alwaysPing, this);
}
void flist_messenger::closeEvent(QCloseEvent *event)
{
	if (disconnected)
		quitApp();
	if (trayIcon->isVisible())
	{
		if ( !notificationsAreaMessageShown)
		{
			QString title("Still running~");
			QString msg("F-chat Messenger is still running! "
						"To close it completely, right click the tray icon and click \"Quit\".");
			trayIcon->showMessage(title, msg, QSystemTrayIcon::Information, 10000);
			notificationsAreaMessageShown = true;
		}
		hide();
		event->ignore();
	}
}
void flist_messenger::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
	switch (reason) {
	case QSystemTrayIcon::Trigger:
	case QSystemTrayIcon::DoubleClick:
		showNormal();
		activateWindow();
		break;
	default:
		break;
	}
}
void flist_messenger::enterPressed()
{
    if (currentPanel == 0) return;

    QString input = plainTextEdit->toPlainText();
    if (currentPanel->getMode() == FChannel::CHANMODE_ADS && !input.startsWith("/")) {
        btnSendAdvClicked();
    } else {
        parseInput();
    }
}

void flist_messenger::createTrayIcon()
{
	trayIconMenu = new QMenu(this);
	trayIconMenu->addAction(QString("Minimize"), this, SLOT(hide()));
	trayIconMenu->addAction(QString("Maximize"), this, SLOT(showMaximized()));
	trayIconMenu->addAction(QString("Restore"), this, SLOT(showNormal()));
	trayIconMenu->addSeparator();
	trayIconMenu->addAction(QIcon(QString(":/images/cross.png")), QString("Quit"), this, SLOT(quitApp()));

	trayIcon = new QSystemTrayIcon(this);
	connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

	trayIcon->setContextMenu(trayIconMenu);
	trayIcon->setIcon(QIcon(":/images/icon.png"));
	trayIcon->setToolTip(QString("F-chat Messenger"));
	trayIcon->setVisible(true);
}
flist_messenger::~flist_messenger()
{
	// TODO
}
void flist_messenger::printDebugInfo(std::string s)
{
	if (debugging)
	{
		std::cout << s << std::endl;
		if (console)
		{
			QString q = s.c_str();
			QString gt = "&gt;";
			QString lt = "&lt;";
			QString amp = "&amp;";
			q.replace ( '&', amp ).replace ( '<', lt ).replace ( '>', gt );
			console->addLine(q, true, false);
		}
	}
}
void flist_messenger::setupConnectBox()
{
	this->setWindowTitle ( "F-chat messenger - Login" );
	this->setWindowIcon ( QIcon ( ":/images/icon.png" ) );

	// The "please log in" label
	verticalLayoutWidget = new QWidget ( this );
	verticalLayout = new QVBoxLayout ( verticalLayoutWidget );
	label = new QLabel();
	label->setText ( "Please log in using your F-list details." );
	verticalLayout->addWidget ( label );

	// The acc and pass textfields, along with labels
	gridLayout = new QGridLayout;
	label = new QLabel ( QString ( "Account:" ) );
	gridLayout->addWidget ( label, 0, 0 );
	label = new QLabel ( QString ( "Password:" ) );
	gridLayout->addWidget ( label, 1, 0 );
	ReturnLogin* loginreturn = new ReturnLogin(this);
	lineEdit = new QLineEdit(username);
	lineEdit->installEventFilter(loginreturn);
	lineEdit->setObjectName ( QString ( "accountNameInput" ) );
	gridLayout->addWidget ( lineEdit, 0, 1 );
	lineEdit = new QLineEdit;
	lineEdit->installEventFilter(loginreturn);
	lineEdit->setEchoMode ( QLineEdit::Password );
	lineEdit->setObjectName ( QString ( "passwordInput" ) );
	gridLayout->addWidget ( lineEdit, 1, 1 );

//	// "Checking version" label
//	lblCheckingVersion = new QLabel( QString ( "Checking version..." ) );
//	gridLayout->addWidget ( lblCheckingVersion, 2, 1 );

	// The login button
	btnConnect = new QPushButton;
	btnConnect->setObjectName ( QString ( "loginButton" ) );
	btnConnect->setText ( "Login" );
	btnConnect->setIcon ( QIcon ( ":/images/tick.png" ) );
//	btnConnect->hide();
	gridLayout->addWidget ( btnConnect, 2, 1 );
	verticalLayout->addLayout ( gridLayout );
	this->setCentralWidget ( verticalLayoutWidget );
	connect ( btnConnect, SIGNAL ( clicked() ), this, SLOT ( connectClicked() ) );

	int wid = QApplication::desktop()->width();
	int hig = QApplication::desktop()->height();
	int mwid = 265;
	int mhig = 100;
	setGeometry ( ( wid / 2 ) - ( int ) ( mwid*0.5 ), ( hig / 2 ) - ( int ) ( mhig*0.5 ), mwid, mhig );

//	// Fetch version.
//	lurl = QString ( "http://www.f-list.net/json/getApiTicket.json" );
//	lreply = qnam.get ( QNetworkRequest ( lurl ) );
//	connect ( lreply, SIGNAL ( finished() ), this, SLOT ( versionInfoReceived() ) );

}
void flist_messenger::connectClicked()
{
	if (versionIsOkay)
	{
		btnConnect->setEnabled(false);
		lineEdit = this->findChild<QLineEdit *> ( QString ( "accountNameInput" ) );
		username = lineEdit->text();
		lineEdit = this->findChild<QLineEdit *> ( QString ( "passwordInput" ) );
		password = lineEdit->text();
		loginStep = 1;
		prepareLogin ( username, password );
	}
}
void flist_messenger::setupLoginBox()
{
	clearConnectBox();
	groupBox = new QGroupBox ( this );
	groupBox->setObjectName ( QString::fromUtf8 ( "loginGroup" ) );
	groupBox->setGeometry ( QRect ( 0, 0, 250, 30 ) );
	pushButton = new QPushButton ( groupBox );
	pushButton->setObjectName ( QString::fromUtf8 ( "loginButton" ) );
	pushButton->setGeometry ( QRect ( 5, 40, 255, 26 ) );
	pushButton->setText ( "Login" );
	comboBox = new QComboBox ( groupBox );
	comboBox->setObjectName ( QString::fromUtf8 ( "charSelectBox" ) );
	comboBox->setGeometry ( QRect ( 80, 10, 180, 27 ) );

	for ( int i = 0;i < selfCharacterList.count();++i )
	{
		comboBox->addItem ( selfCharacterList[i] );

		if ( selfCharacterList[i].toStdString() == defaultCharacter )
		{
			comboBox->setCurrentIndex ( i );
		}
	}

	label = new QLabel ( groupBox );

	label->setObjectName ( QString::fromUtf8 ( "charlabel" ) );
	label->setGeometry ( QRect ( 10, 13, 71, 21 ) );
	label->setText ( "Character:" );
	setCentralWidget ( groupBox );
	connect ( pushButton, SIGNAL ( clicked() ), this, SLOT ( loginClicked() ) );
	int wid = QApplication::desktop()->width();
	int hig = QApplication::desktop()->height();
	int mwid = 265;
	int mhig = height();
	setGeometry ( ( wid / 2 ) - ( int ) ( mwid*0.5 ), ( hig / 2 ) - ( int ) ( mhig*0.5 ), mwid, mhig );
}
void flist_messenger::setupSetStatusUI()
{
	setStatusDialog = new QDialog ( this );
	QGroupBox* ss_gbOverview;
	QLabel* ss_lblStatusRequest;
	QLabel* ss_lblMessageRequest;
	QLabel* ss_lblInstructions;
	QPushButton* ss_btnSubmit;
	QPushButton* ss_btnCancel;
	QVBoxLayout* ss_vblOverview;
	QVBoxLayout* ss_vblContents;
	QHBoxLayout* ss_hblButtons;
	ss_gbOverview = new QGroupBox ( QString ( "Setting your status" ) );
	ss_lblStatusRequest = new QLabel ( QString ( "Your status:" ) );
	ss_lblMessageRequest = new QLabel ( QString ( "Your status message:" ) );
	ss_lblInstructions = new QLabel ( QString ( "This message will show up when people message you or click your name. Filling it out will also make you show up extra big in character searches, when your status is set to \"Looking\"." ) );
	ss_lblInstructions->setWordWrap ( true );
	ss_leMessage = new QLineEdit();
	ss_btnCancel = new QPushButton ( QString ( "Cancel" ) );
	ss_btnCancel->setIcon ( QIcon ( ":/images/cross.png" ) );
	ss_btnSubmit = new QPushButton ( QString ( "Submit" ) );
	ss_btnSubmit->setIcon ( QIcon ( ":/images/tick.png" ) );
	ss_hblButtons = new QHBoxLayout;
	ss_vblOverview = new QVBoxLayout;
	ss_vblContents = new QVBoxLayout;
	ss_cbStatus = new QComboBox;
	ss_cbStatus->addItem ( QIcon ( ":/images/status-default.png" ), QString ( "Online" ) );
	ss_cbStatus->addItem ( QIcon ( ":/images/status.png" ), QString ( "Looking for play!" ) );
	ss_cbStatus->addItem ( QIcon ( ":/images/status-blue.png" ), QString ( "Away" ));
	ss_cbStatus->addItem ( QIcon ( ":/images/status-away.png" ), QString ( "Busy" ) );
	ss_cbStatus->addItem ( QIcon ( ":/images/status-busy.png" ), QString ( "Do not disturb" ) );

	setStatusDialog->setWindowTitle ( QString ( "F-chat - Set Status" ) );
	setStatusDialog->setLayout ( ss_vblOverview );
	ss_vblOverview->addWidget ( ss_gbOverview );
	ss_vblOverview->addLayout ( ss_hblButtons );
	ss_hblButtons->addStretch();
	ss_hblButtons->addWidget ( ss_btnSubmit );
	ss_hblButtons->addWidget ( ss_btnCancel );
	ss_gbOverview->setLayout ( ss_vblContents );
	ss_vblContents->addWidget ( ss_lblStatusRequest );
	ss_vblContents->addWidget ( ss_cbStatus );
	ss_vblContents->addWidget ( ss_lblMessageRequest );
	ss_vblContents->addWidget ( ss_leMessage );
	ss_vblContents->addWidget ( ss_lblInstructions );
	connect ( ss_btnSubmit, SIGNAL ( clicked() ), this, SLOT ( ss_btnSubmitClicked() ) );
	connect ( ss_btnCancel, SIGNAL ( clicked() ), this, SLOT ( ss_btnCancelClicked() ) );
}
void flist_messenger::destroyMenu()
{
	if ( recentCharMenu )
		recentCharMenu->deleteLater();
}
void flist_messenger::destroyChanMenu()
{
	if (recentChannelMenu)
		recentChannelMenu->deleteLater();
}
void flist_messenger::setupFriendsDialog()
{
	friendsDialog = new QDialog ( this );
	fr_vblOverview = new QVBoxLayout;
	fr_twOverview = new QTabWidget;
	fr_hblSouthButtons = new QHBoxLayout;
	fr_lwFriends = new QListWidget;
	fr_lwIgnore = new QListWidget;
	fr_hblIgnoreButtons = new QHBoxLayout;
	fr_hblFriendsButtons = new QHBoxLayout;
	fr_btnIgnoreRemove = new QPushButton ( QIcon ( ":/images/cross-circle-frame.png" ), QString ( "Remove" ) );
	fr_btnIgnoreAdd = new QPushButton ( QString ( "Add user..." ) );
	fr_btnClose = new QPushButton ( QIcon ( ":/images/cross.png" ), QString ( "Close" ) );
	fr_vblFriends = new QVBoxLayout;
	fr_vblIgnore = new QVBoxLayout;
	fr_btnFriendsPM = new QPushButton ( QIcon ( ":/images/users.png" ), QString ( "Open PM" ) );
	fr_lblFriendsInstructions = new QLabel ( QString ( "(Adding friends only goes via the website.)" ) );
	fr_lblIgnoreInstructions = new QLabel ( QString ( "(You can ignore users by right-clicking them.)" ) );

	friendsDialog->setLayout ( fr_vblOverview );
	fr_vblOverview->addWidget ( fr_twOverview );
	fr_vblOverview->addLayout ( fr_hblSouthButtons );
	fr_hblSouthButtons->addStretch();
	fr_hblSouthButtons->addWidget ( fr_btnClose );

	// Friends tab
	fr_gbFriends = new QGroupBox;
	fr_gbFriends->setLayout ( fr_vblFriends );
	fr_vblFriends->addWidget ( fr_lwFriends );
	fr_vblFriends->addLayout ( fr_hblFriendsButtons );
	fr_vblFriends->addWidget ( fr_lblFriendsInstructions );
	fr_hblFriendsButtons->addStretch();
	fr_hblFriendsButtons->addWidget ( fr_btnFriendsPM );

	// Ignore tab
	fr_gbIgnore = new QGroupBox;
	fr_gbIgnore->setLayout ( fr_vblIgnore );
	fr_vblIgnore->addWidget ( fr_lwIgnore );
	fr_vblIgnore->addLayout ( fr_hblIgnoreButtons );
	fr_vblIgnore->addWidget ( fr_lblIgnoreInstructions );
	fr_hblIgnoreButtons->addStretch();
	fr_hblIgnoreButtons->addWidget ( fr_btnIgnoreAdd );
	fr_hblIgnoreButtons->addWidget ( fr_btnIgnoreRemove );

	fr_twOverview->addTab ( fr_gbFriends, QIcon ( ":/images/users.png" ), QString ( "Friends" ) );
	fr_twOverview->addTab ( fr_gbIgnore, QString ( "Ignore list" ) );

	fr_lwFriends->setContextMenuPolicy ( Qt::CustomContextMenu );
	connect ( fr_lwFriends, SIGNAL ( customContextMenuRequested ( const QPoint& ) ),
			  this, SLOT ( fr_friendsContextMenuRequested ( const QPoint& ) ) );
	connect ( fr_btnFriendsPM, SIGNAL ( clicked() ), this, SLOT ( fr_btnFriendsPMClicked() ) );
	connect ( fr_btnIgnoreRemove, SIGNAL ( clicked() ), this, SLOT ( fr_btnIgnoreRemoveClicked() ) );
	connect ( fr_btnIgnoreAdd, SIGNAL ( clicked() ), this, SLOT ( fr_btnIgnoreAddClicked() ) );
	connect ( fr_btnClose, SIGNAL ( clicked() ), this, SLOT ( fr_btnCloseClicked() ) );
	friendsDialog->setLayout ( fr_vblOverview );
}
void flist_messenger::setupReportDialog()
{
	reportDialog = new QDialog(this);
	re_vblOverview = new QVBoxLayout;
	re_hblButtons = new QHBoxLayout;
	re_lblWho = new QLabel("Reporting user:");
	re_lblProblem = new QLabel("Describe your problem.");
	re_lblInstructions = new QLabel("Note that channel mods do <b>not</b> receive staff alerts. If you are reporting a problem in a private channel, please contact the channel's moderator instead. Furthermore, if you are reporting something that is not against the rules, we will not be able to help you. If you are reporting private messages, try using /ignore first.");
	re_lblInstructions->setWordWrap(true);
	re_leWho = new QLineEdit();
	re_teProblem = new QTextEdit();
    re_teProblem->setObjectName("reportinput");
	re_btnSubmit = new QPushButton(QIcon(QString(":/images/tick.png")), QString("Submit"));
	re_btnCancel = new QPushButton(QIcon(QString(":/images/cross.png")), QString("Cancel"));

	reportDialog->setLayout(re_vblOverview);
	re_vblOverview->addWidget(re_lblProblem);
	re_vblOverview->addWidget(re_teProblem);
	re_vblOverview->addWidget(re_lblWho);
	re_vblOverview->addWidget(re_leWho);
	re_vblOverview->addWidget(re_lblInstructions);
	re_vblOverview->addLayout(re_hblButtons);
	re_hblButtons->addStretch();
	re_hblButtons->addWidget(re_btnSubmit);
	re_hblButtons->addWidget(re_btnCancel);

	connect ( re_btnSubmit, SIGNAL ( clicked() ), this, SLOT ( re_btnSubmitClicked() ) );
	connect ( re_btnCancel, SIGNAL ( clicked() ), this, SLOT ( re_btnCancelClicked() ) );
}
void flist_messenger::setupHelpDialog()
{
	helpDialog = new QDialog(this);
	QVBoxLayout* he_vblOverview;
	QTabWidget* he_twOverview;
	QTextEdit* he_teCommands;
	QTextEdit* he_teColors;
	QTextEdit* he_teTags;
	QTextEdit* he_teAdmin;
	QTextBrowser* he_teHelp;
	helpDialog->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	he_vblOverview = new QVBoxLayout;
	he_twOverview = new QTabWidget;
	he_teCommands = new QTextEdit;
	he_teColors = new QTextEdit;
	he_teTags = new QTextEdit;
	he_teHelp = new QTextBrowser;
	he_teAdmin = new QTextEdit;
	helpDialog->setLayout(he_vblOverview);
	he_vblOverview->addWidget(he_twOverview);
	he_twOverview->addTab(he_teCommands, QString("Commands"));
	he_twOverview->addTab(he_teAdmin, QString("Admin"));
	he_twOverview->addTab(he_teColors, QString("Colours"));
	he_twOverview->addTab(he_teTags, QString("BBCode"));
	he_twOverview->addTab(he_teHelp, QString("Info"));
	he_teCommands->setReadOnly(true);
	he_teHelp->setReadOnly(true);
	he_teColors->setReadOnly(true);
	he_teAdmin->setReadOnly(true);
	he_teTags->setReadOnly(true);
	QString str = "/me <message><br />";
	str +=	"/join &lt;channel&gt;<br />";
	str +=	"/priv &lt;character&gt;<br />";
	str +=	"/ignore &lt;character&gt;<br />";
	str +=	"/unignore &lt;character&gt;<br />";
	str +=	"/ignorelist<br />";
	str +=	"/code<br />";
	str +=	"/roll &lt;1d10&gt; (WIP)<br />";
	str +=	"/status &lt;Online|Looking|Busy|DND&gt; &lt;optional message&gt;<br />";
	str +=	"<b>Channel owners:</b><br />";
	str +=	"/makeroom &lt;name&gt;<br />";
	str +=	"/invite &lt;person&gt;<br />";
	str +=	"/openroom<br />";
	str +=	"/closeroom<br />";
	str +=	"/setdescription &lt;description&gt;<br />";
	str +=	"/getdescription<br />";
	str +=	"/setmode &lt;chat|ads|both&gt;";
	he_teCommands->setHtml(str);
	str = "<b>Genders:</b><br />";
	str += "<span style=\"color:"+QString(FCharacter::genderColors[FCharacter::GENDER_NONE].name())+"\">" + FCharacter::genderStrings[FCharacter::GENDER_NONE] + "</span><br />";
	str += "<span style=\"color:"+QString(FCharacter::genderColors[FCharacter::GENDER_MALE].name())+"\">" + FCharacter::genderStrings[FCharacter::GENDER_MALE] + "</span><br />";
	str += "<span style=\"color:"+QString(FCharacter::genderColors[FCharacter::GENDER_FEMALE].name())+"\">" + FCharacter::genderStrings[FCharacter::GENDER_FEMALE] + "</span><br />";
	str += "<span style=\"color:"+QString(FCharacter::genderColors[FCharacter::GENDER_TRANSGENDER].name())+"\">" + FCharacter::genderStrings[FCharacter::GENDER_TRANSGENDER] + "</span><br />";
	str += "<span style=\"color:"+QString(FCharacter::genderColors[FCharacter::GENDER_SHEMALE].name())+"\">" + FCharacter::genderStrings[FCharacter::GENDER_SHEMALE] + "</span><br />";
	str += "<span style=\"color:"+QString(FCharacter::genderColors[FCharacter::GENDER_HERM].name())+"\">" + FCharacter::genderStrings[FCharacter::GENDER_HERM] + "</span><br />";
	str += "<span style=\"color:"+QString(FCharacter::genderColors[FCharacter::GENDER_MALEHERM].name())+"\">" + FCharacter::genderStrings[FCharacter::GENDER_MALEHERM] + "</span><br />";
	str += "<span style=\"color:"+QString(FCharacter::genderColors[FCharacter::GENDER_CUNTBOY].name())+"\">" + FCharacter::genderStrings[FCharacter::GENDER_CUNTBOY] + "</span><br /><br />";
	str += "<b><i>Global OP</i></b><br />";
	str += "<b>Channel OP</b><br />";
	he_teColors->setHtml(str);
	str="<b>Admin commands</b><br /><br />";
	str+="/broadcast <message><br />";
	str+="/op<br />";
	str+="/deop<br />";
	str+="<b>Chatop commands</b><br />";
	str+="<br />";
	str+="/gkick<br />";
	str+="/timeout<br />";
	str+="/ipban<br />";
	str+="/accountban<br />";
	str+="/gunban<br />";
	str+="/createchannel<br />";
	str+="/killchannel<br />";
	str+="<b>Chan-Op commands</b><br />";
	str+="<br />";
	str+="/warn<br />";
	str+="/kick<br />";
	str+="/ban<br />";
	str+="/unban<br />";
	str+="/banlist<br />";
	str+="/coplist<br />";
	str+="<b>Chan Owner commands</b><br />";
	str+="<br />";
	str+="/cop<br />";
	str+="/cdeop<br />";
	str+="/setmode $lt;chat|ads|both&gt;<br />";
	he_teAdmin->setHtml(str);
	str = "<b>BBCode tags:</b><br /><br />"
			"<i>[i]italic[/i]</i><br />"
			"<u>[u]underline[/u]</u><br />"
			"<b>[b]bold[/b]</b><br />"
			"[user]name[/user] - Link to user<br />"
			"[channel]channel name[/channel] - Link to channel<br />"
			"[session=name]linkcode[/session] - Link to private room<br />"
			"[url=address]label[/url] - The word \"label\" will link to the address<br />";
	he_teTags->setHtml(str);
	str = "<b>F-chat Desktop Messenger</b><br />";
	str+= "by <a href=\"#USR-Viona\">Viona</a>, <a href=\"#USR-Kira\">Kira</a>, <a href=\"#USR-Aniko\">Aniko</a>, <a href=\"#USR-Hexxy\">Hexxy</a> & <a href=\"#USR-Eager\">Eager</a>.<br />";
	str+= "For bug reports, PM Viona or post <a href=\"#LNK-http://www.f-list.net/forum.php?forum=1698\">here</a>.<br />";
	str+= "(Please do not use the helpdesk or contact other staff for this.)<br /><br />";
	str+= "Thank you for using the messenger's beta version. For updates, regularly check the F-chat Desktop Client group forums.<br />";
	str+= "To get debug output, run the application with the \"-d\" argument.<br />";
	he_teHelp->setHtml(str);
	helpDialog->resize(500, 400);
	connect(he_teHelp, SIGNAL(anchorClicked(QUrl)), this, SLOT(anchorClicked(QUrl)));
}
void flist_messenger::helpDialogRequested()
{
	if (helpDialog == 0 || helpDialog->parent() != this)
		setupHelpDialog();
	helpDialog->show();
}
void flist_messenger::channelSettingsDialogRequested()
{
	// This is one that always needs to be setup, because its contents may vary.
	if (setupChannelSettingsDialog())
		channelSettingsDialog->show();
}
bool flist_messenger::setupChannelSettingsDialog()
{

	if (tb_recent->type() == FChannel::CHANTYPE_PM)
	{
		// Setup for PM
		if (characterList.count(tb_recent->recipient()) == 0) // Recipient is offline
			return false;
		channelSettingsDialog = new QDialog(this);
		FCharacter* ch = characterList[tb_recent->recipient()];
		cs_chanCurrent = tb_recent;
		channelSettingsDialog->setWindowTitle(ch->name());
		channelSettingsDialog->setWindowIcon(tb_recent->pushButton->icon());
		cs_qsPlainDescription = ch->statusMsg();
		cs_vblOverview = new QVBoxLayout;
		cs_gbDescription = new QGroupBox("Status");
		cs_vblDescription = new QVBoxLayout;
		QLabel* lblStatus = new QLabel(ch->statusString());
		cs_tbDescription = new QTextBrowser;
		cs_tbDescription->setHtml(bbparser.parse(cs_qsPlainDescription));
		cs_tbDescription->setReadOnly(true);
		cs_hblButtons = new QHBoxLayout;
		cs_btnCancel = new QPushButton(QIcon ( QString ( ":/images/cross.png" ) ), "Cancel");

		channelSettingsDialog->setLayout(cs_vblOverview);
		cs_vblOverview->addWidget(cs_gbDescription);
		cs_gbDescription->setLayout(cs_vblDescription);
		cs_vblDescription->addWidget(lblStatus);
		cs_vblDescription->addWidget(cs_tbDescription);
		cs_vblOverview->addLayout(cs_hblButtons);
		cs_hblButtons->addStretch();
		cs_hblButtons->addWidget(cs_btnCancel);

		connect(cs_btnCancel, SIGNAL(clicked()), this, SLOT(cs_btnCancelClicked()));
		connect(cs_tbDescription, SIGNAL ( anchorClicked ( QUrl ) ), this, SLOT ( anchorClicked ( QUrl ) ) );
	} else {
		channelSettingsDialog = new QDialog(this);
		FChannel* ch = tb_recent;
		cs_chanCurrent = ch;
		channelSettingsDialog->setWindowTitle(ch->title());
		channelSettingsDialog->setWindowIcon(ch->pushButton->icon());
		cs_qsPlainDescription = ch->description();
		cs_vblOverview = new QVBoxLayout;
		cs_gbDescription = new QGroupBox("Description");
		cs_vblDescription = new QVBoxLayout;
		cs_tbDescription = new QTextBrowser;
		cs_tbDescription->setHtml(bbparser.parse(cs_qsPlainDescription));
		cs_teDescription = new QTextEdit;
		cs_teDescription->setPlainText(cs_qsPlainDescription);
		cs_teDescription->hide();
		cs_chbEditDescription = new QCheckBox("Editable mode (OPs only)");
		if ( ! ( ch->isOp(characterList[charName]) || characterList[charName]->isChatOp() ) )
			cs_chbEditDescription->setEnabled(false);
		cs_chbAlwaysPing = new QCheckBox("Always ping in this channel");
		cs_chbAlwaysPing->setChecked(ch->getAlwaysPing());
		cs_hblButtons = new QHBoxLayout;
		cs_btnCancel = new QPushButton(QIcon ( QString ( ":/images/cross.png" ) ), "Cancel");
		cs_btnSave = new QPushButton(QIcon ( QString ( ":/images/tick.png" ) ), "Save");

		channelSettingsDialog->setLayout(cs_vblOverview);
		cs_vblOverview->addWidget(cs_gbDescription);
		cs_gbDescription->setLayout(cs_vblDescription);
		cs_vblDescription->addWidget(cs_teDescription);
		cs_vblDescription->addWidget(cs_tbDescription);
		cs_vblDescription->addWidget(cs_chbEditDescription);
		cs_vblOverview->addWidget(cs_chbAlwaysPing);
		cs_vblOverview->addLayout(cs_hblButtons);
		cs_hblButtons->addStretch();
		cs_hblButtons->addWidget(cs_btnSave);
		cs_hblButtons->addWidget(cs_btnCancel);

		connect(cs_chbEditDescription, SIGNAL(toggled(bool)), this, SLOT(cs_chbEditDescriptionToggled(bool)));
		connect(cs_btnCancel, SIGNAL(clicked()), this, SLOT(cs_btnCancelClicked()));
		connect(cs_btnSave, SIGNAL(clicked()), this, SLOT(cs_btnSaveClicked()));
		connect(cs_tbDescription, SIGNAL ( anchorClicked ( QUrl ) ), this, SLOT ( anchorClicked ( QUrl ) ) );
	}
	return true;
}
void flist_messenger::cs_chbEditDescriptionToggled(bool state)
{

	if (state)
	{
		// display plain text BBCode
		cs_teDescription->setPlainText(cs_qsPlainDescription);
		cs_teDescription->show();
		cs_tbDescription->hide();
	} else {
		// display html
		cs_qsPlainDescription = cs_teDescription->toPlainText();
		cs_tbDescription->setHtml(bbparser.parse(cs_qsPlainDescription));
		cs_tbDescription->show();
		cs_teDescription->hide();
	}
}
void flist_messenger::cs_btnCancelClicked()
{
	cs_qsPlainDescription = "";
	cs_chanCurrent = 0;
	channelSettingsDialog->deleteLater();
}
void flist_messenger::cs_btnSaveClicked()
{
	if (cs_qsPlainDescription != cs_chanCurrent->description())
	{
		std::cout << "Editing description." << std::endl;
		// Update description
		JSONNode node;
		JSONNode channode ( "channel", cs_chanCurrent->name().toStdString() );
		node.push_back ( channode );
		JSONNode descnode ( "description", cs_qsPlainDescription.toStdString() );
		node.push_back ( descnode );
		std::string out = "CDS " + node.write();
		sendWS ( out );
	}
	cs_chanCurrent->setAlwaysPing(cs_chbAlwaysPing->isChecked());

	QString setting = cs_chanCurrent->name();
	setting += "/alwaysping";
	QSettings settings(settingsPath, QSettings::IniFormat);
	settings.setValue(setting, BOOLSTR(cs_chbAlwaysPing->isChecked()));

	cs_qsPlainDescription = "";
	cs_chanCurrent = 0;
	channelSettingsDialog->deleteLater();
}

void flist_messenger::setupAddIgnoreDialog()
{
	addIgnoreDialog = new QDialog ( this );
	ai_btnSubmit = new QPushButton ( QIcon ( QString ( ":/images/tick.png" ) ), QString ( "Submit" ) );
	ai_btnCancel = new QPushButton ( QIcon ( QString ( ":/images/cross.png" ) ), QString ( "Cancel" ) );
	ai_vblOverview = new QVBoxLayout;
	ai_hblSouthButtons = new QHBoxLayout;
	ai_leName = new QLineEdit;
	ai_lblInstructions = new QLabel ( QString ( "Characters on this list are not allowed to message you. If people switch characters to get around this, please contact an F-chat operator about it!<br /><b>Name:</b>" ) );
	ai_lblInstructions->setWordWrap ( true );

	addIgnoreDialog->setLayout ( ai_vblOverview );
	ai_vblOverview->addWidget ( ai_lblInstructions );
	ai_vblOverview->addWidget ( ai_leName );
	ai_vblOverview->addLayout ( ai_hblSouthButtons );
	ai_hblSouthButtons->addStretch();
	ai_hblSouthButtons->addWidget ( ai_btnSubmit );
	ai_hblSouthButtons->addWidget ( ai_btnCancel );

	connect ( ai_btnSubmit, SIGNAL ( clicked() ), this, SLOT ( ai_btnSubmitClicked() ) );
	connect ( ai_btnCancel, SIGNAL ( clicked() ), this, SLOT ( ai_btnCancelClicked() ) );
}
void flist_messenger::addIgnoreDialogRequested()
{
	if ( addIgnoreDialog == 0 || addIgnoreDialog->parent() != this )
		setupAddIgnoreDialog();

	ai_leName->setText ( QString ( "" ) );
	addIgnoreDialog->show();
}
void flist_messenger::ai_btnCancelClicked()
{
	addIgnoreDialog->hide();
}
void flist_messenger::ai_btnSubmitClicked()
{
	QString character = ai_leName->text().simplified();

	if ( character != "" && selfIgnoreList.count ( character ) == 0 )
	{
		sendIgnoreAdd(character);
		addIgnoreDialog->hide();
	}
}
void flist_messenger::re_btnSubmitClicked()
{
	submitReport();
	reportDialog->hide();
}

void flist_messenger::submitReport()
{
	lurl = QString ( "http://www.f-list.net/json/getApiTicket.json?secure=no&account=" + username + "&password=" + password );
	lreply = qnam.get ( QNetworkRequest ( lurl ) );
	connect ( lreply, SIGNAL ( finished() ), this, SLOT ( reportTicketFinished() ) );
}

void flist_messenger::handleReportFinished()
{
    if ( lreply->error() != QNetworkReply::NoError ){
        QString message = "Response error during sending of report ";
        message.append ( "of type " );
        message.append ( NumberToString::_uitoa<unsigned int> ( ( unsigned int ) lreply->error() ).c_str() );
        QMessageBox::critical ( this, "FATAL ERROR DURING TICKET RETRIEVAL!", message );
        return;
    } else {
        QByteArray respbytes = lreply->readAll();
        lreply->deleteLater();
        std::string response ( respbytes.begin(), respbytes.end() );
        JSONNode respnode = libJSON::parse ( response );
        JSONNode childnode = respnode.at ( "error" );
        if ( childnode.as_string() != "" )
        {
            btnConnect->setEnabled(true);
            std::string message = "Error from server: " + childnode.as_string();
            QMessageBox::critical ( this, "Error", message.c_str() );
            return;
        } else {
            childnode = respnode.at ( "log_id" );
            std::string logid = childnode.as_string();
            QString gt = "&gt;";
            QString lt = "&lt;";
            QString amp = "&amp;";
            QString problem = re_teProblem->toPlainText().replace ( '&', amp ).replace ( '<', lt ).replace ( '>', gt );
            QString who = re_leWho->text().replace ( '&', amp ).replace ( '<', lt ).replace ( '>', gt );
            if (who.trimmed() == "") who = "None";
            QString report = "Current Tab/Channel: " + currentPanel->title() + " | Reporting User: " + who + " | " + problem;
            JSONNode node;
            JSONNode actionnode("action", "report");
            std::cout << logid << std::endl;
            JSONNode logidnode("logid", logid);
            JSONNode charnode("character", charName.toStdString());
            JSONNode reportnode("report", report.toStdString());
            node.push_back(actionnode);
            node.push_back(charnode);
            node.push_back(reportnode);
            node.push_back(logidnode);
            std::string output = "SFC " + node.write();
            sendWS(output);
            reportDialog->hide();
            re_leWho->clear();
            re_teProblem->clear();
            QString message = "Your report was uploaded";
            QMessageBox::information ( this, "Success.", message );
        }

    }
}

void flist_messenger::reportTicketFinished()
{
    if ( lreply->error() != QNetworkReply::NoError )
	{
		QString message = "Response error during fetching of ticket ";
		message.append ( NumberToString::_uitoa<unsigned int> ( loginStep ).c_str() );
		message.append ( " of type " );
		message.append ( NumberToString::_uitoa<unsigned int> ( ( unsigned int ) lreply->error() ).c_str() );
		QMessageBox::critical ( this, "FATAL ERROR DURING TICKET RETRIEVAL!", message );
		return;
	}
	QByteArray respbytes = lreply->readAll();
	lreply->deleteLater();
	std::string response ( respbytes.begin(), respbytes.end() );
	JSONNode respnode = libJSON::parse ( response );
	JSONNode childnode = respnode.at ( "error" );
	if ( childnode.as_string() != "" )
	{
		btnConnect->setEnabled(true);
		std::string message = "Error from server: " + childnode.as_string();
		QMessageBox::critical ( this, "Error", message.c_str() );
		return;
    }
    JSONNode subnode = respnode.at ( "ticket" );
    loginTicket = subnode.as_string().c_str();
    QString url_string="http://www.f-list.net/json/api/report-submit.php?account=";
    url_string += username;
    url_string += "&character=";
    url_string += charName;
    url_string += "&ticket=";
    url_string += loginTicket.c_str();
    lurl = url_string;
    std::cout << url_string.toStdString() << std::endl;
    QByteArray postData;
    JSONNode* lognodes;
    lognodes = currentPanel->toJSON();
    std::string toWrite;
    toWrite = lognodes->write();
    QNetworkRequest request(lurl);
    fix_broken_escaped_apos(toWrite);
    lurl.addQueryItem("log", toWrite.c_str());
    postData = lurl.encodedQuery();
    lreply = qnam.post ( request, postData );
    connect ( lreply, SIGNAL ( finished() ), this, SLOT ( handleReportFinished() ) );
    delete lognodes;
}
void flist_messenger::re_btnCancelClicked()
{
	reportDialog->hide();
}
void flist_messenger::se_btnSubmitClicked()
{
	se_helpdesk = se_chbHelpdesk->isChecked();
	se_ping = se_chbEnablePing->isChecked();
	se_leaveJoin = se_chbLeaveJoin->isChecked();
	se_alwaysPing = se_chbAlwaysPing->isChecked();
	se_sounds = ! se_chbMute->isChecked();
	se_chatLogs = se_chbEnableChatLogs->isChecked();
	se_onlineOffline = se_chbOnlineOffline->isChecked();

	selfPingList.clear();
	QString liststr = se_lePingList->text();
	QStringList list = liststr.split(',');
	foreach (QString s, list)
	{
		s = s.trimmed();
		if (s != "")
			selfPingList.append(s.trimmed());
	}
	saveSettings();
	settingsDialog->hide();
}
void flist_messenger::se_btnCancelClicked()
{
	settingsDialog->hide();
}
void flist_messenger::tb_closeClicked()
{
	if (tb_recent == 0) return;
	bool current = false;
	if (currentPanel && tb_recent == currentPanel)
	{
		current = true;
	}
	if (tb_recent->type() == FChannel::CHANTYPE_PM)
	{
		tb_recent->setActive(false);
		tb_recent->pushButton->setVisible(false);
		tb_recent->setTyping ( FChannel::TYPINGSTATUS_CLEAR );
	} else {
		std::string channel = tb_recent->name().toStdString();
		leaveChannel ( channel );
	}
	if (current)
	{
		QString c = "CONSOLE";
		switchTab(c);
	}
}
void flist_messenger::tb_settingsClicked()
{
	channelSettingsDialogRequested();
}
void flist_messenger::setupCharacterInfoUI()
{
	characterInfoDialog = new QDialog ( this );
	QVBoxLayout* ci_vblOverview;
	QVBoxLayout* ci_vblContents;
	QGroupBox* ci_gbOverview;
	QTabWidget* ci_twKP;
	QLabel* ci_lblStatus;
	QLabel* ci_lblKinks;
	QLabel* ci_lblProfile;
	QHBoxLayout* ci_hblButtons;
	QPushButton* ci_btnClose;
	ci_vblOverview = new QVBoxLayout;
	ci_vblContents = new QVBoxLayout;
	ci_gbOverview = new QGroupBox;
	ci_lblName = new QLabel;
	ci_lblStatus = new QLabel;
	ci_lblStatusMessage = new QLabel;
	ci_lblStatusMessage->setWordWrap ( true );
	ci_hblButtons = new QHBoxLayout;
	ci_btnClose = new QPushButton;
	ci_lblKinks = new QLabel ( QString ( "Kinks" ) );
	ci_lblProfile = new QLabel ( QString ( "Profile" ) );
	ci_teKinks = new QTextEdit;
	ci_teProfile = new QTextEdit;
	ci_twKP = new QTabWidget;

	characterInfoDialog->setLayout ( ci_vblOverview );
	ci_vblOverview->addWidget ( ci_gbOverview );
	ci_vblOverview->addLayout ( ci_hblButtons );
	ci_gbOverview->setTitle ( QString ( "Character Info" ) );
	ci_gbOverview->setLayout ( ci_vblContents );
	ci_vblContents->addWidget ( ci_lblName );
	ci_vblContents->addWidget ( ci_lblStatusMessage );
	ci_vblContents->addWidget ( ci_twKP );
	ci_twKP->addTab ( ci_teProfile, QString ( "Profile" ) );
	ci_twKP->addTab ( ci_teKinks, QString ( "Kinks" ) );
	ci_hblButtons->addStretch();
	ci_hblButtons->addWidget ( ci_btnClose );
	ci_btnClose->setIcon ( QIcon ( ":/images/cross.png" ) );
	ci_btnClose->setText ( QString ( "Close" ) );
	ci_teKinks->setReadOnly ( true );
	ci_teProfile->setReadOnly ( true );

	connect ( ci_btnClose, SIGNAL ( clicked() ), this, SLOT ( ci_btnCloseClicked() ) );
}
void flist_messenger::ci_btnCloseClicked()
{
	characterInfoDialog->close();
}
void flist_messenger::setupMakeRoomUI()
{
	makeRoomDialog = new QDialog ( this );
	mr_gbOverview = new QGroupBox ( QString ( "Making a private room" ) );
	mr_lblNameRequest = new QLabel ( QString ( "Room name:" ) );
	mr_lblInstructions = new QLabel ( QString ( "Rooms are always invite-only when first made. If you want to make your room public, type /openroom in it." ) );
	mr_lblInstructions->setWordWrap ( true );
	mr_lblWarning = new QLabel ( QString ( "<b>Note:</b> It is your own responsibility to moderate your room. If your room breaks public rules, it can be deleted by staff members." ) );
	mr_lblWarning->setWordWrap ( true );
	mr_leName = new QLineEdit();
	mr_btnCancel = new QPushButton ( QString ( "Cancel" ) );
	mr_btnCancel->setIcon ( QIcon ( ":/images/cross.png" ) );
	mr_btnSubmit = new QPushButton ( QString ( "Submit" ) );
	mr_btnSubmit->setIcon ( QIcon ( ":/images/tick.png" ) );
	mr_hblButtons = new QHBoxLayout;
	mr_vblOverview = new QVBoxLayout;
	mr_vblContents = new QVBoxLayout;

	makeRoomDialog->setWindowTitle ( QString ( "F-chat - Make Room" ) );
	makeRoomDialog->setLayout ( mr_vblOverview );
	mr_vblOverview->addWidget ( mr_gbOverview );
	mr_gbOverview->setLayout ( mr_vblContents );
	mr_vblContents->addWidget ( mr_lblInstructions );
	mr_vblContents->addWidget ( mr_lblNameRequest );
	mr_vblContents->addWidget ( mr_leName );
	mr_vblContents->addWidget ( mr_lblWarning );
	mr_vblOverview->addLayout ( mr_hblButtons );
	mr_hblButtons->addStretch();
	mr_hblButtons->addWidget ( mr_btnSubmit );
	mr_hblButtons->addWidget ( mr_btnCancel );
	connect ( mr_btnSubmit, SIGNAL ( clicked() ), this, SLOT ( mr_btnSubmitClicked() ) );
	connect ( mr_btnCancel, SIGNAL ( clicked() ), this, SLOT ( mr_btnCancelClicked() ) );
}
void flist_messenger::setupChannelsUI()
{
	if ( channelsDialog )
	{
		delete channelsDialog;
		delete cd_vblOverview;
		delete cd_vblChannels;
		delete cd_vblProoms;
		delete cd_twOverview;
		delete cd_gbChannels;
		delete cd_hblChannelsSouthButtons;
		delete cd_hblChannelsCenter;
		delete cd_hblProomsSouthButtons;
		delete cd_hblProomsCenter;
		delete cd_btnChannelsCancel;
		delete cd_btnChannelsJoin;
		delete cd_btnProomsCancel;
		delete cd_btnProomsJoin;
		delete cd_channelsList;
		delete cd_proomsList;
		delete cd_gbProoms;
	}

	channelsDialog = new QDialog ( this );

	cd_vblOverview = new QVBoxLayout;
	cd_vblChannels = new QVBoxLayout;
	cd_vblProoms = new QVBoxLayout;
	cd_twOverview = new QTabWidget;
	cd_gbChannels = new QGroupBox;
	cd_hblChannelsSouthButtons = new QHBoxLayout;
	cd_hblChannelsCenter = new QHBoxLayout;
	cd_hblProomsSouthButtons = new QHBoxLayout;
	cd_hblProomsCenter = new QHBoxLayout;
	cd_btnChannelsCancel = new QPushButton ( QString ( "Cancel" ) );
	cd_btnChannelsJoin = new QPushButton ( QString ( "Join" ) );
	cd_btnProomsCancel = new QPushButton ( QString ( "Cancel" ) );
	cd_btnProomsJoin = new QPushButton ( QString ( "Join" ) );
	cd_channelsList = new QListWidget;
	cd_proomsList = new QListWidget;
	cd_gbProoms = new QGroupBox;

	channelsDialog->setObjectName ( QString ( "F-chat - Channels" ) );
	channelsDialog->setWindowIcon ( QIcon ( ":/images/hash.png" ) );
	channelsDialog->setWindowTitle ( QString ( "F-chat - Channels" ) );
	channelsDialog->setLayout ( cd_vblOverview );
	cd_vblOverview->addWidget ( cd_twOverview );

	//Channels tab
	cd_gbChannels->setTitle ( QString ( "Channels" ) );
	cd_gbChannels->setLayout ( cd_vblChannels );
	cd_vblChannels->addLayout ( cd_hblChannelsCenter );
	cd_vblChannels->addLayout ( cd_hblChannelsSouthButtons );
	cd_hblChannelsCenter->addWidget ( cd_channelsList );
	cd_hblChannelsSouthButtons->addStretch();
	cd_hblChannelsSouthButtons->addWidget ( cd_btnChannelsJoin );
	cd_hblChannelsSouthButtons->addWidget ( cd_btnChannelsCancel );

	//Prooms tab
	cd_gbProoms->setTitle ( QString ( "Public rooms" ) );
	cd_gbProoms->setLayout ( cd_vblProoms );
	cd_vblProoms->addLayout ( cd_hblProomsCenter );
	cd_vblProoms->addLayout ( cd_hblProomsSouthButtons );
	cd_hblProomsCenter->addWidget ( cd_proomsList );
	cd_hblProomsSouthButtons->addStretch();
	cd_hblProomsSouthButtons->addWidget ( cd_btnProomsJoin );
	cd_hblProomsSouthButtons->addWidget ( cd_btnProomsCancel );

	cd_twOverview->addTab ( cd_gbChannels, QIcon ( ":/images/hash.png" ), QString ( "Channels" ) );
	cd_twOverview->addTab ( cd_gbProoms, QIcon ( ":/images/key.png" ), QString ( "Public Rooms" ) );
	cd_btnChannelsJoin->setIcon ( QIcon ( ":/images/hash.png" ) );
	cd_btnProomsJoin->setIcon ( QIcon ( ":/images/key.png" ) );
	cd_btnChannelsCancel->setIcon ( QIcon ( ":/images/cross.png" ) );
	cd_btnProomsCancel->setIcon ( QIcon ( ":/images/cross.png" ) );

	connect ( cd_btnChannelsJoin, SIGNAL ( clicked() ), this, SLOT ( cd_btnJoinClicked() ) );
	connect ( cd_btnChannelsCancel, SIGNAL ( clicked() ), this, SLOT ( cd_btnCancelClicked() ) );
	connect ( cd_btnProomsCancel, SIGNAL ( clicked() ), this, SLOT ( cd_btnCancelClicked() ) );
	connect ( cd_btnProomsJoin, SIGNAL ( clicked() ), this, SLOT ( cd_btnProomsJoinClicked() ) );
	channelsDialog->setGeometry ( this->geometry().left() + 15, this->geometry().top() + 15, 350, 250 );
}
void flist_messenger::loginClicked()
{
	disconnected = false;
	soundPlayer.play ( soundPlayer.SOUND_LOGIN );
	charName = comboBox->currentText();
	FMessage::selfName = charName;
	if ( tcpSock )
	{
		tcpSock->abort();
		tcpSock->deleteLater();
	}

	clearLoginBox();

	setupRealUI();
	tcpSock = new QTcpSocket ( this );
	tcpSock->connectToHost ( "chat.f-list.net", 9722 );
	connect ( tcpSock, SIGNAL ( connected() ), this, SLOT ( connectedToSocket() ) );
	connect ( tcpSock, SIGNAL ( readyRead() ), this, SLOT ( readReadyOnSocket() ) );
	connect ( tcpSock, SIGNAL ( error ( QAbstractSocket::SocketError ) ), this, SLOT ( socketError ( QAbstractSocket::SocketError ) ) );
}
void flist_messenger::clearConnectBox()
{
	btnConnect = 0;
	verticalLayoutWidget->deleteLater();
}
void flist_messenger::clearLoginBox()
{
	groupBox->deleteLater();
}
void flist_messenger::setupRealUI()
{
	// Setting up console first because it needs to receive server output.
	console = new FChannel ( "FCHATSYSTEMCONSOLE", FChannel::CHANTYPE_CONSOLE );
	QString name = "Console";
	console->setTitle ( name );
	channelList["FCHATSYSTEMCONSOLE"] = console;

	if ( objectName().isEmpty() )
		setObjectName ( "MainWindow" );

	resize ( 836, 454 );
	setWindowTitle ( VERSION );
	setWindowIcon ( QIcon ( ":/images/apple-touch-icon.png" ) );
	actionDisconnect = new QAction ( this );
	actionDisconnect->setObjectName ( "actionDisconnect" );
	actionDisconnect->setText ( "Disconnect (WIP)" );
	actionQuit = new QAction ( this );
	actionQuit->setObjectName ( "actionQuit" );
	actionQuit->setText ( "Quit" );
	actionHelp = new QAction ( this );
	actionHelp->setObjectName ( "actionHelp" );
	actionHelp->setText ( "Help" );
	actionAbout = new QAction ( this );
	actionAbout->setObjectName ( QString::fromUtf8 ( "actionAbout" ) );
	actionAbout->setText ( QString::fromUtf8 ( "About" ) );
	verticalLayoutWidget = new QWidget ( this );
	verticalLayoutWidget->setObjectName ( QString::fromUtf8 ( "overview" ) );
	verticalLayoutWidget->setGeometry ( QRect ( 5, -1, 841, 511 ) );
	verticalLayout = new QVBoxLayout ( verticalLayoutWidget );
	verticalLayout->setObjectName ( "overviewLayout" );
	verticalLayout->setContentsMargins ( 0, 0, 0, 0 );
	horizontalLayoutWidget = new QWidget ( this );
	horizontalLayoutWidget->setObjectName ( "main" );
	horizontalLayoutWidget->setGeometry ( QRect ( 0, -1, 831, 401 ) );
	horizontalLayout = new QHBoxLayout ( horizontalLayoutWidget );
	horizontalLayout->setObjectName ( "mainLayout" );
	horizontalLayout->setContentsMargins ( 0, 0, 0, 0 );
	activePanels = new QScrollArea ( horizontalLayoutWidget );
	activePanels->setObjectName ( QString::fromUtf8 ( "activePanels" ) );
	QSizePolicy sizePolicy ( QSizePolicy::Minimum, QSizePolicy::Expanding );
	sizePolicy.setHorizontalStretch ( 0 );
	sizePolicy.setVerticalStretch ( 0 );
	sizePolicy.setHeightForWidth ( activePanels->sizePolicy().hasHeightForWidth() );
	activePanels->setSizePolicy ( sizePolicy );
	activePanels->setMinimumSize ( QSize ( 67, 0 ) );
	activePanels->setMaximumSize ( QSize ( 16777215, 16777215 ) );
	activePanels->setBaseSize ( QSize ( 0, 0 ) );
	activePanels->setFrameShape ( QFrame::StyledPanel );
	activePanels->setFrameShadow ( QFrame::Sunken );
	activePanels->setVerticalScrollBarPolicy ( Qt::ScrollBarAsNeeded );
	activePanels->setHorizontalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
	activePanels->setWidgetResizable ( true );
	activePanelsContents = new QVBoxLayout();
	activePanelsContents->setObjectName ( "activePanelsLayout" );
	activePanelsContents->setGeometry ( QRect ( 0, 0, 61, 393 ) );
	activePanels->setFixedWidth ( 45 );
	activePanels->setLayout ( activePanelsContents );
	horizontalLayout->addWidget ( activePanels );
	centralStuff = new QVBoxLayout;
	centralButtonsWidget = new QWidget;
	centralButtons = new QHBoxLayout ( centralButtonsWidget );
	centralButtonsWidget->setLayout ( centralButtons );
	lblChannelName = new QLabel ( QString ( "" ) );
	lblChannelName->setObjectName( "currentpanelname" );
	lblChannelName->setWordWrap( true );
	lblChannelName->setFixedWidth( 350 );
	lblChannelName->setFixedHeight( 35 );
	btnSettings = new QPushButton;
	btnSettings->setIcon(QIcon(":/images/terminal.png"));
	btnSettings->setToolTip(QString("Settings"));
	btnSettings->setObjectName("settingsbutton");
	btnChannels = new QPushButton;
	btnChannels->setIcon ( QIcon ( ":/images/hash.png" ) );
	btnChannels->setToolTip(QString("Channels"));
	btnChannels->setObjectName("channelsbutton");
	btnMakeRoom = new QPushButton;
	btnMakeRoom->setIcon ( QIcon ( ":/images/key.png" ) );
	btnMakeRoom->setToolTip(QString("Make a room!"));
	btnMakeRoom->setObjectName("makeroombutton");
	btnSetStatus = new QPushButton;
	btnSetStatus->setIcon ( QIcon ( ":/images/status-default.png" ) );
	btnSetStatus->setToolTip(QString("Set your status"));
	btnSetStatus->setObjectName("setstatusbutton");
	btnFriends = new QPushButton;
	btnFriends->setIcon ( QIcon ( ":/images/users.png" ) );
	btnFriends->setToolTip(QString("Friends and bookmarks"));
	btnFriends->setObjectName("friendsbutton");
	btnReport = new QPushButton;
	btnReport->setIcon(QIcon(":/images/auction-hammer--exclamation.png"));
	btnReport->setToolTip(QString("Alert Staff!"));
	btnReport->setObjectName("alertstaffbutton");
	centralButtons->addWidget ( lblChannelName );
	centralButtons->addStretch();
	centralButtons->addWidget(btnSettings);
	centralButtons->addWidget(btnReport);
	centralButtons->addWidget(btnFriends);
	centralButtons->addWidget(btnSetStatus);
	centralButtons->addWidget(btnMakeRoom);
	centralButtons->addWidget(btnChannels);
	connect(btnSettings, SIGNAL(clicked()), this, SLOT(settingsDialogRequested()));
	connect(btnReport, SIGNAL(clicked()), this, SLOT(reportDialogRequested()));
	connect(btnFriends, SIGNAL ( clicked() ), this, SLOT ( friendsDialogRequested() ) );
	connect(btnChannels, SIGNAL ( clicked() ), this, SLOT ( channelsDialogRequested() ) );
	connect(btnMakeRoom, SIGNAL ( clicked() ), this, SLOT ( makeRoomDialogRequested() ) );
	connect(btnSetStatus, SIGNAL ( clicked() ), this, SLOT ( setStatusDialogRequested() ) );
	textEdit = new QTextBrowser;
	textEdit->setOpenLinks(false);
	textEdit->setObjectName ( "chatoutput" );
	textEdit->setContextMenuPolicy ( Qt::DefaultContextMenu );
	textEdit->setDocumentTitle ( "" );
	textEdit->setReadOnly ( true );
	textEdit->setFrameShape ( QFrame::NoFrame );
	FMessage::textField = textEdit;
	connect ( textEdit, SIGNAL ( anchorClicked ( QUrl ) ), this, SLOT ( anchorClicked ( QUrl ) ) );
	centralStuff->addWidget ( centralButtonsWidget );
	centralStuff->addWidget ( textEdit );
	horizontalLayout->addLayout ( centralStuff );
	listWidget = new QListWidget ( horizontalLayoutWidget );
	listWidget->setObjectName ( "userlist" );
	QSizePolicy sizePolicy1 ( QSizePolicy::Preferred, QSizePolicy::Expanding );
	sizePolicy1.setHorizontalStretch ( 0 );
	sizePolicy1.setVerticalStretch ( 0 );
	sizePolicy1.setHeightForWidth ( listWidget->sizePolicy().hasHeightForWidth() );
	listWidget->setSizePolicy ( sizePolicy1 );
	listWidget->setMinimumSize ( QSize ( 30, 0 ) );
	listWidget->setMaximumSize ( QSize ( 150, 16777215 ) );
	listWidget->setBaseSize ( QSize ( 100, 0 ) );
	listWidget->setContextMenuPolicy ( Qt::CustomContextMenu );
	listWidget->setIconSize ( QSize ( 16, 16 ) );
	connect ( listWidget, SIGNAL ( customContextMenuRequested ( const QPoint& ) ), this, SLOT ( userListContextMenuRequested ( const QPoint& ) ) );
	horizontalLayout->addWidget ( listWidget );
	verticalLayout->addWidget ( horizontalLayoutWidget );
	QWidget *textFieldWidget = new QWidget;
	textFieldWidget->setObjectName("inputarea");
	QHBoxLayout *inputLayout = new QHBoxLayout ( textFieldWidget );
	inputLayout->setObjectName("inputareaLayout");
	plainTextEdit = new QPlainTextEdit;
	plainTextEdit->setObjectName ( "chatinput" );
	QSizePolicy sizePolicy2 ( QSizePolicy::Expanding, QSizePolicy::Maximum );
	sizePolicy2.setHorizontalStretch ( 0 );
	sizePolicy2.setVerticalStretch ( 0 );
	sizePolicy2.setHeightForWidth ( plainTextEdit->sizePolicy().hasHeightForWidth() );
	plainTextEdit->setSizePolicy ( sizePolicy2 );
	plainTextEdit->setMaximumSize ( QSize ( 16777215, 75 ) );
	returnFilter = new UseReturn ( this );
	plainTextEdit->installEventFilter ( returnFilter );
	plainTextEdit->setFrameShape ( QFrame::NoFrame );
	connect ( plainTextEdit, SIGNAL ( textChanged() ), this, SLOT ( inputChanged() ) );
	QVBoxLayout *vblSouthButtons = new QVBoxLayout;
	btnSendChat = new QPushButton("Send Message");
	btnSendAdv = new QPushButton("Post RP ad");
	btnSendChat->setObjectName("sendmsgbutton");
	btnSendAdv->setObjectName("sendadvbutton");
	vblSouthButtons->addWidget(btnSendChat);
	vblSouthButtons->addWidget(btnSendAdv);
	QSizePolicy sizePolicy4(QSizePolicy::Minimum, QSizePolicy::Maximum);
	sizePolicy4.setHorizontalStretch(0);
	sizePolicy4.setVerticalStretch(0);
	connect(btnSendChat, SIGNAL(clicked()), this, SLOT(btnSendChatClicked()));
	connect(btnSendAdv, SIGNAL(clicked()), this, SLOT(btnSendAdvClicked()));

	inputLayout->addWidget(plainTextEdit);
	inputLayout->addLayout(vblSouthButtons);
	verticalLayout->addWidget ( textFieldWidget );
	setCentralWidget ( verticalLayoutWidget );
	menubar = new QMenuBar ( this );
	menubar->setObjectName ( "menubar" );
	menubar->setGeometry ( QRect ( 0, 0, 836, 23 ) );
	menuHelp = new QMenu ( menubar );
	menuHelp->setObjectName ( "menuHelp" );
	menuHelp->setTitle ( "Help" );
	menuFile = new QMenu ( menubar );
	menuFile->setObjectName ( "menuFile" );
	menuFile->setTitle ( "File" );
	setMenuBar ( menubar );
	menubar->addAction ( menuFile->menuAction() );
	menubar->addAction ( menuHelp->menuAction() );
	menuHelp->addAction ( actionHelp );
	menuHelp->addSeparator();
	menuHelp->addAction ( actionAbout );
	menuFile->addAction ( actionDisconnect );
	menuFile->addSeparator();
	menuFile->addAction ( actionQuit );
	connect(actionHelp, SIGNAL(triggered()), this, SLOT(helpDialogRequested()));
	connect ( actionAbout, SIGNAL ( triggered() ), this, SLOT ( aboutApp() ) );
	connect ( actionQuit, SIGNAL ( triggered() ), this, SLOT ( quitApp() ) );
	int wid = QApplication::desktop()->width();
	int hig = QApplication::desktop()->height();
	int mwid = width();
	int mhig = height();
	setGeometry ( ( wid / 2 ) - ( int ) ( mwid*0.5 ), ( hig / 2 ) - ( int ) ( mhig*0.5 ), mwid, mhig );
	setupConsole();
}
void flist_messenger::setupConsole()
{
	activePanelsSpacer = new QSpacerItem ( 0, 20, QSizePolicy::Ignored, QSizePolicy::Expanding );
	currentPanel = console;
	pushButton = new QPushButton();
	pushButton->setObjectName ( QString ( "CONSOLE" ) );
	pushButton->setGeometry ( QRect ( 0, 0, 30, 30 ) );
	pushButton->setFixedSize ( 30, 30 );
	pushButton->setIcon ( QIcon ( ":/images/terminal.png" ) );
	pushButton->setToolTip ( QString ( "View Console" ) );
	pushButton->setCheckable ( true );
	pushButton->setStyleSheet ( "background-color: rgb(255, 255, 255);" );
	pushButton->setChecked ( true );
	console->pushButton = pushButton;
	connect ( pushButton, SIGNAL ( clicked() ), this, SLOT ( channelButtonClicked() ) );
	activePanelsContents->addWidget ( pushButton, 0, Qt::AlignTop );
	activePanelsContents->addSpacerItem ( activePanelsSpacer );
	FMessage::console = console;
}
void flist_messenger::userListContextMenuRequested ( const QPoint& point )
{
	QListWidgetItem* lwi = listWidget->itemAt ( point );

	if ( lwi )
	{
		FCharacter* ch = characterList[lwi->text() ];
		ul_recent = ch;
		displayCharacterContextMenu ( ch );
	}
}
void flist_messenger::displayChannelContextMenu(FChannel *ch)
{
	if (!ch)
	{
		printDebugInfo("ERROR: Tried to display a context menu for a null pointer channel.");
	}
	else
	{
		QMenu* menu = new QMenu(this);
		recentChannelMenu = menu;
		menu->addAction(QIcon(":/images/cross.png"), QString("Close tab"), this, SLOT(tb_closeClicked()));
		menu->addAction(QIcon(":/images/terminal.png"), QString("Settings"), this, SLOT(tb_settingsClicked()));
		connect ( menu, SIGNAL ( aboutToHide() ), this, SLOT ( destroyChanMenu() ) );
		menu->exec ( QCursor::pos() );
	}
}
void flist_messenger::displayCharacterContextMenu ( FCharacter* ch )
{
	if ( !ch )
	{
		printDebugInfo("ERROR: Tried to display a context menu for a null pointer character.");
	}
	else
	{
		QMenu* menu = new QMenu ( this );
		recentCharMenu = menu;
		menu->addAction ( QIcon ( ":/images/users.png" ), QString ( "Private Message" ), this, SLOT ( ul_pmRequested() ) );
		menu->addAction ( QIcon ( ":/images/book-open-list.png" ), QString ( "F-list Profile"), this, SLOT ( ul_profileRequested() ) );
		menu->addAction ( QIcon ( ":/images/tag-label.png" ), QString ( "Quick Profile" ), this, SLOT ( ul_infoRequested() ) );
		if (selfIgnoreList.contains(ch->name()))
			menu->addAction ( QIcon ( ":/images/heart.png" ), QString ( "Unignore" ), this, SLOT(ul_ignoreRemove()) );
		else
			menu->addAction ( QIcon ( ":/images/heart-break.png" ), QString ( "Ignore" ), this, SLOT(ul_ignoreAdd()) );
		bool op = characterList[charName]->isChatOp();
		if (op)
		{
			menu->addAction ( QIcon ( ":/images/fire.png" ), QString ( "Chat Kick" ), this, SLOT(ul_chatKick()) );
			menu->addAction ( QIcon ( ":/images/auction-hammer--exclamation.png" ), QString ( "Chat Ban" ), this, SLOT(ul_chatBan()) );
			menu->addAction ( QIcon ( ":/images/alarm-clock.png" ), QString ( "Timeout..." ), this, SLOT(timeoutDialogRequested()) );
		}
		if (op || currentPanel->isOwner(characterList[charName]))
		{
			if (currentPanel->isOp(ch))
				menu->addAction ( QIcon ( ":/images/auction-hammer--minus.png" ), QString ( "Remove Channel OP" ), this, SLOT(ul_channelOpRemove()) );
			else
				menu->addAction ( QIcon ( ":/images/auction-hammer--plus.png" ), QString ( "Add Channel OP" ), this, SLOT(ul_channelOpAdd()) );
		}
		if ((op || currentPanel->isOp(characterList[charName])) && !ch->isChatOp())
		{
			menu->addAction ( QIcon ( ":/images/lightning.png" ), QString ( "Channel Kick" ), this, SLOT(ul_channelKick()) );
			menu->addAction ( QIcon ( ":/images/auction-hammer.png" ), QString ( "Channel Ban" ), this, SLOT(ul_channelBan()) );
		}
		connect ( menu, SIGNAL ( aboutToHide() ), this, SLOT ( destroyMenu() ) );
		menu->exec ( QCursor::pos() );
	}
}
void flist_messenger::anchorClicked ( QUrl link )
{
	QString ls = link.toString();
	/* Anchor commands:
  * AHI: Ad hoc invite. Join channel.
  * CSA: Confirm staff report.
  * LNK: Link. Should open the link in the system's browser.
  * USR: User. Open PM tab.
  */
	if ( ls.length() >= 5 )
	{
		QString cmd = ls.left(5);
		if (cmd == "#AHI-")
		{
			QString channel = ls.right ( ls.length() - 5 );
			joinChannel ( channel );
		}
		else if (cmd == "#CSA-")
		{
			// Confirm staff alert
			JSONNode node;
			JSONNode actionnode("action", "confirm");
			JSONNode modnode("moderator", charName.toStdString());
			JSONNode idnode("callid", ls.right(ls.length()-5).toStdString());
			node.push_back(actionnode);
			node.push_back(modnode);
			node.push_back(idnode);
			std::string output = "SFC " + node.write();
			sendWS(output);
		}
		else if (cmd == "#LNK-")
		{
			QUrl link(ls.right(ls.length()-5));
			QDesktopServices::openUrl(link);
		}
		else if (cmd == "#USR-")
		{
			QString flist = "http://www.f-list.net/c/";
			flist += ls.right(ls.length()-5);
			QUrl link(flist);
			QDesktopServices::openUrl(link);
		}
	}
	textEdit->verticalScrollBar()->setSliderPosition(textEdit->verticalScrollBar()->maximum());
}
void flist_messenger::insertLineBreak()
{
	if (textEdit)
		plainTextEdit->insertPlainText("\n");
}
void flist_messenger::refreshUserlist()
{
	if ( currentPanel == 0 )
		return;

	listWidget = this->findChild<QListWidget *> ( QString ( "userlist" ) );
	listWidget->clear();
	QList<FCharacter*> charList = currentPanel->charList();
	QListWidgetItem* charitem = 0;
	FCharacter* character;
	FCharacter::characterStatus status;
	foreach ( character, charList )
	{
		status = character->status();
		charitem = new QListWidgetItem ( character->name() );
		QIcon* i = character->statusIcon();
		charitem->setIcon ( *i );

		QFont f = charitem->font();

		if ( character->isChatOp() )
		{
			f.setBold ( true );
			f.setItalic ( true );
		}
		else if ( currentPanel->isOp ( character ) )
		{
			f.setBold ( true );
		}
		charitem->setFont ( f );
		charitem->setTextColor ( character->genderColor() );
		listWidget->addItem ( charitem );
	}

	if ( currentPanel->type() == FChannel::CHANTYPE_PM || currentPanel->type() == FChannel::CHANTYPE_CONSOLE )
	{
		listWidget->hide();
	}
	else
	{
		listWidget->show();
	}
}
void flist_messenger::settingsDialogRequested()
{
	if (settingsDialog == 0 || settingsDialog->parent() != this)
		setupSettingsDialog();

	se_chbHelpdesk->setChecked(se_helpdesk);
	se_chbEnablePing->setChecked(se_ping);
	se_chbLeaveJoin->setChecked(se_leaveJoin);
	se_chbAlwaysPing->setChecked(se_alwaysPing);
	se_chbMute->setChecked(!se_sounds);
	se_chbEnableChatLogs->setChecked(se_chatLogs);
	se_chbOnlineOffline->setChecked(se_onlineOffline);

	QString liststr = "";
	foreach (QString s, selfPingList)
	{
		liststr += s;
		liststr += ", ";
	}
	se_lePingList->setText(liststr.left(liststr.length()-2));
	settingsDialog->show();
}
void flist_messenger::setupSettingsDialog()
{
	if (settingsDialog)
		settingsDialog->deleteLater();

	settingsDialog = new QDialog(this);
	se_chbLeaveJoin = new QCheckBox(QString("Display leave/join notices"));
	se_chbOnlineOffline = new QCheckBox(QString("Display online/offline notices for friends"));
	se_chbEnableChatLogs = new QCheckBox(QString("Save chat logs"));
	se_chbMute = new QCheckBox(QString("Mute sounds"));
	se_chbAlwaysPing = new QCheckBox(QString("Always ping on PM/highlight"));
	se_chbEnablePing = new QCheckBox(QString("Highlight when your name is said, or one of the following words:"));
	se_lePingList = new QLineEdit;
	se_chbHelpdesk = new QCheckBox(QString("Display helpdesk notices (WIP)"));

	QTabWidget* twOverview = new QTabWidget;
	QGroupBox* gbGeneral = new QGroupBox(QString("General"));
	QGroupBox* gbNotifications = new QGroupBox(QString("Notifications"));
	QVBoxLayout* vblOverview = new QVBoxLayout;
	QVBoxLayout* vblGeneral = new QVBoxLayout;
	QVBoxLayout* vblNotifications = new QVBoxLayout;
	QHBoxLayout* hblButtons = new QHBoxLayout;
	QPushButton* btnSubmit = new QPushButton(QIcon(":/images/tick.png"), QString("Save settings"));
	QPushButton* btnCancel = new QPushButton(QIcon(":/images/cross.png"), QString("Cancel"));

	settingsDialog->setLayout(vblOverview);
	vblOverview->addWidget(twOverview);
	vblOverview->addLayout(hblButtons);
	hblButtons->addStretch();
	hblButtons->addWidget(btnSubmit);
	hblButtons->addWidget(btnCancel);

	twOverview->addTab(gbGeneral, QString("General"));
	gbGeneral->setLayout(vblGeneral);
	vblGeneral->addWidget(se_chbLeaveJoin);
	vblGeneral->addWidget(se_chbOnlineOffline);
	vblGeneral->addWidget(se_chbEnableChatLogs);
	vblGeneral->addWidget(se_chbHelpdesk);
	twOverview->addTab(gbNotifications, QString("Sounds"));
	gbNotifications->setLayout(vblNotifications);
	vblNotifications->addWidget(se_chbMute);
	vblNotifications->addWidget(se_chbEnablePing);
	vblNotifications->addWidget(se_lePingList);
	vblNotifications->addWidget(se_chbAlwaysPing);

	connect(btnSubmit, SIGNAL(clicked()), this, SLOT(se_btnSubmitClicked()));
	connect(btnCancel, SIGNAL(clicked()), this, SLOT(se_btnCancelClicked()));
}
void flist_messenger::friendsDialogRequested()
{
	if ( friendsDialog == 0 || friendsDialog->parent() != this )
		setupFriendsDialog();

	// Fill lists
	refreshFriendLists();
	friendsDialog->show();
}
void flist_messenger::refreshFriendLists()
{
	QString s;
	FCharacter* f = 0;
	QListWidgetItem* lwi = 0;
	fr_lwFriends->clear();

	foreach ( s, selfFriendsList )
	{
		if ( characterList.count ( s ) > 0 )
		{
			f = characterList[s];
			lwi = new QListWidgetItem ( * ( f->statusIcon() ), f->name() );
			addToFriendsList ( lwi );
		}
	}

	fr_lwIgnore->clear();

	foreach ( s, selfIgnoreList )
	{
		lwi = new QListWidgetItem ( s );
		addToIgnoreList ( lwi );
	}

}

void flist_messenger::makeRoomDialogRequested()
{
	if ( makeRoomDialog == 0 || makeRoomDialog->parent() != this )
		setupMakeRoomUI();

	makeRoomDialog->show();
}
void flist_messenger::setStatusDialogRequested()
{
	if ( setStatusDialog == 0 || setStatusDialog->parent() != this )
		setupSetStatusUI();

	ss_leMessage->setText ( selfStatusMessage );

	setStatusDialog->show();
}
void flist_messenger::characterInfoDialogRequested()
{
	//[19:48 PM]>>PRO {"character":"Hexxy"}
	//[19:41 PM]>>KIN {"character":"Cinnamon Flufftail"}
	FCharacter* ch = ul_recent;
	JSONNode outNode;
	JSONNode cn ( "character", ch->name().toStdString() );
	outNode.push_back ( cn );
	std::string out = "PRO " + outNode.write();
	sendWS ( out );
	out = "KIN " + outNode.write();
	sendWS ( out );

	if ( characterInfoDialog == 0 || characterInfoDialog->parent() != this )
		setupCharacterInfoUI();

	QString n = "<b>";
	n += ch->name();
	n += "</b> (";
	n += ch->statusString();
	n += ")";
	ci_lblName->setText ( n );
	ci_lblStatusMessage->setText ( ch->statusMsg() );
	characterInfoDialog->show();
}
void flist_messenger::reportDialogRequested()
{
	if (reportDialog == 0 || reportDialog->parent() != this)
		setupReportDialog();

	re_leWho->setText("None");
	re_teProblem->clear();
	reportDialog->show();

}
void flist_messenger::channelsDialogRequested()
{
	if ( channelsDialog == 0 || channelsDialog->parent() != this )
		setupChannelsUI();

	channelsDialog->show();
	// >>CHA
	std::string out = "CHA";
	sendWS ( out );
	out = "ORS";
	sendWS ( out );
}
void flist_messenger::refreshChatLines()
{
	if ( currentPanel == 0 )
		return;

	textEdit->clear();
	currentPanel->printChannel ( textEdit );
}
QPushButton* flist_messenger::addToActivePanels ( QString& channel, QString& tooltip )
{
	printDebugInfo("Joining " + channel.toStdString());
	pushButton = this->findChild<QPushButton *> ( channel );

	if ( pushButton != 0 )
	{
		pushButton->setVisible ( true );
	}
	else
	{
		activePanelsContents->removeItem ( activePanelsSpacer );
		pushButton = new QPushButton();
		pushButton->setObjectName ( channel );
		pushButton->setGeometry ( QRect ( 0, 0, 30, 30 ) );
		pushButton->setFixedSize ( 30, 30 );
		pushButton->setStyleSheet ( "background-color: rgb(255, 255, 255);" );
		pushButton->setAutoFillBackground ( true );
		pushButton->setCheckable ( true );
		pushButton->setChecked ( false );
		pushButton->setToolTip ( tooltip );
		pushButton->setContextMenuPolicy(Qt::CustomContextMenu);
		connect ( pushButton, SIGNAL ( clicked() ), this, SLOT ( channelButtonClicked() ) );
		connect ( pushButton, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(tb_channelRightClicked(QPoint)));
		activePanelsContents->addWidget ( pushButton, 0, Qt::AlignTop );

		if ( channel.length() > 4 && channel.toStdString().substr ( 0, 3 ) == "PM-" )
		{
			avatarFetcher.applyAvatarToButton ( pushButton, QString ( channel.toStdString().substr ( 3, channel.length() - 3 ).c_str() ) );
			//pushButton->setIconSize(pushButton->iconSize()*1.5);
		}
		else if ( channel.length() > 5 && channel.toStdString().substr ( 0, 4 ) == "ADH-" )
		{
			pushButton->setIcon ( QIcon ( ":/images/key.png" ) );
		}
		else
		{
			pushButton->setIcon ( QIcon ( ":/images/hash.png" ) );
		}

		activePanelsContents->addSpacerItem ( activePanelsSpacer );
	}

	return pushButton;
}
void flist_messenger::receivePM ( QString& message, QString& character )
{
	FChannel* pmPanel = 0;
	QString panelname = "PM-" + character;

	if ( channelList.count ( panelname ) == 0 )
	{
		channelList["PM-"+character] = new FChannel ( "PM-" + character, FChannel::CHANTYPE_PM );
		FCharacter* charptr = characterList[character];
		QString panelname = "PM-" + character;
		QString paneltitle = charptr->PMTitle();
		pmPanel = channelList["PM-"+character];
		pmPanel->setTitle ( paneltitle );
		pmPanel->setRecipient ( character );
		pmPanel->pushButton = addToActivePanels ( panelname, paneltitle );
	}
	pmPanel = channelList[panelname];
	if (pmPanel->getActive() == false)
	{
		pmPanel->setActive(true);
		pmPanel->pushButton->setVisible(true);
	}
	pmPanel->setTyping ( FChannel::TYPINGSTATUS_CLEAR );
	pmPanel->updateButtonColor();

	FMessage msg(FMessage::MESSAGETYPE_PRIVMESSAGE, pmPanel, characterList[character], message, currentPanel);
}
void flist_messenger::aboutApp()
{
	QMessageBox::about ( this, "About F-List Messenger", "Created by:\n* Viona\n* Kira\n* Aniko\n* Hexxy\n* Eager\n\nCopyright(c) 2010-2011 F-list Team" );
}
void flist_messenger::quitApp()
{
	QApplication::quit();
}
void flist_messenger::connectedToSocket()
{
	tcpSock->write ( WSConnect.c_str() );
	tcpSock->flush();
}
void flist_messenger::readReadyOnSocket()
{
	if ( doingWS )
	{
		QByteArray buffer = tcpSock->readAll();
		std::string buf ( buffer.begin(), buffer.end() );

		if ( buf.find ( "\r\n\r\n" ) != std::string::npos )
			doingWS = false;

		JSONNode loginnode;
		JSONNode tempnode ( "method", "ticket" );
		loginnode.push_back ( tempnode );
		tempnode.set_name ( "account" );
		tempnode = username.toStdString();
		loginnode.push_back ( tempnode );
		tempnode.set_name ( "character" );
		tempnode = charName.toStdString();
		loginnode.push_back ( tempnode );
		tempnode.set_name ( "ticket" );
		tempnode = loginTicket;//.toStdString();
		loginnode.push_back ( tempnode );
		tempnode.set_name ( "cname" );
		tempnode = CLIENTID;
		loginnode.push_back ( tempnode );
		tempnode.set_name ( "cversion" );
		tempnode = VERSIONNUM;
		loginnode.push_back ( tempnode );
		std::string idenStr = "IDN " + loginnode.write();
		sendWS ( idenStr );
	}
	else
	{
		QByteArray buffer = tcpSock->readAll();
		std::string buf ( networkBuffer );
		buf.append ( buffer.begin(), buffer.end() );
		int spos = buf.find ( ( char ) 0 );
		int epos = buf.find ( ( char ) 0xff );

		while ( spos != std::string::npos && epos != std::string::npos )
		{
			std::string cmd = buf.substr ( spos + 1, epos - ( spos + 1 ) );
			parseCommand ( cmd );
			spos = buf.find ( ( char ) 0, epos );
			epos = buf.find ( ( char ) 0xff, epos + 1 );
		}

		if ( spos != std::string::npos && epos == std::string::npos )
		{
			networkBuffer = buf.substr ( spos, buf.length() - spos );
		}
		else if ( networkBuffer.length() )
		{
			networkBuffer.clear();
		}
	}
}
void flist_messenger::socketError ( QAbstractSocket::SocketError socketError )
{
	QString sockErrorStr = tcpSock->errorString();
	if (btnConnect)
		btnConnect->setEnabled(true);
	if ( currentPanel )
	{
		QString input = "<b>Socket Error: </b>" + sockErrorStr;
		FMessage msg(FMessage::SYSTYPE_FEEDBACK, currentPanel, 0, input, currentPanel);
	}
	else
		QMessageBox::critical ( this, "Socket Error!", "Socket Error: " + sockErrorStr );

	disconnected = true;
	tcpSock->abort();
	tcpSock->deleteLater();
	tcpSock = 0;
}
void flist_messenger::sendWS ( std::string& input )
{
	if ( disconnected )
	{
		textEdit->append ( "Attempted to send a message, but client is disconnected." );
	}
	else
	{
		fix_broken_escaped_apos ( input );
		printDebugInfo( ">>" + input);
		QByteArray buf;
		QDataStream stream ( &buf, QIODevice::WriteOnly );
		input.resize ( input.length() );
		stream << ( quint8 ) 0;
		stream.writeRawData ( input.c_str(), input.length() );
		stream << ( quint8 ) 0xff;
		tcpSock->write ( buf );
		tcpSock->flush();
	}
}
bool flist_messenger::is_broken_escaped_apos ( std::string const &data, std::string::size_type n )
{
	return n + 2 <= data.size()
			and data[n] == '\\'
			and data[n+1] == '\'';
}
void flist_messenger::fix_broken_escaped_apos ( std::string &data )
{
	for ( std::string::size_type n = 0; n != data.size(); ++n )
	{
		if ( is_broken_escaped_apos ( data, n ) )
		{
			data.replace ( n, 2, 1, '\'' );
		}
	}
}

bool UseReturn::eventFilter ( QObject* obj, QEvent* event )
{
	if ( event->type() == QEvent::KeyPress )
	{
		QKeyEvent *keyEvent = static_cast<QKeyEvent *> ( event );
		if (keyEvent->modifiers() == Qt::ShiftModifier)
		{
			switch ( keyEvent->key() )
			{
			case Qt::Key_Enter:
			case Qt::Key_Return:
				static_cast<flist_messenger*>(parent())->insertLineBreak();
				return true;
				break;
			default:
				return QObject::eventFilter( obj, event);
			}
		}
		else
		{
			switch ( keyEvent->key() )
			{
			case Qt::Key_Enter:

			case Qt::Key_Return:
                static_cast<flist_messenger*> ( parent() )->enterPressed();
				return true;
				break;
			default:
				return QObject::eventFilter ( obj, event );
			}
		}
	}
	else
	{
		return QObject::eventFilter ( obj, event );
	}
}
bool ReturnLogin::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::KeyPress)
	{
		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
		switch(keyEvent->key())
		{
		case Qt::Key_Enter:
		case Qt::Key_Return:
			static_cast<flist_messenger*>(parent())->connectClicked();
		default:
			return QObject::eventFilter(obj, event);
		}
	} else {
		return QObject::eventFilter(obj, event);
	}
}
void flist_messenger::joinChannel ( QString& channel )
{
	JSONNode joinnode;
	JSONNode channode ( "channel", channel.toStdString() );
	joinnode.push_back ( channode );
	std::string msg = "JCH " + joinnode.write();

	sendWS ( msg );
}
void flist_messenger::leaveChannel ( std::string& channel, bool toServer )
{
	QString qchan = channel.c_str();

	channelList[qchan]->emptyCharList();
	channelList[qchan]->setActive ( false );
	channelList[qchan]->pushButton->setVisible ( false );
	currentPanel = console;

	if ( toServer )
	{
		JSONNode leavenode;
		JSONNode channode ( "channel", channel );
		leavenode.push_back ( channode );
		std::string msg = "LCH " + leavenode.write();
		sendWS ( msg );
	}
}
void flist_messenger::advertiseChannel(std::string &channel, std::string &message)
{
	JSONNode msgnode;
	JSONNode channode("channel", channel);
	JSONNode textnode("message", message);
	msgnode.push_back(channode);
	msgnode.push_back(textnode);
	std::string msg = "LRP " + msgnode.write();
	sendWS(msg);
}
void flist_messenger::messageChannel ( std::string& channel, std::string& message )
{
	JSONNode msgnode;
	JSONNode channode ( "channel", channel );
	JSONNode textnode ( "message", message );
	msgnode.push_back ( channode );
	msgnode.push_back ( textnode );
	std::string msg = "MSG " + msgnode.write();
	sendWS ( msg );
}
void flist_messenger::messagePrivate ( std::string& character, std::string& message )
{
	JSONNode msgnode;
	JSONNode targetnode ( "recipient", character );
	JSONNode textnode ( "message", message );
	msgnode.push_back ( targetnode );
	msgnode.push_back ( textnode );
	std::string msg = "PRI " + msgnode.write();
	sendWS ( msg );
}
void flist_messenger::sendIgnoreAdd (QString& character )
{
	character = character.toLower();
	JSONNode ignorenode;
	JSONNode targetnode ( "character", character.toStdString() );
	JSONNode actionnode ( "action", "add" );
	ignorenode.push_back ( targetnode );
	ignorenode.push_back ( actionnode );
	std::string msg = "IGN " + ignorenode.write();
	sendWS ( msg );
}
void flist_messenger::sendIgnoreDelete ( QString& character )
{
	JSONNode ignorenode;
	JSONNode targetnode ( "character", character.toStdString() );
	JSONNode actionnode ( "action", "delete" );
	ignorenode.push_back ( targetnode );
	ignorenode.push_back ( actionnode );
	std::string msg = "IGN " + ignorenode.write();
	sendWS ( msg );
}
void flist_messenger::changeStatus ( std::string& status, std::string& statusmsg )
{
	selfStatus = QString ( status.c_str() );
	selfStatusMessage = QString ( statusmsg.c_str() );
	JSONNode stanode;
	JSONNode statusnode ( "status", status );
	JSONNode stamsgnode ( "statusmsg", statusmsg );
	stanode.push_back ( statusnode );
	stanode.push_back ( stamsgnode );
	std::string msg = "STA " + stanode.write();
	sendWS ( msg );

	if ( selfStatus == "online" )
		btnSetStatus->setIcon ( QIcon ( ":/images/status-default.png" ) );
	else if ( selfStatus == "busy" )
		btnSetStatus->setIcon ( QIcon ( ":/images/status-away.png" ) );
	else if ( selfStatus == "dnd" )
		btnSetStatus->setIcon ( QIcon ( ":/images/status-busy.png" ) );
	else if ( selfStatus == "looking" )
		btnSetStatus->setIcon ( QIcon ( ":/images/status.png" ) );
	else if ( selfStatus == "away" )
		btnSetStatus->setIcon ( QIcon ( ":/images/status-blue.png" ) );

	QString output = QString ( "Status changed successfully." );

	FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, currentPanel, 0, output, currentPanel);

}
void flist_messenger::fr_btnCloseClicked()
{
	friendsDialog->close();
}
void flist_messenger::fr_btnFriendsPMClicked()
{
	QListWidgetItem* lwi = fr_lwFriends->selectedItems().at ( 0 );

	if (lwi)
	{
		QString name = lwi->text();
		openPMTab ( name );
	}
}
void flist_messenger::fr_btnIgnoreRemoveClicked()
{
	QListWidgetItem* lwi = fr_lwIgnore->selectedItems().at ( 0 );

	if ( lwi )
	{
		QString name = lwi->text();
		sendIgnoreDelete(name);
	}
}
void flist_messenger::fr_btnIgnoreAddClicked()
{
	addIgnoreDialogRequested();
}
void flist_messenger::fr_friendsContextMenuRequested ( const QPoint& point )
{
	QListWidgetItem* lwi = fr_lwFriends->itemAt ( point );

	if ( lwi && characterList.count(lwi->text()))
	{
		FCharacter* ch = characterList[lwi->text() ];
		ul_recent = ch;
		displayCharacterContextMenu ( ch );
	}
}
void flist_messenger::tb_channelRightClicked ( const QPoint & point )
{
	QObject* sender = this->sender();
	QPushButton* button = qobject_cast<QPushButton*> ( sender );
	if (button) {
		tb_recent = channelList[button->objectName()];
		std::cout << tb_recent->title().toStdString() << std::endl;
		channelButtonMenuRequested();
	}
}
void flist_messenger::channelButtonMenuRequested()
{
	displayChannelContextMenu(tb_recent);
}
void flist_messenger::channelButtonClicked()
{
	QObject* sender = this->sender();
	QPushButton* button = qobject_cast<QPushButton*> ( sender );

	if ( button )
	{
		QString tab = button->objectName();
		switchTab ( tab );
	}
}
void flist_messenger::usersCommand()
{
	QString msg;
	msg.sprintf("<b>%d users online.</b>", characterList.size());
	FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, currentPanel, 0, msg, currentPanel);
}
void flist_messenger::inputChanged()
{
	if ( currentPanel && currentPanel->type() == FChannel::CHANTYPE_PM )
	{
		if ( plainTextEdit->toPlainText().simplified() == "" )
		{
			typingCleared ( currentPanel );
			currentPanel->setTypingSelf ( FChannel::TYPINGSTATUS_CLEAR );
		}
		else
		{
			if ( currentPanel->getTypingSelf() != FChannel::TYPINGSTATUS_TYPING )
			{
				typingContinued ( currentPanel );
				currentPanel->setTypingSelf ( FChannel::TYPINGSTATUS_TYPING );
			}
		}
	}
}
void flist_messenger::typingCleared ( FChannel* channel )
{
	// TPN {"character":"Leon Priest","status":"clear"}
	std::string character = channel->recipient().toStdString().c_str();
	std::string status = "clear";

	JSONNode node;
	JSONNode statusnode ( "status", status );
	JSONNode charnode ( "character", character );
	node.push_back ( statusnode );
	node.push_back ( charnode );
	std::string msg = "TPN " + node.write();
	sendWS ( msg );
}
void flist_messenger::typingContinued ( FChannel* channel )
{
	// TPN {"character":"Leon Priest","status":"typing"}
	std::string character = channel->recipient().toStdString().c_str();
	std::string status = "typing";

	JSONNode node;
	JSONNode statusnode ( "status", status );
	JSONNode charnode ( "character", character );
	node.push_back ( statusnode );
	node.push_back ( charnode );
	std::string msg = "TPN " + node.write();
	sendWS ( msg );
}
void flist_messenger::typingPaused ( FChannel* channel )
{
	// TPN {"character":"Leon Priest","status":"paused"}
	std::string character = channel->recipient().toStdString().c_str();
	std::string status = "paused";

	JSONNode node;
	JSONNode statusnode ( "status", status );
	JSONNode charnode ( "character", character );
	node.push_back ( statusnode );
	node.push_back ( charnode );
	std::string msg = "TPN " + node.write();
	sendWS ( msg );
}
void flist_messenger::ul_pmRequested()
{
	openPMTab();
}
void flist_messenger::ul_infoRequested()
{
	characterInfoDialogRequested();
}
void flist_messenger::ul_ignoreAdd()
{
	FCharacter* c = ul_recent;
	QString name = c->name();
	if (selfIgnoreList.contains(name))
	{
		printDebugInfo("[CLIENT BUG] Tried to ignore somebody who is already on the ignorelist.");
	} else {
		sendIgnoreAdd(name);
	}
}
void flist_messenger::ul_ignoreRemove()
{
	FCharacter* c = ul_recent;
	QString name = c->name();
	if (!selfIgnoreList.contains(name))
	{
		printDebugInfo("[CLIENT BUG] Tried to unignore somebody who is not on the ignorelist.");
	} else {
		sendIgnoreDelete(name);
	}
}
void flist_messenger::ul_channelBan()
{
	FCharacter* ch = ul_recent;

	JSONNode kicknode;
	JSONNode charnode ( "character", ch->name().toStdString() );
	kicknode.push_back ( charnode );
	JSONNode channode ( "channel", currentPanel->name().toStdString() );
	kicknode.push_back ( channode );
	std::string out = "CBU " + kicknode.write();
	sendWS ( out );
}
void flist_messenger::ul_channelKick()
{
	FCharacter* ch = ul_recent;
	JSONNode kicknode;
	JSONNode charnode ( "character", ch->name().toStdString() );
	kicknode.push_back ( charnode );
	JSONNode channode ( "channel", currentPanel->name().toStdString() );
	kicknode.push_back ( channode );
	std::string out = "CKU " + kicknode.write();
	sendWS ( out );
}
void flist_messenger::ul_chatBan()
{
	FCharacter* ch = ul_recent;
	JSONNode node;
	JSONNode charnode ( "character", ch->name().toStdString() );
	node.push_back ( charnode );
	std::string out = "ACB " + node.write();
	sendWS ( out );
}
void flist_messenger::ul_chatKick()
{
	FCharacter* ch = ul_recent;
	JSONNode kicknode;
	JSONNode charnode ( "character", ch->name().toStdString() );
	kicknode.push_back ( charnode );
	std::string out = "KIK " + kicknode.write();
	sendWS ( out );
}
void flist_messenger::ul_chatTimeout()
{
	timeoutDialogRequested();
}
void flist_messenger::ul_channelOpAdd()
{
	FCharacter* ch = ul_recent;
	JSONNode opnode;
	JSONNode charnode ( "character", ch->name().toStdString() );
	opnode.push_back ( charnode );
	JSONNode channode ( "channel", currentPanel->name().toStdString() );
	opnode.push_back ( channode );
	std::string out = "COA " + opnode.write();
	sendWS ( out );
}
void flist_messenger::ul_channelOpRemove()
{
	FCharacter* ch = ul_recent;
	JSONNode opnode;
	JSONNode charnode ( "character", ch->name().toStdString() );
	opnode.push_back ( charnode );
	JSONNode channode ( "channel", currentPanel->name().toStdString() );
	opnode.push_back ( channode );
	std::string out = "COR " + opnode.write();
	sendWS ( out );
}
void flist_messenger::ul_chatOpAdd()
{
	FCharacter* ch = ul_recent;
	std::string character = ch->name().toStdString();
	JSONNode opnode;
	JSONNode charnode ( "character", character );
	opnode.push_back ( charnode );
	std::string out = "AOP " + opnode.write();
	sendWS ( out );
}
void flist_messenger::ul_chatOpRemove()
{
	FCharacter* ch = ul_recent;
	std::string character = ch->name().toStdString();
	JSONNode opnode;
	JSONNode charnode ( "character", character );
	opnode.push_back ( charnode );
	std::string out = "DOP " + opnode.write();
	sendWS ( out );
}
void flist_messenger::ul_profileRequested()
{
	FCharacter* ch = ul_recent;
	QString l = "http://www.f-list.net/c/";
	l += ch->name();
	QUrl link(l);
	QDesktopServices::openUrl(link);
}

void flist_messenger::setupTimeoutDialog()
{
	QVBoxLayout* to_vblOverview;
	QHBoxLayout* to_hblButtons;
	QLabel* to_lblWho;
	QLabel* to_lblLength;
	QLabel* to_lblReason;
	QPushButton* to_btnSubmit;
	QPushButton* to_btnCancel;
	timeoutDialog = new QDialog(this);
	to_lblWho = new QLabel("Who?");
	to_lblLength = new QLabel("How long? (Minutes, up to 90)");
	to_lblReason = new QLabel("Why?");
	to_leWho = new QLineEdit;
	to_leLength = new QLineEdit;
	to_leReason = new QLineEdit;
	to_vblOverview = new QVBoxLayout;
	to_hblButtons = new QHBoxLayout;
	to_btnSubmit = new QPushButton(QIcon(QString(":/images/tick.png")), QString("Submit"));
	to_btnCancel = new QPushButton(QIcon(QString(":/images/cross.png")), QString("Cancel"));

	timeoutDialog->setLayout(to_vblOverview);
	to_vblOverview->addWidget(to_lblWho);
	to_vblOverview->addWidget(to_leWho);
	to_vblOverview->addWidget(to_lblLength);
	to_vblOverview->addWidget(to_leLength);
	to_vblOverview->addWidget(to_lblReason);
	to_vblOverview->addWidget(to_leReason);
	to_vblOverview->addLayout(to_hblButtons);
	to_hblButtons->addStretch();
	to_hblButtons->addWidget(to_btnSubmit);
	to_hblButtons->addWidget(to_btnCancel);
	connect(to_btnSubmit, SIGNAL(clicked()), this, SLOT(to_btnSubmitClicked()));
	connect(to_btnCancel, SIGNAL(clicked()), this, SLOT(to_btnCancelClicked()));
}
void flist_messenger::timeoutDialogRequested()
{
	if (timeoutDialog == 0 || timeoutDialog->parent() != this)
		setupTimeoutDialog();
	to_leWho->setText(ul_recent->name());
	timeoutDialog->show();
}
void flist_messenger::to_btnSubmitClicked()
{
	QString who = to_leWho->text();
	QString length = to_leLength->text();
	QString why = to_leReason->text();
	bool *ok = new bool(true);
	int minutes = length.toInt(ok);

	if (! *ok || minutes <= 0 || 90 < minutes) {
		QString error("Wrong length.");
		FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, currentPanel, 0, error, currentPanel);
	}
	else if (who.simplified() == "" || why.simplified() == "")
	{
		QString error("Didn't fill out all fields.");
		FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, currentPanel, 0, error, currentPanel);
	} else {
		std::string character = who.simplified().toStdString();
		std::string reason = why.simplified().toStdString();
		JSONNode node;
		JSONNode charnode ( "character", character );
		node.push_back ( charnode );
		JSONNode timenode ( "time", length.toStdString() );
		node.push_back ( timenode );
		JSONNode renode ( "reason", reason );
		node.push_back ( renode );
		std::string out = "TMO " + node.write();
		sendWS ( out );
	}
	delete ok;
	timeoutDialog->hide();
}
void flist_messenger::to_btnCancelClicked()
{
	timeoutDialog->hide();
}
void flist_messenger::switchTab ( QString& tabname )
{
	if ( channelList.count ( tabname ) == 0 && tabname != "CONSOLE" )
	{
		printDebugInfo( "ERROR: Tried to switch to " + tabname.toStdString() + " but it doesn't exist.");
		return;
	}

	QString input = plainTextEdit->toPlainText();

	if ( currentPanel && currentPanel->type() == FChannel::CHANTYPE_PM && currentPanel->getTypingSelf() == FChannel::TYPINGSTATUS_TYPING )
	{
		typingPaused ( currentPanel );
	}

	currentPanel->setInput ( input );

	currentPanel->pushButton->setChecked ( false );
	FChannel* chan = 0;

	if ( tabname == "CONSOLE" )
		chan = console;
	else
		chan = channelList.value ( tabname );

	currentPanel = chan;
	lblChannelName->setText ( chan->title() );
	input = currentPanel->getInput();
	plainTextEdit->setPlainText ( input );
	plainTextEdit->setFocus();
	currentPanel->setHighlighted ( false );
	currentPanel->setHasNewMessages ( false );
	currentPanel->updateButtonColor();
	currentPanel->pushButton->setChecked ( true );
	refreshUserlist();
	refreshChatLines();
	textEdit->verticalScrollBar()->setSliderPosition(textEdit->verticalScrollBar()->maximum());
	if (currentPanel->getMode() == FChannel::CHANMODE_CHAT) // disable ads
    {
		btnSendAdv->setDisabled(true);
        btnSendChat->setDisabled(false);
    }
    else if (currentPanel->getMode() == FChannel::CHANMODE_ADS)
    {
        btnSendAdv->setDisabled(false);
        btnSendChat->setDisabled(true);
    }
    else if (currentPanel->type() == FChannel::CHANTYPE_PM || currentPanel->type() == FChannel::CHANTYPE_CONSOLE ) // Disable ads
    {
		btnSendAdv->setDisabled(true);
        btnSendChat->setDisabled(false);
    }
    else
    {
        btnSendAdv->setDisabled(false);
        btnSendChat->setDisabled(false);
    }
}
void flist_messenger::openPMTab()
{
	QString ch = ul_recent->name();
	openPMTab ( ch );
}
void flist_messenger::openPMTab ( QString &character )
{
	if (character.toLower() == charName.toLower())
	{
		QString msg = "You can't PM yourself!";
		FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, currentPanel, 0, msg, currentPanel);
		return;
	}
	if ( characterList.count ( character ) == 0 )
	{
		QString msg = "That character is not logged in.";
		FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, currentPanel, 0, msg, currentPanel);
		return;
	}

	QString panelname = "PM-" + character;

	if ( channelList.count ( panelname ) != 0 )
	{
		channelList[panelname]->setActive(true);
		channelList[panelname]->pushButton->setVisible(true);
		switchTab ( panelname );
	}
	else
	{
		channelList["PM-"+character] = new FChannel ( "PM-" + character, FChannel::CHANTYPE_PM );
		FCharacter* charptr = characterList[character];
		QString panelname = "PM-" + character;
		QString paneltitle = charptr->PMTitle();
		FChannel* pmPanel = channelList["PM-"+character];
		pmPanel->setTitle ( paneltitle );
		pmPanel->setRecipient ( character );
		pmPanel->pushButton = addToActivePanels ( panelname, paneltitle );
		plainTextEdit->clear();
		switchTab ( panelname );
	}
}
void flist_messenger::btnSendChatClicked()
{
	// SLOT
	parseInput();
}
void flist_messenger::btnSendAdvClicked()
{
	if (se_sounds)
		soundPlayer.play ( soundPlayer.SOUND_CHAT );

	QPlainTextEdit *messagebox = this->findChild<QPlainTextEdit *> ( QString ( "chatinput" ) );
	QString inputText = QString ( messagebox->toPlainText() );
	QString gt = "&gt;";
	QString lt = "&lt;";
	QString amp = "&amp;";
	QString ownText = inputText;
	ownText.replace ( '&', amp ).replace ( '<', lt ).replace ( '>', gt );
	QString msg;
	if ( inputText.length() == 0 )
	{
		msg = "<b>Error:</b> No message.";
		FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, currentPanel, 0, msg, currentPanel);
		return;
	}
	if ( currentPanel == 0 )
    {
		printDebugInfo("[CLIENT ERROR] currentPanel == 0");
		return;
	}
	if ( currentPanel == console || currentPanel->getMode() == FChannel::CHANMODE_CHAT || currentPanel->type() == FChannel::CHANTYPE_PM )
	{
		msg = "<b>Error:</b> Can't advertise here.";
		FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, currentPanel, 0, msg, currentPanel);		return;
	}
	if ( inputText.length() > flist_messenger::BUFFERPUB )
	{
		msg = "<B>Error:</B> Message exceeds the maximum number of characters.";
		FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, currentPanel, 0, msg, currentPanel);
		return;
	}

	std::string chan = currentPanel->name().toStdString();
	std::string message = inputText.toStdString();
	bool isOp = false;
	QString genderColor;
	if ( characterList.count ( charName ) != 0 )
	{
		FCharacter* chanchar = characterList[charName];
		genderColor = chanchar->genderColor().name();
		isOp = ( chanchar->isChatOp() || currentPanel->isOp( chanchar ) || currentPanel->isOwner( chanchar ) );
	}
	msg = "<font color=\"green\"><b>Roleplay ad by</font> <font color=\"";
	msg+= genderColor;
	msg+= "\">";
	msg+= charName;
	msg+= "</b></font>: ";
	msg+= ownText;
	FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, currentPanel, 0, msg, currentPanel);

	plainTextEdit->clear();
	advertiseChannel ( chan, message );
}
void flist_messenger::parseInput()
{
	if (se_sounds)
		soundPlayer.play ( soundPlayer.SOUND_CHAT );

	bool pm = ( bool ) ( currentPanel->type() == FChannel::CHANTYPE_PM );
	QPlainTextEdit *messagebox = this->findChild<QPlainTextEdit *> ( QString ( "chatinput" ) );
	QString inputText = QString ( messagebox->toPlainText() );

	bool isCommand = ( inputText[0] == '/' );

	if ( !isCommand && currentPanel == console )
		return;

	bool success = false;
	QString msg = 0;
	QString gt = "&gt;";
	QString lt = "&lt;";
	QString amp = "&amp;";
	QString ownText = inputText;
	ownText.replace ( '&', amp ).replace ( '<', lt ).replace ( '>', gt );
	if ( inputText.length() == 0 )
	{
		msg = "<b>Error:</b> No message.";
		FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, currentPanel, 0, msg, currentPanel);
		return;
	}
	if ( currentPanel == 0 )
	{
		printDebugInfo("[CLIENT ERROR] currentPanel == 0");
		return;
	}
	int buffer;
	if ( pm )
		buffer = flist_messenger::BUFFERPRIV;
	else
		buffer = flist_messenger::BUFFERPUB;

	if ( inputText.length() > buffer )
	{
		msg = "<B>Error:</B> Message exceeds the maximum number of characters.";
		FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, currentPanel, 0, msg, currentPanel);
		return;
	}

	if ( isCommand )
	{
		QStringList parts = inputText.split ( ' ' );

		if ( parts[0].toLower() == "/clear" )
		{
			textEdit->clear();
			success = true;
		}
		else if ( parts[0].toLower() == "/debug" )
		{
			JSONNode root;
			JSONNode temp;
			temp.set_name("command");
			temp = parts[1].toStdString().c_str();
			root.push_back(temp);
			std::string debug = "ZZZ ";
			debug += root.write().c_str();
			sendWS( debug );
			success = true;
		}
		else if ( parts[0].toLower() == "/me" )
		{
			msg = "<i>*<b>" + charName + "</b> " + ownText.mid ( 4 ).simplified() + "</i>";
			success = true;
		}
		else if ( parts[0].toLower() == "/me's" )
		{
			msg = "<i>*<b>" + charName + "'s</b> " + ownText.mid( 4 ).simplified() + "</i>";
			success = true;
		}
		else if ( parts[0].toLower() == "/join" )
		{
			QString channel = inputText.mid ( 6, -1 ).simplified();
			joinChannel ( channel );
			success = true;
		}
		else if ( parts[0].toLower() == "/leave" )
		{
			if ( currentPanel == console || currentPanel->type() == FChannel::CHANTYPE_PM )
			{
				success = false;
			}
			else
			{
				std::string channel = currentPanel->name().toStdString();
				leaveChannel ( channel );
				success = true;
			}
		}
		else if ( parts[0].toLower() == "/status" )
		{
			QString status = parts[1];
			QString statusMsg = inputText.mid ( 9 + status.length(), -1 ).simplified();
			std::string stdstat = status.toStdString();
			std::string stdmsg = statusMsg.toStdString();
			changeStatus ( stdstat, stdmsg );
			success = true;
		}
		else if ( parts[0].toLower() == "/users" )
		{
			usersCommand();
			success = true;
		}
		else if ( parts[0].toLower() == "/priv" )
		{
			QString character = inputText.mid ( 6 ).simplified();
			openPMTab ( character );
			success = true;
		}
		else if ( parts[0].toLower() == "/ignore" )
		{
			QString character = inputText.mid ( 8 ).simplified();

			if ( character != "" )
			{
				sendIgnoreAdd(character);
				success = true;
			}
		}
		else if ( parts[0].toLower() == "/unignore" )
		{
			QString character = inputText.mid ( 10 ).simplified();

			if ( character != "" )
			{
				if ( selfIgnoreList.count ( character ) == 0 )
				{
					QString out = QString ( "This character is not in your ignore list." );
					FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, currentPanel, 0, out, currentPanel);
				}
				else
				{
					sendIgnoreDelete(character);
					success = true;
				}
			}
		}
		else if ( parts[0].toLower() == "/channels" || parts[0].toLower() == "/prooms" )
		{
			channelsDialogRequested();
			success = true;
		}
		else if ( parts[0].toLower() == "/kick" )
		{
			//[16:29 PM]>>CKU {"channel":"ADH-STAFFROOMFORSTAFFPPL","character":"Viona"}
			JSONNode kicknode;
			JSONNode charnode ( "character", inputText.mid ( 6 ).simplified().toStdString() );
			kicknode.push_back ( charnode );
			JSONNode channode ( "channel", currentPanel->name().toStdString() );
			kicknode.push_back ( channode );
			std::string out = "CKU " + kicknode.write();
			sendWS ( out );
			success = true;
		}
		else if ( parts[0].toLower() == "/gkick" )
		{
			// [16:22 PM]>>KIK {"character":"Tamu"}
			JSONNode kicknode;
			JSONNode charnode ( "character", inputText.mid ( 7 ).simplified().toStdString() );
			kicknode.push_back ( charnode );
			std::string out = "KIK " + kicknode.write();
			sendWS ( out );
			success = true;
		}
		else if ( parts[0].toLower() == "/ban" )
		{
			//[17:23 PM]>>CBU {"channel":"ADH-89ff2273b20cfc422ca1","character":"Viona"}
			JSONNode kicknode;
			JSONNode charnode ( "character", inputText.mid ( 5 ).simplified().toStdString() );
			kicknode.push_back ( charnode );
			JSONNode channode ( "channel", currentPanel->name().toStdString() );
			kicknode.push_back ( channode );
			std::string out = "CBU " + kicknode.write();
			sendWS ( out );
			success = true;
		}
		else if ( parts[0].toLower() == "/accountban" )
		{
			//[22:42 PM]>>ACB {"character":"Mack"}
			JSONNode node;
			JSONNode charnode ( "character", inputText.mid ( 12 ).simplified().toStdString() );
			node.push_back ( charnode );
			std::string out = "ACB " + node.write();
			sendWS ( out );
			success = true;
		}
		else if ( parts[0].toLower() == "/makeroom" )
		{
			// [17:24 PM]>>CCR {"channel":"abc"}
			JSONNode makenode;
			JSONNode namenode ( "channel", inputText.mid ( 10 ).simplified().toStdString() );
			makenode.push_back ( namenode );
			std::string out = "CCR " + makenode.write();
			sendWS ( out );
			success = true;
		}
		else if ( parts[0].toLower() == "/closeroom")
		{
			// [13:12 PM]>>RST {"channel":"ADH-68c2 7 1 4e731ccfbe0","status":"public"}
			JSONNode statusnode;
			JSONNode channelnode("channel", currentPanel->name().toStdString());
			JSONNode statnode("status", "private");
			statusnode.push_back(channelnode);
			statusnode.push_back(statnode);
			std::string out = "RST " + statusnode.write();
			sendWS( out );
			success = true;
		}
		else if ( parts[0].toLower() == "/openroom")
		{
			// [13:12 PM]>>RST {"channel":"ADH-68c2 7 1 4e731ccfbe0","status":"private"}
			JSONNode statusnode;
			JSONNode channelnode("channel", currentPanel->name().toStdString());
			JSONNode statnode("status", "public");
			statusnode.push_back(channelnode);
			statusnode.push_back(statnode);
			std::string out = "RST " + statusnode.write();
			sendWS( out );
			success = true;
		}
		else if ( parts[0].toLower() == "/invite" )
		{
			//[16:37 PM]>>CIU {"channel":"ADH-STAFFROOMFORSTAFFPPL","character":"Viona"}
			JSONNode invitenode;
			JSONNode charnode ( "character", inputText.mid ( 8 ).simplified().toStdString() );
			invitenode.push_back ( charnode );
			JSONNode channode ( "channel", currentPanel->name().toStdString() );
			invitenode.push_back ( channode );
			std::string out = "CIU " + invitenode.write();
			sendWS ( out );
			success = true;
		}
		else if ( parts[0].toLower() == "/warn" )
		{
			msg = "<b>" + charName + "</b>: <span id=\"warning\">" + ownText.mid ( 6 ) + "</span>";
			success = true;
		}
		else if ( parts[0].toLower() == "/cop" )
		{
			//COA {"channel":"","character":""}
			JSONNode opnode;
			JSONNode charnode ( "character", inputText.mid ( 5 ).simplified().toStdString() );
			opnode.push_back ( charnode );
			JSONNode channode ( "channel", currentPanel->name().toStdString() );
			opnode.push_back ( channode );
			std::string out = "COA " + opnode.write();
			sendWS ( out );
			success = true;
		}
		else if ( parts[0].toLower() == "/cdeop" )
		{
			//[16:27 PM]>>COR {"channel":"ADH-STAFFROOMFORSTAFFPPL","character":"Viona"}
			JSONNode opnode;
			JSONNode charnode ( "character", inputText.mid ( 7 ).simplified().toStdString() );
			opnode.push_back ( charnode );
			JSONNode channode ( "channel", currentPanel->name().toStdString() );
			opnode.push_back ( channode );
			std::string out = "COR " + opnode.write();
			sendWS ( out );
			success = true;
		}
		else if ( parts[0].toLower() == "/op" )
		{
			std::string character = inputText.mid ( 4 ).simplified().toStdString();
			JSONNode opnode;
			JSONNode charnode ( "character", character );
			opnode.push_back ( charnode );
			std::string out = "AOP " + opnode.write();
			sendWS ( out );
			success = true;
		}
		else if ( parts[0].toLower() == "/reward" )
		{
			// [17:19 PM]>>RWD {"character":"Arisato Hamuko"}
			JSONNode node;
			JSONNode charnode ( "character", inputText.mid ( 8 ).simplified().toStdString() );
			node.push_back ( charnode );
			std::string out = "RWD " + node.write();
			sendWS ( out );
			success = true;
		}
		else if ( parts[0].toLower() == "/deop" )
		{
			// [17:27 PM]>>DOP {"character":"Viona"}
			JSONNode node;
			JSONNode charnode ( "character", inputText.mid ( 6 ).simplified().toStdString() );
			node.push_back ( charnode );
			std::string out = "DOP " + node.write();
			sendWS ( out );
			success = true;
		}
		else if ( parts[0].toLower() == "/code")
		{
			QString out = "";
			switch(currentPanel->type())
			{
			case FChannel::CHANTYPE_NORMAL:
				out = "Copy this code to your message: [b][noparse][channel]" + currentPanel->name() + "[/channel][/noparse][/b]";
				break;
			case FChannel::CHANTYPE_ADHOC:
				out = "Copy this code to your message: [b][noparse][session=" + currentPanel->title() + "]" + currentPanel->name() + "[/session][/noparse][/b]";
				break;
			default:
				out = "This command is only for channels!";
				break;
			}
			FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, currentPanel, 0, out, currentPanel);
			success = true;
		}
		else if ( parts[0].toLower() == "/unban" )
		{
			// [17:30 PM]>>CUB {"channel":"ADH-cbae3bdf02cd39e8949e","character":"Viona"}
			JSONNode node;
			JSONNode charnode ( "character", inputText.mid ( 7 ).simplified().toStdString() );
			node.push_back ( charnode );
			JSONNode channode ( "channel", currentPanel->name().toStdString() );
			node.push_back ( channode );
			std::string out = "CUB " + node.write();
			sendWS ( out );
			success = true;
		}
		else if ( parts[0].toLower() == "/banlist" )
		{
			// [17:30 PM]>>CBL {"channel":"ADH-cbae3bdf02cd39e8949e"}
			JSONNode node;
			JSONNode channode ( "channel", currentPanel->name().toStdString() );
			node.push_back ( channode );
			std::string out = "CBL " + node.write();
			sendWS ( out );
			success = true;
		}
		else if ( parts[0].toLower() == "/setdescription" )
		{
			// [17:31 PM]>>CDS {"channel":"ADH-cbae3bdf02cd39e8949e","description":":3!"}
			JSONNode node;
			JSONNode channode ( "channel", currentPanel->name().toStdString() );
			node.push_back ( channode );
			JSONNode descnode ( "description", inputText.mid ( 16 ).simplified().toStdString() );
			node.push_back ( descnode );
			std::string out = "CDS " + node.write();
			sendWS ( out );
			success = true;
		}
		else if ( parts[0].toLower() == "/coplist" )
		{
			// [17:31 PM]>>COL {"channel":"ADH-cbae3bdf02cd39e8949e"}
			JSONNode node;
			JSONNode channode ( "channel", currentPanel->name().toStdString() );
			node.push_back ( channode );
			std::string out = "COL " + node.write();
			sendWS ( out );
			success = true;
		}
		else if ( parts[0].toLower() == "/timeout" )
		{
			// [17:16 PM]>>TMO {"time":1,"character":"Arisato Hamuko","reason":"Test."}
			QStringList tparts = inputText.mid ( 9 ).split ( ',' );
			bool isInt;
			int time = tparts[1].simplified().toInt ( &isInt );

			if ( isInt == false )
			{
				QString err = "Time is not a number.";
				FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, currentPanel, 0, err, currentPanel);
			}
			else
			{
				std::string character = tparts[0].simplified().toStdString();
				std::string reason = tparts[2].simplified().toStdString();
				JSONNode node;
				JSONNode charnode ( "character", character );
				node.push_back ( charnode );
				JSONNode timenode ( "time", time );
				node.push_back ( timenode );
				JSONNode renode ( "reason", reason );
				node.push_back ( renode );
				std::string out = "TMO " + node.write();
				sendWS ( out );
				success = true;
			}
		}
		else if ( parts[0].toLower() == "/gunban" )
		{
			// [22:43 PM]>>UNB {"character":"Mack"}
			JSONNode node;
			JSONNode charnode ( "character", inputText.mid ( 8 ).simplified().toStdString() );
			node.push_back ( charnode );
			std::string out = "UNB " + node.write();
			sendWS ( out );
			success = true;
		}
		else if ( parts[0].toLower() == "/createchannel" )
		{
			// [0:59 AM]>>CRC {"channel":"test"}
			JSONNode node;
			JSONNode channode ( "channel", inputText.mid ( 15 ).simplified().toStdString() );
			node.push_back ( channode );
			std::string out = "CRC " + node.write();
			sendWS ( out );
			success = true;
		}
		else if ( parts[0].toLower() == "/killchannel" )
		{
			// [0:59 AM]>>KIC {"channel":"test"}
			JSONNode node;
			JSONNode channode ( "channel", inputText.mid ( 13 ).simplified().toStdString() );
			node.push_back ( channode );
			std::string out = "KIC " + node.write();
			sendWS ( out );
			success = true;
		}
		else if ( parts[0].toLower() == "/broadcast" )
		{
			//[1:14 AM]>>BRO {"message":"test"}
			JSONNode node;
			JSONNode msgnode ( "message", inputText.mid ( 11 ).simplified().toStdString() );
			node.push_back ( msgnode );
			std::string out = "BRO " + node.write();
			sendWS ( out );
			success = true;
		}
		else if ( parts[0].toLower() == "/setmode" )
		{
			//[23:59 PM]>>RMO {"channel":"ADH-9bbe33158b12f525f422","mode":"chat"}
			if (inputText.length() < 10 || (parts[1].toLower() != "chat" && parts[1].toLower() != "ads" && parts[1].toLower() != "both") )
			{
				QString err("Correct usage: /setmode &lt;chat|ads|both&gt;");
				FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, currentPanel, 0, err, currentPanel);
			}

			if (currentPanel->isOp(characterList[charName]) || characterList[charName]->isChatOp())
			{
				QString mode = inputText.mid(9);
				JSONNode node;
				JSONNode channode("channel", currentPanel->name().toStdString());
				JSONNode modenode("mode", mode.toStdString());
				node.push_back(channode);
				node.push_back(modenode);
				std::string out = "RMO " + node.write();
				sendWS(out);
			}
			else
			{
				QString err("You can only do that in channels you moderate.");
				FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, currentPanel, 0, err, currentPanel);
			}
			success = true;
		}
		else if ( parts[0].toLower() == "/bottle" )
		{
			if (currentPanel == 0 || currentPanel->type() == FChannel::CHANTYPE_CONSOLE || currentPanel->type() == FChannel::CHANTYPE_PM)
			{
				QString err("You can't use that in this panel.");
				FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, currentPanel, 0, err, currentPanel);
			}
			else
			{
				std::string out = "RLL ";
				JSONNode node;
				JSONNode channode("channel", currentPanel->name().toStdString());
				JSONNode dicenode("dice", "bottle");
				node.push_back(channode);
				node.push_back(dicenode);
				out += node.write();
				sendWS(out);
			}
			success = true;

		}
		else if ( parts[0].toLower() == "/roll" )
		{
			if (currentPanel == 0 || currentPanel->type() == FChannel::CHANTYPE_CONSOLE || currentPanel->type() == FChannel::CHANTYPE_PM)
			{
				QString err("You can't use that in this panel.");
				FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, currentPanel, 0, err, currentPanel);
			}
			else
			{
				QString roll;
				if (parts.count() < 2)
				{
					roll = "1d10";
				} else {
					roll = parts[1].toLower();
				}
				std::string out = "RLL ";
				JSONNode node;
				JSONNode channode("channel", currentPanel->name().toStdString());
				JSONNode dicenode("dice", roll.toStdString());
				node.push_back(channode);
				node.push_back(dicenode);
				out += node.write();
				sendWS(out);
			}
			success = true;
		}
		else if (debugging && parts[0].toLower() == "/channeltojson")
		{
			QString output("[noparse]");
			JSONNode* node = currentPanel->toJSON();
			output += node->write().c_str();
			output += "[/noparse]";
			FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, currentPanel, 0, output, currentPanel);
			delete node;
			success = true;
        }
        else if (debugging && parts[0].toLower() == "/refreshqss")
        {
            QFile stylefile("default.qss");
            stylefile.open(QFile::ReadOnly);
            QString stylesheet = QLatin1String(stylefile.readAll());
            setStyleSheet(stylesheet);
            QString output = "Refreshed stylesheet from default.qss";
            FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, currentPanel, 0, output, currentPanel);
            success = true;
        }
		else if (parts[0].toLower() == "/channeltostring")
		{
			QString* output = currentPanel->toString();
			FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, currentPanel, 0, *output, currentPanel);
			success = true;
			delete output;
		}
	}
	else
	{
		msg = "<b>" + charName + "</b>: " + ownText;
		success = true;
	}

	if ( success )
	{
		plainTextEdit->clear();
	}

	if ( msg != 0 )
	{
		if ( pm )
		{
			std::string character = currentPanel->recipient().toStdString();
			std::string message(inputText.toUtf8().data());
			messagePrivate ( character, message );
			FMessage fmsg(FMessage::MESSAGETYPE_PRIVMESSAGE, currentPanel, characterList[charName], ownText, currentPanel);
		}
		else
		{
			std::string chan = currentPanel->name().toStdString();
			std::string message(inputText.toUtf8().data());
			messageChannel ( chan, message );
			FMessage fmsg(FMessage::MESSAGETYPE_CHANMESSAGE, currentPanel, characterList[charName], ownText, currentPanel);
		}
	}
}

ChannelListItem::ChannelListItem ( QString& name, int chars ) : QListWidgetItem ( name )
{
	this->name = name;
	this->title = name;
	this->chars = chars;
}
ChannelListItem::ChannelListItem ( QString& name, QString& title, int chars ) : QListWidgetItem ( name )
{
	this->name = name;
	this->chars = chars;
	this->title = title;
}
bool ChannelListItem::operator > ( ChannelListItem* cli )
{
	if ( cli->getChars() > this->chars )
		return true;
	else
		return false;
}

void flist_messenger::addToChannelsDialogList ( ChannelListItem *cli )
{
	QString text = cli->getTitle();
	text += " (";
	QString it;
	it.setNum ( cli->getChars() );
	text += it;
	text += ")";
	cli->setText ( text );

	bool c = true;

	for ( int i = 0;i < cd_channelsList->count() && c;i++ )
	{
		if ( cli->getChars() > ( ( ChannelListItem* ) ( cd_channelsList->item ( i ) ) )->getChars() )
		{
			cd_channelsList->insertItem ( i, cli );
			c = false;
		}
	}

	if ( c )
		cd_channelsList->addItem ( cli );
}
void flist_messenger::addToProomsDialogList ( ChannelListItem *cli )
{
	QString text = cli->getTitle();
	text += " (";
	QString it;
	it.setNum ( cli->getChars() );
	text += it;
	text += ")";
	cli->setText ( text );

	bool c = true;

	for ( int i = 0;i < cd_proomsList->count() && c;i++ )
	{
		if ( cli->getChars() > ( ( ChannelListItem* ) ( cd_proomsList->item ( i ) ) )->getChars() )
		{
			cd_proomsList->insertItem ( i, cli );
			c = false;
		}
	}

	if ( c )
		cd_proomsList->addItem ( cli );
}
void flist_messenger::addToFriendsList ( QListWidgetItem *lwi )
{
	QString name = lwi->text();
	bool c = true;

	for ( int i = 0;i < fr_lwFriends->count() && c;i++ )
	{
		if ( name.toLower() < fr_lwFriends->item ( i )->text().toLower() )
		{
			fr_lwFriends->insertItem ( i, lwi );
			c = false;
		}
	}

	if ( c )
		fr_lwFriends->addItem ( lwi );
}
void flist_messenger::addToIgnoreList ( QListWidgetItem *lwi )
{
	QString name = lwi->text();
	bool c = true;

	for ( int i = 0;i < fr_lwIgnore->count() && c;i++ )
	{
		if ( name.toLower() < fr_lwIgnore->item ( i )->text().toLower() )
		{
			fr_lwIgnore->insertItem ( i, lwi );
			c = false;
		}
	}

	if ( c )
		fr_lwIgnore->addItem ( lwi );
}
void flist_messenger::mr_btnCancelClicked()
{
	makeRoomDialog->hide();
}
void flist_messenger::cd_btnCancelClicked()
{
	channelsDialog->hide();
}
void flist_messenger::mr_btnSubmitClicked()
{
	QString title = mr_leName->text().simplified();
	if ( title == "" ) return;
	mr_leName->clear();
	makeRoomDialog->hide();
	JSONNode makenode;
	JSONNode namenode ( "channel", title.toStdString() );
	makenode.push_back ( namenode );
	std::string out = "CCR " + makenode.write();
	sendWS ( out );
}
void flist_messenger::cd_btnJoinClicked()
{
	QList<QListWidgetItem *> cliList = cd_channelsList->selectedItems();
	ChannelListItem* cli = 0;

	for ( int i = 0;i < cliList.count();i++ )
	{
		cli = ( ChannelListItem* ) ( cliList.at ( i ) );
		QString name = cli->getName();

		if ( channelList.count ( name ) == 0 || !channelList[name]->getActive() )
			joinChannel ( name );
	}
}
void flist_messenger::cd_btnProomsJoinClicked()
{
	QList<QListWidgetItem *> cliList = cd_proomsList->selectedItems();
	ChannelListItem* cli = 0;

	for ( int i = 0;i < cliList.count();i++ )
	{
		cli = ( ChannelListItem* ) ( cliList.at ( i ) );
		QString name = cli->getName();

		if ( channelList.count ( name ) == 0 || !channelList[name]->getActive() )
			joinChannel ( name );
	}
}
void flist_messenger::ss_btnCancelClicked()
{
	setStatusDialog->hide();
}
void flist_messenger::ss_btnSubmitClicked()
{
	QString message = ss_leMessage->text();
	QString status = ss_cbStatus->currentText();

	if ( status == "Looking for play!" )
		status = "looking";
	else if ( status == "Online" )
		status = "online";
	else if ( status == "Do not disturb" )
		status = "dnd";
	else if ( status == "Away" )
		status = "away";
	else
		status = "busy";

	std::string stdstat = status.toStdString();
	std::string stdmsg = message.toStdString();
	changeStatus ( stdstat, stdmsg );
	setStatusDialog->hide();
}
void flist_messenger::saveSettings()
{
	QSettings settings(settingsPath, QSettings::IniFormat);
	settings.setValue("join", BOOLSTR(se_leaveJoin));
	settings.setValue("online", BOOLSTR(se_onlineOffline));
	settings.setValue("ping", BOOLSTR(se_ping));
	settings.setValue("sounds", BOOLSTR(se_sounds));
	settings.setValue("alwaysping", BOOLSTR(se_alwaysPing));
	settings.setValue("helpdesk", BOOLSTR(se_helpdesk));
	settings.setValue("logs", BOOLSTR(se_chatLogs));
	settings.setValue("username", username);
	QString pinglist, s;
	foreach (s, selfPingList)
	{
		pinglist.append(", ");
		pinglist.append(s);
	}
	settings.setValue("pinglist", pinglist.mid(2));
	QString channels;
	FChannel* c;
	foreach(c, channelList)
	{
		if (c->getActive() && c->type() != FChannel::CHANTYPE_CONSOLE && c->type() != FChannel::CHANTYPE_PM)
		{
			channels.append("|||");
			channels.append(c->name());
		}
	}
	settings.setValue("channels", channels.mid(3));
}
void flist_messenger::loadSettings()
{
	QSettings settings(settingsPath, QSettings::IniFormat);
	if (settings.status() != QSettings::NoError)
		loadDefaultSettings();
	else
	{
		se_leaveJoin = STRBOOL(settings.value("join").toString());
		se_onlineOffline = STRBOOL(settings.value("online").toString());
		se_ping = STRBOOL(settings.value("ping").toString());
		se_sounds = STRBOOL(settings.value("sounds").toString());
		se_alwaysPing = STRBOOL(settings.value("alwaysping").toString());
		se_helpdesk = STRBOOL(settings.value("helpdesk").toString());
		se_chatLogs = STRBOOL(settings.value("logs").toString());
		username = settings.value("username").toString();

		QString pinglist = settings.value("pinglist").toString();
		if (pinglist != "")
		{
			QStringList l = pinglist.split(", ");
			foreach (QString s, l)
				selfPingList.append(s);
		}
		QString channels = settings.value("channels").toString();
		if (channels != "")
		{
			QStringList c = channels.split("|||");
			foreach (QString s, c)
				defaultChannels.append(s);
		}
	}
	FMessage::doPing = se_ping;
	FMessage::doAlwaysPing = se_alwaysPing;
	FMessage::pingList = selfPingList;
	FMessage::doSounds = se_sounds;

}

void flist_messenger::loadDefaultSettings()
{
	selfPingList.clear();
	defaultChannels.clear();
	defaultChannels.append(QString("Frontpage"));
	se_leaveJoin = true;
	se_onlineOffline = true;
	se_ping = true;
	se_sounds = true;
	se_alwaysPing = false;
	se_helpdesk = false;
	se_chatLogs = true;
}
void flist_messenger::parseCommand ( std::string& input )
{
	try
	{
		printDebugInfo("<<" + input);
		std::string cmd = input.substr ( 0, 3 );
		JSONNode nodes;

		if ( input.length() > 4 )
		{
			nodes = libJSON::parse ( input.substr ( 4, input.length() - 4 ) );
		}

		if ( cmd == "ADL" )
		{
			JSONNode childnode = nodes.at ( "ops" );
			int size = childnode.size();

			for ( int i = 0;i < size;++i )
			{
				QString op = childnode[i].as_string().c_str();
				opList.append ( op.toLower() );

				if ( characterList.count ( op.toLower() ) != 0 )
				{
					// Set flag in character
					FCharacter* character = characterList[op];
					character->setIsChatOp ( true );

					// Sort userlists that contain this user
					FChannel* channel = 0;
					foreach ( channel, channelList )
					{
						if ( channel->charList().contains ( character ) )
						{
							channel->sortChars();
						}
					}
				}
			}
		}
		else if ( cmd == "AOP" )
		{
			//AOP {"character": "Viona"}
			//op
			QString ch = nodes.at ( "character" ).as_string().c_str();
			opList.append ( ch.toLower() );

			if ( characterList.count ( ch ) != 0 )
			{
				// Set flag in character
				FCharacter* character = characterList[ch];
				character->setIsChatOp ( true );

				// Sort userlists that contain this user
				FChannel* channel = 0;
				foreach ( channel, channelList )
				{
					if ( channel->charList().contains ( character ) )
					{
						channel->sortChars();
					}
				}
			}
		}
		else if ( cmd == "DOP" )
		{
			//DOP {"character": "Viona"}
			//Deop
			QString ch = nodes.at ( "character" ).as_string().c_str();
			opList.removeAll ( ch.toLower() );

			if ( characterList.count ( ch ) != 0 )
			{
				// Set flag in character
				FCharacter* character = characterList[ch];
				character->setIsChatOp ( false );

				// Sort userlists that contain this user
				FChannel* channel = 0;
				foreach ( channel, channelList )
				{
					if ( channel->charList().contains ( character ) )
					{
						channel->sortChars();
					}
				}
			}
		}
		else if ( cmd == "BRO" )
		{
			QString message(QString::fromUtf8(nodes.at ( "message" ).as_string().c_str()));
			FMessage fmsg(FMessage::MESSAGETYPE_BROADCAST, currentPanel, 0, message, currentPanel, channelList);
		}
		else if ( cmd == "CDS" )
		{
			QString channel = nodes.at ( "channel" ).as_string().c_str();

			if ( channelList.count ( channel ) == 0 )
			{
				printDebugInfo("[SERVER BUG] Server gave us the description of a channel we don't know about yet: " + input);
				return;
			}

			QString desc(QString::fromUtf8(nodes.at ( "description" ).as_string().c_str()));

			channelList[channel]->setDescription ( desc );
			QString msg = "You have joined <b>" + channelList[channel]->title() + "</b>: " + desc;
			FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, channelList[channel], 0, msg, currentPanel);
		}
		else if ( cmd == "CHA" )
		{
			cd_channelsList->clear();
			JSONNode childnode = nodes.at ( "channels" );
			int size = childnode.size();

			for ( int i = 0;i < size; ++i )
			{
				JSONNode channelnode = childnode.at ( i );
				QString name = channelnode.at ( "name" ).as_string().c_str();
				int characters;
				QString cs = channelnode.at ( "characters" ).as_string().c_str();
				characters = cs.toInt();
				printDebugInfo("Channel with characters: " + characters);
				// QString mode = channelnode.at("mode").as_string().c_str();
				ChannelListItem* chan = new ChannelListItem ( name, characters );
				addToChannelsDialogList ( chan );
			}
		}
		else if ( cmd == "CIU" )
		{
			//CIU {"sender": "EagerToPlease", "name": "ADH-085bcf60bef81b0790b7", "title": "Domination and Degradation"}
			QString output;
			QString sender = nodes.at ( "sender" ).as_string().c_str();
			QString name = nodes.at ( "name" ).as_string().c_str();
			QString title = nodes.at ( "title" ).as_string().c_str();
			output = "<b>" + sender + "</b> has invited you to join the room <a href=\"#AHI-" + name + "\">" + title + "</a>.";
			FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, currentPanel, 0, output, currentPanel);
		}
		else if ( cmd == "CKU" )
		{
			// CKU {"operator": "Maeve", "character": "Viona", "channel": "ADH-ad41f30904d30b529d08"}
			// Channel kick
			QString op = nodes.at("operator").as_string().c_str();
			QString chara = nodes.at("character").as_string().c_str();
			QString chan = nodes.at("channel").as_string().c_str();
			FChannel* channel = channelList[chan];
			FCharacter* character = characterList[chara];
			if (!channel)
			{
				printDebugInfo("[SERVER ERROR] Server tells us about a kick, but the channel doesn't exist.");
			} else if (!character){
				printDebugInfo("[SERVER ERROR] Server tells us about a kick, but the recipient doesn't exist.");
			} else {
				QString output;
				output = QString("<b>");
				output+= op;
				output+= QString("</b> has kicked <b>");
				output+= chara;
				output+= QString("</b> from ");
				output+= channel->title();
				if (chara == charName)
				{
					std::string chanstr = channel->name().toStdString();
					leaveChannel(chanstr, false);
					FMessage fmsg(FMessage::SYSTYPE_KICKBAN, currentPanel, 0, output, currentPanel);
				} else {
					FMessage fmsg(FMessage::SYSTYPE_KICKBAN, channel, 0, output, currentPanel);
				}
			}
		}
		else if ( cmd == "CBU" )
		{
			// CBU {"operator": "Maximilian Paton", "character": "Viona", "channel": "ADH-0e67e06da606b550020b"}
			// Channel ban
			QString op = nodes.at("operator").as_string().c_str();
			QString chara = nodes.at("character").as_string().c_str();
			QString chan = nodes.at("channel").as_string().c_str();
			FChannel* channel = channelList[chan];
			FCharacter* character = characterList[chara];
			if (!channel)
			{
				printDebugInfo("[ERROR] Server tells us about a ban, but the channel doesn't exist.");
			} else if (!character){
				printDebugInfo("[ERROR] Server tells us about a ban, but the recipient doesn't exist.");
			} else {
				QString output("<b>");
				output+= op;
				output+= "</b> has kicked and banned <b>";
				output+= chara;
				output+= "</b> from ";
				output+= channel->title();
				if (chara == charName)
				{
					std::string chanstr = channel->name().toStdString();
					leaveChannel(chanstr, false);
					FMessage fmsg(FMessage::SYSTYPE_KICKBAN, currentPanel, 0, output, currentPanel);
				} else {
					FMessage fmsg(FMessage::SYSTYPE_KICKBAN, channel, 0, output, currentPanel);
				}
			}
		}
		else if ( cmd == "COA" )
		{
			// COA {"channel": "Diapers/Infantilism"}
		}
		else if ( cmd == "COR" )
		{
			// COR {"channel": "Diapers/Infantilism"}
		}
		else if ( cmd == "COL" )
		{
			QString channelname = nodes.at ( "channel" ).as_string().c_str();

			if ( channelList.count ( channelname ) != 0 )
			{
				FChannel* channel = channelList[channelname];
				QStringList ops;
				JSONNode childnode = nodes.at ( "oplist" );
				int size = childnode.size();

				for ( int i = 0; i < size; ++i )
				{
					QString username = childnode.at ( i ).as_string().c_str();
					ops.push_back ( username );
				}

				channel->setOps ( ops );
			}
		}
		else if ( cmd == "CON" )
		{
			QString msg;
			msg += nodes["count"].as_string().c_str();
			msg += " users are currently connected.";
			FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, currentPanel, 0, msg, currentPanel);
		}
		else if ( cmd == "ERR" )
		{
			QString output;
			QString message = nodes.at ( "message" ).as_string().c_str();
			QString number = nodes.at ( "number" ).as_string().c_str();
			output = "<b>Error " + number + ": </b>" + message;

			if ( textEdit )
				FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, currentPanel, 0, output, currentPanel);
			else
				QMessageBox::critical ( this, "Error", message );

			if(number == "34")
			{
				JSONNode loginnode;
				JSONNode tempnode ( "method", "ticket" );
				loginnode.push_back ( tempnode );
				tempnode.set_name ( "account" );
				tempnode = username.toStdString();
				loginnode.push_back ( tempnode );
				tempnode.set_name ( "character" );
				tempnode = charName.toStdString();
				loginnode.push_back ( tempnode );
				tempnode.set_name ( "ticket" );
				tempnode = loginTicket;//.toStdString();
				loginnode.push_back ( tempnode );
				tempnode.set_name ( "cname" );
				tempnode = CLIENTID;
				loginnode.push_back ( tempnode );
				tempnode.set_name ( "cversion" );
				tempnode = VERSIONNUM;
				loginnode.push_back ( tempnode );
				std::string idenStr = "IDN " + loginnode.write();
				sendWS ( idenStr );
			}
		}
		else if ( cmd == "FLN" )
		{
			QString remchar = nodes.at ( "character" ).as_string().c_str();
			bool posted = false;
			QString offline = "<b>" + remchar + "</b> has disconnected.";
			if ( se_onlineOffline && selfFriendsList.contains ( remchar ) )
			{
				FMessage fmsg(FMessage::SYSTYPE_ONLINE, currentPanel, 0, offline, currentPanel);
				posted = true;
			}
			QString pmPanel = "PM-" + remchar;
			if (channelList.count(pmPanel))
			{
				channelList[pmPanel]->setTyping ( FChannel::TYPINGSTATUS_CLEAR );
				if (posted == false || channelList[pmPanel] != currentPanel)
					FMessage fmsg(FMessage::SYSTYPE_ONLINE, channelList[pmPanel], 0, offline, currentPanel);


				QString paneltitle = remchar;
				paneltitle += " (Offline)";
				channelList[pmPanel]->setTitle ( paneltitle );
				QString empty = "";
				channelList[pmPanel]->setRecipient(empty);
			}

			if ( characterList.count ( remchar ) )
			{
				FCharacter* character = characterList[remchar];

				for ( QHash<QString, FChannel*>::const_iterator iter = channelList.begin();iter != channelList.end(); ++iter )
				{
					if (se_leaveJoin && (*iter)->charList().count(character))
					{
						QString output = "[b]";
						output += remchar;
						output += "[/b] has left the channel.";
						FMessage fmsg(FMessage::SYSTYPE_JOIN, *iter, 0, output, currentPanel);
					}
					( *iter )->remChar ( character );
				}

				character = 0;

				delete characterList[remchar];
				characterList.remove ( remchar );
			}
		}
		else if ( cmd == "FRL" )
		{
			JSONNode childnode = nodes.at ( "characters" );

			int children = childnode.size();

			for ( int i = 0; i < children; ++i )
			{
				QString addchar = childnode.at(i).as_string().c_str();

				if ( !selfFriendsList.contains ( addchar ) )
				{
					selfFriendsList.append ( addchar );
				}
			}
		}
		else if ( cmd == "HLO" )
		{
			QString msg = "<B>";
			msg += nodes.at ( "message" ).as_string().c_str();
			msg += "</B>";
			FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, currentPanel, 0, msg, currentPanel);

			foreach (QString s, defaultChannels)
			{
				joinChannel(s);
			}
		}
		else if ( cmd == "ICH" )
		{
			QString channelname = nodes.at ( "channel" ).as_string().c_str();
			bool isAdh = channelname.startsWith ( "ADH-" );

			if ( channelList.count ( channelname ) == 0 )
			{
				if ( isAdh )
				{
					channelList[channelname] = new FChannel ( channelname, FChannel::CHANTYPE_ADHOC );
				}
				else
				{
					channelList[channelname] = new FChannel ( channelname, FChannel::CHANTYPE_NORMAL );
				}
			}

			FChannel* channel = channelList[channelname];
			JSONNode childnode = nodes.at ( "users" );

            int size = childnode.size();

			for ( int i = 0;i < size; ++i )
			{
				QString charname = childnode.at ( i ).at ( "identity" ).as_string().c_str();
				FCharacter* character = 0;

				if ( characterList.count ( charname ) == 0 )
				{
					printDebugInfo("[SERVER BUG] Server gave us a character in the channel user list that we don't know about yet: " + charname.toStdString() + ", " + input);
					continue;
				}

				character = characterList[charname];
				channel->addChar ( character, false );
            }
            QString mode = nodes.at("mode").as_string().c_str();
            if (mode == "chat") {
                channel->setMode(FChannel::CHANMODE_CHAT);
            } else if (mode == "ads") {
                channel->setMode(FChannel::CHANMODE_ADS);
            } else {
                channel->setMode(FChannel::CHANMODE_BOTH);
            }
			channel->sortChars();
			refreshUserlist();
		}
		else if ( cmd == "IDN" )
		{
			QString msg = "<B>";
			msg += nodes["character"].as_string().c_str();
			msg += "</B> Connected.";
			FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, console, 0, msg, console);
		}
		else if ( cmd == "IGN" )
		{
//			[12:38 PM]<<IGN {"character":"Kalyra","action":"add"}
//			[12:38 PM]<<IGN {"character":"Kalyra","action":"delete"}
			if ( nodes["action"].as_string() == "init" )
			{
				JSONNode childnode = nodes.at("characters");
				int count = childnode.size();
				for ( int i = 0 ; i < count ; ++i )
				{
					QString charname = childnode.at( i ).as_string().c_str();
					if ( !selfIgnoreList.contains( charname ) )
						selfIgnoreList.append( charname );
				}
			}
			if ( nodes["action"].as_string() == "add" )
			{
				QString character = nodes["character"].as_string().c_str();
				selfIgnoreList.append ( character );
				QString out = character +  QString ( " has been added to your ignore list." );
				FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, currentPanel, 0, out, currentPanel);
			}
			if ( nodes["action"].as_string() == "delete" )
			{
				QString character = nodes["character"].as_string().c_str();
				selfIgnoreList.removeAll ( character );
				QString out = character + QString ( " has been removed from your ignore list." );
				FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, currentPanel, 0, out, currentPanel);
			}
			if (friendsDialog)
				refreshFriendLists();
		}
		else if ( cmd == "JCH" )
		{
			QString channel = nodes.at ( "channel" ).as_string().c_str();

			if ( channelList.count ( channel ) == 0 )
			{
				QString adh = "ADH-";
				if ( channel.startsWith ( adh ) )
				{
					FChannel* chan = new FChannel(channel, FChannel::CHANTYPE_ADHOC);
					channelList[channel] = chan;
					QString channeltitle = nodes.at ( "title" ).as_string().c_str();
					chan->setTitle ( channeltitle );
					chan->pushButton = addToActivePanels(channel, channeltitle);
				}
				else
				{
					FChannel* chan = new FChannel(channel, FChannel::CHANTYPE_NORMAL);
					channelList[channel] = chan;
					chan->setTitle(channel);
					chan->pushButton = addToActivePanels(channel, channel);
				}
			}
			else if ( channelList[channel]->getActive() == false )
			{
				channelList[channel]->setActive ( true );
				channelList[channel]->pushButton->setVisible(true);
			}

			QString charname = nodes.at ( "character" ).at ( "identity" ).as_string().c_str();

			FCharacter* character = 0;

			if ( characterList.count ( charname ) == 0 )
			{
				printDebugInfo("[SERVER BUG]: Server told us about a character joining, but we don't know about them yet. " + charname.toStdString());
				return;
			}
			character = characterList[charname];
			channelList[channel]->addChar ( character );
			if ( charname == this->charName )
			{
				switchTab ( channel );
			}
			else
			{
				if ( currentPanel->name() == channel )
					refreshUserlist();
				if (se_leaveJoin)
				{
					QString output = "<b>";
					output += charname;
					output += "</b> has joined the channel.";
					FMessage fmsg(FMessage::SYSTYPE_JOIN, channelList[channel], 0, output, currentPanel);
				}
			}
		}
		else if ( cmd == "KID" )
		{
			// [19:41 PM]>>KIN {"character":"Cinnamon Flufftail"}
			// [19:41 PM]<<KID {"message": "(custom) kinks of Cinnamon Flufftail.", "type": "start"}
			// [19:41 PM](custom) kinks of Cinnamon Flufftail.
			// [19:41 PM]<<KID {"type": "custom", "value": "<3", "key": "*Viona"}
			// [19:41 PM]*Viona: <3
			// [19:41 PM]<<KID {"message": "End of (custom) kinks.", "type": "end"}
			QString type = nodes.at ( "type" ).as_string().c_str();

			if ( type == "start" )
			{
				ci_teKinks->clear();
			}
			else if ( type == "custom" )
			{
				QString out;
				QString value = nodes.at ( "value" ).as_string().c_str();
				QString key = nodes.at ( "key" ).as_string().c_str();
				out = QString ( "<b>" ) + key + QString ( ":</b> " ) + value;
				ci_teKinks->append ( out );
			}
		}
		else if ( cmd == "LCH" )
		{
			QString channel = nodes.at ( "channel" ).as_string().c_str();

			if ( channelList.count ( channel ) == 0 )
			{
				return;
			}

			QString charname = nodes.at ( "character" ).as_string().c_str();
			FCharacter* character = 0;
			if ( characterList.count ( charname ) == 0 )
			{
				return;
			}

			if ( charname == charName )
			{
				std::string stdchan = channel.toStdString();
				leaveChannel ( stdchan, false );
				return;
			}
			else
			{
				character = characterList[charname];
				channelList[channel]->remChar ( character );
				if (se_leaveJoin)
				{
					QString output = "<b>";
					output += charname;
					output += "</b> has left the channel.";
					FMessage fmsg(FMessage::SYSTYPE_JOIN, channelList[channel], 0, output, currentPanel);
				}
			}
		}
		else if ( cmd == "LIS" )
		{
			nodes.preparse();
			JSONNode childnode = nodes.at ( "characters" );
			int size = childnode.size();

			for ( int i = 0;i < size;++i )
			{
				int j = 0;
				JSONNode charnode = childnode.at ( i );
				QString addchar = charnode.at ( 0 ).as_string().c_str();	// Identity

				if ( characterList.count ( addchar ) == 0 )
				{
					characterList[addchar] = new FCharacter ( addchar, selfFriendsList.count(addchar) > 0 ? true : false );
				}

				FCharacter* character = characterList[addchar];

				QString gender = charnode.at ( 1 ).as_string().c_str();	// Gender
				character->setGender ( gender );
				QString status = charnode.at ( 2 ).as_string().c_str();	// Status
				character->setStatus ( status );
				QString statusmsg = charnode.at ( 3 ).as_string().c_str();	// Status
				character->setStatusMsg ( statusmsg );

				if ( opList.contains ( addchar.toLower() ) )
					character->setIsChatOp ( true );
			}
		}
		else if ( cmd == "LRP" )
		{
			//<<LRP {"message":":3","character":"Prison","channel":"Sex Driven LFRP"}
			QString channelname = nodes.at ( "channel" ).as_string().c_str();

			if ( channelList.count ( channelname ) == 0 )
			{
				return;
			}
			FChannel* channel = channelList[channelname];
			QString character = nodes.at ( "character" ).as_string().c_str();
			if ( selfIgnoreList.count ( character ) )
			{
				// Ignore message
				return;
			}
			QString message(QString::fromUtf8(nodes.at ( "message" ).as_string().c_str()));

			QString genderColor = "#FFFFFF";
			bool isOp = false;


			FCharacter* chanchar = 0;
			if ( characterList.count ( character ) != 0 )
			{
				chanchar = characterList[character];
				genderColor = chanchar->genderColor().name();
				isOp = ( chanchar->isChatOp() || channel->isOp( chanchar ) || channel->isOwner( chanchar ) );
			}
			FMessage fmsg(FMessage::MESSAGETYPE_ROLEPLAYAD, channel, chanchar, message, currentPanel);
		}
		else if ( cmd == "MSG" )
		{
			QString channelname = nodes.at ( "channel" ).as_string().c_str();

			if ( channelList.count ( channelname ) == 0 )
			{
				return;
			}

			FChannel* channel = channelList[channelname];

			QString character = nodes.at ( "character" ).as_string().c_str();

			if ( selfIgnoreList.count ( character.toLower() ) )
			{
				// Ignore message
				return;
			}

			QString message(QString::fromUtf8(nodes.at ( "message" ).as_string().c_str()));

			FCharacter* chanchar = 0;
			if ( characterList.count ( character ) != 0 )
			{
				chanchar = characterList[character];
			}

			FMessage fmsg(FMessage::MESSAGETYPE_CHANMESSAGE, channel, chanchar, message, currentPanel);
		}
		else if ( cmd == "NLN" )
		{
			QString addchar = nodes.at ( "identity" ).as_string().c_str();

			if ( characterList.count ( addchar ) == 0 )
			{
				characterList[addchar] = new FCharacter ( addchar, selfFriendsList.count(addchar) > 0 ? true : false );
			}

			bool posted = false;
			QString online = "<b>" + addchar + "</b> has connected.";
			if ( se_onlineOffline && selfFriendsList.contains ( addchar ) )
			{
				FMessage fmsg(FMessage::SYSTYPE_ONLINE, currentPanel, 0, online, currentPanel);
				posted = true;
			}
			QString pmPanel = "PM-" + addchar;
			if (channelList.count(pmPanel))
			{
				if (posted == false || channelList[pmPanel] != currentPanel)
					FMessage fmsg(FMessage::SYSTYPE_ONLINE, channelList[pmPanel], 0, online, currentPanel);
				channelList[pmPanel]->setRecipient(addchar);
				QString paneltitle = characterList[addchar]->PMTitle();
				channelList[pmPanel]->setTitle ( paneltitle );
			}
			FCharacter* character = characterList[addchar];

			QString gender = nodes.at ( "gender" ).as_string().c_str();
			character->setGender ( gender );
			QString status = nodes.at ( "status" ).as_string().c_str();
			character->setStatus ( status );

			if ( opList.contains ( addchar.toLower() ) )
				character->setIsChatOp ( true );
		}
		else if ( cmd == "ORS" )
		{
			/* ORS
	{"channels": [
	 {"name": "ADH-29a2ec641d78e5bd197e", "characters": "1", "title": "Eifania's Little Room"},
	 {"name": "ADH-74e4caef2965f4b33dd4", "characters": "1", "title": "Acrophobia"},
	 {"name": "ADH-fa132c6f2740c5ebaed7", "characters": "10", "title": "Femboy Faggot Fucksluts"}
	]}
   */
			cd_proomsList->clear();
			JSONNode childnode = nodes.at ( "channels" );
			int size = childnode.size();

			for ( int i = 0;i < size; ++i )
			{
				JSONNode channelnode = childnode.at ( i );
				QString name = channelnode.at ( "name" ).as_string().c_str();
				QString cs = channelnode.at ( "characters" ).as_string().c_str();
				int characters = cs.toInt();
				QString title = channelnode.at ( "title" ).as_string().c_str();
				ChannelListItem* chan = new ChannelListItem ( name, title, characters );
				addToProomsDialogList ( chan );
			}
		}
		else if ( cmd == "PIN" )
		{
			std::string msg = "PIN";
			sendWS ( msg );
		}
		else if ( cmd == "PRD" )
		{
			QString type = nodes.at ( "type" ).as_string().c_str();

			if ( type == "start" )
			{
				ci_teProfile->clear();
			}
			else if ( type == "info" )
			{
				QString out;
				QString value = nodes.at ( "value" ).as_string().c_str();
				QString key = nodes.at ( "key" ).as_string().c_str();
				out = QString ( "<b>" ) + key + QString ( ":</b> " ) + value;
				ci_teProfile->append ( out );
			}
		}
		else if ( cmd == "PRI" )
		{
			QString character = nodes.at ( "character" ).as_string().c_str();

			if ( selfIgnoreList.count ( character.toLower() ) )
            {
//				std::string out = "IGN ";
//				JSONNode notify;
//				JSONNode ch ( "character", character.toStdString() );
//				JSONNode ac ( "action", "notify" );
//				notify.push_back ( ch );
//				notify.push_back ( ac );
//				out += notify.write();
//				sendWS ( out );
			}
			else
			{
				QString message(QString::fromUtf8(nodes.at ( "message" ).as_string().c_str()));
				receivePM ( message, character );
			}
		}
		else if ( cmd == "RLL" )
		{
			// RLL {"message": "[b]Chromatic[/b] rolls 1d6: [b]2[/b]", "character": "Chromatic", "channel": "ADH-8b02d6012cbad0e7e2c0"}
			QString output = nodes.at ( "message" ).as_string().c_str();
			QString channelname = nodes.at ( "channel" ).as_string().c_str();

			FChannel* channel = channelList[channelname];
			FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, channel, 0, output, currentPanel);
		}
		else if ( cmd == "RTB" )
		{
			// RTB {"type":"note","sender":"Viona","subject":"test"}
		}
		else if ( cmd == "SFC" )
		{
			/* A staff report */
			QString output;
			QString action = nodes.at ( "action" ).as_string().c_str();

			if ( action == "report" )
			{
				bool logged;
				QString callid = nodes.at ( "callid" ).as_string().c_str();
				QString character = nodes.at ( "character" ).as_string().c_str();
				QString logid;
				try
				{
					logid = nodes.at ( "logid" ).as_string().c_str();
					logged = true;
				}
				catch ( std::out_of_range )
				{
					logged = false;
				}
				QString report = nodes.at ( "report" ).as_string().c_str();
				output	= "<b>STAFF ALERT!</b> From " + character + "<br />";
				output += report + "<br />";
				if (logged)
					output += "<a href=\"#LNK-http://www.f-list.net/fchat/getLog.php?log=" + logid + "\" ><b>Log~</b></a> | ";
				output += "<a href=\"#CSA-" + callid + "\"><b>Confirm Alert</b></a>";
				FMessage fmsg(FMessage::MESSAGETYPE_REPORT, currentPanel, 0, output, currentPanel, channelList);
			}
			else if ( action == "confirm" )
			{
				output = "<b>";
				output += nodes.at ( "moderator" ).as_string().c_str();
				output += "</b> is handling <b>";
				output += nodes.at ( "character" ).as_string().c_str();
				output += "</b>\'s report.";
				FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, currentPanel, 0, output, currentPanel);
			}
		}
		else if ( cmd == "STA" )
		{
			QString stachar = nodes.at ( "character" ).as_string().c_str();

			if ( characterList.count ( stachar ) )
			{
				FCharacter* character = characterList[stachar];
				QString status(QString::fromUtf8(nodes.at ( "status" ).as_string().c_str()));
				character->setStatus ( status );
				QString statmsg;
				// Crown messages can cause there to be no statusmsg.
				try
				{
					statmsg = nodes.at ( "statusmsg" ).as_string().c_str();
					character->setStatusMsg ( statmsg );
				}
				catch ( std::out_of_range )
				{
					statmsg = "";
				}

				if ( se_onlineOffline && selfFriendsList.contains ( stachar ) )
				{
					QString statusline = "<b>" + stachar + "</b> is now " + character->statusString();

					if ( statmsg.length() != 0 )
						statusline += " (" + statmsg + ")";

					FMessage fmsg(FMessage::SYSTYPE_ONLINE, currentPanel, 0, statusline, currentPanel);
				}

				if ( channelList.count ( "PM-" + stachar ) )
				{
					FChannel* pmPanel = channelList["PM-"+stachar];
					QString paneltitle = character->PMTitle();
					pmPanel->setTitle ( paneltitle );
				}
			}
			else
			{
				printDebugInfo("[SERVER BUG]: Server told us status for a character we don't know about: " + input);
			}

			refreshUserlist();
		}
		else if ( cmd == "RLL" )
		{
			// Dice rolling or bottling.
			QString output;
			QString message = nodes.at("message").as_string().c_str();
			QString channelname = nodes.at("channel").as_string().c_str();
			FChannel* channel = channelList[channelname];
			if (channel)
			{
				output = message;
				FMessage fmsg(FMessage::SYSTYPE_DICE, channel, 0, output, currentPanel);
			}
		}
		else if ( cmd == "SYS" )
		{
			QString output;
			QString message = nodes.at ( "message" ).as_string().c_str();
			output = "<b>System message:</b> " + message;
			FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, currentPanel, 0, output, currentPanel);
		}
		else if ( cmd == "TPN" )
		{
			// Unparsed command: TPN {"status": "clear", "character": "Becca Greene"}
			// Unparsed command: TPN {"status": "typing", "character": "Becca Greene"}
			// Unparsed command: TPN {"status": "paused", "character": "Becca Greene"}
			QString status = nodes.at ( "status" ).as_string().c_str();
			QString character = nodes.at ( "character" ).as_string().c_str();
			QString panelName = "PM-" + character;

			if ( channelList.count ( panelName ) != 0 )
			{
				FChannel* panel = channelList[panelName];

				if ( status == "typing" )
				{
					panel->setTyping ( FChannel::TYPINGSTATUS_TYPING );
				}
				else if ( status == "paused" )
				{
					panel->setTyping ( FChannel::TYPINGSTATUS_PAUSED );
				}
				else // if (status == "clear")
				{
					panel->setTyping ( FChannel::TYPINGSTATUS_CLEAR );
				}

				panel->updateButtonColor();
			}
		}
		else if ( cmd == "VAR" )
		{
			// <<VAR {"value":4096,"variable":"chat_max"}
			QString value = nodes.at("value").as_string().c_str();
			QString variable = nodes.at("value").as_string().c_str();
			serverVariables[variable] = value;
		}
		else if ( cmd == "RMO" )
		{
			//			[12:15 AM] Unparsed command: RMO {"mode":"chat","channel":"ADH-af9c1cd5e1bf31220ab2"}
			//			[12:15 AM] Unparsed command: RMO {"mode":"both","channel":"ADH-af9c1cd5e1bf31220ab2"}
			//			[12:15 AM] Unparsed command: RMO {"mode":"ads","channel":"ADH-af9c1cd5e1bf31220ab2"}
			QString output;
            QString mode = nodes.at("mode").as_string().c_str();
			QString channel = nodes.at("channel").as_string().c_str();
			FChannel* chan = channelList[channel];
			if (chan==0) return;
			QString name = channelList[channel]->title();
			if (mode == "ads")
			{
				chan->setMode(FChannel::CHANMODE_ADS);
				output = name + "'s mode was changed to: Ads only.";
			}
			else if (mode == "chat")
			{
				chan->setMode(FChannel::CHANMODE_CHAT);
				output = name + "'s mode was changed to: Chat only.";
			}
			else
			{
				chan->setMode(FChannel::CHANMODE_BOTH);
				output = name + "'s mode was changed to: Chat and ads.";
            }
			FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, chan, 0, output, currentPanel);
		}
		else if ( cmd == "ZZZ" )
		{
			QString output;
			QString message = nodes.at ( "message" ).as_string().c_str();
			output = "<b>Debug Reply:</b> " + message;
			FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, currentPanel, 0, output, currentPanel);
		}
		else
		{
			printDebugInfo("Unparsed command: " + input);
			QString qinput = "Unparsed command: ";
			qinput += input.c_str();
			FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, currentPanel, 0, qinput, currentPanel);
		}
	}
	catch ( std::invalid_argument )
	{
		printDebugInfo("Server returned invalid json in its response: " + input);
	}
	catch ( std::out_of_range )
	{
		printDebugInfo("Server produced unexpected json without a field we expected: " + input);
	}
}

void flist_messenger::flashApp(QString& reason)
{
	printDebugInfo(reason.toStdString());
	QApplication::alert(this, 10000);
}

#include "flist_messenger.moc"

