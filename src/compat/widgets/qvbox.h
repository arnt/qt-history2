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

#ifndef QVBOX_H
#define QVBOX_H

#include <qvboxwidget.h>

class Q_COMPAT_EXPORT QVBox : public QVBoxWidget
{
    Q_OBJECT
public:
    QVBox(QWidget* parent=0, const char* name=0, Qt::WFlags f=0)
        : QVBoxWidget(parent,name,f) {}
};

#endif
