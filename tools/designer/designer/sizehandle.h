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

#ifndef SIZEHANDLE_H
#define SIZEHANDLE_H

#include <qwidget.h>
#include <qhash.h>

class QMouseEvent;
class FormWindow;
class WidgetSelection;
class QPaintEvent;

class SizeHandle : public QWidget
{
    Q_OBJECT

public:
    enum Direction { LeftTop, Top, RightTop, Right, RightBottom, Bottom, LeftBottom, Left };

    SizeHandle( FormWindow *parent, Direction d, WidgetSelection *s );
    void setWidget( QWidget *w );
    void setActive( bool a );
    void updateCursor();

    void setEnabled( bool ) {}

protected:
    void paintEvent( QPaintEvent *e );
    void mousePressEvent( QMouseEvent *e );
    void mouseMoveEvent( QMouseEvent *e );
    void mouseReleaseEvent( QMouseEvent *e );

private:
    void trySetGeometry( QWidget *w, int x, int y, int width, int height );
    void tryResize( QWidget *w, int width, int height );

private:
    QWidget *widget;
    Direction dir;
    QPoint oldPressPos;
    FormWindow *formWindow;
    WidgetSelection *sel;
    QRect geom, origGeom;
    bool active;

};

class WidgetSelection
{
public:
    WidgetSelection( FormWindow *parent, QHash<QWidget *, WidgetSelection *> *selDict );

    void setWidget( QWidget *w, bool updateDict = TRUE );
    bool isUsed() const;

    void updateGeometry();
    void hide();
    void show();
    void update();
    
    QWidget *widget() const;

protected:
    QHash<int, SizeHandle*> handles;
    QWidget *wid;
    FormWindow *formWindow;
    QHash<QWidget *, WidgetSelection *> *selectionDict;

};

#endif
