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

#ifndef PREPROCESSORCONTROL_H
#define PREPROCESSORCONTROL_H

#include "tokenengine.h"
#include "tokenizer.h"
#include "rpplexer.h"
#include "rpptreeevaluator.h"
#include "rpp.h"
#include <QString>
#include <QStringList>
#include <QHash>

class IncludeFiles
{
public:
    IncludeFiles(const QString &basePath, const QStringList &searchPaths);
    QString quoteLookup(const QString &currentFile,
                        const QString &includeFile)const;
    QString angleBracketLookup(const QString &includeFile) const;
    QString resolve(const QString &filename) const;
private:
    QString searchIncludePaths(const QString &includeFile)const;
    QStringList m_searchPaths;
    QString m_basePath;
};

class PreprocessorCache: public QObject
{
Q_OBJECT
public:
    PreprocessorCache();
    TokenEngine::TokenContainer sourceTokens(const QString &filename);
    Rpp::Source *sourceTree(const QString &filename);
    bool containsSourceTokens(const QString &filename);
    bool containsSourceTree(const QString &filename);
signals:
    void error(QString type, QString text);
    void readFile(QByteArray &contents, QString filename);
private:
    QByteArray readFile(const QString & filename) const;
    Tokenizer m_tokenizer;
    Rpp::RppLexer m_lexer;
    Rpp::Preprocessor m_preprocessor;
    TypedPool<Rpp::Item> m_memoryPool;
    QHash<QString, Rpp::Source *> m_sourceTrees;
    QHash<QString, TokenEngine::TokenContainer> m_sourceTokens;
};

class PreprocessorController: public QObject
{
Q_OBJECT
public:
    PreprocessorController(IncludeFiles includefiles,
                           PreprocessorCache &preprocessorCache,
                           QStringList preLoadFilesFilenames = QStringList());

    TokenEngine::TokenSectionSequence evaluate(const QString &filename, Rpp::DefineMap *activedefinitions);
public slots:
    void includeSlot(::Rpp::Source *&includee, const ::Rpp::Source *includer,
         const QString &filename, ::Rpp::RppTreeEvaluator::IncludeType includeType);
    void readFile(QByteArray &contents, QString filename);
signals:
    void error(QString type, QString text);
private:
    IncludeFiles m_includeFiles;
    Rpp::RppTreeEvaluator m_rppTreeEvaluator;
    PreprocessorCache &m_preprocessorCache;
    QHash<QString, QByteArray> m_preLoadFiles;
};

Rpp::DefineMap *defaultMacros(PreprocessorCache &preprocessorCache);

class StandardOutErrorHandler : public QObject
{
Q_OBJECT
public slots:
    void error(QString type, QString text);
};

class RppPreprocessor
{
public:
    RppPreprocessor(QString basePath, QStringList includePaths, QStringList preLoadFilesFilename = QStringList());
    ~RppPreprocessor();
    TokenEngine::TokenSectionSequence evaluate(const QString &filename);
private:
    IncludeFiles m_includeFiles;
    PreprocessorCache m_cache;
    Rpp::DefineMap *m_activeDefinitions;
    PreprocessorController m_controller;
    StandardOutErrorHandler m_errorHandler;
};

#endif
