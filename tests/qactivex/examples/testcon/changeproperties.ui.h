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

#include <qmessagebox.h>

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

void ChangeProperties::setControl( QAxWidget *ax )
{
    activex = ax;
    updateProperties();
}

void ChangeProperties::propertySelected( QListViewItem *item )
{
    editValue->setEnabled( item != 0 );
    buttonSet->setEnabled( item != 0 );
    valueLabel->setEnabled( item != 0 );
    
    if ( !item )
	return;
    
    QVariant value = activex->property( item->text(0) );
    QString valueString;
    switch ( value.type() ) {
    case QVariant::Color:
	{
	    QColor col = value.toColor();
	    valueString = col.name();
	}
	break;
    case QVariant::Font:
	{
	    QFont fnt = value.toFont();
	    valueString = fnt.toString();
	}
	break;
	
    default:
	valueString =  value.toString();
	break;
    }
    editValue->setText( valueString );
    


    QString prop = item->text(0);
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
    
    QString prop = item->text(0);
    QVariant value = activex->property( prop );
    switch ( value.type() ) {
    case QVariant::Color:
	{
	    QColor col;
	    col.setNamedColor( editValue->text() );
	    if ( col.isValid() ) {
		value = col;
	    } else {
		QMessageBox::warning( this, tr("Can't parse input"), 
		                            QString( tr("Failed to create a color from %1\n"
					                "The string has to be a valid color name (e.g. 'red')\n"
							"or a RGB triple of format '#rrggbb'."
							).arg( editValue->text() ) ) );
	    }
	}
	break;
    case QVariant::Font:
	{
	    QFont fnt;
	    if ( fnt.fromString( editValue->text() ) ) {
		value = fnt;
	    } else {
		QMessageBox::warning( this, tr("Can't parse input"), 
		                            QString( tr("Failed to create a font from %1\n"
					                "The string has to have a format family,<point size> or\n"
							"family,pointsize,stylehint,weight,italic,underline,strikeout,fixedpitch,rawmode."
							).arg( editValue->text() ) ) );
	    }
	}
	break;

    default:
	value = editValue->text();
	break;
    }
 
    activex->setProperty( prop, value );
    setControl( activex );
    listProperties->setCurrentItem( listProperties->findItem( prop, 0 ) );
}


void ChangeProperties::init()
{
    activex = 0;
}

void ChangeProperties::editRequestChanged( QCheckListItem *item )
{
    if ( !item )
	return;
    QString property = item->text();
    activex->setPropertyWritable( property.latin1(), item->isOn() );
}


void ChangeProperties::updateProperties()
{
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
	    
	    switch ( var.type() ) {
	    case QVariant::Color:
		{
		    QColor col = var.toColor();
		    item->setText( 2, col.name() );
		}
		break;
	    case QVariant::Font:
		{
		    QFont fnt = var.toFont();
		    item->setText( 2, fnt.toString() );
		}
		break;

	    default:
		item->setText( 2, var.toString() );
		break;
	    }
	    
 
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
