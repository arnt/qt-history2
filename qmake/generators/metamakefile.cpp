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
#include "metamakefile.h"
#include "qregexp.h"
#include "qdir.h"
#include "makefile.h"
#include "project.h"

MetaMakefileGenerator::~MetaMakefileGenerator()
{

}

class BuildsMetaMakefileGenerator : public MetaMakefileGenerator
{
    bool init_flag;
private:
    struct Build {
        QString name;
        MakefileGenerator *makefile;
    };
    QList<Build *> makefiles;
    void clearBuilds();
    MakefileGenerator *processBuild(const QString &);

public:
    BuildsMetaMakefileGenerator(QMakeProject *p) : MetaMakefileGenerator(p), init_flag(false) { }
    virtual ~BuildsMetaMakefileGenerator() { clearBuilds(); }

    virtual bool init();
    virtual bool write(const QString &);
};

void
BuildsMetaMakefileGenerator::clearBuilds()
{
    for(int i = 0; i < makefiles.count(); i++) {
        Build *build = makefiles[i];
        delete build->makefile;
        delete build;
    }
    makefiles.clear();
}

bool
BuildsMetaMakefileGenerator::init()
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

bool
BuildsMetaMakefileGenerator::write(const QString &oldpwd)
{
    Build *glue = 0;
    if(!makefiles.isEmpty() && !makefiles.first()->name.isNull()) {
        glue = new Build;
        glue->makefile = createMakefileGenerator(project, true);
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
            if(!(Option::output.isOpen())) {
                if(Option::output.fileName() == "-") {
                    Option::output.setFileName("");
                    Option::output_dir = qmake_getpwd();
                    Option::output.open(QIODevice::WriteOnly | QIODevice::Text, stdout);
                    using_stdout = true;
                } else {
                    if(Option::output.fileName().isEmpty() && Option::qmake_mode == Option::QMAKE_GENERATE_MAKEFILE)
                        Option::output.setFileName(project->first("QMAKE_MAKEFILE"));
                    Option::output_dir = oldpwd;
                    if(!build->makefile->openOutput(Option::output, build->name)) {
                        fprintf(stderr, "Failure to open file: %s\n",
                                Option::output.fileName().isEmpty() ? "(stdout)" :
                                Option::output.fileName().toLatin1().constData());
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
                    debug_msg(1, "%s === %s", it.key().toLatin1().constData(),
                              it.value().join(" :: ").toLatin1().constData());
            }
        }
    }
    return ret;
}

MakefileGenerator
*BuildsMetaMakefileGenerator::processBuild(const QString &build)
{
    if(project) {
        debug_msg(1, "Meta Generator: Parsing '%s' for build [%s].",
                  project->projectFile().toLatin1().constData(),build.toLatin1().constData());

        //initialize the base
        QMap<QString, QStringList> basevars;
        if(!project->isEmpty(build + ".CONFIG"))
            basevars["CONFIG"] += project->values(build + ".CONFIG");
        basevars["CONFIG"] += build;
        basevars["CONFIG"] += "build_pass";
        basevars["BUILD_PASS"] = QStringList(build);
        QStringList buildname = project->values(build + ".name");
        basevars["BUILD_NAME"] = (buildname.isEmpty() ? QStringList(build) : buildname);

        //create project
        QMakeProject *build_proj = new QMakeProject(project->properities(), basevars);

        //all the user configs must be set after the "initial" files (qmake.conf, .qmake.cache, etc)
        const QStringList old_user_config = Option::user_configs;
        if(!project->isEmpty(build + ".CONFIG"))
            Option::user_configs += project->values(build + ".CONFIG");
        build_proj->read(project->projectFile());
        Option::user_configs = old_user_config;

        //done
        return createMakefileGenerator(build_proj);
    }
    return 0;
}

class SubdirsMetaMakefileGenerator : public MetaMakefileGenerator
{
    bool init_flag;
private:
    struct Subdir {
        QString input_dir;
        QString output_dir, output_file;
        MetaMakefileGenerator *makefile;
    };
    QList<Subdir *> subs;
    MakefileGenerator *processBuild(const QString &);

public:
    SubdirsMetaMakefileGenerator(QMakeProject *p) : MetaMakefileGenerator(p), init_flag(false) { }
    virtual ~SubdirsMetaMakefileGenerator();

    virtual bool init();
    virtual bool write(const QString &);
};

bool
SubdirsMetaMakefileGenerator::init()
{
    if(init_flag)
        return false;
    init_flag = true;

    if(Option::recursive) {
        const QString old_output_dir = Option::output_dir;
        const QString oldpwd = qmake_getpwd();
        const QStringList &subdirs = project->values("SUBDIRS");
        for(int i = 0; i < subdirs.size(); ++i) {
            Subdir *sub = new Subdir;
            subs.append(sub);

            bool subPro = false;
            QFileInfo subdir(subdirs.at(i));
            if(subdir.isDir())
                subdir = QFileInfo(subdirs.at(i) + "/" + subdir.fileName() + Option::pro_ext);
            else
                subPro = true;

            //handle sub project
            QMakeProject *sub_proj = new QMakeProject(project->properities());
            printf("RECURSIVE: reading %s\n", subdir.absoluteFilePath().toLatin1().constData());
            sub->input_dir = subdir.absolutePath();
            qmake_setpwd(sub->input_dir);
            sub->output_dir = qmake_getpwd(); //this is not going to work for shadow builds ### --Sam
            Option::output_dir = sub->output_dir;
            if(Option::output_dir.at(Option::output_dir.length()-1) != QLatin1Char('/'))
                Option::output_dir += QLatin1Char('/');
            sub_proj->read(subdir.fileName());
            sub->makefile = MetaMakefileGenerator::createMetaGenerator(sub_proj);
            Option::output_dir = old_output_dir;
            qmake_setpwd(oldpwd);

#if 0
            //handle output file
            if(!Option::output.fileName().isEmpty()) {
                QFileInfo output(Option::output.fileName());
                if(!output.isDir())
                    sub->output_file += output.fileName();
            }
            if(subPro)
                sub->output_file += "." + subdir.baseName();
#endif
        }
        Option::output_dir = old_output_dir;
        qmake_setpwd(oldpwd);
    }

    Subdir *self = new Subdir;
    self->input_dir = qmake_getpwd();
    self->output_dir = Option::output_dir;
    self->output_file = Option::output.fileName();
    self->makefile = new BuildsMetaMakefileGenerator(project);
    self->makefile->init();
    subs.append(self);
    return true;
}

bool
SubdirsMetaMakefileGenerator::write(const QString &)
{
    bool ret = true;
    const QString &oldpwd = qmake_getpwd();
    const QString &output_dir = Option::output_dir;
    const QString &output_name = Option::output.fileName();
    for(int i = 0; ret && i < subs.count(); i++) {
        qmake_setpwd(subs.at(i)->input_dir);
        Option::output_dir = QFileInfo(subs.at(i)->output_dir).absoluteFilePath();
        if(Option::output_dir.at(Option::output_dir.length()-1) != QLatin1Char('/'))
            Option::output_dir += QLatin1Char('/');
        Option::output.setFileName(subs.at(i)->output_file);
        if(i != subs.count()-1)
            printf("RECURSIVE: writing %s/%s\n", Option::output_dir.toLatin1().constData(),
                   Option::output.fileName().toLatin1().constData());
        if(!(ret = subs.at(i)->makefile->write(qmake_getpwd())))
            break;
        qmake_setpwd(oldpwd);
    }
    //restore because I'm paranoid
    Option::output.setFileName(output_name);
    Option::output_dir = output_dir;
    return ret;
}

SubdirsMetaMakefileGenerator::~SubdirsMetaMakefileGenerator()
{
    for(int i = 0; i < subs.count(); i++) {
        Subdir *sub = subs[i];
        delete sub->makefile;
        delete sub;
    }
    subs.clear();
}

//Factory things
#include "unixmake.h"
#include "mingw_make.h"
#include "projectgenerator.h"
#ifndef QMAKE_OPENSOURCE_EDITION
# include "msvc_nmake.h"
# include "borland_bmake.h"
# include "metrowerks_xml.h"
# include "pbuilder_pbx.h"
# include "msvc_dsp.h"
# include "msvc_vcproj.h"
#endif

MakefileGenerator *
MetaMakefileGenerator::createMakefileGenerator(QMakeProject *proj, bool noIO)
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
                proj->projectFile().toLatin1().constData());
    } else if(gen == "UNIX") {
        mkfile = new UnixMakefileGenerator;
    } else if(gen == "MINGW") {
        mkfile = new MingwMakefileGenerator;
#ifndef QMAKE_OPENSOURCE_EDITION
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
    } else if(gen == "METROWERKS") {
        mkfile = new MetrowerksMakefileGenerator;
    } else if(gen == "PROJECTBUILDER" || gen == "XCODE") {
        mkfile = new ProjectBuilderMakefileGenerator;
#endif
    } else {
        fprintf(stderr, "Unknown generator specified: %s\n", gen.toLatin1().constData());
    }
    if (mkfile) {
        mkfile->setNoIO(noIO);
        mkfile->setProjectFile(proj);
    }
    return mkfile;
}

MetaMakefileGenerator *
MetaMakefileGenerator::createMetaGenerator(QMakeProject *proj)
{
    MetaMakefileGenerator *ret = 0;
    if((Option::qmake_mode == Option::QMAKE_GENERATE_MAKEFILE ||
        Option::qmake_mode == Option::QMAKE_GENERATE_PRL)) {
        if(proj->first("TEMPLATE").endsWith("subdirs"))
            ret = new SubdirsMetaMakefileGenerator(proj);
    }
    if(!ret)
        ret = new BuildsMetaMakefileGenerator(proj);
    ret->init();
    return ret;
}

