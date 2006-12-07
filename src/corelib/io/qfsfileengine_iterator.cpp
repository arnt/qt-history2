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

#include "qfsfileengine_iterator_p.h"
#include "qvariant.h"

QFSFileEngineIterator::QFSFileEngineIterator(QDir::Filters filters, const QStringList &filterNames)
    : QAbstractFileEngineIterator(filters, filterNames)
{
    newPlatformSpecifics();
}

QFSFileEngineIterator::~QFSFileEngineIterator()
{
    deletePlatformSpecifics();
}

QString QFSFileEngineIterator::next()
{
    if (!hasNext())
        return QString();

    advance();
    return currentFilePath();
}

QString QFSFileEngineIterator::currentFileName() const
{
    return currentEntry;
}

QFileInfo QFSFileEngineIterator::currentFileInfo() const
{
    return QAbstractFileEngineIterator::currentFileInfo();
}
