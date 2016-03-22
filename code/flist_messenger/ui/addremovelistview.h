#ifndef ADDREMOVELISTVIEW_H
#define ADDREMOVELISTVIEW_H

#include <QWidget>
#include <QModelIndex>

namespace Ui {
class AddRemoveListView;
}
class QAbstractItemModel;
class QSortFilterProxyModel;
class AddRemoveListData;
class QCompleter;

class AddRemoveListView : public QWidget
{
	Q_OBJECT
	
public:
	explicit AddRemoveListView(QWidget *parent = 0);
	~AddRemoveListView();
	
	void setDataSource(AddRemoveListData *data);
	AddRemoveListData *dataSource() const;
	
protected:
	virtual void hideEvent(QHideEvent *);
	virtual void showEvent(QShowEvent *);
	
private:
	Ui::AddRemoveListView *ui;
	AddRemoveListData *dataProvider;
	QCompleter *completer;
	
	QAbstractItemModel *completionData, *listData;
	QSortFilterProxyModel *completionSorter, *listSorter;
	
	void detachData();
	void attachData();
	
private slots:
	void addClicked();
	void removeClicked();
	void listSelectionChanged(QModelIndex current, QModelIndex previous);
	void textChanged(QString newText);
};

class AddRemoveListData
{
public:
	// Return true if this string refers to the EditRole of a valid item to insert.
	virtual bool isStringValidForAdd(QString text) = 0;
	// or remove. Don't return true for both at once, that doesn't make sense.
	virtual bool isStringValidForRemove(QString text) = 0;
	
	// Actually insert an item by giving its EditRole string
	virtual void addByString(QString text) = 0;
	
	// Remove the item whose EditRole string is this.
	virtual void removeByString(QString text) = 0;
	
	// Retrieve a model suggesting potential additions to the list (don't worry about if they've already been added).
	virtual QAbstractItemModel *getCompletionSource();
	// Return the completion source for disposal.
	virtual void doneWithCompletionSource(QAbstractItemModel *source);
	virtual int completionColumn();
	
	// Retrieve a model listing items currently in the list.
	virtual QAbstractItemModel *getListSource() = 0;
	// Return the list source for disposal.
	virtual void doneWithListSource(QAbstractItemModel *source) = 0;
	virtual int listColumn();
};

#endif // ADDREMOVELISTVIEW_H
