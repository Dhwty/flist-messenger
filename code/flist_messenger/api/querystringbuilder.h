#ifndef QUERYSTRINGBUILDER_H
#define QUERYSTRINGBUILDER_H

#include <QString>
#include <QByteArray>

class QueryStringBuilder
{
	class Internal;

	Internal *d;

public:
	QueryStringBuilder();
	~QueryStringBuilder();

	void addQueryItem(QString key, QString value);
	QByteArray encodedQuery();
};

#endif // QUERYSTRINGBUILDER_H
