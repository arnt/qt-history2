#ifndef PREPROCESSORCONTROL_H
#define PREPROCESSORCONTROL_H
#include <QString>
#include <QStringList>
#include <QMap>
#include "tokenengine.h"
#include "tokenizer.h"
#include "rpplexer.h"
#include "rpptreeevaluator.h"
#include "rpp.h"

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

class PreprocessorCache
{
public:
    PreprocessorCache() {};
    TokenEngine::TokenContainer sourceTokens(const QString &filename);
    Rpp::Source *sourceTree(const QString &filename);
private:
    QByteArray readFile(const QString & filename) const;
    Tokenizer tokenizer;
    Rpp::RppLexer lexer;
    Rpp::Preprocessor preprocessor;
    TypedPool<Rpp::Item> memoryPool;
    QMap<QString, Rpp::Source *> m_sourceTrees;
    QMap<QString, TokenEngine::TokenContainer> m_sourceTokens;
};

class PreprocessorController: public QObject
{
Q_OBJECT
public:
    PreprocessorController(IncludeFiles includefiles,
                           PreprocessorCache &preprocessorCahce,
                           Rpp::DefineMap *activedefinitions );

    TokenEngine::TokenSectionSequence evaluate(const QString &filename);
public slots:
    void includeSlot(Rpp::Source *&includee, const Rpp::Source *includer,
         const QString &filename, Rpp::RppTreeEvaluator::IncludeType includeType);
private:
    IncludeFiles m_includeFiles;
    Rpp::RppTreeEvaluator m_rppTreeEvaluator;
    PreprocessorCache &m_preprocessorCache;
    Rpp::DefineMap *m_activeDefinitions;
};

#endif

