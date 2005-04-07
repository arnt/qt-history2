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

#include "previewframe.h"

#include <qvboxwidget.h>
#include <qpainter.h>

PreviewFrame::PreviewFrame( QWidget *parent, const char *name )
    : QVBoxWidget( parent, name )
{
    setMinimumSize(200, 200);
    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    setLineWidth(1);

    Workspace * w = new Workspace( this );
    previewWidget = new PreviewWidget;
    w->addWindow(previewWidget);
    previewWidget->move( 10, 10 );
}

void PreviewFrame::setPreviewPalette(QPalette pal)
{
    previewWidget->setPalette(pal);
}

Workspace::Workspace( QWidget* parent, const char* name)
    : QWorkspace( parent, name )
{
}

void Workspace::paintEvent( QPaintEvent* )
{
    QPainter p ( this );
    p.fillRect(rect(), palette().color(backgroundRole()).dark());
    p.setPen( QPen( Qt::white ) );
    p.drawText ( 0, height() / 2,  width(), height(), Qt::AlignHCenter,
                "The moose in the noose\nate the goose who was loose." );
}
