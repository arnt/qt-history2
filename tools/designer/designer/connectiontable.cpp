/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "connectiontable.h"

ConnectionTable::ConnectionTable( QWidget *parent, const char *name )
    : QTable( 0, 4, parent, name )
{
    setSorting( true );
    setShowGrid( false );
    setFocusStyle( FollowStyle );
    setSelectionMode( SingleRow );
    horizontalHeader()->setLabel( 0, tr( "Sender" ) );
    horizontalHeader()->setLabel( 1, tr( "Signal" ) );
    horizontalHeader()->setLabel( 2, tr( "Receiver" ) );
    horizontalHeader()->setLabel( 3, tr( "Slot" ) );
    setColumnStretchable( 0, true );
    setColumnStretchable( 1, true );
    setColumnStretchable( 2, true );
    setColumnStretchable( 3, true );
}

void ConnectionTable::sortColumn( int col, bool ascending, bool )
{
    horizontalHeader()->setSortIndicator( col, ascending );
    if ( isEditing() )
	endEdit( currEditRow(), currEditCol(), false, false );
    QTable::sortColumn( col, ascending, true );
    setCurrentCell( 0, 0 );
    emit resorted();
}
