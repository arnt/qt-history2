/****************************************************************************
**
** Definition of the QDockArea class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the workspace module of the Qt GUI Toolkit.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QDOCKAREA_H
#define QDOCKAREA_H

#ifndef QT_H
#include "qwidget.h"
#include "qlist.h"
#include "qdockwindow.h"
#include "qlayout.h"
#include "qpointer.h"
#endif // QT_H

#ifndef QT_NO_MAINWINDOW

class QSplitter;
class QBoxLayout;
class QDockAreaLayout;
class QMouseEvent;
class QDockWindowResizeHandle;
class QDockAreaPrivate;
class QTextStream;

class Q_GUI_EXPORT QDockAreaLayout : public QLayout
{
    Q_OBJECT
    friend class QDockArea;

public:
    QDockAreaLayout( QWidget* parent, Qt::Orientation o, QList<QDockWindow *> *wl, int space = -1, int margin = -1, const char *name = 0 )
	: QLayout( parent, space, margin, name ), orient( o ), dockWindows( wl ), parentWidget( parent ) { init(); }
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
    QList<QRect> lineList() const { return lines; }
    QList<QDockWindow *> lineStarts() const { return ls; }

protected:
    void setGeometry( const QRect& );

private:
    void init();
    int layoutItems( const QRect&, bool testonly = FALSE );
    Qt::Orientation orient;
    bool dirty;
    int cached_width, cached_height;
    int cached_hfw, cached_wfh;
    QList<QDockWindow *> *dockWindows;
    QWidget *parentWidget;
    QList<QRect> lines;
    QList<QDockWindow *> ls;
#if defined(Q_DISABLE_COPY) // Disabled copy constructor and operator=
    QDockAreaLayout( const QDockAreaLayout & );
    QDockAreaLayout &operator=( const QDockAreaLayout & );
#endif
};

class Q_GUI_EXPORT QDockArea : public QWidget
{
    Q_OBJECT
    Q_ENUMS( HandlePosition )
    Q_PROPERTY( Orientation orientation READ orientation )
    Q_PROPERTY( int count READ count )
    Q_PROPERTY( bool empty READ isEmpty )
    Q_PROPERTY( HandlePosition handlePosition READ handlePosition )

    friend class QDockWindow;
    friend class QDockWindowResizeHandle;
    friend class QDockAreaLayout;

public:
    enum HandlePosition { Normal, Reverse };

    QDockArea( Orientation o, HandlePosition h = Normal, QWidget* parent=0, const char* name=0 );
    ~QDockArea();

    void moveDockWindow( QDockWindow *w, const QPoint &globalPos, const QRect &rect, bool swap );
    void removeDockWindow( QDockWindow *w, bool makeFloating, bool swap, bool fixNewLines = TRUE );
    void moveDockWindow( QDockWindow *w, int index = -1 );
    bool hasDockWindow( QDockWindow *w, int *index = 0 );

    void invalidNextOffset( QDockWindow *dw );

    Orientation orientation() const { return orient; }
    HandlePosition handlePosition() const { return hPos; }

    bool eventFilter( QObject *, QEvent * );
    bool isEmpty() const;
    int count() const;
    QList<QDockWindow *> dockWindowList() const;

    bool isDockWindowAccepted( QDockWindow *dw );
    void setAcceptDockWindow( QDockWindow *dw, bool accept );

public slots:
    void lineUp( bool keepNewLines );

private:
    struct DockWindowData
    {
	int index;
	int offset;
	int line;
	QSize fixedExtent;
	QPointer<QDockArea> area;
    };

    int findDockWindow( QDockWindow *w );
    int lineOf( int index );
    DockWindowData *dockWindowData( QDockWindow *w );
    void dockWindow( QDockWindow *dockWindow, DockWindowData *data );
    void updateLayout();
    void invalidateFixedSizes();
    int maxSpace( int hint, QDockWindow *dw );
    void setFixedExtent( int d, QDockWindow *dw );
    bool isLastDockWindow( QDockWindow *dw );

private:
    Orientation orient;
    QList<QDockWindow *> dockWindows;
    QDockAreaLayout *layout;
    HandlePosition hPos;
    QList<QDockWindow *> forbiddenWidgets;
    QDockAreaPrivate *d;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QDockArea( const QDockArea & );
    QDockArea& operator=( const QDockArea & );
#endif

};

#ifndef QT_NO_TEXTSTREAM
Q_GUI_EXPORT QTextStream &operator<<( QTextStream &, const QDockArea & );
Q_GUI_EXPORT QTextStream &operator>>( QTextStream &, QDockArea & );
#endif

#endif

#endif //QT_NO_MAINWINDOW
