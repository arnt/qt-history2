/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qscrollview.h#11 $
**
** Definition of QScrollView class
**
** Created : 970523
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/
#ifndef QVIEWP_H
#define QVIEWP_H

#ifndef QT_H
#include "qframe.h"
#include "qscrbar.h"
#endif // QT_H

struct QScrollViewData;

class QScrollView : public QFrame
{
    Q_OBJECT
public:
    QScrollView(QWidget *parent=0, const char *name=0, WFlags f=0);
    ~QScrollView();

    void setContents(QWidget* child);

    enum ScrollBarMode { Auto, AlwaysOff, AlwaysOn };

    ScrollBarMode vScrollBarMode() const;
    virtual void  setVScrollBarMode( ScrollBarMode );

    ScrollBarMode hScrollBarMode() const;
    virtual void  setHScrollBarMode( ScrollBarMode );

    QWidget*     cornerWidget() const;
    virtual void setCornerWidget(QWidget*);

    QScrollBar*  horizontalScrollBar();
    QScrollBar*  verticalScrollBar();

    void	contentsResize( int w, int h, bool = TRUE );
    int		contentsWidth() const;
    int		contentsHeight() const;
    int		contentsX() const;
    int		contentsY() const;

    void	resize( int w, int h );
    void	resize( const QSize& );
    void	show();

signals:
    void	contentsMoved(int x, int y);

public slots:
    void	ensureVisible(int x, int y);
    void	ensureVisible(int x, int y, int xmargin, int ymargin);
    void	center(int x, int y);
    void	center(int x, int y, float xmargin, float ymargin);

    void	updateScrollBars();

protected:
    void	resizeEvent(QResizeEvent*);
    bool	eventFilter( QObject *, QEvent *e );

    virtual void drawContentsOffset(QPainter*, int ox, int oy,
		    int cx, int cy, int cw, int ch);
    void	frameChanged();

    QWidget*	viewport();

    void setMargins(int left, int top, int right, int bottom);
    int leftMargin() const;
    int topMargin() const;
    int rightMargin() const;
    int bottomMargin() const;

private:
    void moveContents(int x, int y);

    QScrollViewData* d;

private slots:
    void hslide(int);
    void vslide(int);

private:	// Disabled copy constructor and operator=
    QScrollView( const QScrollView & );
    QScrollView &operator=( const QScrollView & );
    void changeFrameRect(const QRect&);
};

#endif
