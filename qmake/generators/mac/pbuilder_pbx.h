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

#ifndef PBUILDER_PBX_H
#define PBUILDER_PBX_H

#include "unixmake.h"

class ProjectBuilderMakefileGenerator : public UnixMakefileGenerator
{
    bool writingUnixMakefileGenerator;
    QString pbx_dir;
    int pbuilderVersion() const;
    bool writeSubDirs(QTextStream &);
    bool writeMakeParts(QTextStream &);
    bool writeMakefile(QTextStream &);

    QString pbxbuild();
    QMap<QString, QString> keys;
    QString keyFor(const QString &file);
    QString fixForOutput(const QString &file);
    QStringList fixListForOutput(const QString &where);
    int     reftypeForFile(const QString &where);
    QString projectSuffix() const;
    enum { SettingsAsList=0x01, SettingsNoQuote=0x02 };
    inline QString writeSettings(QString var, QString val, int flags=0, int indent_level=0)
    { Q_UNUSED(indent_level); return writeSettings(var, QStringList(val), flags); }
    QString writeSettings(QString var, QStringList vals, int flags=0, int indent_level=0);

public:
    ProjectBuilderMakefileGenerator();
    ~ProjectBuilderMakefileGenerator();

    virtual bool supportsMetaBuild() { return false; }
    virtual bool openOutput(QFile &, const QString &) const;
protected:
    virtual QString escapeFilePath(const QString &path) const;
    bool doPrecompiledHeaders() const { return false; }
    virtual bool doDepends() const { return false; } //never necesary
};

inline ProjectBuilderMakefileGenerator::~ProjectBuilderMakefileGenerator()
{ }

#endif // PBUILDER_PBX_H
