#include "flist_channellistdialog.h"

FChannelListModel::FChannelListModel()
{
	hash = QIcon ( ":/images/hash.png" );
	key = QIcon (":/images/key.png");

	QString a("Transformation");
	QString b("ADH-whatevereverever");
	QString c("Literate Ferals");
	rooms.push_back(new FChannelSummary(FChannelSummary::Public, a, 15));
	channels.push_back(new FChannelSummary(FChannelSummary::Private, b, c, 1));
}

FChannelListModel::~FChannelListModel()
{

	for(std::vector<FChannelSummary*>::iterator i = channels.begin(); i != channels.end(); i++)
  {
    delete *i;
  }
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
				return byIndex(index.row())->count;
			case colTitle:
				return byIndex(index.row())->title;
      default:
        return QString("?");
    }
  }
	else if ((role == Qt::DecorationRole) && (index.column() == colType))
  {
		switch(byIndex(index.row())->type)
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
			return byIndex(index.row())->type;
		case colMembers:
			return byIndex(index.row())->count;
		case colTitle:
			return byIndex(index.row())->title;
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

FChannelSummary *FChannelListModel::byIndex(uint index) const
{
	if(index >= rooms.size())
	{
		return channels[index - rooms.size()];
	}
	return rooms[index];
}

FChannelListSortProxy::FChannelListSortProxy(QObject *parent) :
	QSortFilterProxyModel(parent),
	_showType(FChannelSummary::Unknown)
{
	setSortRole(FChannelListModel::SortKeyRole);
	setFilterRole(FChannelListModel::SortKeyRole);
	setFilterKeyColumn(FChannelListModel::colTitle);
	setSortLocaleAware(true);
	setSortCaseSensitivity(Qt::CaseInsensitive);
	setFilterCaseSensitivity(Qt::CaseInsensitive);
}

bool FChannelListSortProxy::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
	if (!QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent))
	{ return false; }

	if (_showType == FChannelSummary::Unknown) { return true; }

	QModelIndex i = sourceModel()->index(source_row, FChannelListModel::colType, source_parent);
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
	invalidateFilter();
}

FChannelListDialog::FChannelListDialog(QWidget *parent = 0) : QDialog(parent), Ui::FChannelListDialogUi()
{
	setupUi(this);
	datasort = new FChannelListSortProxy();
	chTable->setModel(datasort);

	data = new FChannelListModel();
	datasort->setSourceModel(data);

	chTable->resizeColumnsToContents();
}

FChannelListDialog::~FChannelListDialog()
{
  delete data;
}

void FChannelListDialog::on_buttonBox_accepted()
{
  this->hide();
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
	if(active) datasort->setShowType(FChannelSummary::Private);
}

void FChannelListDialog::on_chTypePrivate_toggled(bool active)
{
	if(active) datasort->setShowType(FChannelSummary::Public);
}
