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

#ifndef QRESOURCE_ITERATOR_P_H
#define QRESOURCE_ITERATOR_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qabstractfileengine.h"
#include "qdir.h"

class QResourceFileEngineIteratorPrivate;
class QResourceFileEngineIterator : public QAbstractFileEngineIterator
{
public:
    QResourceFileEngineIterator(QDir::Filters filters, const QStringList &filterNames);
    ~QResourceFileEngineIterator();

    QString next();
    bool hasNext() const;

    QString currentFileName() const;

private:
    QStringList entries;
    int index;
};

#endif
