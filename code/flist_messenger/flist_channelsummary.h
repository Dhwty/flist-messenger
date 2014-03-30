#ifndef FLIST_CHANNELSUMMARY_H
#define FLIST_CHANNELSUMMARY_H

class FChannelSummary
{
public:
	FChannelSummary() : 
		name(),
		title(),
		count(0)
	{}
	FChannelSummary(QString &name, int count) : 
		name(name),
		title(name),
		count(count)
	{}
	FChannelSummary(QString &name, QString &title, int count) : 
		name(name),
		title(title),
		count(count)
	{}

	QString name;
	QString title;
	int count;
};

#endif // FLIST_CHANNELSUMMARY_H
