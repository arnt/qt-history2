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

#ifndef IMAGECOLLECTION_H
#define IMAGECOLLECTION_H

#include "imagecollection_global.h"
#include <abstractimagecollection.h>

class QT_IMAGECOLLECTION_EXPORT ImageCollection: public AbstractImageCollection
{
    Q_OBJECT
public:
    ImageCollection(AbstractFormEditor *core, QObject *parent = 0);
    virtual ~ImageCollection();

    virtual AbstractFormEditor *core() const;

    virtual QString fileName() const;
    virtual QString prefix() const;

    virtual int count() const;
    virtual QString item(int index) const;

private:
    AbstractFormEditor *m_core;
};

#endif // IMAGECOLLECTION_H
