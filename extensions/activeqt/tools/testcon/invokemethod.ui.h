/****************************************************************************
** $Id: $
**
** Copyright (C) 2001-$THISYEAR$ Trolltech AS.  All rights reserved.
**
** This file is part of an example program for the ActiveQt integration.
** This example program may be used, distributed and modified without 
** limitation.
**
*****************************************************************************/

#include <qlistbox.h>

void InvokeMethod::invoke()
{
    if (!activex)
	return;

    setValue();
    QString method = comboMethods->currentText();
    QList<QVariant> vars;

#if 0
    QListViewItemIterator it(listParameters);
    while (it.current()) {
	QListViewItem *parameter = it.current();
	++it;
	vars << parameter->text(2);
    }
    QVariant result = activex->dynamicCall(method, vars);
    it = QListViewItemIterator(listParameters);
    int v = 0;
    while (it.current()) {
	QListViewItem *parameter = it.current();
	++it;
	parameter->setText(2, vars[v++].toString());
    }

    QString resString = result.toString();
    QString resType = result.typeName();
    editReturn->setText(resType + " " + resString);
#endif
}

void InvokeMethod::methodSelected(const QString &method)
{
    if (!activex)
	return;
    listParameters->clear();
    listParameters->setSorting(-1);
    const QMetaObject *mo = activex->metaObject();
    const QMetaMember slot = mo->member(mo->indexOfSlot(method.latin1()));
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
	Q3ListViewItem *item = new Q3ListViewItem(listParameters, pname, ptype);
    }

    if (listParameters->firstChild())
	listParameters->setCurrentItem(listParameters->firstChild());
    editReturn->setText(slot.typeName());
}

void InvokeMethod::parameterSelected(Q3ListViewItem *item)
{
    if (!activex)
	return;
    editValue->setEnabled(item !=  0);
    buttonSet->setEnabled(item != 0 );
    if (!item)
	return;
    editValue->setText(item->text(2));
}

void InvokeMethod::setValue()
{
    if (!activex)
	return;
    Q3ListViewItem *item = listParameters->currentItem();
    if (!item)
	return;
    item->setText(2, editValue->text());
}

void InvokeMethod::init()
{
    setControl(0);
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
    if (mo->memberCount()) {
	for (int i = mo->memberOffset(); i < mo->memberCount(); ++i) {
	    const QMetaMember member = mo->member(i);
            if (member.memberType() == QMetaMember::Slot)
	        comboMethods->insertItem(member.signature());
	}
#if 0
        comboMethods->listBox()->sort();
#endif
	methodSelected(comboMethods->currentText());
    }
}
