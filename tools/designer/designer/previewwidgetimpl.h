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

#ifndef PREVIEWWIDGETIMPL_H
#define PREVIEWWIDGETIMPL_H

#include "previewwidget.h"

class PreviewWidget : public PreviewWidgetBase
{
    Q_OBJECT

public:
    PreviewWidget( QWidget *parent = 0, const char *name = 0 );

    void closeEvent(QCloseEvent *);
    bool eventFilter(QObject *, QEvent *);
};

#endif
