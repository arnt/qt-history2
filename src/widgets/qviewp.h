/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qviewp.h#2 $
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

    // Visual properties.
    virtual int scrollBarWidth() const;
    virtual bool scrollBarOnLeft() const;
    virtual bool scrollBarOnTop() const;
    virtual bool emptyCorner() const;
    virtual bool alwaysEmptyCorner() const;

    virtual void setBackgroundColor(const QColor&);
    virtual void setBackgroundPixmap(const QPixmap&);

    void viewResize( int w, int h );
    int viewWidth() const;
    int viewHeight() const;

    void resize( int w, int h );

public slots:
    void centerOn(int x, int y);
    void ensureVisible(int x, int y);
    void ensureVisible(int x, int y, int xmargin, int ymargin);
    void centralize(int x, int y);
    void centralize(int x, int y, float xmargin, float ymargin);

    void updateScrollBars();

protected:
    void resizeEvent(QResizeEvent*);
    bool eventFilter( QObject *, QEvent *e );

    virtual void drawContents(QPainter*, int cx, int cy, int cw, int ch);
    virtual void drawContentsOffset(QPainter*, int ox, int oy,
		    int cx, int cy, int cw, int ch);

private:
    void moveView(int x, int y);

    int viewX() const;
    int viewY() const;

    bool viewVisible() const;

    QScrollBar hbar;
    QScrollBar vbar;
    QWidget porthole;
    QWidget* viewed;
    static bool signal_choke;
    int vx, vy, vwidth, vheight; // for drawContents-style usage

private slots:
    void hslide(int);
    void vslide(int);
};

inline QScrollBar& QViewport::horizontalScrollBar() { return hbar; }
inline QScrollBar& QViewport::verticalScrollBar() { return vbar; }

#endif
