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
    if(builds.count() > 1 && Option::output.fileName() == "-") {
        use_single_build = true;
        warn_msg(WarnLogic, "Cannot direct to stdout when using multiple BUILDS.");
    } else if(0 && !use_single_build && project->first("TEMPLATE") == "subdirs") {
        use_single_build = true;
        warn_msg(WarnLogic, "Cannot specify multiple builds with TEMPLATE subdirs.");
    }
    if(!use_single_build) {
        for(int i = 0; i < builds.count(); i++) {
            QString build = builds[i];
            MakefileGenerator *makefile = processBuild(build);
            if(!makefile) 
                return false;
            if(!makefile->supportsMetaBuild()) {
                warn_msg(WarnLogic, "QMAKESPEC does not support multiple BUILDS.");
                clearBuilds();
                use_single_build = true;
                break;
            } else {
                Build *b = new Build;
                if(builds.count() != 1)
                    b->name = build;
                b->makefile = makefile;
                makefiles += b;
            }
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
    Build *glue = 0;
    if(!makefiles.first()->name.isNull()) {
        glue = new Build;
        glue->makefile = createMakefileGenerator(project);
        makefiles += glue;
    }

    bool ret = true;
    const QString &output_name = Option::output.fileName();
    for(int i = 0; ret && i < makefiles.count(); i++) {
        Option::output.setFileName(output_name);
        Build *build = makefiles[i];

        bool using_stdout = false;
        if(build->makefile && (Option::qmake_mode == Option::QMAKE_GENERATE_MAKEFILE ||
                               Option::qmake_mode == Option::QMAKE_GENERATE_PROJECT)
           && (!build->makefile->supportsMergedBuilds()
            || (build->makefile->supportsMergedBuilds() && (!glue || build == glue)))) {
            //open output
            if(!(Option::output.state() & IO_Open)) {
                if(Option::output.fileName() == "-") {
                    Option::output.setFileName("");
                    Option::output_dir = QDir::currentPath();
                    Option::output.open(IO_WriteOnly | IO_Translate, stdout);
                    using_stdout = true;
                } else {
                    if(Option::output.fileName().isEmpty() && Option::qmake_mode == Option::QMAKE_GENERATE_MAKEFILE)
                        Option::output.setFileName(project->first("QMAKE_MAKEFILE"));
                    Option::output_dir = oldpwd;
                    if(!build->makefile->openOutput(Option::output, build->name)) {
                        fprintf(stderr, "Failure to open file: %s\n",
                                Option::output.fileName().isEmpty() ? "(stdout)" : Option::output.fileName().latin1());
                        return false;
                    }
                }
            }
        } else {
           using_stdout = true; //kind of..
        }

        if(!build->makefile) {
            ret = false;
        } else if(build == glue) {
            ret = build->makefile->writeProjectMakefile();
        } else {
            ret = build->makefile->write();
            if (glue && glue->makefile->supportsMergedBuilds())
                ret = glue->makefile->mergeBuildProject(build->makefile);
        }
        if(!using_stdout) {
            Option::output.close();
            if(!ret)
                Option::output.remove();
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
    MakefileGenerator *mkfile = NULL;
    if(Option::qmake_mode == Option::QMAKE_GENERATE_PROJECT) {
        mkfile = new ProjectGenerator;
        mkfile->setProjectFile(proj);
        return mkfile;
    }


    QString gen = proj->first("MAKEFILE_GENERATOR");
    if(gen.isEmpty()) {
        fprintf(stderr, "No generator specified in config file: %s\n",
                proj->projectFile().latin1());
    } else if(gen == "UNIX") {
        mkfile = new UnixMakefileGenerator;
    } else if(gen == "MSVC") {
        // Visual Studio =< v6.0
        if(proj->first("TEMPLATE").indexOf(QRegExp("^vc.*")) != -1)
            mkfile = new DspMakefileGenerator;
        else
            mkfile = new NmakeMakefileGenerator;
    } else if(gen == "MSVC.NET") {
        // Visual Studio >= v7.0
        if(proj->first("TEMPLATE").indexOf(QRegExp("^vc.*")) != -1)
            mkfile = new VcprojGenerator;
        else
            mkfile = new NmakeMakefileGenerator;
    } else if(gen == "BMAKE") {
        mkfile = new BorlandMakefileGenerator;
    } else if(gen == "MINGW") {
        mkfile = new MingwMakefileGenerator;
    } else if(gen == "METROWERKS") {
        mkfile = new MetrowerksMakefileGenerator;
    } else if(gen == "PROJECTBUILDER") {
        mkfile = new ProjectBuilderMakefileGenerator;
    } else {
        fprintf(stderr, "Unknown generator specified: %s\n", gen.latin1());
    }
    mkfile->setProjectFile(proj);
    return mkfile;
}


MetaMakefileGenerator *
MetaMakefileGenerator::createMetaGenerator(QMakeProject *proj)
{
    MetaMakefileGenerator *ret = new MetaMakefileGenerator(proj);
    ret->init();
    return ret;
}
