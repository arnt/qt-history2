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

#ifndef QPATHCLIPPER_P_H
#define QPATHCLIPPER_P_H

#include <QtGui/qpainterpath.h>
#include <QtCore/qlist.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

class PathVertex;

class Q_GUI_EXPORT QPathClipper
{
public:
    enum Operation {
        BoolAnd,
        BoolOr,
        BoolSub,
        BoolInSub
    };
public:
    QPathClipper();
    QPathClipper(const QPainterPath &subject,
                 const QPainterPath &clip);
    ~QPathClipper();

    void setSubjectPath(const QPainterPath &path);
    QPainterPath subjectPath() const;

    void setClipPath(const QPainterPath &path);
    QPainterPath clipPath() const;

    QPainterPath clip(Operation op=BoolAnd);
private:
    Q_DISABLE_COPY(QPathClipper)
    class Private;
    Private *d;
};

QT_END_HEADER

#endif // QPATHCLIPPER_P_H
