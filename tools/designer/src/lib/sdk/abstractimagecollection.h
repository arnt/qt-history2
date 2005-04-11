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

#ifndef ABSTRACTIMAGECOLLECTION_H
#define ABSTRACTIMAGECOLLECTION_H

#include <QtDesigner/sdk_global.h>
#include <QtCore/QObject>

class QDesignerFormEditorInterface;

class QT_SDK_EXPORT QDesignerImageCollectionInterface: public QObject
{
    Q_OBJECT
public:
    QDesignerImageCollectionInterface(QObject *parent = 0);
    virtual ~QDesignerImageCollectionInterface();

    virtual QDesignerFormEditorInterface *core() const = 0;

    virtual QString fileName() const = 0;
    virtual QString prefix() const = 0;

    virtual int count() const = 0;
    virtual QString item(int index) const = 0;
};

#endif // ABSTRACTIMAGECOLLECTION_H
