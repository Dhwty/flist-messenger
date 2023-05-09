#ifndef FLIST_ATTENTIONSETTINGSWIDGET_H
#define FLIST_ATTENTIONSETTINGSWIDGET_H

#include <QWidget>
#include <QString>
#include <QStringList>
#include "flist_channel.h"

class QComboBox;
class QHBoxLayout;
class QCheckBox;
class QLineEdit;

class FAttentionSettingsWidget : public QWidget {
        Q_OBJECT
    public:
        explicit FAttentionSettingsWidget(QString channelname, QString channeltitle = QString(), FChannel::ChannelType channeltype = FChannel::CHANTYPE_CONSOLE,
                                          QWidget *parent = 0);

        void loadSettings();
        void saveSettings();

    private:
        QHBoxLayout *buildHBox(QString labeltext, QWidget *widget);
        QComboBox *buildPulldown(QStringList list);
        void setPullDown(QComboBox *pulldown, QString field, int dflt);
        void savePullDown(QComboBox *pulldown, QString field);
    signals:

    public slots:

    private:
        QString escapedname;
        QString settingprefix;
        QString channelname;
        QString channeltitle;
        FChannel::ChannelType channeltype;

        QCheckBox *keyword_exclusive_checkbox;
        QLineEdit *keyword_text;

        QComboBox *message_channel_ding_pulldown;
        QComboBox *message_rpad_ding_pulldown;
        QComboBox *message_character_ding_pulldown;
        QComboBox *message_keyword_ding_pulldown;
        QComboBox *message_channel_flash_pulldown;
        QComboBox *message_rpad_flash_pulldown;
        QComboBox *message_character_flash_pulldown;
        QComboBox *message_keyword_flash_pulldown;
};

#endif // FLIST_ATTENTIONSETTINGSWIDGET_H
