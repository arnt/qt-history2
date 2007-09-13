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

#ifndef QTSPINBOX_H
#define QTSPINBOX_H

#include <QtGui/QSpinBox>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

class QtSpinBox : public QSpinBox
{
    Q_OBJECT
public:
    QtSpinBox(QWidget *parent = 0);

    void stepBy(int steps);
};

class QtDoubleSpinBox : public QDoubleSpinBox
{
    Q_OBJECT
public:
    QtDoubleSpinBox(QWidget *parent = 0);
    void fixup(QString &input) const;

    void stepBy(int steps);
};

}

QT_END_NAMESPACE

#endif
