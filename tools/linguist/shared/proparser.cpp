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

#include "proparser.h"
#include "findsourcesvisitor.h"
#include <stdio.h>
#include <proreader.h>

#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QStringList>

// assumes that the list is sorted (or more correctly, that all equal elements are grouped together).
void removeDuplicates(QStringList *strings, bool alreadySorted /*= true*/)
{
    if (!alreadySorted) {
        strings->sort();
    }
    QString prev;
    QStringList::iterator it = strings->begin();
    while (it != strings->end()) {
        if (*it == prev) {
            it = strings->erase(it);
        }else{
            prev = *it;
            ++it;
        }
    }
}

bool evaluateProFile(const QString &fileName, bool verbose,QMap<QByteArray, QStringList> *varMap)
{
    bool ok = true;

    QStringList sourceFiles;
    QString codecForTr;
    QString codecForSource;
    QStringList tsFileNames;

    ProReader pr;
    ProFileTranslationsScanner *visitor = new ProFileTranslationsScanner(verbose);
    QFileInfo fi(fileName);
    QDir rootPath;
    ok = fi.exists();
    if (ok) {
        rootPath.setPath(fi.absolutePath());
        ProFile *pro = pr.read(fi.absoluteFilePath());
        ok = pro->Accept(visitor);
    }
    if (ok) {
        if (visitor->getTemplateType() == FindSourcesVisitor::TT_Subdirs) {
            QString oldPath = QDir::currentPath();
            QFileInfo fi(fileName);
            QDir::setCurrent(fi.absolutePath());
            QStringList subdirs = visitor->getVariable("SUBDIRS");
            for (int is = 0; is < subdirs.count() && ok; ++is) {
                QString subdir = subdirs[is];
                QDir dir( subdir );
                QStringList profiles = dir.entryList(QStringList() << "*.pro");
                if (profiles.count()) {
                    ProReader subreader;
                    ProFileTranslationsScanner *subvisitor = new ProFileTranslationsScanner(verbose);
                    QString profile = subdir + QLatin1Char('/') + profiles[0];
                    fi.setFile(profile);
                    ProFile *pro = subreader.read(fi.absoluteFilePath());
                    QString tmpPath = QDir::currentPath();
                    ok = pro->Accept(subvisitor);
                    if (ok) {
                        sourceFiles += subvisitor->expandVariableToAbsoluteFileNames(QLatin1String("SOURCES"), profile);
                        sourceFiles += subvisitor->expandVariableToAbsoluteFileNames(QLatin1String("HEADERS"), profile);

                        QStringList forms = subvisitor->expandVariableToAbsoluteFileNames(QLatin1String("INTERFACES"), profile)
                            + subvisitor->expandVariableToAbsoluteFileNames(QLatin1String("FORMS"), profile)
                            + subvisitor->expandVariableToAbsoluteFileNames(QLatin1String("FORMS3"), profile);
                        sourceFiles << forms;
                    }
                    delete subvisitor;
                }
            }
            QDir::setCurrent(oldPath);
        } else {
            // app/lib template
            sourceFiles += visitor->expandVariableToAbsoluteFileNames(QLatin1String("SOURCES"), fileName);
            sourceFiles += visitor->expandVariableToAbsoluteFileNames(QLatin1String("HEADERS"), fileName);

            tsFileNames << visitor->getVariable("TRANSLATIONS");

            QStringList trcodec = visitor->getVariable(QLatin1String("CODEC"))
                + visitor->getVariable(QLatin1String("DEFAULTCODEC"))
                + visitor->getVariable(QLatin1String("CODECFORTR"));
            if (!trcodec.isEmpty())
                codecForTr = trcodec.last().toLatin1();

            QStringList srccodec = visitor->getVariable(QLatin1String("CODECFORSRC"));
            if (!srccodec.isEmpty()) 
                codecForSource = srccodec.last().toLatin1();
            
            QStringList forms = visitor->expandVariableToAbsoluteFileNames(QLatin1String("INTERFACES"), fileName)
                + visitor->expandVariableToAbsoluteFileNames(QLatin1String("FORMS"), fileName)
                + visitor->expandVariableToAbsoluteFileNames(QLatin1String("FORMS3"), fileName);
            sourceFiles << forms;
        }
    }
    if (ok) {
        removeDuplicates(&sourceFiles, false);
        removeDuplicates(&tsFileNames, false);

        varMap->insert("SOURCES", sourceFiles);
        varMap->insert("CODECFORTR", QStringList() << codecForTr);
        varMap->insert("CODECFORSRC", QStringList() << codecForSource);
        varMap->insert("TRANSLATIONS", tsFileNames);
    }
    return ok;

}
