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

#ifndef FLIST_PARSER_H
#define FLIST_PARSER_H

#include <QRegExp>
#include <QString>
#include <QSet>
#include <QStack>
#include <QMap>
#include <QList>

/**
 * This code is based off of the BBCode parser written in java by Telroth
 * Converted to C++ by Zwagoth.
 **/

class BBCodeParser
{

public:
    BBCodeParser();
    ~BBCodeParser() {}

    class Tag
    {

    public:
        QString name, param;
        QString content;
        Tag()
        {
            content.reserve ( 1024 );
        }
    };

    class BBCodeTag
    {

    private:
        bool blacklist;
        QSet<QString> allowedTags;

    public:
        BBCodeTag ( bool blacklist ) :
                blacklist ( blacklist )
        {
        }

        bool allows ( QString& tag );
        void setTagList ( QSet<QString>& tagList );
        virtual QString parse ( QString& param, QString& content ) = 0;
    };

    class WrapperBBCodeTag : public BBCodeTag
    {

    private:
        QString pre, post;

    public:
        WrapperBBCodeTag ( QString pre, QString post, bool blacklist ) :
                pre ( pre ),
                post ( post ),
                BBCodeTag ( blacklist )
        {
        }

        QString parse ( QString& param, QString& content )
        {
            return pre + content + post;
        }
    };

    class BBCodeTagColor : public BBCodeTag
    {

    public:
        BBCodeTagColor ( bool blacklist ) :
                BBCodeTag ( blacklist )
        {
        }

        QString parse ( QString& param, QString& content );
    };

    class BBCodeTagURL : public BBCodeTag
    {

    public:
        BBCodeTagURL ( bool blacklist ) :
                BBCodeTag ( blacklist )
        {
        }

        QString parse ( QString& param, QString& content );
    };

    class BBCodeTagNoparse : public BBCodeTag
    {

    public:
        BBCodeTagNoparse ( bool blacklist ) :
                BBCodeTag ( blacklist )
        {
        }

        QString parse ( QString& param, QString& content );
    };

	class BBCodeTagChannel : public BBCodeTag
    {

    public:
        BBCodeTagChannel ( bool blacklist ) :
                BBCodeTag ( blacklist )
        {
        }

        QString parse ( QString& param, QString& content );
    };

	class BBCodeTagSession : public BBCodeTag
    {

    public:
        BBCodeTagSession ( bool blacklist ) :
                BBCodeTag ( blacklist )
        {
        }

        QString parse ( QString& param, QString& content );
    };

	class BBCodeTagIcon : public BBCodeTag
    {

    public:
        BBCodeTagIcon ( bool blacklist ) :
                BBCodeTag ( blacklist )
        {
        }

        QString parse ( QString& param, QString& content );
    };

	class BBCodeTagUser : public BBCodeTag
    {

    public:
        BBCodeTagUser ( bool blacklist ) :
                BBCodeTag ( blacklist )
        {
        }

        QString parse ( QString& param, QString& content );
    };

    void addTag(QString tag, BBCodeTag* code);
    void addSmiley(QString tag, QString code);
    QString parse(QString& input);
    QStack<Tag*>* parse(QString& input, int start, int end);
    void removeTag(QString tag);
	void removeSmiley(QString smiley);

    static BBCodeTag*			BBCODE_BOLD;
    static BBCodeTag*			BBCODE_ITALIC;
    static BBCodeTag*			BBCODE_UNDERLINE;
    static BBCodeTag*			BBCODE_SUPERSCRIPT;
    static BBCodeTag*			BBCODE_SUBSCRIPT;
    static BBCodeTag*			BBCODE_STRIKETHROUGH;
    static BBCodeTag* 			BBCODE_COLOR;
    static BBCodeTag*			BBCODE_URL;
	static BBCodeTag*			BBCODE_CHANNEL;
	static BBCodeTag*			BBCODE_SESSION;
	static BBCodeTag*			BBCODE_ICON;
	static BBCodeTag*			BBCODE_USER;
    static BBCodeTag*			BBCODE_NOPARSE;
private:
    Tag* getTag(QString& name, QString& param);
    void escapeAppend(QString& input, int start, int end, QString& output);
    QMap<QString, BBCodeTag*> 	tags;
    QMap<QString, QString> 		smilies;
    QList<Tag*>					tagpool;
};

#endif // FLIST_PARSER_H
