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

#ifndef QGRID_H
#define QGRID_H

#ifndef QT_H
#include "qframe.h"
#endif // QT_H

#ifndef QT_NO_GRID

class QGridLayout;

class Q_GUI_EXPORT QGrid : public QFrame
{
    Q_OBJECT
public:
    QGrid(int n, QWidget* parent=0, Qt::WFlags f = 0);
    QGrid(int n, Qt::Orientation orient, QWidget* parent=0, Qt::WFlags f = 0);

    void setSpacing(int);

#ifdef QT_COMPAT
    QGrid(int n, QWidget* parent, const char* name, Qt::WFlags f);
    QGrid(int n, Qt::Orientation orient, QWidget* parent, const char* name, Qt::WFlags f);
    typedef Qt::Orientation Direction;
#endif

protected:
    void childEvent(QChildEvent*);
private:
    QGridLayout *lay;
private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QGrid(const QGrid &);
    QGrid& operator=(const QGrid &);
#endif
};

#endif // QT_NO_GRID

#endif // QGRID_H
