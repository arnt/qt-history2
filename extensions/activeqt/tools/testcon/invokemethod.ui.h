/****************************************************************************
** $Id: $
**
** Copyright (C) 2001-2002 Trolltech AS.  All rights reserved.
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
/*
    const QMetaData *data = mo->slot( mo->findSlot( method, FALSE ), TRUE );
    const QUMethod *slot = data ? data->method : 0;
    if ( !slot )
	return;

    for ( int p = slot->count-1; p >= 0; --p ) {
	const QUParameter *param = slot->parameters + p;
	if ( !param )
	    continue;
	QListViewItem *item = 0;
	if ( param->inOut != QUParameter::Out ) {
	    item = new QListViewItem( listParameters );
	    QString pname = param->name ? param->name : "<unnamed>";
	    item->setText( 0, pname );
	}
	QString ptype;
	if ( !param->type ) {
	    ptype = "<unknown type>";
	} else if ( QUType::isEqual( param->type, &static_QUType_ptr ) ) {
	    ptype = (const char*)param->typeExtra;
	} else if ( QUType::isEqual( param->type, &static_QUType_varptr ) ) {
	    ptype = QVariant::typeToName( (QVariant::Type)*(char*)param->typeExtra );
	} else if ( QUType::isEqual( param->type, &static_QUType_QVariant ) ) {
	    ptype = param->type->desc();
	} else if ( QUType::isEqual( param->type, &static_QUType_enum ) ) {
	    QUEnum *uEnum = (QUEnum*)param->typeExtra;
	    ptype = uEnum->name;
	} else {
	    ptype = param->type->desc();
	}
	if ( !item ) {
	    editReturn->setText( ptype );
	} else {
	    item->setText( 1, ptype );
	}
    }

    if ( slot->count ) {
	listParameters->setCurrentItem( listParameters->firstChild() );
    }
*/
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
