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
#ifndef __METAMAKEFILE_H__
#define __METAMAKEFILE_H__

class QMakeProject;
class MakefileGenerator;
#include <qlist.h>
#include <qstring.h>

class MetaMakefileGenerator
{
protected:
    MetaMakefileGenerator(QMakeProject *p) : project(p) { }
    QMakeProject *project;

public:

    virtual ~MetaMakefileGenerator();

    static MetaMakefileGenerator *createMetaGenerator(QMakeProject *);
    static MakefileGenerator *createMakefileGenerator(QMakeProject *proj, bool noIO = false);

    inline QMakeProject *projectFile() const { return project; }

    virtual bool init() = 0;
    virtual int type() const { return -1; }
    virtual bool write(const QString &) = 0;
};

#endif /* __METAMAKEFILE_H__ */
