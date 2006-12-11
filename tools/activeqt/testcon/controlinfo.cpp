/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_COMMERCIAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "controlinfo.h"

#include <QtGui>

ControlInfo::ControlInfo(QWidget *parent)
: QDialog(parent)
{
    setupUi(this);

    listInfo->setColumnCount(2);
    listInfo->headerItem()->setText(0, tr("Item"));
    listInfo->headerItem()->setText(1, tr("Details"));
}

void ControlInfo::setControl(QWidget *activex)
{
    listInfo->clear();

    const QMetaObject *mo = activex->metaObject();
    QTreeWidgetItem *group = new QTreeWidgetItem(listInfo);
    group->setText(0, tr("Class Info"));
    group->setText(1, QString::number(mo->classInfoCount()));

    QTreeWidgetItem *item = 0;
    int i;
    int count;
    for (i = mo->classInfoOffset(); i < mo->classInfoCount(); ++i) {
	const QMetaClassInfo info = mo->classInfo(i);
	item = new QTreeWidgetItem(group);
        item->setText(0, QString::fromLatin1(info.name()));
        item->setText(1, QString::fromLatin1(info.value()));
    }
    group = new QTreeWidgetItem(listInfo);
    group->setText(0, tr("Signals"));

    count = 0;
    for (i = mo->methodOffset(); i < mo->methodCount(); ++i) {
	const QMetaMethod method = mo->method(i);
        if (method.methodType() == QMetaMethod::Signal) {
            ++count;
	    item = new QTreeWidgetItem(group);
            item->setText(0, QString::fromLatin1(method.signature()));
        }
    }
    group->setText(1, QString::number(count));

    group = new QTreeWidgetItem(listInfo);
    group->setText(0, tr("Slots"));

    count = 0;
    for (i = mo->methodOffset(); i < mo->methodCount(); ++i) {
	const QMetaMethod method = mo->method(i);
        if (method.methodType() == QMetaMethod::Slot) {
            ++count;
	    item = new QTreeWidgetItem(group);
            item->setText(0, QString::fromLatin1(method.signature()));
        }
    }
    group->setText(1, QString::number(count));

    group = new QTreeWidgetItem(listInfo);
    group->setText(0, tr("Properties"));
    group->setText(1, QString::number(mo->propertyCount()));

    for (i = mo->propertyOffset(); i < mo->propertyCount(); ++i) {
	const QMetaProperty property = mo->property(i);
	item = new QTreeWidgetItem(group);
        item->setText(0, QString::fromLatin1(property.name()));
        item->setText(1, QString::fromLatin1(property.typeName()));
        if (!property.isDesignable()) {
            item->setTextColor(0, Qt::gray);
            item->setTextColor(1, Qt::gray);
        }
    }
}
