#include "addremovelistview.h"
#include "ui_addremovelistview.h"
#include "flist_global.h"

#include <QCompleter>
#include <QSortFilterProxyModel>
#include <QItemSelectionModel>

AddRemoveListView::AddRemoveListView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AddRemoveListView)
{
	ui->setupUi(this);

	connect(ui->btnAdd, SIGNAL(clicked(bool)), this, SLOT(addClicked()));
	connect(ui->btnRemove, SIGNAL(clicked(bool)), this, SLOT(removeClicked()));
	connect(ui->leItemEdit, SIGNAL(textChanged(QString)), this, SLOT(textChanged(QString)));
	
	completer = new QCompleter(this);
	completer->setCompletionMode(QCompleter::PopupCompletion);
	completer->setCaseSensitivity(Qt::CaseInsensitive);
	completer->setCompletionRole(Qt::EditRole);
	ui->leItemEdit->setCompleter(completer);

	completionData = 0;
	completionSorter = 0;
	listData = 0;
	listSorter = 0;
}

AddRemoveListView::~AddRemoveListView()
{
	delete ui;
}

void AddRemoveListView::setDataSource(AddRemoveListData *data)
{
	if(dataProvider)
	{
		detachData();
	}

	dataProvider = data;
	attachData();
}

AddRemoveListData *AddRemoveListView::dataSource() const
{
	return dataProvider;
}

void AddRemoveListView::hideEvent(QHideEvent *)
{
	detachData();
}

void AddRemoveListView::showEvent(QShowEvent *)
{
	attachData();
}

void AddRemoveListView::detachData()
{
	if(completionData)
	{
		completer->setModel(0);
		dataProvider->doneWithCompletionSource(completionData);
		completionData = 0;
		delete completionSorter;
		completionSorter = 0;
	}
	
	if(listData)
	{
		disconnect(ui->ivItemList->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(listSelectionChanged(QModelIndex,QModelIndex)));

		ui->ivItemList->setModel(0);
		dataProvider->doneWithListSource(listData);
		listData = 0;
		delete listSorter;
		listSorter = 0;
	}
}

void AddRemoveListView::attachData()
{
	completionData = dataProvider->getCompletionSource();
	if(completionData)
	{
		completionSorter = new QSortFilterProxyModel();
		completionSorter->setSourceModel(completionData);
		completionSorter->setSortRole(Qt::EditRole);
		completionSorter->sort(dataProvider->completionColumn());
		completer->setModel(completionSorter);
		completer->setCompletionColumn(dataProvider->completionColumn());
	}
	
	listData = dataProvider->getListSource();
	listSorter = new QSortFilterProxyModel();
	listSorter->setSourceModel(listData);
	listSorter->setSortRole(Qt::EditRole);
	listSorter->sort(dataProvider->listColumn());
	ui->ivItemList->setModel(listSorter);
	ui->ivItemList->setModelColumn(dataProvider->listColumn());

	connect(ui->ivItemList->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(listSelectionChanged(QModelIndex,QModelIndex)));
}

void AddRemoveListView::addClicked()
{
	QString data = ui->leItemEdit->text();
	if(data != "" && dataProvider->isStringValidForAdd(data))
	{
		dataProvider->addByString(data);
		ui->leItemEdit->clear();
		ui->btnAdd->setEnabled(false);
		ui->btnRemove->setEnabled(false);
		ui->ivItemList->clearSelection();
	}
}

void AddRemoveListView::removeClicked()
{
	QString data = ui->leItemEdit->text();
	if(data != "" && dataProvider->isStringValidForRemove(data))
	{
		dataProvider->removeByString(data);
		ui->leItemEdit->clear();
		ui->btnAdd->setEnabled(false);
		ui->btnRemove->setEnabled(false);
		ui->ivItemList->clearSelection();
	}
}

void AddRemoveListView::listSelectionChanged(QModelIndex current, QModelIndex previous)
{
	if(current == previous) { return; }

	if(current.isValid())
	{
		ui->leItemEdit->setText(listSorter->data(current, Qt::EditRole).toString());
		ui->btnAdd->setEnabled(false);
		ui->btnRemove->setEnabled(true);
	}
	else
	{
		ui->btnRemove->setEnabled(false);
	}
}

void AddRemoveListView::textChanged(QString newText)
{
	bool addable = dataProvider->isStringValidForAdd(newText);
	bool removable = dataProvider->isStringValidForRemove(newText);

	ui->btnAdd->setEnabled(addable);
	ui->btnRemove->setEnabled(removable);

	if(!removable)
	{
		ui->ivItemList->clearSelection();
	}

	if(addable && completionSorter)
	{
		for(int i = 0; i < completionSorter->rowCount(); i++)
		{
			QModelIndex mi = completionSorter->index(i, dataProvider->completionColumn());

			QString text = completionSorter->data(mi, Qt::EditRole).toString();
			int compresult = text.compare(newText, Qt::CaseInsensitive);
			if(compresult == 0)
			{
				ui->leItemEdit->setText(text);
				break;
			}
			if(compresult > 0) { break; }
		}
	}

	if(removable)
	{
		for(int i = 0; i < listSorter->rowCount(); i++)
		{
			QModelIndex mi = listSorter->index(i, dataProvider->listColumn());

			QString text = listSorter->data(mi, Qt::EditRole).toString();
			int compresult = text.compare(newText, Qt::CaseInsensitive);
			if(compresult == 0)
			{
				ui->leItemEdit->setText(text);
				ui->ivItemList->setCurrentIndex(mi);
				QItemSelectionModel *model = ui->ivItemList->selectionModel();
				model->select(mi, QItemSelectionModel::ClearAndSelect);
				break;
			}
			if(compresult > 0) { break; }
		}
	}
}

QAbstractItemModel *AddRemoveListData::getCompletionSource() { return 0; }
void AddRemoveListData::doneWithCompletionSource(QAbstractItemModel *source) { }
int AddRemoveListData::completionColumn() { return 0; }
int AddRemoveListData::listColumn() { return 0; }
