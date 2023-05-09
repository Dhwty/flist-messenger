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
#include <QSplitter>
#include <QClipboard>

#include "flist_account.h"
#include "flist_server.h"
#include "flist_session.h"
#include "flist_message.h"
#include "flist_settings.h"
#include "flist_attentionsettingswidget.h"

// Bool to string macro
#define BOOLSTR(b) ((b) ? "true" : "false")
// String to bool macro
#define STRBOOL(s) ((s == "true") ? true : false)

QString flist_messenger::settingsPath = "./settings.ini";

void flist_messenger::init() {
    settingsPath = QApplication::applicationDirPath() + "/settings.ini";
}

flist_messenger::flist_messenger(bool d) {
    server = new FServer(this);
    account = server->addAccount();
    account->ui = this;
    // account = new FAccount(0, 0);
    doingWS = true;
    notificationsAreaMessageShown = false;
    console = 0;
    chatview = 0;
    // tcpSock = 0;
    debugging = d;
    disconnected = true;
    friendsDialog = 0;
    makeRoomDialog = 0;
    setStatusDialog = 0;
    ci_dialog = 0;
    recentCharMenu = 0;
    recentChannelMenu = 0;
    reportDialog = 0;
    helpDialog = 0;
    aboutDialog = 0;
    timeoutDialog = 0;
    settingsDialog = 0;
    trayIcon = 0;
    trayIconMenu = 0;
    channelSettingsDialog = 0;
    createTrayIcon();
    loadSettings();
    loginController = new FLoginController(fapi, account, this);
    setupLoginBox();
    cl_data = new FChannelListModel();
    cl_dialog = 0;
    selfStatus = "online";

    FCharacter::initClass();
    FChannelPanel::initClass();
}

void flist_messenger::closeEvent(QCloseEvent *event) {
    if (disconnected) quitApp();
    if (trayIcon->isVisible()) {
        if (!notificationsAreaMessageShown) {
            QString title("Still running~");
            QString msg(
                    "F-chat Messenger is still running! "
                    "To close it completely, right click the tray icon and click \"Quit\".");
            trayIcon->showMessage(title, msg, QSystemTrayIcon::Information, 10000);
            notificationsAreaMessageShown = true;
        }
        hide();
        event->ignore();
    }
}

void flist_messenger::iconActivated(QSystemTrayIcon::ActivationReason reason) {
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

void flist_messenger::enterPressed() {
    if (currentPanel == 0) return;

    QString input = plainTextEdit->toPlainText();
    if (currentPanel->getMode() == CHANNEL_MODE_ADS && !input.startsWith("/")) {
        btnSendAdvClicked();
    } else {
        parseInput();
    }
}

void flist_messenger::createTrayIcon() {
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

flist_messenger::~flist_messenger() {
    // TODO: Delete everything
    delete cl_dialog;
    delete cl_data;
}

void flist_messenger::printDebugInfo(std::string s) {
    if (debugging) {
        std::cout << s << std::endl;
        if (console) {
            QString q = s.c_str();
            QString gt = "&gt;";
            QString lt = "&lt;";
            QString amp = "&amp;";
            q.replace('&', amp).replace('<', lt).replace('>', gt);
            console->addLine(q, true);
        }
    }
}

void flist_messenger::setupLoginBox() {
    this->setWindowTitle("F-chat messenger - Login");
    this->setWindowIcon(QIcon(":/images/icon.png"));

    loginWidget = new FLoginWindow(this);
    loginController->setWidget(loginWidget);
    this->setCentralWidget(loginWidget);

    this->resize(265, 100);
    centerOnScreen(this);
}

void flist_messenger::startConnect(QString charName) {
    this->charName = charName;
    FSession *session = account->addSession(charName);
    session->autojoinchannels = defaultChannels;
    this->centralWidget()->deleteLater();

    setupRealUI();
    connect(session, SIGNAL(socketErrorSignal(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)));

    connect(session, SIGNAL(notifyCharacterOnline(FSession *, QString, bool)), this, SLOT(notifyCharacterOnline(FSession *, QString, bool)));
    connect(session, SIGNAL(notifyCharacterStatusUpdate(FSession *, QString)), this, SLOT(notifyCharacterStatusUpdate(FSession *, QString)));
    connect(session, SIGNAL(notifyIgnoreAdd(FSession *, QString)), this, SLOT(notifyIgnoreAdd(FSession *, QString)));
    connect(session, SIGNAL(notifyIgnoreRemove(FSession *, QString)), this, SLOT(notifyIgnoreRemove(FSession *, QString)));

    session->connectSession();
}

void flist_messenger::destroyMenu() {
    if (recentCharMenu) recentCharMenu->deleteLater();
}

void flist_messenger::destroyChanMenu() {
    if (recentChannelMenu) recentChannelMenu->deleteLater();
}

void flist_messenger::setupReportDialog() {
    reportDialog = new QDialog(this);
    re_vblOverview = new QVBoxLayout;
    re_hblButtons = new QHBoxLayout;
    re_lblWho = new QLabel("Reporting user:");
    re_lblProblem = new QLabel("Describe your problem.");
    re_lblInstructions = new QLabel(
            "Note that channel mods do <b>not</b> receive staff alerts. If you are reporting a problem in a private channel, please contact the channel's moderator instead. "
            "Furthermore, if you are reporting something that is not against the rules, we will not be able to help you. If you are reporting private messages, try using /ignore "
            "first.");
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

    connect(re_btnSubmit, SIGNAL(clicked()), this, SLOT(re_btnSubmitClicked()));
    connect(re_btnCancel, SIGNAL(clicked()), this, SLOT(re_btnCancelClicked()));
}

void flist_messenger::helpDialogRequested() {
    if (helpDialog == 0 || helpDialog->parent() != this) helpDialog = new FHelpDialog(this);
    helpDialog->show();
}

void flist_messenger::channelSettingsDialogRequested() {
    // This is one that always needs to be setup, because its contents may vary.
    if (setupChannelSettingsDialog()) channelSettingsDialog->show();
}

bool flist_messenger::setupChannelSettingsDialog() {
    FChannelPanel *channelpanel = channelList.value(tb_recent_name);
    if (!channelpanel) {
        debugMessage(QString("Tried to setup a channel settings dialog for the panel '%1', but the panel does not exist.").arg(tb_recent_name));
        return false;
    }
    FSession *session = account->getSession(channelpanel->getSessionID());

    bool is_pm = channelpanel->type() == FChannel::CHANTYPE_PM;
    // todo: Allow to work with offline characters.
    // if(is_pm && !session->isCharacterOnline(channelpanel->recipient())) { // Recipient is offline
    //	return false;
    // }

    cs_chanCurrent = channelpanel;

    channelSettingsDialog = new QDialog(this);
    channelSettingsDialog->setWindowIcon(channelpanel->pushButton->icon());
    FCharacter *character = 0;
    if (is_pm) {
        character = session->getCharacter(channelpanel->recipient());
        if (character) {
            channelSettingsDialog->setWindowTitle(character->name());
            cs_qsPlainDescription = character->statusMsg();
        } else {
            channelSettingsDialog->setWindowTitle(channelpanel->recipient());
            cs_qsPlainDescription = "";
        }
    } else {
        channelSettingsDialog->setWindowTitle(channelpanel->title());
        cs_qsPlainDescription = channelpanel->description();
    }

    // QTabWidget* twOverview = new QTabWidget;

    QTabWidget *twOverview = new QTabWidget;
    QGroupBox *gbNotifications = new QGroupBox(QString("Notifications"));
    QVBoxLayout *vblNotifications = new QVBoxLayout;

    if (is_pm) {
        cs_attentionsettings = new FAttentionSettingsWidget(channelpanel->recipient(), channelpanel->recipient(), channelpanel->type());
    } else {
        cs_attentionsettings = new FAttentionSettingsWidget(channelpanel->getChannelName(), channelpanel->title(), channelpanel->type());
    }

    cs_vblOverview = new QVBoxLayout;
    cs_gbDescription = new QGroupBox(is_pm ? "Status" : "Description");
    cs_vblDescription = new QVBoxLayout;
    cs_tbDescription = new QTextBrowser;
    cs_tbDescription->setHtml(bbparser.parse(cs_qsPlainDescription));
    cs_tbDescription->setReadOnly(true);
    cs_teDescription = new QTextEdit;
    cs_teDescription->setPlainText(cs_qsPlainDescription);
    cs_teDescription->hide();
    if (!is_pm) {
        cs_chbEditDescription = new QCheckBox("Editable mode (OPs only)");
        if (!(channelpanel->isOp(session->getCharacter(session->character)) || session->isCharacterOperator(session->character))) {
            cs_chbEditDescription->setEnabled(false);
        }
    }
    cs_hblButtons = new QHBoxLayout;
    cs_btnCancel = new QPushButton(QIcon(QString(":/images/cross.png")), "Cancel");
    cs_btnSave = new QPushButton(QIcon(QString(":/images/tick.png")), "Save");

    channelSettingsDialog->setLayout(cs_vblOverview);
    cs_vblOverview->addWidget(twOverview);
    twOverview->addTab(cs_gbDescription, "General");
    cs_gbDescription->setLayout(cs_vblDescription);
    if (is_pm) {
        QLabel *lblStatus = new QLabel(character ? character->statusString() : "Offline");
        cs_vblDescription->addWidget(lblStatus);
    }
    cs_vblDescription->addWidget(cs_teDescription);
    cs_vblDescription->addWidget(cs_tbDescription);
    if (!is_pm) {
        cs_vblDescription->addWidget(cs_chbEditDescription);
    }

    twOverview->addTab(gbNotifications, QString("Notifications"));
    gbNotifications->setLayout(vblNotifications);
    vblNotifications->addWidget(cs_attentionsettings);

    cs_vblOverview->addLayout(cs_hblButtons);
    cs_hblButtons->addStretch();
    cs_hblButtons->addWidget(cs_btnSave);
    cs_hblButtons->addWidget(cs_btnCancel);

    if (!is_pm) {
        connect(cs_chbEditDescription, SIGNAL(toggled(bool)), this, SLOT(cs_chbEditDescriptionToggled(bool)));
    }

    cs_attentionsettings->loadSettings();

    connect(cs_btnCancel, SIGNAL(clicked()), this, SLOT(cs_btnCancelClicked()));
    connect(cs_btnSave, SIGNAL(clicked()), this, SLOT(cs_btnSaveClicked()));
    connect(cs_tbDescription, SIGNAL(anchorClicked(QUrl)), this, SLOT(anchorClicked(QUrl)));

    return true;
}

void flist_messenger::cs_chbEditDescriptionToggled(bool state) {
    if (state) {
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

void flist_messenger::cs_btnCancelClicked() {
    cs_qsPlainDescription = "";
    cs_chanCurrent = 0;
    channelSettingsDialog->deleteLater();
}

void flist_messenger::cs_btnSaveClicked() {
    if (cs_qsPlainDescription != cs_chanCurrent->description()) {
        std::cout << "Editing description." << std::endl;
        // Update description
        account->getSessionByCharacter(charName)->setChannelDescription(cs_chanCurrent->getChannelName(), cs_qsPlainDescription);
    }
    // Save settings to the ini file.
    cs_attentionsettings->saveSettings();
    // And reload those settings into the channel panel.
    cs_chanCurrent->loadSettings();

    cs_qsPlainDescription = "";
    cs_chanCurrent = 0;
    channelSettingsDialog->deleteLater();
}

void flist_messenger::re_btnSubmitClicked() {
    submitReport();
    reportDialog->hide();
}

void flist_messenger::submitReport() {
    // todo: fix this
    lurl = QString(FLIST_BASEURL_TICKET) + QString("?secure=no&account=" + account->getUserName() + "&password=" + account->getPassword());
    lreply = qnam.get(QNetworkRequest(lurl));
    // todo: fix this!
    lreply->ignoreSslErrors();

    connect(lreply, SIGNAL(finished()), this, SLOT(reportTicketFinished()));
}

void flist_messenger::handleReportFinished() {
    if (lreply->error() != QNetworkReply::NoError) {
        QString message = "Response error during sending of report ";
        message.append("of type ");
        message.append(QString::number(lreply->error()));
        QMessageBox::critical(this, "FATAL ERROR DURING TICKET RETRIEVAL!", message);
        return;
    } else {
        QByteArray respbytes = lreply->readAll();
        lreply->deleteLater();

        QJsonDocument responseDoc = QJsonDocument::fromJson(respbytes);
        QVariantMap responseMap = responseDoc.toVariant().toMap();

        QString childnode = responseMap.value("error").toString();

        if (!childnode.isEmpty()) {
            QString message = "Error from server: " + childnode;
            QMessageBox::critical(this, "Error", message);
            return;
        } else {
            childnode = responseMap.value("log_id").toString();

            QString logid = childnode;
            QString gt = "&gt;";
            QString lt = "&lt;";
            QString amp = "&amp;";
            QString problem = re_teProblem->toPlainText().replace('&', amp).replace('<', lt).replace('>', gt);
            QString who = re_leWho->text().replace('&', amp).replace('<', lt).replace('>', gt);
            if (who.trimmed() == "") who = "None";
            QString report = "Current Tab/Channel: " + currentPanel->title() + " | Reporting User: " + who + " | " + problem;

            QVariantMap reportMap;

            reportMap.insert("action", "report");
            reportMap.insert("logid", logid);
            reportMap.insert("character", charName);
            reportMap.insert("report", report);

            QJsonDocument reportDoc;
            reportDoc.fromVariant(reportMap);

            qDebug() << logid;

            std::string output = "SFC " + reportDoc.toJson(QJsonDocument::Compact).toStdString();
            sendWS(output);
            reportDialog->hide();
            re_leWho->clear();
            re_teProblem->clear();
            QString message = "Your report was uploaded";
            QMessageBox::information(this, "Success.", message);
        }
    }
}

void flist_messenger::reportTicketFinished() {
    if (lreply->error() != QNetworkReply::NoError) {
        QString message = "Response error during fetching of ticket ";
        message.append(" of type ");
        message.append(QString::number(lreply->error()));
        QMessageBox::critical(this, "FATAL ERROR DURING TICKET RETRIEVAL!", message);
        return;
    }
    QByteArray respbytes = lreply->readAll();
    lreply->deleteLater();

    QJsonDocument responseDoc = QJsonDocument::fromJson(respbytes);
    QVariantMap responseMap = responseDoc.toVariant().toMap();

    QString childnode = responseMap.value("error").toString();

    if (!childnode.isEmpty()) {
        QString message = "Error from server: " + childnode;
        QMessageBox::critical(this, "Error", message);
        return;
    }

    QString subnode = responseMap.value("ticket").toString();
    account->ticket = subnode;

    QString url_string = QString(FLIST_BASEURL_REPORT) + "?account=";
    url_string += account->getUserName();
    url_string += "&character=";
    url_string += charName;
    url_string += "&ticket=";
    url_string += account->ticket;
    lurl = url_string;
    qDebug() << url_string.toStdString();

    QByteArray postData;
    QJsonDocument *lognodes;
    lognodes = currentPanel->toJSON();
    std::string toWrite;
    toWrite = lognodes->toJson(QJsonDocument::Compact).toStdString();
    QNetworkRequest request(lurl);
    fix_broken_escaped_apos(toWrite);
    lparam = QUrlQuery();
    lparam.addQueryItem("log", toWrite.c_str());
#if QT_VERSION >= 0x050000
    postData = lparam.query(QUrl::FullyEncoded).toUtf8();
#else
    postData = lparam.encodedQuery();
#endif
    lreply = qnam.post(request, postData);
    // todo: fix this!
    lreply->ignoreSslErrors();

    connect(lreply, SIGNAL(finished()), this, SLOT(handleReportFinished()));
    delete lognodes;
}

void flist_messenger::re_btnCancelClicked() {
    reportDialog->hide();
}

void flist_messenger::se_btnSubmitClicked() {
    // se_helpdesk = se_chbHelpdesk->isChecked();
    settings->setShowJoinLeaveMessage(se_chbLeaveJoin->isChecked());
    settings->setPlaySounds(!se_chbMute->isChecked());
    settings->setLogChat(se_chbEnableChatLogs->isChecked());
    settings->setShowOnlineOfflineMessage(se_chbOnlineOffline->isChecked());

    se_attentionsettings->saveSettings();
    saveSettings();
    loadSettings();
    settingsDialog->hide();
}

void flist_messenger::se_btnCancelClicked() {
    settingsDialog->hide();
}

void flist_messenger::closeChannelPanel(QString panelname) {
    FChannelPanel *channelpanel = channelList.value(panelname);
    if (!channelpanel) {
        debugMessage(QString("[BUG] Told to close channel panel '%1', but the panel does not exist.'").arg(panelname));
        return;
    }
    if (channelpanel == console) {
        debugMessage(QString("[BUG] Was told to close the chanel panel for the console! Channel panel '%1'.").arg(panelname));
        return;
    }
    if (channelpanel->type() == FChannel::CHANTYPE_PM) {
        channelpanel->setTyping(TYPING_STATUS_CLEAR);
    } else {
        channelpanel->emptyCharList();
    }
    channelpanel->setActive(false);
    channelpanel->pushButton->setVisible(false);
    if (channelpanel == currentPanel) {
        QString c = "CONSOLE";
        switchTab(c);
    }
}

/**
Close a channel panel and if it has an associated channel, leave the channel too.
 */
void flist_messenger::leaveChannelPanel(QString panelname) {
    FChannelPanel *channelpanel = channelList.value(panelname);
    if (!channelpanel) {
        debugMessage(QString("[BUG] Told to leave channel panel '%1', but the panel does not exist.'").arg(panelname));
        return;
    }
    if (channelpanel == console) {
        debugMessage(QString("[BUG] Was told to leave the chanel panel for the console! Channel panel '%1'.").arg(panelname));
        return;
    }
    if (channelpanel->getSessionID().isEmpty()) {
        debugMessage(QString("[BUG] Was told to leave the chanel panel '%1', but it has no session associated with it!.").arg(panelname));
        return;
    }
    FSession *session = account->getSession(channelpanel->getSessionID());
    QString channelname;
    if (channelpanel->type() != FChannel::CHANTYPE_PM) {
        channelname = channelpanel->getChannelName();
    }
    closeChannelPanel(panelname);
    if (!channelname.isEmpty()) {
        session->sendChannelLeave(channelname);
    }
}

void flist_messenger::tb_closeClicked() {
    leaveChannelPanel(tb_recent_name);
}

void flist_messenger::tb_settingsClicked() {
    channelSettingsDialogRequested();
}

void flist_messenger::setupRealUI() {
    // Setting up console first because it needs to receive server output.
    // todo: Make account->getSession(currentPanel->getSessionID()) code robust to having a sessionless panel.
    // todo: Remove 'charName' so that this panel can be sessionless
    console = new FChannelPanel(this, charName, "FCHATSYSTEMCONSOLE", "FCHATSYSTEMCONSOLE", FChannel::CHANTYPE_CONSOLE);
    QString name = "Console";
    console->setTitle(name);
    channelList["FCHATSYSTEMCONSOLE"] = console;

    if (objectName().isEmpty()) setObjectName("MainWindow");

    resize(836, 454);
    setWindowTitle(FLIST_VERSION);
    setWindowIcon(QIcon(":/images/icon.ico"));
    actionDisconnect = new QAction(this);
    actionDisconnect->setObjectName("actionDisconnect");
    actionDisconnect->setText("Disconnect (WIP)");
    actionDisconnect->setIcon(QIcon(":/images/plug-disconnect.png"));
    actionQuit = new QAction(this);
    actionQuit->setObjectName("actionQuit");
    actionQuit->setText("&Quit");
    actionQuit->setIcon(QIcon(":/images/cross-button.png"));
    actionHelp = new QAction(this);
    actionHelp->setObjectName("actionHelp");
    actionHelp->setText("Help");
    actionHelp->setIcon(QIcon(":/images/question.png"));
    actionAbout = new QAction(this);
    actionAbout->setObjectName(QString::fromUtf8("actionAbout"));
    actionAbout->setText(QString::fromUtf8("About"));
    actionAbout->setIcon(QIcon(":/images/icon.ico"));
    verticalLayoutWidget = new QWidget(this);
    verticalLayoutWidget->setObjectName(QString::fromUtf8("overview"));
    verticalLayoutWidget->setGeometry(QRect(5, -1, 841, 511));
    verticalLayout = new QVBoxLayout(verticalLayoutWidget);
    verticalLayout->setObjectName("overviewLayout");
    verticalLayout->setContentsMargins(0, 0, 0, 0);
    horizontalLayoutWidget = new QWidget(this);
    horizontalLayoutWidget->setObjectName("main");
    horizontalLayoutWidget->setGeometry(QRect(0, -1, 831, 401));
    horizontalLayout = new QHBoxLayout(horizontalLayoutWidget);
    horizontalLayout->setObjectName("mainLayout");
    horizontalLayout->setContentsMargins(0, 0, 0, 0);
    activePanels = new QScrollArea(horizontalLayoutWidget);
    activePanels->setObjectName(QString::fromUtf8("activePanels"));
    QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(activePanels->sizePolicy().hasHeightForWidth());
    activePanels->setSizePolicy(sizePolicy);
    activePanels->setMinimumSize(QSize(67, 0));
    activePanels->setMaximumSize(QSize(16777215, 16777215));
    activePanels->setBaseSize(QSize(0, 0));
    activePanels->setFrameShape(QFrame::StyledPanel);
    activePanels->setFrameShadow(QFrame::Sunken);
    activePanels->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    activePanels->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    activePanels->setWidgetResizable(true);
    activePanelsContents = new QVBoxLayout();
    activePanelsContents->setObjectName("activePanelsLayout");
    activePanelsContents->setGeometry(QRect(0, 0, 61, 393));
    activePanels->setFixedWidth(45);
    activePanels->setLayout(activePanelsContents);
    horizontalLayout->addWidget(activePanels);
    horizontalsplitter = new QSplitter(Qt::Horizontal);
    centralstuffwidget = new QWidget(horizontalsplitter);
    centralstuffwidget->setContentsMargins(0, 0, 0, 0);
    centralStuff = new QVBoxLayout(centralstuffwidget);
    centralButtonsWidget = new QWidget;
    centralButtons = new QHBoxLayout(centralButtonsWidget);
    centralButtonsWidget->setLayout(centralButtons);
    lblChannelName = new QLabel(QString(""));
    lblChannelName->setObjectName("currentpanelname");
    lblChannelName->setWordWrap(true);
    QSizePolicy lblchannelnamesizepolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    lblChannelName->setSizePolicy(lblchannelnamesizepolicy);
    btnSettings = new QPushButton;
    btnSettings->setIcon(QIcon(":/images/terminal.png"));
    btnSettings->setToolTip(QString("Settings"));
    btnSettings->setObjectName("settingsbutton");
    btnChannels = new QPushButton;
    btnChannels->setIcon(QIcon(":/images/hash.png"));
    btnChannels->setToolTip(QString("Channels"));
    btnChannels->setObjectName("channelsbutton");
    btnMakeRoom = new QPushButton;
    btnMakeRoom->setIcon(QIcon(":/images/key.png"));
    btnMakeRoom->setToolTip(QString("Make a room!"));
    btnMakeRoom->setObjectName("makeroombutton");
    btnSetStatus = new QPushButton;
    btnSetStatus->setIcon(QIcon(":/images/status-default.png"));
    btnSetStatus->setToolTip(QString("Set your status"));
    btnSetStatus->setObjectName("setstatusbutton");
    btnFriends = new QPushButton;
    btnFriends->setIcon(QIcon(":/images/users.png"));
    btnFriends->setToolTip(QString("Friends and bookmarks"));
    btnFriends->setObjectName("friendsbutton");
    btnReport = new QPushButton;
    btnReport->setIcon(QIcon(":/images/auction-hammer--exclamation.png"));
    btnReport->setToolTip(QString("Alert Staff!"));
    btnReport->setObjectName("alertstaffbutton");
    centralButtons->addWidget(lblChannelName);
    centralButtons->addWidget(btnSettings);
    centralButtons->addWidget(btnReport);
    centralButtons->addWidget(btnFriends);
    centralButtons->addWidget(btnSetStatus);
    centralButtons->addWidget(btnMakeRoom);
    centralButtons->addWidget(btnChannels);
    connect(btnSettings, SIGNAL(clicked()), this, SLOT(settingsDialogRequested()));
    connect(btnReport, SIGNAL(clicked()), this, SLOT(reportDialogRequested()));
    connect(btnFriends, SIGNAL(clicked()), this, SLOT(friendsDialogRequested()));
    connect(btnChannels, SIGNAL(clicked()), this, SLOT(channelsDialogRequested()));
    connect(btnMakeRoom, SIGNAL(clicked()), this, SLOT(makeRoomDialogRequested()));
    connect(btnSetStatus, SIGNAL(clicked()), this, SLOT(setStatusDialogRequested()));
    chatview = new FLogTextBrowser(this);
    chatview->setOpenLinks(false);
    chatview->setObjectName("chatoutput");
    chatview->setContextMenuPolicy(Qt::DefaultContextMenu);
    // chatview->setDocumentTitle ( "" );
    // chatview->setReadOnly ( true );
    chatview->setFrameShape(QFrame::NoFrame);
    connect(chatview, SIGNAL(anchorClicked(QUrl)), this, SLOT(anchorClicked(QUrl)));
    centralStuff->addWidget(centralButtonsWidget);
    centralStuff->addWidget(chatview);
    horizontalsplitter->addWidget(centralstuffwidget);
    QSizePolicy centralstuffwidgetsizepolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    centralstuffwidgetsizepolicy.setHorizontalStretch(4);
    centralstuffwidgetsizepolicy.setVerticalStretch(0);
    centralstuffwidgetsizepolicy.setHeightForWidth(false);
    centralstuffwidget->setSizePolicy(centralstuffwidgetsizepolicy);

    listWidget = new QListWidget(horizontalsplitter);
    listWidget->setObjectName("userlist");
    QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Expanding);
    sizePolicy1.setHorizontalStretch(1);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(listWidget->sizePolicy().hasHeightForWidth());
    listWidget->setSizePolicy(sizePolicy1);
    listWidget->setMinimumSize(QSize(30, 0));
    listWidget->setMaximumSize(QSize(16777215, 16777215));
    listWidget->setBaseSize(QSize(100, 0));
    listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    listWidget->setIconSize(QSize(16, 16));
    connect(listWidget, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(userListContextMenuRequested(const QPoint &)));
    horizontalsplitter->addWidget(listWidget);
    horizontalLayout->addWidget(horizontalsplitter);
    verticalLayout->addWidget(horizontalLayoutWidget);
    QWidget *textFieldWidget = new QWidget;
    textFieldWidget->setObjectName("inputarea");
    QHBoxLayout *inputLayout = new QHBoxLayout(textFieldWidget);
    inputLayout->setObjectName("inputareaLayout");
    plainTextEdit = new QPlainTextEdit;
    plainTextEdit->setObjectName("chatinput");
    QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Maximum);
    sizePolicy2.setHorizontalStretch(0);
    sizePolicy2.setVerticalStretch(0);
    sizePolicy2.setHeightForWidth(plainTextEdit->sizePolicy().hasHeightForWidth());
    plainTextEdit->setSizePolicy(sizePolicy2);
    plainTextEdit->setMaximumSize(QSize(16777215, 75));
    returnFilter = new UseReturn(this);
    plainTextEdit->installEventFilter(returnFilter);
    plainTextEdit->setFrameShape(QFrame::NoFrame);
    connect(plainTextEdit, SIGNAL(textChanged()), this, SLOT(inputChanged()));
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
    verticalLayout->addWidget(textFieldWidget);
    setCentralWidget(verticalLayoutWidget);
    menubar = new QMenuBar(this);
    menubar->setObjectName("menubar");
    menubar->setGeometry(QRect(0, 0, 836, 23));
    menuHelp = new QMenu(menubar);
    menuHelp->setObjectName("menuHelp");
    menuHelp->setTitle("Help");
    menuFile = new QMenu(menubar);
    menuFile->setObjectName("menuFile");
    menuFile->setTitle("&File");
    setMenuBar(menubar);
    menubar->addAction(menuFile->menuAction());
    menubar->addAction(menuHelp->menuAction());
    menuHelp->addAction(actionHelp);
    menuHelp->addSeparator();
    menuHelp->addAction(actionAbout);
    menuFile->addAction(actionDisconnect);
    menuFile->addSeparator();
    menuFile->addAction(actionQuit);
    connect(actionHelp, SIGNAL(triggered()), this, SLOT(helpDialogRequested()));
    connect(actionAbout, SIGNAL(triggered()), this, SLOT(aboutApp()));
    connect(actionQuit, SIGNAL(triggered()), this, SLOT(quitApp()));
    centerOnScreen(this);
    setupConsole();
}

void flist_messenger::setupConsole() {
    activePanelsSpacer = new QSpacerItem(0, 20, QSizePolicy::Ignored, QSizePolicy::Expanding);
    currentPanel = console;
    pushButton = new QPushButton();
    pushButton->setObjectName(QString("CONSOLE"));
    pushButton->setGeometry(QRect(0, 0, 30, 30));
    pushButton->setFixedSize(30, 30);
    pushButton->setIcon(QIcon(":/images/terminal.png"));
    pushButton->setToolTip(QString("View Console"));
    pushButton->setCheckable(true);
    pushButton->setStyleSheet("background-color: rgb(255, 255, 255);");
    pushButton->setChecked(true);
    console->pushButton = pushButton;
    connect(pushButton, SIGNAL(clicked()), this, SLOT(channelButtonClicked()));
    activePanelsContents->addWidget(pushButton, 0, Qt::AlignTop);
    activePanelsContents->addSpacerItem(activePanelsSpacer);
}

void flist_messenger::userListContextMenuRequested(const QPoint &point) {
    QListWidgetItem *lwi = listWidget->itemAt(point);

    if (lwi) {
        FSession *session = account->getSession(currentPanel->getSessionID());
        ul_recent_name = lwi->text();
        FCharacter *ch = session->getCharacter(ul_recent_name);
        displayCharacterContextMenu(ch);
    }
}

void flist_messenger::friendListContextMenuRequested(QString character) {
    FSession *session = account->getSessionByCharacter(charName); // todo: fix this
    if (session->isCharacterOnline(character)) {
        ul_recent_name = character;
        FCharacter *ch = session->getCharacter(ul_recent_name);
        displayCharacterContextMenu(ch);
    }
}

void flist_messenger::displayChannelContextMenu(FChannelPanel *ch) {
    if (!ch) {
        printDebugInfo("ERROR: Tried to display a context menu for a null pointer channel.");
    } else {
        QMenu *menu = new QMenu(this);
        recentChannelMenu = menu;
        menu->addAction(QIcon(":/images/cross.png"), QString("Close tab"), this, SLOT(tb_closeClicked()));
        menu->addAction(QIcon(":/images/terminal.png"), QString("Settings"), this, SLOT(tb_settingsClicked()));
        connect(menu, SIGNAL(aboutToHide()), this, SLOT(destroyChanMenu()));
        menu->exec(QCursor::pos());
    }
}

void flist_messenger::displayCharacterContextMenu(FCharacter *ch) {
    if (!ch) {
        printDebugInfo("ERROR: Tried to display a context menu for a null pointer character.");
    } else {
        QMenu *menu = new QMenu(this);
        recentCharMenu = menu;
        menu->addAction(QIcon(":/images/users.png"), QString("Private Message"), this, SLOT(ul_pmRequested()));
        menu->addAction(QIcon(":/images/book-open-list.png"), QString("F-list Profile"), this, SLOT(ul_profileRequested()));
        menu->addAction(QIcon(":/images/tag-label.png"), QString("Quick Profile"), this, SLOT(ul_infoRequested()));
        menu->addAction(QString("Copy Profile Link"), this, SLOT(ul_copyLink()));
        FSession *session = account->getSession(currentPanel->getSessionID());
        if (session->isCharacterIgnored(ch->name()))
            menu->addAction(QIcon(":/images/heart.png"), QString("Unignore"), this, SLOT(ul_ignoreRemove()));
        else
            menu->addAction(QIcon(":/images/heart-break.png"), QString("Ignore"), this, SLOT(ul_ignoreAdd()));
        bool op = session->isCharacterOperator(session->character);
        if (op) {
            menu->addAction(QIcon(":/images/fire.png"), QString("Chat Kick"), this, SLOT(ul_chatKick()));
            menu->addAction(QIcon(":/images/auction-hammer--exclamation.png"), QString("Chat Ban"), this, SLOT(ul_chatBan()));
            menu->addAction(QIcon(":/images/alarm-clock.png"), QString("Timeout..."), this, SLOT(timeoutDialogRequested()));
        }
        if (op || currentPanel->isOwner(session->getCharacter(session->character))) {
            if (currentPanel->isOp(ch))
                menu->addAction(QIcon(":/images/auction-hammer--minus.png"), QString("Remove Channel OP"), this, SLOT(ul_channelOpRemove()));
            else
                menu->addAction(QIcon(":/images/auction-hammer--plus.png"), QString("Add Channel OP"), this, SLOT(ul_channelOpAdd()));
        }
        if ((op || currentPanel->isOp(session->getCharacter(session->character))) && !ch->isChatOp()) {
            menu->addAction(QIcon(":/images/lightning.png"), QString("Channel Kick"), this, SLOT(ul_channelKick()));
            menu->addAction(QIcon(":/images/auction-hammer.png"), QString("Channel Ban"), this, SLOT(ul_channelBan()));
        }
        connect(menu, SIGNAL(aboutToHide()), this, SLOT(destroyMenu()));
        menu->exec(QCursor::pos());
    }
}

void flist_messenger::cl_joinRequested(QStringList channels) {
    FSession *session = account->getSession(charName); // TODO: fix this
    foreach (QString channel, channels) {
        FChannelPanel *channelpanel = channelList.value(channel);
        if (!channelpanel || !channelpanel->getActive()) {
            session->joinChannel(channel);
        }
    }
}

void flist_messenger::anchorClicked(QUrl link) {
    QString linktext = link.toString();
    /* Anchor commands:
     * AHI: Ad hoc invite. Join channel.
     * CSA: Confirm staff report.
     */
    if (linktext.startsWith("#AHI-")) {
        QString channel = linktext.mid(5);
        FSession *session = account->getSession(currentPanel->getSessionID());
        session->joinChannel(channel);
    } else if (linktext.startsWith("#CSA-")) {
        // Confirm staff alert
        FSession *session = account->getSession(currentPanel->getSessionID());
        session->sendConfirmStaffReport(linktext.mid(5));
    } else if (linktext.startsWith("http://") || linktext.startsWith("https://")) {
        // todo: Make this configurable between opening, copying, or doing nothing.
        QDesktopServices::openUrl(link);
    } else {
        // todo: Show a debug or error message?
    }
    chatview->verticalScrollBar()->setSliderPosition(chatview->verticalScrollBar()->maximum());
}

void flist_messenger::insertLineBreak() {
    if (chatview) plainTextEdit->insertPlainText("\n");
}

void flist_messenger::refreshUserlist() {
    if (currentPanel == 0) {
        return;
    }
    // todo: Should 'listWidget' be renewed like this?
    listWidget = this->findChild<QListWidget *>(QString("userlist"));

    // Save current scroll position.
    int scrollpos = listWidget->verticalScrollBar()->value();

    // Remember currently selected characters, so that we can restore them.
    // Probably overkill, but this does support the selection of multiple rows.
    QList<QListWidgetItem *> selecteditems = listWidget->selectedItems();
    QStringList selectedcharacters;
    foreach (QListWidgetItem *selecteditem, selecteditems) {
        selectedcharacters.append(selecteditem->text());
    }
    int currentrow = -1;
    int oldrow = listWidget->currentRow();
    QString currentname;
    {
        QListWidgetItem *currentitem = listWidget->currentItem();
        if (currentitem) {
            currentname = currentitem->text();
        }
    }
    selecteditems.clear();

    // Clear the existing list and reload it.
    listWidget->clear();
    QList<FCharacter *> charList = currentPanel->charList();
    foreach (FCharacter *character, charList) {
        QListWidgetItem *charitem = new QListWidgetItem(character->name());
        QIcon *i = character->statusIcon();
        charitem->setIcon(*i);

        QFont f = charitem->font();

        if (character->isChatOp()) {
            f.setBold(true);
            f.setItalic(true);
        } else if (currentPanel->isOp(character)) {
            f.setBold(true);
        }
        charitem->setFont(f);
        charitem->setForeground(character->genderColor());
        listWidget->addItem(charitem);
        // Is this what the current row was previously?
        if (character->name() == currentname) {
            currentrow = listWidget->count() - 1;
        }
        // or a selected item?
        if (selectedcharacters.contains(character->name())) {
            if (currentrow < 0) {
                currentrow = listWidget->count() - 1;
            }
            selecteditems.append(charitem);
        }
    }

    // Restore selection.
    if (selecteditems.count() <= 0) {
        // Could not find any previous matches, just set the current row to the same as the old row.
        listWidget->setCurrentRow(oldrow);
    } else {
        // Set current row.
        listWidget->setCurrentRow(currentrow, QItemSelectionModel::NoUpdate);
        // Restore selected items.
        foreach (QListWidgetItem *selecteditem, selecteditems) {
            selecteditem->setSelected(true);
        }
    }

    // Restore scroll position.
    listWidget->verticalScrollBar()->setValue(scrollpos);

    // Hide/show widget based upon panel type.
    if (currentPanel->type() == FChannel::CHANTYPE_PM || currentPanel->type() == FChannel::CHANTYPE_CONSOLE) {
        listWidget->hide();
    } else {
        listWidget->show();
    }
}

void flist_messenger::settingsDialogRequested() {
    if (settingsDialog == 0 || settingsDialog->parent() != this) setupSettingsDialog();

    // se_chbHelpdesk->setChecked(se_helpdesk);
    se_chbLeaveJoin->setChecked(settings->getShowJoinLeaveMessage());
    se_chbMute->setChecked(!settings->getPlaySounds());
    se_chbEnableChatLogs->setChecked(settings->getLogChat());
    se_chbOnlineOffline->setChecked(settings->getShowOnlineOfflineMessage());
    se_attentionsettings->loadSettings();

    settingsDialog->show();
}

void flist_messenger::setupSettingsDialog() {
    if (settingsDialog) settingsDialog->deleteLater();

    settingsDialog = new QDialog(this);
    se_chbLeaveJoin = new QCheckBox(QString("Display leave/join notices"));
    se_chbOnlineOffline = new QCheckBox(QString("Display online/offline notices for friends"));
    se_chbEnableChatLogs = new QCheckBox(QString("Save chat logs"));
    se_chbMute = new QCheckBox(QString("Mute sounds"));
    // se_chbHelpdesk = new QCheckBox(QString("Display helpdesk notices (WIP)"));
    se_attentionsettings = new FAttentionSettingsWidget("");

    QTabWidget *twOverview = new QTabWidget;
    QGroupBox *gbGeneral = new QGroupBox(QString("General"));
    QGroupBox *gbNotifications = new QGroupBox(QString("Notifications"));
    QGroupBox *gbSounds = new QGroupBox(QString("Sounds"));
    QVBoxLayout *vblOverview = new QVBoxLayout;
    QVBoxLayout *vblGeneral = new QVBoxLayout;
    QVBoxLayout *vblNotifications = new QVBoxLayout;
    QVBoxLayout *vblSounds = new QVBoxLayout;
    QHBoxLayout *hblButtons = new QHBoxLayout;
    QPushButton *btnSubmit = new QPushButton(QIcon(":/images/tick.png"), QString("Save settings"));
    QPushButton *btnCancel = new QPushButton(QIcon(":/images/cross.png"), QString("Cancel"));

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
    // vblGeneral->addWidget(se_chbHelpdesk);
    vblGeneral->addStretch(0);
    twOverview->addTab(gbNotifications, QString("Notifications"));
    gbNotifications->setLayout(vblNotifications);
    vblNotifications->addWidget(se_attentionsettings);
    twOverview->addTab(gbSounds, QString("Sounds"));
    vblSounds->addWidget(se_chbMute);
    vblSounds->addStretch(0);
    gbSounds->setLayout(vblSounds);

    connect(btnSubmit, SIGNAL(clicked()), this, SLOT(se_btnSubmitClicked()));
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(se_btnCancelClicked()));
}

void flist_messenger::setupFriendsDialog() {
    if (friendsDialog == 0 || friendsDialog->parent() != this) {
        friendsDialog = new FriendsDialog(account->getSessionByCharacter(charName), this);
        connect(friendsDialog, SIGNAL(privateMessageRequested(QString)), this, SLOT(openPMTab(QString)));
        connect(friendsDialog, SIGNAL(friendContextMenuRequested(QString)), this, SLOT(friendListContextMenuRequested(QString)));
    }
}

void flist_messenger::friendsDialogRequested() {
    setupFriendsDialog();
    friendsDialog->showFriends();
}

void flist_messenger::ignoreDialogRequested() {
    setupFriendsDialog();
    friendsDialog->showIgnore();
}

void flist_messenger::makeRoomDialogRequested() {
    if (makeRoomDialog == 0 || makeRoomDialog->parent() != this) {
        makeRoomDialog = new FMakeRoomDialog(this);
        connect(makeRoomDialog, SIGNAL(requestedRoomCreation(QString)), this, SLOT(createPrivateChannel(QString)));
    }
    makeRoomDialog->show();
}

void flist_messenger::setStatusDialogRequested() {
    if (setStatusDialog == 0 || setStatusDialog->parent() != this) {
        setStatusDialog = new StatusDialog(this);
        connect(setStatusDialog, SIGNAL(statusSelected(QString, QString)), this, SLOT(changeStatus(QString, QString)));
    }

    setStatusDialog->setShownStatus(selfStatus, selfStatusMessage);
    setStatusDialog->show();
}

void flist_messenger::characterInfoDialogRequested() {
    //[19:48 PM]>>PRO {"character":"Hexxy"}
    //[19:41 PM]>>KIN {"character":"Cinnamon Flufftail"}
    FSession *session = account->getSession(currentPanel->getSessionID());
    FCharacter *ch = session->getCharacter(ul_recent_name);
    if (!ch) {
        debugMessage(QString("Tried to request character info on the character '%1' but they went offline already!").arg(ul_recent_name));
        return;
    }
    session->requestProfileKinks(ch->name());

    if (!ci_dialog) {
        ci_dialog = new FCharacterInfoDialog(this);
    }
    ci_dialog->setDisplayedCharacter(ch);
    ci_dialog->show();
}

void flist_messenger::reportDialogRequested() {
    if (reportDialog == 0 || reportDialog->parent() != this) setupReportDialog();

    re_leWho->setText("None");
    re_teProblem->clear();
    reportDialog->show();
}

void flist_messenger::channelsDialogRequested() {
    account->getSessionByCharacter(charName)->requestChannels();

    if (cl_dialog == 0) {
        cl_dialog = new FChannelListDialog(cl_data, this);
        connect(cl_dialog, SIGNAL(joinRequested(QStringList)), this, SLOT(cl_joinRequested(QStringList)));
    }
    cl_dialog->show();
}

void flist_messenger::refreshChatLines() {
    if (currentPanel == 0) return;

    chatview->clear();
    currentPanel->printChannel(chatview);
}

FChannelTab *flist_messenger::addToActivePanels(QString &panelname, QString &channelname, QString &tooltip) {
    printDebugInfo("Joining " + channelname.toStdString());
    channelTab = this->findChild<FChannelTab *>(panelname);

    if (channelTab != 0) {
        channelTab->setVisible(true);
    } else {
        activePanelsContents->removeItem(activePanelsSpacer);
        channelTab = new FChannelTab();
        channelTab->setObjectName(panelname);
        channelTab->setGeometry(QRect(0, 0, 30, 30));
        channelTab->setFixedSize(30, 30);
        channelTab->setStyleSheet("background-color: rgb(255, 255, 255);");
        channelTab->setAutoFillBackground(true);
        channelTab->setCheckable(true);
        channelTab->setChecked(false);
        channelTab->setToolTip(tooltip);
        channelTab->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(channelTab, SIGNAL(clicked()), this, SLOT(channelButtonClicked()));
        connect(channelTab, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(tb_channelRightClicked(QPoint)));
        activePanelsContents->addWidget(channelTab, 0, Qt::AlignTop);

        if (panelname.startsWith("PM|||")) {
            avatarFetcher.applyAvatarToButton(channelTab, channelname);
            // channelTab->setIconSize(channelTab->iconSize()*1.5);
        } else if (panelname.startsWith("ADH|||")) {
            // todo: custom buttons
            channelTab->setIcon(QIcon(":/images/key.png"));
        } else {
            // todo: custom buttons
            channelTab->setIcon(QIcon(":/images/hash.png"));
        }

        activePanelsContents->addSpacerItem(activePanelsSpacer);
    }

    return channelTab;
}

void flist_messenger::aboutApp() {
    if (!aboutDialog) {
        aboutDialog = new FAboutDialog(this);
        connect(aboutDialog, SIGNAL(anchorClicked(QUrl)), this, SLOT(anchorClicked(QUrl)));
    }
    aboutDialog->show();
}

void flist_messenger::quitApp() {
    if (trayIcon) {
        trayIcon->hide();
    }
    QApplication::quit();
}

void flist_messenger::socketError(QAbstractSocket::SocketError socketError) {
    (void)socketError;
    FSession *session = account->getSession(charName); // todo: fix this
    QString sockErrorStr = session->tcpsocket->errorString();
    if (currentPanel) {
        QString errorstring = "<b>Socket Error: </b>" + sockErrorStr;
        messageSystem(0, errorstring, MESSAGE_TYPE_ERROR);
    } else
        QMessageBox::critical(this, "Socket Error!", "Socket Error: " + sockErrorStr);

    disconnected = true;
    // session->tcpsocket->abort();
    // session->tcpsocket->deleteLater();
    // session->tcpsocket = 0;
}

void flist_messenger::socketSslError(QList<QSslError> sslerrors) {
    QMessageBox msgbox;
    QString errorstring;
    foreach (const QSslError &error, sslerrors) {
        if (!errorstring.isEmpty()) {
            errorstring += ", ";
        }
        errorstring += error.errorString();
    }
    msgbox.critical(this, "SSL ERROR DURING LOGIN!", errorstring);
    messageSystem(0, errorstring, MESSAGE_TYPE_ERROR);
}

// todo: Calls to sendWS() are to be replaced with appropriate calls to FSession methods.
void flist_messenger::sendWS(std::string &input) {
    FSession *session = account->getSession(charName);
    session->wsSend(input);
}

void flist_messenger::changeStatus(QString status, QString statusmsg) {
    selfStatus = status;
    selfStatusMessage = statusmsg;
    account->getSessionByCharacter(charName)->sendStatus(status, statusmsg);

    if (status == "online")
        btnSetStatus->setIcon(QIcon(":/images/status-default.png"));
    else if (status == "busy")
        btnSetStatus->setIcon(QIcon(":/images/status-away.png"));
    else if (status == "dnd")
        btnSetStatus->setIcon(QIcon(":/images/status-busy.png"));
    else if (status == "looking")
        btnSetStatus->setIcon(QIcon(":/images/status.png"));
    else if (status == "away")
        btnSetStatus->setIcon(QIcon(":/images/status-blue.png"));

    QString output = QString("Status changed successfully.");

    messageSystem(0, output, MESSAGE_TYPE_FEEDBACK);
}

void flist_messenger::tb_channelRightClicked(const QPoint &point) {
    (void)point;
    QObject *sender = this->sender();
    QPushButton *button = qobject_cast<QPushButton *>(sender);
    if (button) {
        tb_recent_name = button->objectName();
        channelButtonMenuRequested();
    }
}

void flist_messenger::channelButtonMenuRequested() {
    displayChannelContextMenu(channelList.value(tb_recent_name));
}

void flist_messenger::channelButtonClicked() {
    QObject *sender = this->sender();
    QPushButton *button = qobject_cast<QPushButton *>(sender);

    if (button) {
        QString tab = button->objectName();
        switchTab(tab);
    }
}

void flist_messenger::usersCommand() {
    QString msg;
    FSession *session = account->getSession(currentPanel->getSessionID());
    msg.asprintf("<b>%d users online.</b>", session->getCharacterCount());
    messageSystem(session, msg, MESSAGE_TYPE_FEEDBACK);
}

void flist_messenger::inputChanged() {
    if (currentPanel && currentPanel->type() == FChannel::CHANTYPE_PM) {
        if (plainTextEdit->toPlainText().simplified() == "") {
            typingCleared(currentPanel);
            currentPanel->setTypingSelf(TYPING_STATUS_CLEAR);
        } else {
            if (currentPanel->getTypingSelf() != TYPING_STATUS_TYPING) {
                typingContinued(currentPanel);
                currentPanel->setTypingSelf(TYPING_STATUS_TYPING);
            }
        }
    }
}

void flist_messenger::typingCleared(FChannelPanel *channel) {
    account->getSessionByCharacter(charName)->sendTypingNotification(channel->recipient(), TYPING_STATUS_CLEAR);
}

void flist_messenger::typingContinued(FChannelPanel *channel) {
    account->getSessionByCharacter(charName)->sendTypingNotification(channel->recipient(), TYPING_STATUS_TYPING);
}

void flist_messenger::typingPaused(FChannelPanel *channel) {
    account->getSessionByCharacter(charName)->sendTypingNotification(channel->recipient(), TYPING_STATUS_PAUSED);
}

void flist_messenger::ul_pmRequested() {
    openPMTab();
}

void flist_messenger::ul_infoRequested() {
    characterInfoDialogRequested();
}

void flist_messenger::ul_ignoreAdd() {
    FSession *session = account->getSession(charName); // todo: fix this
    if (session->isCharacterIgnored(ul_recent_name)) {
        printDebugInfo("[CLIENT BUG] Tried to ignore somebody who is already on the ignorelist.");
    } else {
        session->sendIgnoreAdd(ul_recent_name);
    }
}

void flist_messenger::ul_ignoreRemove() {
    FSession *session = account->getSession(charName); // todo: fix this
    if (!session->isCharacterIgnored(ul_recent_name)) {
        printDebugInfo("[CLIENT BUG] Tried to unignore somebody who is not on the ignorelist.");
    } else {
        session->sendIgnoreDelete(ul_recent_name);
    }
}

void flist_messenger::ul_channelBan() {
    account->getSessionByCharacter(charName)->banFromChannel(currentPanel->getChannelName(), ul_recent_name);
}

void flist_messenger::ul_channelKick() {
    account->getSessionByCharacter(charName)->kickFromChannel(currentPanel->getChannelName(), ul_recent_name);
}

void flist_messenger::ul_chatBan() {
    account->getSessionByCharacter(charName)->banFromChat(ul_recent_name);
}

void flist_messenger::ul_chatKick() {
    account->getSessionByCharacter(charName)->kickFromChat(ul_recent_name);
}

void flist_messenger::ul_chatTimeout() {
    timeoutDialogRequested();
}

void flist_messenger::ul_channelOpAdd() {
    account->getSessionByCharacter(charName)->giveChanop(currentPanel->getChannelName(), ul_recent_name);
}

void flist_messenger::ul_channelOpRemove() {
    account->getSessionByCharacter(charName)->takeChanop(currentPanel->getChannelName(), ul_recent_name);
}

void flist_messenger::ul_chatOpAdd() {
    account->getSessionByCharacter(charName)->giveGlobalop(ul_recent_name);
}

void flist_messenger::ul_chatOpRemove() {
    account->getSessionByCharacter(charName)->takeGlobalop(ul_recent_name);
}

void flist_messenger::ul_profileRequested() {
    QUrl link(QString("https://www.f-list.net/c/%1/").arg(ul_recent_name));
    QDesktopServices::openUrl(link);
}

void flist_messenger::ul_copyLink() {
    QString link = QString("https://www.f-list.net/c/%1/").arg(ul_recent_name);

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(link, QClipboard::Clipboard);
    clipboard->setText(link, QClipboard::Selection);
}

void flist_messenger::setupTimeoutDialog() {
    QVBoxLayout *to_vblOverview;
    QHBoxLayout *to_hblButtons;
    QLabel *to_lblWho;
    QLabel *to_lblLength;
    QLabel *to_lblReason;
    QPushButton *to_btnSubmit;
    QPushButton *to_btnCancel;
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

void flist_messenger::timeoutDialogRequested() {
    if (timeoutDialog == 0 || timeoutDialog->parent() != this) setupTimeoutDialog();
    to_leWho->setText(ul_recent_name);
    timeoutDialog->show();
}

void flist_messenger::to_btnSubmitClicked() {
    QString who = to_leWho->text();
    QString length = to_leLength->text();
    QString why = to_leReason->text();
    bool ok = true;
    int minutes = length.toInt(&ok);

    if (!ok || minutes <= 0 || 90 < minutes) {
        QString error("Wrong length.");
        messageSystem(0, error, MESSAGE_TYPE_FEEDBACK);
    } else if (who.simplified() == "" || why.simplified() == "") {
        QString error("Didn't fill out all fields.");
        messageSystem(0, error, MESSAGE_TYPE_FEEDBACK);
    } else {
        account->getSessionByCharacter(charName)->sendCharacterTimeout(who.simplified(), minutes, why.simplified());
    }
    timeoutDialog->hide();
}

void flist_messenger::to_btnCancelClicked() {
    timeoutDialog->hide();
}

/**
Update the user interface based upon the mode of the current panel.
 */
void flist_messenger::updateChannelMode() {
    if (currentPanel->getMode() == CHANNEL_MODE_CHAT || currentPanel->type() == FChannel::CHANTYPE_PM || currentPanel->type() == FChannel::CHANTYPE_CONSOLE) {
        // Chat only, so disable ability to send ads.
        btnSendAdv->setDisabled(true);
        btnSendChat->setDisabled(false);
    } else if (currentPanel->getMode() == CHANNEL_MODE_ADS) {
        // Ads only, disable the ability to send regular chat messages.
        btnSendAdv->setDisabled(false);
        btnSendChat->setDisabled(true);
    } else {
        // Regular channel, allow both ads and chat messages.
        btnSendAdv->setDisabled(false);
        btnSendChat->setDisabled(false);
    }
}

void flist_messenger::switchTab(QString &tabname) {
    if (channelList.count(tabname) == 0 && tabname != "CONSOLE") {
        printDebugInfo("ERROR: Tried to switch to " + tabname.toStdString() + " but it doesn't exist.");
        return;
    }

    QString input = plainTextEdit->toPlainText();

    if (currentPanel && currentPanel->type() == FChannel::CHANTYPE_PM && currentPanel->getTypingSelf() == TYPING_STATUS_TYPING) {
        typingPaused(currentPanel);
    }

    currentPanel->setInput(input);

    currentPanel->pushButton->setChecked(false);
    FChannelPanel *chan = 0;

    if (tabname == "CONSOLE")
        chan = console;
    else
        chan = channelList.value(tabname);

    currentPanel = chan;
    lblChannelName->setText(chan->title());
    input = currentPanel->getInput();
    plainTextEdit->setPlainText(input);
    plainTextEdit->setFocus();
    currentPanel->setHighlighted(false);
    currentPanel->setHasNewMessages(false);
    currentPanel->updateButtonColor();
    currentPanel->pushButton->setChecked(true);
    refreshUserlist();
    refreshChatLines();
    chatview->verticalScrollBar()->setSliderPosition(chatview->verticalScrollBar()->maximum());
    QTimer::singleShot(0, this, SLOT(scrollChatViewEnd()));
    updateChannelMode();
    chatview->setSessionID(currentPanel->getSessionID());
}

/**
A slot used to scroll to the very end of the chat view. This is
intended to be used from a 0 second one shot timer, to ensure that any
widgets that have deferred their resizing have done their business.
 */
void flist_messenger::scrollChatViewEnd() {
    chatview->verticalScrollBar()->setSliderPosition(chatview->verticalScrollBar()->maximum());
}

void flist_messenger::openPMTab() {
    openPMTab(ul_recent_name);
}

void flist_messenger::openPMTab(QString character) {
    FSession *session = account->getSession(currentPanel->getSessionID());
    if (character.toLower() == session->character.toLower()) {
        QString msg = "You can't PM yourself!";
        messageSystem(session, msg, MESSAGE_TYPE_FEEDBACK);
        return;
    }
    if (!session->isCharacterOnline(character)) {
        QString msg = "That character is not logged in.";
        messageSystem(session, msg, MESSAGE_TYPE_FEEDBACK);
        return;
    }

    QString panelname = "PM|||" + session->getSessionID() + "|||" + character;

    if (channelList.count(panelname) != 0) {
        channelList[panelname]->setActive(true);
        channelList[panelname]->pushButton->setVisible(true);
        switchTab(panelname);
    } else {
        channelList[panelname] = new FChannelPanel(this, session->getSessionID(), panelname, character, FChannel::CHANTYPE_PM);
        FCharacter *charptr = session->getCharacter(character);
        QString paneltitle;
        if (charptr != NULL) {
            paneltitle = charptr->PMTitle();
        } else {
            paneltitle = "Private chat with " + character;
        }
        FChannelPanel *pmPanel = channelList.value(panelname);
        pmPanel->setTitle(paneltitle);
        pmPanel->setRecipient(character);
        pmPanel->pushButton = addToActivePanels(panelname, character, paneltitle);
        switchTab(panelname);
    }
}

void flist_messenger::btnSendChatClicked() {
    // SLOT
    parseInput();
}

// todo: Move most of this functionality into FSession
void flist_messenger::btnSendAdvClicked() {
    if (settings->getPlaySounds()) {
        soundPlayer.play(soundPlayer.SOUND_CHAT);
    }
    QPlainTextEdit *messagebox = this->findChild<QPlainTextEdit *>(QString("chatinput"));
    QString inputText = QString(messagebox->toPlainText());
    if (currentPanel == 0) {
        printDebugInfo("[CLIENT ERROR] currentPanel == 0");
        return;
    }
    FSession *session = account->getSession(currentPanel->getSessionID());
    if (inputText.length() == 0) {
        messageSystem(session, QString("<b>Error:</b> No message."), MESSAGE_TYPE_FEEDBACK);
        return;
    }
    if (currentPanel == console || currentPanel->getMode() == CHANNEL_MODE_CHAT || currentPanel->type() == FChannel::CHANTYPE_PM) {
        messageSystem(session, QString("<b>Error:</b> Can't advertise here."), MESSAGE_TYPE_FEEDBACK);
        return;
    }
    // todo: Use the configuration stored in the session.
    if (inputText.length() > flist_messenger::BUFFERPUB) {
        messageSystem(session, QString("<b>Error:</b> Message exceeds the maximum number of characters."), MESSAGE_TYPE_FEEDBACK);
        return;
    }
    plainTextEdit->clear();
    session->sendChannelAdvertisement(currentPanel->getChannelName(), inputText);
}

// todo: Move some of this functionality into the FSession class.
void flist_messenger::parseInput() {
    if (settings->getPlaySounds()) {
        soundPlayer.play(soundPlayer.SOUND_CHAT);
    }

    bool pm = (bool)(currentPanel->type() == FChannel::CHANTYPE_PM);
    QPlainTextEdit *messagebox = this->findChild<QPlainTextEdit *>(QString("chatinput"));
    QString inputText = QString(messagebox->toPlainText());

    bool isCommand = (inputText[0] == '/');

    if (!isCommand && currentPanel == console) return;

    FSession *session = account->getSession(currentPanel->getSessionID());

    bool success = false;
    bool plainmessage = false;
    QString gt = "&gt;";
    QString lt = "&lt;";
    QString amp = "&amp;";
    QString ownText = inputText;
    ownText.replace('&', amp).replace('<', lt).replace('>', gt);
    if (inputText.length() == 0) {
        messageSystem(session, QString("<b>Error:</b> No message."), MESSAGE_TYPE_FEEDBACK);
        return;
    }
    if (currentPanel == 0) {
        printDebugInfo("[CLIENT ERROR] currentPanel == 0");
        return;
    }

    // todo: Make these read the configuration value from the session.
    int buffermaxlength;
    if (pm) {
        buffermaxlength = flist_messenger::BUFFERPRIV;
    } else {
        buffermaxlength = flist_messenger::BUFFERPUB;
    }

    // todo: Should the maximum length only apply to stuff tagged 'plainmessage'?
    if (inputText.length() > buffermaxlength) {
        messageSystem(session, QString("<B>Error:</B> Message exceeds the maximum number of characters."), MESSAGE_TYPE_FEEDBACK);
        return;
    }

    if (isCommand) {
        QStringList parts = inputText.split(' ');
        QString slashcommand = parts[0].toLower();
        if (slashcommand == "/clear") {
            chatview->clear();
            if (currentPanel) {
                currentPanel->clearLines();
            }
            success = true;
        } else if (slashcommand == "/debug") {
            session->sendDebugCommand(parts[1]);
            success = true;
        } else if (slashcommand == "/debugrecv") {
            // Artificially receive a packet from the server. The packet is not validated.
            if (session) {
                session->wsRecv(ownText.mid(11).toStdString());
                success = true;
            } else {
                messageSystem(session, QString("Can't do '/debugrecv', as there is no session associated with this console."), MESSAGE_TYPE_FEEDBACK);
            }
        } else if (slashcommand == "/debugsend") {
            // Send a packet directly to the server. The packet is not validated.
            if (session) {
                std::string send = ownText.mid(11).toStdString();
                session->wsSend(send);
                success = true;
            } else {
                messageSystem(session, QString("Can't do '/debugsend', as there is no session associated with this console."), MESSAGE_TYPE_FEEDBACK);
            }
        } else if (slashcommand == "/me") {
            plainmessage = true;
            success = true;
        } else if (slashcommand == "/me's") {
            plainmessage = true;
            success = true;
        } else if (slashcommand == "/join") {
            QString channel = inputText.mid(6, -1).simplified();
            session->joinChannel(channel);
            success = true;
        } else if (slashcommand == "/leave" || slashcommand == "/close") {
            // todo: Detect other console types.
            if (currentPanel == console || !session) {
                messageSystem(session, QString("Can't close and leave, as this is a console."), MESSAGE_TYPE_FEEDBACK);
                success = false;
            } else {
                leaveChannelPanel(currentPanel->getPanelName());
                success = true;
            }
        } else if (slashcommand == "/status") {
            QString status = parts[1];
            QString statusMsg = inputText.mid(9 + status.length(), -1).simplified();
            changeStatus(status, statusMsg);
            success = true;
        } else if (slashcommand == "/users") {
            usersCommand();
            success = true;
        } else if (slashcommand == "/priv") {
            QString character = inputText.mid(6).simplified();
            plainTextEdit->clear();
            openPMTab(character);
            success = true;
        } else if (slashcommand == "/ignore") {
            QString character = inputText.mid(8).simplified();

            if (character != "") {
                session->sendIgnoreAdd(character);
                success = true;
            }
        } else if (slashcommand == "/unignore") {
            QString character = inputText.mid(10).simplified();

            if (character != "") {
                if (!session->isCharacterIgnored(character)) {
                    QString out = QString("This character is not in your ignore list.");
                    messageSystem(session, out, MESSAGE_TYPE_FEEDBACK);
                } else {
                    session->sendIgnoreDelete(character);
                    success = true;
                }
            }
        } else if (slashcommand == "/ignorelist") {
            ignoreDialogRequested();
            success = true;
        } else if (slashcommand == "/channels" || slashcommand == "/prooms") {
            channelsDialogRequested();
            success = true;
        } else if (slashcommand == "/kick") {
            session->kickFromChannel(currentPanel->getChannelName(), inputText.mid(6).simplified());
            success = true;
        } else if (slashcommand == "/gkick") {
            session->kickFromChat(inputText.mid(7).simplified());
            success = true;
        } else if (slashcommand == "/ban") {
            session->banFromChannel(currentPanel->getChannelName(), inputText.mid(5).simplified());
            success = true;
        } else if (slashcommand == "/accountban") {
            session->banFromChat(inputText.mid(12).simplified());
            success = true;
        } else if (slashcommand == "/makeroom") {
            createPrivateChannel(inputText.mid(10));
        } else if (slashcommand == "/closeroom") {
            session->setRoomIsPublic(currentPanel->getChannelName(), false);
            success = true;
        } else if (slashcommand == "/openroom") {
            session->setRoomIsPublic(currentPanel->getChannelName(), true);
            success = true;
        } else if (slashcommand == "/invite") {
            session->inviteToChannel(currentPanel->getChannelName(), inputText.mid(8).simplified());
            success = true;
        } else if (slashcommand == "/warn") {
            plainmessage = true;
            success = true;
        } else if (slashcommand == "/cop") {
            session->giveChanop(currentPanel->getChannelName(), inputText.mid(5).simplified());
            success = true;
        } else if (slashcommand == "/cdeop") {
            session->takeChanop(currentPanel->getChannelName(), inputText.mid(7).simplified());
            success = true;
        } else if (slashcommand == "/op") {
            session->giveGlobalop(inputText.mid(4).simplified());
            success = true;
        } else if (slashcommand == "/reward") {
            session->giveReward(inputText.mid(8).simplified());
            success = true;
        } else if (slashcommand == "/deop") {
            session->takeGlobalop(inputText.mid(6).simplified());
            success = true;
        } else if (slashcommand == "/code") {
            QString out = "";
            switch (currentPanel->type()) {
                case FChannel::CHANTYPE_NORMAL:
                    out = "Copy this code to your message: [b][noparse][channel]" + currentPanel->getChannelName() + "[/channel][/noparse][/b]";
                    break;
                case FChannel::CHANTYPE_ADHOC:
                    out = "Copy this code to your message: [b][noparse][session=" + currentPanel->title() + "]" + currentPanel->getChannelName() + "[/session][/noparse][/b]";
                    break;
                default:
                    out = "This command is only for channels!";
                    break;
            }
            messageSystem(session, out, MESSAGE_TYPE_FEEDBACK);
            success = true;
        } else if (slashcommand == "/unban") {
            session->unbanFromChannel(currentPanel->getChannelName(), inputText.mid(7).simplified());
            success = true;
        } else if (slashcommand == "/banlist") {
            session->requestChannelBanList(currentPanel->getChannelName());
            success = true;
        } else if (slashcommand == "/getdescription") {
            QString desc = currentPanel->description();
            messageSystem(session, desc, MESSAGE_TYPE_FEEDBACK);
            success = true;
        } else if (slashcommand == "/setdescription") {
            // [17:31 PM]>>CDS {"channel":"ADH-cbae3bdf02cd39e8949e","description":":3!"}
            // todo: Does this require more intelligent filtering on excess whitespace?
            session->setChannelDescription(currentPanel->getChannelName(), inputText.mid(16).trimmed());
            success = true;
        } else if (slashcommand == "/setowner") {
            QString errmsg;
            FChannel *channel = session->getChannel(currentPanel->getChannelName());

            success = true;
            if (parts.count() < 2) {
                errmsg = "Must supply a character name";
                messageSystem(session, errmsg, MESSAGE_TYPE_FEEDBACK);
                success = false;
            } else if (!channel) {
                errmsg = "You must be in a channel to set the owner";
                messageSystem(session, errmsg, MESSAGE_TYPE_FEEDBACK);
            } else {
                QString newOwner = inputText.mid(10);
                session->setChannelOwner(currentPanel->getChannelName(), newOwner);
            }
        } else if (slashcommand == "/coplist") {
            session->requestChanopList(currentPanel->getChannelName());
            success = true;
        } else if (slashcommand == "/timeout") {
            // [17:16 PM]>>TMO {"time":1,"character":"Arisato Hamuko","reason":"Test."}
            QStringList tparts = inputText.mid(9).split(',');
            bool isInt;
            int time = tparts[1].simplified().toInt(&isInt);

            if (isInt == false) {
                QString err = "Time is not a number.";
                messageSystem(session, err, MESSAGE_TYPE_FEEDBACK);
            } else {
                QString who(tparts[0].simplified());
                QString why(tparts[2].simplified());
                session->sendCharacterTimeout(who, time, why);
                success = true;
            }
        } else if (slashcommand == "/gunban") {
            session->unbanFromChat(inputText.mid(8).simplified());
            success = true;
        } else if (slashcommand == "/createchannel") {
            createPublicChannel(inputText.mid(15));
        } else if (slashcommand == "/killchannel") {
            session->killChannel(inputText.mid(13).simplified());
            success = true;
        } else if (slashcommand == "/broadcast") {
            session->broadcastMessage(inputText.mid(11).simplified());
            success = true;
        } else if (slashcommand == "/setmode") {
            ChannelMode mode = CHANNEL_MODE_UNKNOWN;
            if (parts.length() == 2) {
                mode = (ChannelMode)ChannelModeEnum.keyToValue(parts[1]);
            }
            if (mode == CHANNEL_MODE_UNKNOWN) {
                QString err("Correct usage: /setmode &lt;chat|ads|both&gt;");
                messageSystem(session, err, MESSAGE_TYPE_FEEDBACK);
            } else if (currentPanel->isOp(session->getCharacter(session->character)) || session->isCharacterOperator(session->character)) {
                session->setChannelMode(currentPanel->getChannelName(), mode);
                success = true;
            } else {
                QString err("You can only do that in channels you moderate.");
                messageSystem(session, err, MESSAGE_TYPE_FEEDBACK);
                success = true;
            }
        } else if (slashcommand == "/bottle") {
            if (currentPanel == 0 || currentPanel->type() == FChannel::CHANTYPE_CONSOLE || currentPanel->type() == FChannel::CHANTYPE_PM) {
                QString err("You can't use that in this panel.");
                messageSystem(session, err, MESSAGE_TYPE_FEEDBACK);
            } else {
                session->spinBottle(currentPanel->getChannelName());
            }
            success = true;
        } else if (slashcommand == "/roll") {
            if (currentPanel == 0 || currentPanel->type() == FChannel::CHANTYPE_CONSOLE) {
                QString err("You can't use that in this panel.");
                messageSystem(session, err, MESSAGE_TYPE_FEEDBACK);
            } else {
                QString roll;
                if (parts.count() < 2) {
                    roll = "1d10";
                } else {
                    roll = parts[1].toLower();
                }
                if (currentPanel->type() == FChannel::CHANTYPE_PM) {
                    session->rollDicePM(currentPanel->recipient(), roll);
                } else {
                    session->rollDiceChannel(currentPanel->getChannelName(), roll);
                }
            }
            success = true;
        } else if (debugging && slashcommand == "/channeltojson") {
            QString output("[noparse]");
            QJsonDocument *node = currentPanel->toJSON();
            output += node->toJson(QJsonDocument::Compact);
            output += "[/noparse]";
            messageSystem(session, output, MESSAGE_TYPE_FEEDBACK);
            delete node;
            success = true;
        } else if (debugging && slashcommand == "/refreshqss") {
            QFile stylefile("default.qss");
            stylefile.open(QFile::ReadOnly);
            QString stylesheet = QLatin1String(stylefile.readAll());
            setStyleSheet(stylesheet);
            QString output = "Refreshed stylesheet from default.qss";
            messageSystem(session, output, MESSAGE_TYPE_FEEDBACK);
            success = true;
        } else if (slashcommand == "/channeltostring") {
            QString *output = currentPanel->toString();
            messageSystem(session, *output, MESSAGE_TYPE_FEEDBACK);
            success = true;
            delete output;
        }
    } else {
        plainmessage = true;
        success = true;
    }

    if (success) {
        plainTextEdit->clear();
    }

    if (plainmessage) {
        if (pm) {
            session->sendCharacterMessage(currentPanel->recipient(), inputText);
        } else {
            session->sendChannelMessage(currentPanel->getChannelName(), inputText);
        }
    }
}

void flist_messenger::saveSettings() {
    FChannelPanel *c;
    defaultChannels.clear();
    foreach (c, channelList) {
        if (c->getActive() && c->type() != FChannel::CHANTYPE_CONSOLE && c->type() != FChannel::CHANTYPE_PM) {
            defaultChannels.append(c->getChannelName());
        }
    }
    settings->setDefaultChannels(defaultChannels.join("|||"));
}

void flist_messenger::loadSettings() {
    account->setUserName(settings->getUserAccount());
    defaultChannels.clear();
    defaultChannels = settings->getDefaultChannels().split("|||", Qt::SkipEmptyParts);

    keywordlist = settings->qsettings->value("Global/keywords").toString().split(",", Qt::SkipEmptyParts);
    for (int i = 0; i < keywordlist.size(); i++) {
        keywordlist[i] = keywordlist[i].trimmed();
        if (keywordlist[i].isEmpty()) {
            keywordlist.removeAt(i);
            i--;
        }
    }
}

void flist_messenger::loadDefaultSettings() {}

#define PANELNAME(channelname, charname) (((channelname).startsWith("ADH-") ? "ADH|||" : "CHAN|||") + (charname) + "|||" + (channelname))

void flist_messenger::flashApp(QString &reason) {
    printDebugInfo(reason.toStdString());
    QApplication::alert(this, 10000);
}

FSession *flist_messenger::getSession(QString sessionid) {
    return server->getSession(sessionid);
}

void flist_messenger::setChatOperator(FSession *session, QString characteroperator, bool opstatus) {
    debugMessage((opstatus ? "Added chat operator: " : "Removed chat operator: ") + characteroperator);
    // Sort userlists that contain this character
    if (session->isCharacterOnline(characteroperator)) {
        // Set flag in character
        FCharacter *character = session->getCharacter(characteroperator);
        FChannelPanel *channel = 0;
        foreach (channel, channelList) {
            // todo: filter by session
            if (channel->charList().contains(character)) {
                // todo: Maybe queue channel sorting as an idle task?
                channel->sortChars();
                if (currentPanel == channel) {
                    refreshUserlist();
                }
            }
        }
    }
}

void flist_messenger::openCharacterProfile(FSession *session, QString charactername) {
    (void)session;
    ul_recent_name = charactername;
    characterInfoDialogRequested();
}

void flist_messenger::addCharacterChat(FSession *session, QString charactername) {
    QString panelname = "PM|||" + session->getSessionID() + "|||" + charactername;
    FChannelPanel *channelpanel = channelList.value(panelname);
    if (!channelpanel) {
        channelpanel = new FChannelPanel(this, session->getSessionID(), panelname, charactername, FChannel::CHANTYPE_PM);
        channelList[panelname] = channelpanel;
        channelpanel->setTitle(charactername);
        channelpanel->setRecipient(charactername);
        FCharacter *character = session->getCharacter(charactername);
        QString title;
        if (character) {
            title = character->PMTitle();
        } else {
            title = QString("Private chat with %1 (Offline)").arg(charactername);
        }
        channelpanel->pushButton = addToActivePanels(panelname, charactername, title);
    }
    if (channelpanel->getActive() == false) {
        channelpanel->setActive(true);
        channelpanel->pushButton->setVisible(true);
    }
    channelpanel->setTyping(TYPING_STATUS_CLEAR);
    channelpanel->updateButtonColor();
}

void flist_messenger::addChannel(FSession *session, QString channelname, QString title) {
    debugMessage("addChannel(\"" + channelname + "\", \"" + title + "\")");
    FChannelPanel *channelpanel;
    QString panelname = PANELNAME(channelname, session->getSessionID());
    if (!channelList.contains(panelname)) {
        if (channelname.startsWith("ADH-")) {
            channelpanel = new FChannelPanel(this, session->getSessionID(), panelname, channelname, FChannel::CHANTYPE_ADHOC);
        } else {
            channelpanel = new FChannelPanel(this, session->getSessionID(), panelname, channelname, FChannel::CHANTYPE_NORMAL);
        }
        channelList[panelname] = channelpanel;
        channelpanel->setTitle(title);
        channelpanel->pushButton = addToActivePanels(panelname, channelname, title);
    } else {
        channelpanel = channelList.value(panelname);
        // Ensure that the channel's title is set correctly for ad-hoc channels.
        if (channelname != title && channelpanel->title() != title) {
            channelpanel->setTitle(title);
        }
        if (channelpanel->getActive() == false) {
            channelpanel->setActive(true);
            channelpanel->pushButton->setVisible(true);
        }
    }
    // todo: Update UI elements with base on channel mode? (chat/RP AD/both)
}

void flist_messenger::removeChannel(FSession *session, QString channelname) {
    (void)session;
    (void)channelname;
}

void flist_messenger::addChannelCharacter(FSession *session, QString channelname, QString charactername, bool notify) {
    FChannelPanel *channelpanel;
    QString panelname = PANELNAME(channelname, session->getSessionID());
    if (!session->isCharacterOnline(charactername)) {
        printDebugInfo("[SERVER BUG]: Server told us about a character joining a channel, but we don't know about them yet. " + charactername.toStdString());
        return;
    }
    if (!channelList.contains(panelname)) {
        printDebugInfo("[BUG]: Told about a character joining a channel, but the panel for the channel doesn't exist. " + channelname.toStdString());
        return;
    }
    channelpanel = channelList.value(panelname);
    channelpanel->addChar(session->getCharacter(charactername), notify);
    if (charactername == session->character) {
        switchTab(panelname);
    } else {
        if (notify) {
            if (currentPanel->getChannelName() == channelname) {
                // Only refresh the userlist if the panel has focus and notify is set.
                refreshUserlist();
            }
            messageChannel(session, channelname, "<b>" + charactername + "</b> has joined the channel", MESSAGE_TYPE_JOIN);
        }
    }
}

void flist_messenger::removeChannelCharacter(FSession *session, QString channelname, QString charactername) {
    FChannelPanel *channelpanel;
    QString panelname = PANELNAME(channelname, session->getSessionID());
    if (!session->isCharacterOnline(charactername)) {
        printDebugInfo("[SERVER BUG]: Server told us about a character leaving a channel, but we don't know about them yet. " + charactername.toStdString());
        return;
    }
    if (!channelList.contains(panelname)) {
        printDebugInfo("[BUG]: Told about a character leaving a channel, but the panel for the channel doesn't exist. " + channelname.toStdString());
        return;
    }
    channelpanel = channelList.value(panelname);
    channelpanel->remChar(session->getCharacter(charactername));
    if (currentPanel->getChannelName() == channelname) {
        refreshUserlist();
    }
    messageChannel(session, channelname, "<b>" + charactername + "</b> has left the channel", MESSAGE_TYPE_LEAVE);
}

void flist_messenger::setChannelOperator(FSession *session, QString channelname, QString charactername, bool opstatus) {
    QString panelname = PANELNAME(channelname, session->getSessionID());
    FChannelPanel *channelpanel = channelList.value(panelname);
    if (channelpanel) {
        if (opstatus) {
            channelpanel->addOp(charactername);
        } else {
            channelpanel->removeOp(charactername);
        }
        if (currentPanel == channelpanel) {
            refreshUserlist();
        }
    }
}

void flist_messenger::joinChannel(FSession *session, QString channelname) {
    (void)session;
    if (currentPanel->getChannelName() == channelname) {
        refreshUserlist();
    }
    // todo: Refresh any elements in a half prepared state.
}

/**
This notifies the UI that the given session has now left the given channel. The UI should close or disable the releavent widgets. It should not send any leave commands.
 */
void flist_messenger::leaveChannel(FSession *session, QString channelname) {
    QString panelname = PANELNAME(channelname, session->getSessionID());
    if (!channelList.contains(panelname)) {
        printDebugInfo("[BUG]: Told to leave a channel, but the panel for the channel doesn't exist. " + channelname.toStdString());
        return;
    }
    closeChannelPanel(panelname);
}

void flist_messenger::setChannelDescription(FSession *session, QString channelname, QString description) {
    QString panelname = PANELNAME(channelname, session->getSessionID());
    FChannelPanel *channelpanel = channelList.value(panelname);
    if (!channelpanel) {
        printDebugInfo(QString("[BUG]: Was told the description of the channel '%1', but the panel for the channel doesn't exist.").arg(channelname).toStdString());
        return;
    }
    channelpanel->setDescription(description);
    QString message = QString("You have joined <b>%1</b>: %2").arg(channelpanel->title()).arg(bbparser.parse(description));
    messageChannel(session, channelname, message, MESSAGE_TYPE_CHANNEL_DESCRIPTION, true, false);
}

void flist_messenger::setChannelMode(FSession *session, QString channelname, ChannelMode mode) {
    QString panelname = PANELNAME(channelname, session->getSessionID());
    FChannelPanel *channelpanel = channelList.value(panelname);
    if (!channelpanel) {
        printDebugInfo(QString("[BUG]: Was told the mode of the channel '%1', but the panel for the channel doesn't exist.").arg(channelname).toStdString());
        return;
    }
    channelpanel->setMode(mode);
    if (channelpanel == currentPanel) {
        // update UI with mode state
        updateChannelMode();
    }
}

/**
The initial flood of channel data is complete and delayed tasks like sorting can now be performed.
 */
void flist_messenger::notifyChannelReady(FSession *session, QString channelname) {
    QString panelname = PANELNAME(channelname, session->getSessionID());
    FChannelPanel *channelpanel = channelList.value(panelname);
    if (!channelpanel) {
        printDebugInfo(QString("[BUG]: Was notified that the channel '%1' was ready, but the panel for the channel doesn't exist.").arg(channelname).toStdString());
        return;
    }
    channelpanel->sortChars();
    if (currentPanel == channelpanel) {
        refreshUserlist();
    }
}

void flist_messenger::notifyCharacterOnline(FSession *session, QString charactername, bool online) {
    QString panelname = "PM|||" + session->getSessionID() + "|||" + charactername;
    QList<QString> channels;
    QList<QString> characters;
    bool system = session->isCharacterFriend(charactername);
    if (channelList.contains(panelname)) {
        characters.append(charactername);
        system = true;
        // todo: Update panel with changed online/offline status.
    }
    if (characters.count() > 0 || channels.count() > 0 || system) {
        messageMany(session, channels, characters, system, "<b>" + charactername + "</b> is now " + (online ? "online." : "offline."),
                    online ? MESSAGE_TYPE_ONLINE : MESSAGE_TYPE_OFFLINE);
    }
}

void flist_messenger::notifyCharacterStatusUpdate(FSession *session, QString charactername) {
    QString panelname = "PM|||" + session->getSessionID() + "|||" + charactername;
    QList<QString> channels;
    QList<QString> characters;
    bool system = session->isCharacterFriend(charactername);
    if (channelList.contains(panelname)) {
        characters.append(charactername);
        system = true;
        // todo: Update panel with changed status.
    }
    if (characters.count() > 0 || channels.count() > 0 || system) {
        FCharacter *character = session->getCharacter(charactername);
        QString statusmessage = character->statusMsg();
        if (!statusmessage.isEmpty()) {
            statusmessage = QString(" (%1)").arg(statusmessage);
        }
        QString message = QString("<b>%1</b> is now %2%3").arg(charactername).arg(character->statusString()).arg(statusmessage);
        messageMany(session, channels, characters, system, message, MESSAGE_TYPE_STATUS);
    }
    // Refresh character list if they are present in the current panel.
    if (currentPanel->hasCharacter(session->getCharacter(charactername))) {
        refreshUserlist();
    }
}

void flist_messenger::setCharacterTypingStatus(FSession *session, QString charactername, TypingStatus typingstatus) {
    QString panelname = "PM|||" + session->getSessionID() + "|||" + charactername;
    FChannelPanel *channelpanel;
    channelpanel = channelList.value(panelname);
    if (!channelpanel) {
        return;
    }
    channelpanel->setTyping(typingstatus);
    channelpanel->updateButtonColor();
}

void flist_messenger::notifyCharacterCustomKinkDataUpdated(FSession *session, QString charactername) {
    if (!ci_dialog) {
        debugMessage(QString("Received custom kink data for the character '%1' but the profile window has not been created.").arg(charactername));
        ci_dialog = new FCharacterInfoDialog(this);
    }
    FCharacter *character = session->getCharacter(charactername);
    if (!character) {
        debugMessage(QString("Received custom kink data for the character '%1' but the character is not known.").arg(charactername));
        return;
    }
    ci_dialog->updateKinks(character);
}

void flist_messenger::notifyCharacterProfileDataUpdated(FSession *session, QString charactername) {
    if (!ci_dialog) {
        debugMessage(QString("Received profile data for the character '%1' but the profile window has not been created.").arg(charactername));
        ci_dialog = new FCharacterInfoDialog(this);
    }
    FCharacter *character = session->getCharacter(charactername);
    if (!character) {
        debugMessage(QString("Received profile data for the character '%1' but the character is not known.").arg(charactername));
        return;
    }

    ci_dialog->updateProfile(character);
}

void flist_messenger::notifyIgnoreAdd(FSession *s, QString character) {
    messageSystem(s, QString("%1 has been added to your ignore list.").arg(character), MESSAGE_TYPE_IGNORE_UPDATE);
}

void flist_messenger::notifyIgnoreRemove(FSession *s, QString character) {
    messageSystem(s, QString("%1 has been removed from your ignore list.").arg(character), MESSAGE_TYPE_IGNORE_UPDATE);
}

// todo: Making channelkey should be moved out to messageMessage().
bool flist_messenger::getChannelBool(QString key, FChannelPanel *channelpanel, bool dflt) {
    QString channelkey;
    if (channelpanel->type() == FChannel::CHANTYPE_PM) {
        channelkey = QString("Character/%1/%2").arg(channelpanel->recipient(), key);
    } else {
        channelkey = QString("Character/%1/%2").arg(channelpanel->getChannelName(), key);
    }
    return settings->getBool(channelkey, dflt);
}

bool flist_messenger::needsAttention(QString key, FChannelPanel *channelpanel, AttentionMode dflt) {
    // todo: check settings from the channel itself
    AttentionMode attentionmode;
    QString channelkey;
    if (channelpanel->type() == FChannel::CHANTYPE_PM) {
        channelkey = QString("Character/%1/%2").arg(channelpanel->recipient(), key);
    } else {
        channelkey = QString("Character/%1/%2").arg(channelpanel->getChannelName(), key);
    }
    attentionmode = (AttentionMode)AttentionModeEnum.keyToValue(settings->getString(channelkey), ATTENTION_DEFAULT);
    // debugMessage(QString("Channel (%1) attention mode: %2").arg(channelkey, AttentionModeEnum.valueToKey(attentionmode)));
    if (attentionmode == ATTENTION_DEFAULT) {
        attentionmode = (AttentionMode)AttentionModeEnum.keyToValue(settings->getString("Global/" + key), ATTENTION_DEFAULT);
    }
    // debugMessage(QString("Global attention mode: %1").arg(AttentionModeEnum.valueToKey(attentionmode)));
    if (attentionmode == ATTENTION_DEFAULT) {
        attentionmode = dflt;
    }
    // debugMessage(QString("Final attention mode: %1").arg(AttentionModeEnum.valueToKey(attentionmode)));
    switch (attentionmode) {
        case ATTENTION_DEFAULT:
        case ATTENTION_NEVER:
            return false;
        case ATTENTION_IFNOTFOCUSED:
            if (channelpanel == currentPanel) {
                return false;
            }
            return true;
        case ATTENTION_ALWAYS:
            return true;
    }
    // should be unreachable
    return false;
}

void flist_messenger::messageMessage(FMessage message) {
    QStringList panelnames;
    QString sessionid = message.getSessionID();
    FSession *session = getSession(sessionid);
    // bool destinationchannelalwaysding = false; //1 or more destination channels that are set to always ding
    bool message_rpad_ding = false;
    bool message_channel_ding = false;
    bool message_character_ding = false;
    bool message_keyword_ding = false;
    bool message_rpad_flash = false;
    bool message_channel_flash = false;
    bool message_character_flash = false;
    bool message_keyword_flash = false;
    QString panelname;
    bool globalkeywordmatched = false;
    QString plaintext = message.getPlainTextMessage();
    switch (message.getMessageType()) {
        case MESSAGE_TYPE_ROLL:
        case MESSAGE_TYPE_RPAD:
        case MESSAGE_TYPE_CHAT:
            foreach (QString keyword, keywordlist) {
                if (plaintext.contains(keyword, Qt::CaseInsensitive)) {
                    globalkeywordmatched = true;
                    break;
                }
            }
            break;
        default:
            break;
    }

    if (!session) {
        debugMessage("[CLIENT BUG] Sessionless messages are not handled yet.");
        // todo: Sanity check that character and  channel lists are empty
        // todo: check console
        // todo: check notify
    } else {
        if (message.getBroadcast()) {
            // Doing a broadcast, find all panels for this session and flag them.
            foreach (FChannelPanel *channelpanel, channelList) {
                if (channelpanel->getSessionID() == sessionid) {
                    panelnames.append(channelpanel->getPanelName());
                }
            }
        } else {
            foreach (QString charactername, message.getDestinationCharacterList()) {
                panelname = "PM|||" + sessionid + "|||" + charactername;
                panelnames.append(panelname);
            }
            foreach (QString channelname, message.getDestinationChannelList()) {
                panelname = PANELNAME(channelname, sessionid);
                panelnames.append(panelname);
            }
            if (message.getConsole()) {
                panelnames.append("FCHATSYSTEMCONSOLE");
            }
        }
    }
    if (message.getNotify()) {
        // todo: should this be made session aware?
        if (!panelnames.contains(currentPanel->getPanelName())) {
            panelnames.append(currentPanel->getPanelName());
        }
    }
    foreach (QString panelname, panelnames) {
        FChannelPanel *channelpanel;
        channelpanel = channelList.value(panelname);
        if (!channelpanel) {
            debugMessage("[BUG] Tried to put a message on '" + panelname + "' but there is no channel panel for it. message:" + message.getFormattedMessage());
            continue;
        }
        // Filter based on message type.
        switch (message.getMessageType()) {
            case MESSAGE_TYPE_LOGIN:
                break;
            case MESSAGE_TYPE_ONLINE:
            case MESSAGE_TYPE_OFFLINE:
                if (!settings->getShowOnlineOfflineMessage()) {
                    continue;
                }
                break;
            case MESSAGE_TYPE_STATUS:
                if (!settings->getShowOnlineOfflineMessage()) {
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
                if (!settings->getShowJoinLeaveMessage()) {
                    continue;
                }
                break;
            case MESSAGE_TYPE_ROLL:
            case MESSAGE_TYPE_RPAD:
            case MESSAGE_TYPE_CHAT:
                switch (message.getMessageType()) {
                    case MESSAGE_TYPE_ROLL:
                        // todo: should rolls treated like ads or messages or as their own thing?
                    case MESSAGE_TYPE_RPAD:
                        message_rpad_ding |= needsAttention("message_rpad_ding", channelpanel, ATTENTION_NEVER);
                        message_rpad_flash |= needsAttention("message_rpad_flash", channelpanel, ATTENTION_NEVER);
                        break;
                    case MESSAGE_TYPE_CHAT:
                        if (channelpanel->type() == FChannel::CHANTYPE_PM) {
                            message_character_ding |= needsAttention("message_character_ding", channelpanel, ATTENTION_ALWAYS);
                            message_character_flash |= needsAttention("message_character_flash", channelpanel, ATTENTION_NEVER);
                        } else {
                            message_channel_ding |= needsAttention("message_channel_ding", channelpanel, ATTENTION_NEVER);
                            message_channel_flash |= needsAttention("message_channel_flash", channelpanel, ATTENTION_NEVER);
                        }
                        break;
                    default:
                        break;
                }
                // Per channel keyword matching.
                foreach (QString keyword, channelpanel->getKeywordList()) {
                    if (plaintext.contains(keyword, Qt::CaseInsensitive)) {
                        message_keyword_ding |= true;
                        message_keyword_flash |= true;
                        break;
                    }
                }
                // Can it match global keywords?
                if (globalkeywordmatched && !getChannelBool("ignore_global_keywords", channelpanel, false)) {
                    message_keyword_ding |= true;
                    message_keyword_flash |= true;
                }
                channelpanel->setHasNewMessages(true);
                if (panelname.startsWith("PM")) {
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
                debugMessage("Unhandled message type " + QString::number(message.getMessageType()) + " for message '" + message.getFormattedMessage() + "'.");
        }
        channelpanel->addLine(message.getFormattedMessage(), settings->getLogChat());
        if (channelpanel == currentPanel) {
            chatview->append(message.getFormattedMessage());
        }
    }
    // if(session && message.getSourceCharacter() == session->character) {
    //	//Message originated from the user, so don't play any sounds.
    //	return;
    // }

    FSound::soundName soundtype = FSound::SOUND_NONE;
    bool flash = message_rpad_flash || message_channel_flash || message_character_flash || message_keyword_flash;

    if (message_rpad_ding || message_channel_ding || message_character_ding || message_keyword_ding) {
        if (session && message.getSourceCharacter() != session->character) {
            soundtype = FSound::SOUND_ATTENTION;
        }
    }

    switch (message.getMessageType()) {
        case MESSAGE_TYPE_LOGIN:
        case MESSAGE_TYPE_ONLINE:
        case MESSAGE_TYPE_OFFLINE:
        case MESSAGE_TYPE_STATUS:
        case MESSAGE_TYPE_CHANNEL_DESCRIPTION:
        case MESSAGE_TYPE_CHANNEL_MODE:
        case MESSAGE_TYPE_CHANNEL_INVITE:
        case MESSAGE_TYPE_JOIN:
        case MESSAGE_TYPE_LEAVE:
            break;
        case MESSAGE_TYPE_ROLL:
        case MESSAGE_TYPE_RPAD:
        case MESSAGE_TYPE_CHAT:
            // Detection is handled above.
            break;
        case MESSAGE_TYPE_REPORT:
            soundtype = FSound::SOUND_MODALERT;
            break;
        case MESSAGE_TYPE_ERROR:
            break;
        case MESSAGE_TYPE_SYSTEM:
        case MESSAGE_TYPE_BROADCAST:
            soundtype = FSound::SOUND_ATTENTION;
            break;
        case MESSAGE_TYPE_FEEDBACK:
            break;
        case MESSAGE_TYPE_KICK:
        case MESSAGE_TYPE_KICKBAN:
            break;
        case MESSAGE_TYPE_IGNORE_UPDATE:
            break;
        default:
            debugMessage("Unhandled sound for message type " + QString::number(message.getMessageType()) + " for message '" + message.getFormattedMessage() + "'.");
    }
    // debugMessage(QString("Sound: %1").arg(soundtype));
    if (soundtype != FSound::SOUND_NONE && settings->getPlaySounds()) {
        soundPlayer.play(soundtype);
    }
    if (flash) {
        // todo: Special handling on message type?
        QString reason(message.getFormattedMessage());
        flashApp(reason);
    }
}

void flist_messenger::messageMany(QList<QString> &panelnames, QString message, MessageType messagetype) {
    // Put the message on all the given channel panels.
    QString panelname;
    QString messageout = "<small>[" + QTime::currentTime().toString("hh:mm:ss AP") + "]</small> " + message;
    foreach (panelname, panelnames) {
        FChannelPanel *channelpanel;
        channelpanel = channelList.value(panelname);
        if (!channelpanel) {
            debugMessage("[BUG] Tried to put a message on '" + panelname + "' but there is no channel panel for it. message:" + message);
            continue;
        }
        // Filter based on message type.
        switch (messagetype) {
            case MESSAGE_TYPE_LOGIN:
                break;
            case MESSAGE_TYPE_ONLINE:
            case MESSAGE_TYPE_OFFLINE:
                if (!settings->getShowOnlineOfflineMessage()) {
                    continue;
                }
                break;
            case MESSAGE_TYPE_STATUS:
                if (!settings->getShowOnlineOfflineMessage()) {
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
                if (!settings->getShowJoinLeaveMessage()) {
                    continue;
                }
                break;
            case MESSAGE_TYPE_ROLL:
            case MESSAGE_TYPE_RPAD:
            case MESSAGE_TYPE_CHAT:
                // todo: trigger sounds
                channelpanel->setHasNewMessages(true);
                if (panelname.startsWith("PM")) {
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
        if (channelpanel == currentPanel) {
            chatview->append(messageout);
        }
    }
    // todo: Sound support is still less than what it was originally.
    if (/*se_ping &&*/ settings->getPlaySounds()) {
        switch (messagetype) {
            case MESSAGE_TYPE_LOGIN:
            case MESSAGE_TYPE_ONLINE:
            case MESSAGE_TYPE_OFFLINE:
            case MESSAGE_TYPE_STATUS:
            case MESSAGE_TYPE_CHANNEL_DESCRIPTION:
            case MESSAGE_TYPE_CHANNEL_MODE:
            case MESSAGE_TYPE_CHANNEL_INVITE:
            case MESSAGE_TYPE_JOIN:
            case MESSAGE_TYPE_LEAVE:
                break;
            case MESSAGE_TYPE_ROLL:
            case MESSAGE_TYPE_RPAD:
            case MESSAGE_TYPE_CHAT:
                soundPlayer.play(FSound::SOUND_CHAT);
                break;
            case MESSAGE_TYPE_REPORT:
                soundPlayer.play(FSound::SOUND_MODALERT);
                break;
            case MESSAGE_TYPE_ERROR:
            case MESSAGE_TYPE_SYSTEM:
            case MESSAGE_TYPE_BROADCAST:
                soundPlayer.play(FSound::SOUND_ATTENTION);
                break;
            case MESSAGE_TYPE_FEEDBACK:
                break;
            case MESSAGE_TYPE_KICK:
            case MESSAGE_TYPE_KICKBAN:
                break;
            case MESSAGE_TYPE_IGNORE_UPDATE:
                break;
            case MESSAGE_TYPE_NOTE:
                soundPlayer.play(FSound::SOUND_NEWNOTE);
                break;
            case MESSAGE_TYPE_FRIEND:
                soundPlayer.play(FSound::SOUND_FRIENDUPDATE);
                break;
            case MESSAGE_TYPE_BOOKMARK:
                break;
            default:
                debugMessage("Unhandled sound for message type " + QString::number(messagetype) + " for message '" + message + "'.");
        }
    }
    // todo: flashing the window/panel tab
}

void flist_messenger::messageMany(FSession *session, QList<QString> &channels, QList<QString> &characters, bool system, QString message, MessageType messagetype) {
    QList<QString> panelnames;
    QString charactername;
    QString channelname;
    QString panelnamemidfix = "|||" + session->getSessionID() + "|||";
    if (system) {
        // todo: session based consoles?
        panelnames.append("FCHATSYSTEMCONSOLE");
    }
    foreach (charactername, characters) {
        panelnames.append("PM" + panelnamemidfix + charactername);
    }
    foreach (channelname, channels) {
        if (channelname.startsWith("ADH-")) {
            panelnames.append("ADH" + panelnamemidfix + channelname);
        } else {
            panelnames.append("CHAN" + panelnamemidfix + channelname);
        }
    }
    if (system) {
        if (!panelnames.contains(currentPanel->getPanelName())) {
            panelnames.append(currentPanel->getPanelName());
        }
    }
    messageMany(panelnames, message, messagetype);
}

void flist_messenger::messageAll(FSession *session, QString message, MessageType messagetype) {
    QList<QString> panelnames;
    FChannelPanel *channelpanel;
    // todo: session based consoles?
    panelnames.append("FCHATSYSTEMCONSOLE");
    QString match = "|||" + session->getSessionID() + "|||";
    // Extract all panels that are relevant to this session.
    foreach (channelpanel, channelList) {
        QString panelname = channelpanel->getPanelName();
        if (panelname.contains(match)) {
            panelnames.append(panelname);
        }
    }
    messageMany(panelnames, message, messagetype);
}

void flist_messenger::messageChannel(FSession *session, QString channelname, QString message, MessageType messagetype, bool console, bool notify) {
    QList<QString> panelnames;
    panelnames.append(PANELNAME(channelname, session->getSessionID()));
    if (console) {
        panelnames.append("FCHATSYSTEMCONSOLE");
    }
    if (notify) {
        QString panelname = currentPanel->getPanelName();
        if (!panelnames.contains(panelname)) {
            panelnames.append(panelname);
        }
    }
    messageMany(panelnames, message, messagetype);
}

void flist_messenger::messageCharacter(FSession *session, QString charactername, QString message, MessageType messagetype) {
    QList<QString> panelnames;
    panelnames.append("PM|||" + session->getSessionID() + "|||" + charactername);
    messageMany(panelnames, message, messagetype);
}

void flist_messenger::messageSystem(FSession *session, QString message, MessageType messagetype) {
    (void)session; // todo: session based consoles?
    QList<QString> panelnames;
    panelnames.append("FCHATSYSTEMCONSOLE");
    if (currentPanel && !panelnames.contains(currentPanel->getPanelName())) {
        panelnames.append(currentPanel->getPanelName());
    }
    messageMany(panelnames, message, messagetype);
}

void flist_messenger::updateKnownChannelList(FSession *session) {
    cl_data->updateChannels(session->knownchannellist.begin(), session->knownchannellist.end());
}

void flist_messenger::updateKnownOpenRoomList(FSession *session) {
    cl_data->updateRooms(session->knownopenroomlist.begin(), session->knownopenroomlist.end());
}

void flist_messenger::createPublicChannel(QString name) {
    account->getSession(charName)->createPublicChannel(name);
}

void flist_messenger::createPrivateChannel(QString name) {
    account->getSession(charName)->createPrivateChannel(name);
}
