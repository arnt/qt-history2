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

#ifndef QGRIDWIDGET_H
#define QGRIDWIDGET_H

#ifndef QT_H
#include "qframe.h"
#endif // QT_H

#ifndef QT_NO_GRIDWIDGET

class QGridLayout;

class Q_GUI_EXPORT QGridWidget : public QFrame
{
    Q_OBJECT
public:
    QGridWidget(int n, QWidget* parent=0, Qt::WFlags f = 0);
    QGridWidget(int n, Qt::Orientation orientation, QWidget* parent=0, Qt::WFlags f = 0);

    void setSpacing(int);

#ifdef QT_COMPAT
    QT_COMPAT_CONSTRUCTOR QGridWidget(int n, QWidget* parent, const char* name, Qt::WFlags f);
    QT_COMPAT_CONSTRUCTOR QGridWidget(int n, Qt::Orientation orientation, QWidget* parent, const char* name, Qt::WFlags f);
    typedef Qt::Orientation Direction;
#endif

protected:
    void childEvent(QChildEvent*);

private:
    Q_DISABLE_COPY(QGridWidget)

    QGridLayout *lay;
};

#endif // QT_NO_GRIDWIDGET

#endif // QGRIDWIDGET_H
