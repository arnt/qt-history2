/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/* NOTES WHILE WRITING THIS:

  - QCache::find() should be QCache::value() as everywhere else
  - QAbstractItemView misses a public interface to have data refreshed
  - QAbstractItemModel needs a "refresh everything" signal
  - DisplayRole - why not TextRole?

  DOCU:
  
  - insertRow() and insertColumn() are gone in QAbstractItemModel
  - paramters of rowsInserted() and rowsRemoved() are underdocumented
  - parent item concept for non-hierarchical views is underdocumented
  - implies that QAbstractListModel::columnCount() can be implemented, but it's not virtual
  - A qHash() implementation for QModelIndex keys would be useful
  - 
*/


#include "centralwidget.h"

#include <qcache.h>

#include <qtabwidget.h>
#include <qlistview.h>
#include <qlayout.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qfile.h>
#include <qtextstream.h>

#include "msoutl.h"

uint qHash(const QModelIndex &key)
{
    return (key.row()+1) * (key.column()+1) + key.column();
}

class AddressBookModel : public QAbstractListModel
{
public:
    AddressBookModel(ABCentralWidget *parent);
    ~AddressBookModel();

    int rowCount() const;
    QVariant data(const QModelIndex &index, int role) const;

    void addItem(const QString &firstName, const QString &lastName, const QString &address, const QString &email);
    void update();

private:
    Outlook::Application outlook;
    Outlook::Items * contactItems;

    mutable QCache<QModelIndex, QStringList> cache;
};

AddressBookModel::AddressBookModel(ABCentralWidget *parent)
: QAbstractListModel(parent), cache(100)
{
    if (!outlook.isNull()) {
        Outlook::NameSpace session(outlook.Session());
        session.Logon();
        Outlook::MAPIFolder folder(session.GetDefaultFolder(Outlook::olFolderContacts));
        contactItems = new Outlook::Items(folder.Items());
	connect(contactItems, SIGNAL(ItemAdd(IDispatch*)), parent, SLOT(updateOutlook()));
	connect(contactItems, SIGNAL(ItemChange(IDispatch*)), parent, SLOT(updateOutlook()));
	connect(contactItems, SIGNAL(ItemRemove()), parent, SLOT(updateOutlook()));    
    }
}

AddressBookModel::~AddressBookModel()
{
    if (!outlook.isNull())
        Outlook::NameSpace(outlook.Session()).Logoff();
}

int AddressBookModel::rowCount() const
{
    return contactItems ? contactItems->Count() : 0;
}

QVariant AddressBookModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (index.type() == QModelIndex::HorizontalHeader)
        return index.column();
    if (index.type() == QModelIndex::VerticalHeader)
        return index.row();

    QStringList data;
    if (!cache.contains(index)) {
        Outlook::ContactItem contact(contactItems->Item(index.row() + 1));
        data << contact.FirstName() << contact.LastName() << contact.HomeAddress() << contact.Email1Address();
        cache.insert(index, new QStringList(data));
    } else {
        data = *cache.find(index);
    }
    if (index.column() < data.count())
        return data.at(index.column());

    return QVariant();
}

void AddressBookModel::addItem(const QString &firstName, const QString &lastName, const QString &address, const QString &email)
{
    Outlook::ContactItem item(outlook.CreateItem(Outlook::olContactItem));
    if (!item.isNull()) {
        item.SetFirstName(firstName);
        item.SetLastName(lastName);
        item.SetHomeAddress(address);
        item.SetEmail1Address(email);

        item.Save();
    }
}

void AddressBookModel::update()
{
    cache.clear();

    emit dataChanged(QModelIndex(), QModelIndex());
    emit rowsInserted(QModelIndex(), 0, rowCount());
}


ABCentralWidget::ABCentralWidget(QWidget *parent)
: QWidget(parent)
{
    mainGrid = new QGridLayout(this);
//    mainGrid->setNumRows(2, 1, 5, 5);

    setupTabWidget();
    setupListView();

    mainGrid->setRowStretch(0, 0);
    mainGrid->setRowStretch(1, 1);
}

ABCentralWidget::~ABCentralWidget()
{
}

void ABCentralWidget::save(const QString &filename)
{
#if 0
    if (!listView->firstChild())
        return;

    QFile f(filename);
    if (!f.open(IO_WriteOnly))
        return;

    QTextStream t(&f);

    QListViewItemIterator it(listView);

    for (; it.current(); ++it)
        for (unsigned int i = 0; i < 4; i++)
            t << it.current()->text(i) << "\n";

    f.close();
#endif
}

void ABCentralWidget::load(const QString &filename)
{
#if 0
    listView->clear();

    QFile f(filename);
    if (!f.open(IO_ReadOnly))
        return;

    QTextStream t(&f);

    while (!t.atEnd()) {
        QListViewItem *item = new QListViewItem(listView);
        for (unsigned int i = 0; i < 4; i++)
            item->setText(i, t.readLine());
    }

    f.close();
#endif
}

void ABCentralWidget::setupTabWidget()
{
    tabWidget = new QTabWidget(this);

    QWidget *input = new QWidget(tabWidget);
    QGridLayout *grid1 = new QGridLayout(input /*, 2, 5, 5, 5*/);

    QLabel *liFirstName = new QLabel("First &Name", input);
    liFirstName->resize(liFirstName->sizeHint());
    grid1->addWidget(liFirstName, 0, 0);

    QLabel *liLastName = new QLabel("&Last Name", input);
    liLastName->resize(liLastName->sizeHint());
    grid1->addWidget(liLastName, 0, 1);

    QLabel *liAddress = new QLabel("Add&ress", input);
    liAddress->resize(liAddress->sizeHint());
    grid1->addWidget(liAddress, 0, 2);

    QLabel *liEMail = new QLabel("&E-Mail", input);
    liEMail->resize(liEMail->sizeHint());
    grid1->addWidget(liEMail, 0, 3);

    add = new QPushButton("A&dd", input);
    add->resize(add->sizeHint());
    grid1->addWidget(add, 0, 4);
    connect(add, SIGNAL(clicked()), this, SLOT(addEntry()));

    iFirstName = new QLineEdit(input);
    iFirstName->resize(iFirstName->sizeHint());
    grid1->addWidget(iFirstName, 1, 0);
    liFirstName->setBuddy(iFirstName);

    iLastName = new QLineEdit(input);
    iLastName->resize(iLastName->sizeHint());
    grid1->addWidget(iLastName, 1, 1);
    liLastName->setBuddy(iLastName);

    iAddress = new QLineEdit(input);
    iAddress->resize(iAddress->sizeHint());
    grid1->addWidget(iAddress, 1, 2);
    liAddress->setBuddy(iAddress);

    iEMail = new QLineEdit(input);
    iEMail->resize(iEMail->sizeHint());
    grid1->addWidget(iEMail, 1, 3);
    liEMail->setBuddy(iEMail);

    change = new QPushButton("&Change", input);
    change->resize(change->sizeHint());
    grid1->addWidget(change, 1, 4);
    connect(change, SIGNAL(clicked()), this, SLOT(changeEntry()));

    tabWidget->addTab(input, "&Add/Change Entry");

    // --------------------------------------

    QWidget *search = new QWidget(this);
    QGridLayout *grid2 = new QGridLayout(search/*, 2, 5, 5, 5*/);

    cFirstName = new QCheckBox("First &Name", search);
    cFirstName->resize(cFirstName->sizeHint());
    grid2->addWidget(cFirstName, 0, 0);
    connect(cFirstName, SIGNAL(clicked()), this, SLOT(toggleFirstName()));

    cLastName = new QCheckBox("&Last Name", search);
    cLastName->resize(cLastName->sizeHint());
    grid2->addWidget(cLastName, 0, 1);
    connect(cLastName, SIGNAL(clicked()), this, SLOT(toggleLastName()));

    cAddress = new QCheckBox("Add&ress", search);
    cAddress->resize(cAddress->sizeHint());
    grid2->addWidget(cAddress, 0, 2);
    connect(cAddress, SIGNAL(clicked()), this, SLOT(toggleAddress()));

    cEMail = new QCheckBox("&E-Mail", search);
    cEMail->resize(cEMail->sizeHint());
    grid2->addWidget(cEMail, 0, 3);
    connect(cEMail, SIGNAL(clicked()), this, SLOT(toggleEMail()));

    sFirstName = new QLineEdit(search);
    sFirstName->resize(sFirstName->sizeHint());
    grid2->addWidget(sFirstName, 1, 0);

    sLastName = new QLineEdit(search);
    sLastName->resize(sLastName->sizeHint());
    grid2->addWidget(sLastName, 1, 1);

    sAddress = new QLineEdit(search);
    sAddress->resize(sAddress->sizeHint());
    grid2->addWidget(sAddress, 1, 2);

    sEMail = new QLineEdit(search);
    sEMail->resize(sEMail->sizeHint());
    grid2->addWidget(sEMail, 1, 3);

    find = new QPushButton("F&ind", search);
    find->resize(find->sizeHint());
    grid2->addWidget(find, 1, 4);
    connect(find, SIGNAL(clicked()), this, SLOT(findEntries()));

    cFirstName->setChecked(TRUE);
    sFirstName->setEnabled(TRUE);
    sLastName->setEnabled(FALSE);
    sAddress->setEnabled(FALSE);
    sEMail->setEnabled(FALSE);

    tabWidget->addTab(search, "&Search");

    mainGrid->addWidget(tabWidget, 0, 0);
}

void ABCentralWidget::setupListView()
{
    listView = new QListView(this);
/*
    listView->addColumn("First Name");
    listView->addColumn("Last Name");
    listView->addColumn("Address");
    listView->addColumn("E-Mail");
*/

    listView->setSelectionMode(QListView::SingleSelection);
    model = new AddressBookModel(this);
    listView->setModel(model);

    connect(listView, SIGNAL(clicked(QModelIndex, int)), this, SLOT(itemSelected(QModelIndex, int)));

    mainGrid->addWidget(listView, 1, 0);
//    listView->setAllColumnsShowFocus(TRUE);
}

void ABCentralWidget::updateOutlook()
{
    model->update();
}

void ABCentralWidget::addEntry()
{
    if (!iFirstName->text().isEmpty() || !iLastName->text().isEmpty() ||
         !iAddress->text().isEmpty() || !iEMail->text().isEmpty()) {
        model->addItem(iFirstName->text(), iFirstName->text(), iAddress->text(), iEMail->text());
    }

    iFirstName->setText("");
    iLastName->setText("");
    iAddress->setText("");
    iEMail->setText("");
}

void ABCentralWidget::changeEntry()
{
#if 0
    ABListViewItem *item = (ABListViewItem*)listView->currentItem();

    if (item &&
         (!iFirstName->text().isEmpty() || !iLastName->text().isEmpty() ||
           !iAddress->text().isEmpty() || !iEMail->text().isEmpty())) {

	QAxObject *contactItem = item->contactItem();
	contactItem->setProperty("FirstName", iFirstName->text());
	contactItem->setProperty("LastName", iLastName->text());
	contactItem->setProperty("HomeAddress", iAddress->text());
	contactItem->setProperty("Email1Address", iEMail->text());
	contactItem->dynamicCall("Save()");

	item->setText(0, iFirstName->text());
	item->setText(1, iLastName->text());
	item->setText(2, iAddress->text());
	item->setText(3, iEMail->text());
    }
#endif
}

void ABCentralWidget::selectionChanged()
{
    iFirstName->setText("");
    iLastName->setText("");
    iAddress->setText("");
    iEMail->setText("");
}

void ABCentralWidget::itemSelected(const QModelIndex &index, int button)
{
    if (!index.isValid() || button != Qt::LeftButton)
	return;

    QAbstractItemModel *model = listView->model();
    iFirstName->setText(model->data(model->index(index.row(), 0)).toString());
    iLastName->setText(model->data(model->index(index.row(), 1)).toString());
    iAddress->setText(model->data(model->index(index.row(), 2)).toString());
    iEMail->setText(model->data(model->index(index.row(), 3)).toString());
}

void ABCentralWidget::toggleFirstName()
{
    sFirstName->setText("");

    if (cFirstName->isChecked()) {
        sFirstName->setEnabled(TRUE);
        sFirstName->setFocus();
    }
    else
        sFirstName->setEnabled(FALSE);
}

void ABCentralWidget::toggleLastName()
{
    sLastName->setText("");

    if (cLastName->isChecked()) {
        sLastName->setEnabled(TRUE);
        sLastName->setFocus();
    }
    else
        sLastName->setEnabled(FALSE);
}

void ABCentralWidget::toggleAddress()
{
    sAddress->setText("");

    if (cAddress->isChecked()) {
        sAddress->setEnabled(TRUE);
        sAddress->setFocus();
    }
    else
        sAddress->setEnabled(FALSE);
}

void ABCentralWidget::toggleEMail()
{
    sEMail->setText("");

    if (cEMail->isChecked()) {
        sEMail->setEnabled(TRUE);
        sEMail->setFocus();
    }
    else
        sEMail->setEnabled(FALSE);
}

void ABCentralWidget::findEntries()
{
#if 0
    if (!cFirstName->isChecked() &&
         !cLastName->isChecked() &&
         !cAddress->isChecked() &&
         !cEMail->isChecked()) {
        listView->clearSelection();
        return;
    }

    QListViewItemIterator it(listView);

    for (; it.current(); ++it) {
        bool select = TRUE;

        if (cFirstName->isChecked()) {
            if (select && it.current()->text(0).contains(sFirstName->text()))
                select = TRUE;
            else
                select = FALSE;
        }
        if (cLastName->isChecked()) {
            if (select && it.current()->text(1).contains(sLastName->text()))
                select = TRUE;
            else
                select = FALSE;
        }
        if (cAddress->isChecked()) {
            if (select && it.current()->text(2).contains(sAddress->text()))
                select = TRUE;
            else
                select = FALSE;
        }
        if (cEMail->isChecked()) {
            if (select && it.current()->text(3).contains(sEMail->text()))
                select = TRUE;
            else
                select = FALSE;
        }

        if (select)
            it.current()->setSelected(TRUE);
        else
            it.current()->setSelected(FALSE);
        it.current()->repaint();
    }
#endif
}
