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

#include <qworkspace.h>

void AmbientProperties::init()
{
    container = 0;
}

void AmbientProperties::setControl( QWidget *widget )
{
    container = widget;
    backSample->setPaletteBackgroundColor( container->paletteBackgroundColor() );
    foreSample->setPaletteBackgroundColor( container->paletteForegroundColor() );
    fontSample->setFont( container->font() );
    enabledButton->setOn( container->isEnabled() );
    enabledSample->setEnabled( container->isEnabled() );
}

void AmbientProperties::backColor()
{
    QColor col = QColorDialog::getColor( backSample->paletteBackgroundColor(), this );
    backSample->setPaletteBackgroundColor( col );
    container->setPaletteBackgroundColor( col );

    QWorkspace *ws = (QWorkspace*)container->qt_cast( "QWorkspace" );
    if ( ws ) {
	QWidgetList list( ws->windowList() );
	QWidgetListIt it( list );
	while ( it.current() ) {
	    QWidget *widget = it.current();
	    widget->setPaletteBackgroundColor( col );
	    ++it;
	}
    }
}

void AmbientProperties::foreColor()
{
    QColor col = QColorDialog::getColor( foreSample->paletteBackgroundColor(), this );
    foreSample->setPaletteBackgroundColor( col );
    container->setPaletteForegroundColor( col );

    QWorkspace *ws = (QWorkspace*)container->qt_cast( "QWorkspace" );
    if ( ws ) {
	QWidgetList list( ws->windowList() );
	QWidgetListIt it( list );
	while ( it.current() ) {
	    QWidget *widget = it.current();
	    widget->setPaletteForegroundColor( col );
	    ++it;
	}
    }
}

void AmbientProperties::pickFont()
{
    bool ok;
    QFont f = QFontDialog::getFont( &ok, fontSample->font(), this );
    if ( !ok )
	return;
    fontSample->setFont( f );
    container->setFont( f );

    QWorkspace *ws = (QWorkspace*)container->qt_cast( "QWorkspace" );
    if ( ws ) {
	QWidgetList list( ws->windowList() );
	QWidgetListIt it( list );
	while ( it.current() ) {
	    QWidget *widget = it.current();
	    widget->setFont( f );
	    ++it;
	}
    }
}

void AmbientProperties::toggleEnabled(bool on)
{
    enabledSample->setEnabled( on );
    container->setEnabled( on );
}
