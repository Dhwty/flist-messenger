#include "ui/channellistdialog.h"
#include "flist_global.h"

FChannelListModel::FChannelListModel()
{
	hash = QIcon ( ":/images/hash.png" );
	key = QIcon (":/images/key.png");
}

FChannelListModel::~FChannelListModel()
{
}

int FChannelListModel::rowCount(const QModelIndex & parent) const
{
	if (parent.isValid()) { return 0; }
	return channels.size() + rooms.size();
}

int FChannelListModel::columnCount(const QModelIndex & parent) const
{
	if (parent.isValid()) { return 0; }
	return colCount; // Type, name, members
}

QVariant FChannelListModel::data(const QModelIndex &index, int role) const
{
	if(role == Qt::DisplayRole)
	{
		switch(index.column())
		{
		case colType:
			return "";
		case colMembers:
			return byIndex(index.row()).count;
		case colTitle:
			return byIndex(index.row()).title;
		default:
			return QString("?");
		}
	}
	else if ((role == Qt::DecorationRole) && (index.column() == colType))
	{
		switch(byIndex(index.row()).type)
		{
		case FChannelSummary::Public:
			return hash;
		case FChannelSummary::Private:
			return key;
		case FChannelSummary::Unknown:
			return QVariant();
		}
	}
	else if (role == SortKeyRole)
	{
		switch(index.column())
		{
		case colType:
			return byIndex(index.row()).type;
		case colMembers:
			return byIndex(index.row()).count;
		case colTitle:
			return byIndex(index.row()).title;
		}
	}

	return QVariant();
}

QVariant FChannelListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if(orientation == Qt::Horizontal)
	{
		if(role == Qt::DisplayRole)
		{
			switch(section)
			{
			case colType:
				return QVariant();
			case colMembers:
				return QString("#");
			case colTitle:
				return QString("Name");
			}
		}
		else if (role==Qt::SizeHintRole && section == colType)
		{
			return QSize(16,16);
		}
	}
  
	return QVariant();
}

const FChannelSummary &FChannelListModel::byIndex(uint index) const
{
	if(index >= rooms.size())
	{
		return channels[index - rooms.size()];
	}
	return rooms[index];
}

FChannelListSortProxy::FChannelListSortProxy(QObject *parent) :
	QSortFilterProxyModel(parent),
    _showType(FChannelSummary::Unknown),
    _showEmpty(false)
{
	setDynamicSortFilter(true);
	setSortRole(FChannelListModel::SortKeyRole);
	setFilterRole(FChannelListModel::SortKeyRole);
    setFilterKeyColumn(-1);
	setSortLocaleAware(true);
	setSortCaseSensitivity(Qt::CaseInsensitive);
	setFilterCaseSensitivity(Qt::CaseInsensitive);
}

// Prevent displaying lists with zero characters
bool FChannelListSortProxy::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
	if (!QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent))
	{ return false; }


    QModelIndex i = sourceModel()->index(source_row, FChannelListModel::colMembers, source_parent);
    int nMembers = sourceModel()->data(i,  FChannelListModel::SortKeyRole).toInt();
    if ((!_showEmpty) && (nMembers == 0)) { return false; }
    if (_showType == FChannelSummary::Unknown) { return true; }
    i = sourceModel()->index(source_row, FChannelListModel::colType, source_parent);    
	FChannelSummary::Type t = static_cast<FChannelSummary::Type>(sourceModel()->data(i, FChannelListModel::SortKeyRole).toInt());
    return t == _showType;
}

FChannelSummary::Type FChannelListSortProxy::showType()
{
	return _showType;
}

void FChannelListSortProxy::setShowType(FChannelSummary::Type t)
{
	_showType = t;
	invalidate();
}

void FChannelListSortProxy::setShowEmpty(bool show)
{
    _showEmpty = show;
    invalidate();
}

FChannelListDialog::FChannelListDialog(FChannelListModel *m, QWidget *parent = 0) : QDialog(parent), Ui::FChannelListDialogUi()
{
	setupUi(this);
	joinButton = new QPushButton(QIcon(":/icons/tick.png"), QString("Join"));
	buttonBox->addButton(joinButton, QDialogButtonBox::AcceptRole);
	datasort = new FChannelListSortProxy();
	chTable->setModel(datasort);
	setModel(m);
	chTable->sortByColumn(FChannelListModel::colTitle, Qt::AscendingOrder);
	datasort->setShowType(FChannelSummary::Public);
}

FChannelListDialog::~FChannelListDialog()
{
}

FChannelListModel *FChannelListDialog::model()
{
	return data;
}
void FChannelListDialog::setModel(FChannelListModel *m)
{
	data = m;
	datasort->setSourceModel(data);
	chTable->resizeColumnsToContents();
#if QT_VERSION >= 0x050000
	chTable->horizontalHeader()->setSectionResizeMode(FChannelListModel::colType,	QHeaderView::ResizeToContents);
#else
	chTable->horizontalHeader()->setResizeMode(FChannelListModel::colType,	QHeaderView::ResizeToContents);
#endif
}

void FChannelListDialog::on_buttonBox_accepted()
{
	QStringList channels;
	QModelIndexList selected = chTable->selectionModel()->selectedRows(FChannelListModel::colTitle);
	for (QModelIndexList::ConstIterator i = selected.begin(); i != selected.end(); i++)
	{
		QModelIndex realIndex = datasort->mapToSource(*i);
		uint rownum = realIndex.row();
		FChannelSummary chan = data->byIndex(rownum);
		channels.push_back(chan.name);
	}
	emit joinRequested(channels);
	chTable->clearSelection();
}

void FChannelListDialog::on_chFilterText_textChanged(const QString &text)
{
	datasort->setFilterFixedString(text);
}

void FChannelListDialog::on_chTypeBoth_toggled(bool active)
{
	if(active) datasort->setShowType(FChannelSummary::Unknown);
}

void FChannelListDialog::on_chTypePublic_toggled(bool active)
{
	if(active) datasort->setShowType(FChannelSummary::Public);
}

void FChannelListDialog::on_chTypePrivate_toggled(bool active)
{
	if(active) datasort->setShowType(FChannelSummary::Private);
}

void FChannelListDialog::on_chShowEmpty_toggled(bool showEmpty)
{
    datasort->setShowEmpty(showEmpty);
}
