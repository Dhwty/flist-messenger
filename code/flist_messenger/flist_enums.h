#ifndef FLIST_ENUMS_H
#define FLIST_ENUMS_H

#include <QString>
#include <QHash>

class EnumLookup {
public:
	EnumLookup(QString enumlist, QString dflt = QString());
	int keyToValue(QString key) {return valuelookup.value(key, defaultvalue);}
	int keyToValue(QString key, int defaultvalue) {return valuelookup.value(key, defaultvalue);}
	QString valueToKey(int val) {return keylookup.value(val, QString());}
private:
	QString defaultkey;
	int defaultvalue;
	QHash<QString, int>valuelookup;
	QHash<int, QString>keylookup;
};

enum MessageType {
	MESSAGE_TYPE_LOGIN,
	MESSAGE_TYPE_ONLINE,
	MESSAGE_TYPE_OFFLINE,
	MESSAGE_TYPE_STATUS,
	MESSAGE_TYPE_CHANNEL_DESCRIPTION,
	MESSAGE_TYPE_CHANNEL_MODE,
	MESSAGE_TYPE_JOIN,
	MESSAGE_TYPE_LEAVE,
	MESSAGE_TYPE_CHANNEL_INVITE,
	MESSAGE_TYPE_KICK,
	MESSAGE_TYPE_KICKBAN,
	MESSAGE_TYPE_IGNORE_UPDATE,
	MESSAGE_TYPE_SYSTEM,
	MESSAGE_TYPE_REPORT,
	MESSAGE_TYPE_ERROR,
	MESSAGE_TYPE_BROADCAST,
	MESSAGE_TYPE_FEEDBACK,
	MESSAGE_TYPE_RPAD,
	MESSAGE_TYPE_CHAT,
	MESSAGE_TYPE_ROLL,
	MESSAGE_TYPE_NOTE,
	MESSAGE_TYPE_BOOKMARK,
	MESSAGE_TYPE_FRIEND,
};

enum TypingStatus {
	TYPING_STATUS_CLEAR,
	TYPING_STATUS_TYPING,
	TYPING_STATUS_PAUSED,
};

enum ChannelMode {
	//TODO: Use the unknown value for stuff
	CHANNEL_MODE_UNKNOWN,
	CHANNEL_MODE_CHAT,
	CHANNEL_MODE_ADS,
	CHANNEL_MODE_BOTH
};
#define CHANNEL_MODE_ENUM "unknown, chat, ads, both"
#define CHANNEL_MODE_DEFAULT "unknown"
extern EnumLookup ChannelModeEnum;

enum AttentionMode {
	ATTENTION_DEFAULT,
	ATTENTION_NEVER,
	ATTENTION_IFNOTFOCUSED,
	ATTENTION_ALWAYS
};
#define ATTENTION_MODE_ENUM "default, never, ifnotfocused, always"
extern EnumLookup AttentionModeEnum;

enum BoolTristate {
	BOOL_FALSE,
	BOOL_TRUE,
	BOOL_DEFAULT
};
#define BOOL_TRISTATE_ENUM "false, true, default"
#define BOOL_TRISTATE_DEFAULT
extern EnumLookup BoolTristateEnum;

#endif // FLIST_ENUMS_H
