/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QVIEWPORT_H
#define QVIEWPORT_H

#include "qframe.h"

class QScrollBar;
class QViewportPrivate;

class Q_GUI_EXPORT QViewport : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(Qt::ScrollBarPolicy verticalScrollBarPolicy READ verticalScrollBarPolicy WRITE setVerticalScrollBarPolicy)
    Q_PROPERTY(Qt::ScrollBarPolicy horizontalScrollBarPolicy READ horizontalScrollBarPolicy WRITE setHorizontalScrollBarPolicy)

public:
    QViewport(QWidget* parent=0);
    ~QViewport();

    Qt::ScrollBarPolicy verticalScrollBarPolicy() const;
    void setVerticalScrollBarPolicy(Qt::ScrollBarPolicy);
    QScrollBar *verticalScrollBar() const;

    Qt::ScrollBarPolicy horizontalScrollBarPolicy() const;
    void setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy);
    QScrollBar *horizontalScrollBar() const;

    QWidget *viewport() const;
    QSize maximumViewportSize() const;

    QSize minimumSizeHint() const;

    QSize sizeHint() const;

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
    Q_DECLARE_PRIVATE(QViewport)
    Q_DISABLE_COPY(QViewport)
    Q_PRIVATE_SLOT(d, void hslide(int))
    Q_PRIVATE_SLOT(d, void vslide(int))
    Q_PRIVATE_SLOT(d,void showOrHideScrollBars())

};


#endif
