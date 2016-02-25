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

#ifndef flist_character_H
#define flist_character_H

#include <QString>
#include <QHash>
#include <QIcon>
#include <time.h>

class FCharacter
{

public:
	enum characterStatus
	{
		STATUS_ONLINE,
		STATUS_LOOKING,
		STATUS_BUSY,
		STATUS_DND,
		STATUS_CROWN,
		STATUS_AWAY,
		STATUS_MAX
	};

	enum characterGender
	{
		GENDER_NONE,
		GENDER_MALE,
		GENDER_FEMALE,
		GENDER_TRANSGENDER,
		GENDER_SHEMALE,
		GENDER_HERM,
		GENDER_MALEHERM,
		GENDER_CUNTBOY,
		GENDER_OFFLINE_UNKNOWN,
		GENDER_MAX
	};
	static QIcon*		statusIcons[STATUS_MAX];
	static QString		statusStrings[STATUS_MAX];
	static QString		genderStrings[GENDER_MAX];
	static QColor		genderColors[GENDER_MAX];
	FCharacter();
	FCharacter ( QString& name, bool friended );
	~FCharacter() {}

	void setName ( QString& name );
	QString& name()
	{
		return charName;
	}

	void setStatus ( QString& status );
	characterStatus status()
	{
		return charStatus;
	}

	QString& statusString();
	void setStatusMsg ( QString& status );
	QIcon* statusIcon();
	QString& statusMsg()
	{
		return statusMessage;
	}

	void setGender ( QString& gender );
	characterGender gender()
	{
		return charGender;
	}

	QString& genderString();
	void updateActivityTimer();
	quint32 activityTimer()
	{
		return lastActivity;
	}

	QString activityString();
	bool isChatOp()
	{
		return chatOp;
	}
	void setIsFriend(bool b) { isFriend = b; }
	bool getFriend()
	{
		return isFriend;
	}

	void setIsChatOp ( const bool op );
	QColor& genderColor();
	QString PMTitle();

	QString getUrl() {return "https://www.f-list.net/c/" + charName + "/";} //todo: HTTP request character encoding. //todo: Get server address from FServer?

	void clearCustomKinkData() {customkinkdatakeys.clear(); customkinkdata.clear();}
	void addCustomKinkData(QString key, QString value) {customkinkdatakeys.removeAll(key); customkinkdatakeys.append(key); customkinkdata[key] = value;}
	QStringList &getCustomKinkDataKeys() {return customkinkdatakeys;}
	QHash<QString, QString> &getCustomKinkData() {return customkinkdata;}

	void clearProfileData() {profiledatakeys.clear(); profiledata.clear();}
	void addProfileData(QString key, QString value) {profiledatakeys.removeAll(key); profiledatakeys.append(key); profiledata[key] = value;}
	QStringList &getProfileDataKeys() {return profiledatakeys;}
	QHash<QString, QString> &getProfileData() {return profiledata;}
	

	static void initClass();

private:
	QString				charName;
	QString				statusMessage;
	characterStatus		charStatus;
	characterGender		charGender;
	quint32				lastActivity;
	bool				chatOp;
	bool				isFriend;

	//todo: The following data should really go in 'FCharacterProfile', but placed here to expediate the transfer of flist_messenger::processCommand()'s functionality into 'FSession'. 'FCharacterProfile' is expected to use the JSON HTTP API to retrieve data and will be less session dependant.
	QStringList customkinkdatakeys; //<Custom kink keys in the order they were reported by the server
	QHash<QString, QString> customkinkdata; //Custom kink data.
	QStringList profiledatakeys; //<Profile keys in the order they were reported by the server
	QHash<QString, QString> profiledata; //Profile data.
};

#endif //flist_character_H
