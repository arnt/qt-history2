/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "biff.h"
#include <qstring.h>
#include <qfileinfo.h>
#include <qpainter.h>

#include <unistd.h>
#include <stdlib.h>

#include "bmp.cpp"


Biff::Biff( QWidget *parent, const char *name )
    : QWidget( parent, name, WType_Modal )
{
    QFileInfo fi = QString(getenv( "MAIL" ));
    if ( !fi.exists() ) {
	QString s( "/var/spool/mail/" );
	s += getlogin();
	fi.setFile( s );
    }

    if ( fi.exists() ) {
	mailbox = fi.absFilePath();
	startTimer( 1000 );
    }

    setMinimumSize( 48, 48 );
    setMaximumSize( 48, 48 );
    resize( 48, 48 );

    hasNewMail.loadFromData( hasmail_bmp_data, hasmail_bmp_len );
    noNewMail.loadFromData( nomail_bmp_data, nomail_bmp_len );

    gotMail = FALSE;
    lastModified = fi.lastModified();
}


void Biff::timerEvent( QTimerEvent * )
{
    QFileInfo fi( mailbox );
    bool newState = ( fi.lastModified() != lastModified &&
		      fi.lastModified() > fi.lastRead() );
    if ( newState != gotMail ) {
	if ( gotMail )
	    lastModified = fi.lastModified();
	gotMail = newState;
	repaint( FALSE );
    }
}
    

void Biff::paintEvent( QPaintEvent * )
{
    if ( gotMail )
	bitBlt( this, 0, 0, &hasNewMail );
    else
	bitBlt( this, 0, 0, &noNewMail );
}


void Biff::mousePressEvent( QMouseEvent * )
{
    QFileInfo fi( mailbox );
    lastModified = fi.lastModified();
}
