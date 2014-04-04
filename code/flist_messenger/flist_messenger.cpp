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
#include "flist_global.h"

#include <QString>

#include "flist_account.h"
#include "flist_server.h"
#include "flist_session.h"

// Bool to string macro
#define BOOLSTR(b) ( (b) ? "true" : "false" )
// String to bool macro
#define STRBOOL(s) ( (s=="true") ? true : false )

QString flist_messenger::settingsPath = "./settings.ini";

//get ticket, get characters, get friends list, get default character
void flist_messenger::prepareLogin ( QString username, QString password )
{
	connect(account, SIGNAL( loginError(FAccount *, QString, QString) ), this, SLOT( loginError(FAccount *, QString, QString ) ));
	connect(account, SIGNAL( loginComplete(FAccount *) ), this, SLOT( loginComplete(FAccount *) ));

	account->loginUserPass(username, password);
}
void flist_messenger::handleSslErrors( QList<QSslError> sslerrors )
{
        QMessageBox msgbox;
        QString errorstring;
        foreach(const QSslError &error, sslerrors) {
                if(!errorstring.isEmpty()) {
                        errorstring += ", ";
                }
                errorstring += error.errorString();
        }
        msgbox.critical ( this, "SSL ERROR DURING LOGIN!", errorstring );
}
void flist_messenger::handleLogin()
{
        QMessageBox msgbox;

	if ( account->loginreply->error() != QNetworkReply::NoError )
        {
                QString message = "Response error during login step ";
                message.append ( NumberToString::_uitoa<unsigned int> ( loginStep ).c_str() );
                message.append ( " of type " );
		message.append ( NumberToString::_uitoa<unsigned int> ( ( unsigned int ) account->loginreply->error() ).c_str() );
                msgbox.critical ( this, "FATAL ERROR DURING LOGIN!", message );
                if (btnConnect) btnConnect->setEnabled(true);
                return;
        }

	QByteArray respbytes = account->loginreply->readAll();

	account->loginreply->deleteLater();
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

	account->ticket = subnode.as_string().c_str();
        subnode = respnode.at ( "default_character" );
        account->defaultCharacter = subnode.as_string().c_str();
        childnode = respnode.at ( "characters" );
        int children = childnode.size();

        for ( int i = 0; i < children; ++i )
        {
                QString addchar = childnode[i].as_string().c_str();
                account->characterList.append ( addchar );
        }
        setupLoginBox();
}

void flist_messenger::loginError(FAccount *account, QString errortitle, QString errorstring)
{
	(void) account; //todo:
	if (btnConnect) btnConnect->setEnabled(true);
	QMessageBox msgbox;
	msgbox.critical(this, errortitle, errorstring);
}
void flist_messenger::loginComplete(FAccount *account)
{
        (void) account; //todo: initialise window with username
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
	server = new FServer(this);
	account = server->addAccount();
	account->ui = this;
	//account = new FAccount(0, 0);
        versionIsOkay = true;
        doingWS = true;
        notificationsAreaMessageShown = false;
        console = 0;
        textEdit = 0;
        //tcpSock = 0;
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
	FChannelPanel::initClass();
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
    if (currentPanel->getMode() == CHANNEL_MODE_ADS && !input.startsWith("/")) {
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
	lineEdit = new QLineEdit(account->getUserName());
        lineEdit->installEventFilter(loginreturn);
        lineEdit->setObjectName ( QString ( "accountNameInput" ) );
        gridLayout->addWidget ( lineEdit, 0, 1 );
        lineEdit = new QLineEdit;
        lineEdit->installEventFilter(loginreturn);
        lineEdit->setEchoMode ( QLineEdit::Password );
        lineEdit->setObjectName ( QString ( "passwordInput" ) );
        gridLayout->addWidget ( lineEdit, 1, 1 );

//        // "Checking version" label
//        lblCheckingVersion = new QLabel( QString ( "Checking version..." ) );
//        gridLayout->addWidget ( lblCheckingVersion, 2, 1 );

        // The login button
        btnConnect = new QPushButton;
        btnConnect->setObjectName ( QString ( "loginButton" ) );
        btnConnect->setText ( "Login" );
        btnConnect->setIcon ( QIcon ( ":/images/tick.png" ) );
//        btnConnect->hide();
        gridLayout->addWidget ( btnConnect, 2, 1 );
        verticalLayout->addLayout ( gridLayout );
        this->setCentralWidget ( verticalLayoutWidget );
        connect ( btnConnect, SIGNAL ( clicked() ), this, SLOT ( connectClicked() ) );

        int wid = QApplication::desktop()->width();
        int hig = QApplication::desktop()->height();
        int mwid = 265;
        int mhig = 100;
        setGeometry ( ( wid / 2 ) - ( int ) ( mwid*0.5 ), ( hig / 2 ) - ( int ) ( mhig*0.5 ), mwid, mhig );

//        // Fetch version.
//        lurl = QString ( "https://www.f-list.net/json/getApiTicket.json" );
//        lreply = qnam.get ( QNetworkRequest ( lurl ) );
//        connect ( lreply, SIGNAL ( finished() ), this, SLOT ( versionInfoReceived() ) );

}
void flist_messenger::connectClicked()
{
        if (versionIsOkay)
        {
                btnConnect->setEnabled(false);
                lineEdit = this->findChild<QLineEdit *> ( QString ( "accountNameInput" ) );
		account->setUserName(lineEdit->text());
                lineEdit = this->findChild<QLineEdit *> ( QString ( "passwordInput" ) );
                QString password = lineEdit->text();
                loginStep = 1;
		prepareLogin ( account->getUserName(), password );
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

        for ( int i = 0;i < account->characterList.count();++i )
        {
                comboBox->addItem ( account->characterList[i] );

                if ( account->characterList[i] == account->defaultCharacter )
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
        str +=        "/join &lt;channel&gt;<br />";
        str +=        "/priv &lt;character&gt;<br />";
        str +=        "/ignore &lt;character&gt;<br />";
        str +=        "/unignore &lt;character&gt;<br />";
        str +=        "/ignorelist<br />";
        str +=        "/code<br />";
        str +=        "/roll &lt;1d10&gt; (WIP)<br />";
        str +=        "/status &lt;Online|Looking|Busy|DND&gt; &lt;optional message&gt;<br />";
        str +=        "<b>Channel owners:</b><br />";
        str +=        "/makeroom &lt;name&gt;<br />";
        str +=        "/invite &lt;person&gt;<br />";
        str +=        "/openroom<br />";
        str +=        "/closeroom<br />";
        str +=        "/setdescription &lt;description&gt;<br />";
        str +=        "/getdescription<br />";
        str +=        "/setmode &lt;chat|ads|both&gt;";
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
        str+= "For bug reports, PM Viona or post <a href=\"#LNK-https://www.f-list.net/forum.php?forum=1698\">here</a>.<br />";
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
	FSession *session = account->getSession(charName);

	if (tb_recent->type() == FChannelPanel::CHANTYPE_PM)
        {
                // Setup for PM
		if (!session->isCharacterOnline(tb_recent->recipient())) { // Recipient is offline
			return false;
		}
                channelSettingsDialog = new QDialog(this);
                FCharacter* ch = session->getCharacter(tb_recent->recipient());
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
		FChannelPanel* ch = tb_recent;
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
                if ( ! ( ch->isOp(session->characterlist[charName]) || session->characterlist[charName]->isChatOp() ) )
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
                JSONNode channode ( "channel", cs_chanCurrent->getChannelName().toStdString() );
                node.push_back ( channode );
                JSONNode descnode ( "description", cs_qsPlainDescription.toStdString() );
                node.push_back ( descnode );
                std::string out = "CDS " + node.write();
                sendWS ( out );
        }
        cs_chanCurrent->setAlwaysPing(cs_chbAlwaysPing->isChecked());

        QString setting = cs_chanCurrent->getChannelName();
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
	FSession *session = account->getSession(charName);
        QString character = ai_leName->text().simplified();

        if(character != "" && !session->isCharacterIgnored(character))
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
	//todo: fix this
	lurl = QString("https://www.f-list.net/json/getApiTicket.json?secure=no&account=" + account->getUserName() + "&password=" + account->getPassword());
        lreply = qnam.get ( QNetworkRequest ( lurl ) );
        //todo: fix this!
        lreply->ignoreSslErrors();

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
    account->ticket = subnode.as_string().c_str();
    QString url_string="https://www.f-list.net/json/api/report-submit.php?account=";
    url_string += account->getUserName();
    url_string += "&character=";
    url_string += charName;
    url_string += "&ticket=";
    url_string += account->ticket;
    lurl = url_string;
    std::cout << url_string.toStdString() << std::endl;
    QByteArray postData;
    JSONNode* lognodes;
    lognodes = currentPanel->toJSON();
    std::string toWrite;
    toWrite = lognodes->write();
    QNetworkRequest request(lurl);
    fix_broken_escaped_apos(toWrite);
    lparam = QUrlQuery();
    lparam.addQueryItem("log", toWrite.c_str());
#if QT_VERSION >= 0x050000
    postData = lparam.query(QUrl::FullyEncoded).toUtf8();
#else
    postData = lparam.encodedQuery();
#endif
    lreply = qnam.post ( request, postData );
    //todo: fix this!
    lreply->ignoreSslErrors();

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
	if (tb_recent->type() == FChannelPanel::CHANTYPE_PM)
        {
                tb_recent->setActive(false);
                tb_recent->pushButton->setVisible(false);
		tb_recent->setTyping ( TYPING_STATUS_CLEAR );
        } else {
		leaveChannel(tb_recent->getPanelName(), tb_recent->getChannelName(), true);
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
        charName = comboBox->currentText();
	FSession *session = account->getSession(charName);

	session->autojoinchannels = defaultChannels;

        clearLoginBox();

        setupRealUI();

        connect ( session, SIGNAL ( socketErrorSignal ( QAbstractSocket::SocketError ) ), this, SLOT ( socketError ( QAbstractSocket::SocketError ) ) );

	session->connectSession();
		
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
	console = new FChannelPanel ( "FCHATSYSTEMCONSOLE", "FCHATSYSTEMCONSOLE", FChannelPanel::CHANTYPE_CONSOLE );
        QString name = "Console";
        console->setTitle ( name );
        channelList["FCHATSYSTEMCONSOLE"] = console;

        if ( objectName().isEmpty() )
                setObjectName ( "MainWindow" );

        resize ( 836, 454 );
        setWindowTitle ( FLIST_VERSION );
        setWindowIcon ( QIcon ( ":/images/apple-touch-icon.png" ) );
        actionDisconnect = new QAction ( this );
        actionDisconnect->setObjectName ( "actionDisconnect" );
        actionDisconnect->setText ( "Disconnect (WIP)" );
        actionQuit = new QAction ( this );
        actionQuit->setObjectName ( "actionQuit" );
        actionQuit->setText ( "&Quit" );
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
        menuFile->setTitle ( "&File" );
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
}
void flist_messenger::userListContextMenuRequested ( const QPoint& point )
{
        QListWidgetItem* lwi = listWidget->itemAt ( point );

        if ( lwi )
        {
		FSession *session = account->getSession(charName);
                FCharacter* ch = session->characterlist[lwi->text() ];
                ul_recent = ch;
                displayCharacterContextMenu ( ch );
        }
}
void flist_messenger::displayChannelContextMenu(FChannelPanel *ch)
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
		FSession *session = account->getSession(charName);
                if (session->isCharacterIgnored(ch->name()))
                        menu->addAction ( QIcon ( ":/images/heart.png" ), QString ( "Unignore" ), this, SLOT(ul_ignoreRemove()) );
                else
                        menu->addAction ( QIcon ( ":/images/heart-break.png" ), QString ( "Ignore" ), this, SLOT(ul_ignoreAdd()) );
                bool op = session->characterlist[charName]->isChatOp();
                if (op)
                {
                        menu->addAction ( QIcon ( ":/images/fire.png" ), QString ( "Chat Kick" ), this, SLOT(ul_chatKick()) );
                        menu->addAction ( QIcon ( ":/images/auction-hammer--exclamation.png" ), QString ( "Chat Ban" ), this, SLOT(ul_chatBan()) );
                        menu->addAction ( QIcon ( ":/images/alarm-clock.png" ), QString ( "Timeout..." ), this, SLOT(timeoutDialogRequested()) );
                }
                if (op || currentPanel->isOwner(session->characterlist[charName]))
                {
                        if (currentPanel->isOp(ch))
                                menu->addAction ( QIcon ( ":/images/auction-hammer--minus.png" ), QString ( "Remove Channel OP" ), this, SLOT(ul_channelOpRemove()) );
                        else
                                menu->addAction ( QIcon ( ":/images/auction-hammer--plus.png" ), QString ( "Add Channel OP" ), this, SLOT(ul_channelOpAdd()) );
                }
                if ((op || currentPanel->isOp(session->characterlist[charName])) && !ch->isChatOp())
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
			FSession *session = account->getSession(charName);
                        session->joinChannel(channel);
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
                        QString flist = "https://www.f-list.net/c/";
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

	//todo: Remember currently selected characters and then restore them once the list is refreshed.
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

	if ( currentPanel->type() == FChannelPanel::CHANTYPE_PM || currentPanel->type() == FChannelPanel::CHANTYPE_CONSOLE )
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

	FSession *session = account->getSession(charName);

        foreach ( s, session->friendslist )
        {
		if(session->isCharacterOnline(s)) {
                        f = session->characterlist[s];
                        lwi = new QListWidgetItem ( * ( f->statusIcon() ), f->name() );
                        addToFriendsList ( lwi );
                }
        }

        fr_lwIgnore->clear();

        foreach ( s, session->ignorelist )
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
FChannelTab* flist_messenger::addToActivePanels ( QString& panelname, QString &channelname, QString& tooltip )
{
        printDebugInfo("Joining " + channelname.toStdString());
	channelTab = this->findChild<FChannelTab *> ( panelname );

	if ( channelTab != 0 )
        {
		channelTab->setVisible ( true );
        }
        else
        {
                activePanelsContents->removeItem ( activePanelsSpacer );
		channelTab = new FChannelTab();
		channelTab->setObjectName ( panelname );
		channelTab->setGeometry ( QRect ( 0, 0, 30, 30 ) );
		channelTab->setFixedSize ( 30, 30 );
		channelTab->setStyleSheet ( "background-color: rgb(255, 255, 255);" );
		channelTab->setAutoFillBackground ( true );
		channelTab->setCheckable ( true );
		channelTab->setChecked ( false );
		channelTab->setToolTip ( tooltip );
		channelTab->setContextMenuPolicy(Qt::CustomContextMenu);
		connect ( channelTab, SIGNAL ( clicked() ), this, SLOT ( channelButtonClicked() ) );
		connect ( channelTab, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(tb_channelRightClicked(QPoint)));
		activePanelsContents->addWidget ( channelTab, 0, Qt::AlignTop );

                if ( panelname.startsWith("PM|||") )
                {
			avatarFetcher.applyAvatarToButton(channelTab, channelname);
			//channelTab->setIconSize(channelTab->iconSize()*1.5);
                }
                else if ( panelname.startsWith("ADH|||") )
                {
			//todo: custom buttons
			channelTab->setIcon ( QIcon ( ":/images/key.png" ) );
                }
                else
                {
			//todo: custom buttons
			channelTab->setIcon ( QIcon ( ":/images/hash.png" ) );
                }

                activePanelsContents->addSpacerItem ( activePanelsSpacer );
        }

	return channelTab;
}

void flist_messenger::aboutApp()
{
        QMessageBox::about ( this, "About F-List Messenger", "Created by:\n* Viona\n* Kira\n* Aniko\n* Hexxy\n* Eager\n\nCopyright(c) 2010-2011 F-list Team" );
}
void flist_messenger::quitApp()
{
        QApplication::quit();
}

void flist_messenger::socketError ( QAbstractSocket::SocketError socketError )
{
        (void) socketError;
        FSession *session = account->getSession(charName);
        QString sockErrorStr = session->tcpsocket->errorString();
        if (btnConnect)
                btnConnect->setEnabled(true);
        if ( currentPanel )
        {
                QString errorstring = "<b>Socket Error: </b>" + sockErrorStr;
		messageSystem(0, errorstring, MESSAGE_TYPE_ERROR);
        }
        else
                QMessageBox::critical ( this, "Socket Error!", "Socket Error: " + sockErrorStr );

        disconnected = true;
        //session->tcpsocket->abort();
        //session->tcpsocket->deleteLater();
        //session->tcpsocket = 0;
}
void flist_messenger::socketSslError (QList<QSslError> sslerrors ) {
        QMessageBox msgbox;
        QString errorstring;
        foreach(const QSslError &error, sslerrors) {
                if(!errorstring.isEmpty()) {
                        errorstring += ", ";
                }
                errorstring += error.errorString();
        }
        msgbox.critical ( this, "SSL ERROR DURING LOGIN!", errorstring );
	messageSystem(0, errorstring, MESSAGE_TYPE_ERROR);
}

void flist_messenger::sendWS ( std::string& input )
{
	FSession *session = account->getSession(charName);
	session->wsSend(input);
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
void flist_messenger::leaveChannel(QString &panelname, QString &channelname, bool toServer)
{
        channelList[panelname]->emptyCharList();
        channelList[panelname]->setActive ( false );
        channelList[panelname]->pushButton->setVisible ( false );
        currentPanel = console;

        if ( toServer )
        {
                JSONNode leavenode;
                JSONNode channode ( "channel", channelname.toStdString() );
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

	messageSystem(0, output, MESSAGE_TYPE_FEEDBACK);
}
void flist_messenger::fr_btnCloseClicked()
{
        friendsDialog->close();
}
void flist_messenger::fr_btnFriendsPMClicked()
{
	if(fr_lwFriends->selectedItems().size() <= 0) {
		return;
	}
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

	FSession *session = account->getSession(charName);
	if(lwi && session->isCharacterOnline(lwi->text())) {
                FCharacter* ch = session->characterlist[lwi->text() ];
                ul_recent = ch;
                displayCharacterContextMenu ( ch );
        }
}
void flist_messenger::tb_channelRightClicked ( const QPoint & point )
{
        (void) point;
        QObject* sender = this->sender();
        QPushButton* button = qobject_cast<QPushButton*> ( sender );
        if (button) {
                tb_recent = channelList.value(button->objectName());
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
	FSession *session = account->getSession(charName);
        msg.sprintf("<b>%d users online.</b>", session->characterlist.size());
	messageSystem(session, msg, MESSAGE_TYPE_FEEDBACK);
}
void flist_messenger::inputChanged()
{
	if ( currentPanel && currentPanel->type() == FChannelPanel::CHANTYPE_PM )
        {
                if ( plainTextEdit->toPlainText().simplified() == "" )
                {
                        typingCleared ( currentPanel );
			currentPanel->setTypingSelf ( TYPING_STATUS_CLEAR );
                }
                else
                {
			if ( currentPanel->getTypingSelf() != TYPING_STATUS_TYPING )
                        {
                                typingContinued ( currentPanel );
				currentPanel->setTypingSelf ( TYPING_STATUS_TYPING );
                        }
                }
        }
}
void flist_messenger::typingCleared ( FChannelPanel* channel )
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
void flist_messenger::typingContinued ( FChannelPanel* channel )
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
void flist_messenger::typingPaused ( FChannelPanel* channel )
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
	FSession *session = account->getSession(charName);
        if (session->isCharacterIgnored(name))
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
	FSession *session = account->getSession(charName);
        if (!session->isCharacterIgnored(name))
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
        JSONNode channode ( "channel", currentPanel->getChannelName().toStdString() );
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
        JSONNode channode ( "channel", currentPanel->getChannelName().toStdString() );
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
        JSONNode channode ( "channel", currentPanel->getChannelName().toStdString() );
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
        JSONNode channode ( "channel", currentPanel->getChannelName().toStdString() );
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
        QString l = "https://www.f-list.net/c/";
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
		messageSystem(0, error, MESSAGE_TYPE_FEEDBACK);
        }
        else if (who.simplified() == "" || why.simplified() == "")
        {
                QString error("Didn't fill out all fields.");
		messageSystem(0, error, MESSAGE_TYPE_FEEDBACK);
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

/**
Update the user interface based upon the mode of the current panel.
 */
void flist_messenger::updateChannelMode()
{
	if (currentPanel->getMode() == CHANNEL_MODE_CHAT ||
	    currentPanel->type() == FChannelPanel::CHANTYPE_PM || 
	    currentPanel->type() == FChannelPanel::CHANTYPE_CONSOLE) {
		//Chat only, so disable ability to send ads.
		btnSendAdv->setDisabled(true);
		btnSendChat->setDisabled(false);
	} else if (currentPanel->getMode() == CHANNEL_MODE_ADS) {
		//Ads only, disable the ability to send regular chat messages.
		btnSendAdv->setDisabled(false);
		btnSendChat->setDisabled(true);
	} else {
		//Regular channel, allow both ads and chat messages.
		btnSendAdv->setDisabled(false);
		btnSendChat->setDisabled(false);
	}
}
void flist_messenger::switchTab ( QString& tabname )
{
        if ( channelList.count ( tabname ) == 0 && tabname != "CONSOLE" )
        {
                printDebugInfo( "ERROR: Tried to switch to " + tabname.toStdString() + " but it doesn't exist.");
                return;
        }

        QString input = plainTextEdit->toPlainText();

	if ( currentPanel && currentPanel->type() == FChannelPanel::CHANTYPE_PM && currentPanel->getTypingSelf() == TYPING_STATUS_TYPING )
        {
                typingPaused ( currentPanel );
        }

        currentPanel->setInput ( input );

        currentPanel->pushButton->setChecked ( false );
	FChannelPanel* chan = 0;

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
	updateChannelMode();
}
void flist_messenger::openPMTab()
{
        QString ch = ul_recent->name();
        openPMTab ( ch );
}
void flist_messenger::openPMTab ( QString &character )
{
	FSession *session = account->getSession(charName);
        if (character.toLower() == charName.toLower())
        {
                QString msg = "You can't PM yourself!";
		messageSystem(session, msg, MESSAGE_TYPE_FEEDBACK);
                return;
        }
	if(!session->isCharacterOnline(character)) {
                QString msg = "That character is not logged in.";
		messageSystem(session, msg, MESSAGE_TYPE_FEEDBACK);
                return;
        }

        QString panelname = "PM|||" + charName + "|||" + character;

        if ( channelList.count ( panelname ) != 0 )
        {
                channelList[panelname]->setActive(true);
                channelList[panelname]->pushButton->setVisible(true);
                switchTab ( panelname );
        }
        else
        {
		channelList[panelname] = new FChannelPanel(panelname, character, FChannelPanel::CHANTYPE_PM);
                FCharacter* charptr = session->characterlist[character];
                QString paneltitle = charptr->PMTitle();
		FChannelPanel* pmPanel = channelList.value(panelname);
                pmPanel->setTitle ( paneltitle );
                pmPanel->setRecipient ( character );
                pmPanel->pushButton = addToActivePanels ( panelname, character, paneltitle );
                plainTextEdit->clear();
                switchTab ( panelname );
        }
}
void flist_messenger::btnSendChatClicked()
{
        // SLOT
        parseInput();
}
//todo: Move most of this functionality into FSession
void flist_messenger::btnSendAdvClicked()
{
        if (se_sounds)
                soundPlayer.play ( soundPlayer.SOUND_CHAT );

	FSession *session = account->getSession(charName);
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
		messageSystem(session, msg, MESSAGE_TYPE_FEEDBACK);
                return;
        }
        if ( currentPanel == 0 )
	{
                printDebugInfo("[CLIENT ERROR] currentPanel == 0");
                return;
        }
	if ( currentPanel == console || currentPanel->getMode() == CHANNEL_MODE_CHAT || currentPanel->type() == FChannelPanel::CHANTYPE_PM )
        {
                msg = "<b>Error:</b> Can't advertise here.";
		messageSystem(session, msg, MESSAGE_TYPE_FEEDBACK);
		return;
        }
        if ( inputText.length() > flist_messenger::BUFFERPUB )
        {
                msg = "<B>Error:</B> Message exceeds the maximum number of characters.";
		messageSystem(session, msg, MESSAGE_TYPE_FEEDBACK);
                return;
        }

        std::string chan = currentPanel->getChannelName().toStdString();
        std::string message = inputText.toStdString();
        bool isOp = false;
        QString genderColor;
	if(session->isCharacterOnline(charName)) {
                FCharacter* chanchar = session->characterlist[charName];
                genderColor = chanchar->genderColor().name();
                isOp = ( chanchar->isChatOp() || currentPanel->isOp( chanchar ) || currentPanel->isOwner( chanchar ) );
        }
        msg = "<font color=\"green\"><b>Roleplay ad by</font> <font color=\"";
        msg+= genderColor;
        msg+= "\">";
        msg+= charName;
        msg+= "</b></font>: ";
        msg+= ownText;
	messageChannel(session, currentPanel->getChannelName(), msg, MESSAGE_TYPE_RPAD);

        plainTextEdit->clear();
        advertiseChannel ( chan, message );
}
//todo: Move some of this functionality into the FSession class.
void flist_messenger::parseInput()
{
        if (se_sounds)
                soundPlayer.play ( soundPlayer.SOUND_CHAT );

	bool pm = ( bool ) ( currentPanel->type() == FChannelPanel::CHANTYPE_PM );
        QPlainTextEdit *messagebox = this->findChild<QPlainTextEdit *> ( QString ( "chatinput" ) );
        QString inputText = QString ( messagebox->toPlainText() );

        bool isCommand = ( inputText[0] == '/' );

        if ( !isCommand && currentPanel == console )
                return;

	FSession *session = account->getSession(charName);

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
		messageSystem(session, msg, MESSAGE_TYPE_FEEDBACK);
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
		messageSystem(session, msg, MESSAGE_TYPE_FEEDBACK);
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
                } else if(parts[0].toLower() == "/debugrecv") {
			//Artificially receive a packet from the server. The packet is not validated.
			session->wsRecv(ownText.mid(11).toStdString());
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
                        session->joinChannel(channel);
                        success = true;
                }
                else if ( parts[0].toLower() == "/leave" )
                {
			if ( currentPanel == console || currentPanel->type() == FChannelPanel::CHANTYPE_PM )
                        {
                                success = false;
                        }
                        else
                        {
                                leaveChannel(currentPanel->getPanelName(), currentPanel->getChannelName(), true);
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
                                if (!session->isCharacterIgnored(character))
                                {
                                        QString out = QString ( "This character is not in your ignore list." );
					messageSystem(session, out, MESSAGE_TYPE_FEEDBACK);
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
                        JSONNode channode ( "channel", currentPanel->getChannelName().toStdString() );
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
                        JSONNode channode ( "channel", currentPanel->getChannelName().toStdString() );
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
                        JSONNode channelnode("channel", currentPanel->getChannelName().toStdString());
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
                        JSONNode channelnode("channel", currentPanel->getChannelName().toStdString());
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
                        JSONNode channode ( "channel", currentPanel->getChannelName().toStdString() );
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
                        JSONNode channode ( "channel", currentPanel->getChannelName().toStdString() );
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
                        JSONNode channode ( "channel", currentPanel->getChannelName().toStdString() );
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
			case FChannelPanel::CHANTYPE_NORMAL:
                                out = "Copy this code to your message: [b][noparse][channel]" + currentPanel->getChannelName() + "[/channel][/noparse][/b]";
                                break;
			case FChannelPanel::CHANTYPE_ADHOC:
                                out = "Copy this code to your message: [b][noparse][session=" + currentPanel->title() + "]" + currentPanel->getChannelName() + "[/session][/noparse][/b]";
                                break;
                        default:
                                out = "This command is only for channels!";
                                break;
                        }
			messageSystem(session, out, MESSAGE_TYPE_FEEDBACK);
                        success = true;
                }
                else if ( parts[0].toLower() == "/unban" )
                {
                        // [17:30 PM]>>CUB {"channel":"ADH-cbae3bdf02cd39e8949e","character":"Viona"}
                        JSONNode node;
                        JSONNode charnode ( "character", inputText.mid ( 7 ).simplified().toStdString() );
                        node.push_back ( charnode );
                        JSONNode channode ( "channel", currentPanel->getChannelName().toStdString() );
                        node.push_back ( channode );
                        std::string out = "CUB " + node.write();
                        sendWS ( out );
                        success = true;
                }
                else if ( parts[0].toLower() == "/banlist" )
                {
                        // [17:30 PM]>>CBL {"channel":"ADH-cbae3bdf02cd39e8949e"}
                        JSONNode node;
                        JSONNode channode ( "channel", currentPanel->getChannelName().toStdString() );
                        node.push_back ( channode );
                        std::string out = "CBL " + node.write();
                        sendWS ( out );
                        success = true;
                }
                else if ( parts[0].toLower() == "/setdescription" )
                {
                        // [17:31 PM]>>CDS {"channel":"ADH-cbae3bdf02cd39e8949e","description":":3!"}
                        JSONNode node;
                        JSONNode channode ( "channel", currentPanel->getChannelName().toStdString() );
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
                        JSONNode channode ( "channel", currentPanel->getChannelName().toStdString() );
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
				messageSystem(session, err, MESSAGE_TYPE_FEEDBACK);
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
				messageSystem(session, err, MESSAGE_TYPE_FEEDBACK);
                        }

                        if (currentPanel->isOp(session->characterlist[charName]) || session->characterlist[charName]->isChatOp())
                        {
                                QString mode = inputText.mid(9);
                                JSONNode node;
                                JSONNode channode("channel", currentPanel->getChannelName().toStdString());
                                JSONNode modenode("mode", mode.toStdString());
                                node.push_back(channode);
                                node.push_back(modenode);
                                std::string out = "RMO " + node.write();
                                sendWS(out);
                        }
                        else
                        {
                                QString err("You can only do that in channels you moderate.");
				messageSystem(session, err, MESSAGE_TYPE_FEEDBACK);
                        }
                        success = true;
                }
                else if ( parts[0].toLower() == "/bottle" )
                {
			if (currentPanel == 0 || currentPanel->type() == FChannelPanel::CHANTYPE_CONSOLE || currentPanel->type() == FChannelPanel::CHANTYPE_PM)
                        {
                                QString err("You can't use that in this panel.");
				messageSystem(session, err, MESSAGE_TYPE_FEEDBACK);
                        }
                        else
                        {
                                std::string out = "RLL ";
                                JSONNode node;
                                JSONNode channode("channel", currentPanel->getChannelName().toStdString());
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
			if (currentPanel == 0 || currentPanel->type() == FChannelPanel::CHANTYPE_CONSOLE || currentPanel->type() == FChannelPanel::CHANTYPE_PM)
                        {
                                QString err("You can't use that in this panel.");
				messageSystem(session, err, MESSAGE_TYPE_FEEDBACK);
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
                                JSONNode channode("channel", currentPanel->getChannelName().toStdString());
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
			messageSystem(session, output, MESSAGE_TYPE_FEEDBACK);
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
	    messageSystem(session, output, MESSAGE_TYPE_FEEDBACK);
            success = true;
        }
                else if (parts[0].toLower() == "/channeltostring")
                {
                        QString* output = currentPanel->toString();
			messageSystem(session, *output, MESSAGE_TYPE_FEEDBACK);
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
		if(pm) {
			session->sendCharacterMessage(currentPanel->recipient(), inputText);
		} else {
			session->sendChannelMessage(currentPanel->getChannelName(), inputText);
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
	FSession *session = account->getSession(charName);

        for ( int i = 0;i < cliList.count();i++ )
        {
                cli = ( ChannelListItem* ) ( cliList.at ( i ) );
                QString name = cli->getName();

                if ( channelList.count ( name ) == 0 || !channelList[name]->getActive() )
                        session->joinChannel(name);
        }
}
void flist_messenger::cd_btnProomsJoinClicked()
{
        QList<QListWidgetItem *> cliList = cd_proomsList->selectedItems();
        ChannelListItem* cli = 0;
	FSession *session = account->getSession(charName);

        for ( int i = 0;i < cliList.count();i++ )
        {
                cli = ( ChannelListItem* ) ( cliList.at ( i ) );
                QString name = cli->getName();

                if ( channelList.count ( name ) == 0 || !channelList[name]->getActive() )
                        session->joinChannel(name);
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
	settings.setValue("username", account->getUserName());
        QString pinglist, s;
        foreach (s, selfPingList)
        {
                pinglist.append(", ");
                pinglist.append(s);
        }
        settings.setValue("pinglist", pinglist.mid(2));
        QString channels;
	FChannelPanel* c;
        foreach(c, channelList)
        {
		if (c->getActive() && c->type() != FChannelPanel::CHANTYPE_CONSOLE && c->type() != FChannelPanel::CHANTYPE_PM)
                {
                        channels.append("|||");
                        channels.append(c->getChannelName());
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
                QString tmp = settings.value("username").toString();
                account->setUserName(tmp);

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

#define PANELNAME(channelname,charname)  (((channelname).startsWith("ADH-") ? "ADH|||" : "CHAN|||") + (charname) + "|||" + (channelname))

void flist_messenger::flashApp(QString& reason)
{
        printDebugInfo(reason.toStdString());
        QApplication::alert(this, 10000);
}

void flist_messenger::setChatOperator(FSession *session, QString characteroperator, bool opstatus)
{
	debugMessage((opstatus ? "Added chat operator: " : "Removed chat operator: ") + characteroperator);
	// Sort userlists that contain this character
	if(session->isCharacterOnline(characteroperator)) {
		// Set flag in character
		FCharacter* character = session->getCharacter(characteroperator);
		FChannelPanel* channel = 0;
		foreach(channel, channelList) {
			//todo: filter by session
			if(channel->charList().contains(character)) {
				//todo: Maybe queue channel sorting as an idle task?
				channel->sortChars();
				if(currentPanel == channel) {
					refreshUserlist();
				}
			}
		}
	}
}

void flist_messenger::addCharacterChat(FSession *session, QString charactername)
{
	QString panelname = "PM|||" + session->character + "|||" + charactername;
	FChannelPanel *channelpanel = channelList.value(panelname);
	if(!channelpanel) {
		channelpanel = new FChannelPanel (panelname, charactername, FChannelPanel::CHANTYPE_PM);
		channelList[panelname] = channelpanel;
		channelpanel->setTitle(charactername);
		channelpanel->setRecipient(charactername);
		FCharacter *character = session->getCharacter(charactername);
		QString title = character->PMTitle();
		channelpanel->pushButton = addToActivePanels(panelname, charactername, title);
	}
	if(channelpanel->getActive() == false) {
		channelpanel->setActive(true);
		channelpanel->pushButton->setVisible(true);
	}
	channelpanel->setTyping(TYPING_STATUS_CLEAR);
	channelpanel->updateButtonColor();
}

void flist_messenger::addChannel(FSession *session, QString channelname, QString title)
{
	debugMessage("addChannel(\"" + channelname + "\", \"" + title + "\")");
	FChannelPanel* channelpanel;
	QString panelname = PANELNAME(channelname, session->character);
	if(!channelList.contains(panelname)) {
		if(channelname.startsWith("ADH-")) {
			channelpanel = new FChannelPanel(panelname, channelname, FChannelPanel::CHANTYPE_ADHOC);
		} else {
			channelpanel = new FChannelPanel(panelname, channelname, FChannelPanel::CHANTYPE_NORMAL);
		}
		channelList[panelname] = channelpanel;
		channelpanel->setTitle(title);
		channelpanel->pushButton = addToActivePanels(panelname, channelname, title);
	} else {
		channelpanel = channelList.value(panelname);
		//Ensure that the channel's title is set correctly for ad-hoc channels.
		if(channelname != title && channelpanel->title() != title) {
			channelpanel->setTitle(title);
		}
		if(channelpanel->getActive() == false) {
			channelpanel->setActive(true);
			channelpanel->pushButton->setVisible(true);
		}
	}
	//todo: Update UI elements with base on channel mode? (chat/RP AD/both)
}
void flist_messenger::removeChannel(FSession *session, QString channelname)
{
	(void)session; (void) channelname;
}
void flist_messenger::addChannelCharacter(FSession *session, QString channelname, QString charactername, bool notify)
{
	FChannelPanel* channelpanel;
	QString panelname = PANELNAME(channelname, session->character);
	if(!session->isCharacterOnline(charactername)) {
		printDebugInfo("[SERVER BUG]: Server told us about a character joining a channel, but we don't know about them yet. " + charactername.toStdString());
		return;
	}
	if(!channelList.contains(panelname)) {
		printDebugInfo("[BUG]: Told about a character joining a channel, but the panel for the channel doesn't exist. " + channelname.toStdString());
		return;
	}
	channelpanel = channelList.value(panelname);
	channelpanel->addChar(session->characterlist[charactername]);
	if(charactername == session->character) {
		switchTab(panelname);
	} else {
		if(notify) {
			if(currentPanel->getChannelName() == channelname) {
				//Only refresh the userlist if the panel has focus and notify is set.
				refreshUserlist();
			}
			messageChannel(session, channelname, "<b>" + charactername +"</b> has joined the channel", MESSAGE_TYPE_JOIN);
		}
	}
}
void flist_messenger::removeChannelCharacter(FSession *session, QString channelname, QString charactername)
{
	FChannelPanel* channelpanel;
	QString panelname = PANELNAME(channelname, session->character);
	if(!session->isCharacterOnline(charactername)) {
		printDebugInfo("[SERVER BUG]: Server told us about a character leaving a channel, but we don't know about them yet. " + charactername.toStdString());
		return;
	}
	if(!channelList.contains(panelname)) {
		printDebugInfo("[BUG]: Told about a character leaving a channel, but the panel for the channel doesn't exist. " + channelname.toStdString());
		return;
	}
	channelpanel = channelList.value(panelname);
	channelpanel->remChar(session->characterlist[charactername]);
	if(currentPanel->getChannelName() == channelname) {
		refreshUserlist();
	}
	messageChannel(session, channelname, "<b>" + charactername +"</b> has left the channel", MESSAGE_TYPE_LEAVE);
}
void flist_messenger::setChannelOperator(FSession *session, QString channelname, QString charactername, bool opstatus)
{
	QString panelname = PANELNAME(channelname, session->character);
	FChannelPanel *channelpanel = channelList.value(panelname);
	if(channelpanel) {
		if(opstatus) {
			channelpanel->addOp(charactername);
		} else {
			channelpanel->removeOp(charactername);
		}
		if(currentPanel == channelpanel) {
			refreshUserlist();
		}
	}
}

void flist_messenger::joinChannel(FSession *session, QString channelname)
{
	(void) session;
	if(currentPanel->getChannelName() == channelname) {
		refreshUserlist();
	}
	//todo: Refresh any elements in a half prepared state.
}
void flist_messenger::leaveChannel(FSession *session, QString channelname)
{
	QString panelname = PANELNAME(channelname, session->character);
	if(!channelList.contains(panelname)) {
		printDebugInfo("[BUG]: Told to leave a channel, but the panel for the channel doesn't exist. " + channelname.toStdString());
		return;
	}
	leaveChannel(panelname, channelname, false);
}
void flist_messenger::setChannelDescription(FSession *session, QString channelname, QString description)
{
	QString panelname = PANELNAME(channelname, session->character);
	FChannelPanel *channelpanel = channelList.value(panelname);
	if(!channelpanel) {
		printDebugInfo(QString("[BUG]: Was told the description of the channel '%1', but the panel for the channel doesn't exist.").arg(channelname).toStdString());
		return;
	}
	channelpanel->setDescription(description);
	QString message = QString("You have joined <b>%1</b>: %2").arg(channelpanel->title()).arg(bbparser.parse(description));
	messageChannel(session, channelname, message, MESSAGE_TYPE_CHANNEL_DESCRIPTION, true, false);
}
void flist_messenger::setChannelMode(FSession *session, QString channelname, ChannelMode mode)
{
	QString panelname = PANELNAME(channelname, session->character);
	FChannelPanel *channelpanel = channelList.value(panelname);
	if(!channelpanel) {
		printDebugInfo(QString("[BUG]: Was told the mode of the channel '%1', but the panel for the channel doesn't exist.").arg(channelname).toStdString());
		return;
	}
	channelpanel->setMode(mode);
	if(channelpanel == currentPanel) {
		//update UI with mode state
		updateChannelMode();
	}
}


void flist_messenger::notifyCharacterOnline(FSession *session, QString charactername, bool online)
{
	QString panelname = "PM|||" + session->character + "|||" + charactername;
	QList<QString> channels;
	QList<QString> characters;
	bool system = session->friendslist.contains(charactername);
	if(channelList.contains(panelname)) {
		characters.append(charactername);
		system = true;
		//todo: Update panel with changed online/offline status.
	}
	if(characters.count() > 0 || channels.count() > 0 || system) {
		messageMany(session, channels, characters, system, "<b>" + charactername +"</b> is now " + (online ? "online." : "offline."), online ? MESSAGE_TYPE_ONLINE : MESSAGE_TYPE_OFFLINE);
	}
}
void flist_messenger::notifyCharacterStatusUpdate(FSession *session, QString charactername)
{
	QString panelname = "PM|||" + session->character + "|||" + charactername;
	QList<QString> channels;
	QList<QString> characters;
	bool system = session->friendslist.contains(charactername);
	if(channelList.contains(panelname)) {
		characters.append(charactername);
		system = true;
		//todo: Update panel with changed status.
	}
	if(characters.count() > 0 || channels.count() > 0 || system) {
		FCharacter *character = session->getCharacter(charactername);
		QString statusmessage =  character->statusMsg();
		if(!statusmessage.isEmpty()) {
			statusmessage = QString(" (%1)").arg(statusmessage);
		}
		QString message = QString("<b>%1</b> is now %2%3").arg(charactername).arg(character->statusString()).arg(statusmessage);
		messageMany(session, channels, characters, system, message, MESSAGE_TYPE_STATUS);
	}
	//Refresh character list if they are present in the current panel.
	if(currentPanel->hasCharacter(session->getCharacter(charactername))) {
		refreshUserlist();
	}
}
void flist_messenger::setCharacterTypingStatus(FSession *session, QString charactername, TypingStatus typingstatus)
{
	QString panelname = "PM|||" + session->character + "|||" + charactername;
	FChannelPanel *channelpanel;
	channelpanel = channelList.value(panelname);
	if(!channelpanel) {
		return;
	}
	channelpanel->setTyping(typingstatus);
	channelpanel->updateButtonColor();
}
void flist_messenger::notifyCharacterCustomKinkDataUpdated(FSession *session, QString charactername)
{
	if(!ci_teKinks) {
		debugMessage(QString("Received custom kink data for the character '%1' but the profile window has not been created.").arg(charactername));
		return;
	}
	FCharacter *character = session->getCharacter(charactername);
	if(!character) {
		debugMessage(QString("Received custom kink data for the character '%1' but the character is not known.").arg(charactername));
		return;
	}
	QStringList &keys = character->getCustomKinkDataKeys();
	QHash<QString, QString> &kinkdata = character->getCustomKinkData();
	ci_teKinks->clear();
	foreach(QString key, keys) {
		ci_teKinks->append(QString("<b>%1:</b> %2").arg(key).arg(kinkdata[key]));
	}
}
void flist_messenger::notifyCharacterProfileDataUpdated(FSession *session, QString charactername)
{
	if(!ci_teProfile) {
		debugMessage(QString("Received profile data for the character '%1' but the profile window has not been created.").arg(charactername));
		return;
	}
	FCharacter *character = session->getCharacter(charactername);
	if(!character) {
		debugMessage(QString("Received profile data for the character '%1' but the character is not known.").arg(charactername));
		return;
	}
	QStringList &keys = character->getProfileDataKeys();
	QHash<QString, QString> &profiledata = character->getProfileData();
	ci_teProfile->clear();
	foreach(QString key, keys) {
		ci_teProfile->append(QString("<b>%1:</b> %2").arg(key).arg(profiledata[key]));
	}
}

void flist_messenger::notifyIgnoreUpdate(FSession *session)
{
	(void) session;
	if(friendsDialog) {
		refreshFriendLists();
	}
}
void flist_messenger::setIgnoreCharacter(FSession *session, QString charactername, bool ignore)
{
	(void) session; (void) charactername; (void) ignore;
	if(friendsDialog) {
		refreshFriendLists();
	}
}



void flist_messenger::messageMany(QList<QString> &panelnames, QString message, MessageType messagetype)
{
	//Put the message on all the given channel panels.
	QString panelname;
	QString messageout = "<small>[" + QTime::currentTime().toString("hh:mm:ss AP") + "]</small> " + message;
	foreach(panelname, panelnames) {
		FChannelPanel *channelpanel;
		channelpanel = channelList.value(panelname);
		if(!channelpanel) {
			debugMessage("[BUG] Tried to put a message on '" + panelname + "' but there is no channel panel for it. message:" + message);
			continue;
		}
		//Filter based on message type.
		switch(messagetype) {
		case MESSAGE_TYPE_LOGIN:
			break;
		case MESSAGE_TYPE_ONLINE:
		case MESSAGE_TYPE_OFFLINE:
			if(!se_onlineOffline) {
				continue;
			}
			break;
		case MESSAGE_TYPE_STATUS:
			if(!se_onlineOffline) {
				continue;
			}
			break;
		case MESSAGE_TYPE_CHANNEL_DESCRIPTION:
		case MESSAGE_TYPE_CHANNEL_MODE:
			break;
		case MESSAGE_TYPE_CHANNEL_INVITE:
			break;
		case MESSAGE_TYPE_JOIN:
		case MESSAGE_TYPE_LEAVE:
			if(!se_leaveJoin) {
				continue;
			}
			break;
		case MESSAGE_TYPE_ROLL:
		case MESSAGE_TYPE_RPAD:
		case MESSAGE_TYPE_CHAT:
			//todo: trigger sounds
			channelpanel->setHasNewMessages(true);
			if(panelname.startsWith("PM")) {
				channelpanel->setHighlighted(true);
			}
			channelpanel->updateButtonColor();
			break;
		case MESSAGE_TYPE_REPORT:
		case MESSAGE_TYPE_ERROR:
		case MESSAGE_TYPE_SYSTEM:
		case MESSAGE_TYPE_BROADCAST:
		case MESSAGE_TYPE_FEEDBACK:
			break;
		case MESSAGE_TYPE_KICK:
		case MESSAGE_TYPE_KICKBAN:
			break;
		case MESSAGE_TYPE_IGNORE_UPDATE:
			break;
		default:
			debugMessage("Unhandled message type " + QString::number(messagetype) + " for message '" + message + "'.");
		}
		channelpanel->addLine(messageout, true);
		if(channelpanel == currentPanel) {
			textEdit->append(messageout);
		}
	}
}
void flist_messenger::messageMany(FSession *session, QList<QString> &channels, QList<QString> &characters, bool system, QString message, MessageType messagetype)
{
	QList<QString> panelnames;
	QString charactername;
	QString channelname;
	QString panelnamemidfix = "|||" + session->character + "|||";
	if(system) {
		//todo: session based consoles?
		panelnames.append("FCHATSYSTEMCONSOLE");
	}
	foreach(charactername, characters) {
		panelnames.append("PM" + panelnamemidfix + charactername);
	}
	foreach(channelname, channels) {
		if(channelname.startsWith("ADH-")) {
			panelnames.append("ADH" + panelnamemidfix + channelname);
		} else {
			panelnames.append("CHAN" + panelnamemidfix + channelname);
		}
	}
	if(system) {
		if(!panelnames.contains(currentPanel->getPanelName())) {
			panelnames.append(currentPanel->getPanelName());
		}
	}
	messageMany(panelnames, message, messagetype);
}
void flist_messenger::messageAll(FSession *session, QString message, MessageType messagetype)
{
	QList<QString> panelnames;
	FChannelPanel *channelpanel;
	//todo: session based consoles?
	panelnames.append("FCHATSYSTEMCONSOLE");
	QString match = "|||" + session->character + "|||";
	//Extract all panels that are relevant to this session.
	foreach(channelpanel, channelList) {
		QString panelname = channelpanel->getPanelName();
		if(panelname.contains(match)) {
			panelnames.append(panelname);
		}
	}
	messageMany(panelnames, message, messagetype);
}
void flist_messenger::messageChannel(FSession *session, QString channelname, QString message, MessageType messagetype, bool console, bool notify)
{
	QList<QString> panelnames;
	panelnames.append(PANELNAME(channelname, session->character));
	if(console) {
		panelnames.append("FCHATSYSTEMCONSOLE");
	}
	if(notify) {
		QString panelname = currentPanel->getPanelName();
		if(!panelnames.contains(panelname)) {
			panelnames.append(panelname);
		}
	}
	messageMany(panelnames, message, messagetype);
}
void flist_messenger::messageCharacter(FSession *session, QString charactername, QString message, MessageType messagetype)
{
	QList<QString> panelnames;
	panelnames.append("PM|||" + session->character + "|||" + charactername);
	messageMany(panelnames, message, messagetype);
}
void flist_messenger::messageSystem(FSession *session, QString message, MessageType messagetype)
{
	(void) session; //todo: session based consoles?
	QList<QString> panelnames;
	panelnames.append("FCHATSYSTEMCONSOLE");
	if(currentPanel && !panelnames.contains(currentPanel->getPanelName())) {
		panelnames.append(currentPanel->getPanelName());
	}
	messageMany(panelnames, message, messagetype);
}

void flist_messenger::updateKnownChannelList(FSession *session)
{
	if(!cd_channelsList) {
		return;
	}
	cd_channelsList->clear();
	for(int i = 0; i < session->knownchannellist.size(); i++) {
		FChannelSummary *channelsummary = &session->knownchannellist[i];
		ChannelListItem *channellistitem = new ChannelListItem(channelsummary->name, channelsummary->count);
		addToChannelsDialogList(channellistitem);
	}
}
void flist_messenger::updateKnownOpenRoomList(FSession *session)
{
	if(!cd_proomsList) {
		return;
	}
	cd_proomsList->clear();
	for(int i = 0; i < session->knownopenroomlist.size(); i++) {
		FChannelSummary *channelsummary = &session->knownopenroomlist[i];
		ChannelListItem *channellistitem = new ChannelListItem(channelsummary->name, channelsummary->title, channelsummary->count);
		addToProomsDialogList(channellistitem);
	}
}
