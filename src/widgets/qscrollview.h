/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qscrollview.h#37 $
**
** Definition of QScrollView class
**
** Created : 970523
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/
#ifndef QSCROLLVIEW_H
#define QSCROLLVIEW_H

#ifndef QT_H
#include "qframe.h"
#include "qscrollbar.h"
#endif // QT_H

struct QScrollViewData;

class Q_EXPORT QScrollView : public QFrame
{
    Q_OBJECT
public:
    QScrollView(QWidget *parent=0, const char *name=0, WFlags f=0);
    ~QScrollView();

    enum ResizePolicy { Default, Manual, AutoOne };
    virtual void setResizePolicy( ResizePolicy );
    ResizePolicy resizePolicy() const;

    void removeChild(QWidget* child);
    virtual void addChild( QWidget* child, int x=0, int y=0 );
    virtual void moveChild( QWidget* child, int x, int y );
    int childX(QWidget* child);
    int childY(QWidget* child);
    bool childIsVisible(QWidget* child);
    void showChild(QWidget* child, bool yes=TRUE);

    enum ScrollBarMode { Auto, AlwaysOff, AlwaysOn };

    ScrollBarMode vScrollBarMode() const;
    virtual void  setVScrollBarMode( ScrollBarMode );

    ScrollBarMode hScrollBarMode() const;
    virtual void  setHScrollBarMode( ScrollBarMode );

    QWidget*     cornerWidget() const;
    virtual void setCornerWidget(QWidget*);

    QScrollBar*  horizontalScrollBar() const;
    QScrollBar*  verticalScrollBar() const;
    QWidget*	 viewport() const;
    QWidget*	 clipper() const;

    int		visibleWidth() const;
    int		visibleHeight() const;

    int		contentsWidth() const;
    int		contentsHeight() const;
    int		contentsX() const;
    int		contentsY() const;

    void	resize( int w, int h );
    void	resize( const QSize& );
    void	show();

    void	updateContents( int x, int y, int w, int h );
    void	repaintContents( int x, int y, int w, int h, bool erase=TRUE );

    void	contentToViewport(int x, int y, int& vx, int& vy);
    void	viewportToContent(int vx, int vy, int& x, int& y);
    void	enableClipper(bool y);

    QSize	viewportSize( int, int ) const;

signals:
    void	contentsMoving(int x, int y);

public slots:
    virtual void resizeContents( int w, int h );
    void	scrollBy( int dx, int dy );
    virtual void        setContentsPos( int x, int y );
    void	ensureVisible(int x, int y);
    void	ensureVisible(int x, int y, int xmargin, int ymargin);
    void	center(int x, int y);
    void	center(int x, int y, float xmargin, float ymargin);

    void	updateScrollBars();

protected:
    void	resizeEvent(QResizeEvent*);
    void 	wheelEvent( QWheelEvent * );
    bool	eventFilter( QObject *, QEvent *e );

    virtual void viewportPaintEvent( QPaintEvent* );
    virtual void viewportResizeEvent( QResizeEvent* );
    virtual void viewportMousePressEvent( QMouseEvent* );
    virtual void viewportMouseReleaseEvent( QMouseEvent* );
    virtual void viewportMouseDoubleClickEvent( QMouseEvent* );
    virtual void viewportMouseMoveEvent( QMouseEvent* );
    virtual void viewportDragEnterEvent( QDragEnterEvent * );
    virtual void viewportDragMoveEvent( QDragMoveEvent * );
    virtual void viewportDragLeaveEvent( QDragLeaveEvent * );
    virtual void viewportDropEvent( QDropEvent * );
    virtual void viewportWheelEvent( QWheelEvent * );

    virtual void drawContentsOffset(QPainter*, int ox, int oy,
		    int cx, int cy, int cw, int ch);
    virtual void drawContents(QPainter*, int cx, int cy, int cw, int ch);
    void	frameChanged();

    virtual void setMargins(int left, int top, int right, int bottom);
    int leftMargin() const;
    int topMargin() const;
    int rightMargin() const;
    int bottomMargin() const;

    bool focusNextPrevChild( bool next );

    virtual void setHBarGeometry(QScrollBar& hbar, int x, int y, int w, int h);
    virtual void setVBarGeometry(QScrollBar& vbar, int x, int y, int w, int h);

private:
    void moveContents(int x, int y);

    QScrollViewData* d;

private slots:
    void hslide(int);
    void vslide(int);

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QScrollView( const QScrollView & );
    QScrollView &operator=( const QScrollView & );
#endif
    void changeFrameRect(const QRect&);
};

#endif
