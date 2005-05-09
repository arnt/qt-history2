/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
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

#include "detailsdialog.h"

DetailsDialog::DetailsDialog(const QString &title, QWidget *parent)
    : QDialog(parent)
{
    QLabel *nameLabel = new QLabel(tr("Name:"));
    QLabel *addressLabel = new QLabel(tr("Address:"));

    nameEdit = new QLineEdit;
    addressEdit = new QTextEdit;
    addressEdit->setPlainText("");
    offersCheckBox = new QCheckBox(tr("Send offers:"));

    setupItemsTable();

    QPushButton *okButton = new QPushButton(tr("OK"));
    QPushButton *cancelButton = new QPushButton(tr("Cancel"));
    okButton->setDefault(true);

    connect(okButton, SIGNAL(clicked()), this, SLOT(verify()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

    QGridLayout *detailsLayout = new QGridLayout;
    detailsLayout->addWidget(nameLabel, 0, 0);
    detailsLayout->addWidget(nameEdit, 0, 1);
    detailsLayout->addWidget(addressLabel, 1, 0);
    detailsLayout->addWidget(addressEdit, 1, 1);
    detailsLayout->addWidget(itemsTable, 0, 2, 2, 2);
    detailsLayout->addWidget(offersCheckBox, 2, 1, 1, 4);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(detailsLayout);
    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);

    setWindowTitle(title);
}

void DetailsDialog::setupItemsTable()
{
    items << tr("T-shirt") << tr("Badge") << tr("Reference book")
          << tr("Coffee cup");

    itemsTable = new QTableWidget(items.count(), 2);

    for (int row = 0; row < items.count(); ++row) {
        QTableWidgetItem *name = new QTableWidgetItem(items[row]);
        name->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        itemsTable->setItem(row, 0, name);
        QTableWidgetItem *quantity = new QTableWidgetItem("1");
        itemsTable->setItem(row, 1, quantity);
    }
}

QList<QPair<QString,int> > DetailsDialog::orderItems()
{
    QList<QPair<QString,int> > orderList;

    for (int row = 0; row < items.count(); ++row) {
        QPair<QString,int> item;
        item.first = itemsTable->item(row, 0)->text();
        int quantity = itemsTable->item(row, 1)->data(Qt::DisplayRole).toInt();
        item.second = qMax(0, quantity);
        orderList.append(item);
    }

    return orderList;
}

QString DetailsDialog::senderName() const
{
    return nameEdit->text();
}

QString DetailsDialog::senderAddress() const
{
    return addressEdit->toPlainText();
}

bool DetailsDialog::sendOffers()
{
    return offersCheckBox->isChecked();
}

void DetailsDialog::verify()
{
    if (!nameEdit->text().isEmpty() && !addressEdit->toPlainText().isEmpty()) {
        accept();
        return;
    }

    int answer = QMessageBox::warning(this, tr("Incomplete Form"),
        tr("The form does not contain all the necessary information.\n"
           "Do you want to discard it?"),
        QMessageBox::Yes, QMessageBox::No);

    if (answer == QMessageBox::Yes)
        reject();
}
