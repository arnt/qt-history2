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

#ifndef PREVIEWFRAME_H
#define PREVIEWFRAME_H

#include <qvbox.h>
#include <qworkspace.h>

#include "previewwidgetimpl.h"

class PreviewWorkspace : public QWorkspace
{
    Q_OBJECT
public:
    PreviewWorkspace( QWidget* parent = 0, const char* name = 0 )
        : QWorkspace( parent, name ) {}
    ~PreviewWorkspace() {}

protected:
    void paintEvent( QPaintEvent* );
};

class PreviewFrame : public QVBox
{
    Q_OBJECT

public:
    PreviewFrame( QWidget *parent = 0, const char *name = 0 );
    void setPreviewPalette(QPalette);

private:
    PreviewWidget 	*previewWidget;
};

#endif
