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

#ifndef FLIST_AVATAR_H
#define FLIST_AVATAR_H

#include <QtNetwork/QNetworkReply>
#include <QObject>
#include <QPixmap>
#include <QHash>
#include <QAbstractButton>
#include <QUrl>
#include <QMultiMap>
#include <QtNetwork/QNetworkAccessManager>

class FAvatar : public QObject
{
	Q_OBJECT

public:
	explicit FAvatar ( QObject *parent = 0 );
	QPixmap getAvatar ( QString userName );
	void applyAvatarToButton ( QAbstractButton *button, QString name );

signals:

public slots:
	void buttonAvatarReady ( QNetworkReply* );

private:
	QMultiMap<QString, QAbstractButton*> buttonDownloads;
	QHash<QString, QPixmap> imageHash;
	QNetworkAccessManager manager;
};

#endif // FLIST_AVATAR_H
