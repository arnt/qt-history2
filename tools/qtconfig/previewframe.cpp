/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "previewframe.h"

#include <QBoxLayout>
#include <QPainter>
#include <QMdiSubWindow>

QT_BEGIN_NAMESPACE

PreviewFrame::PreviewFrame( QWidget *parent, const char *name )
    : QFrame( parent, name )
{
    setMinimumSize(200, 200);
    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    setLineWidth(1);

    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->setMargin(0);
    Workspace * w = new Workspace( this );
    vbox->addWidget(w);

    previewWidget = new PreviewWidget;
    previewWidget->setAutoFillBackground(true);
    QMdiSubWindow *frame = w->addSubWindow(previewWidget, Qt::Window);
    frame->move(10,10);
    frame->show();
}

void PreviewFrame::setPreviewPalette(QPalette pal)
{
    previewWidget->setPalette(pal);
}

Workspace::Workspace( QWidget* parent, const char* name)
    : QMdiArea(parent)
{
    setObjectName(name);
}

void Workspace::paintEvent( QPaintEvent* )
{
    QPainter p (viewport());
    p.fillRect(rect(), palette().color(backgroundRole()).dark());
    p.setPen( QPen( Qt::white ) );
    p.drawText ( 0, height() / 2,  width(), height(), Qt::AlignHCenter,
                QLatin1String("The moose in the noose\nate the goose who was loose.") );
}

QT_END_NAMESPACE
