/****************************************************************************
**
** Definition of QViewport widget class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
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
    Q_PROPERTY( ScrollBarPolicy ScrollBarPolicy READ verticalScrollBarPolicy WRITE setVerticalScrollBarPolicy )
    Q_PROPERTY( ScrollBarPolicy horizontalScrollBarPolicy READ horizontalScrollBarPolicy WRITE setHorizontalScrollBarPolicy )

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

protected:
    QViewport(QViewportPrivate &dd, QWidget *parent);
    void setViewportMargins(int left, int top, int right, int bottom);

    bool event(QEvent *);
    bool viewportEvent(QEvent *);

    virtual void resizeEvent(QResizeEvent *);
    virtual void paintEvent(QPaintEvent *);
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mouseDoubleClickEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);
#ifndef QT_NO_WHEELEVENT
    virtual void wheelEvent(QWheelEvent *);
#endif

    virtual void keyPressEvent( QKeyEvent * );

    virtual void scrollContentsBy(int dx, int dy);

private slots:
    void hslide(int);
    void vslide(int);
    void showOrHideScrollBars();

private:
    Q_DECLARE_PRIVATE( QViewport );
};


#endif
