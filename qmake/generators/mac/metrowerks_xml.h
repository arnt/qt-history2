/****************************************************************************
**
** Definition of MetrowerksMakefileGenerator class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of qmake.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef __METROWERKS_XML_H__
#define __METROWERKS_XML_H__

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
    MetrowerksMakefileGenerator(QMakeProject *p);
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

#endif /* __METROWERKS_XML_H__ */
