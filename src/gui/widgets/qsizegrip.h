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

#ifndef QSIZEGRIP_H
#define QSIZEGRIP_H

#include "QtGui/qwidget.h"

#ifndef QT_NO_SIZEGRIP

class QSizeGripPrivate;
class Q_GUI_EXPORT QSizeGrip : public QWidget
{
    Q_OBJECT
public:
    explicit QSizeGrip(QWidget *parent);
    ~QSizeGrip();

    QSize sizeHint() const;

    void show();
    void hide();
protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);

    bool eventFilter(QObject *, QEvent *);
    bool event(QEvent *);

#ifdef QT_COMPAT
    QT_COMPAT_CONSTRUCTOR QSizeGrip(QWidget *parent, const char *name);
#endif
private:
    Q_DECLARE_PRIVATE(QSizeGrip)
    Q_DISABLE_COPY(QSizeGrip)
};

#endif // QT_NO_SIZEGRIP

#endif // QSIZEGRIP_H
