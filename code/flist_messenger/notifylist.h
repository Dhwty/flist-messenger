#ifndef NOTIFYLIST_H
#define NOTIFYLIST_H

#include <QObject>

class NotifyListNotifier;

class NotifyStringList
{
public:
	explicit NotifyStringList();
	explicit NotifyStringList(const QList<QString> &other);
	explicit NotifyStringList(const QStringList &other);
	virtual ~NotifyStringList();

	void append(const QString &value);
	int removeAll(const QString &value);
	bool contains(const QString &value) const;
	bool contains(const QString &value, Qt::CaseSensitivity sensitivity) const;
	int count() const;
	void clear();

	QString &operator[](const int &index);

	NotifyListNotifier *notifier() const;

private:
	QStringList contents;
	NotifyListNotifier *_notifier;
};

class NotifyListNotifier : public QObject
{
	Q_OBJECT
public:
	explicit NotifyListNotifier(QObject *parent = 0);

	void notifyAdded(const int firstIndex, const int lastIndex);
	void notifyRemoved(const int firstIndex, const int lastIndex);
	void notifyBeforeAdd(const int firstIndex, const int lastIndex);
	void notifyBeforeRemove(const int firstIndex, const int lastIndex);

signals:
	void added(const int firstIndex, const int lastIndex);
	void removed(const int firstIndex, const int lastIndex);

	// These are emitted before changes take effect, because QAbstractItemModels like that.
	void beforeAdd(const int firstIndex, const int lastIndex);
	void beforeRemove(const int firstIndex, const int lastIndex);

};
#endif // NOTIFYLIST_H
