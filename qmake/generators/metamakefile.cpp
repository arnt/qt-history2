/****************************************************************************
**
** Definition of QWidget class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include "metamakefile.h"
#include "qregexp.h"
#include "qdir.h"
#include "makefile.h"
#include "project.h"

void
MetaMakefileGenerator::clearBuilds()
{
    for(int i = 0; i < makefiles.count(); i++) {
        Build *build = makefiles[i];
        delete build->makefile;
        delete build;
    }
    makefiles.clear();
}

bool
MetaMakefileGenerator::init()
{
    if(init_flag)
        return false;
    init_flag = true;

    const QStringList &builds = project->variables()["BUILDS"];
    bool use_single_build = builds.isEmpty();
    if(!use_single_build && Option::output.name() == "-") {
        use_single_build = true;
        warn_msg(WarnLogic, "Cannot direct to stdout when using multiple BUILDS.");
    } else if(0 && !use_single_build && project->first("TEMPLATE") == "subdirs") {
        use_single_build = true;
        warn_msg(WarnLogic, "Cannot specify multiple builds with TEMPLATE subdirs.");
    }
    if(!use_single_build) {
        for(QStringList::ConstIterator it = builds.begin(); it != builds.end(); ++it) {
            MakefileGenerator *makefile = processBuild((*it));
            if(!makefile) 
                return false;
            if(!makefile->supportsMetaBuild()) {
                warn_msg(WarnLogic, "QMAKESPEC does not support multiple BUILDS.");
                clearBuilds();
                use_single_build = true;
            }
            Build *build = new Build;
            build->name = (*it);
            build->makefile = makefile;
            makefiles += build;
        }
    }
    if(use_single_build) {
        Build *build = new Build;
        build->makefile = createMakefileGenerator(project);
        makefiles += build;
    }
    return true;
}

#include "metamakefile.h"

bool
MetaMakefileGenerator::write(const QString &oldpwd) 
{
    if(!init())
        return false;

    Build *glue = 0;
    if(makefiles.count() > 1) {
        glue = new Build;
        glue->makefile = createMakefileGenerator(project);
        makefiles += glue;
    }

    bool ret = true;
    const QString &output_name = Option::output.name();
    for(int i = 0; ret && i < makefiles.count(); i++) {
        Option::output.setName(output_name);
        Build *build = makefiles[i];

        bool using_stdout = false;
        if(build->makefile && (Option::qmake_mode == Option::QMAKE_GENERATE_MAKEFILE ||
                               Option::qmake_mode == Option::QMAKE_GENERATE_PROJECT)) {
            //open output
            if(!(Option::output.state() & IO_Open)) {
                if(Option::output.name() == "-") {
                    Option::output.setName("");
                    Option::output_dir = QDir::currentDirPath();
                    Option::output.open(IO_WriteOnly | IO_Translate, stdout);
                    using_stdout = true;
                } else {
                    if(Option::output.name().isEmpty() && Option::qmake_mode == Option::QMAKE_GENERATE_MAKEFILE)
                        Option::output.setName(project->first("QMAKE_MAKEFILE"));
                    Option::output_dir = oldpwd;
                    if(!build->makefile->openOutput(Option::output, build->name)) {
                        fprintf(stderr, "Failure to open file: %s\n",
                                Option::output.name().isEmpty() ? "(stdout)" : Option::output.name().latin1());
                        return false;
                    }
                }
            }
        } else {
            using_stdout = true; //kind of..
        }

        if(!build->makefile)
            ret = false;
        else if(build == glue)
            ret = build->makefile->writeProjectMakefile();
        else
            ret = build->makefile->write();
        if(!ret) {
            if(!using_stdout)
                QFile::remove(Option::output.name());
        } else {
            Option::output.close();
        }

        // debugging
        if(Option::debug_level) {
            QMap<QString, QStringList> &vars = project->variables();
            for(QMap<QString, QStringList>::Iterator it = vars.begin(); it != vars.end(); ++it) {
                if(!it.key().startsWith(".") && !it.value().isEmpty())
                    debug_msg(1, "%s === %s", it.key().latin1(), it.value().join(" :: ").latin1());
            }
        }
    }
    return ret;
}

MakefileGenerator
*MetaMakefileGenerator::processBuild(const QString &build)
{
    if(project) {
        //it is ugly how I just use this, but almost better than adding a weird parameter (IMHO)
        const QStringList old_user_config = Option::user_configs;
        if(!project->isEmpty(build + ".CONFIG"))
            Option::user_configs.prepend(project->values(build + ".CONFIG").join(" "));
        Option::user_configs.prepend(build);
        Option::user_configs.prepend("build_pass");
        QMakeProject *build_proj = new QMakeProject(project->properities());
        build_proj->read(project->projectFile());
        Option::user_configs = old_user_config; 
        build_proj->variables()["CONFIG"] += "no_autoqmake";
        return createMakefileGenerator(build_proj);
    }
    return 0;
}


//Factory thing
#include "unixmake.h"
#include "msvc_nmake.h"
#include "borland_bmake.h"
#include "mingw_make.h"
#include "msvc_dsp.h"
#include "msvc_vcproj.h"
#include "metrowerks_xml.h"
#include "pbuilder_pbx.h"
#include "projectgenerator.h"

MakefileGenerator *
MetaMakefileGenerator::createMakefileGenerator(QMakeProject *proj)
{
    if(Option::qmake_mode == Option::QMAKE_GENERATE_PROJECT)
        return new ProjectGenerator(proj);

    MakefileGenerator *mkfile = NULL;
    QString gen = proj->first("MAKEFILE_GENERATOR");
    if(gen.isEmpty()) {
        fprintf(stderr, "No generator specified in config file: %s\n",
                proj->projectFile().latin1());
    } else if(gen == "UNIX") {
        mkfile = new UnixMakefileGenerator(proj);
    } else if(gen == "MSVC") {
        // Visual Studio =< v6.0
        if(proj->first("TEMPLATE").indexOf(QRegExp("^vc.*")) != -1)
            mkfile = new DspMakefileGenerator(proj);
        else
            mkfile = new NmakeMakefileGenerator(proj);
    } else if(gen == "MSVC.NET") {
        // Visual Studio >= v7.0
        if(proj->first("TEMPLATE").indexOf(QRegExp("^vc.*")) != -1)
            mkfile = new VcprojGenerator(proj);
        else
            mkfile = new NmakeMakefileGenerator(proj);
    } else if(gen == "BMAKE") {
        mkfile = new BorlandMakefileGenerator(proj);
    } else if(gen == "MINGW") {
        mkfile = new MingwMakefileGenerator(proj);
    } else if(gen == "METROWERKS") {
        mkfile = new MetrowerksMakefileGenerator(proj);
    } else if(gen == "PROJECTBUILDER") {
        mkfile = new ProjectBuilderMakefileGenerator(proj);
    } else {
        fprintf(stderr, "Unknown generator specified: %s\n", gen.latin1());
    }
    return mkfile;
}


MetaMakefileGenerator *
MetaMakefileGenerator::createMetaGenerator(QMakeProject *proj)
{
#if 0
    if(gen == "MSVC" && proj->first("TEMPLATE").indexOf(QRegExp("^vc.*")) != -1) 
        return new VcprojMetaGenerator(proj);
#endif
    return new MetaMakefileGenerator(proj);
}
