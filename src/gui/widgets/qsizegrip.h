/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSIZEGRIP_H
#define QSIZEGRIP_H

#include <QtGui/qwidget.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_SIZEGRIP

class QSizeGripPrivate;
class Q_GUI_EXPORT QSizeGrip : public QWidget
{
    Q_OBJECT
public:
    explicit QSizeGrip(QWidget *parent);
    ~QSizeGrip();

    QSize sizeHint() const;

    void setVisible(bool);
protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void moveEvent(QMoveEvent *moveEvent);

    bool eventFilter(QObject *, QEvent *);
    bool event(QEvent *);
#ifdef Q_WS_WIN
    bool winEvent( MSG *m, long *result );
#endif
public:
#ifdef QT3_SUPPORT
    QT3_SUPPORT_CONSTRUCTOR QSizeGrip(QWidget *parent, const char *name);
#endif
private:
    Q_DECLARE_PRIVATE(QSizeGrip)
    Q_DISABLE_COPY(QSizeGrip)
};

#endif // QT_NO_SIZEGRIP

QT_END_HEADER

#endif // QSIZEGRIP_H
