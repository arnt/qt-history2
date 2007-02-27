/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
****************************************************************************/

#ifndef QCOLUMNVIEWGRIP_P_H
#define QCOLUMNVIEWGRIP_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <private/qwidget_p.h>

#ifndef QT_NO_QCOLUMNVIEW

class QColumnViewGripPrivate;

class Q_AUTOTEST_EXPORT QColumnViewGrip : public QWidget {

Q_OBJECT

Q_SIGNALS:
    void gripMoved(int offset);

public:
    explicit QColumnViewGrip(QWidget *parent = 0);
    ~QColumnViewGrip();
    int moveGrip(int offset);

protected:
    QColumnViewGrip(QColumnViewGripPrivate &, QWidget *parent = 0, Qt::WFlags f = 0);
    void paintEvent(QPaintEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);

private:
    Q_DECLARE_PRIVATE(QColumnViewGrip)
    Q_DISABLE_COPY(QColumnViewGrip)
};

class QColumnViewGripPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QColumnViewGrip)

public:
    QColumnViewGripPrivate();
    ~QColumnViewGripPrivate() {}

    int originalXLocation;
};

#endif // QT_NO_QCOLUMNVIEW

#endif //QCOLUMNVIEWGRIP_P_H
