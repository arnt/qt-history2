/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef BUTTON_H
#define BUTTON_H

#include <QToolButton>

class Button : public QToolButton
{
    Q_OBJECT

public:
    Button(const QString &text, const QColor &color, QWidget *parent = 0);

    QSize sizeHint() const;
};

#endif
