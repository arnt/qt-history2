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
    
    QCString method = comboMethods->currentText();
    QVariant var[8];
    int p = 0;
    QListViewItemIterator it( listParameters );
    while ( it.current() ) {
	QListViewItem *parameter = it.current();
	++it;
	var[p] = parameter->text(2);
	++p;
    }
    activex->dynamicCall( method, var[0], var[1], var[2], var[3], var[4], var[5], var[6], var[7] );
}

void InvokeMethod::methodSelected( const QString &method )
{
    if ( !activex )
	return;
    
    listParameters->clear();
    const QMetaObject *mo = activex->metaObject();
    const QMetaData *data = mo->slot( mo->findSlot( method, FALSE ), TRUE );
    const QUMethod *slot = data ? data->method : 0;
    if ( !slot )
	return;
    
    for ( int p = 0; p < slot->count; ++p ) {
	const QUParameter *param = slot->parameters + p;
	if ( !param || (param->inOut == QUParameter::Out ) )
	    continue;
	QListViewItem *item = new QListViewItem( listParameters );
	QString pname = param->name ? param->name : "<unnamed>";
	item->setText( 0, pname );
	QString ptype;
	if ( param->type && !QUType::isEqual( param->type, &static_QUType_ptr ) ) {
	    ptype = param->type->desc();
	} else if ( param->type ) {
	    ptype = (const char*)param->typeExtra;
	} else {
	    ptype = "<unknown type>";
	}
	item->setText( 1, ptype );
    }
    if ( slot->count ) {
	listParameters->setCurrentItem( listParameters->firstChild() );
    }
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

void InvokeMethod::setControl( QAxWidget *ax )
{
    activex = ax;
    bool hasControl = activex && !activex->isNull();
    labelMethods->setEnabled( hasControl );
    comboMethods->setEnabled( hasControl );
    buttonInvoke->setEnabled( hasControl );
    boxParameters->setEnabled( hasControl );
    
    comboMethods->clear();
    listParameters->clear();
    
    if ( hasControl ) {
	const QMetaObject *mo = activex->metaObject();
	const int nummethods = mo->numSlots( FALSE );
	for ( int i = 0; i < nummethods; ++i ) {
	    const QMetaData *data = mo->slot( i, FALSE );
	    comboMethods->insertItem( data->name );
	}
	comboMethods->listBox()->sort();
	if ( nummethods )
	    methodSelected( comboMethods->currentText() );
    } else {
	editValue->clear();
    }
}
