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

#ifndef FILEPORTER_H
#define FILEPORTER_H

#include <QString>
#include <QMap>
#include "portingrules.h"
#include "replacetoken.h"
#include "filewriter.h"
#include "preprocessorcontrol.h"

class FilePorter
{
public:
    FilePorter(PreprocessorCache &preprocessorCache);
    void port(QString fileName);
private:
    QByteArray loadFile(const QString &fileName);
    QByteArray includeAnalyse(QByteArray fileContents);

    PreprocessorCache &preprocessorCache;
    QList<TokenReplacement*> tokenReplacementRules;
    ReplaceToken replaceToken;
    QMap<QString, int> qt4HeaderNames;
    Tokenizer tokenizer;    //used by includeAnalyse
};

#endif
