/****************************************************************************
**
** Definition of MetaMakefileGenerator class.
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
#ifndef __METAMAKEFILE_H__
#define __METAMAKEFILE_H__

class QMakeProject;
class MakefileGenerator;
#include <qlist.h>
#include <qstring.h>

class MetaMakefileGenerator 
{
    bool init_flag;
protected:
    bool init();
    struct Build {
        QString name;
        MakefileGenerator *makefile;
    };
    QMakeProject *project;
    QList<Build *> makefiles;
    static MakefileGenerator *createGenerator(QMakeProject *);
public:
    ~MetaMakefileGenerator() { clearBuilds(); }

    static MetaMakefileGenerator *createMetaGenerator(QMakeProject *);
    virtual bool write(const QString &);

private:
    void clearBuilds();
    MetaMakefileGenerator(QMakeProject *p) : init_flag(false) { project = p; }

    MakefileGenerator *processBuild(const QString &);
    MakefileGenerator *createMakefileGenerator(QMakeProject *proj);
};

#endif /* __METAMAKEFILE_H__ */
