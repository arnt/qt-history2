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

    QColor c = container->palette().color(container->backgroundRole());
    QPalette p = backSample->palette(); p.setColor(backSample->backgroundRole(), c); backSample->setPalette(p);

    c = container->palette().color(container->foregroundRole());
    p = foreSample->palette(); p.setColor(foreSample->backgroundRole(), c); foreSample->setPalette(p);

    fontSample->setFont( container->font() );
    enabledButton->setOn( container->isEnabled() );
    enabledSample->setEnabled( container->isEnabled() );
}

void AmbientProperties::backColor()
{
    QColor c = QColorDialog::getColor(backSample->palette().color(backSample->backgroundRole()), this);
    QPalette p = backSample->palette(); p.setColor(backSample->backgroundRole(), c); backSample->setPalette(p);
    p = container->palette(); p.setColor(container->backgroundRole(), c); container->setPalette(p);

    QWorkspace *ws = (QWorkspace*)container->qt_metacast( "QWorkspace" );
    if ( ws ) {
	QWidgetList list( ws->windowList() );
	for (int i = 0; i < list.count(); ++i) {
	    QWidget *widget = list.at(i);
	    p = widget->palette(); p.setColor(widget->backgroundRole(), c); widget->setPalette(p);
	}
    }
}

void AmbientProperties::foreColor()
{
    QColor c = QColorDialog::getColor(foreSample->palette().color(foreSample->backgroundRole()), this);
    QPalette p = foreSample->palette(); p.setColor(foreSample->backgroundRole(), c); foreSample->setPalette(p);
    p = container->palette(); p.setColor(container->foregroundRole(), c); container->setPalette(p);

    QWorkspace *ws = (QWorkspace*)container->qt_metacast( "QWorkspace" );
    if ( ws ) {
	QWidgetList list( ws->windowList() );
	for (int i = 0; i < list.count(); ++i) {
	    QWidget *widget = list.at(i);
	    p = widget->palette(); p.setColor(widget->foregroundRole(), c); widget->setPalette(p);
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

    QWorkspace *ws = (QWorkspace*)container->qt_metacast( "QWorkspace" );
    if ( ws ) {
	QWidgetList list( ws->windowList() );
	for (int i = 0; i < list.count(); ++i) {
	    QWidget *widget = list.at(i);
	    widget->setFont( f );
	}
    }
}

void AmbientProperties::toggleEnabled(bool on)
{
    enabledSample->setEnabled( on );
    container->setEnabled( on );
}
