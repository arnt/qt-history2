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

#ifndef PREVIEWFRAME_H
#define PREVIEWFRAME_H

#include "previewwidget.h"

#include <qvboxwidget.h>
#include <q3workspace.h>

class Workspace : public Q3Workspace
{
    Q_OBJECT

public:
    Workspace( QWidget* parent = 0, const char* name = 0 );
    ~Workspace() {}

protected:
    void paintEvent( QPaintEvent* );
};

class PreviewFrame : public QVBoxWidget
{
    Q_OBJECT

public:
    PreviewFrame( QWidget *parent = 0, const char *name = 0 );
    void setPreviewPalette(QPalette);

private:
    PreviewWidget 	*previewWidget;
};

#endif
