#include <QDialog>
#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include <QIcon>
#include <vector>

#include "flist_channelsummary.h"
#include "ui_flist_channellistdialog.h"

class FChannelListModel : public QAbstractTableModel
{
  
public:
	enum columns
	{
		colType = 0,
		colMembers = 1,
		colTitle = 2,
		colCount = 3
	};

	static const int SortKeyRole = Qt::UserRole + 0;

  FChannelListModel();
  ~FChannelListModel();
  
  int rowCount(const QModelIndex & parent = QModelIndex()) const;
  int columnCount(const QModelIndex & parent = QModelIndex()) const;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
  
private:
	std::vector<FChannelSummary*> channels;
	std::vector<FChannelSummary*> rooms;
	QIcon hash;
	QIcon key;

	FChannelSummary *byIndex(uint index) const;
};

class FChannelListSortProxy : public QSortFilterProxyModel
{
public:

	FChannelListSortProxy(QObject * parent = 0);
	FChannelSummary::Type showType();
	void setShowType(FChannelSummary::Type t);

protected:
	virtual bool filterAcceptsRow(int source_row, const QModelIndex & source_parent) const;

private:
	FChannelSummary::Type _showType;
};

class FChannelListDialog : public QDialog, private Ui::FChannelListDialogUi
{
  Q_OBJECT
  
public:
  FChannelListDialog(QWidget *parent/*, QAbstractTableModel *channels*/);
  ~FChannelListDialog();
  
private:
  FChannelListModel *data;
	FChannelListSortProxy *datasort;
  
private slots:
  void on_buttonBox_accepted();
	void on_chFilterText_textChanged(const QString&);
	void on_chTypeBoth_toggled(bool);
	void on_chTypePublic_toggled(bool);
	void on_chTypePrivate_toggled(bool);
};
