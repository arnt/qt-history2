/****************************************************************************
**
** Definition of ProjectGenerator class.
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
#ifndef __PROJECTGENERATOR_H__
#define __PROJECTGENERATOR_H__

#include "makefile.h"

class ProjectGenerator : public MakefileGenerator
{
    bool init_flag;
    bool addFile(QString);
    bool addConfig(const QString &, bool add=true);
    QString getWritableVar(const QString &, bool fixPath=true);
protected:
    virtual void init();
    virtual bool writeMakefile(QTextStream &);
public:
    ProjectGenerator(QMakeProject *p);
    ~ProjectGenerator();
    virtual bool openOutput(QFile &) const;
};

inline ProjectGenerator::~ProjectGenerator()
{ }


#endif /* __PROJECTGENERATOR_H__ */
