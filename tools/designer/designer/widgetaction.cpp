/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "widgetaction.h"
#include <qwidget.h>
#include <qtoolbutton.h>
#include "mainwindow.h"
#include <qstylefactory.h>
#include <qstyle.h>

static QStyle *windowsStyle = 0;

void WidgetAction::addedTo( QWidget *w, QWidget *container )
{
    if ( qt_cast<QToolButton*>(w) && qt_cast<QScrollView*>(container->parent()->parent()) ) {
	if ( !windowsStyle )
	    windowsStyle = QStyleFactory::create( "windows" );
	w->setStyle( windowsStyle );
	( (QToolButton*)w )->setUsesTextLabel( TRUE );
	( (QToolButton*)w )->setTextPosition( QToolButton::Right );
    }
}

WidgetAction::~WidgetAction()
{
    if ( MainWindow::self )
	MainWindow::self->toolActions.removeRef( this );
}

void WidgetAction::init( const QString &g )
{
    MainWindow::self->toolActions.append( this );
    grp = g;
}
