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

// Some day we may implement a proper websocket connection system. Today is not that day.
std::string flist_messenger::WSConnect = "GET / HTTP/1.1\r\nUpgrade: WebSocket\r\nConnection: Upgrade\r\nHost: f-list.net:9722\r\nOrigin: http://www.f-list.net\r\nSec-WebSocket-Key1: Z+t 6` H  XM%31   7T 5=7 330 r@\r\nSec-WebSocket-Key2: 267 0 4 0  \\ K36 3 2\r\n\r\nabcdefgh";
QString flist_messenger::settingsPath = "./settings.ini";

//get ticket, get characters, get friends list, get default character
void flist_messenger::prepareLogin ( QString& username, QString& password )
{
    this->username = username;
    this->password = password;
    lurl = QUrl ( "http://www.f-list.net/json/getApiTicket.json?" );
    QNetworkRequest request(lurl);
    QUrlQuery q;
    q.addQueryItem("secure", "no");
    q.addQueryItem("account", username);
    q.addQueryItem("password", password);
    QByteArray postData = q.query(QUrl::FullyEncoded).toUtf8();
    lreply = qnam.post ( request, postData );
    connect ( lreply, SIGNAL ( finished() ), this, SLOT ( handleLogin() ) );
}
void flist_messenger::handleLogin()
{
    if ( lreply->error() != QNetworkReply::NoError )
    {
        QString message = "Response error during login ";
        message.append ( " of type " );
        message.append ( NumberToString::_uitoa<unsigned int> ( ( unsigned int ) lreply->error() ).c_str() );
        gui->connectError(message);
        return;
    }
    QByteArray respbytes = lreply->readAll();

    lreply->deleteLater();
    std::string response ( respbytes.begin(), respbytes.end() );
    JSONNode respnode = libJSON::parse ( response );
    JSONNode childnode = respnode.at ( "error" );

    if ( childnode.as_string() != "" )
    {
        QString message = QString("Error from server: ");
        message += childnode.as_string().c_str();
        gui->connectError(message);
        return;
    }

    JSONNode subnode = respnode.at ( "ticket" );

    loginTicket = subnode.as_string().c_str();
    subnode = respnode.at ( "default_character" );
    defaultCharacter = subnode.as_string().c_str();
    childnode = respnode.at ( "characters" );
    int children = childnode.size();

    for ( int i = 0; i < children; ++i )
    {
        QString addchar = childnode[i].as_string().c_str();
        selfCharacterList.append ( addchar );
    }

    gui->setupLoginBox(selfCharacterList, defaultCharacter);
}
void flist_messenger::versionInfoReceived()
{
}

void flist_messenger::init()
{
    settingsPath = QApplication::applicationDirPath() + "/settings.ini";
    //QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
}

flist_messenger::flist_messenger(bool d)
    : currentPanel(0),
      console(0)
{
    versionIsOkay = true;
    doingWS = true;
    console = 0;
    debugging = d;
    disconnected = true;
    settings = new FSettings(settingsPath);
    FCharacter::initClass();
    FChannel::initClass(settingsPath);
    FMessage::initClass(se_ping, se_alwaysPing, this);
    network = new FNetwork(this);
    gui = new FGui(this);
    connect(gui, SIGNAL(consoleButtonReady()), this, SLOT(setupConsole()));
    connect(this, SIGNAL(tabSwitched(FChannel*,FChannel*)), gui, SLOT(tabSwitched(FChannel*,FChannel*)));
}
//PORTED, LOCALLY REWRITTEN. DO NOT DELETE.
void flist_messenger::setupSocket(QString &charName)
{
    std::cout<<username.toStdString()<<std::endl;
    this->charName = charName;
    network->tryLogin(username, charName, loginTicket);
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
            console->addLine(q, true);
        }
    }
}
void flist_messenger::setupLoginBox()
{
    /*	clearConnectBox();
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
*/
}


void flist_messenger::submitReport(QString &problem, QString &who)
{
    QString message = "Uploading and sending your report and logs...";
    postSystemMessage(FSystemMessage::SYSTYPE_FEEDBACK, currentPanel, message);
    this->re_problem = problem;
    this->re_who = who;
    QString url = "http://www.f-list.net/json/getApiTicket.json?";
    url += "&secure=no";
    url += ("&account=" + username);
    url += ("&password=" + password);
    lurl = QUrl(url);
    lreply = qnam.get ( QNetworkRequest ( lurl ) );
    connect ( lreply, SIGNAL ( finished() ), this, SLOT ( reportTicketFinished() ) );
}

void flist_messenger::handleReportFinished()
{
    QString message = "Handling finished report...";
    postSystemMessage(FSystemMessage::SYSTYPE_FEEDBACK, currentPanel, message);
    if ( lreply->error() != QNetworkReply::NoError ){
        message = "Response error during sending of report ";
        message.append ( "of type " );
        message.append ( NumberToString::_uitoa<unsigned int> ( ( unsigned int ) lreply->error() ).c_str() );
        postSystemMessage(FSystemMessage::SYSTYPE_FEEDBACK, currentPanel, message);
        return;
    } else {
        QByteArray respbytes = lreply->readAll();
        lreply->deleteLater();
        std::string response ( respbytes.begin(), respbytes.end() );
        JSONNode respnode = libJSON::parse ( response );
        JSONNode childnode = respnode.at ( "error" );
        if ( childnode.as_string() != "" )
        {
            message = "Error from server: ";
            message += childnode.as_string().c_str();
            postSystemMessage(FSystemMessage::SYSTYPE_FEEDBACK, currentPanel, message);
            return;
        } else {
            childnode = respnode.at ( "log_id" );
            std::string logid = childnode.as_string();
            QString gt = "&gt;";
            QString lt = "&lt;";
            QString amp = "&amp;";
            QString problem = re_problem.replace ( '&', amp ).replace ( '<', lt ).replace ( '>', gt );
            QString who = re_who.replace ( '&', amp ).replace ( '<', lt ).replace ( '>', gt );
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
            network->sendWS(output);
            message = "Your report was uploaded";
            postSystemMessage(FSystemMessage::SYSTYPE_FEEDBACK, currentPanel, message);
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
        postSystemMessage(FSystemMessage::SYSTYPE_FEEDBACK, currentPanel, message);
        return;
    }
    QByteArray respbytes = lreply->readAll();
    lreply->deleteLater();
    std::string response ( respbytes.begin(), respbytes.end() );
    JSONNode respnode = libJSON::parse ( response );
    JSONNode childnode = respnode.at ( "error" );
    if ( childnode.as_string() != "" )
    {
        QString message = "<b>Error from server:</b> ";
        message += childnode.as_string().c_str();
        postSystemMessage(FSystemMessage::SYSTYPE_FEEDBACK, currentPanel, message);
        return;
    }
    JSONNode subnode = respnode.at ( "ticket" );
    loginTicket = subnode.as_string().c_str();
    QString url_string="http://www.f-list.net/json/api/report-submit.php?account=";
    url_string += username;
    url_string += "&character=";
    url_string += charName;
    url_string += "&ticket=";
    url_string += loginTicket;
    lurl = url_string;
    std::cout << url_string.toStdString() << std::endl;
    QByteArray postData;
    JSONNode* lognodes;
    lognodes = currentPanel->toJSON();
    std::string toWrite;
    toWrite = lognodes->write();
    QNetworkRequest request(lurl);
    /*
    QUrl params;
    params.addQueryItem("character", charName);
    network->fixBrokenEscapedApos(toWrite);
    params.addQueryItem("log", toWrite.c_str());
    postData = params.encodedQuery();
    */
    network->fixBrokenEscapedApos(toWrite);
    QUrlQuery q;
    q.addQueryItem("log", toWrite.c_str());
    postData = q.query(QUrl::FullyEncoded).toUtf8();
    lreply = qnam.post ( request, postData );
    connect ( lreply, SIGNAL ( finished() ), this, SLOT ( handleReportFinished() ) );
    delete lognodes;
}
//PORTED
void flist_messenger::loginClicked()
{
    /*
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
*/
}
//PORTED
void flist_messenger::setupRealUI()
{
    /*
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
    btnSetStatus->setIcon ( QIcon ( ":/status-white.png" ) );
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
*/
}
void flist_messenger::setupConsole()
{
    QString name = "CONSOLE";
    QString title= "The Console";
    console = new FChannel(name, FChannel::CHANTYPE_CONSOLE);
    currentPanel = console;
    channelList[name] = console;
    console->setTitle(title);

    switchTab(name);
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
            network->sendJoinChannel ( channel );
        }
        else if (cmd == "#CSA-")
        {
            // Confirm staff alert
            QString id = ls.right(ls.length()-5);
            network->sendConfirmReport(charName, id);
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
}
void flist_messenger::refreshUserlist()
{
    //	if ( currentPanel == 0 )
    //		return;

    //	listWidget = this->findChild<QListWidget *> ( QString ( "userlist" ) );
    //	listWidget->clear();
    //	QList<FCharacter*> charList = currentPanel->charList();
    //	QListWidgetItem* charitem = 0;
    //	FCharacter* character;
    //	FCharacter::characterStatus status;
    //	foreach ( character, charList )
    //	{
    //		status = character->status();
    //		charitem = new QListWidgetItem ( character->name() );
    //		QIcon* i = character->statusIcon();
    //		charitem->setIcon ( *i );

    //		QFont f = charitem->font();

    //		if ( character->isChatOp() )
    //		{
    //			f.setBold ( true );
    //			f.setItalic ( true );
    //		}
    //		else if ( currentPanel->isOp ( character ) )
    //		{
    //			f.setBold ( true );
    //		}
    //		charitem->setFont ( f );
    //		charitem->setTextColor ( character->genderColor() );
    //		listWidget->addItem ( charitem );
    //	}

    //	if ( currentPanel->type() == FChannel::CHANTYPE_PM || currentPanel->type() == FChannel::CHANTYPE_CONSOLE )
    //	{
    //		listWidget->hide();
    //	}
    //	else
    //	{
    //		listWidget->show();
    //	}
}

void flist_messenger::receivePM ( QString& message, QString& character )
{
    FChannel* pmPanel = 0;
    FCharacter* charptr=characterList[character];;
    QString panelname = "PM-" + character;

    if ( channelList.count ( panelname ) == 0 )
    {
        channelList["PM-"+character] = new FChannel ( "PM-" + character, FChannel::CHANTYPE_PM );
        QString paneltitle = charptr->PMTitle();
        pmPanel = channelList["PM-"+character];
        pmPanel->setTitle ( paneltitle );
        pmPanel->setRecipient ( character );
        gui->addPanelButton(panelname, paneltitle);
    }
    pmPanel = channelList[panelname];
    if (pmPanel->getActive() == false)
    {
        pmPanel->setActive(true);
        gui->showChannelButton(panelname);
    }
    pmPanel->setTyping ( FChannel::TYPINGSTATUS_CLEAR );
    pmPanel->updateButtonColor();

    postChatMessage(pmPanel, charptr, message);
}
//PORTED
void flist_messenger::connectedToSocket()
{
/*
    disconnected = false;
    tcpSock->write ( WSConnect.c_str() );
    tcpSock->flush();
*/
}
//PORTED
void flist_messenger::readReadyOnSocket()
{
    /*
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
        network->sendWS ( idenStr );
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
*/
}
//PORTED
void flist_messenger::socketError ( QAbstractSocket::SocketError socketError )
{
    /*
    QString input = "<b>Socket Error: </b>" + tcpSock->errorString();
    gui->connectError(input);
    disconnected = true;
*/
}

void flist_messenger::setOpList(QList<QString> &opList)
{
    this->opList = QList<QString>(opList);

    foreach (QString op, opList)
    {
        if ( characterList.count ( op ) != 0 )
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

void flist_messenger::addOp(QString &op)
{
    opList.append(op);
    if ( characterList.count ( op ) != 0 )
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

void flist_messenger::removeOp(QString &op)
{
    opList.removeAll(op);
    if ( characterList.count ( op ) != 0 )
    {
        // Set flag in character
        FCharacter* character = characterList[op];
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
std::string flist_messenger::escape(std::string &s)
{
    std::size_t n = s.length();
    std::string escaped;
    escaped.reserve(n * 2); // pessimistic preallocation

    for (std::size_t i = 0; i < n; ++i) {
        if (s[i] == '\\' || s[i] == '\'')
            escaped += '\\';
        escaped += s[i];
    }
    return escaped;
}

void flist_messenger::receiveBroadcast(QString &msg)
{
    postBroadcastMessage(msg);
}
void flist_messenger::joinedChannel(QString &name, QString &title)
{
    if ( channelList.count ( name ) == 0 )
    {
        QString adh = "ADH-";
        FChannel::channelType type = name.startsWith ( adh ) ? FChannel::CHANTYPE_ADHOC : FChannel::CHANTYPE_NORMAL;
        channelList[name] = new FChannel(name, type);
        channelList[name]->setTitle(title);
    }
    else if ( channelList[name]->getActive() == false )
    {
        channelList[name]->setActive ( true );
        channelList[name]->pushButton->setVisible(true);
    }
    gui->addPanelButton(name, title);
    switchTab(name);
}

void flist_messenger::postAdvertMessage(FChannel* channel, FCharacter* character, QString& msg)
{
    // make post
    FAdvertMessage post(channel, character, msg);
    QString output = post.getOutput();

    // post it in the panel
    channel->addLine(output, true);

    // Post it to the GUI if it's the current panel.
    if (channel == currentPanel)
    {
        gui->appendChatLine(output);
    }
    else
    {
        // if it was not the current panel, the panel's button needs to be updated.
        QString name = channel->name();
        QString styleSheet = channel->updateButtonColor();
        gui->updateButtonColor(name, styleSheet);
    }
}
void flist_messenger::postBroadcastMessage(QString& msg)
{
    // make post
    FBroadcastMessage post(msg);
    QString output = post.getOutput();
    // post it in all panels.
    foreach (FChannel* c, channelList)
    {
        if (c->getActive())
        {
            c->addLine(post.getOutput(), true);
        }
    }

    // Since it is posted in all channels, it can also simply be added to the current output.
    gui->appendChatLine(output);
}
void flist_messenger::postChatMessage(FChannel* channel, FCharacter* character, QString& msg)
{
    // make post
    FChatMessage post(channel, character, msg);
    QString output = post.getOutput();

    // post it in the panel
    channel->addLine(output, true);

    // Post it to the GUI if it's the current panel.
    if (channel == currentPanel)
    {
        gui->appendChatLine(output);
    }
    else
    {
        // if it was not the current panel, the panel's button needs to be updated.
        QString name = channel->name();
        QString styleSheet = channel->updateButtonColor();
        gui->updateButtonColor(name, styleSheet);
    }
}
void flist_messenger::postReportMessage(FCharacter* character, QString& msg)
{
    // make post
    FReportMessage post(character, msg);
    QString output = post.getOutput();
    // post it in all panels.
    foreach (FChannel* c, channelList)
    {
        if (c->getActive())
        {
            c->addLine(post.getOutput(), true);
        }
    }

    // Since it is posted in all channels, it can also simply be added to the current output.
    gui->appendChatLine(output);
}
void flist_messenger::postSystemMessage(FSystemMessage::SystemMessageType sysType, FChannel* channel, QString& msg)
{
    // make post
    FSystemMessage post(sysType, channel, msg);
    QString output = post.getOutput();

    // Post it to the channel, and to the console.
    channel->addLine(output, true);
    if (channel != console)
    {
        console->addLine(output, console);
    }

    // Add it to the chat output
    if (currentPanel == channel || currentPanel == console)
    {
        gui->appendChatLine(output);
    }
}

void flist_messenger::leaveChannel ( QString& channel, bool toServer )
{
    FChannel* chan = channelList[channel];
    if (chan == 0)
    {
        std::cout << "Leaving this will not work..." << std::endl;
        return;
    } else {
        std::cout << "Leaving " << chan->name().toStdString() << std::endl;
    }
    chan->emptyCharList();
    chan->setActive ( false );
    gui->showChannelButton(channel, false);
    QString con = console->name();
    switchTab(con);
    if ( toServer )
    {
        network->sendLeaveChannel(channel);
    }
}
void flist_messenger::joinChannel(QString &channel, bool toServer)
{
    network->sendJoinChannel(channel);
}

void flist_messenger::changeStatus ( QString& status, QString& statusmsg )
{
    selfStatus = status;
    selfStatusMessage = statusmsg;
    network->sendSetStatus(status, statusmsg);

    gui->setStatusButtonIcon(selfStatus);

    QString output = QString ( "Status changed successfully." );
    postSystemMessage(FSystemMessage::SYSTYPE_FEEDBACK, currentPanel, output);
}
// PORTED, DO NOT YET REMOVE
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
void flist_messenger::inputChanged()
{
/*
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
*/
}
void flist_messenger::typingCleared ( FChannel* channel )
{
    // TPN {"character":"Leon Priest","status":"clear"}
    QString character = channel->recipient();
    QString status = "clear";
    network->sendTypingChanged(character, status);
}
void flist_messenger::typingContinued ( FChannel* channel )
{
    // TPN {"character":"Leon Priest","status":"typing"}
    QString character = channel->recipient();
    QString status = "typing";
    network->sendTypingChanged(character, status);
}
void flist_messenger::typingPaused ( FChannel* channel )
{
    // TPN {"character":"Leon Priest","status":"paused"}
    QString character = channel->recipient();
    QString status = "paused";
    network->sendTypingChanged(character, status);
}

void flist_messenger::switchTab ( QString& tabname )
{
    FChannel* tempCurrent = currentPanel;
    if ( channelList.count ( tabname ) == 0)
    {
        printDebugInfo( "ERROR: Tried to switch to " + tabname.toStdString() + " but it doesn't exist.");
        return;
    }

    if ( currentPanel &&
         currentPanel->type() == FChannel::CHANTYPE_PM &&
         currentPanel->getTypingSelf() == FChannel::TYPINGSTATUS_TYPING )
    {
        typingPaused ( currentPanel );
    }

    FChannel* chan = channelList[tabname];
    currentPanel = chan;

    emit tabSwitched(tempCurrent, currentPanel);
}
void flist_messenger::openPMTab ( QString &character )
{
    if (character.toLower() == charName.toLower())
    {
        QString error = "You can't PM yourself!";
        postSystemMessage(FSystemMessage::SYSTYPE_FEEDBACK, currentPanel, error);
        return;
    }
    if ( characterList.count ( character ) == 0 )
    {
        QString error = "That character is not logged in.";
        postSystemMessage(FSystemMessage::SYSTYPE_FEEDBACK, currentPanel, error);
        return;
    }

    QString panelname = "PM-" + character;

    if ( channelList.count ( panelname ) != 0 )
    {
        channelList[panelname]->setActive(true);
        gui->showChannelButton(panelname);
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
        gui->addPanelButton(panelname, paneltitle);
        switchTab ( panelname );
    }
}
void flist_messenger::deleteIgnore(QString &name)
{
    name = name.toLower();
    if (selfIgnoreList.count(name))
    {
        network->sendIgnoreDelete(name);
    } else {
        QString out = name;
        out += " was not on your ignore list.";
        postSystemMessage(FSystemMessage::SYSTYPE_FEEDBACK, currentPanel, out);
    }
}
void flist_messenger::addIgnore(QString &name)
{
    name = name.toLower();
    if (selfIgnoreList.count(name) == 0)
    {
        network->sendIgnoreAdd(name);
    } else {
        QString out = name;
        out += " was already on your ignore list.";
        postSystemMessage(FSystemMessage::SYSTYPE_FEEDBACK, currentPanel, out);
    }
}
void flist_messenger::_channelBan(QString &name)
{
    QString channel = currentPanel->name();
    network->sendChannelBan(channel, name);
}
void flist_messenger::_channelKick(QString &name)
{
    QString channel = currentPanel->name();
    network->sendChannelKick(channel, name);
}
void flist_messenger::_chatBan(QString &name)
{
    network->sendBan(name);
}
void flist_messenger::_chatKick(QString &name)
{
    network->sendKick(name);
}
void flist_messenger::_addOp(QString &name)
{
    network->sendAddOp(name);
}
void flist_messenger::_deleteOp(QString &name)
{
    network->sendDeleteOp(name);
}
void flist_messenger::_addChanOp(QString &name)
{
    QString channel = currentPanel->name();
    network->sendChannelAddOp(channel, name);
}
void flist_messenger::_deleteChanOp(QString &name)
{
    QString channel = currentPanel->name();
    network->sendChannelDeleteOp(channel, name);
}
void flist_messenger::_timeout(QString &name, int time, QString &reason)
{
    name = name.simplified();
    reason = reason.simplified();
    if (time <= 0 || 90 < time) {
        QString error("Wrong length. Has to be between 1 and 90 minutes.");
        postSystemMessage(FSystemMessage::SYSTYPE_FEEDBACK, currentPanel, error);
    }
    else if (name == "" || reason == "")
    {
        QString error("Didn't fill out all fields.");
        postSystemMessage(FSystemMessage::SYSTYPE_FEEDBACK, currentPanel, error);
    } else {
        network->sendTimeout(name, time, reason);
    }
}
void flist_messenger::_makeRoom(QString &name)
{
    network->sendMakeRoom(name);
}
void flist_messenger::_setDescription(QString &channel, QString &description)
{
    network->sendSetDescription(channel, description);
}

//PORTED
void flist_messenger::btnSendChatClicked()
{
    // SLOT
    // parseInput();
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
    QString error;
    if ( inputText.length() == 0 )
    {
        error = "<b>Error:</b> No message.";
        postSystemMessage(FSystemMessage::SYSTYPE_FEEDBACK, currentPanel, error);
        return;
    }
    if ( currentPanel == 0 )
    {
        printDebugInfo("[CLIENT ERROR] currentPanel == 0");
        return;
    }
    if ( currentPanel == console || currentPanel->getMode() == FChannel::CHANMODE_CHAT || currentPanel->type() == FChannel::CHANTYPE_PM )
    {
        error = "<b>Error:</b> Can't advertise here.";
        postSystemMessage(FSystemMessage::SYSTYPE_FEEDBACK, currentPanel, error);
        return;
    }
    if ( inputText.length() > flist_messenger::BUFFERPUB )
    {
        error = "<B>Error:</B> Message exceeds the maximum number of characters.";
        postSystemMessage(FSystemMessage::SYSTYPE_FEEDBACK, currentPanel, error);
        return;
    }

    QString chan = currentPanel->name();
    QString message = inputText;

    postAdvertMessage(currentPanel, characterList[charName], ownText);
    gui->clearInput();
    network->sendAdvert( chan, message );
}

void flist_messenger::parseInput(QString &input)
{
    if (se_sounds)
        soundPlayer.play ( soundPlayer.SOUND_CHAT );

    bool pm = ( bool ) ( currentPanel->type() == FChannel::CHANTYPE_PM );
    bool isCommand = ( input[0] == '/' );

    if ( !isCommand && currentPanel == console )
        return;

    bool success = false;
    QString msg = 0;
    QString error = 0;
    QString gt = "&gt;";
    QString lt = "&lt;";
    QString amp = "&amp;";
    QString ownText = input;
    ownText.replace ( '&', amp ).replace ( '<', lt ).replace ( '>', gt );
    if ( input.length() == 0 )
    {
        error = "<b>Error:</b> No message.";
        postSystemMessage(FSystemMessage::SYSTYPE_FEEDBACK, currentPanel, error);
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

    if ( input.length() > buffer )
    {
        error = "<B>Error:</B> Message exceeds the maximum number of characters.";
        postSystemMessage(FSystemMessage::SYSTYPE_FEEDBACK, currentPanel, error);
        return;
    }

    if ( isCommand )
    {
        QStringList parts = input.split ( ' ' );

        if ( parts[0].toLower() == "/debug" )
        {
            QString command = parts[1];
            network->sendDebug(command);
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
            QString channel = input.mid ( 6, -1 ).simplified();
            network->sendJoinChannel(channel);
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
                QString channel = currentPanel->name();
                leaveChannel ( channel );
                success = true;
            }
        }
        else if ( parts[0].toLower() == "/status" )
        {
            QString status = parts[1];
            QString statusMsg = input.mid ( 9 + status.length(), -1 ).simplified();
            network->sendSetStatus(status, statusMsg);
            changeStatus ( status, statusMsg );
            success = true;
        }
        else if ( parts[0].toLower() == "/priv" )
        {
            QString character = input.mid ( 6 ).simplified();
            openPMTab ( character );
            success = true;
        }
        else if ( parts[0].toLower() == "/ignore" )
        {
            QString character = input.mid ( 8 ).simplified();

            if ( character != "" )
            {
                network->sendIgnoreAdd(character);
                success = true;
            }
        }
        else if ( parts[0].toLower() == "/unignore" )
        {
            QString character = input.mid ( 10 ).simplified();

            if ( character != "" )
            {
                if ( selfIgnoreList.count ( character ) == 0 )
                {
                    error = QString ( "This character is not in your ignore list." );
                    postSystemMessage(FSystemMessage::SYSTYPE_FEEDBACK, currentPanel, error);
                }
                else
                {
                    network->sendIgnoreDelete(character);
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
            QString character = input.mid(6).simplified();
            QString channel = currentPanel->name();
            network->sendChannelKick(channel, character);
            success = true;
        }
        else if ( parts[0].toLower() == "/gkick" )
        {
            // [16:22 PM]>>KIK {"character":"Tamu"}
            QString character = input.mid(7).simplified();
            network->sendKick(character);
            success = true;
        }
        else if ( parts[0].toLower() == "/ban" )
        {
            //[17:23 PM]>>CBU {"channel":"ADH-89ff2273b20cfc422ca1","character":"Viona"}
            QString channel = currentPanel->name();
            QString character = input.mid(5).simplified();
            network->sendChannelBan(channel, character);
            success = true;
        }
        else if ( parts[0].toLower() == "/accountban" )
        {
            //[22:42 PM]>>ACB {"character":"Mack"}
            QString character = input.mid(12).simplified();
            network->sendBan(character);
            success = true;
        }
        else if ( parts[0].toLower() == "/makeroom" )
        {
            // [17:24 PM]>>CCR {"channel":"abc"}
            QString name = input.mid(10).simplified();
            network->sendMakeRoom(name);
            success = true;
        }
        else if ( parts[0].toLower() == "/closeroom")
        {
            // [13:12 PM]>>RST {"channel":"ADH-68c2 7 1 4e731ccfbe0","status":"public"}
            QString channel = currentPanel->name();
            QString status = "private";
            network->sendRestrictRoom(channel, status);
            success = true;
        }
        else if ( parts[0].toLower() == "/openroom")
        {
            // [13:12 PM]>>RST {"channel":"ADH-68c2 7 1 4e731ccfbe0","status":"private"}
            QString channel = currentPanel->name();
            QString status = "public";
            network->sendRestrictRoom(channel, status);
            success = true;
        }
        else if ( parts[0].toLower() == "/invite" )
        {
            //[16:37 PM]>>CIU {"channel":"ADH-STAFFROOMFORSTAFFPPL","character":"Viona"}
            QString character = input.mid(8).simplified();
            QString channel = currentPanel->name();
            network->sendChannelInvite(channel, character);
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
            QString channel = currentPanel->name();
            QString character = input.mid(5).simplified();
            network->sendChannelAddOp(channel, character);
            success = true;
        }
        else if ( parts[0].toLower() == "/cdeop" )
        {
            //[16:27 PM]>>COR {"channel":"ADH-STAFFROOMFORSTAFFPPL","character":"Viona"}
            QString channel = currentPanel->name();
            QString character = input.mid(7).simplified();
            network->sendChannelDeleteOp(channel, character);
            success = true;
        }
        else if ( parts[0].toLower() == "/op" )
        {
            QString character = input.mid ( 4 ).simplified();
            network->sendAddOp(character);
            success = true;
        }
        else if ( parts[0].toLower() == "/reward" )
        {
            // [17:19 PM]>>RWD {"character":"Arisato Hamuko"}
            QString character = input.mid(8).simplified();
            network->sendReward(character);
            success = true;
        }
        else if ( parts[0].toLower() == "/deop" )
        {
            // [17:27 PM]>>DOP {"character":"Viona"}
            QString character = input.mid(6).simplified();
            network->sendDeleteOp(character);
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
            postSystemMessage(FSystemMessage::SYSTYPE_FEEDBACK, currentPanel, out);
            success = true;
        }
        else if ( parts[0].toLower() == "/unban" )
        {
            // [17:30 PM]>>CUB {"channel":"ADH-cbae3bdf02cd39e8949e","character":"Viona"}
            QString channel = currentPanel->name();
            QString character = input.mid(7).simplified();
            network->sendChannelUnban(channel, character);
            success = true;
        }
        else if ( parts[0].toLower() == "/banlist" )
        {
            // [17:30 PM]>>CBL {"channel":"ADH-cbae3bdf02cd39e8949e"}
            QString channel = currentPanel->name();
            network->sendRequestBanlist(channel);
            success = true;
        }
        else if ( parts[0].toLower() == "/setdescription" )
        {
            // [17:31 PM]>>CDS {"channel":"ADH-cbae3bdf02cd39e8949e","description":":3!"}
            QString channel = currentPanel->name();
            QString description = input.mid(16).simplified();
            network->sendSetDescription(channel, description);
            success = true;
        }
        else if ( parts[0].toLower() == "/coplist" )
        {
            // [17:31 PM]>>COL {"channel":"ADH-cbae3bdf02cd39e8949e"}
            QString channel = currentPanel->name();
            network->sendRequestCoplist(channel);
            success = true;
        }
        else if ( parts[0].toLower() == "/timeout" )
        {
            // [17:16 PM]>>TMO {"time":1,"character":"Arisato Hamuko","reason":"Test."}
            QStringList tparts = input.mid ( 9 ).split ( ',' );
            bool isInt;
            int time = tparts[1].simplified().toInt ( &isInt );

            if ( isInt == false )
            {
                QString error = "Time is not a number.";
                postSystemMessage(FSystemMessage::SYSTYPE_FEEDBACK, currentPanel, error);
            }
            else
            {
                QString character = tparts[0].simplified();
                QString reason = tparts[2].simplified();
                network->sendTimeout(character, time, reason);
                success = true;
            }
        }
        else if ( parts[0].toLower() == "/gunban" )
        {
            // [22:43 PM]>>UNB {"character":"Mack"}
            QString character = input.mid(8).simplified();
            network->sendUnban(character);
            success = true;
        }
        else if ( parts[0].toLower() == "/createchannel" )
        {
            // [0:59 AM]>>CRC {"channel":"test"}
            QString channel = input.mid(15).simplified();
            network->sendCreateChannel(channel);
            success = true;
        }
        else if ( parts[0].toLower() == "/killchannel" )
        {
            // [0:59 AM]>>KIC {"channel":"test"}
            QString channel = input.mid(13).simplified();
            network->sendKillChannel(channel);
            success = true;
        }
        else if ( parts[0].toLower() == "/broadcast" )
        {
            //[1:14 AM]>>BRO {"message":"test"}
            QString message = input.mid(11).simplified();
            network->sendBroadcast(message);
            success = true;
        }
        else if ( parts[0].toLower() == "/setmode" )
        {
            //[23:59 PM]>>RMO {"channel":"ADH-9bbe33158b12f525f422","mode":"chat"}
            if (input.length() < 10 || (parts[1].toLower() != "chat" && parts[1].toLower() != "ads" && parts[1].toLower() != "both") )
            {
                QString error = "Correct usage: /setmode &lt;chat|ads|both&gt;";
                postSystemMessage(FSystemMessage::SYSTYPE_FEEDBACK, currentPanel, error);
            }

            if (currentPanel->isOp(characterList[charName]) || characterList[charName]->isChatOp())
            {
                QString mode = input.mid(9);
                QString channel = currentPanel->name();
                network->sendSetMode(channel, mode);
            }
            else
            {
                QString error ="You can only do that in channels you moderate.";
                postSystemMessage(FSystemMessage::SYSTYPE_FEEDBACK, currentPanel, error);
            }
            success = true;
        }
        else if ( parts[0].toLower() == "/bottle" )
        {
            if (currentPanel == 0 || currentPanel->type() == FChannel::CHANTYPE_CONSOLE || currentPanel->type() == FChannel::CHANTYPE_PM)
            {
                QString error ="You can't use that in this panel.";
                postSystemMessage(FSystemMessage::SYSTYPE_FEEDBACK, currentPanel, error);
            }
            else
            {
                QString channel = currentPanel->name();
                QString dice = "bottle";
                network->sendRoll(channel, dice);
            }
            success = true;

        }
        else if ( parts[0].toLower() == "/roll" )
        {
            if (currentPanel == 0 || currentPanel->type() == FChannel::CHANTYPE_CONSOLE || currentPanel->type() == FChannel::CHANTYPE_PM)
            {
                QString error = "You can't use that in this panel.";
                postSystemMessage(FSystemMessage::SYSTYPE_FEEDBACK, currentPanel, error);
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
                QString channel = currentPanel->name();
                network->sendRoll(channel, roll);
            }
            success = true;
        }
        else if (debugging && parts[0].toLower() == "/channeltojson")
        {
            QString output("[noparse]");
            JSONNode* node = currentPanel->toJSON();
            output += node->write().c_str();
            output += "[/noparse]";
            postSystemMessage(FSystemMessage::SYSTYPE_FEEDBACK, currentPanel, output);
            delete node;
            success = true;
        }
        else if (debugging && parts[0].toLower() == "/refreshqss")
        {
            QFile stylefile("default.qss");
            stylefile.open(QFile::ReadOnly);
            QString stylesheet = QLatin1String(stylefile.readAll());
            gui->setStyleSheet(stylesheet);
            QString output = "Refreshed stylesheet from default.qss";
            postSystemMessage(FSystemMessage::SYSTYPE_FEEDBACK, currentPanel, output);
            success = true;
        }
        else if (parts[0].toLower() == "/channeltostring")
        {
            QString* output = currentPanel->toString();
            postSystemMessage(FSystemMessage::SYSTYPE_FEEDBACK, currentPanel, *output);
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
        gui->clearInput();
    }

    if ( msg != 0 )
    {
        if ( pm )
        {
            QString character = currentPanel->recipient();
            QString message(input.toUtf8());
            network->sendPrivateMessage( character, message );
            postChatMessage(currentPanel, characterList[charName], ownText);
        }
        else
        {
            QString chan = currentPanel->name();
            QString message(input.toUtf8());
            network->sendChatMessage( chan, message );
            postChatMessage(currentPanel, characterList[charName], ownText);
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


// PORTED
void flist_messenger::parseCommand ( std::string& input )
{
    try
    {
        std::cout << "<<" << input << std::endl;
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
            postBroadcastMessage(message);
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
            postSystemMessage(FSystemMessage::SYSTYPE_FEEDBACK, channelList[channel], msg);
        }
        else if ( cmd == "CHA" )
        {
            JSONNode childnode = nodes.at ( "channels" );
            int size = childnode.size();
            QList<ChannelListItem*> list;
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
                list.append(chan);
            }
            gui->fillChannelList(list);
        }
        else if ( cmd == "CIU" )
        {
            //CIU {"sender": "EagerToPlease", "name": "ADH-085bcf60bef81b0790b7", "title": "Domination and Degradation"}
            QString output;
            QString sender = nodes.at ( "sender" ).as_string().c_str();
            QString name = nodes.at ( "name" ).as_string().c_str();
            QString title = nodes.at ( "title" ).as_string().c_str();
            output = "<b>" + sender + "</b> has invited you to join the room <a href=\"#AHI-" + name + "\">" + title + "</a>.";
            postSystemMessage(FSystemMessage::SYSTYPE_FEEDBACK, currentPanel, output);
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
                    QString chanstr = channel->name();
                    leaveChannel(chanstr, false);
                    postSystemMessage(FSystemMessage::SYSTYPE_KICKBAN, currentPanel, output);
                } else {
                    postSystemMessage(FSystemMessage::SYSTYPE_KICKBAN, currentPanel, output);
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
                    QString chanstr = channel->name();
                    leaveChannel(chanstr, false);
                    postSystemMessage(FSystemMessage::SYSTYPE_KICKBAN, currentPanel, output);
                } else {
                    postSystemMessage(FSystemMessage::SYSTYPE_KICKBAN, currentPanel, output);
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
            //FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, currentPanel, 0, msg, currentPanel);
            gui->appendChatLine(msg);
        }
        else if ( cmd == "ERR" )
        {
            QString output;
            QString message = nodes.at ( "message" ).as_string().c_str();
            QString number = nodes.at ( "number" ).as_string().c_str();
            output = "<b>Error " + number + ": </b>" + message;
            postSystemMessage(FSystemMessage::SYSTYPE_FEEDBACK, currentPanel, output);

//			if(number == "34")
//			{
//				JSONNode loginnode;
//				JSONNode tempnode ( "method", "ticket" );
//				loginnode.push_back ( tempnode );
//				tempnode.set_name ( "account" );
//				tempnode = username.toStdString();
//				loginnode.push_back ( tempnode );
//				tempnode.set_name ( "character" );
//				tempnode = charName.toStdString();
//				loginnode.push_back ( tempnode );
//				tempnode.set_name ( "ticket" );
//                tempnode = loginTicket.toStdString();
//				loginnode.push_back ( tempnode );
//				tempnode.set_name ( "cname" );
//				tempnode = CLIENTID;
//				loginnode.push_back ( tempnode );
//				tempnode.set_name ( "cversion" );
//				tempnode = VERSIONNUM;
//				loginnode.push_back ( tempnode );
//				std::string idenStr = "IDN " + loginnode.write();
//				network->sendWS ( idenStr );
//			}

        }
        else if ( cmd == "FLN" )
        {
            QString remchar = nodes.at ( "character" ).as_string().c_str();
            bool posted = false;
            QString offline = "<b>" + remchar + "</b> has disconnected.";
            if ( se_onlineOffline && selfFriendsList.contains ( remchar ) )
            {
                postSystemMessage(FSystemMessage::SYSTYPE_ONLINE, currentPanel, offline);
                posted = true;
            }
            QString pmPanel = "PM-" + remchar;
            if (channelList.count(pmPanel))
            {
                channelList[pmPanel]->setTyping ( FChannel::TYPINGSTATUS_CLEAR );
                if (posted == false || channelList[pmPanel] != currentPanel)
                    postSystemMessage(FSystemMessage::SYSTYPE_ONLINE, channelList[pmPanel], offline);

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
                        postSystemMessage(FSystemMessage::SYSTYPE_JOIN, *iter, output);
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
            gui->appendChatLine(msg);
            //			FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, currentPanel, 0, msg, currentPanel);

            //			foreach (QString s, defaultChannels)
            //			{
            //				joinChannel(s);
            //			}
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
            channel->sortChars();
            gui->refreshUserlist(currentPanel);
        }
        else if ( cmd == "IDN" )
        {
            QString msg = "<B>";
            msg += nodes["character"].as_string().c_str();
            msg += "</B> Connected.";
            gui->appendChatLine(msg);
            //			FMessage fmsg(FMessage::SYSTYPE_FEEDBACK, console, 0, msg, console);
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
                postSystemMessage(FSystemMessage::SYSTYPE_FEEDBACK, currentPanel, out);
            }
            if ( nodes["action"].as_string() == "delete" )
            {
                QString character = nodes["character"].as_string().c_str();
                selfIgnoreList.removeAll ( character );
                QString out = character + QString ( " has been removed from your ignore list." );
                postSystemMessage(FSystemMessage::SYSTYPE_FEEDBACK, currentPanel, out);
            }
        }
        else if ( cmd == "JCH" )
        {
            QString channel = nodes.at ( "channel" ).as_string().c_str();
            if (channelList.count(channel) == 0)
            {
                QString adh = "ADH-";
                if ( channel.startsWith ( adh ) )
                {
                    QString channelTitle = nodes.at ( "title" ).as_string().c_str();
                    joinedChannel(channel, channelTitle);
                }
                else
                {
                    joinedChannel(channel, channel);
                }
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
            if ( currentPanel->name() == channel )
                gui->refreshUserlist(currentPanel);
            if (se_leaveJoin)
            {
                QString output = "<b>";
                output += charname;
                output += "</b> has joined the channel.";
                postSystemMessage(FSystemMessage::SYSTYPE_JOIN, channelList[channel], output);
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
                gui->ci_clearKinks();
            }
            else if ( type == "custom" )
            {
                QString out;
                QString value = nodes.at ( "value" ).as_string().c_str();
                QString key = nodes.at ( "key" ).as_string().c_str();
                out = QString ( "<b>" ) + key + QString ( ":</b> " ) + value;
                gui->ci_addToKinks(out);
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
                leaveChannel ( channel, false );
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
                    postSystemMessage(FSystemMessage::SYSTYPE_JOIN, channelList[channel], output);
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
            postAdvertMessage(channel, chanchar, message);
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

            postChatMessage(channel, chanchar, message);
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
                postSystemMessage(FSystemMessage::SYSTYPE_ONLINE, currentPanel, online);
                posted = true;
            }
            QString pmPanel = "PM-" + addchar;
            if (channelList.count(pmPanel))
            {
                if (posted == false || channelList[pmPanel] != currentPanel)
                    postSystemMessage(FSystemMessage::SYSTYPE_ONLINE, channelList[pmPanel], online);
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
            //			ORS
            //	{"channels": [
            //	 {"name": "ADH-29a2ec641d78e5bd197e", "characters": "1", "title": "Eifania's Little Room"},
            //	 {"name": "ADH-74e4caef2965f4b33dd4", "characters": "1", "title": "Acrophobia"},
            //	 {"name": "ADH-fa132c6f2740c5ebaed7", "characters": "10", "title": "Femboy Faggot Fucksluts"}
            //	]}
            QList<ChannelListItem*> list;
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
                list.append(chan);
            }
            gui->fillProomList(list);
        }
        else if ( cmd == "PIN" )
        {
            network->sendPing();
        }
        else if ( cmd == "PRD" )
        {
            QString type = nodes.at ( "type" ).as_string().c_str();

            if ( type == "start" )
            {
                gui->ci_clearProfile();
            }
            else if ( type == "info" )
            {
                QString out;
                QString value = nodes.at ( "value" ).as_string().c_str();
                QString key = nodes.at ( "key" ).as_string().c_str();
                out = QString ( "<b>" ) + key + QString ( ":</b> " ) + value;
                gui->ci_addToProfile(out);
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
                //				network->sendWS ( out );
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
            postSystemMessage(FSystemMessage::SYSTYPE_FEEDBACK, channel, output);
        }
        else if ( cmd == "RTB" )
        {
            // RTB {"type":"note","sender":"Viona","subject":"test"}
        }
        else if ( cmd == "SFC" )
        {
            // A staff report
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
                postReportMessage(characterList[character], output);
            }
            else if ( action == "confirm" )
            {
                output = "<b>";
                output += nodes.at ( "moderator" ).as_string().c_str();
                output += "</b> is handling <b>";
                output += nodes.at ( "character" ).as_string().c_str();
                output += "</b>\'s report.";
                postSystemMessage(FSystemMessage::SYSTYPE_FEEDBACK, currentPanel, output);
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

                    postSystemMessage(FSystemMessage::SYSTYPE_ONLINE, currentPanel, statusline);
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

            gui->refreshUserlist(currentPanel);
        }
        else if ( cmd == "RLL" )
        {
            // Dice rolling or bottling.
            QString message = nodes.at("message").as_string().c_str();
            QString channelname = nodes.at("channel").as_string().c_str();
            FChannel* channel = channelList[channelname];
            if (channel)
            {
                postSystemMessage(FSystemMessage::SYSTYPE_DICE, channel, message);
            }
        }
        else if ( cmd == "SYS" )
        {
            QString output;
            QString message = nodes.at ( "message" ).as_string().c_str();
            output = "<b>System message:</b> " + message;
            postSystemMessage(FSystemMessage::SYSTYPE_FEEDBACK, currentPanel, output);
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
            chan->setMode(FChannel::CHANMODE_ADS);
            postSystemMessage(FSystemMessage::SYSTYPE_FEEDBACK, chan, output);
        }
        else if ( cmd == "ZZZ" )
        {
            QString output;
            QString message = nodes.at ( "message" ).as_string().c_str();
            output = "<b>Debug Reply:</b> " + message;
            postSystemMessage(FSystemMessage::SYSTYPE_FEEDBACK, currentPanel, output);
        }
        else
        {
            printDebugInfo("Unparsed command: " + input);
            QString qinput = "Unparsed command: ";
            qinput += input.c_str();
            postSystemMessage(FSystemMessage::SYSTYPE_FEEDBACK, currentPanel, qinput);
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

void flist_messenger::friendsDialogRequested()
{
    gui->fillFriendList(selfFriendsList, selfIgnoreList);
}
void flist_messenger::channelsDialogRequested()
{
    gui->channelsDialogRequested();
    network->sendChannelsRequest();
    network->sendProomsRequest();
}
void flist_messenger::characterDialogRequested(QString& name)
{
    network->sendRequestCharacterInfo(name);
}

#include "flist_messenger.moc"
