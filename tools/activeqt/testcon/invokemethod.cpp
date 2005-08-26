/****************************************************************************
**
** Copyright (C) 2001-$THISYEAR$ Trolltech AS.  All rights reserved.
**
** This file is part of an example program for the ActiveQt integration.
** This example program may be used, distributed and modified without 
** limitation.
**
*****************************************************************************/

#include "invokemethod.h"

#include <ActiveQt>

InvokeMethod::InvokeMethod(QWidget *parent)
: QDialog(parent), activex(0)
{
    setupUi(this);

    listParameters->setColumnCount(3);
    listParameters->headerItem()->setText(0, "Parameter");
    listParameters->headerItem()->setText(1, "Type");
    listParameters->headerItem()->setText(2, "Value");
}

void InvokeMethod::setControl(QAxBase *ax)
{
    activex = ax;
    bool hasControl = activex && !activex->isNull();
    labelMethods->setEnabled(hasControl);
    comboMethods->setEnabled(hasControl);
    buttonInvoke->setEnabled(hasControl);
    boxParameters->setEnabled(hasControl);
    
    comboMethods->clear();
    listParameters->clear();
    
    if (!hasControl) {
	editValue->clear();
	return;
    }

    const QMetaObject *mo = activex->metaObject();
    if (mo->methodCount()) {
	for (int i = mo->methodOffset(); i < mo->methodCount(); ++i) {
	    const QMetaMethod method = mo->method(i);
            if (method.methodType() == QMetaMethod::Slot)
	        comboMethods->addItem(method.signature());
	}
        comboMethods->model()->sort(0);

	on_comboMethods_activated(comboMethods->currentText());
    }
}

void InvokeMethod::on_buttonInvoke_clicked()
{
    if (!activex)
	return;

    on_buttonSet_clicked();
    QString method = comboMethods->currentText();
    QList<QVariant> vars;

    int itemCount = listParameters->topLevelItemCount();
    for (int i = 0; i < itemCount; ++i) {
	QTreeWidgetItem *parameter = listParameters->topLevelItem(i);
	vars << parameter->text(2);
    }
    QVariant result = activex->dynamicCall(method.toLatin1(), vars);

    int v = 0;
    for (int i = 0; i < itemCount; ++i) {
	QTreeWidgetItem *parameter = listParameters->topLevelItem(i);
	parameter->setText(2, vars[v++].toString());
    }

    QString resString = result.toString();
    QString resType = result.typeName();
    editReturn->setText(resType + " " + resString);
}

void InvokeMethod::on_comboMethods_activated(const QString &method)
{
    if (!activex)
	return;
    listParameters->clear();

    const QMetaObject *mo = activex->metaObject();
    const QMetaMethod slot = mo->method(mo->indexOfSlot(method.toLatin1()));
    QString signature = slot.signature();
    signature = signature.mid(signature.indexOf('(') + 1);
    signature.truncate(signature.length()-1);

    QList<QByteArray> pnames = slot.parameterNames();
    QList<QByteArray> ptypes = slot.parameterTypes();

    for (int p = ptypes.count()-1; p >= 0; --p) {
	QString ptype(ptypes.at(p));
	if (ptype.isEmpty())
	    continue;
	QString pname(pnames.at(p));
	if (pname.isEmpty())
	    pname = QString("<unnamed %1>").arg(p);
	QTreeWidgetItem *item = new QTreeWidgetItem(listParameters);
        item->setText(0, pname);
        item->setText(1, ptype);
    }

    if (listParameters->topLevelItemCount())
	listParameters->setCurrentItem(listParameters->topLevelItem(0));
    editReturn->setText(slot.typeName());
}

void InvokeMethod::on_listParameters_currentItemChanged(QTreeWidgetItem *item)
{
    if (!activex)
	return;
    editValue->setEnabled(item != 0);
    buttonSet->setEnabled(item != 0);
    if (!item)
	return;
    editValue->setText(item->text(2));
}

void InvokeMethod::on_buttonSet_clicked()
{
    if (!activex)
	return;
    QTreeWidgetItem *item = listParameters->currentItem();
    if (!item)
	return;
    item->setText(2, editValue->text());
}

