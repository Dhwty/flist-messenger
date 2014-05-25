#ifndef FLIST_CHANNELSUMMARY_H
#define FLIST_CHANNELSUMMARY_H

class FChannelSummary
{
public:
	enum Type
	{
		Public,
		Private
	};

	FChannelSummary() : 
		type(Public),
		name(),
		title(),
		count(0)
	{}
	FChannelSummary(Type type, QString &name, int count) :
		type(type),
		name(name),
		title(name),
		count(count)
	{}
	FChannelSummary(Type type, QString &name, QString &title, int count) :
		type(type),
		name(name),
		title(title),
		count(count)
	{}

	Type type;
	QString name;
	QString title;
	int count;
};

#endif // FLIST_CHANNELSUMMARY_H
