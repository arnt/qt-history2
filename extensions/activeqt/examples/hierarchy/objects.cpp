/****************************************************************************
** $Id: $
**
** Copyright (C) 2001-2003 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for the ActiveQt integration.
** This example program may be used, distributed and modified without 
** limitation.
**
*****************************************************************************/


#include "objects.h"
#include <qlayout.h>
#include <qpainter.h>

/* Implementation of QParentWidget */
QParentWidget::QParentWidget( QWidget *parent, const char *name, WFlags f )
: QWidget( parent, name, f )
{
    vbox = new QVBoxLayout( this );
    vbox->setAutoAdd( TRUE );
}

void QParentWidget::createSubWidget( const QString &name )
{
    QSubWidget *sw = new QSubWidget( this, name );
    sw->setLabel( name );
    sw->show();
}

QSubWidget *QParentWidget::subWidget( const QString &name )
{
    return (QSubWidget*)child( name, "QSubWidget" );
}

QSize QParentWidget::sizeHint() const
{
    return QWidget::sizeHint().expandedTo( QSize(100, 100) );
}

/* Implementation of QSubWidget */
QSubWidget::QSubWidget( QWidget *parent, const char *name, WFlags f )
: QWidget( parent, name, f )
{
}

void QSubWidget::setLabel( const QString &text )
{
    lbl = text;
    setName( text );
    update();
}

QString QSubWidget::label() const
{
    return lbl;
}

QSize QSubWidget::sizeHint() const
{
    QFontMetrics fm( font() );
    return QSize( fm.width(lbl), fm.height() );
}

void QSubWidget::paintEvent( QPaintEvent * )
{
    QPainter painter(this);
    painter.setPen( colorGroup().text() );
    painter.drawText( rect(), AlignCenter, lbl );
}
