/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "propertydialog.h"

#include <QHeaderView>
#include <QLayout>
#include <QDebug>

PropertyDialog::PropertyDialog(QWidget *parent, Qt::WindowFlags f)
    : QDialog(parent, f)
{
    buttonBox = new QDialogButtonBox;
    propertyTable = new QTableWidget;
    label = new QLabel;

    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    propertyTable->setColumnCount(2);
    const QStringList labels = QStringList() << "Name" << "Value";
    propertyTable->setHorizontalHeaderLabels(labels);
    propertyTable->horizontalHeader()->setStretchLastSection(true);
    propertyTable->setEditTriggers(QAbstractItemView::AllEditTriggers);

    connect(buttonBox, SIGNAL(accepted()), SLOT(accept()), Qt::QueuedConnection);
    connect(buttonBox, SIGNAL(rejected()), SLOT(reject()), Qt::QueuedConnection);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(label);
    layout->addWidget(propertyTable);
    layout->addWidget(buttonBox);
}

void PropertyDialog::setInfo(const QString &caption)
{
    label->setText(caption);
}

void PropertyDialog::addProperty(const QString &aname, QVariant::Type type)
{
    int rowCount = propertyTable->rowCount();
    propertyTable->setRowCount(rowCount + 1);

    QString name = aname;
    if (name.isEmpty())
        name = "argument " + QString::number(rowCount + 1);
    QTableWidgetItem *nameItem = new QTableWidgetItem(name + " (" + QVariant::typeToName(type) + ")");
    nameItem->setFlags(nameItem->flags() &
            ~(Qt::ItemIsEditable | Qt::ItemIsSelectable));
    propertyTable->setItem(rowCount, 0, nameItem);

    QTableWidgetItem *valueItem = new QTableWidgetItem;
    valueItem->setData(Qt::DisplayRole, QVariant(type));
    propertyTable->setItem(rowCount, 1, valueItem);
}

int PropertyDialog::exec()
{
    propertyTable->resizeColumnToContents(0);
    propertyTable->setFocus();
    propertyTable->setCurrentCell(0, 1);
    return QDialog::exec();
}

QList<QVariant> PropertyDialog::values() const
{
    QList<QVariant> result;

    for (int i = 0; i < propertyTable->rowCount(); ++i)
        result << propertyTable->item(i, 1)->data(Qt::EditRole);

    return result;
}

