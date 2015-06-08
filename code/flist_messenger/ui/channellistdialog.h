#include <QDialog>
#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include <QIcon>
#include <QPushButton>
#include <vector>

#include "flist_channelsummary.h"
#include "ui_channellistdialog.h"

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
	const FChannelSummary &byIndex(uint index) const;
  
	template<class InputIterator>
	void updateChannels(InputIterator first, InputIterator last)
	{
		beginResetModel();
		channels.clear();
		channels.assign(first, last);
		endResetModel();
	}

	template<class InputIterator>
	void updateRooms(InputIterator first, InputIterator last)
	{
		beginResetModel();
		rooms.clear();
		rooms.assign(first, last);
		endResetModel();
	}

private:
	std::vector<FChannelSummary> channels;
	std::vector<FChannelSummary> rooms;
	QIcon hash;
	QIcon key;
};

class FChannelListSortProxy : public QSortFilterProxyModel
{
public:

	FChannelListSortProxy(QObject * parent = 0);
	FChannelSummary::Type showType();
	void setShowType(FChannelSummary::Type t);
    void setShowEmpty(bool show);

protected:
	virtual bool filterAcceptsRow(int source_row, const QModelIndex & source_parent) const;

private:
	FChannelSummary::Type _showType;
    bool _showEmpty;
};

class FChannelListDialog : public QDialog, private Ui::FChannelListDialogUi
{
	Q_OBJECT
  
public:
	FChannelListDialog(FChannelListModel *m, QWidget *parent);
	~FChannelListDialog();

	FChannelListModel *model();
	void setModel(FChannelListModel*);
    void setShowEmpty(bool show);

signals:
	void joinRequested(QStringList channels);
  
private:
	FChannelListModel *data;
	FChannelListSortProxy *datasort;
	QPushButton *joinButton;
  
private slots:
	void on_buttonBox_accepted();
	void on_chFilterText_textChanged(const QString&);
    void on_chShowEmpty_toggled(bool);
	void on_chTypeBoth_toggled(bool);
	void on_chTypePublic_toggled(bool);
	void on_chTypePrivate_toggled(bool);
};
