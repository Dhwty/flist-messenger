#include "addremovelistview.h"
#include "ui_addremovelistview.h"
#include "flist_global.h"

#include <QCompleter>
#include <QSortFilterProxyModel>

AddRemoveListView::AddRemoveListView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AddRemoveListView)
{
	ui->setupUi(this);

	connect(ui->btnAdd, SIGNAL(clicked(bool)), this, SLOT(addClicked()));
	connect(ui->btnRemove, SIGNAL(clicked(bool)), this, SLOT(removeClicked()));
	connect(ui->ivItemList->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(listSelectionChanged(QModelIndex,QModelIndex)));
	connect(ui->leItemEdit, SIGNAL(textChanged(QString)), this, SLOT(textChanged(QString)));
	
	completer = new QCompleter(this);
	completer->setCompletionMode(QCompleter::PopupCompletion);
	completer->setCaseSensitivity(Qt::CaseInsensitive);
	completer->setCompletionRole(Qt::EditRole);
	ui->leItemEdit->setCompleter(completer);
}

AddRemoveListView::~AddRemoveListView()
{
	delete ui;
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
	completionSorter = new QSortFilterProxyModel();
	completionSorter->setSourceModel(completionData);
	completionSorter->setSortRole(Qt::EditRole);
	completionSorter->sort(dataProvider->completionColumn());
	completer->setModel(completionSorter);
	completer->setCompletionColumn(dataProvider->completionColumn());
	
	listData = dataProvider->getListSource();
	listSorter = new QSortFilterProxyModel();
	listSorter->setSourceModel(listData);
	listSorter->setSortRole(Qt::EditRole);
	listSorter->sort(dataProvider->listColumn());
	ui->ivItemList->setModel(listData);
	ui->ivItemList->setModelColumn(dataProvider->listColumn());
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
		ui->leItemEdit->setText(listSorter->data(current.row(),,Qt::EditRole));
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

	if(addable)
	{
		for(int i = 0; i < completionSorter->rowCount(); i++)
		{
			QModelIndex mi(i, dataProvider->completionColumn(), listSorter);

			QString text = completionSorter->data(mi, Qt::EditRole);
			int compresult = text.compare(data, Qt::CaseInsensitive);
			if(compresult == 0)
			{
				ui->leItemEdit->setText(text);
				done = true;
				break;
			}
			if(compresult > 0) { break; }
		}
	}

	if(removable)
	{
		for(int i = 0; i < listSorter->rowCount(); i++)
		{
			QModelIndex mi(i, dataProvider->listColumn(), listSorter);

			QString text = listSorter->data(mi, Qt::EditRole);
			int compresult = text.compare(data, Qt::CaseInsensitive);
			if(compresult == 0)
			{
				ui->leItemEdit->setText(text);
				ui->ivItemList->setCurrentIndex(mi);
				break;
			}
			if(compresult > 0) { break; }
		}
	}
}
