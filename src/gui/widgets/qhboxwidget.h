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


#ifndef QHBOXWIDGET_H
#define QHBOXWIDGET_H

#include "QtGui/qwidget.h"
#include "QtGui/qframe.h"

#ifndef QT_NO_HBOXWIDGET


class QBoxLayout;

class Q_GUI_EXPORT QHBoxWidget : public QFrame
{
    Q_OBJECT
public:
    QHBoxWidget(QWidget* parent=0, Qt::WFlags f=0);

    void setMargin(int);
    void setSpacing(int);
    bool setStretchFactor(QWidget*, int stretch);
    QSize sizeHint() const;

protected:
    void childEvent(QChildEvent *);

#ifdef QT_COMPAT
public:
    QT_COMPAT_CONSTRUCTOR QHBoxWidget(QWidget* parent, const char* name, Qt::WFlags f=0);
#endif
protected:
    QHBoxWidget(Qt::Orientation orientation, QWidget* parent, Qt::WFlags f);

private:
    Q_DISABLE_COPY(QHBoxWidget)
    QBoxLayout *lay;
};

#endif // QT_NO_HBOX

#endif // QHBOX_H
