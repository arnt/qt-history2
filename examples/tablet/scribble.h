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

#ifndef SCRIBBLE_H
#define SCRIBBLE_H

#include <qmainwindow.h>
#include <qpen.h>
#include <qpoint.h>
#include <qpixmap.h>
#include <qwidget.h>
#include <qstring.h>
#include <qpointarray.h>

class QMouseEvent;
class QResizeEvent;
class QPaintEvent;
class QSpinBox;
class QToolButton;
class Canvas;

class Scribble : public QMainWindow
{
    Q_OBJECT

public:
    Scribble( QWidget *parent = 0, const char *name = 0 );

protected:
    Canvas* canvas;

    QSpinBox *bPWidth;
    QToolButton *bPColor, *bSave, *bClear;

protected slots:
    void slotSave();
    void slotColor();
    void slotWidth( int );
    void slotClear();

};

#endif
