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

#include "flist_avatar.h"
#include <QNetworkReply>
#include <QIcon>
#include <iostream>

FAvatar::FAvatar ( QObject *parent ) :
		QObject ( parent )
{
	manager.setParent ( this );
}

QPixmap FAvatar::getAvatar ( QString userName )
{
	// WARNING:  This function works only if the avatar has already been
	// cached by another function or a previous call.
	QString name = userName.toLower();

	if ( imageHash.contains ( name ) )
	{
		return imageHash[name];
	}

	QUrl targetUrl ( "https://static.f-list.net/images/avatar/" + name.toLower() + ".png" );
	connect ( &manager, SIGNAL ( finished ( QNetworkReply* ) ), SLOT ( buttonAvatarReady ( QNetworkReply* ) ) );
	manager.get ( QNetworkRequest ( targetUrl ) );
	return QPixmap();
}

void FAvatar::applyAvatarToButton ( QAbstractButton *button, QString name )
{
	name = name.toLower();
	std::cout << "Attempting fetch of icon for " + name.toStdString() + ".\n";

	if ( imageHash.contains ( name ) )
	{
		button->setIcon ( QIcon ( imageHash[name] ) );
		return;
	}

	QUrl targetUrl ( "https://static.f-list.net/images/avatar/" + name.toLower() + ".png" );

	buttonDownloads.insert ( name, button );

	std::cout << "Beginning download for " + name.toStdString() + ".\n";
	connect ( &manager, SIGNAL ( finished ( QNetworkReply* ) ), SLOT ( buttonAvatarReady ( QNetworkReply* ) ) );
	manager.get ( QNetworkRequest ( targetUrl ) );
}

void FAvatar::buttonAvatarReady ( QNetworkReply* reply )
{
	reply->deleteLater();
	QString name = reply->url().toString();
	name.chop ( 4 );	    // remove ".png" from the end.
	name = name.remove ( 0, 40 );  // remove start of url from the beginning.
	QList<QAbstractButton*> buttonList;

	if ( buttonDownloads.values ( name ).length() > 0 )
	{
		buttonList = buttonDownloads.values ( name );
		buttonDownloads.remove ( name );
	}
	else
	{
		std::cout << "Download with no button for " + name.toStdString() + ".\n";
		return;
	}

	std::cout << "Finished avatar download for " + name.toStdString() + "\n";

	if ( !reply->error() )
	{
		QImage image;

		if ( image.loadFromData ( reply->readAll() ) )
		{
			QPixmap pixmap = QPixmap::fromImage ( image );

			if ( !pixmap.isNull() )
			{
				imageHash[name] = pixmap;
				QAbstractButton* button;
				foreach ( button, buttonList )
				{
					button->setIcon ( QIcon ( pixmap ) );
				}
			}
		}
	}
}
