#ifndef STRINGCHARACTERLISTMODEL_H
#define STRINGCHARACTERLISTMODEL_H

#include <QAbstractListModel>

class NotifyStringList;

class FSession;

class StringCharacterListModel : public QAbstractListModel
{
	Q_OBJECT
public:
	explicit StringCharacterListModel(NotifyStringList *list, QObject *parent = 0);

	// Basic functionality:
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

private:
	NotifyStringList *dataSource;

private slots:
	void sourceAdded(int first, int last);
	void sourceRemoved(int first, int last);
	void sourceBeforeAdd(int first, int last);
	void sourceBeforeRemove(int first, int last);

};

#endif // STRINGCHARACTERLISTMODEL_H
