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

class FilePorter
{
public:
    enum FileType {Header, Source};
    FilePorter(QString rulesFileName);
    void port(QString inBasePath, QString inFilePath, 
             QString outBasePath, QString outFilePath, FileType fileType );
private:
    QByteArray noPreprocess(const QString &fileName);
    QByteArray includeAnalyse(QByteArray file, FileType fileType);

    PortingRules portingRules;
    QList<TokenReplacement*> tokenReplacementRules;
    ReplaceToken replaceToken;
    QMap<QString, int> qt4HeaderNames;
};

#endif
