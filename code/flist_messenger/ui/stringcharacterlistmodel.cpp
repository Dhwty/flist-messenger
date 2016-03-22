#include "stringcharacterlistmodel.h"
#include "flist_session.h"
#include "flist_character.h"
#include "notifylist.h"
#include "flist_global.h"

StringCharacterListModel::StringCharacterListModel(NotifyStringList *list, QObject *parent)
	: QAbstractListModel(parent),
	  dataSource(list)
{
	NotifyListNotifier *notifier = dataSource->notifier();
	connect(notifier, SIGNAL(added(int,int)), this, SLOT(sourceAdded(int,int)));
	connect(notifier, SIGNAL(beforeAdd(int,int)), this, SLOT(sourceBeforeAdd(int,int)));
	connect(notifier, SIGNAL(beforeRemove(int,int)), this, SLOT(sourceBeforeRemove(int,int)));
	connect(notifier, SIGNAL(removed(int,int)), this, SLOT(sourceRemoved(int,int)));
}

int StringCharacterListModel::rowCount(const QModelIndex &parent) const
{
	return dataSource->count();
}

QVariant StringCharacterListModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	QString charName((*dataSource)[index.row()]);
	switch(role)
	{
	case Qt::EditRole:
	case Qt::DisplayRole:
		return QVariant(charName);
	default:
		return QVariant();
	}
}

void StringCharacterListModel::sourceAdded(int first, int last)
{
	this->endInsertRows();
}

void StringCharacterListModel::sourceRemoved(int first, int last)
{
	this->endRemoveRows();
}

void StringCharacterListModel::sourceBeforeAdd(int first, int last)
{
	this->beginInsertRows(QModelIndex(), first, last);
}

void StringCharacterListModel::sourceBeforeRemove(int first, int last)
{
	this->beginRemoveRows(QModelIndex(), first, last);
}
