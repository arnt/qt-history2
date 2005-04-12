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

#include <QtGui>

#include "addressview.h"
#include "msoutl.h"

class AddressBookModel : public QAbstractListModel
{
public:
    AddressBookModel(AddressView *parent);
    ~AddressBookModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QVariant data(const QModelIndex &index, int role) const;

    void changeItem(const QModelIndex &index, const QString &firstName, const QString &lastName, const QString &address, const QString &email);
    void addItem(const QString &firstName, const QString &lastName, const QString &address, const QString &email);
    void update();

private:
    Outlook::Application outlook;
    Outlook::Items * contactItems;

    mutable QHash<QModelIndex, QStringList> cache;
};

AddressBookModel::AddressBookModel(AddressView *parent)
: QAbstractListModel(parent)
{
    if (!outlook.isNull()) {
        Outlook::NameSpace session(outlook.Session());
        session.Logon();
        Outlook::MAPIFolder *folder = session.GetDefaultFolder(Outlook::olFolderContacts);
        contactItems = new Outlook::Items(folder->Items());
	connect(contactItems, SIGNAL(ItemAdd(IDispatch*)), parent, SLOT(updateOutlook()));
	connect(contactItems, SIGNAL(ItemChange(IDispatch*)), parent, SLOT(updateOutlook()));
	connect(contactItems, SIGNAL(ItemRemove()), parent, SLOT(updateOutlook()));    

        delete folder;
    }
}

AddressBookModel::~AddressBookModel()
{
    delete contactItems;

    if (!outlook.isNull())
        Outlook::NameSpace(outlook.Session()).Logoff();
}

int AddressBookModel::rowCount(const QModelIndex &) const
{
    return contactItems ? contactItems->Count() : 0;
}

int AddressBookModel::columnCount(const QModelIndex &parent) const
{
    return 4;
}

QVariant AddressBookModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
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

QVariant AddressBookModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    QStringList data;
    if (cache.contains(index)) {
        data = cache.value(index);
    } else {
        Outlook::ContactItem contact(contactItems->Item(index.row() + 1));
        data << contact.FirstName() << contact.LastName() << contact.HomeAddress() << contact.Email1Address();
        cache.insert(index, data);
    }

    if (index.column() < data.count())
        return data.at(index.column());

    return QVariant();
}

void AddressBookModel::changeItem(const QModelIndex &index, const QString &firstName, const QString &lastName, const QString &address, const QString &email)
{
    Outlook::ContactItem item(contactItems->Item(index.row() + 1));

    item.SetFirstName(firstName);
    item.SetLastName(lastName);
    item.SetHomeAddress(address);
    item.SetEmail1Address(email);

    item.Save();

    cache.take(index);
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
    QGridLayout *mainGrid = new QGridLayout(this);

    QLabel *liFirstName = new QLabel("First &Name", this);
    liFirstName->resize(liFirstName->sizeHint());
    mainGrid->addWidget(liFirstName, 0, 0);

    QLabel *liLastName = new QLabel("&Last Name", this);
    liLastName->resize(liLastName->sizeHint());
    mainGrid->addWidget(liLastName, 0, 1);

    QLabel *liAddress = new QLabel("Add&ress", this);
    liAddress->resize(liAddress->sizeHint());
    mainGrid->addWidget(liAddress, 0, 2);

    QLabel *liEMail = new QLabel("&E-Mail", this);
    liEMail->resize(liEMail->sizeHint());
    mainGrid->addWidget(liEMail, 0, 3);

    add = new QPushButton("A&dd", this);
    add->resize(add->sizeHint());
    mainGrid->addWidget(add, 0, 4);
    connect(add, SIGNAL(clicked()), this, SLOT(addEntry()));

    iFirstName = new QLineEdit(this);
    iFirstName->resize(iFirstName->sizeHint());
    mainGrid->addWidget(iFirstName, 1, 0);
    liFirstName->setBuddy(iFirstName);

    iLastName = new QLineEdit(this);
    iLastName->resize(iLastName->sizeHint());
    mainGrid->addWidget(iLastName, 1, 1);
    liLastName->setBuddy(iLastName);

    iAddress = new QLineEdit(this);
    iAddress->resize(iAddress->sizeHint());
    mainGrid->addWidget(iAddress, 1, 2);
    liAddress->setBuddy(iAddress);

    iEMail = new QLineEdit(this);
    iEMail->resize(iEMail->sizeHint());
    mainGrid->addWidget(iEMail, 1, 3);
    liEMail->setBuddy(iEMail);

    change = new QPushButton("&Change", this);
    change->resize(change->sizeHint());
    mainGrid->addWidget(change, 1, 4);
    connect(change, SIGNAL(clicked()), this, SLOT(changeEntry()));

    treeView = new QTreeView(this);
    treeView->setSelectionMode(QTreeView::SingleSelection);
    treeView->setRootIsDecorated(false);

    model = new AddressBookModel(this);
    treeView->setModel(model);

    connect(treeView->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)), this, SLOT(itemSelected(QModelIndex)));

    mainGrid->addWidget(treeView, 2, 0, 1, 5);
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

void AddressView::itemSelected(const QModelIndex &index)
{
    if (!index.isValid())
	return;

    QAbstractItemModel *model = treeView->model();
    iFirstName->setText(model->data(model->index(index.row(), 0)).toString());
    iLastName->setText(model->data(model->index(index.row(), 1)).toString());
    iAddress->setText(model->data(model->index(index.row(), 2)).toString());
    iEMail->setText(model->data(model->index(index.row(), 3)).toString());
}
