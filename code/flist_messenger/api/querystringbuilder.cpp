#include "querystringbuilder.h"

#include <QMultiMap>
#include <QStringBuilder>
#include <stdint.h>

class QueryStringBuilder::Internal
{
public:
	QMultiMap<QString,QString> data;
};

QueryStringBuilder::QueryStringBuilder()
{
	this->d = new Internal();
}

QueryStringBuilder::~QueryStringBuilder()
{
	delete this->d;
}

void QueryStringBuilder::addQueryItem(QString key, QString value)
{
	d->data.insert(key, value);
}

namespace {
/* Translation table
 *   0  Leave alone
 *   1  Percent encode
 * This table lists which bytes need to be percent-encoded to turn utf-8 into
 * a query string.
 *
 * According to RFC 1738 the only legal characters in a querystring are alphanumeric
 * and $-_.+!*'(),
 */
char translation[256] = {
//  Controls
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
//    ! " # $ % & ' ( ) * + , - . /
    1,0,1,1,0,1,1,0,0,0,0,0,0,0,0,1,
//  0 1 2 3 4 5 6 7 8 9 : ; < = > ?
    0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,
//  @ A B C D E F G H I J K L M N O
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
//  P Q R S T U V W X Y Z [ \ ] ^ _
    0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,
//  ` a b c d e f g h i j k l m n o
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
//  p q r s t u v w x y z { | } ~
    0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,
//  Everything with the top bit set
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

char nibbles[16] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };

void encodeString(QString str, QByteArray &output)
{
	QByteArray input = str.toUtf8();
	output.reserve(input.length());
	for(int i = 0; i < input.length(); i++)
	{
		uint8_t c = input.at(i);
		if(translation[c])
		{
			output.append('%');
			output.append(nibbles[c >> 4]);
			output.append(nibbles[c & 0x0F]);
		}
		else
		{
			output.append(input[i]);
		}
	}
}
}

QByteArray QueryStringBuilder::encodedQuery()
{
	QByteArray output;
	QMapIterator<QString, QString> iter(d->data);
	while(iter.hasNext())
	{
		iter.next();
		encodeString(iter.key(), output);
		output.append('=');
		encodeString(iter.value(), output);
		if(iter.hasNext())
		{
			output.append('&');
		}
	}
	return output;
}

