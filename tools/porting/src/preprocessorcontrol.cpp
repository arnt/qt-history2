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

#include "preprocessorcontrol.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryFile>
using namespace TokenEngine;
using namespace Rpp;

IncludeFiles::IncludeFiles(const QString &basePath, const QStringList &searchPaths)
:m_basePath(basePath)
{
    //prepend basePath to all relative paths in searchPaths
    foreach (QString path, searchPaths) {
        QString finalPath;
        if (QDir::isAbsolutePath(path))
            finalPath = QDir::cleanPath(path);
        else
            finalPath = QDir::cleanPath(m_basePath + "/" + path);

        if(QFile::exists(finalPath))
            m_searchPaths.append(finalPath);
    }
}

/*
    Performs an #include "..." style file lookup.
    Aboslute filenames are checked directly. Relative filenames are first
    looked for relative to the current file path, then the includepaths
    are searched if not found.
*/
QString IncludeFiles::quoteLookup(const QString &currentFile,
                                  const QString &includeFile) const
{
    //if includeFile is absolute, check if it exists
    if(QDir::isAbsolutePath(includeFile))
        if(QFile::exists(includeFile))
            return includeFile;
        else
            return QString();

    //If currentFile is not an absolute file path, make it one by
    //prepending m_baspath
    QString currentFilePath;
    if(QDir::isAbsolutePath(currentFile))
        currentFilePath = currentFile;
    else
        currentFilePath = QDir::cleanPath(m_basePath + "/" + currentFile);

    //Check if it includeFile exists in the same dir as currentFilePath
    const QString currentPath = QFileInfo(currentFilePath).path();
    QString localFile = QDir::cleanPath(currentPath + "/" + includeFile);
    if(QFile::exists(localFile))
        return localFile;

    return searchIncludePaths(includeFile);
}

/*
    Performs an #include <...> style file lookup.
    Aboslute filenames are checked directly.
    Relative paths are searched for in the includepaths.
*/
QString IncludeFiles::angleBracketLookup(const QString &includeFile) const
{
    //if includeFile is absolute, check if it exists
    if(QDir::isAbsolutePath(includeFile))
        if(QFile::exists(includeFile))
            return includeFile;
        else
            return QString();

    return searchIncludePaths(includeFile);
}

QString IncludeFiles::resolve(const QString &filename) const
{
    if(QDir::isAbsolutePath(filename))
        return filename;

    QString prepended = QDir::cleanPath(m_basePath + "/" + filename);
    if(QFile::exists(prepended))
        return prepended;
    else
        return QString();
}


/*
    Searches for includeFile paths by appending it to all includePaths
    and checking if the file exists. Returns QString() if the file is not
    found.
*/
QString IncludeFiles::searchIncludePaths(const QString &includeFile) const
{
    QString foundFile;
    foreach(QString includePath, m_searchPaths) {
        QString testFile = includePath + "/" + includeFile;
        if(QFile::exists(testFile)){
            foundFile = testFile;
            break;
        }
    }
    return foundFile;
}

QByteArray PreprocessorCache::readFile(const QString &filename) const
{
    QFile f(filename);
    f.open(QIODevice::ReadOnly);
    if(!f.isOpen())
        return QByteArray();
    return f.readAll();
}

PreprocessorCache::PreprocessorCache()
{
    connect(&m_preprocessor, SIGNAL(error(QString, QString)),
            this, SIGNAL(error(QString, QString)));
}


/*
    Return a TokenSequence with the contents of filname.
    Assumens filename exists and is readable, returns a empty
    TokenSequence if not.

    The result is cached.
*/
TokenContainer PreprocessorCache::sourceTokens(const QString &filename)
{
//    cout << "sourceTokens: " << filename.toLatin1().constData() << endl;
    if(m_sourceTokens.contains(filename))
        return m_sourceTokens.value(filename);
    if(!QFile::exists(filename))
        return TokenContainer();

    QByteArray fileContents = readFile(filename);
    QList<TokenEngine::Token> tokenList = m_tokenizer.tokenize(fileContents);
    TokenContainer tokenContainer(fileContents, tokenList);
    m_sourceTokens.insert(filename, tokenContainer);
    return tokenContainer;
}

/*
    Return a Source* tree representing the contents of filename.
    Assumens filename exists and is readable, returns a empty
    Source object if not.

    The result is cached.
*/
Source *PreprocessorCache::sourceTree(const QString &filename)
{
    if(m_sourceTrees.contains(filename))
        return m_sourceTrees.value(filename);
    TokenContainer tokenContainer = sourceTokens(filename);
    QList<Type> tokenTypes = m_lexer.lex(tokenContainer);
    Source *source = m_preprocessor.parse(tokenContainer, tokenTypes, &m_memoryPool);
    source->setFileName(filename);

    if(tokenContainer.count() > 0) //don't cache empty files
        m_sourceTrees.insert(filename, source);

    return source;
}

PreprocessorController::PreprocessorController(IncludeFiles includeFiles,
        PreprocessorCache &preprocessorCache, DefineMap *activeDefinitions)
:m_includeFiles(includeFiles),
 m_preprocessorCache(preprocessorCache),
 m_activeDefinitions(activeDefinitions)
{
    //connect include callback
    connect(&m_rppTreeEvaluator,
        SIGNAL(includeCallback(Source *&, const Source *,
        const QString &, RppTreeEvaluator::IncludeType)),
        this,
        SLOT(includeSlot(Source *&, const Source *,
        const QString &, RppTreeEvaluator::IncludeType)));

    //connect error handlers
    connect(&m_preprocessorCache , SIGNAL(error(QString, QString)),
            this, SIGNAL(error(QString, QString)));


}

/*
    Callback from RppTreeEvaluator, called when we evaluate an #include
    directive. We do a filename lookup based on the type of include, and then ask
    the cache to give us the source tree for that file.
*/
void PreprocessorController::includeSlot(Source *&includee,
                                         const Source *includer,
                                         const QString &filename,
                                         RppTreeEvaluator::IncludeType includeType)
{
    QString newFilename;
    if(includeType==RppTreeEvaluator::QuoteInclude)
        newFilename = m_includeFiles.quoteLookup(includer->fileName(), filename);
    else //AngleBracketInclude
        newFilename = m_includeFiles.angleBracketLookup(filename);

    if (!QFile::exists(newFilename))
        emit error("Error", "Could not find file " + filename);

    includee = m_preprocessorCache.sourceTree(newFilename);
}

/*
    Preprocess file give by filename. Filename is resloved agains the basepath
    set in IncludeFiles.
*/
TokenSectionSequence PreprocessorController::evaluate(const QString &filename)
{
    QString resolvedFilename = m_includeFiles.resolve(filename);
    if(!QFile::exists(resolvedFilename))
        emit error("Error", "Could not find file: " + filename);
    Source *source  = m_preprocessorCache.sourceTree(resolvedFilename);
    return m_rppTreeEvaluator.evaluate(source, m_activeDefinitions);
}

QByteArray defaultDefines =
    "#define __attribute__(a...)  \n \
         #define __extension \n \
         #define __extension__ \n \
         #define __restrict \n \
         #define __restrict__      \n \
         #define __volatile         volatile\n \
         #define __volatile__       volatile\n \
         #define __inline             inline\n \
         #define __inline__           inline\n \
         #define __const               const\n \
         #define __const__             const\n \
         #define __asm               asm\n \
         #define __asm__             asm\n \
         #define __GNUC__                 2\n \
         #define __GNUC_MINOR__          95\n  ";


/*
    Returns a DefineMap containing the above macro definitions.
    The DefineMap will contain pointers to data stored in the cache
*/
Rpp::DefineMap *defaultMacros(PreprocessorCache &cache)
{
    DefineMap *defineMap = new DefineMap();
    //write out defualt macros to a temp file
    QTemporaryFile tempfile;
    tempfile.open();
    tempfile.write(defaultDefines);

    IncludeFiles *includeFiles = new IncludeFiles(QString(), QStringList());
    PreprocessorController preprocessorController(*includeFiles, cache, defineMap);
    //evaluate default macro file.
    preprocessorController.evaluate(tempfile.fileName());
    delete includeFiles;
    return defineMap;
}
