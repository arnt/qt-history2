/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
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
    nameLabel = new QLabel(tr("Name:"));
    addressLabel = new QLabel(tr("Address:"));
    addressLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    nameEdit = new QLineEdit;
    addressEdit = new QTextEdit;

    offersCheckBox = new QCheckBox(tr("Send information about products and "
                                      "special offers"));

    setupItemsTable();

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(verify()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(nameLabel, 0, 0);
    mainLayout->addWidget(nameEdit, 0, 1);
    mainLayout->addWidget(addressLabel, 1, 0);
    mainLayout->addWidget(addressEdit, 1, 1);
    mainLayout->addWidget(itemsTable, 0, 2, 2, 1);
    mainLayout->addWidget(offersCheckBox, 2, 1, 1, 2);
    mainLayout->addWidget(buttonBox, 3, 0, 1, 3);
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

QList<QPair<QString, int> > DetailsDialog::orderItems()
{
    QList<QPair<QString, int> > orderList;

    for (int row = 0; row < items.count(); ++row) {
        QPair<QString, int> item;
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
