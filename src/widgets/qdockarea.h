/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qworkspace.cpp#27 $
**
** Definition of the QDockArea class
**
** Created : 001010
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the workspace module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QDOCKAREA_H
#define QDOCKAREA_H

#ifndef QT_H
#include "qwidget.h"
#include "qlist.h"
#include "qdockwindow.h"
#include "qlayout.h"
#include "qvaluelist.h"
#include "qlist.h"
#include "qguardedptr.h"
#endif // QT_H

class QSplitter;
class QBoxLayout;
class QDockAreaLayout;
class QMouseEvent;

#if defined(Q_TEMPLATEDLL)
// MOC_SKIP_BEGIN
template class Q_EXPORT QValueList<QRect>;
template class Q_EXPORT QList<QDockWindow>;
// MOC_SKIP_END
#endif

class Q_EXPORT QDockAreaLayout : public QLayout
{
    Q_OBJECT
    friend class QDockArea;

public:
    QDockAreaLayout( QWidget* parent, Qt::Orientation o, QList<QDockWindow> *wl, int space = -1, int margin = -1, const char *name = 0 )
	: QLayout( parent, space, margin, name ), orient( o ), dockWidgets( wl ), parentWidget( parent ) { init(); }
    ~QDockAreaLayout() {}

    void addItem( QLayoutItem * ) {}
    bool hasHeightForWidth() const;
    int heightForWidth( int ) const;
    int widthForHeight( int ) const;
    QSize sizeHint() const;
    QSize minimumSize() const;
    QLayoutIterator iterator();
    QSizePolicy::ExpandData expanding() const { return QSizePolicy::NoDirection; }
    void invalidate();
    Qt::Orientation orientation() const { return orient; }
    QValueList<QRect> lineList() const { return lines; }
    QList<QDockWindow> lineStarts() const { return ls; }

protected:
    void setGeometry( const QRect& );

private:
    void init();
    int layoutItems( const QRect&, bool testonly = FALSE );
    Qt::Orientation orient;
    int cached_width, cached_height;
    int cached_hfw, cached_wfh;
    QList<QDockWindow> *dockWidgets;
    QWidget *parentWidget;
    QValueList<QRect> lines;
    QList<QDockWindow> ls;

};

class Q_EXPORT QDockArea : public QWidget
{
    Q_OBJECT

    friend class QDockWindow;

public:
    enum Gravity { Normal, Reverse };

    QDockArea( Orientation o, Gravity g = Normal, QWidget *parent = 0, const char *name = 0 );
    ~QDockArea();

    void moveDockWindow( QDockWindow *w, const QPoint &globalPos, const QRect &rect, bool swap );
    void removeDockWindow( QDockWindow *w, bool makeFloating, bool swap );
    void addDockWindow( QDockWindow *w, int index = -1 );
    bool hasDockWindow( QDockWindow *w, int *index = 0 );

    void invalidNextOffset( QDockWindow *dw );

    Orientation orientation() const { return orient; }
    Gravity gravity() const { return grav; }

    bool eventFilter( QObject *, QEvent * );
    bool isEmpty() const;
    void updateLayout();
    QList<QDockWindow> dockWidgetList() const;
    void lineUp( bool keepNewLines );

    bool isDockWindowAccepted( QDockWindow *dw );
    void setAcceptDockWindow( QDockWindow *dw, bool accept );

signals:
    void rightButtonPressed( const QPoint &globalPos );

protected:
    void mousePressEvent( QMouseEvent *e );

private:
    struct DockWindowData
    {
	int index;
	int offset;
	int line;
	QGuardedPtr<QDockArea> area;
    };

    int findDockWindow( QDockWindow *w );
    int lineOf( int index );
    DockWindowData *dockWidgetData( QDockWindow *w );
    void dockWidget( QDockWindow *dockWidget, DockWindowData *data );

private:
    Orientation orient;
    QList<QDockWindow> *dockWidgets;
    QDockAreaLayout *layout;
    Gravity grav;
    QList<QDockWindow> forbiddenWidgets;


};

#endif

