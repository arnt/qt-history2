#ifndef FILEPORTER_H
#define FILEPORTER_H

#include <QString>
#include <QMap>
#include "rulesfromxml.h"
#include "replacetoken.h"
#include "filewriter.h"
class FilePorter
{
public:
    enum FileType {Header, Source};
    FilePorter(QString rulesFileName);
    void port(QString inBasePath, QString inFilePath, QString outBasePath, QString outFilePath, FileType fileType );
private:
    QByteArray noPreprocess(const QString &fileName);
    QByteArray includeAnalyse(QByteArray file, FileType fileType);

    RulesFromXml rulesFromXml;
    QList<TokenReplacement*> tokenReplacementRules;
    ReplaceToken replaceToken;
    QMap<QString, int> qt4HeaderNames;
};


#endif
