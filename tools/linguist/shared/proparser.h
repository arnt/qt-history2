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

#ifndef PROPARSER_H
#define PROPARSER_H

#include "findsourcesvisitor.h"

// Subclass it to intercept the logMessage method
class ProFileTranslationsScanner : public FindSourcesVisitor {
public:
    ProFileTranslationsScanner(bool verbose) : FindSourcesVisitor() 
    {
        m_verbose = verbose;
    }

private:
    /* reimp */
    void logMessage(const QString &message, FindSourcesVisitor::MessageType mt) {
        if (m_verbose && (mt == FindSourcesVisitor::MT_DebugLevel1)) {
            Q_UNUSED(message);
            Q_UNUSED(mt);
            FindSourcesVisitor::logMessage(message, mt);
        }
    }

private:
    bool m_verbose;
};

#endif

void removeDuplicates(QStringList *strings, bool alreadySorted = true);

bool evaluateProFile(const QString &fileName, bool verbose, QMap<QByteArray, QStringList> *varMap);
