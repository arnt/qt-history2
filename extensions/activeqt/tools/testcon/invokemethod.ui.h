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
    if ( !activex )
	return;
    
    setValue();
    QString method = comboMethods->currentText();
    QList<QVariant> vars;
    QListViewItemIterator it( listParameters );
    while ( it.current() ) {
	QListViewItem *parameter = it.current();
	++it;
	vars << parameter->text(2);
    }
    QVariant result = activex->dynamicCall(method, vars);
    it = QListViewItemIterator( listParameters );
    int v = 0;
    while ( it.current() ) {
	QListViewItem *parameter = it.current();
	++it;
	parameter->setText( 2, vars[v++].toString() );
    }

    QString resString = result.toString();
    QString resType = result.typeName();
    editReturn->setText( resType + " " + resString );
}

void InvokeMethod::methodSelected( const QString &method )
{
    if ( !activex )
	return;
    listParameters->clear();
    listParameters->setSorting( -1 );
    const QMetaObject *mo = activex->metaObject();
    const QMetaMember slot = mo->slot(mo->indexOfSlot(method.latin1()));
    QString signature = slot.signature();
    signature = signature.mid(signature.indexOf('(') + 1);
    signature.truncate(signature.length()-1);

    QStringList pnames = QString(slot.parameters()).split(',');
    QStringList ptypes = signature.split(",");

    for (int p = ptypes.count()-1; p >= 0; --p ) {
	QString ptype(ptypes.at(p));
	if (ptype.isEmpty())
	    continue;
	QString pname(pnames.at(p));
	if (pname.isEmpty())
	    pname = QString("<unnamed %1>").arg(p);
	QListViewItem *item = new QListViewItem(listParameters, pname, ptype);
    }

    if (listParameters->firstChild())
	listParameters->setCurrentItem( listParameters->firstChild() );
    editReturn->setText(slot.type());
}

void InvokeMethod::parameterSelected( QListViewItem *item )
{
    if ( !activex )
	return;
    editValue->setEnabled( item !=  0 );
    buttonSet->setEnabled( item != 0  );
    if ( !item )
	return;
    editValue->setText( item->text( 2 ) );
}

void InvokeMethod::setValue()
{
    if ( !activex )
	return;
    QListViewItem *item = listParameters->currentItem();
    if ( !item )
	return;
    item->setText( 2, editValue->text() );
}

void InvokeMethod::init()
{
    setControl( 0 );
}

void InvokeMethod::setControl( QAxBase *ax )
{
    activex = ax;
    bool hasControl = activex && !activex->isNull();
    labelMethods->setEnabled( hasControl );
    comboMethods->setEnabled( hasControl );
    buttonInvoke->setEnabled( hasControl );
    boxParameters->setEnabled( hasControl );
    
    comboMethods->clear();
    listParameters->clear();
    
    if ( !hasControl ) {
	editValue->clear();
	return;
    }

    const QMetaObject *mo = activex->metaObject();
    if (mo->slotCount()) {
	for ( int i = mo->slotOffset(); i < mo->slotCount(); ++i ) {
	    const QMetaMember slot = mo->slot(i);
	    comboMethods->insertItem(slot.signature());
	}
	comboMethods->listBox()->sort();
	methodSelected( comboMethods->currentText() );
    }
}
