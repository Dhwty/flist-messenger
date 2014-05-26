#include "flist_channellistdialog.h"

FChannelListModel::Item::Item(chType t, QString n, int m) : type(t), name(n), members(m) { }

FChannelListModel::FChannelListModel()
{
  channels.push_back(new Item(chTypePublic, QString("Transformation"), 15));
  channels.push_back(new Item(chTypePrivate, QString("Literate Ferals"), 4));
}

FChannelListModel::~FChannelListModel()
{
  hash = QIcon ( ":/images/hash.png" );
  key = QIcon (":/images/key.png");

  for(std::vector<Item*>::iterator i = channels.begin(); i != channels.end(); i++)
  {
    delete *i;
  }
}

int FChannelListModel::rowCount(const QModelIndex & parent) const
{
  if (parent.isValid()) { return 0; }
  return channels.size();
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
        return channels[index.row()]->members;
      case colName:
        return channels[index.row()]->name;
      default:
        return QString("?");
    }
  }
  else if ((role == Qt::DecorationRole) && (index.column() == colType))
  {
    switch(channels[index.row()]->type)
    {
    case chTypePublic:
      return QIcon ( ":/images/hash.png" );
    case chTypePrivate:
      return QIcon ( ":/images/key.png" );
    }
  }
  else if (role == Qt::SizeHintRole && index.column() == colType)
  {
    return QSize(16,16);
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
          return QString("Type");
        case colMembers:
          return QString("Characters");
        case colName:
          return QString("Name");
      }
    }
  }
  
  return QVariant();
}

FChannelListDialog::FChannelListDialog(QWidget *parent = 0) : QDialog(parent), Ui::FChannelListDialogUi()
{
  setupUi(this);
  data = new FChannelListModel();
  chTable->setModel(data);
}

FChannelListDialog::~FChannelListDialog()
{
  delete data;
}

void FChannelListDialog::on_buttonBox_accepted()
{
  this->hide();
}
