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
    virtual ~MetaMakefileGenerator() { clearBuilds(); }

    static MetaMakefileGenerator *createMetaGenerator(QMakeProject *);
    static MakefileGenerator *createMakefileGenerator(QMakeProject *proj);
    virtual bool write(const QString &);

private:
    void clearBuilds();
    MetaMakefileGenerator(QMakeProject *p) : init_flag(false) { project = p; }

    MakefileGenerator *processBuild(const QString &);
};

#endif /* __METAMAKEFILE_H__ */
