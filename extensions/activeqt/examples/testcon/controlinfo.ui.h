/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of an example program for the ActiveQt integration.
** EDITIONS: UNKNOWN
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/


void ControlInfo::setControl( QAxWidget *activex )
{
    listInfo->clear();

    QMetaObject *mo = activex->metaObject();
    QListViewItem *item = new QListViewItem( listInfo, "Class Info", QString::number( mo->numClassInfo() ) );
    int i;
    for ( i = 0; i < mo->numClassInfo(FALSE ); ++i ) {
	const QClassInfo *info = mo->classInfo( i, FALSE );
	(void)new QListViewItem( item, info->name, info->value );
    }
    item = new QListViewItem( listInfo, "Signals", QString::number( mo->numSignals( FALSE ) ) );
    for ( i = 0; i < mo->numSignals(FALSE ); ++i ) {
	const QMetaData *signal = mo->signal( i, FALSE );
	(void)new QListViewItem( item, signal->name );
    }
    item = new QListViewItem( listInfo, "Slots", QString::number( mo->numSlots( FALSE ) ) );
    for ( i = 0; i < mo->numSlots( FALSE ); ++i ) {
	const QMetaData *slot = mo->slot( i, FALSE );
	(void)new QListViewItem( item, slot->name );
    }
    item = new QListViewItem( listInfo, "Properties", QString::number( mo->numProperties( FALSE ) ) );    
    for ( i = 0; i < mo->numProperties( FALSE ); ++i ) {
	const QMetaProperty *property = mo->property( i, FALSE );
	(void)new QListViewItem( item, property->name(), property->type() );
    }
}
