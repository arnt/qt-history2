/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include <qhbox.h>
#include "previewframe.h"

PreviewFrame::PreviewFrame( QWidget *parent, const char *name )
    : QFrame( parent, name )
{
    setMinimumSize(200, 200);
    setFrameStyle(StyledPanel | Sunken);
    setLineWidth(1);
    setEraseColor(QColor(180, 180, 180));

    QHBox* box = new QHBox(this);
    box->setFrameStyle(StyledPanel | Raised);
    box->setLineWidth(2);
    box->move(10, 10);
    previewWidget = new PreviewWidget(box);
    box->resize(previewWidget->width() + 2 * box->lineWidth(),
		previewWidget->height() + 2 * box->lineWidth());
}

void PreviewFrame::setPreviewPalette(QPalette pal)
{
    previewWidget->setPalette(pal);
}
