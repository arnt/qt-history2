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

#include <qvbox.h>
#include <qpainter.h>

#include "previewframe.h"

PreviewFrame::PreviewFrame( QWidget *parent, const char *name )
    : QVBox( parent, name )
{
    setMinimumSize(200, 200);
    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    setLineWidth(1);

    PreviewWorkspace * w = new PreviewWorkspace( this );
    w->setEraseColor(palette().dark());
    previewWidget = new PreviewWidget( w );
    previewWidget->move( 10, 10 );
}

void PreviewFrame::setPreviewPalette(QPalette pal)
{
    previewWidget->setPalette(pal);
}

void PreviewWorkspace::paintEvent( QPaintEvent* )
{
    QPainter p ( this );
    p.setPen( QPen( white ) );
    p.drawText ( 0, height() / 2,  width(), height(), AlignHCenter,
		"The moose in the noose\nate the goose who was loose." );
}

