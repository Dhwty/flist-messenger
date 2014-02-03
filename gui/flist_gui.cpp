#include "flist_gui.h"

FGui::FGui(flist_messenger *parent, FSettings *settings) :
    QMainWindow()
{
    notificationsAreaMessageShown = false;
    activePanelButtons = QHash<QString, QPushButton*>();
    activePanelsSpacer = new QSpacerItem ( 0, 20, QSizePolicy::Ignored, QSizePolicy::Expanding );
    makeRoomDialog = 0;
    setStatusDialog = 0;
    reportDialog = 0;
    helpDialog = 0;
    settingsDialog = 0;
    channelSettingsDialog = 0;
    timeoutDialog = 0;
    characterInfoDialog = 0;
    btnConnect = 0;
    friendsDialog = 0;
    channelsDialog = 0;
    ul_recent = 0;
    tb_recent = 0;
    recentCharMenu = 0;
    recentChannelMenu = 0;
    trayIcon = 0;
    trayIconMenu = 0;
    addIgnoreDialog = 0;
    soundPlayer = new FSound();

    connect(this, SIGNAL(friendListRequested()), parent, SLOT(friendsDialogRequested()));
    connect(this, SIGNAL(channelListRequested()), parent, SLOT(channelsDialogRequested()));
    connect(this, SIGNAL(characterInfoRequested(QString&)), parent, SLOT(characterDialogRequested(QString&)));

    this->parent = parent;
    this->setupConnectBox(settings->getUsername(), settings->getPassword());
    this->show();
}

void FGui::messageBox(QString &message, MessageBoxType type)
{
    QMessageBox box;
    switch (type)
    {
    case MSGTYPE_ERROR:
        box.setWindowTitle(QString("System error!"));
        break;
    }
    box.setText(message);
    box.show();
}

void FGui::setupRealUI()
{
    setObjectName ( "MainWindow" );
    resize ( 836, 454 );
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
    connect(btnFriends, SIGNAL ( clicked() ), this, SLOT ( friendsDialogRequested() ) );
    connect(btnChannels, SIGNAL ( clicked() ), this, SLOT ( btnChannelsClicked() ) );
    connect(btnSettings, SIGNAL(clicked()), this, SLOT(settingsDialogRequested()));
    connect(btnReport, SIGNAL(clicked()), this, SLOT(reportDialogRequested()));
    connect(btnMakeRoom, SIGNAL ( clicked() ), this, SLOT ( makeRoomDialogRequested() ) );
    connect(btnSetStatus, SIGNAL ( clicked() ), this, SLOT ( setStatusDialogRequested() ) );
    textEdit = new QTextBrowser;
    textEdit->setOpenLinks(false);
    textEdit->setObjectName ( "chatoutput" );
    textEdit->setContextMenuPolicy ( Qt::DefaultContextMenu );
    textEdit->setDocumentTitle ( "" );
    textEdit->setReadOnly ( true );
    textEdit->setFrameShape ( QFrame::NoFrame );
    /*
    connect ( textEdit, SIGNAL ( anchorClicked ( QUrl ) ), this, SLOT ( anchorClicked ( QUrl ) ) );
    */
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
//    connect(btnSendAdv, SIGNAL(clicked()), this, SLOT(btnSendAdvClicked()));
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
//    connect(actionHelp, SIGNAL(triggered()), this, SLOT(helpDialogRequested()));
//    connect ( actionAbout, SIGNAL ( triggered() ), this, SLOT ( aboutApp() ) );
    connect ( actionQuit, SIGNAL ( triggered() ), this, SLOT ( quitApp() ) );
    int wid = QApplication::desktop()->width();
    int hig = QApplication::desktop()->height();
    int mwid = width();
    int mhig = height();
    setGeometry ( ( wid / 2 ) - ( int ) ( mwid*0.5 ), ( hig / 2 ) - ( int ) ( mhig*0.5 ), mwid, mhig );

    //Setup the console button.
    QString name = "CONSOLE";
    QString tooltip = "View the console!";
    addPanelButton(name, tooltip);
    emit consoleButtonReady();
}

void FGui::setupConnectBox(QString &username, QString &password)
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
    lineEdit = new QLineEdit(password);
    lineEdit->installEventFilter(loginreturn);
    lineEdit->setEchoMode ( QLineEdit::Password );
    lineEdit->setObjectName ( QString ( "passwordInput" ) );
    gridLayout->addWidget ( lineEdit, 1, 1 );

    // The login button
    btnConnect = new QPushButton;
    btnConnect->setObjectName ( QString ( "loginButton" ) );
    btnConnect->setText ( "Login" );
    btnConnect->setIcon ( QIcon ( ":/images/tick.png" ) );
    gridLayout->addWidget ( btnConnect, 2, 1 );
    verticalLayout->addLayout ( gridLayout );
    this->setCentralWidget ( verticalLayoutWidget );
    connect ( btnConnect, SIGNAL ( clicked() ), this, SLOT ( btnConnectClicked() ) );

    int wid = QApplication::desktop()->width();
    int hig = QApplication::desktop()->height();
    int mwid = 265;
    int mhig = 100;
    setGeometry ( ( wid / 2 ) - ( int ) ( mwid*0.5 ), ( hig / 2 ) - ( int ) ( mhig*0.5 ), mwid, mhig );
}
void FGui::connectError(QString &error)
{
    if (btnConnect)
        btnConnect->setEnabled(true);
    QString title = "Login error!";
    QMessageBox::critical(this, title, error);
}

void FGui::clearConnectBox()
{
    btnConnect = 0;
    verticalLayoutWidget->deleteLater();
}

void FGui::flashApp()
{
    QApplication::alert(this, 10000);
}

void FGui::setupLoginBox(QList<QString>& characterList, QString& defaultCharacter)
{
    doSetupLoginBox(characterList, defaultCharacter);
}

void FGui::doSetupLoginBox(QList<QString>& characterList, QString& defaultCharacter)
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

    for ( int i = 0;i < characterList.count();++i )
    {
        comboBox->addItem ( characterList[i] );

        if ( characterList[i] == defaultCharacter )
        {
            comboBox->setCurrentIndex ( i );
        }
    }

    label = new QLabel ( groupBox );

    label->setObjectName ( QString::fromUtf8 ( "charlabel" ) );
    label->setGeometry ( QRect ( 10, 13, 71, 21 ) );
    label->setText ( "Character:" );
    setCentralWidget ( groupBox );
    connect ( pushButton, SIGNAL ( clicked() ), this, SLOT ( btnLoginClicked() ) );
    int wid = QApplication::desktop()->width();
    int hig = QApplication::desktop()->height();
    int mwid = 265;
    int mhig = height();
    setGeometry ( ( wid / 2 ) - ( int ) ( mwid*0.5 ), ( hig / 2 ) - ( int ) ( mhig*0.5 ), mwid, mhig );
}
void FGui::clearLoginBox()
{
    groupBox->deleteLater();
}

void FGui::appendChatLine(QString &line)
{
    textEdit->append(line);
}

QPushButton* FGui::addPanelButton(QString &name, QString &tooltip)
{
    pushButton = activePanelButtons[name];

    if ( pushButton != 0 )
    {
        pushButton->setVisible(true);
    }
    else
    {
        activePanelsContents->removeItem ( activePanelsSpacer );
        pushButton = new QPushButton();
        pushButton->setObjectName ( name );
        pushButton->setGeometry ( QRect ( 0, 0, 30, 30 ) );
        pushButton->setFixedSize ( 30, 30 );
        pushButton->setStyleSheet ( "background-color: rgb(255, 255, 255);" );
        pushButton->setAutoFillBackground ( true );
        pushButton->setCheckable ( true );
        pushButton->setChecked ( false );
        pushButton->setToolTip ( tooltip );
        pushButton->setContextMenuPolicy(Qt::CustomContextMenu);
        activePanelButtons[name] = pushButton;
        connect ( pushButton, SIGNAL ( clicked() ), parent, SLOT ( channelButtonClicked() ) );
        connect ( pushButton, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(tb_channelRightClicked(QPoint)));
        activePanelsContents->addWidget ( pushButton, 0, Qt::AlignTop );

        if ( name.length() > 4 && name.toStdString().substr ( 0, 3 ) == "PM-" )
        {
            avatarFetcher.applyAvatarToButton ( pushButton, QString ( name.toStdString().substr ( 3, name.length() - 3 ).c_str() ) );
            //pushButton->setIconSize(pushButton->iconSize()*1.5);
        }
        else if ( name.length() > 5 && name.toStdString().substr ( 0, 4 ) == "ADH-" )
        {
            pushButton->setIcon ( QIcon ( ":/images/key.png" ) );
        }
        else if (name == "CONSOLE")
        {
            pushButton->setIcon ( QIcon( ":/images/terminal.png" ));
        }
        else
        {
            pushButton->setIcon ( QIcon ( ":/images/hash.png" ) );
        }

        activePanelsContents->addSpacerItem ( activePanelsSpacer );
    }
    return pushButton;
}

void FGui::addToFriendsList(QListWidgetItem *lwi)
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
void FGui::addToIgnoreList(QListWidgetItem *lwi)
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
void FGui::addToProomsDialogList(ChannelListItem *cli)
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
void FGui::addToChannelsDialogList(ChannelListItem *cli)
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

void FGui::refreshUserlist(FChannel *currentPanel)
{
    if ( currentPanel == 0 )
        return;

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

void FGui::fillFriendList(QList<QString> &friends, QList<QString> &ignores)
{
    QString s;
    FCharacter* f = 0;
    QListWidgetItem* lwi = 0;
    fr_lwFriends->clear();

    foreach ( s, friends )
    {
        if ( (f = parent->getCharacter(s)) != 0 )
        {
            lwi = new QListWidgetItem ( * ( f->statusIcon() ), f->name() );
            addToFriendsList ( lwi );
        }
    }

    fr_lwIgnore->clear();

    foreach ( s, ignores )
    {
        lwi = new QListWidgetItem ( s );
        addToIgnoreList ( lwi );
    }
}
void FGui::fillChannelList(QList<ChannelListItem *> &channels)
{
    cd_channelsList->clear();
    foreach (ChannelListItem* item, channels)
    {
        addToChannelsDialogList(item);
    }
}
void FGui::fillProomList(QList<ChannelListItem *> &prooms)
{
    cd_proomsList->clear();
    foreach (ChannelListItem* item, prooms)
    {
        addToProomsDialogList(item);
    }
}

void FGui::updateButtonColor(QString &channel, QString &color)
{
    activePanelButtons[channel]->setStyleSheet( color );
}
void FGui::showChannelButton(QString &name, bool visible)
{
    QPushButton* btn = activePanelButtons[name];
    if (!btn)
        return;
    btn->setVisible(visible);
}
void FGui::setStatusButtonIcon(QString &status)
{
    if ( status == "online" )
        btnSetStatus->setIcon ( QIcon ( ":/status-white.png" ) );
    else if ( status == "busy" )
        btnSetStatus->setIcon ( QIcon ( ":/status-orange.png" ) );
    else if ( status == "dnd" )
        btnSetStatus->setIcon ( QIcon ( ":/status-red.png" ) );
    else if ( status == "looking" )
        btnSetStatus->setIcon ( QIcon ( ":/status-green.png" ) );
    else if ( status == "away" )
        btnSetStatus->setIcon ( QIcon ( ":/status-yellow.png" ) );
}

void FGui::clearInput()
{
    plainTextEdit->clear();
}

void FGui::clearOutput()
{
    textEdit->clear();
}
void FGui::ci_addToKinks(QString &kink)
{
    kink += "<br/>";
    ci_teKinks->append(kink);
}
void FGui::ci_addToProfile(QString &tag)
{
    tag += "<br/>";
    ci_teProfile->append(tag);
}
void FGui::ci_clearKinks()
{
    ci_teKinks->clear();
}
void FGui::ci_clearProfile()
{
    ci_teKinks->clear();
}

void FGui::createTrayIcon()
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

void FGui::destroyMenu()
{
    if ( recentCharMenu )
        recentCharMenu->deleteLater();
}
void FGui::destroyChanMenu()
{
    if (recentChannelMenu)
        recentChannelMenu->deleteLater();
}

void FGui::setupFriendsDialog()
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

void FGui::setupChannelsUI()
{
    if ( channelsDialog )
    {
        channelsDialog->deleteLater();
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

void FGui::setupCharacterInfoUI()
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
void FGui::setupTimeoutDialog()
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
void FGui::setupSettingsDialog()
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

void FGui::setupMakeRoomUI()
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

void FGui::setupSetStatusUI()
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
    ss_cbStatus->addItem ( QIcon ( ":/status-white.png" ), QString ( "Online" ) );
    ss_cbStatus->addItem ( QIcon ( ":/status-green.png" ), QString ( "Looking for play!" ) );
    ss_cbStatus->addItem ( QIcon ( ":/status-yellow.png" ), QString ( "Away" ));
    ss_cbStatus->addItem ( QIcon ( ":/status-orange.png" ), QString ( "Busy" ) );
    ss_cbStatus->addItem ( QIcon ( ":/status-red.png" ), QString ( "Do not disturb" ) );

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
void FGui::setupReportDialog()
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
void FGui::setupHelpDialog()
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
    str+= "For bug reports, PM Viona or post <a href=\"#LNK-https://www.f-list.net/forum.php?forum=1698\">here</a>.<br />";
    str+= "(Please do not use the helpdesk or contact other staff for this.)<br /><br />";
    str+= "Thank you for using the messenger's beta version. For updates, regularly check the F-chat Desktop Client group forums.<br />";
    str+= "To get debug output, run the application with the \"-d\" argument.<br />";
    he_teHelp->setHtml(str);
    helpDialog->resize(500, 400);
    connect(he_teHelp, SIGNAL(anchorClicked(QUrl)), this, SLOT(anchorClicked(QUrl)));
}
bool FGui::setupChannelSettingsDialog()
{
/*
    if (tb_recent->type() == FChannel::CHANTYPE_PM)
    {
        // Setup for PM
        //if (characterList.count(tb_recent->recipient()) == 0)
        //    return false;
        if(!parent->haveCharacter(tb_recent->recipient())) {
            // Recipient is offline
            return false;
        }
        channelSettingsDialog = new QDialog(this);
        FCharacter* ch = parent->getCharacter(tb_recent->recipient()); //characterList[tb_recent->recipient()];
        cs_chanCurrent = tb_recent;
        channelSettingsDialog->setWindowTitle(ch->name());
        channelSettingsDialog->setWindowIcon(tb_recent->pushButton->icon());
        cs_qsPlainDescription = ch->statusMsg();
        cs_vblOverview = new QVBoxLayout;
        cs_gbDescription = new QGroupBox("Status");
        cs_vblDescription = new QVBoxLayout;
        QLabel* lblStatus = new QLabel(ch->statusString());
        cs_tbDescription = new QTextBrowser;
        cs_tbDescription->setHtml(parent->getParser()->parse(cs_qsPlainDescription));
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
        return true;
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
        cs_tbDescription->setHtml(parent->getParser()->parse(cs_qsPlainDescription));
        cs_teDescription = new QTextEdit;
        cs_teDescription->setPlainText(cs_qsPlainDescription);
        cs_teDescription->hide();
        cs_chbEditDescription = new QCheckBox("Editable mode (OPs only)");
        if ( ! ( ch->isOp(parent->me()) || parent->me()->isChatOp() ) )
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
        return true;
    }
    */
    std::cout << "Channel settings disabled due to crashing." << std::endl;
    return false;
}

//==================================================================
//
// SLOTS
//
//==================================================================


void FGui::friendsDialogRequested()
{
    if ( friendsDialog == 0 )
        setupFriendsDialog();

    // Fill lists
    emit friendListRequested();
    friendsDialog->show();
}
void FGui::channelsDialogRequested()
{
    if ( channelsDialog == 0 )
        setupChannelsUI();

    channelsDialog->show();
}
void FGui::characterInfoDialogRequested()
{
    FCharacter* ch = ul_recent;
    QString name = ch->name();
    if ( characterInfoDialog == 0 )
        setupCharacterInfoUI();

    emit characterInfoRequested(name);

    QString n = "<b>";
    n += ch->name();
    n += "</b> (";
    n += ch->statusString();
    n += ")";
    ci_lblName->setText ( n );
    ci_lblStatusMessage->setText ( ch->statusMsg() );
    characterInfoDialog->show();
}
void FGui::timeoutDialogRequested(QString &name)
{
    if (timeoutDialog == 0 )
        setupTimeoutDialog();
    to_leWho->setText(name);
    timeoutDialog->show();
}

void FGui::helpDialogRequested()
{
    if (helpDialog == 0 )
        setupHelpDialog();
    helpDialog->show();
}
void FGui::channelSettingsDialogRequested()
{
    // This is one that always needs to be setup, because its contents may vary.
    if (setupChannelSettingsDialog())
        channelSettingsDialog->show();
}
void FGui::settingsDialogRequested()
{
    if (settingsDialog == 0)
        setupSettingsDialog();

    FSettings* settings = parent->getSettings();

    se_chbHelpdesk->setChecked(settings->showHelpdesk());
    se_chbEnablePing->setChecked(settings->doPing());
    se_chbLeaveJoin->setChecked(settings->postLeaveJoin());
    se_chbAlwaysPing->setChecked(settings->doAlwaysPing());
    se_chbMute->setChecked(!settings->doSounds());
    se_chbEnableChatLogs->setChecked(settings->doChatLogs());
    se_chbOnlineOffline->setChecked(settings->postOnlineOffline());

    QString liststr = "";
    QStringList* selfPingList = settings->getPingList();
    foreach (QString s, *selfPingList)
    {
        liststr += s;
        liststr += ", ";
    }
    se_lePingList->setText(liststr.left(liststr.length()-2));
    settingsDialog->show();
}
void FGui::makeRoomDialogRequested()
{
    if ( makeRoomDialog == 0)
        setupMakeRoomUI();

    makeRoomDialog->show();
}
void FGui::setStatusDialogRequested()
{
    if ( setStatusDialog == 0)
        setupSetStatusUI();
    const QString *msg = parent->getStatusMessage();
    ss_leMessage->setText ( *msg );

    setStatusDialog->show();
}
void FGui::reportDialogRequested()
{
    if (reportDialog == 0)
        setupReportDialog();

    re_leWho->setText("None");
    re_teProblem->clear();
    reportDialog->show();

}
void FGui::btnChannelsClicked()
{
    emit channelListRequested();
}

void FGui::iconActivated(QSystemTrayIcon::ActivationReason reason)
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

void FGui::insertLineBreak()
{
    if (textEdit)
        plainTextEdit->insertPlainText("\n");
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
                static_cast<FGui*>(parent())->insertLineBreak();
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
                static_cast<FGui*> ( parent() )->enterPressed();
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
            static_cast<FGui*>(parent())->btnConnectClicked();
        default:
            return QObject::eventFilter(obj, event);
        }
    } else {
        return QObject::eventFilter(obj, event);
    }
}

void FGui::aboutApp()
{
    QMessageBox::about ( this, "About F-List Messenger", "Created by:\n* Viona\n* Kira\n* Aniko\n* Hexxy\n* Eager\n\nCopyright(c) 2010-2011 F-list Team" );
}
void FGui::quitApp()
{
    QApplication::quit();
}

void FGui::btnConnectClicked()
{
    btnConnect->setEnabled(false);
    lineEdit = this->findChild<QLineEdit *> ( QString ( "accountNameInput" ) );
    QString username = lineEdit->text();
    std::cout << username.toStdString() << std::endl;
    lineEdit = this->findChild<QLineEdit *> ( QString ( "passwordInput" ) );
    QString password = lineEdit->text();
    parent->prepareLogin ( username, password );
}

void FGui::btnLoginClicked()
{
    soundPlayer->play ( FSound::SOUND_LOGIN );
    QString charName = comboBox->currentText();

    clearLoginBox();
    setupRealUI();

    FMessage::selfName = charName;
    parent->setupSocket(charName);
}
void FGui::enterPressed()
{
    QString input = plainTextEdit->toPlainText();
    parent->parseInput(input);
}
void FGui::tabSwitched(FChannel *from, FChannel *to)
{
    // Save input to the fromchannel
    QString input = plainTextEdit->toPlainText();
    from->setInput(input);
    // Get input from the tochannel, put it in the chat input area
    input = to->getInput();
    plainTextEdit->setPlainText(input);

    // Set label
    lblChannelName->setText(to->title());
    // Check button, uncheck previous button.
    to->setHighlighted(false);
    to->setHasNewMessages(false);
    QString style = to->updateButtonColor();
    activePanelButtons[from->name()]->setChecked(false);
    activePanelButtons[to->name()]->setStyleSheet(style);
    activePanelButtons[to->name()]->setChecked(true);

    // Refresh userlist and output of the channel.
    clearOutput();
    to->printChannel(textEdit);
    refreshUserlist(to);
    plainTextEdit->setFocus();
}
void FGui::closeEvent(QCloseEvent *event)
{
    if (parent->getDisconnected())
        quitApp();
    if (trayIcon && trayIcon->isVisible())
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

void FGui::userListContextMenuRequested ( const QPoint& point )
{
    QListWidgetItem* lwi = listWidget->itemAt ( point );

    if ( lwi )
    {
        FCharacter* ch = parent->getCharacter(lwi->text());
        ul_recent = ch;
        displayCharacterContextMenu ( ch );
    }
}

void FGui::displayChannelContextMenu(FChannel *ch)
{
    if (!ch)
    {
        return;
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

void FGui::displayCharacterContextMenu ( FCharacter* ch )
{
    if ( !ch )
    {
        parent->printDebugInfo("ERROR: Tried to display a context menu for a null pointer character.");
    }
    else
    {
        QMenu* menu = new QMenu ( this );
        recentCharMenu = menu;
        menu->addAction ( QIcon ( ":/images/users.png" ), QString ( "Private Message" ), this, SLOT ( ul_pmRequested() ) );
        menu->addAction ( QIcon ( ":/images/book-open-list.png" ), QString ( "F-list Profile"), this, SLOT ( ul_profileRequested() ) );
        menu->addAction ( QIcon ( ":/images/tag-label.png" ), QString ( "Quick Profile" ), this, SLOT ( ul_infoRequested() ) );
        if (parent->isIgnored(ch->name()))
            menu->addAction ( QIcon ( ":/images/heart.png" ), QString ( "Unignore" ), this, SLOT(ul_ignoreRemove()) );
        else
            menu->addAction ( QIcon ( ":/images/heart-break.png" ), QString ( "Ignore" ), this, SLOT(ul_ignoreAdd()) );
        bool op = parent->me()->isChatOp();
        if (op)
        {
            menu->addAction ( QIcon ( ":/images/fire.png" ), QString ( "Chat Kick" ), this, SLOT(ul_chatKick()) );
            menu->addAction ( QIcon ( ":/images/auction-hammer--exclamation.png" ), QString ( "Chat Ban" ), this, SLOT(ul_chatBan()) );
            menu->addAction ( QIcon ( ":/images/alarm-clock.png" ), QString ( "Timeout..." ), this, SLOT(timeoutDialogRequested()) );
        }
        if (op || parent->getCurrentPanel()->isOwner(parent->me()))
        {
            if (parent->getCurrentPanel()->isOp(ch))
                menu->addAction ( QIcon ( ":/images/auction-hammer--minus.png" ), QString ( "Remove Channel OP" ), this, SLOT(ul_channelOpRemove()) );
            else
                menu->addAction ( QIcon ( ":/images/auction-hammer--plus.png" ), QString ( "Add Channel OP" ), this, SLOT(ul_channelOpAdd()) );
        }
        if ((op || parent->getCurrentPanel()->isOp(parent->me())) && !ch->isChatOp())
        {
            menu->addAction ( QIcon ( ":/images/lightning.png" ), QString ( "Channel Kick" ), this, SLOT(ul_channelKick()) );
            menu->addAction ( QIcon ( ":/images/auction-hammer.png" ), QString ( "Channel Ban" ), this, SLOT(ul_channelBan()) );
        }
        connect ( menu, SIGNAL ( aboutToHide() ), this, SLOT ( destroyMenu() ) );
        menu->exec ( QCursor::pos() );
    }
}

void FGui::channelButtonMenuRequested()
{
    displayChannelContextMenu(tb_recent);
}
void FGui::addIgnoreDialogRequested()
{
    if ( addIgnoreDialog == 0 )
    {
        std::cout << "Setting it up!" << std::endl;
        setupAddIgnoreDialog();
    }

    ai_leName->setText ( QString ( "" ) );
    addIgnoreDialog->show();
}
void FGui::setupAddIgnoreDialog()
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
void FGui::ai_btnCancelClicked()
{
    addIgnoreDialog->hide();
}
void FGui::ai_btnSubmitClicked()
{
    QString character = ai_leName->text().simplified();

    if ( character != "" && ! parent->isIgnored(character) )
    {
        parent->addIgnore(character);
        addIgnoreDialog->hide();
        QListWidgetItem* lwi = new QListWidgetItem(character);
        addToIgnoreList(lwi);
    }
}
void FGui::fr_friendsContextMenuRequested ( const QPoint& point )
{
    QListWidgetItem* lwi = fr_lwFriends->itemAt ( point );

    if ( lwi )
    {
        FCharacter* ch = parent->getCharacter(lwi->text());
        ul_recent = ch;
        displayCharacterContextMenu ( ch );
    }
}

void FGui::tb_channelRightClicked ( const QPoint& point )
{
    QObject* sender = this->sender();
    QPushButton* button = qobject_cast<QPushButton*> ( sender );
    if (button) {
        tb_recent = parent->getChannel(button->objectName());
        std::cout << tb_recent->title().toStdString() << std::endl;
        channelButtonMenuRequested();
    }
}


void FGui::ul_pmRequested()
{
    QString character = ul_recent->name();
    parent->openPMTab(character);
}
void FGui::ul_infoRequested()
{

}
void FGui::ul_ignoreAdd()
{
    FCharacter* c = ul_recent;
    QString name = c->name();
    parent->addIgnore(name);
}
void FGui::ul_ignoreRemove()
{
    FCharacter* c = ul_recent;
    QString name = c->name();
    parent->deleteIgnore(name);
}
void FGui::ul_channelBan()
{
    QString name = ul_recent->name();
    parent->_channelBan(name);
}
void FGui::ul_channelKick()
{
    QString name = ul_recent->name();
    parent->_channelKick(name);
}
void FGui::ul_chatBan()
{
    QString name = ul_recent->name();
    parent->_chatBan(name);
}
void FGui::ul_chatKick()
{
    QString name = ul_recent->name();
    parent->_chatKick(name);
}
void FGui::ul_chatTimeout()
{
    QString name = ul_recent->name();
    timeoutDialogRequested(name);
}
void FGui::ul_channelOpAdd()
{
    QString name = ul_recent->name();
    parent->_addChanOp(name);
}
void FGui::ul_channelOpRemove()
{
    QString name = ul_recent->name();
    parent->_deleteChanOp(name);
}
void FGui::ul_chatOpAdd()
{
    QString name = ul_recent->name();
    parent->_addOp(name);
}

void FGui::ul_chatOpRemove()
{
    QString name = ul_recent->name();
    parent->_deleteOp(name);
}
void FGui::ul_profileRequested()
{
    FCharacter* ch = ul_recent;
    QString l = "https://www.f-list.net/c/";
    l += ch->name();
    QUrl link(l);
    QDesktopServices::openUrl(link);
}

void FGui::fr_btnCloseClicked()
{
    friendsDialog->close();
}
void FGui::fr_btnFriendsPMClicked()
{
    QListWidgetItem* lwi = fr_lwFriends->selectedItems().at ( 0 );

    if (lwi)
    {
        QString name = lwi->text();
        parent->openPMTab ( name );
    }
}
void FGui::fr_btnIgnoreRemoveClicked()
{
    QListWidgetItem* lwi = fr_lwIgnore->selectedItems().at ( 0 );

    if ( lwi )
    {
        QString name = lwi->text();
        parent->deleteIgnore(name);
        fr_lwIgnore->removeItemWidget(lwi);
    }
}
void FGui::fr_btnIgnoreAddClicked()
{
    addIgnoreDialogRequested();
}
void FGui::cd_btnCancelClicked()
{
    channelsDialog->hide();
}
void FGui::cd_btnJoinClicked()
{
    QList<QListWidgetItem *> cliList = cd_channelsList->selectedItems();
    ChannelListItem* cli = 0;

    for ( int i = 0;i < cliList.count();i++ )
    {
        cli = ( ChannelListItem* ) ( cliList.at ( i ) );
        QString name = cli->getName();
        parent->joinChannel ( name );
    }
}
void FGui::cd_btnProomsJoinClicked()
{
    QList<QListWidgetItem *> cliList = cd_proomsList->selectedItems();
    ChannelListItem* cli = 0;

    for ( int i = 0;i < cliList.count();i++ )
    {
        cli = ( ChannelListItem* ) ( cliList.at ( i ) );
        QString name = cli->getName();
        parent->joinChannel ( name );
    }
}
void FGui::ci_btnCloseClicked()
{
    characterInfoDialog->close();
}

void FGui::to_btnSubmitClicked()
{
    QString who = to_leWho->text();
    QString length = to_leLength->text();
    QString why = to_leReason->text();
    int time = length.toInt();
    parent->_timeout(who, time, why);
    timeoutDialog->hide();
}
void FGui::to_btnCancelClicked()
{
    timeoutDialog->hide();
}
void FGui::mr_btnCancelClicked()
{
    makeRoomDialog->hide();
}
void FGui::mr_btnSubmitClicked()
{
    QString title = mr_leName->text().simplified();
    parent->_makeRoom(title);
    makeRoomDialog->hide();
}
void FGui::ss_btnCancelClicked()
{
    setStatusDialog->hide();
}
void FGui::ss_btnSubmitClicked()
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

    parent->changeStatus ( status, message );

    setStatusDialog->hide();
}
void FGui::cs_chbEditDescriptionToggled(bool state)
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
        cs_tbDescription->setHtml(parent->getParser()->parse(cs_qsPlainDescription));
        cs_tbDescription->show();
        cs_teDescription->hide();
    }
}
void FGui::cs_btnCancelClicked()
{
    cs_qsPlainDescription = "";
    cs_chanCurrent = 0;
    channelSettingsDialog->deleteLater();
}
void FGui::cs_btnSaveClicked()
{
    if (cs_qsPlainDescription != cs_chanCurrent->description())
    {
        std::cout << "Editing description." << std::endl;
        // Update description
        QString channel = cs_chanCurrent->name();
        QString description = cs_qsPlainDescription;
        parent->_setDescription(channel, description);
    }
    cs_chanCurrent->setAlwaysPing(cs_chbAlwaysPing->isChecked());

    QString setting = cs_chanCurrent->name();
    setting += "/alwaysping";
    QString* path = parent->getSettings()->getPath();
    QSettings settings(*path, QSettings::IniFormat);
    settings.setValue(setting, BOOLSTR(cs_chbAlwaysPing->isChecked()));

    cs_qsPlainDescription = "";
    cs_chanCurrent = 0;
    channelSettingsDialog->deleteLater();
}
void FGui::re_btnSubmitClicked()
{
    QString who = re_leWho->text();
    QString why = re_teProblem->toPlainText();
    parent->submitReport(why, who);
    reportDialog->hide();
    re_leWho->clear();
    re_teProblem->clear();
}
void FGui::re_btnCancelClicked()
{
    reportDialog->hide();
}
void FGui::se_btnSubmitClicked()
{
    FSettings* settings = parent->getSettings();
    settings->setHelpdesk(se_chbHelpdesk->isChecked());
    settings->setPing(se_chbEnablePing->isChecked());
    settings->setLeaveJoin(se_chbLeaveJoin->isChecked());
    settings->setAlwaysPing(se_chbAlwaysPing->isChecked());
    settings->setSounds(!se_chbMute->isChecked());
    settings->setChatLogs(se_chbEnableChatLogs->isChecked());
    settings->setOnlineOffline(se_chbOnlineOffline->isChecked());

    QString liststr = se_lePingList->text();
    settings->setPingList(liststr);
    settings->saveSettings();
    settingsDialog->hide();
}
void FGui::se_btnCancelClicked()
{
    settingsDialog->hide();
}
void FGui::tb_closeClicked()
{
//    if (tb_recent == 0) return;
//    bool current = false;
//    if (currentPanel && tb_recent == currentPanel)
//    {
//        current = true;
//    }
//    if (tb_recent->type() == FChannel::CHANTYPE_PM)
//    {
//        tb_recent->setActive(false);
//        tb_recent->pushButton->setVisible(false);
//        tb_recent->setTyping ( FChannel::TYPINGSTATUS_CLEAR );
//    } else {
//        std::string channel = tb_recent->name().toStdString();
//        leaveChannel ( channel );
//    }
//    if (current)
//    {
//        QString c = "CONSOLE";
//        switchTab(c);
//    }
}
void FGui::tb_settingsClicked()
{
    channelSettingsDialogRequested();
}

