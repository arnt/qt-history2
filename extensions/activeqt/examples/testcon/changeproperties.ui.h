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
#include <qfiledialog.h>
#include <qpixmap.h>
#include <qregexp.h>

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

    editValue->setText( item->text( 2 ) );
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
    QVariant::Type type = value.type();
    if ( !value.isValid() ) {
	const QMetaObject *mo = activex->metaObject();
	const QMetaProperty *mp = mo->property( mo->findProperty( prop, TRUE ), TRUE );
	type = QVariant::nameToType( mp->type() );
    }
    switch ( type ) {
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
    case QVariant::Pixmap:
	{
	    QString fileName = editValue->text();
	    if ( fileName.isEmpty() )
		fileName = QFileDialog::getOpenFileName( QString::null, QString::null, this );
	    QPixmap pm( fileName );
	    if ( pm.isNull() )
		return;

	    value = pm;
	}
	break;
    case QVariant::Bool:
	{
	    QString txt = editValue->text().lower();
	    value = QVariant( ( txt != "0" && txt != "false" ), 23 );
	}
	break;
    case QVariant::List:
	{
	    QStringList txtList = QStringList::split( QRegExp( "[,;]" ), editValue->text() );
	    QValueList<QVariant> varList;
	    for ( QStringList::Iterator it = txtList.begin(); it != txtList.end(); ++it ) {
		QVariant svar = *it;
		QString str = svar.toString();
		str = str.stripWhiteSpace();
		bool ok;
		int i = str.toInt( &ok );
		if ( ok ) {
		    varList << i;
		    continue;
		}
		double d = str.toDouble( &ok );
		if ( ok ) {
		    varList << d;
		    continue;
		}
		varList << str;
	    }
	    value = varList;
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
	    if ( property->isEnumType() ) {
		const QMetaEnum *enumData = property->enumData;
		item->setText( 1, enumData->name );
	    } else {
		item->setText( 1, property->type() );
	    }
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
	    case QVariant::Bool:
		{
		    item->setText( 2, var.toBool() ? "true" : "false" );
		}
		break;
	    case QVariant::Pixmap:
		{
		    QPixmap pm = var.toPixmap();
		    item->setPixmap( 2, pm );
		}
		break;
	    case QVariant::List:
		{
		    QValueList<QVariant> varList = var.toList();
		    QStringList strList;
		    for ( QValueList<QVariant>::Iterator it = varList.begin(); it != varList.end(); ++it ) {
			QVariant var = *it;
			strList << var.toString();
		    }
		    item->setText( 2, strList.join( ", " ) );
		}
		break;
	    case QVariant::Int:
		if ( property->isEnumType() ) {
		    const QMetaEnum *enumData = property->enumData;
		    int val = var.toInt();
		    for ( uint j = 0; j < enumData->count; ++j ) {
			if ( enumData->items[j].value == val )
			    item->setText( 2, enumData->items[j].key );
		    }
		    break;
		}
		//FALLTHROUGH
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
