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

#include "flist_sound.h"
#include <iostream>
#include <QFile>
#include "flist_global.h"
#include "flist_settings.h"

FSound::FSound() {}

void FSound::play(soundName sound) {
    if (sound == SOUND_NONE) {
        return;
    }
    QString soundFile = soundToString(sound);
    QSoundEffect effect;

    std::cout << "Playing sound " + soundFile.toStdString() + "\n";
    effect.setSource(QUrl(soundFile));
    effect.play();

    // shouldn't be necessary, but... - GH
    effect.deleteLater();
}

QString FSound::soundToString(soundName sound) {
    QString soundKey, soundFile, soundPath;

    switch (sound) {
        case SOUND_ATTENTION:
            soundKey = "attention";
            soundFile = "attention.wav";
            break;

        case SOUND_CHAT:
            soundKey = "chat";
            soundFile = "chat.wav";
            break;

        case SOUND_LOGIN:
            soundKey = "login";
            soundFile = "login.wav";
            break;

        case SOUND_MODALERT:
            soundKey = "modalert";
            soundFile = "modalert.wav";
            break;

        case SOUND_NEWNOTE:
            soundKey = "newnote";
            soundFile = "newnote.wav";
            break;

        case SOUND_FRIENDUPDATE:
            soundKey = "friendupdate";
            soundFile = "friendupdate.wav";
            break;

        case SOUND_SYSTEM:
            soundFile = "system.wav";
            break;

        default:
            std::cout << "Invalid sound.\n";
            soundKey = "INVALID";
            soundFile = "INVALID";
            break;
    }

    soundPath = settings->getString(QString("Global/Sounds/%1").arg(soundKey));
    if (soundPath.isEmpty() || !QFile::exists(soundPath)) {
        soundPath = "./sounds/" + soundFile;
    }
    if (!QFile::exists(soundPath)) {
        soundPath = ":/sounds/" + soundFile;
    }

    return soundPath;
}
