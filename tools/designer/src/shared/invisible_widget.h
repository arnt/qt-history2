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

#ifndef INVISIBLE_WIDGET_H
#define INVISIBLE_WIDGET_H

#include "shared_global.h"

#include <QtGui/QWidget>

class QT_SHARED_EXPORT InvisibleWidget: public QWidget
{
    Q_OBJECT
public:
    InvisibleWidget(QWidget *parent = 0);
};

#endif // INVISIBLE_WIDGET_H
