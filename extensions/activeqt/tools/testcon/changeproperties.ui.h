/****************************************************************************
**
** Copyright (C) 2001-$THISYEAR$ Trolltech AS.  All rights reserved.
**
** This file is part of an example program for the ActiveQt integration.
** This example program may be used, distributed and modified without 
** limitation.
**
*****************************************************************************/

#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qpixmap.h>
#include <qregexp.h>
#include <q3listview.h>

#if 0

class CheckListItem : public Q3CheckListItem
{
public:
    CheckListItem( Q3ListView *parent, const QString &text )
	    : Q3CheckListItem( parent, text, CheckBox )
    {
	dialog = (ChangeProperties*)parent->window()->qt_metacast( "ChangeProperties" );
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

#endif

void ChangeProperties::setControl( QAxWidget *ax )
{
    activex = ax;
    updateProperties();
}

void ChangeProperties::propertySelected( Q3ListViewItem *item )
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
    const QMetaProperty property = mo->property(mo->indexOfProperty(prop.latin1()));

    valueLabel->setEnabled( property.isWritable() );
    editValue->setEnabled( property.isWritable() );
    buttonSet->setEnabled( property.isWritable() );
}

void ChangeProperties::setValue()
{
    Q3ListViewItem *item = listProperties->currentItem();
    if ( !item )
	return;
    
    QString prop = item->text(0);
    QVariant value = activex->property(prop.latin1());
    QVariant::Type type = value.type();
    if ( !value.isValid() ) {
	const QMetaObject *mo = activex->metaObject();
	const QMetaProperty property = mo->property(mo->indexOfProperty(prop.latin1()));
	type = QVariant::nameToType( property.typeName() );
    }
    switch ( type ) {
    case QVariant::Color:
	{
	    QColor col;
	    col.setNamedColor( editValue->text() );
	    if ( col.isValid() ) {
		value = qVariant(col);
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
		value = qVariant(fnt);
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

	    value = qVariant(pm);
	}
	break;
    case QVariant::Bool:
	{
	    QString txt = editValue->text().toLower();
	    value = QVariant(txt != "0" && txt != "false" );
	}
	break;
    case QVariant::List:
	{
	    QStringList txtList = editValue->text().split(QRegExp( "[,;]" ));
	    QList<QVariant> varList;
	    for (int i = 0; i < txtList.count(); ++i) {
		QVariant svar(txtList.at(i));
		QString str = svar.toString();
		str = str.trimmed();
		bool ok;
		int n = str.toInt( &ok );
		if ( ok ) {
		    varList << n;
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
 
    activex->setProperty(prop.latin1(), value);
    setControl( activex );
    listProperties->setCurrentItem( listProperties->findItem( prop, 0 ) );
}


void ChangeProperties::init()
{
    activex = 0;
}

void ChangeProperties::editRequestChanged( Q3CheckListItem *item )
{
    if ( !item )
	return;
    /*
    QString property = item->text();
    activex->setPropertyWritable(property.latin1(), item->isOn());
    */
}


void ChangeProperties::updateProperties()
{
    bool hasControl = activex && !activex->isNull();
    tabWidget->setEnabled( hasControl );
    
    listProperties->clear();
    listEditRequests->clear();
    if ( hasControl ) {
	const QMetaObject *mo = activex->metaObject();
	const int numprops = mo->propertyCount();
	for ( int i = mo->propertyOffset(); i < numprops; ++i ) {
	    const QMetaProperty property = mo->property(i);
	    Q3ListViewItem *item = new Q3ListViewItem(listProperties);
	    item->setText(0, property.name());
	    item->setText(1, property.typeName());
	    QVariant var = activex->property(property.name());
	    
	    switch ( var.type() ) {
	    case QVariant::Color:
		{
		    QColor col = qVariant_to<QColor>(var);
		    item->setText( 2, col.name() );
		}
		break;
	    case QVariant::Font:
		{
		    QFont fnt = qVariant_to<QFont>(var);
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
		    QPixmap pm = qVariant_to<QPixmap>(var);
		    item->setPixmap( 2, pm );
		}
		break;
	    case QVariant::List:
		{
		    QList<QVariant> varList = var.toList();
		    QStringList strList;
		    for (int i = 0; i < varList.count(); ++i) {
			QVariant var = varList.at(i);
			strList << var.toString();
		    }
		    item->setText( 2, strList.join( ", " ) );
		}
		break;
	    case QVariant::Int:
		if (property.isEnumType()) {
		    const QMetaEnum enumerator = mo->enumerator(mo->indexOfEnumerator(property.typeName()));
		    item->setText(2, enumerator.valueToKey(var.toInt()));
		    break;
		}
		//FALLTHROUGH
	    default:
		item->setText( 2, var.toString() );
		break;
	    }
 /*
	    if ( property.testFlags( PropRequesting ) ) {
		CheckListItem *check = new CheckListItem( listEditRequests, property->name() );
		check->setOn( activex->propertyWritable( property->name() ) );
	    }
*/
	}
	listProperties->setCurrentItem( listProperties->firstChild() );
    } else {
	editValue->clear();
    }
}
