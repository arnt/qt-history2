/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qviewp.h#1 $
**
** Definition of QViewport class
**
** Created : 970523
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/
#ifndef QVIEWP_H
#define QVIEWP_H

#include <qwidget.h>
#include <qscrbar.h>

class QViewport : public QWidget
{
    Q_OBJECT
public:
    QViewport(QWidget *parent=0, const char *name=0, WFlags f=0);

    void view(QWidget* child);

    QScrollBar& horizontalScrollBar();
    QScrollBar& verticalScrollBar();

    void show();

    void ensureVisible(int x, int y, int xmargin=50, int ymargin=50);
    void centerOn(int x, int y);
    void centralize(int x, int y, float xmargin=0.5, float ymargin=0.5);

    // Visual properties.
    virtual int scrollBarWidth() const;
    virtual bool scrollBarOnLeft() const;
    virtual bool scrollBarOnTop() const;
    virtual bool emptyCorner() const;
    virtual bool alwaysEmptyCorner() const;

    virtual void setBackgroundColor(const QColor&);
    virtual void setBackgroundPixmap(const QPixmap&);

    void viewResize( int w, int h );

protected:
    void resizeEvent(QResizeEvent*);
    bool eventFilter( QObject *, QEvent *e );

    virtual void moveView(int x, int y);
    virtual int viewX() const;
    virtual int viewY() const;
    virtual int viewWidth() const;
    virtual int viewHeight() const;
    virtual bool viewVisible() const;

    virtual void drawContents(QPainter*, int cx, int cy, int cw, int ch);
    virtual void drawContentsOffset(QPainter*, int ox, int oy,
		    int cx, int cy, int cw, int ch);

private:
    QScrollBar hbar;
    QScrollBar vbar;
    QWidget porthole;
    QWidget* viewed;
    static bool signal_choke;
    void updateScrollBars();
    int vx, vy, vwidth, vheight; // for drawContents-style usage

private slots:
    void hslide(int);
    void vslide(int);
};

inline QScrollBar& QViewport::horizontalScrollBar() { return hbar; }
inline QScrollBar& QViewport::verticalScrollBar() { return vbar; }

#endif
