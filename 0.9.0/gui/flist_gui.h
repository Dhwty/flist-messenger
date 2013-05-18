#ifndef FLIST_GUI_H
#define FLIST_GUI_H
#include <QMainWindow>
#include <QtWidgets/QDesktopWidget>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QAction>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QListWidgetItem>
#include <QtWidgets/QDialog>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QCheckBox>
#include <QTextCursor>
#include <QtWebKitWidgets/QWebView>
#include <QIcon>
#include <QSystemTrayIcon>
#include <QTextBrowser>
#include "../flist_messenger.h"
#include "flist_avatar.h"

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

class flist_messenger;
class FSound;
class FAvatar;
class FCharacter;
class FChannel;
class ChannelListItem;
class FSettings;

class FGui : public QMainWindow
{
    Q_OBJECT
public:
    enum MessageBoxType {
        MSGTYPE_ERROR,
        MSGTYPE_SUCCESS,
        MSGTYPE_FAILURE,
        MSGTYPE_MESSAGE,
        MSGTYPE_MAX
    };

    FGui(flist_messenger *parent = 0, FSettings* settings = 0);
    void flashApp();
    void setupLoginBox(QList<QString>& characterList, QString& defaultCharacter);
    void messageBox(QString& message, MessageBoxType type = MSGTYPE_MESSAGE);
    void connectError(QString& error);
    void appendChatLine(QString& line); // Appends a chat line.
    QPushButton* addPanelButton(QString& name, QString& tooltip);
    void refreshUserlist(FChannel* currentPanel);
    void fillFriendList(QList<QString>& friends, QList<QString>& ignores);
    void fillChannelList(QList<ChannelListItem*>& channels);
    void fillProomList(QList<ChannelListItem*>& prooms);
    void updateButtonColor(QString& channel, QString& color);
    void showChannelButton(QString& name, bool visible = true);
    void setStatusButtonIcon(QString& status);
    void clearInput();
    void clearOutput();
    void ci_clearKinks();
    void ci_clearProfile();
    void ci_addToKinks(QString& kink);
    void ci_addToProfile(QString& tag);
signals:
    void consoleButtonReady();
    void friendListRequested();
    void channelListRequested();
    void characterInfoRequested(QString&);
private:
    void createTrayIcon();
    void destroyMenu();
    void destroyChanMenu();
    void setupRealUI();
    void setupConnectBox();
    void clearConnectBox();
    void clearLoginBox();
    void doSetupLoginBox(QList<QString>& characterList, QString& defaultCharacter);
    void addToFriendsList ( QListWidgetItem* lwi );
    void addToIgnoreList ( QListWidgetItem* lwi );
    void addToProomsDialogList(ChannelListItem* cli);
    void addToChannelsDialogList(ChannelListItem* cli);

    void setupFriendsDialog();
    void setupChannelsUI();
    void setupCharacterInfoUI();
    void setupTimeoutDialog();
    void setupSettingsDialog();
    void setupMakeRoomUI();
    void setupSetStatusUI();
    void setupReportDialog();
    void setupHelpDialog();
    bool setupChannelSettingsDialog();

    flist_messenger* parent;

    QPushButton* pushButton;
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
    QSpacerItem* activePanelsSpacer;
    FCharacter* ul_recent;
    FChannel* tb_recent;
    QMenu* recentCharMenu;
    QMenu* recentChannelMenu;
    QSystemTrayIcon* trayIcon;
    QMenu* trayIconMenu;

    FSound* soundPlayer;
    FAvatar avatarFetcher;
    UseReturn* returnFilter;

    QHash<QString, QPushButton*> activePanelButtons;

    bool notificationsAreaMessageShown;
    
public slots:
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void insertLineBreak();
    void aboutApp();
    void quitApp();
    void btnConnectClicked();
    void btnLoginClicked();
    void enterPressed();
    void tabSwitched(FChannel* from, FChannel* to);
    void closeEvent(QCloseEvent *);
    void userListContextMenuRequested (const QPoint& point);
    void channelButtonMenuRequested();
    void displayChannelContextMenu(FChannel *ch);
    void displayCharacterContextMenu(FCharacter* ch);

    void setupAddIgnoreDialog();
    void addIgnoreDialogRequested();
    void fr_friendsContextMenuRequested(const QPoint& point);

    void tb_channelRightClicked(const QPoint& point);
private slots:
    void friendsDialogRequested();
    void channelsDialogRequested();
    void characterInfoDialogRequested();
    void timeoutDialogRequested(QString& name);
    void settingsDialogRequested();
    void makeRoomDialogRequested();
    void setStatusDialogRequested();
    void reportDialogRequested();
    void helpDialogRequested();
    void channelSettingsDialogRequested();

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
    void fr_btnFriendsPMClicked();
    void fr_btnIgnoreRemoveClicked();
    void fr_btnCloseClicked();
    void fr_btnIgnoreAddClicked();
    void ai_btnSubmitClicked();
    void ai_btnCancelClicked();
    void cd_btnJoinClicked();
    void cd_btnProomsJoinClicked();
    void cd_btnCancelClicked();
    void ci_btnCloseClicked();
    void to_btnSubmitClicked();
    void to_btnCancelClicked();
    void mr_btnSubmitClicked(); // vanaf hier
    void mr_btnCancelClicked();
    void ss_btnSubmitClicked();
    void ss_btnCancelClicked();
    void re_btnSubmitClicked();
    void re_btnCancelClicked();
    void se_btnSubmitClicked();
    void se_btnCancelClicked();
    void tb_closeClicked();
    void tb_settingsClicked();
    void cs_chbEditDescriptionToggled(bool state);
    void cs_btnCancelClicked();
    void cs_btnSaveClicked();
private:
    // All dialogs here.
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

    QDialog* channelsDialog;// cd stands for channels dialog
    QTabWidget* cd_twOverview;
    QVBoxLayout* cd_vblOverview;
    QVBoxLayout* cd_vblChannels;
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

    QDialog* characterInfoDialog; // ci stands for character info
    QLabel* ci_lblName;
    QLabel* ci_lblStatusMessage;
    QTextEdit* ci_teKinks;
    QTextEdit* ci_teProfile;

    QDialog* timeoutDialog; // to stands for timeout
    QLineEdit* to_leWho;
    QLineEdit* to_leLength;
    QLineEdit* to_leReason;

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
    FChannel* cs_chanCurrent;

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
};

#endif // FLIST_GUI_H
