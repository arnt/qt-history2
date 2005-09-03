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

#ifndef METROWERKS_XML_H
#define METROWERKS_XML_H

#include "makefile.h"

class MetrowerksMakefileGenerator : public MakefileGenerator
{
    bool createFork(const QString &f);
    bool fixifyToMacPath(QString &c, QString &v, bool exists=true);

    bool init_flag;

    bool writeMakeParts(QTextStream &);
    bool writeSubDirs(QTextStream &);

    bool writeMakefile(QTextStream &);
    QString findTemplate(const QString &file);
    void init();
public:
    MetrowerksMakefileGenerator();
    ~MetrowerksMakefileGenerator();

    bool supportsMetaBuild() { return false; }
    bool openOutput(QFile &file, const QString &build) const;
protected:
    virtual void processPrlFiles();
    virtual void processPrlVariable(const QString &var, const QStringList &l);
    virtual bool doDepends() const { return false; } //never necesary
};

inline MetrowerksMakefileGenerator::~MetrowerksMakefileGenerator()
{ }

#endif // METROWERKS_XML_H
