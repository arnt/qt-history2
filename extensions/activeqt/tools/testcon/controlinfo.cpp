/****************************************************************************
**
** Copyright (C) 2001-$THISYEAR$ Trolltech AS.  All rights reserved.
**
** This file is part of an example program for the ActiveQt integration.
** This example program may be used, distributed and modified without 
** limitation.
**
*****************************************************************************/

#include "controlinfo.h"

#include <QtGui>

ControlInfo::ControlInfo(QWidget *parent)
: QDialog(parent)
{
    setupUi(this);

    listInfo->setColumnCount(2);
    listInfo->headerItem()->setText(0, "Item");
    listInfo->headerItem()->setText(1, "Details");
}

void ControlInfo::setControl(QWidget *activex)
{
    listInfo->clear();

    const QMetaObject *mo = activex->metaObject();
    QTreeWidgetItem *group = new QTreeWidgetItem(listInfo);
    group->setText(0, "Class Info");
    group->setText(1, QString::number(mo->classInfoCount()));

    QTreeWidgetItem *item = 0;
    int i;
    int count;
    for (i = mo->classInfoOffset(); i < mo->classInfoCount(); ++i) {
	const QMetaClassInfo info = mo->classInfo(i);
	item = new QTreeWidgetItem(group);
        item->setText(0, info.name());
        item->setText(1, info.value());
    }
    group = new QTreeWidgetItem(listInfo);
    group->setText(0, "Signals");

    count = 0;
    for (i = mo->methodOffset(); i < mo->methodCount(); ++i) {
	const QMetaMethod method = mo->method(i);
        if (method.methodType() == QMetaMethod::Signal) {
            ++count;
	    item = new QTreeWidgetItem(group);
            item->setText(0, method.signature());
        }
    }
    group->setText(1, QString::number(count));

    group = new QTreeWidgetItem(listInfo);
    group->setText(0, "Slots");

    count = 0;
    for (i = mo->methodOffset(); i < mo->methodCount(); ++i) {
	const QMetaMethod method = mo->method(i);
        if (method.methodType() == QMetaMethod::Slot) {
            ++count;
	    item = new QTreeWidgetItem(group);
            item->setText(0, method.signature());
        }
    }
    group->setText(1, QString::number(count));

    group = new QTreeWidgetItem(listInfo);
    group->setText(0, "Properties");
    group->setText(1, QString::number(mo->propertyCount()));

    for (i = mo->propertyOffset(); i < mo->propertyCount(); ++i) {
	const QMetaProperty property = mo->property(i);
	item = new QTreeWidgetItem(group);
        item->setText(0, property.name());
        item->setText(1, property.typeName());
    }
}
