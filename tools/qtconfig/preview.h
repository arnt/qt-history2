#ifndef PREVIEW_H
#define PREVIEW_H

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

#include <qwidget.h>
#include <qworkspace.h>
#include <qframe.h>
#include "ui_preview.h"

class PreviewWidget : public QWidget
{
    Q_OBJECT
public:
    PreviewWidget(QWidget *parent);
    virtual ~PreviewWidget();

private:
    Ui::PreviewWidget ui;
};

class PreviewWorkspace: public QWorkspace
{
    Q_OBJECT
public:
    PreviewWorkspace(QWidget *parent);
protected:
    void paintEvent(QPaintEvent *);
};

class PreviewFrame: public QFrame
{
    Q_OBJECT
public:
    PreviewFrame(QWidget *parent);

    void setPreviewPalette(const QPalette &palette);

private:
    PreviewWidget *previewWidget;
};

#endif

