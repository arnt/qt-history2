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

#ifndef RCC_H
#define RCC_H

#define TAG_RCC "RCC"
#define TAG_RESOURCE "qresource"
#define TAG_FILE "file"

#define ATTRIBUTE_LANG "lang"
#define ATTRIBUTE_PREFIX "prefix"
#define ATTRIBUTE_ALIAS "alias"

#define CONSTANT_HEADER_SIZE 8

class RCCFileInfo;

class RCCResourceLibrary
{
public:
    inline RCCResourceLibrary();
    ~RCCResourceLibrary();

    bool output(FILE *out);

    bool readFiles();

    enum Format { Binary, C_Code };
    inline void setFormat(Format f) { mFormat = f; }
    inline Format format() const { return mFormat; }

    inline void setInputFiles(QStringList files) { mFileNames = files; }
    inline QStringList inputFiles() const { return mFileNames; }

    QStringList dataFiles() const;

    inline void setVerbose(bool b) { mVerbose = b; }
    inline bool verbose() const { return mVerbose; }

    inline void setInitName(const QString &n) { mInitName = n; }
    inline QString initName() const { return mInitName; }

    inline void setCompressLevel(int c) { mCompressLevel = c; }
    inline int compressLevel() const { return mCompressLevel; }

    inline void setCompressThreshold(int t) { mCompressThreshold = t; }
    inline int compressThreshold() const { return mCompressThreshold; }

    inline void setResourceRoot(QString str) { mResourceRoot = str; }
    inline QString resourceRoot() const { return mResourceRoot; }

private:
    RCCFileInfo *root;
    bool addFile(const QString &alias, const RCCFileInfo &file);
    bool interpretResourceFile(QIODevice *inputDevice, QString currentPath = QString());

    bool writeHeader(FILE *out);
    bool writeDataBlobs(FILE *out);
    bool writeDataNames(FILE *out);
    bool writeDataStructure(FILE *out);
    bool writeInitializer(FILE *out);

    QStringList mFileNames;
    QString mResourceRoot, mInitName;
    Format mFormat;
    bool mVerbose;
    int mCompressLevel;
    int mCompressThreshold;
    int mTreeOffset, mNamesOffset, mDataOffset;
};

inline RCCResourceLibrary::RCCResourceLibrary()
{
    root = 0;
    mVerbose = false;
    mFormat = C_Code;
    mCompressLevel = -1;
    mCompressThreshold = 70;
    mTreeOffset = mNamesOffset = mDataOffset = 0;
}

struct RCCFileInfo
{
    enum Flags
    {
        NoFlags = 0x00,
        Compressed = 0x01,
        Directory = 0x02,
    };

    inline RCCFileInfo(QString name = QString(), QFileInfo fileInfo = QFileInfo(),
                       QLocale locale = QLocale(), uint flags = NoFlags);
    ~RCCFileInfo() { qDeleteAll(children); }
    inline QString resourceName() {
        QString resource = name;
        for(RCCFileInfo *p = parent; p; p = p->parent)
            resource = resource.prepend(p->name + "/");
        return ":" + resource;
    }

    int flags;
    QString name;
    QLocale locale;
    QFileInfo fileInfo;
    RCCFileInfo *parent;
    QHash<QString, RCCFileInfo*> children;

    qint64 nameOffset, dataOffset, childOffset;
    qint64 writeDataBlob(FILE *out, qint64 offset, int compression,
                         RCCResourceLibrary::Format format);
    qint64 writeDataName(FILE *out, qint64 offset, RCCResourceLibrary::Format format);
    bool   writeDataInfo(FILE *out, RCCResourceLibrary::Format format);
};

inline RCCFileInfo::RCCFileInfo(QString name, QFileInfo fileInfo, QLocale locale, uint flags)
{
    this->name = name;
    this->fileInfo = fileInfo;
    this->locale = locale;
    this->flags = flags;
    this->parent = 0;
    this->nameOffset = this->dataOffset = this->childOffset = 0;
}

#endif
