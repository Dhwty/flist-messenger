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

#include "flist_character.h"
#include <QtCore/QSettings>

QIcon*	FCharacter::statusIcons[FCharacter::STATUS_MAX];
QString FCharacter::statusStrings[FCharacter::STATUS_MAX];
QString FCharacter::genderStrings[FCharacter::GENDER_MAX];
QColor  FCharacter::genderColors[FCharacter::GENDER_MAX];

void FCharacter::initClass()
{
	statusIcons[ (quint32)STATUS_ONLINE] = new QIcon ( ":/images/status-default.png" ); //STATUS_ONLINE
	statusIcons[ (quint32)STATUS_LOOKING] = new QIcon ( ":/images/status.png" ); 		//STATUS_LOOKING
	statusIcons[ (quint32)STATUS_BUSY] = new QIcon ( ":/images/status-away.png" );		//STATUS_BUSY
	statusIcons[ (quint32)STATUS_DND] = new QIcon ( ":/images/status-busy.png" );		//STATUS_DND
	statusIcons[ (quint32)STATUS_CROWN] = new QIcon ( ":/images/crown.png" );			//STATUS_CROWN
	statusIcons[ (quint32)STATUS_AWAY] = new QIcon( ":/images/status-blue" );			//STATUS_AWAY

	statusStrings[ (quint32)STATUS_ONLINE] = "Online";
	statusStrings[ (quint32)STATUS_LOOKING] = "Looking";
	statusStrings[ (quint32)STATUS_BUSY] = "Busy";
	statusStrings[ (quint32)STATUS_DND] = "Do Not Disturb";
	statusStrings[ (quint32)STATUS_CROWN] = "Crowned";
	statusStrings[ (quint32)STATUS_AWAY] = "Away";

	genderStrings[ (quint32)GENDER_NONE] = "None";
	genderStrings[ (quint32)GENDER_MALE] = "Male";
	genderStrings[ (quint32)GENDER_FEMALE] = "Female";
	genderStrings[ (quint32)GENDER_TRANSGENDER] = "Transgender";
	genderStrings[ (quint32)GENDER_SHEMALE] = "Shemale";
	genderStrings[ (quint32)GENDER_HERM] = "Hermaphrodite";
	genderStrings[ (quint32)GENDER_MALEHERM] = "Male Hermaphrodite";
	genderStrings[ (quint32)GENDER_CUNTBOY] = "Cunt-Boy";
	genderStrings[ (quint32)GENDER_OFFLINE_UNKNOWN] = "Offline/Unknown";

	genderColors[ (quint32)GENDER_NONE] = QColor ( 255, 255, 255 );
	genderColors[ (quint32)GENDER_MALE] = QColor ( 102, 247, 255 );
	genderColors[ (quint32)GENDER_FEMALE] = QColor ( 255, 102, 184 );
	genderColors[ (quint32)GENDER_TRANSGENDER] = QColor ( 102, 255, 166 );
	genderColors[ (quint32)GENDER_SHEMALE] = QColor ( 224, 255, 102 );
	genderColors[ (quint32)GENDER_HERM] = QColor ( 212, 102, 255 );
	genderColors[ (quint32)GENDER_MALEHERM] = QColor ( 255, 189, 102 );
	genderColors[ (quint32)GENDER_CUNTBOY] = QColor ( 115, 102, 255 );
	genderColors[ (quint32)GENDER_OFFLINE_UNKNOWN] = QColor ( 127, 127, 127 );

	QSettings colset("./colors.ini", QSettings::IniFormat);
	if(colset.status() != QSettings::NoError)
	{
		return;
	}
	QStringList colstr;
	colstr = colset.value("gender/none").toStringList();
	if(colstr.size() >= 2) {
		genderColors[ (quint32)GENDER_NONE] = QColor( colstr[0].toInt(), colstr[1].toInt(), colstr[2].toInt() );
	}
	colstr = colset.value("gender/male").toStringList();
	if(colstr.size() >= 2) {
		genderColors[ (quint32)GENDER_MALE] = QColor( colstr[0].toInt(), colstr[1].toInt(), colstr[2].toInt() );
	}
	colstr = colset.value("gender/female").toStringList();
	if(colstr.size() >= 2) {
		genderColors[ (quint32)GENDER_FEMALE] = QColor( colstr[0].toInt(), colstr[1].toInt(), colstr[2].toInt() );
	}
	colstr = colset.value("gender/transgender").toStringList();
	if(colstr.size() >= 2) {
		genderColors[ (quint32)GENDER_TRANSGENDER] = QColor( colstr[0].toInt(), colstr[1].toInt(), colstr[2].toInt() );
	}
	colstr = colset.value("gender/shemale").toStringList();
	if(colstr.size() >= 2) {
		genderColors[ (quint32)GENDER_SHEMALE] = QColor( colstr[0].toInt(), colstr[1].toInt(), colstr[2].toInt() );
	}
	colstr = colset.value("gender/herm").toStringList();
	if(colstr.size() >= 2) {
		genderColors[ (quint32)GENDER_HERM] = QColor( colstr[0].toInt(), colstr[1].toInt(), colstr[2].toInt() );
	}
	colstr = colset.value("gender/maleherm").toStringList();
	if(colstr.size() >= 2) {
		genderColors[ (quint32)GENDER_MALEHERM] = QColor( colstr[0].toInt(), colstr[1].toInt(), colstr[2].toInt() );
	}
	colstr = colset.value("gender/cuntboy").toStringList();
	if(colstr.size() >= 2) {
		genderColors[ (quint32)GENDER_CUNTBOY] = QColor( colstr[0].toInt(), colstr[1].toInt(), colstr[2].toInt() );
	}
	colstr = colset.value("gender/offlineunknown").toStringList();
	if(colstr.size() >= 2) {
		genderColors[ (quint32)GENDER_OFFLINE_UNKNOWN] = QColor( colstr[0].toInt(), colstr[1].toInt(), colstr[2].toInt() );
	}
}

FCharacter::FCharacter()
{
	updateActivityTimer();
	chatOp = false;
	charStatus = FCharacter::STATUS_ONLINE;
	charGender = FCharacter::GENDER_NONE;
	isFriend = false;
}

FCharacter::FCharacter ( QString& name, bool friended )
{
	charName = name;
	updateActivityTimer();
	chatOp = false;
	isFriend = friended;
	charStatus = FCharacter::STATUS_ONLINE;
	charGender = FCharacter::GENDER_NONE;
}

void FCharacter::setName ( QString& name )
{
	charName = name;
}

void FCharacter::updateActivityTimer()
{
	lastActivity = time ( 0 );
}

QString FCharacter::activityString()
{
	return "";
}

QIcon* FCharacter::statusIcon()
{
	if ( charStatus >= STATUS_MAX )
		return 0;

	return statusIcons[ ( quint32 ) charStatus];
}

void FCharacter::setGender ( QString& gender )
{
	QString lgender = gender.toLower();

	if ( lgender == "male" )
		charGender = GENDER_MALE;
	else if ( lgender == "female" )
		charGender = GENDER_FEMALE;
	else if ( lgender == "none" )
		charGender = GENDER_NONE;
	else if ( lgender == "herm" )
		charGender = GENDER_HERM;
	else if ( lgender == "transgender" )
		charGender = GENDER_TRANSGENDER;
	else if ( lgender == "shemale" )
		charGender = GENDER_SHEMALE;
	else if ( lgender == "cunt-boy" )
		charGender = GENDER_CUNTBOY;
	else if ( lgender == "male-herm" )
		charGender = GENDER_MALEHERM;
	else
		charGender = GENDER_NONE;
}

QString& FCharacter::genderString()
{
	if ( charGender >= GENDER_MAX )
		return genderStrings[ ( quint32 ) GENDER_NONE];

	return genderStrings[ ( quint32 ) charGender];
}

void FCharacter::setStatus ( QString& status )
{
	QString lstatus = status.toLower();

	if ( lstatus == "online" )
	{
		charStatus = STATUS_ONLINE;
	}
	else if ( lstatus == "looking" )
	{
		charStatus = STATUS_LOOKING;
	}
	else if ( lstatus == "busy" )
	{
		charStatus = STATUS_BUSY;
	}
	else if ( lstatus == "dnd" )
	{
		charStatus = STATUS_DND;
	}
	else if ( lstatus == "crown" )
	{
		charStatus = STATUS_CROWN;
	}
	else if ( lstatus == "away" )
	{
		charStatus = STATUS_AWAY;
	}
	else
	{
		charStatus = STATUS_ONLINE;
	}
}

QString& FCharacter::statusString()
{
	if ( charStatus >= STATUS_MAX )
		return statusStrings[ ( quint32 ) STATUS_ONLINE];

	return statusStrings[ ( quint32 ) charStatus];
}

void FCharacter::setStatusMsg ( QString& status )
{
	statusMessage = status;
}

void FCharacter::setIsChatOp ( const bool op )
{
	chatOp = op;
}

QColor& FCharacter::genderColor()
{
	if ( charGender >= GENDER_MAX )
		return genderColors[ ( quint32 ) GENDER_NONE];

	return genderColors[ ( quint32 ) charGender];
}

QString FCharacter::PMTitle()
{
	QString title = "Private chat with " + charName + " (" + statusString();

	if ( !statusMessage.length() )
		title += ")";
	else
		title += ": " + statusMessage + ")";

	return title;
}
