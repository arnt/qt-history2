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

#include "centralwidget.h"

#include <qcache.h>
#include <qcheckbox.h>
#include <qfile.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qtextstream.h>
#include <qtreeview.h>
#include <qwidget.h>

#include "msoutl.h"

uint qHash(const QModelIndex &key)
{
    return (key.row()+1) * (key.column()+1) + key.column();
}

class AddressBookModel : public QAbstractListModel
{
public:
    AddressBookModel(AddressView *parent);
    ~AddressBookModel();

    int rowCount() const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    void changeItem(const QModelIndex &index, const QString &firstName, const QString &lastName, const QString &address, const QString &email);
    void addItem(const QString &firstName, const QString &lastName, const QString &address, const QString &email);
    void update();

private:
    Outlook::Application outlook;
    Outlook::Items * contactItems;

    mutable QCache<QModelIndex, QStringList> cache;
};

AddressBookModel::AddressBookModel(AddressView *parent)
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

int AddressBookModel::columnCount(const QModelIndex &parent) const
{
    return 4;
}

QVariant AddressBookModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != DisplayRole)
        return QVariant();

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

QVariant AddressBookModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != DisplayRole)
        return QVariant();

    switch (section) {
    case 0:
        return "First Name";
    case 1:
        return "Last Name";
    case 2:
        return "Address";
    case 3:
        return "Email";
    default:
        break;
    }

    return QVariant();
}

void AddressBookModel::changeItem(const QModelIndex &index, const QString &firstName, const QString &lastName, const QString &address, const QString &email)
{
    Outlook::ContactItem item(contactItems->Item(index.row() + 1));
    Q_ASSERT(!item.isNull());

    QString test(item.FirstName());
    item.SetFirstName(firstName);
    test = item.FirstName();

    test = item.LastName();
    item.SetLastName(lastName);
    test = item.LastName();

    test = item.HomeAddress();
    item.SetHomeAddress(address);
    test = item.HomeAddress();

    item.SetEmail1Address(email);

    item.Save();

    cache.take(index);

    emit dataChanged(index, index);
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

    emit reset();
}


AddressView::AddressView(QWidget *parent)
: QWidget(parent)
{
    mainGrid = new QGridLayout(this);

    setupTabWidget();
    setupTreeView();

    mainGrid->setRowStretch(0, 0);
    mainGrid->setRowStretch(1, 1);
}

void AddressView::setupTabWidget()
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

void AddressView::setupTreeView()
{
    treeView = new QTreeView(this);

    treeView->setSelectionMode(QListView::SingleSelection);
    model = new AddressBookModel(this);
    treeView->setModel(model);

    connect(treeView, SIGNAL(clicked(QModelIndex, int)), this, SLOT(itemSelected(QModelIndex, int)));

    mainGrid->addWidget(treeView, 1, 0);
}

void AddressView::updateOutlook()
{
    model->update();
}

void AddressView::addEntry()
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

void AddressView::changeEntry()
{
    QModelIndex current = treeView->currentIndex();

    if (current.isValid())
        model->changeItem(current, iFirstName->text(), iLastName->text(), iAddress->text(), iEMail->text());
}

void AddressView::selectionChanged()
{
    iFirstName->setText("");
    iLastName->setText("");
    iAddress->setText("");
    iEMail->setText("");
}

void AddressView::itemSelected(const QModelIndex &index, int button)
{
    if (!index.isValid() || button != Qt::LeftButton)
	return;

    QAbstractItemModel *model = treeView->model();
    iFirstName->setText(model->data(model->index(index.row(), 0)).toString());
    iLastName->setText(model->data(model->index(index.row(), 1)).toString());
    iAddress->setText(model->data(model->index(index.row(), 2)).toString());
    iEMail->setText(model->data(model->index(index.row(), 3)).toString());
}

void AddressView::toggleFirstName()
{
    sFirstName->setText("");

    if (cFirstName->isChecked()) {
        sFirstName->setEnabled(TRUE);
        sFirstName->setFocus();
    }
    else
        sFirstName->setEnabled(FALSE);
}

void AddressView::toggleLastName()
{
    sLastName->setText("");

    if (cLastName->isChecked()) {
        sLastName->setEnabled(TRUE);
        sLastName->setFocus();
    }
    else
        sLastName->setEnabled(FALSE);
}

void AddressView::toggleAddress()
{
    sAddress->setText("");

    if (cAddress->isChecked()) {
        sAddress->setEnabled(TRUE);
        sAddress->setFocus();
    }
    else
        sAddress->setEnabled(FALSE);
}

void AddressView::toggleEMail()
{
    sEMail->setText("");

    if (cEMail->isChecked()) {
        sEMail->setEnabled(TRUE);
        sEMail->setFocus();
    }
    else
        sEMail->setEnabled(FALSE);
}

void AddressView::findEntries()
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
