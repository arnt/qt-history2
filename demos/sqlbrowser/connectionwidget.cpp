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

#include "connectionwidget.h"

#include <qdebug.h>
#include <qsqldatabase.h>
#include <qtreewidget.h>
#include <qheaderview.h>
#include <qlayout.h>

ConnectionWidget::ConnectionWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    tree = new QTreeWidget(this);
    tree->setObjectName(QLatin1String("tree"));
    tree->header()->setResizeMode(QHeaderView::Stretch);
    tree->setHeaderLabels(tr("database"));

    layout->addWidget(tree);

    QMetaObject::connectSlotsByName(this);
}

ConnectionWidget::~ConnectionWidget()
{
}

static QString qDBCaption(const QSqlDatabase &db)
{
    QString nm = db.driverName();
    nm.append(QLatin1Char(':'));
    if (!db.userName().isEmpty())
        nm.append(db.userName()).append(QLatin1Char('@'));
    nm.append(db.databaseName());
    return nm;
}

void ConnectionWidget::refresh()
{
    tree->clear();
    QStringList connectionNames = QSqlDatabase::connectionNames();

    bool gotActiveDb = false;
    for (int i = 0; i < connectionNames.count(); ++i) {
        QTreeWidgetItem *root = new QTreeWidgetItem(tree);
        QSqlDatabase db = QSqlDatabase::database(connectionNames.at(i), false);
        qDebug() << db;
        root->setText(0, qDBCaption(db));
        if (connectionNames.at(i) == activeDb) {
            gotActiveDb = true;
            setActive(root);
        }
        if (db.isOpen()) {
            QStringList tables = db.tables();
            for (int t = 0; t < tables.count(); ++t) {
                QTreeWidgetItem *table = new QTreeWidgetItem(root);
                table->setText(0, tables.at(t));
            }
        }
    }
    if (!gotActiveDb) {
        activeDb = connectionNames.value(0);
        setActive(tree->topLevelItem(0));
    }

    tree->doItemsLayout(); // HACK
}

QSqlDatabase ConnectionWidget::currentDatabase() const
{
    return QSqlDatabase::database(activeDb);
}

void ConnectionWidget::setActive(QTreeWidgetItem *item)
{
    if (!item)
        return;

    QFont font = item->font(0);
    font.setBold(true);
    item->setFont(0, font);
}

void ConnectionWidget::on_tree_doubleClicked(QTreeWidgetItem *item, int column,
                                             Qt::ButtonState button)
{
    Q_ASSERT(item);

    if (button != Qt::LeftButton || column != 0 || !item->parent())
        return;

    emit tableActivated(item->text(0));
}

void ConnectionWidget::on_tree_returnPressed(QTreeWidgetItem *item, int column)
{
    Q_ASSERT(item);

    if (column != 0 || !item->parent())
        return;

    emit tableActivated(item->text(0));
}
