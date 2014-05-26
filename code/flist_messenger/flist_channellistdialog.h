#include <QDialog>
#include <QAbstractTableModel>
#include <QIcon>
#include <vector>

#include "ui_flist_channellistdialog.h"

class FChannelListModel : public QAbstractTableModel
{
  
public:
  enum chType
  {
    chTypePublic,
    chTypePrivate
  };
  
  
  struct Item
  {
    chType type;
    QString name;
    int members;
    
    Item(chType t, QString n, int m);
  };
  
  FChannelListModel();
  ~FChannelListModel();
  
  int rowCount(const QModelIndex & parent = QModelIndex()) const;
  int columnCount(const QModelIndex & parent = QModelIndex()) const;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
  
private:
  std::vector<Item*> channels;
  QIcon hash;
  QIcon key;
  
  enum columns
  {
    colType = 0,
    colMembers = 1,
    colName = 2,
    colCount = 3
  };
};

class FChannelListDialog : public QDialog, private Ui::FChannelListDialogUi
{
  Q_OBJECT
  
public:
  FChannelListDialog(QWidget *parent/*, QAbstractTableModel *channels*/);
  ~FChannelListDialog();
  
private:
  FChannelListModel *data;
  
private slots:
  void on_buttonBox_accepted();
};
