/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/


void ChangeProperties::setControl( QActiveX *ax )
{
    activex = ax;
    bool hasControl = activex && !activex->isNull();
    boxProperties->setEnabled( hasControl );
    
    listProperties->clear();    
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
