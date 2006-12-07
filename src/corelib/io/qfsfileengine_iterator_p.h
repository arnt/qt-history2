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

#ifndef QFSFILEENGINE_ITERATOR_P_H
#define QFSFILEENGINE_ITERATOR_P_H

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

class QFSFileEngineIteratorPrivate;
class QFSFileEngineIteratorPlatformSpecificData;

class QFSFileEngineIterator : public QAbstractFileEngineIterator
{
public:
    QFSFileEngineIterator(QDir::Filters filters, const QStringList &filterNames);
    ~QFSFileEngineIterator();

    QString next();
    bool hasNext() const;

    QString currentFileName() const;
    QFileInfo currentFileInfo() const;

private:
    QFSFileEngineIteratorPlatformSpecificData *platform;
    friend class QFSFileEngineIteratorPlatformSpecificData;
    void newPlatformSpecifics();
    void deletePlatformSpecifics();
    void advance();
    
    QString currentEntry;
};

#endif
