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

#ifndef QHBOX_H
#define QHBOX_H

#include <qhboxwidget.h>

class Q_COMPAT_EXPORT QHBox : public QHBoxWidget
{
    Q_OBJECT
public:
    QHBox(QWidget* parent=0, const char* name=0, Qt::WFlags f=0)
        : QHBoxWidget(parent,name,f) {}
};

#endif
