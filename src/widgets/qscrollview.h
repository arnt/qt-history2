/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qscrollview.h#1 $
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

#include <qframe.h>
#include <qscrbar.h>

struct QScrollViewData;

class QScrollView : public QFrame
{
    Q_OBJECT
public:
    QScrollView(QWidget *parent=0, const char *name=0, WFlags f=0);
    ~QScrollView();

    void view(QWidget* child);

    enum ScrollBarMode { Auto, AlwaysOff, AlwaysOn };

    ScrollBarMode vScrollBarMode() const;
    virtual void  setVScrollBarMode( ScrollBarMode );

    ScrollBarMode hScrollBarMode() const;
    virtual void  setHScrollBarMode( ScrollBarMode );

    QWidget*     cornerWidget() const;
    virtual void setCornerWidget(QWidget*);

    QScrollBar&  horizontalScrollBar();
    QScrollBar&  verticalScrollBar();

    void	setBackgroundColor(const QColor&);
    void	setBackgroundPixmap(const QPixmap&);

    void	viewResize( int w, int h );
    int		viewWidth() const;
    int		viewHeight() const;
    int		viewX() const;
    int		viewY() const;

    void	resize( int w, int h );
    void	show();

signals:
    void	viewMoved(int x, int y);

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

private:
    void moveView(int x, int y);

    QScrollViewData* d;

private slots:
    void hslide(int);
    void vslide(int);
};

#endif
