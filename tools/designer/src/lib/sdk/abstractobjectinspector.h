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

#ifndef ABSTRACTOBJECTINSPECTOR_H
#define ABSTRACTOBJECTINSPECTOR_H

#include "sdk_global.h"

#include <QWidget>

class AbstractFormEditor;
class AbstractFormWindow;

class QT_SDK_EXPORT AbstractObjectInspector: public QWidget
{
    Q_OBJECT
public:
    AbstractObjectInspector(QWidget *parent, Qt::WindowFlags flags = 0);
    virtual ~AbstractObjectInspector();

    virtual AbstractFormEditor *core() const;

public slots:
    virtual void setFormWindow(AbstractFormWindow *formWindow) = 0;
};

#endif // ABSTRACTOBJECTINSPECTOR_H
