/****************************************************************************
**
** Definition of QViewport widget class.
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

#ifndef QVIEWPORT_H
#define QVIEWPORT_H

#ifndef QT_H
#include "qframe.h"
#endif // QT_H

class QScrollBar;
class QViewportPrivate;

class Q_GUI_EXPORT QViewport : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(ScrollBarPolicy ScrollBarPolicy READ verticalScrollBarPolicy WRITE setVerticalScrollBarPolicy)
    Q_PROPERTY(ScrollBarPolicy horizontalScrollBarPolicy READ horizontalScrollBarPolicy WRITE setHorizontalScrollBarPolicy)
    Q_OVERRIDE(bool acceptDrops READ acceptDrops WRITE setAcceptDrops)

public:
    QViewport(QWidget* parent=0);
    ~QViewport();

    ScrollBarPolicy verticalScrollBarPolicy() const;
    void setVerticalScrollBarPolicy(ScrollBarPolicy);
    QScrollBar *verticalScrollBar() const;

    ScrollBarPolicy horizontalScrollBarPolicy() const;
    void setHorizontalScrollBarPolicy(ScrollBarPolicy);
    QScrollBar *horizontalScrollBar() const;

    QWidget *viewport() const;
    QSize maximumViewportSize() const;

    QSize minimumSizeHint() const;

protected:
    QViewport(QViewportPrivate &dd, QWidget *parent);
    void setViewportMargins(int left, int top, int right, int bottom);

    bool event(QEvent *);
    virtual bool viewportEvent(QEvent *);

    void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseDoubleClickEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
#ifndef QT_NO_WHEELEVENT
    void wheelEvent(QWheelEvent *);
#endif
    void contextMenuEvent(QContextMenuEvent *);
#ifndef QT_NO_DRAGANDDROP
    void dragEnterEvent(QDragEnterEvent *);
    void dragMoveEvent(QDragMoveEvent *);
    void dragLeaveEvent(QDragLeaveEvent *);
    void dropEvent(QDropEvent *);
#endif

    void keyPressEvent(QKeyEvent *);

    virtual void scrollContentsBy(int dx, int dy);

private:
    Q_DECLARE_PRIVATE(QViewport);
    Q_PRIVATE_SLOT(void hslide(int))
    Q_PRIVATE_SLOT(void vslide(int))
    Q_PRIVATE_SLOT(void showOrHideScrollBars())

};


#endif
