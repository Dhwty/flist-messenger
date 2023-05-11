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

#include "flist_parser.h"
#include <QUrl>

/**
 * This code is based off of the BBCode parser written in java by Telroth
 * Converted to C++ by Kira.
 **/

BBCodeParser::BBCodeTag* BBCodeParser::BBCODE_BOLD = new BBCodeParser::WrapperBBCodeTag("<b>", "</b>", true);
BBCodeParser::BBCodeTag* BBCodeParser::BBCODE_ITALIC = new BBCodeParser::WrapperBBCodeTag("<i>", "</i>", true);
BBCodeParser::BBCodeTag* BBCodeParser::BBCODE_UNDERLINE = new BBCodeParser::WrapperBBCodeTag("<u>", "</u>", true);
BBCodeParser::BBCodeTag* BBCodeParser::BBCODE_SUPERSCRIPT = new BBCodeParser::WrapperBBCodeTag("<sup>", "</sup>", true);
BBCodeParser::BBCodeTag* BBCodeParser::BBCODE_SUBSCRIPT = new BBCodeParser::WrapperBBCodeTag("<sub>", "</sub>", true);
BBCodeParser::BBCodeTag* BBCodeParser::BBCODE_STRIKETHROUGH = new BBCodeParser::WrapperBBCodeTag("<s>", "</s>", true);
BBCodeParser::BBCodeTag* BBCodeParser::BBCODE_COLOR = new BBCodeParser::BBCodeTagColor(true);
BBCodeParser::BBCodeTag* BBCodeParser::BBCODE_URL = new BBCodeParser::BBCodeTagURL(false);
BBCodeParser::BBCodeTag* BBCodeParser::BBCODE_CHANNEL = new BBCodeParser::BBCodeTagChannel(true);
BBCodeParser::BBCodeTag* BBCodeParser::BBCODE_SESSION = new BBCodeParser::BBCodeTagSession(true);
BBCodeParser::BBCodeTag* BBCodeParser::BBCODE_ICON = new BBCodeParser::BBCodeTagIcon(true);
BBCodeParser::BBCodeTag* BBCodeParser::BBCODE_EICON = new BBCodeParser::BBCodeTagEicon(true);
BBCodeParser::BBCodeTag* BBCodeParser::BBCODE_NOPARSE = new BBCodeParser::BBCodeTagNoparse(false);
BBCodeParser::BBCodeTag* BBCodeParser::BBCODE_USER = new BBCodeParser::BBCodeTagUser(false);

BBCodeParser::BBCodeParser() {
    QSet<QString> blacklist;
    blacklist.insert("sub");
    blacklist.insert("sup");
    BBCODE_SUPERSCRIPT->setTagList(blacklist);
    BBCODE_SUBSCRIPT->setTagList(blacklist);
    addTag("b", BBCODE_BOLD);
    addTag("i", BBCODE_ITALIC);
    addTag("u", BBCODE_UNDERLINE);
    addTag("sup", BBCODE_SUPERSCRIPT);
    addTag("sub", BBCODE_SUBSCRIPT);
    addTag("s", BBCODE_STRIKETHROUGH);
    addTag("color", BBCODE_COLOR);
    addTag("url", BBCODE_URL);
    addTag("channel", BBCODE_CHANNEL);
    addTag("session", BBCODE_SESSION);
    addTag("icon", BBCODE_ICON);
    addTag("eicon", BBCODE_EICON);
    addTag("user", BBCODE_USER);
    addTag("noparse", BBCODE_NOPARSE);
}

void BBCodeParser::BBCodeTag::setTagList(QSet<QString>& tagList) {
    allowedTags = tagList;
}

bool BBCodeParser::BBCodeTag::allows(QString& tag) {
    return allowedTags.contains(tag) != blacklist;
}

QString BBCodeParser::BBCodeTagURL::parse(QString& param, QString& content) {
    QString urlstring(param.isEmpty() ? content : param);
    QUrl url(urlstring);
    // Make sure the URL is valid, not a local file, and has a valid scheme.
    // todo: There are more valid URLs that shouldn't be accepted within the context of a chat message. (Schemes should probably limited to 'http:' and 'https:'.)
    if (url.isValid() && !url.isRelative() && !url.isLocalFile()) {
        param = url.toString();
        return QString("<a href=\"%1\"><img height=\"16\" width=\"16\" src=\":/images/chain.png\" border=\"0\" />%2</a><span class=\"DOMAIN\">[%3]</span>")
                .arg(param, content, url.host());
    } else {
        if (param.isEmpty()) {
            return QString("(BADURL)[%1]").arg(content);
        } else {
            return QString("(BADURL)%1[%2]").arg(content, param);
        }
    }
}

QString BBCodeParser::BBCodeTagColor::parse(QString& param, QString& content) {
    return "<span style=\"color: " + param + ";\">" + content + "</span>";
}

QString BBCodeParser::BBCodeTagNoparse::parse(QString& param, QString& content) {
    (void)param;
    return content;
}

QString BBCodeParser::BBCodeTagChannel::parse(QString& param, QString& content) {
    (void)param;
    static QRegularExpression bbTagChannel("[A-Za-z0-9 \\-/']+", QRegularExpression::CaseInsensitiveOption);
    if (content.indexOf(bbTagChannel) >= 0) {
        return "<a href=\"#AHI-" + content + "\"><img src=\":/images/hash.png\" />" + content + "</a>";
    }
    return content;
}

// todo: Check that this is a real F-List BBCode tag (it's not documented on the wiki), and that 'content' and 'param' are around the right way (currently they look swapped).
//  [session=Ponyville]ADH-27f9aeaebbf401b7178c[/session]
QString BBCodeParser::BBCodeTagSession::parse(QString& param, QString& content) {
    static QRegularExpression bbTagSession("[A-Za-z0-9 \\-]+", QRegularExpression::CaseInsensitiveOption);
    if (content.indexOf(bbTagSession) >= 0) {
        return "<a href=\"#AHI-" + content + "\"><img src=\":/images/key.png\" />" + param + "</a>";
    }
    return content;
}

QString BBCodeParser::BBCodeTagIcon::parse(QString& param, QString& content) {
    (void)param;
    static QRegularExpression bbTagIcon("[A-Za-z0-9 \\-_]+", QRegularExpression::CaseInsensitiveOption);
    if (content.indexOf(bbTagIcon) >= 0) {
        content = content.replace(" ", "%20");
        return "<a href=\"https://www.f-list.net/c/" + content + "\"><img class=\"icon\" src=\"https://static.f-list.net/images/avatar/" + content.toLower()
               + ".png\" style=\"width:50px;height:50px;\" align=\"top\"/></a>";
    }
    return content;
}

QString BBCodeParser::BBCodeTagEicon::parse(QString& param, QString& content) {
    (void)param;
    static QRegularExpression bbTagEicon("[A-Za-z0-9 \\-_]+", QRegularExpression::CaseInsensitiveOption);
    if (content.indexOf(bbTagEicon) >= 0) {
        content = content.replace(" ", "%20");
        return "<img class=\"eicon\" src=\"https://static.f-list.net/images/eicon/" + content.toLower() + ".gif\" style=\"width:50px;height:50px;\" align=\"top\"/>";
    }
    return content;
}

QString BBCodeParser::BBCodeTagUser::parse(QString& param, QString& content) {
    (void)param;
    static QRegularExpression bbTagUser("[A-Za-z0-9 \\-_]+", QRegularExpression::CaseInsensitiveOption);
    if (content.indexOf(bbTagUser) >= 0) {
        return "<a href=\"https://www.f-list.net/c/" + content + "\"><img src=\":/images/user.png\" />" + content + "</a>";
    }
    return content;
}

void BBCodeParser::addSmiley(QString tag, QString code) {
    smilies[tag] = code;
}

void BBCodeParser::addTag(QString tag, BBCodeParser::BBCodeTag* code) {
    tags[tag] = code;
}

void BBCodeParser::removeSmiley(QString smiley) {
    smilies.remove(smiley);
}

void BBCodeParser::removeTag(QString tag) {
    tags.remove(tag);
}

BBCodeParser::Tag* BBCodeParser::getTag(QString& name, QString& param) {
    Tag* t = 0;
    // grab a tag from the recycler
    if (tagpool.size() > 0) {
        t = tagpool.takeLast();
    }
    // or create a new one if there aren't any recycled
    else {
        t = new Tag();
    }
    t->name = name;
    t->param = param;
    // clear content
    t->content.remove(0, t->content.length());
    return t;
}

QString BBCodeParser::parse(QString& input) {
    QStack<Tag*>* stack = parse(input, 0, input.length());
    // close all remaining tags
    for (int l = stack->size() - 1; l > 0; l--) {
        // remove topmost tag
        Tag* t = stack->pop();
        BBCodeTag* bb = tags[t->name];
        // parse and add result to next
        // inner-most tag
        stack->top()->content.append(bb->parse(t->param, t->content));
        // recycle!
        delete t;
    }
    QString result = stack->top()->content;
    delete stack->pop();
    delete stack;
    stack = 0;
    result.replace("\n", "<br/>");
    return result;
}

QStack<BBCodeParser::Tag*>* BBCodeParser::parse(QString& input, int start, int end) {
    QStack<Tag*>* stack = new QStack<Tag*>();
    // add a dummy tag to collect output
    QString null;
    stack->push(getTag(null, null));
    /*
     * The content of the buffer from (start,i)
     * 0 = raw text
     * 1 = bbcode
     * 2 = smiley
     */
    int bufType = 0;
    // marker for tracking where the '=' is in a bbcode tag
    int paramStart = -1;
    for (int i = start; i < end; i++) {
        char c = input[i].toLatin1();
        // if we're processing raw text
        if (bufType == 0) {
            // and we see a tag-open
            if (c == '[') {
                // empty buffer
                escapeAppend(input, start, i, stack->top()->content);
                // mark the opening and continue
                start = i;
                bufType = 1;
            }
            // if we see the begin or end of a smiley
            else if (c == ':') {
                // flush the buffer and switch types
                escapeAppend(input, start, i, stack->top()->content);
                start = i;
                bufType = 2;
            }
        }
        // if we're processing a bbcode tag
        else if (bufType == 1) {
            // and we see another open-tag
            if (c == '[') {
                // beginning couldn't have been a tag. append (start,i) to
                // innermost tag's content
                escapeAppend(input, start, i, stack->top()->content);
                // set the current position as the new tag start
                start = i;
            }
            // and if we see a param element
            else if (c == '=' && paramStart == -1) {
                // mark it
                paramStart = i;
            }
            // but if we see a close-tag
            else if (c == ']') {
                // get the contents of the tag
                // break it into key and parameter
                QString key = input.mid(start + 1, paramStart == -1 ? i - start - 1 : paramStart - start - 1).toLower().trimmed();
                QString param = paramStart == -1 ? "" : input.mid(paramStart + 1, i - paramStart - 1).trimmed();
                paramStart = -1;
                // parse if it's a closing tag or and opening tag
                bool close = key.startsWith("/");
                if (close) {
                    key = key.mid(1).trimmed();
                }
                // is it a valid BBcode?
                if (tags.contains(key)) {
                    // is this tag allowed?
                    bool allowed = true;
                    for (int k = stack->size() - 1; k > 0; k--) {
                        allowed &= tags[stack->at(k)->name]->allows(key);
                        if (!allowed) {
                            break;
                        }
                    }
                    // is it a permitted opening tag?
                    if (allowed && !close) {
                        // push onto stack
                        Tag* t = getTag(key, param);
                        stack->push(t);
                    } else if (close) {
                        // check for opening tag
                        bool closed = false;
                        for (int k = stack->size() - 1; k >= 0; k--) {
                            if (stack->at(k)->name == key) {
                                // close all tags that are nested
                                for (int l = stack->size() - 1; l >= k; l--) {
                                    // remove topmost tag
                                    Tag* t = stack->pop();
                                    BBCodeTag* bb = tags[t->name];
                                    // parse and add result to next
                                    // inner-most tag
                                    stack->top()->content.append(bb->parse(t->param, t->content));
                                    // recycle!
                                    tagpool.append(t);
                                    closed = true;
                                }
                                break;
                            }
                        }
                        if (!closed && !allowed) {
                            escapeAppend(input, start, i + 1, stack->top()->content);
                        }
                    } else {
                        // not an allowed tag, just flush the buffer
                        escapeAppend(input, start, i + 1, stack->top()->content);
                    }
                } else {
                    // oops, it's not. flush the buffer
                    escapeAppend(input, start, i + 1, stack->top()->content);
                }
                // reset the tag parser
                start = i + 1;
                bufType = 0;
            }
        }
        // if we're processing a smiley
        else if (bufType == 2) {
            // and we see the end of the smiley
            if (c == ':') {
                // we have a smiley! (we think)
                QString name = input.mid(start + 1, i - start - 1).trimmed().toLower();
                if (smilies.contains(name)) {
                    stack->top()->content.append(smilies[name]);
                    // reset back to raw text
                    start = i + 1;
                    bufType = 0;
                } else {
                    // not a valid smiley, just flush buffer and set up for possible next smiley
                    escapeAppend(input, start, i, stack->top()->content);
                    // start new buffer
                    start = i;
                }
            }
            // but if we see the beginning of a bbcode tag
            else if (c == '[') {
                // not a smiley, it was all just text. flush the buffer and
                // set up for a bbcode tag
                escapeAppend(input, start, i, stack->top()->content);
                start = i;
                bufType = 1;
            }
        }
    }
    // flush the rest of the buffer
    if (start < input.length()) {
        escapeAppend(input, start, input.length(), stack->top()->content);
    }
    return stack;
}

void BBCodeParser::escapeAppend(QString& input, int start, int end, QString& output) {
    /*
    char c;
    for (int i = start; i < end; i++ )
    {
            c = input[i].toLatin1();
            switch (c)
            {
            case '<':
            case '>':
            case '\n':
                    // screw flist's server-side escaping.
                    //case '&':
                    output.append(input.mid(start, i - start));
                    start = i + 1;
                    if (c == '<') {
                            output.append("&lt;");
                    }
                    if (c == '>') {
                            output.append("&gt;");
                    }
                    if (c == '\n') {
                            output.append("<br/>");
                    }
                    if (c == '&') {
                            output.append("&amp;");
                    }
            }
    }
    // cleanup
    if (start < end) {
    */
    output.append(input.mid(start, end - start));
    //}
}
