
#include "flist_enums.h"

#include <QStringList>

EnumLookup AttentionModeEnum(QString(ATTENTION_MODE_ENUM));
EnumLookup BoolTristate(QString(BOOL_TRISTATE_ENUM), QString(BOOL_TRISTATE_DEFAULT));
EnumLookup ChannelModeEnum(QString(CHANNEL_MODE_ENUM), QString(CHANNEL_MODE_DEFAULT));


EnumLookup::EnumLookup(QString enumlist, QString dflt)
{
	QStringList keys = enumlist.split(",");
	valuelookup.reserve(keys.size());
	keylookup.reserve(keys.size());
	for(int i = 0; i < keys.size(); i++) {
		QString s = keys[i].trimmed();
		valuelookup[s] = i;
		keylookup[i] = s;
	}
	if(dflt.isEmpty()) {
		defaultvalue = 0;
		defaultkey = keylookup[defaultvalue];
	} else {
		defaultkey = dflt;
		defaultvalue = valuelookup[defaultkey];
	}
}
