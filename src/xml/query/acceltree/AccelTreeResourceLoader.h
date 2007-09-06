/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef Patternist_AccelTreeResourceLoader_H
#define Patternist_AccelTreeResourceLoader_H

class QIODevice;
#include <QHash>

#include "AccelTree.h"
#include "NamePool.h"
#include "ResourceLoader.h"
#include "SequenceReceiver.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Handles requests for documents, and instantiates
     * them as AccelTree instances.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class AccelTreeResourceLoader : public ResourceLoader
    {
    public:
        AccelTreeResourceLoader(const NamePool::Ptr &np);

        virtual Item openDocument(const QUrl &uri);
        virtual SequenceType::Ptr announceDocument(const QUrl &uri, const Usage usageHint);
        virtual bool isDocumentAvailable(const QUrl &uri);

    private:
        static bool streamToReceiver(QIODevice *const dev,
                                     const SequenceReceiver::Ptr &receiver,
                                     const NamePool::Ptr &np);
        bool retrieveDocument(const QUrl &uri);

        QHash<QUrl, AccelTree::Ptr> m_loadedDocuments;
        const NamePool::Ptr         m_namePool;
        static inline QString prefixFromQName(const QString &qName);
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
