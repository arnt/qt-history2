/**********************************************************************
**   Copyright (C) 2000 Troll Tech AS.  All rights reserved.
**
**   This file is part of Qt GUI Designer.
**
**   This file may be distributed under the terms of the GNU General
**   Public License version 2 as published by the Free Software
**   Foundation and appearing in the file COPYING included in the
**   packaging of this file. If you did not get the file, send email
**   to info@trolltech.com
**
**   The file is provided AS IS with NO WARRANTY OF ANY KIND,
**   INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR
**   A PARTICULAR PURPOSE.
**
**********************************************************************/

#ifndef SIZEHANDLE_H
#define SIZEHANDLE_H

#include <qwidget.h>
#include <qintdict.h>
#include <qptrdict.h>

class QMouseEvent;
class FormWindow;
class WidgetSelection;

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
    WidgetSelection( FormWindow *parent, QPtrDict<WidgetSelection> *selDict );

    void setWidget( QWidget *w, bool updateDict = TRUE );
    bool isUsed() const;

    void updateGeometry();
    void hide();
    void show();

    QWidget *widget() const;

protected:
    QIntDict<SizeHandle> handles;
    QWidget *wid;
    FormWindow *formWindow;
    QPtrDict<WidgetSelection> *selectionDict;

};

#endif
