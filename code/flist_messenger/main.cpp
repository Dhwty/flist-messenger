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

#include <QApplication>
#include <QFile>
#include "flist_messenger.h"
#include "flist_global.h"

int main(int argc, char** argv)
{
	bool d = (argc > 1 && strcmp(argv[1], "-d") == 0) ? true : false;
	QApplication *app = new QApplication(argc, argv);
	app->setOrganizationName("F-list.net");
	app->setOrganizationDomain("www.f-list.net");
	app->setApplicationName("F-list Messenger");
	globalInit();
	QFile stylefile("default.qss");
	stylefile.open(QFile::ReadOnly);
	QString stylesheet = QLatin1String(stylefile.readAll());
	app->setStyleSheet(stylesheet);
	flist_messenger::init();
	flist_messenger *fmessenger = new flist_messenger(d);
	fmessenger->show();
	return app->exec();
	//todo: globalQuit();
}
