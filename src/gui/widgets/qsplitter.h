/****************************************************************************
**
** Definition of QSplitter class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSPLITTER_H
#define QSPLITTER_H

#ifndef QT_H
#include "qframe.h"
#endif // QT_H

#ifndef QT_NO_SPLITTER

class QSplitterHandle;
class QSplitterPrivate;
class QSplitterLayoutStruct;
class QTextStream;
template <typename T> class QList;

class Q_GUI_EXPORT QSplitter : public QFrame
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QSplitter);
    Q_PROPERTY( Orientation orientation READ orientation WRITE setOrientation )
    Q_PROPERTY( bool opaqueResize READ opaqueResize WRITE setOpaqueResize )
    Q_PROPERTY( int handleWidth READ handleWidth WRITE setHandleWidth )
    Q_PROPERTY( bool childrenCollapsible READ childrenCollapsible WRITE setChildrenCollapsible )

public:
    // ### Qt 4.0: remove Auto from public API
    enum ResizeMode { Stretch, KeepSize, FollowSizeHint, Auto };

    QSplitter( QWidget* parent = 0, const char* name = 0 );
    QSplitter( Orientation, QWidget* parent = 0, const char* name = 0 );
    ~QSplitter();

    virtual void setOrientation( Orientation );
    Orientation orientation() const;

    // ### Qt 4.0: make setChildrenCollapsible() and setCollapsible() virtual

    void setChildrenCollapsible( bool );
    bool childrenCollapsible() const;

    void setCollapsible( QWidget *w, bool );
    virtual void setResizeMode( QWidget *w, ResizeMode );
    virtual void setOpaqueResize( bool = TRUE );
    bool opaqueResize() const;

    void moveToFirst( QWidget * );
    void moveToLast( QWidget * );

    void refresh() { recalc( TRUE ); }
    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    QList<int> sizes() const;
    void setSizes( QList<int> );

    int handleWidth() const;
    void setHandleWidth( int );

protected:
    void childEvent( QChildEvent * );

    bool event( QEvent * );
    void resizeEvent( QResizeEvent * );

    int idAfter( QWidget* ) const;

    void moveSplitter( QCOORD pos, int id );
    virtual void drawSplitter( QPainter*, QCOORD x, QCOORD y,
			       QCOORD w, QCOORD h );
    void changeEvent( QEvent * );
    int adjustPos( int, int );
    virtual void setRubberband( int );
    void getRange( int id, int *, int * );

private:
    enum { DefaultResizeMode = 3 };

    void init();
    void recalc( bool update = FALSE );
    void doResize();
    void storeSizes();
    void getRange( int id, int *, int *, int *, int * );
    void addContribution( int, int *, int *, bool );
    int adjustPos( int, int, int *, int *, int *, int * );
    bool collapsible( QSplitterLayoutStruct * );
    void processChildEvents();
    QSplitterLayoutStruct *findWidget( QWidget * );
    QSplitterLayoutStruct *addWidget( QWidget *, bool prepend = FALSE );
    void recalcId();
    void doMove( bool backwards, int pos, int id, int delta, bool upLeft,
		 bool mayCollapse );
    void setGeo( QSplitterLayoutStruct *s, int pos, int size, bool splitterMoved );
    int findWidgetJustBeforeOrJustAfter( int id, int delta, int &collapsibleSize );
    void updateHandles();

    friend class QSplitterHandle;

#ifndef QT_NO_TEXTSTREAM
    friend Q_GUI_EXPORT QTextStream& operator<<( QTextStream&, const QSplitter& );
    friend Q_GUI_EXPORT QTextStream& operator>>( QTextStream&, QSplitter& );
#endif

private:
#if defined(Q_DISABLE_COPY)
    QSplitter( const QSplitter & );
    QSplitter& operator=( const QSplitter & );
#endif
};

#ifndef QT_NO_TEXTSTREAM
Q_GUI_EXPORT QTextStream& operator<<( QTextStream&, const QSplitter& );
Q_GUI_EXPORT QTextStream& operator>>( QTextStream&, QSplitter& );
#endif

#endif // QT_NO_SPLITTER

#endif // QSPLITTER_H
