/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGui>

#include "iconsizespinbox.h"

IconSizeSpinBox::IconSizeSpinBox(QWidget *parent)
    : QSpinBox(parent)
{
}

int IconSizeSpinBox::valueFromText(const QString &text) const
{
    QRegExp regExp(tr("(\\d+)(\\s*[x×]\\s*\\d+)?"));

    if (regExp.exactMatch(text)) {
        return regExp.cap(1).toInt();
    } else {
        return 0;
    }
}

QString IconSizeSpinBox::textFromValue(int value) const
{
    return tr("%1 × %1").arg(value);
}
