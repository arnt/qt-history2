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

#define PropDesignable  0x00001000
#define PropScriptable	0x00002000
#define PropStored	0x00004000
#define PropBindable	0x00008000
#define PropRequesting	0x00010000

class CheckListItem : public QCheckListItem
{
public:
    CheckListItem( QListView *parent, const QString &text )
	    : QCheckListItem( parent, text, CheckBox )
    {
	dialog = (ChangeProperties*)parent->topLevelWidget()->qt_cast( "ChangeProperties" );
    }
    
protected:
    void stateChange( bool on )
    {
	if ( dialog )
	    dialog->editRequestChanged( this );
    }

private:
    ChangeProperties *dialog;
    
};

void ChangeProperties::setControl( QActiveX *ax )
{
    activex = ax;
    bool hasControl = activex && !activex->isNull();
    tabWidget->setEnabled( hasControl );
     
    listProperties->clear();
    listEditRequests->clear();
    if ( hasControl ) {
	const QMetaObject *mo = activex->metaObject();
	const int numprops = mo->numProperties( FALSE );
	for ( int i = 0; i < numprops; ++i ) {
	    const QMetaProperty *property = mo->property( i, FALSE );
	    QListViewItem *item = new QListViewItem( listProperties );
	    item->setText( 0, property->name() );
	    item->setText( 1, property->type() );
	    QVariant var = activex->property( property->name() );
	    item->setText( 2, var.toString() );
 
	    if ( property->testFlags( PropRequesting ) ) {
		CheckListItem *check = new CheckListItem( listEditRequests, property->name() );
		check->setOn( activex->propertyWritable( property->name() ) );
	    }
	}
	listProperties->setCurrentItem( listProperties->firstChild() );
    } else {
	editValue->clear();
    }
}

void ChangeProperties::propertySelected( QListViewItem *item )
{
    editValue->setEnabled( item != 0 );
    buttonSet->setEnabled( item != 0 );
    valueLabel->setEnabled( item != 0 );
    
    if ( !item )
	return;
    
    QVariant value = activex->property( item->text(0) );
    QString prop = item->text(0);
    editValue->setText( value.toString() );
    valueLabel->setText( prop + " =" );
    
    const QMetaObject *mo = activex->metaObject();
    const QMetaProperty *property = mo->property( mo->findProperty( prop, FALSE ), FALSE );

    valueLabel->setEnabled( property->writable() );
    editValue->setEnabled( property->writable() );
    buttonSet->setEnabled( property->writable() );
}

void ChangeProperties::setValue()
{
    QListViewItem *item = listProperties->currentItem();
    if ( !item )
	return;
    QVariant value = editValue->text();
    QString prop = item->text(0);
    activex->setProperty( prop, value );
    setControl( activex );
    listProperties->setCurrentItem( listProperties->findItem( prop, 0 ) );
}


void ChangeProperties::init()
{
}

void ChangeProperties::editRequestChanged( QCheckListItem *item )
{
    if ( !item )
	return;
    QString property = item->text();
    activex->setPropertyWritable( property.latin1(), item->isOn() );
}
