/****************************************************************************
**
** Definition of QSizeGrip class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSIZEGRIP_H
#define QSIZEGRIP_H

#ifndef QT_H
#include "qwidget.h"
#endif // QT_H

#ifndef QT_NO_SIZEGRIP

class Q_GUI_EXPORT QSizeGrip: public QWidget
{
    Q_OBJECT
public:
    QSizeGrip(QWidget* parent);
#ifdef QT_COMPAT
    QT_COMPAT_CONSTRUCTOR QSizeGrip(QWidget* parent, const char* name);
#endif
    ~QSizeGrip();

    QSize sizeHint() const;

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);

    bool eventFilter(QObject *, QEvent *);

private:
    void init();
    QPoint p;
    QSize s;
    int d;
    QWidget *tlw;
};

#endif //QT_NO_SIZEGRIP
#endif
