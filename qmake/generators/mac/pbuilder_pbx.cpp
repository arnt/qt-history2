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

#include "pbuilder_pbx.h"
#include "option.h"
#include "meta.h"
#include <qdir.h>
#include <qregexp.h>
#include <stdlib.h>
#include <time.h>
#include "qtmd5.h"
#ifdef Q_OS_UNIX
#  include <sys/types.h>
#  include <sys/stat.h>
#endif

//#define GENERATE_AGGREGRATE_SUBDIR

// Note: this is fairly hacky, but it does the job...

ProjectBuilderMakefileGenerator::ProjectBuilderMakefileGenerator() : UnixMakefileGenerator()
{

}

bool
ProjectBuilderMakefileGenerator::writeMakefile(QTextStream &t)
{
    if(!project->variables()["QMAKE_FAILED_REQUIREMENTS"].isEmpty()) {
        /* for now just dump, I need to generated an empty xml or something.. */
        fprintf(stderr, "Project file not generated because all requirements not met:\n\t%s\n",
                var("QMAKE_FAILED_REQUIREMENTS").latin1());
        return true;
    }

    project->variables()["MAKEFILE"].clear();
    project->variables()["MAKEFILE"].append("Makefile");
    if(project->first("TEMPLATE") == "app" || project->first("TEMPLATE") == "lib")
        return writeMakeParts(t);
    else if(project->first("TEMPLATE") == "subdirs")
        return writeSubDirs(t);
    return false;
}

bool
ProjectBuilderMakefileGenerator::writeSubDirs(QTextStream &t)
{
    if(project->isActiveConfig("generate_pbxbuild_makefile")) {
        QString mkwrap = fileFixify(pbx_dir + Option::dir_sep + ".." + Option::dir_sep + project->first("MAKEFILE"),
                                    QDir::currentPath());
        QFile mkwrapf(mkwrap);
        if(mkwrapf.open(QIODevice::WriteOnly | QIODevice::Translate)) {
            debug_msg(1, "pbuilder: Creating file: %s", mkwrap.latin1());
            QTextStream mkwrapt(&mkwrapf);
            UnixMakefileGenerator::writeSubDirs(mkwrapt);
        }
    }

    //HEADER
    t << "// !$*UTF8*$!" << "\n"
      << "{" << "\n"
      << "\t" << "archiveVersion = 1;" << "\n"
      << "\t" << "classes = {" << "\n" << "\t" << "};" << "\n"
      << "\t" << "objectVersion = " << pbuilderVersion() << ";" << "\n"
      << "\t" << "objects = {" << endl;

    //SUBDIRS
    QStringList subdirs = project->variables()["SUBDIRS"];
    QString oldpwd = QDir::currentPath();
    QMap<QString, QStringList> groups;
    for(int subdir = 0; subdir < subdirs.count(); subdir++) {
        QFileInfo fi(Option::fixPathToLocalOS(subdirs[subdir], true));
        if(fi.exists()) {
            if(fi.isDir()) {
                QString profile = subdirs[subdir];
                if(!profile.endsWith(Option::dir_sep))
                    profile += Option::dir_sep;
                profile += fi.baseName() + ".pro";
                subdirs.append(profile);
            } else {
                QMakeProject tmp_proj;
                QString dir = fi.path(), fn = fi.fileName();
                if(!dir.isEmpty()) {
                    if(!QDir::setCurrent(dir))
                        fprintf(stderr, "Cannot find directory: %s\n", dir.latin1());
                }
                if(tmp_proj.read(fn)) {
                    if(Option::debug_level) {
                        QMap<QString, QStringList> &vars = tmp_proj.variables();
                        for(QMap<QString, QStringList>::Iterator it = vars.begin();
                            it != vars.end(); ++it) {
                            if(it.key().left(1) != "." && !it.value().isEmpty())
                                debug_msg(1, "%s: %s === %s", fn.latin1(), it.key().latin1(),
                                          it.value().join(" :: ").latin1());
                        }
                    }
                    if(tmp_proj.first("TEMPLATE") == "subdirs") {
                        subdirs += fileFixify(tmp_proj.variables()["SUBDIRS"]);
                    } else if(tmp_proj.first("TEMPLATE") == "app" || tmp_proj.first("TEMPLATE") == "lib") {
                        QString pbxproj = QDir::currentPath() + Option::dir_sep + tmp_proj.first("TARGET") + projectSuffix();
                        if(!QFile::exists(pbxproj)) {
                            warn_msg(WarnLogic, "Ignored (not found) '%s'", pbxproj.latin1());
                            goto nextfile; // # Dirty!
                        }
                        const QString project_key = keyFor(pbxproj + "_PROJECTREF");
                        project->variables()["QMAKE_PBX_SUBDIRS"] += pbxproj;
                        //PROJECTREF
                        {
                            bool in_root = true;
                            QString name = QDir::currentPath();
                            if(project->isActiveConfig("flat")) {
                                QString flat_file = fileFixify(name, oldpwd, Option::output_dir, FileFixifyRelative);
                                if(flat_file.indexOf(Option::dir_sep) != -1) {
                                    QStringList dirs = flat_file.split(Option::dir_sep);
                                    name = dirs.back();
                                }
                            } else {
                                QString flat_file = fileFixify(name, oldpwd, Option::output_dir, FileFixifyRelative);
                                if(QDir::isRelativePath(flat_file) && flat_file.indexOf(Option::dir_sep) != -1) {
                                    QString last_grp("QMAKE_SUBDIR_PBX_HEIR_GROUP");
                                    QStringList dirs = flat_file.split(Option::dir_sep);
                                    name = dirs.back();
                                    for(QStringList::Iterator dir_it = dirs.begin(); dir_it != dirs.end(); ++dir_it) {
                                        QString new_grp(last_grp + Option::dir_sep + (*dir_it)), new_grp_key(keyFor(new_grp));
                                        if(dir_it == dirs.begin()) {
                                            if(!groups.contains(new_grp))
                                                project->variables()["QMAKE_SUBDIR_PBX_GROUPS"].append(new_grp_key);
                                        } else {
                                            if(!groups[last_grp].contains(new_grp_key))
                                                groups[last_grp] += new_grp_key;
                                        }
                                        last_grp = new_grp;
                                    }
                                    groups[last_grp] += project_key;
                                    in_root = false;
                                }
                            }
                            if(in_root)
                                project->variables()["QMAKE_SUBDIR_PBX_GROUPS"] += project_key;
                            t << "\t\t" << project_key << " = {" << "\n"
                              << "\t\t\t" << "isa = PBXFileReference;" << "\n"
                              << "\t\t\t" << "lastKnownFileType = \"wrapper.pb-project\";" << "\n"
                              << "\t\t\t" << "name = " << tmp_proj.first("TARGET") << projectSuffix() << ";" << "\n"
                              << "\t\t\t" << "path = " << pbxproj << ";" << "\n"
                              << "\t\t\t" << "refType = 0;" << "\n"
                              << "\t\t\t" << "sourceTree = \"<absolute>\";" << "\n"
                              << "\t\t" << "};" << "\n";
                            //WRAPPER
                            t << "\t\t" << keyFor(pbxproj + "_WRAPPER") << " = {" << "\n"
                              << "\t\t\t" << "isa = PBXReferenceProxy;" << "\n";
                            if(tmp_proj.first("TEMPLATE") == "app")
                                t << "\t\t\t" << "fileType = wrapper.application;" << "\n"
                                  << "\t\t\t" << "path = " << tmp_proj.first("TARGET") << ".app;" << "\n";
                            else
                                t << "\t\t\t" << "fileType = \"compiled.mach-o.dylib\";" << "\n"
                                  << "\t\t\t" << "path = " << tmp_proj.first("TARGET") << ".dylib;" << "\n";
                            t << "\t\t\t" << "refType = 3;" << "\n"
                              << "\t\t\t" << "remoteRef = " << keyFor(pbxproj + "_WRAPPERREF") << ";" << "\n"
                              << "\t\t\t" << "sourceTree = BUILT_PRODUCTS_DIR;" << "\n"
                              << "\t\t" << "};" << "\n";
                            t << "\t\t" << keyFor(pbxproj + "_WRAPPERREF") << " = {" << "\n"
                              << "\t\t\t" << "containerPortal = " << project_key << ";" << "\n"
                              << "\t\t\t" << "isa = PBXContainerItemProxy;" << "\n"
                              << "\t\t\t" << "proxyType = 2;" << "\n"
//                              << "\t\t\t" << "remoteGlobalIDString = " << keyFor(pbxproj + "QMAKE_PBX_REFERENCE") << ";" << "\n"
                              << "\t\t\t" << "remoteGlobalIDString = " << keyFor(pbxproj + "QMAKE_PBX_REFERENCE!!!") << ";" << "\n"
                              << "\t\t\t" << "remoteInfo = " << tmp_proj.first("TARGET") << ";" << "\n"
                              << "\t\t" << "};" << "\n";
                            //PRODUCTGROUP
                            t << "\t\t" << keyFor(pbxproj + "_PRODUCTGROUP") << " = {" << "\n"
                              << "\t\t\t" << "children = (" << "\n"
                              << "\t\t\t\t" << keyFor(pbxproj + "_WRAPPER") << "\n"
                              << "\t\t\t" << ");" << "\n"
                              << "\t\t\t" << "isa = PBXGroup;" << "\n"
                              << "\t\t\t" << "name = Products;" << "\n"
                              << "\t\t\t" << "refType = 4;" << "\n"
                              << "\t\t\t" << "sourceTree = \"<group>\";" << "\n"
                              << "\t\t" << "};" << "\n";
                        }
#ifdef GENERATE_AGGREGRATE_SUBDIR
                        //TARGET (for aggregate)
                        {
                            //container
                            const QString container_proxy = keyFor(pbxproj + "_CONTAINERPROXY");
                            t << "\t\t" << container_proxy << " = {" << "\n"
                              << "\t\t\t" << "containerPortal = " << project_key << ";" << "\n"
                              << "\t\t\t" << "isa = PBXContainerItemProxy;" << "\n"
                              << "\t\t\t" << "proxyType = 1;" << "\n"
                              << "\t\t\t" << "remoteGlobalIDString = " << keyFor(pbxproj + "QMAKE_PBX_TARGET") << ";" << "\n"
                              << "\t\t\t" << "remoteInfo = " << tmp_proj.first("TARGET") << ";" << "\n"
                              << "\t\t" << "};" << "\n";
                            //targetref
                            t << "\t\t" << keyFor(pbxproj + "_TARGETREF") << " = {" << "\n"
                              << "\t\t\t" << "isa = PBXTargetDependency;" << "\n"
                              << "\t\t\t" << "name = \"" << tmp_proj.first("TARGET")
                              << " (from " << tmp_proj.first("TARGET") << projectSuffix() << ")\";" << "\n"
                              << "\t\t\t" << "targetProxy = " << container_proxy << ";" << "\n"
                              << "\t\t" << "};" << "\n";
                        }
#endif
                    }
                }
nextfile:
                QDir::setCurrent(oldpwd);
            }
        }
    }
    for(QMap<QString, QStringList>::Iterator grp_it = groups.begin(); grp_it != groups.end(); ++grp_it) {
        t << "\t\t" << keyFor(grp_it.key()) << " = {" << "\n"
          << "\t\t\t" << "isa = PBXGroup;" << "\n"
          << "\t\t\t" << "children = (" << "\n"
          << valGlue(grp_it.value(), "\t\t\t\t", ",\n\t\t\t\t", "\n")
          << "\t\t\t" << ");" << "\n"
          << "\t\t\t" << "name = \"" << grp_it.key().section(Option::dir_sep, -1) << "\";" << "\n"
          << "\t\t\t" << "refType = 4;" << "\n"
          << "\t\t" << "};" << "\n";
    }

    //DUMP EVERYTHING THAT TIES THE ABOVE TOGETHER
    //BUILDSTYLE
    QString active_buildstyle;
#if 0
    for(int as_release = 0; as_release < 2; as_release++)
#else
        bool as_release = !project->isActiveConfig("debug");
#endif
    {
        QString key = keyFor("QMAKE_SUBDIR_PBX_" + QString(as_release ? "RELEASE" : "DEBUG"));
        if(project->isActiveConfig("debug") != as_release)
            active_buildstyle = key;
        project->variables()["QMAKE_SUBDIR_PBX_BUILDSTYLES"].append(key);
        t << "\t\t" << key << " = {" << "\n"
          << "\t\t\t" << "buildRules = (" << "\n"
          << "\t\t\t" << ");" << "\n"
          << "\t\t\t" << "buildSettings = {" << "\n"
          << "\t\t\t\t" << "COPY_PHASE_STRIP = " << (as_release ? "YES" : "NO") << ";" << "\n";
        if(as_release)
            t << "\t\t\t\t" << "DEBUGGING_SYMBOLS = NO;" << "\n";
        t << "\t\t\t" << "};" << "\n"
          << "\t\t\t" << "isa = PBXBuildStyle;" << "\n"
          << "\t\t\t" << "name = " << (as_release ? "Deployment" : "Development") << ";" << "\n"
          << "\t\t" << "};" << "\n";
    }

#ifdef GENERATE_AGGREGRATE_SUBDIR
    //target
    t << "\t\t" << keyFor("QMAKE_SUBDIR_PBX_AGGREGATE_TARGET") << " = {" << "\n"
      << "\t\t\t" << "buidPhases = (" << "\n"
      << "\t\t\t" << ");" << "\n"
      << "\t\t\t" << "buildSettings = {" << "\n"
      << "\t\t\t\t" << "PRODUCT_NAME = " << project->variables()["TARGET"].first() << ";" << "\n"
      << "\t\t\t" << "};" << "\n"
      << "\t\t\t" << "dependencies = (" << "\n";
    {
        const QStringList &qmake_subdirs = project->variables()["QMAKE_PBX_SUBDIRS"];
        for(int i = 0; i < qmake_subdirs.count(); i++)
            t << "\t\t\t\t" << keyFor(qmake_subdirs[i] + "_TARGETREF") << "," << "\n";
    }
    t << "\t\t\t" << ");" << "\n"
      << "\t\t\t" << "isa = PBXAggregateTarget;" << "\n"
      << "\t\t\t" << "name = " << project->variables()["TARGET"].first() << ";" << "\n"
      << "\t\t\t" << "productName = " << project->variables()["TARGET"].first() << ";" << "\n"
      << "\t\t" << "};" << "\n";
#endif

    //ROOT_GROUP
    t << "\t\t" << keyFor("QMAKE_SUBDIR_PBX_ROOT_GROUP") << " = {" << "\n"
      << "\t\t\t" << "children = (" << "\n"
      << varGlue("QMAKE_SUBDIR_PBX_GROUPS", "\t\t\t\t", ",\n\t\t\t\t", "\n")
      << "\t\t\t" << ");" << "\n"
      << "\t\t\t" << "isa = PBXGroup;" << "\n"
      << "\t\t\t" << "refType = 4;" << "\n"
      << "\t\t\t" << "sourceTree = \"<group>\";" << "\n"
      << "\t\t" << "};" << "\n";

    //ROOT
    t << "\t\t" << keyFor("QMAKE_SUBDIR_PBX_ROOT") << " = {" << "\n"
      << "\t\t\t" << "buildSettings = {" << "\n"
      << "\t\t\t" << "};" << "\n"
      << "\t\t\t" << "buildStyles = (" << "\n"
      << varGlue("QMAKE_SUBDIR_PBX_BUILDSTYLES", "\t\t\t\t", ",\n\t\t\t\t", "\n")
      << "\t\t\t" << ");" << "\n"
      << "\t\t\t" << "isa = PBXProject;" << "\n"
      << "\t\t\t" << "mainGroup = " << keyFor("QMAKE_SUBDIR_PBX_ROOT_GROUP") << ";" << "\n"
      << "\t\t\t" << "projectDirPath = \"\";" << "\n"
      << "\t\t\t" << "projectReferences = (" << "\n";
    {
        QStringList &qmake_subdirs = project->variables()["QMAKE_PBX_SUBDIRS"];
        for(int i = 0; i < qmake_subdirs.count(); i++) {
            QString subdir = qmake_subdirs[i];
            t << "\t\t\t\t" << "{" << "\n"
              << "\t\t\t\t\t" << "ProductGroup = " << keyFor(subdir + "_PRODUCTGROUP") << ";" << "\n"
              << "\t\t\t\t\t" << "ProjectRef = " << keyFor(subdir + "_PROJECTREF") << ";" << "\n"
              << "\t\t\t\t" << "}," << "\n";
        }
    }
    t << "\t\t\t" << ");" << "\n"
      << "\t\t\t" << "targets = (" << "\n"
#ifdef GENERATE_AGGREGRATE_SUBDIR
      << "\t\t\t\t" << keyFor("QMAKE_SUBDIR_PBX_AGGREGATE_TARGET") << "\n"
#endif
      << "\t\t\t" << ");" << "\n"
      << "\t\t" << "};" << "\n";

    //FOOTER
    t << "\t" << "};" << "\n"
      << "\t" << "rootObject = " << keyFor("QMAKE_SUBDIR_PBX_ROOT") << ";" << "\n"
      << "}" << endl;

    return true;
}

class ProjectBuilderSources
{
    QString key, group;
public:
    ProjectBuilderSources(const QString &key, const QString &group=QString());
    QStringList files(QMakeProject *project) const;
    inline QString keyName() const { return key; }
    inline QString groupName() const { return group; }
};

ProjectBuilderSources::ProjectBuilderSources(const QString &k, 
                                             const QString &g) : key(k), group(g)
{
    if(group.isNull()) {
        if(k == "SOURCES")
            group = "Sources";
        else if(k == "HEADERS")
            group = "Headers";
        else if(k == "SRCMOC")
            group = "Sources [moc]";
        else if(k == "QMAKE_INTERNAL_INCLUDED_FILES")
            group = "Sources [qmake]";
        else
            fprintf(stderr, "No group available for %s!\n", k.latin1());
    }
}

QStringList 
ProjectBuilderSources::files(QMakeProject *project) const
{
    QStringList ret = project->variables()[key];
    if(key == "QMAKE_INTERNAL_INCLUDED_FILES") {
        QString pfile = project->projectFile();
        if(pfile != "(stdin)")
            ret.prepend(pfile);
    }
    if(key == "SOURCES" && project->first("TEMPLATE") == "app" && !project->isEmpty("ICON")) 
        ret.append(project->first("ICON"));
    return ret;
}


bool
ProjectBuilderMakefileGenerator::writeMakeParts(QTextStream &t)
{
    QStringList tmp;
    bool did_preprocess = false;

    //HEADER
    t << "// !$*UTF8*$!" << "\n"
      << "{" << "\n"
      << "\t" << "archiveVersion = 1;" << "\n"
      << "\t" << "classes = {" << "\n" << "\t" << "};" << "\n"
      << "\t" << "objectVersion = " << pbuilderVersion() << ";" << "\n"
      << "\t" << "objects = {" << endl;

    //MAKE QMAKE equivelant
    if(!project->isActiveConfig("no_autoqmake") && project->projectFile() != "(stdin)") {
        QString mkfile = pbx_dir + Option::dir_sep + "qt_makeqmake.mak";
        QFile mkf(mkfile);
        if(mkf.open(QIODevice::WriteOnly | QIODevice::Translate)) {
            debug_msg(1, "pbuilder: Creating file: %s", mkfile.latin1());
            QTextStream mkt(&mkf);
            writeHeader(mkt);
            mkt << "QMAKE    = "        <<
                (project->isEmpty("QMAKE_QMAKE") ? QString("$(QTDIR)/bin/qmake") :
                 var("QMAKE_QMAKE")) << endl;
            writeMakeQmake(mkt);
            mkf.close();
        }
        QString phase_key = keyFor("QMAKE_PBX_MAKEQMAKE_BUILDPHASE");
        mkfile = fileFixify(mkfile, QDir::currentPath());
        project->variables()["QMAKE_PBX_PRESCRIPT_BUILDPHASES"].append(phase_key);
        t << "\t\t" << phase_key << " = {" << "\n"
          << "\t\t\t" << "buildActionMask = 2147483647;" << "\n"
          << "\t\t\t" << "files = (" << "\n"
          << "\t\t\t" << ");" << "\n"
          << "\t\t\t" << "generatedFileNames = (" << "\n"
          << "\t\t\t" << ");" << "\n"
          << "\t\t\t" << "isa = PBXShellScriptBuildPhase;" << "\n"
          << "\t\t\t" << "name = \"Qt Qmake\";" << "\n"
          << "\t\t\t" << "neededFileNames = (" << "\n"
          << "\t\t\t" << ");" << "\n"
          << "\t\t\t" << "shellPath = /bin/sh;" << "\n"
          << "\t\t\t" << "shellScript = \"make -C " << QDir::currentPath() <<
            " -f " << mkfile << "\";" << "\n"
          << "\t\t" << "};" << "\n";
    }

    //DUMP SOURCES
    QMap<QString, QStringList> groups;
    QList<ProjectBuilderSources> sources;
    sources.append(ProjectBuilderSources("SOURCES"));
    sources.append(ProjectBuilderSources("HEADERS"));
    sources.append(ProjectBuilderSources("SRCMOC"));
    sources.append(ProjectBuilderSources("QMAKE_INTERNAL_INCLUDED_FILES"));
    if(!project->isEmpty("QMAKE_EXTRA_COMPILERS")) {
        const QStringList &quc = project->variables()["QMAKE_EXTRA_COMPILERS"];
        for(QStringList::ConstIterator it = quc.begin(); it != quc.end(); ++it) {
            QString tmp_out = project->first((*it) + ".output");
            if(project->isEmpty((*it) + ".output"))
                continue;
            const QStringList &inputs = project->values((*it) + ".input");
            for(int input = 0; input < inputs.size(); ++input) {
                if(project->isEmpty(inputs.at(input)))
                    continue;
                sources.append(ProjectBuilderSources(inputs.at(input), 
                                                     QString("Sources [") + inputs.at(input) + "]"));
            }
        }
    }
    for(int source = 0; source < sources.size(); ++source) {
        QStringList &src_list = project->variables()["QMAKE_PBX_" + sources.at(source).keyName()];
        QStringList &root_group_list = project->variables()["QMAKE_PBX_GROUPS"];

        QStringList files = fileFixify(sources.at(source).files(project));
        qDebug("%s -- %s -- %s", sources.at(source).keyName().latin1(), 
               sources.at(source).groupName().latin1(), files.join("::").latin1());
        for(int f = 0; f < files.count(); ++f) {
            bool buildable = false;
            if(sources.at(source).keyName() == "SOURCES") 
                buildable = true;
            else if(sources.at(source).keyName() == "SRCMOC") 
                buildable = (f < project->variables()["OBJMOC"].count());

            QString file = files[f];
            if(file.length() >= 2 && (file[0] == '"' || file[0] == '\'') && file[(int) file.length()-1] == file[0])
                file = file.mid(1, file.length()-2);
            if(file.endsWith(Option::cpp_moc_ext) || file.endsWith(Option::prl_ext))
                continue;

            bool in_root = true;
            QString src_key = keyFor(file), name = file;
            if(project->isActiveConfig("flat")) {
                QString flat_file = fileFixify(file, QDir::currentPath(), Option::output_dir, FileFixifyRelative);
                if(flat_file.indexOf(Option::dir_sep) != -1) {
                    QStringList dirs = flat_file.split(Option::dir_sep);
                    name = dirs.back();
                }
            } else {
                QString flat_file = fileFixify(file, QDir::currentPath(), Option::output_dir, FileFixifyRelative);
                if(QDir::isRelativePath(flat_file) && flat_file.indexOf(Option::dir_sep) != -1) {
                    QString last_grp("QMAKE_PBX_" + sources.at(source).groupName() + "_HEIR_GROUP");
                    QStringList dirs = flat_file.split(Option::dir_sep);
                    name = dirs.back();
                    dirs.pop_back(); //remove the file portion as it will be added via src_key
                    for(QStringList::Iterator dir_it = dirs.begin(); dir_it != dirs.end(); ++dir_it) {
                        QString new_grp(last_grp + Option::dir_sep + (*dir_it)), new_grp_key(keyFor(new_grp));
                        if(dir_it == dirs.begin()) {
                            if(!src_list.contains(new_grp_key))
                                src_list.append(new_grp_key);
                        } else {
                            if(!groups[last_grp].contains(new_grp_key))
                                groups[last_grp] += new_grp_key;
                        }
                        last_grp = new_grp;
                    }
                    groups[last_grp] += src_key;
                    in_root = false;
                }
            }
            if(in_root)
                src_list.append(src_key);
            //source reference
            t << "\t\t" << src_key << " = {" << "\n"
              << "\t\t\t" << "isa = PBXFileReference;" << "\n"
              << "\t\t\t" << "name = \"" << name << "\";" << "\n"
              << "\t\t\t" << "path = \"" << file << "\";" << "\n"
              << "\t\t\t" << "refType = " << reftypeForFile(file) << ";" << "\n";
            if (ideType() == MAC_XCODE) {
                QString filetype;
                for(QStringList::Iterator cppit = Option::cpp_ext.begin(); cppit != Option::cpp_ext.end(); ++cppit) {
                    if(file.endsWith((*cppit))) {
                        filetype = "sourcecode.cpp.cpp";
                        break;
                    }
                }
                if(!filetype.isNull())
                    t << "\t\t\t" << "lastKnownFileType = " << filetype << ";" << "\n";
            }
            t << "\t\t" << "};" << "\n";
            if(buildable) { //build reference
                QString obj_key = file + ".o";
                obj_key = keyFor(obj_key);
                t << "\t\t" << obj_key << " = {" << "\n"
                  << "\t\t\t" << "fileRef = " << src_key << ";" << "\n"
                  << "\t\t\t" << "isa = PBXBuildFile;" << "\n"
                  << "\t\t\t" << "settings = {" << "\n"
                  << "\t\t\t\t" << "ATTRIBUTES = (" << "\n"
                  << "\t\t\t\t" << ");" << "\n"
                  << "\t\t\t" << "};" << "\n"
                  << "\t\t" << "};" << "\n";
                project->variables()["QMAKE_PBX_OBJ"].append(obj_key);
            }
        }
        if(!src_list.isEmpty()) {
            QString group_key = keyFor(sources.at(source).groupName());
            if(root_group_list.indexOf(group_key) == -1)
                root_group_list += group_key;

            QStringList &group = groups[sources.at(source).groupName()];
            for(int src = 0; src < src_list.size(); ++src) {
                if(group.indexOf(src_list.at(src)) == -1) 
                    group += src_list.at(src);
            }
        }
    }
    for(QMap<QString, QStringList>::Iterator grp_it = groups.begin(); grp_it != groups.end(); ++grp_it) {
        t << "\t\t" << keyFor(grp_it.key()) << " = {" << "\n"
          << "\t\t\t" << "isa = PBXGroup;" << "\n"
          << "\t\t\t" << "children = (" << "\n"
          << valGlue(grp_it.value(), "\t\t\t\t", ",\n\t\t\t\t", "\n")
          << "\t\t\t" << ");" << "\n"
          << "\t\t\t" << "name = \"" << grp_it.key().section(Option::dir_sep, -1) << "\";" << "\n"
          << "\t\t\t" << "refType = 4;" << "\n"
          << "\t\t" << "};" << "\n";
    }

    //PREPROCESS BUILDPHASE (just a makefile)
    if(!project->isEmpty("UICIMPLS") || !project->isEmpty("SRCMOC") ||
        !project->isEmpty("YACCSOURCES") || !project->isEmpty("LEXSOURCES")) {
        QString mkfile = pbx_dir + Option::dir_sep + "qt_preprocess.mak";
        QFile mkf(mkfile);
        if(mkf.open(QIODevice::WriteOnly | QIODevice::Translate)) {
            did_preprocess = true;
            debug_msg(1, "pbuilder: Creating file: %s", mkfile.latin1());
            QTextStream mkt(&mkf);
            writeHeader(mkt);
            mkt << "MOC       = " << Option::fixPathToTargetOS(var("QMAKE_MOC")) << endl;
            mkt << "UIC       = " << Option::fixPathToTargetOS(var("QMAKE_UIC")) << endl;
            mkt << "LEX       = " << var("QMAKE_LEX") << endl;
            mkt << "LEXFLAGS  = " << var("QMAKE_LEXFLAGS") << endl;
            mkt << "YACC      = " << var("QMAKE_YACC") << endl;
            mkt << "YACCFLAGS = " << var("QMAKE_YACCFLAGS") << endl;
            mkt << "DEFINES       = "
                << varGlue("PRL_EXPORT_DEFINES","-D"," -D"," ")
                << varGlue("DEFINES","-D"," -D","") << endl;
            mkt << "INCPATH       = " << "-I" << specdir();
            if(!project->isActiveConfig("no_include_pwd")) {
                QString pwd = fileFixify(QDir::currentPath());
                if(pwd.isEmpty())
                    pwd = ".";
                mkt << " -I" << pwd;
            }
            mkt << varGlue("INCLUDEPATH"," -I", " -I", "") << endl;
            mkt << "DEL_FILE  = " << var("QMAKE_DEL_FILE") << endl;
            mkt << "MOVE      = " << var("QMAKE_MOVE") << endl << endl;
            mkt << "IMAGES = " << varList("QMAKE_IMAGE_COLLECTION") << endl;
            mkt << "MOCS = " << varList("SRCMOC") << endl;
            mkt << "PARSERS =";
            if(!project->isEmpty("YACCSOURCES")) {
                QStringList &yaccs = project->variables()["YACCSOURCES"];
                for(QStringList::Iterator yit = yaccs.begin(); yit != yaccs.end(); ++yit) {
                    QFileInfo fi((*yit));
                    mkt << " " << fi.path() << Option::dir_sep << fi.baseName()
                        << Option::yacc_mod << Option::cpp_ext.first();
                }
            }
            if(!project->isEmpty("LEXSOURCES")) {
                QStringList &lexs = project->variables()["LEXSOURCES"];
                for(QStringList::Iterator lit = lexs.begin(); lit != lexs.end(); ++lit) {
                    QFileInfo fi((*lit));
                    mkt << " " << fi.path() << Option::dir_sep << fi.baseName()
                        << Option::lex_mod << Option::cpp_ext.first();
                }
            }
            mkt << "\n";
            mkt << "preprocess: $(MOCS) $(PARSERS) compilers" << endl;
            mkt << "clean preprocess_clean: mocclean parser_clean compiler_clean" << endl << endl;
            mkt << "mocclean:" << "\n";
            if(!project->isEmpty("SRCMOC"))
                mkt << "\t-rm -f $(MOCS)" << "\n";
            mkt << "parser_clean:" << "\n";
            if(!project->isEmpty("YACCSOURCES") || !project->isEmpty("LEXSOURCES"))
                mkt << "\t-rm -f $(PARSERS)" << "\n";
            writeMocSrc(mkt, "HEADERS");
            writeMocSrc(mkt, "SOURCES");
            writeYaccSrc(mkt, "YACCSOURCES");
            writeLexSrc(mkt, "LEXSOURCES");
            writeExtraTargets(mkt);
            if(!project->isEmpty("QMAKE_EXTRA_COMPILERS")) {
                mkt << "compilers:";
                const QStringList &compilers = project->variables()["QMAKE_EXTRA_COMPILERS"];
                for(int compiler = 0; compiler < compilers.size(); ++compiler) {
                    QString tmp_out = project->first(compilers.at(compiler) + ".output");
                    if(project->isEmpty(compilers.at(compiler) + ".output"))
                        continue;
                    const QStringList &inputs = project->values(compilers.at(compiler) + ".input");
                    for(int input = 0; input < inputs.size(); ++input) {
                        if(project->isEmpty(inputs.at(input)))
                            continue;
                        if(input && !(input % 3))
                            mkt << "\\\n\t";
                        mkt << " " << replaceExtraCompilerVariables(tmp_out, inputs.at(input), QString());
                    }
                }
                mkt << endl;
                writeExtraCompilerTargets(mkt);
            }
            mkf.close();
        }
        mkfile = fileFixify(mkfile, QDir::currentPath());
        QString phase_key = keyFor("QMAKE_PBX_PREPROCESS_TARGET");
//        project->variables()["QMAKE_PBX_BUILDPHASES"].append(phase_key);
        project->variables()["QMAKE_PBX_PRESCRIPT_BUILDPHASES"].append(phase_key);
        t << "\t\t" << phase_key << " = {" << "\n"
          << "\t\t\t" << "buildActionMask = 2147483647;" << "\n"
          << "\t\t\t" << "files = (" << "\n"
          << "\t\t\t" << ");" << "\n"
          << "\t\t\t" << "generatedFileNames = (" << "\n"
          << varGlue("QMAKE_PBX_OBJ", "\t\t\t\t", ",\n\t\t\t\t", "\n")
          << "\t\t\t" << ");" << "\n"
          << "\t\t\t" << "isa = PBXShellScriptBuildPhase;" << "\n"
          << "\t\t\t" << "name = \"Qt Preprocessors\";" << "\n"
          << "\t\t\t" << "neededFileNames = (" << "\n"
          << varGlue("QMAKE_PBX_OBJ", "\t\t\t\t", ",\n\t\t\t\t", "\n")
          << "\t\t\t" << ");" << "\n"
          << "\t\t\t" << "shellPath = /bin/sh;" << "\n"
          << "\t\t\t" << "shellScript = \"make -C " << QDir::currentPath() <<
            " -f " << mkfile << "\";" << "\n"
          << "\t\t" << "};" << "\n";
   }

    //SOURCE BUILDPHASE
    if(!project->isEmpty("QMAKE_PBX_OBJ")) {
        QString grp = "Build Sources", key = keyFor(grp);
        project->variables()["QMAKE_PBX_BUILDPHASES"].append(key);
        t << "\t\t" << key << " = {" << "\n"
          << "\t\t\t" << "buildActionMask = 2147483647;" << "\n"
          << "\t\t\t" << "files = (" << "\n"
          << varGlue("QMAKE_PBX_OBJ", "\t\t\t\t", ",\n\t\t\t\t", "\n")
          << "\t\t\t" << ");" << "\n"
          << "\t\t\t" << "isa = PBXSourcesBuildPhase;" << "\n"
          << "\t\t\t" << "name = \"" << grp << "\";" << "\n"
          << "\t\t" << "};" << "\n";
    }

    if(!project->isActiveConfig("staticlib")) { //DUMP LIBRARIES
        QStringList &libdirs = project->variables()["QMAKE_PBX_LIBPATHS"];
        QString libs[] = { "QMAKE_LFLAGS", "QMAKE_LIBDIR_FLAGS", "QMAKE_LIBS", QString::null };
        for(int i = 0; !libs[i].isNull(); i++) {
            tmp = project->variables()[libs[i]];
            for(int x = 0; x < tmp.count();) {
                bool remove = false;
                QString library, name, opt = tmp[x].trimmed();
                if(opt.length() >= 2 && (opt[0] == '"' || opt[0] == '\'') && opt[(int) opt.length()-1] == opt[0])
                    opt = opt.mid(1, opt.length()-2);
                if(opt.startsWith("-L")) {
                    QString r = opt.right(opt.length() - 2);
                    fixForOutput(r);
                    libdirs.append(r);
                } else if(opt == "-prebind") {
                    project->variables()["QMAKE_DO_PREBINDING"].append("TRUE");
                    remove = true;
                } else if(opt.startsWith("-l")) {
                    name = opt.right(opt.length() - 2);
                    QString lib("lib" + name);
                    for(QStringList::Iterator lit = libdirs.begin(); lit != libdirs.end(); ++lit) {
                        if(project->isActiveConfig("link_prl")) {
                            /* This isn't real nice, but it is real usefull. This looks in a prl
                               for what the library will ultimately be called so we can stick it
                               in the ProjectFile. If the prl format ever changes (not likely) then
                               this will not really work. However, more concerning is that it will
                               encode the version number in the Project file which might be a bad
                               things in days to come? --Sam
                            */
                            QString lib_file = (*lit) + Option::dir_sep + lib;
                            if(QMakeMetaInfo::libExists(lib_file)) {
                                QMakeMetaInfo libinfo;
                                if(libinfo.readLib(lib_file)) {
                                    if(!libinfo.isEmpty("QMAKE_PRL_TARGET")) {
                                        library = (*lit) + Option::dir_sep + libinfo.first("QMAKE_PRL_TARGET");
                                        debug_msg(1, "pbuilder: Found library (%s) via PRL %s (%s)",
                                                  opt.latin1(), lib_file.latin1(), library.latin1());
                                        remove = true;
                                    }
                                }
                            }
                        }
                        if(!remove) {
                            QString extns[] = { ".dylib", ".so", ".a", QString::null };
                            for(int n = 0; !remove && !extns[n].isNull(); n++) {
                                QString tmp =  (*lit) + Option::dir_sep + lib + extns[n];
                                if(QFile::exists(tmp)) {
                                    library = tmp;
                                    debug_msg(1, "pbuilder: Found library (%s) via %s",
                                              opt.latin1(), library.latin1());
                                    remove = true;
                                }
                            }
                        }
                    }
                } else if(opt == "-framework") {
                    if(x == tmp.count()-1)
                        break;
                    QStringList &fdirs = project->variables()["QMAKE_FRAMEWORKDIR"];
                    if(fdirs.isEmpty())
                        fdirs << "/System/Library/Frameworks/" << "/Library/Frameworks/";
                    const QString framework = tmp[x+1];
                    for(int fdir = 0; fdir < fdirs.count(); fdir++) {
                        if(QFile::exists(fdirs[fdir] + QDir::separator() + framework + ".framework")) {
                            tmp.removeAt(x);
                            remove = true;
                            library = fdirs[fdir] + Option::dir_sep + framework + ".framework";
                            break;
                        }
                    }
                } else if(opt.left(1) != "-") {
                    if(QFile::exists(opt)) {
                        remove = true;
                        library = opt;
                    }
                }
                if(!library.isEmpty()) {
                    if(name.isEmpty()) {
                        int slsh = library.lastIndexOf(Option::dir_sep);
                        if(slsh != -1)
                            name = library.right(library.length() - slsh - 1);
                    }
                    library = fileFixify(library);
                    QString key = keyFor(library);
                    bool is_frmwrk = (library.endsWith(".framework"));
                    t << "\t\t" << key << " = {" << "\n"
                      << "\t\t\t" << "isa = " << (is_frmwrk ? "PBXFrameworkReference" : "PBXFileReference") << ";" << "\n"
                      << "\t\t\t" << "name = \"" << name << "\";" << "\n"
                      << "\t\t\t" << "path = \"" << library << "\";" << "\n"
                      << "\t\t\t" << "refType = " << reftypeForFile(library) << ";" << "\n"
                      << "\t\t" << "};" << "\n";
                    project->variables()["QMAKE_PBX_LIBRARIES"].append(key);
                    QString obj_key = library + ".o";
                    obj_key = keyFor(obj_key);
                    t << "\t\t" << obj_key << " = {" << "\n"
                      << "\t\t\t" << "fileRef = " << key << ";" << "\n"
                      << "\t\t\t" << "isa = PBXBuildFile;" << "\n"
                      << "\t\t\t" << "settings = {" << "\n"
                      << "\t\t\t" << "};" << "\n"
                      << "\t\t" << "};" << "\n";
                    project->variables()["QMAKE_PBX_BUILD_LIBRARIES"].append(obj_key);
                }
                if(remove)
                    tmp.removeAt(x);
                else
                    x++;
            }
            project->variables()[libs[i]] = tmp;
        }
    }
    //SUBLIBS BUILDPHASE (just another makefile)
    if(!project->isEmpty("SUBLIBS")) {
        QString mkfile = pbx_dir + Option::dir_sep + "qt_sublibs.mak";
        QFile mkf(mkfile);
        if(mkf.open(QIODevice::WriteOnly | QIODevice::Translate)) {
            debug_msg(1, "pbuilder: Creating file: %s", mkfile.latin1());
            QTextStream mkt(&mkf);
            writeHeader(mkt);
            mkt << "SUBLIBS= ";
            tmp = project->variables()["SUBLIBS"];
            for(int i = 0; i < tmp.count(); i++)
                t << "tmp/lib" << tmp[i] << ".a ";
            t << endl << endl;
            mkt << "sublibs: $(SUBLIBS)" << endl << endl;
            tmp = project->variables()["SUBLIBS"];
            for(int i = 0; i < tmp.count(); i++)
                t << "tmp/lib" << tmp[i] << ".a" << ":\n\t"
                  << var(QString("MAKELIB") + tmp[i]) << endl << endl;
            mkf.close();
        }
        QString phase_key = keyFor("QMAKE_PBX_SUBLIBS_BUILDPHASE");
        mkfile = fileFixify(mkfile, QDir::currentPath());
        project->variables()["QMAKE_PBX_PRESCRIPT_BUILDPHASES"].append(phase_key);
        t << "\t\t" << phase_key << " = {" << "\n"
          << "\t\t\t" << "buildActionMask = 2147483647;" << "\n"
          << "\t\t\t" << "files = (" << "\n"
          << "\t\t\t" << ");" << "\n"
          << "\t\t\t" << "generatedFileNames = (" << "\n"
          << "\t\t\t" << ");" << "\n"
          << "\t\t\t" << "isa = PBXShellScriptBuildPhase;" << "\n"
          << "\t\t\t" << "name = \"Qt Sublibs\";" << "\n"
          << "\t\t\t" << "neededFileNames = (" << "\n"
          << "\t\t\t" << ");" << "\n"
          << "\t\t\t" << "shellPath = /bin/sh;" << "\n"
          << "\t\t\t" << "shellScript = \"make -C " << QDir::currentPath() <<
            " -f " << mkfile << "\";" << "\n"
          << "\t\t" << "};" << "\n";
    }
    //LIBRARY BUILDPHASE
    if(!project->isEmpty("QMAKE_PBX_LIBRARIES")) {
        tmp = project->variables()["QMAKE_PBX_LIBRARIES"];
        if(!tmp.isEmpty()) {
            QString grp("External Frameworks and Libraries"), key = keyFor(grp);
            project->variables()["QMAKE_PBX_GROUPS"].append(key);
            t << "\t\t" << key << " = {" << "\n"
              << "\t\t\t" << "children = (" << "\n"
              << varGlue("QMAKE_PBX_LIBRARIES", "\t\t\t\t", ",\n\t\t\t\t", "\n")
              << "\t\t\t" << ");" << "\n"
              << "\t\t\t" << "isa = PBXGroup;" << "\n"
              << "\t\t\t" << "name = \"" << grp << "\"" << ";" << "\n"
              << "\t\t\t" << "path = \"\";" << "\n"
              << "\t\t\t" << "refType = 4;" << "\n"
              << "\t\t" << "};" << "\n";
        }
    }
    {
        QString grp("Frameworks & Libraries"), key = keyFor(grp);
        project->variables()["QMAKE_PBX_BUILDPHASES"].append(key);
        t << "\t\t" << key << " = {" << "\n"
          << "\t\t\t" << "buildActionMask = 2147483647;" << "\n"
          << "\t\t\t" << "files = (" << "\n"
          << varGlue("QMAKE_PBX_BUILD_LIBRARIES", "\t\t\t\t", ",\n\t\t\t\t", "\n")
          << "\t\t\t" << ");" << "\n"
          << "\t\t\t" << "isa = PBXFrameworksBuildPhase;" << "\n"
          << "\t\t\t" << "name = \"" << grp << "\";" << "\n"
          << "\t\t" << "};" << "\n";
    }
    if(!project->isActiveConfig("console") && project->first("TEMPLATE") == "app") { //BUNDLE RESOURCES
        QString grp("Bundle Resources"), key = keyFor(grp);
        project->variables()["QMAKE_PBX_BUILDPHASES"].append(key);
        t << "\t\t" << key << " = {" << "\n"
          << "\t\t\t" << "buildActionMask = 2147483647;" << "\n"
          << "\t\t\t" << "files = (" << "\n";
        if(!project->isEmpty("ICON")) {
            QString icon = project->first("ICON");
            if(icon.length() >= 2 && (icon[0] == '"' || icon[0] == '\'') && icon[(int)icon.length()-1] == icon[0])
                icon = icon.mid(1, icon.length()-2);
            t << "\t\t\t\t" << keyFor(icon) << "\n";
        }
        t << "\t\t\t" << ");" << "\n"
          << "\t\t\t" << "isa = PBXResourcesBuildPhase;" << "\n"
          << "\t\t\t" << "name = \"" << grp << "\";" << "\n"
          << "\t\t" << "};" << "\n";
    }
    { //INSTALL BUILDPHASE (copy)
        QString phase_key = keyFor("QMAKE_PBX_TARGET_COPY_PHASE");
        QString destDir = Option::output_dir;
        if (!project->isEmpty("QMAKE_ORIG_DESTDIR"))
            destDir = project->first("QMAKE_ORIG_DESTDIR");
        destDir = QFileInfo(destDir).absoluteFilePath();
        project->variables()["QMAKE_PBX_PRESCRIPT_BUILDPHASES"].append(phase_key);
        t << "\t\t" << phase_key << " = {\n"
          << "\t\t\tname = \"Project Copy\";\n"
          << "\t\t\tbuildActionMask = 2147483647;\n"
          << "\t\t\tdstPath = " << destDir << ";\n"
          << "\t\t\tdstSubfolderSpec = 0;\n"
          << "\t\t\tfiles = (\n"
          << "\t\t\t" << keyFor("QMAKE_PBX_TARGET_COPY_FILE") << ",\n"
          << "\t\t\t);\n"
          << "\t\t\tisa = PBXCopyFilesBuildPhase;\n"
          << "\t\t\trunOnlyForDeploymentPostprocessing = 0;\n"
          << "\t\t};\n"
          << "\t\t" << keyFor("QMAKE_PBX_TARGET_COPY_FILE")  << " = {\n"
          << "\t\t\tfileRef =  " <<  keyFor(pbx_dir + "QMAKE_PBX_REFERENCE") << ";\n"
          << "\t\t\tisa = PBXBuildFile;\n"
          << "\t\t\tsettings = {\n"
          << "\t\t\t};\n"
          << "\t\t};\n";
    }
    //BUNDLE_DATA BUILDPHASE (copy)
    if(!project->isEmpty("QMAKE_BUNDLE_DATA")) {
        QStringList bundle_file_refs;
        //all bundle data
        const QStringList &bundle_data = project->variables()["QMAKE_BUNDLE_DATA"];
        for(int i = 0; i < bundle_data.count(); i++) {
            QStringList pbx_files;
            //all files
            const QStringList &files = project->variables()[bundle_data[i] + ".files"];
            for(int file = 0; file < files.count(); file++) {
                QString file_ref_key = keyFor("QMAKE_PBX_BUNDLE_COPY_FILE_REF." + bundle_data[i] + "-" + files[file]);
                bundle_file_refs += file_ref_key;
                t << "\t\t" << file_ref_key << " = {" << "\n"
                  << "\t\t\t" << "isa = PBXFileReference;" << "\n"
                  << "\t\t\t" << "path = \"" << files[file] << "\";" << "\n"
                  << "\t\t\t" << "refType = " << reftypeForFile(files[file]) << ";" << "\n"
                  << "\t\t" << "};" << "\n";
                QString copy_file_key = keyFor("QMAKE_PBX_BUNDLE_COPY_FILE." + bundle_data[i] + "-" + files[file]);
                pbx_files += copy_file_key;
                t << "\t\t" <<  copy_file_key << " = {\n"
                  << "\t\t\tfileRef =  " <<  file_ref_key << ";\n"
                  << "\t\t\tisa = PBXBuildFile;\n"
                  << "\t\t\tsettings = {\n"
                  << "\t\t\t};\n"
                  << "\t\t};\n";
            }
            //the phase
            QString phase_key = keyFor("QMAKE_PBX_BUNDLE_COPY." + bundle_data[i]);
            project->variables()["QMAKE_PBX_PRESCRIPT_BUILDPHASES"].append(phase_key);
            t << "\t\t" << phase_key << " = {\n"
              << "\t\t\tname = \"Bundle Copy [" << bundle_data[i] << "]\";\n"
              << "\t\t\tbuildActionMask = 2147483647;\n"
              << "\t\t\tdstPath = " << project->first(bundle_data[i] + ".path") << ";\n"
              << "\t\t\tdstSubfolderSpec = 1;\n"
              << "\t\t\tfiles = (\n"
              << valGlue(pbx_files, "\t\t\t\t", ",\n\t\t\t\t", "\n")
              << "\t\t\t);\n"
              << "\t\t\tisa = PBXCopyFilesBuildPhase;\n"
              << "\t\t\trunOnlyForDeploymentPostprocessing = 0;\n"
              << "\t\t};\n";
        }
        QString bundle_copy_key = keyFor("QMAKE_PBX_BUNDLE_COPY");
        project->variables()["QMAKE_PBX_GROUPS"].append(bundle_copy_key);
        t << "\t\t" << bundle_copy_key << " = {" << "\n"
          << "\t\t\t" << "children = (" << "\n"
          << valGlue(bundle_file_refs, "\t\t\t\t", ",\n\t\t\t\t", "\n")
          << "\t\t\t" << ");" << "\n"
          << "\t\t\t" << "isa = PBXGroup;" << "\n"
          << "\t\t\t" << "name = \"Source [bundle data]\"" << ";" << "\n"
          << "\t\t\t" << "path = \"\";" << "\n"
          << "\t\t\t" << "refType = 4;" << "\n"
          << "\t\t" << "};" << "\n";
    }

    if(/*ideType() == MAC_XCODE &&*/ !project->isEmpty("QMAKE_PBX_PRESCRIPT_BUILDPHASES") && 0) {
        // build reference
        t << "\t\t" << keyFor("QMAKE_PBX_PRESCRIPT_BUILDREFERENCE") << " = {" << "\n"
          << "\t\t\t" << "includeInIndex = 0;" << "\n"
          << "\t\t\t" << "isa = PBXFileReference;" << "\n"
          << "\t\t\t" << "path = preprocessor.out;" << "\n"
          << "\t\t\t" << "refType = 3;" << "\n"
          << "\t\t\t" << "sourceTree = BUILT_PRODUCTS_DIR;" << "\n"
          << "\t\t" << "};" << "\n";
        project->variables()["QMAKE_PBX_PRODUCTS"].append(keyFor("QMAKE_PBX_PRESCRIPTS_BUILDREFERENCE"));
        //build phase
        t << "\t\t" << keyFor("QMAKE_PBX_PRESCRIPTS_BUILDPHASE") << " = {" << "\n"
          << "\t\t\t" << "buildPhases = (" << "\n"
          << varGlue("QMAKE_PBX_PRESCRIPT_BUILDPHASES", "\t\t\t\t", ",\n\t\t\t\t", "\n")
          << "\t\t\t" << ");" << "\n"
          << "\t\t\t" << "buildRules = (" << "\n"
          << "\t\t\t" << ");" << "\n"
          << "\t\t\t" << "buildSettings = {" << "\n"
          << "\t\t\t" << "};" << "\n"
          << "\t\t\t" << "dependencies = (" << "\n"
          << "\t\t\t" << ");" << "\n"
          << "\t\t\t" << "isa = PBXNativeTarget;" << "\n"
          << "\t\t\t" << "name = \"Qt Preprocessor Steps\";" << "\n"
          << "\t\t\t" << "productName = \"Qt Preprocessor Steps\";" << "\n"
          << "\t\t\t" << "productReference = " << keyFor("QMAKE_PBX_PRESCRIPTS_BUILDREFERENCE") << ";" << "\n"
          << "\t\t\t" << "productType = \"com.apple.product-type.tool\";" << "\n"
          << "\t\t" << "};" << "\n";
        //dependency
        t << "\t\t" << keyFor("QMAKE_PBX_PRESCRIPTS_DEPENDENCY") << " = {" << "\n"
          << "\t\t\t" << "isa = PBXTargetDependency;" << "\n"
          << "\t\t\t" << "target = " << keyFor("QMAKE_PBX_PRESCRIPTS_BUILDPHASE") << ";" << "\n"
          << "\t\t" << "};" << "\n";
        project->variables()["QMAKE_PBX_TARGET_DEPENDS"].append(keyFor("QMAKE_PBX_PRESCRIPTS_DEPENDENCY"));
        project->variables()["QMAKE_PBX_PRESCRIPT_BUILDPHASES"].clear(); //these are already consumed above
   }

    //DUMP EVERYTHING THAT TIES THE ABOVE TOGETHER
    //ROOT_GROUP
    t << "\t\t" << keyFor("QMAKE_PBX_ROOT_GROUP") << " = {" << "\n"
      << "\t\t\t" << "children = (" << "\n"
      << varGlue("QMAKE_PBX_GROUPS", "\t\t\t\t", ",\n\t\t\t\t", "\n")
      << "\t\t\t" << ");" << "\n"
      << "\t\t\t" << "isa = PBXGroup;" << "\n"
      << "\t\t\t" << "name = " << project->first("QMAKE_ORIG_TARGET") << ";" << "\n"
      << "\t\t\t" << "path = \"\";" << "\n"
      << "\t\t\t" << "refType = 4;" << "\n"
      << "\t\t" << "};" << "\n";
    //REFERENCE
    project->variables()["QMAKE_PBX_PRODUCTS"].append(keyFor(pbx_dir + "QMAKE_PBX_REFERENCE"));
    t << "\t\t" << keyFor(pbx_dir + "QMAKE_PBX_REFERENCE") << " = {" << "\n"
      << "\t\t\t" << "isa = PBXFileReference;" << "\n";
    if(project->first("TEMPLATE") == "app") {
        QString targ = project->first("QMAKE_ORIG_TARGET");
        if(project->isActiveConfig("resource_fork") && !project->isActiveConfig("console")) {
            targ += ".app";
            t << "\t\t\t" << "explicitFileType = wrapper.application;" << "\n";
        } else {
            t << "\t\t\t" << "explicitFileType  = wrapper.executable;" << "\n";
        }
        QString app = (!project->isEmpty("DESTDIR") ? project->first("DESTDIR") + project->first("QMAKE_ORIG_TARGET") :
                       QDir::currentPath()) + Option::dir_sep + targ;
        t << "\t\t\t" << "path = \"" << targ << "\";" << "\n";
    } else {
        QString lib = project->first("QMAKE_ORIG_TARGET");
        if(project->isActiveConfig("staticlib")) {
            lib = project->first("TARGET");
        } else if(!project->isActiveConfig("frameworklib")) {
            if(project->isActiveConfig("plugin"))
                lib = project->first("TARGET");
            else
                lib = project->first("TARGET_");
        }
        int slsh = lib.lastIndexOf(Option::dir_sep);
        if(slsh != -1)
            lib = lib.right(lib.length() - slsh - 1);
        t << "\t\t\t" << "explicitFileType = \"compiled.mach-o.dylib\";" << "\n"
          << "\t\t\t" << "path = " << lib << ";" << "\n";
    }
    t << "\t\t\t" << "refType = " << 3 << ";" << "\n"
      << "\t\t\t" << "sourceTree = BUILT_PRODUCTS_DIR" << ";" << "\n"
      << "\t\t" << "};" << "\n";
    { //Products group
        QString grp("Products"), key = keyFor(grp);
        project->variables()["QMAKE_PBX_GROUPS"].append(key);
        t << "\t\t" << key << " = {" << "\n"
          << "\t\t\t" << "children = (" << "\n"
          << varGlue("QMAKE_PBX_PRODUCTS", "\t\t\t\t", ",\n\t\t\t\t", "\n")
          << "\t\t\t" << ");" << "\n"
          << "\t\t\t" << "isa = PBXGroup;" << "\n"
          << "\t\t\t" << "name = Products;" << "\n"
          << "\t\t\t" << "refType = 4;" << "\n"
          << "\t\t" << "};" << "\n";
    }
    //TARGET
    QString target_key = keyFor(pbx_dir + "QMAKE_PBX_TARGET");
    project->variables()["QMAKE_PBX_TARGETS"].append(target_key);
    t << "\t\t" << target_key << " = {" << "\n"
      << "\t\t\t" << "buildPhases = (" << "\n"
      << varGlue("QMAKE_PBX_PRESCRIPT_BUILDPHASES", "\t\t\t\t", ",\n\t\t\t\t", ",\n")
      << varGlue("QMAKE_PBX_BUILDPHASES", "\t\t\t\t", ",\n\t\t\t\t", "\n")
      << "\t\t\t" << ");" << "\n"
      << "\t\t\t" << "buildSettings = {" << "\n"
      << "\t\t\t\t" << "CC = \"" << fixListForOutput("QMAKE_CC") << "\";" << "\n"
      << "\t\t\t\t" << "CPLUSPLUS = \"" << fixListForOutput("QMAKE_CXX") << "\";" << "\n"
      << "\t\t\t\t" << "FRAMEWORK_SEARCH_PATHS = \"\";" << "\n"
      << "\t\t\t\t" << "HEADER_SEARCH_PATHS = \"" << fixListForOutput("INCLUDEPATH") << " " << fixForOutput(specdir()) << "\";" << "\n"
      << "\t\t\t\t" << "LIBRARY_SEARCH_PATHS = \"" << var("QMAKE_PBX_LIBPATHS") << "\";" << "\n"
      << "\t\t\t\t" << "OPTIMIZATION_CFLAGS = \"\";" << "\n"
      << "\t\t\t\t" << "OTHER_CFLAGS = \"" <<
        fixListForOutput("QMAKE_CFLAGS") << fixForOutput(varGlue("PRL_EXPORT_DEFINES"," -D"," -D","")) <<
        fixForOutput(varGlue("DEFINES"," -D"," -D","")) << "\";" << "\n"
      << "\t\t\t\t" << "LEXFLAGS = \"" << var("QMAKE_LEXFLAGS") << "\";" << "\n"
      << "\t\t\t\t" << "YACCFLAGS = \"" << var("QMAKE_YACCFLAGS") << "\";" << "\n"
      << "\t\t\t\t" << "OTHER_CPLUSPLUSFLAGS = \"" <<
        fixListForOutput("QMAKE_CXXFLAGS") << fixForOutput(varGlue("PRL_EXPORT_DEFINES"," -D"," -D","")) <<
        fixForOutput(varGlue("DEFINES"," -D"," -D","")) << "\";" << "\n"
      << "\t\t\t\t" << "OTHER_REZFLAGS = \"\";" << "\n"
      << "\t\t\t\t" << "SECTORDER_FLAGS = \"\";" << "\n"
      << "\t\t\t\t" << "WARNING_CFLAGS = \"\";" << "\n"
      << "\t\t\t\t" << "PREBINDING = " << (project->isEmpty("QMAKE_DO_PREBINDING") ? "NO" : "YES") << ";" << "\n";
    if(!project->isEmpty("PRECOMPILED_HEADER")) {
        if (ideType() == MAC_XCODE) {
            t << "\t\t\t\t" << "GCC_PRECOMPILE_PREFIX_HEADER = \"YES\";" << "\n"
                << "\t\t\t\t" << "GCC_PREFIX_HEADER = \"" <<  project->first("PRECOMPILED_HEADER") << "\";" << "\n";
        } else {
            t << "\t\t\t\t" << "PRECOMPILE_PREFIX_HEADER = \"YES\";" << "\n"
                << "\t\t\t\t" << "PREFIX_HEADER = \"" <<  project->first("PRECOMPILED_HEADER") << "\";" << "\n";
        }
    }
    if(project->first("TEMPLATE") == "app") {
        QString plist = fileFixify(project->first("QMAKE_INFO_PLIST"));
        if(plist.isEmpty())
            plist = specdir() + QDir::separator() + "Info.plist." + project->first("TEMPLATE");
        if(QFile::exists(plist)) {
            QFile plist_in_file(plist);
            if(plist_in_file.open(QIODevice::ReadOnly)) {
                QTextStream plist_in(&plist_in_file);
                QString plist_in_text = plist_in.read();
                plist_in_text = plist_in_text.replace("@ICON@", 
                  (project->isEmpty("ICON") ? QString("") : project->first("ICON").section(Option::dir_sep, -1)));
                plist_in_text = plist_in_text.replace("@EXECUTABLE@", project->first("QMAKE_ORIG_TARGET"));
                plist_in_text = plist_in_text.replace("@TYPEINFO@",
                  (project->isEmpty("QMAKE_PKGINFO_TYPEINFO") ? QString::fromLatin1("????") : project->first("QMAKE_PKGINFO_TYPEINFO").left(4)));
                QFile plist_out_file("Info.plist");
                if(plist_out_file.open(QIODevice::WriteOnly | QIODevice::Translate)) {
                    QTextStream plist_out(&plist_out_file);
                    plist_out << plist_in_text;
                    t << "\t\t\t\t" << "INFOPLIST_FILE = \"Info.plist\";" << "\n";
                }
            }
        }
    }
#if 1
    t << "\t\t\t\t" << "BUILD_ROOT = \"" << QDir::currentPath() << "\";" << "\n";
#endif
    if(!project->isActiveConfig("staticlib"))
        t << "\t\t\t\t" << "OTHER_LDFLAGS = \"" << fixListForOutput("SUBLIBS") << " " <<
            fixListForOutput("QMAKE_LFLAGS") << " " << fixListForOutput("QMAKE_LIBDIR_FLAGS") <<
            " " << fixListForOutput("QMAKE_LIBS") << "\";" << "\n";
    if(!project->isEmpty("DESTDIR")) {
        QString dir = project->first("DESTDIR");
        if (QDir::isRelativePath(dir))
            dir.prepend(QDir::currentPath() + Option::dir_sep);
        t << "\t\t\t\t" << "INSTALL_DIR = \"" << dir << "\";" << "\n";
    }
    if (project->first("TEMPLATE") == "lib") {
        t << "\t\t\t\t" << "INSTALL_PATH = \"" <<  "\";" << "\n";
    }
    if(!project->isEmpty("VERSION") && project->first("VERSION") != "0.0.0") {
        t << "\t\t\t\t" << "DYLIB_CURRENT_VERSION = \"" << project->first("VER_MAJ") << "."
          << project->first("VER_MIN") << "." << project->first("VER_PAT")  << "\";" << "\n";
        if(project->isEmpty("COMPAT_VERSION"))
            t << "\t\t\t\t" << "DYLIB_COMPATIBILITY_VERSION = \"" << project->first("VER_MAJ") << "."
              << project->first("VER_MIN")  << "\";" << "\n";
    }
    if(!project->isEmpty("COMPAT_VERSION"))
        t << "\t\t\t\t" << "DYLIB_COMPATIBILITY_VERSION = \"" << project->first("COMPAT_VERSION") << "\";" << "\n";
    if(!project->isEmpty("QMAKE_MACOSX_DEPLOYMENT_TARGET"))
        t << "\t\t\t\t" << "MACOSX_DEPLOYMENT_TARGET = \""
          << project->first("QMAKE_MACOSX_DEPLOYMENT_TARGET") << "\";" << "\n";
    if(ideType() == MAC_XCODE) {
        if(!project->isEmpty("OBJECTS_DIR"))
            t << "\t\t\t\t" << "OBJROOT = \"" << project->first("OBJECTS_DIR") << "\";" << "\n";
    }
#if 0
    if(!project->isEmpty("DESTDIR"))
        t << "\t\t\t\t" << "SYMROOT = \"" << project->first("DESTDIR") << "\";" << "\n";
    else
        t << "\t\t\t\t" << "SYMROOT = \"" << QDir::currentPath() << "\";" << "\n";
#endif
    if(project->first("TEMPLATE") == "app") {
        if(ideType() == MAC_PBUILDER && !project->isActiveConfig("console"))
            t << "\t\t\t\t" << "WRAPPER_SUFFIX = app;" << "\n";
        t << "\t\t\t\t" << "PRODUCT_NAME = " << project->first("QMAKE_ORIG_TARGET") << ";" << "\n";
    } else {
        if(!project->isActiveConfig("plugin") && project->isActiveConfig("staticlib")) {
            t << "\t\t\t\t" << "LIBRARY_STYLE = STATIC;" << "\n";
        } else {
            t << "\t\t\t\t" << "LIBRARY_STYLE = DYNAMIC;" << "\n";
        }
        QString lib = project->first("QMAKE_ORIG_TARGET");
        if (!project->isActiveConfig("frameworklib") && !project->isActiveConfig("staticlib"))
            lib.prepend("lib");
        t << "\t\t\t\t" << "PRODUCT_NAME = " << lib << ";" << "\n";
    }
    tmp = project->variables()["QMAKE_PBX_VARS"];
    for(int i = 0; i < tmp.count(); i++) {
        QString var = tmp[i], val = qgetenv(var);
        if(val.isEmpty() && var == "TB")
            val = "/usr/bin/";
        t << "\t\t\t\t" << var << " = \"" << val << "\";" << "\n";
    }
    t << "\t\t\t" << "};" << "\n"
      << "\t\t\t" << "conditionalBuildSettings = {" << "\n"
      << "\t\t\t" << "};" << "\n"
      << "\t\t\t" << "dependencies = (" << "\n"
      << varGlue("QMAKE_PBX_TARGET_DEPENDS", "\t\t\t\t", ",\n\t\t\t\t", "\n")
      << "\t\t\t" << ");" << "\n"
      << "\t\t\t" << "productReference = " << keyFor(pbx_dir + "QMAKE_PBX_REFERENCE") << ";" << "\n"
      << "\t\t\t" << "shouldUseHeadermap = 1;" << "\n";
    if(ideType() == MAC_XCODE)
        t << "\t\t\t" << "isa = PBXNativeTarget;" << "\n";
    if(project->first("TEMPLATE") == "app") {
        if(project->isActiveConfig("console")) {
            if(ideType() == MAC_XCODE)
                t << "\t\t\t" << "productType = \"com.apple.product-type.tool\";" << "\n";
            else
                t << "\t\t\t" << "isa = PBXToolTarget;" << "\n";
        } else {
            if(ideType() == MAC_XCODE)
                t << "\t\t\t" << "productType = \"com.apple.product-type.application\";" << "\n";
            else
                t << "\t\t\t" << "isa = PBXApplicationTarget;" << "\n";
            t << "\t\t\t" << "productSettingsXML = \"";
            bool read_plist = false;
            if(QFile::exists("Info.plist")) {
                QFile plist("Info.plist");
                if(plist.open(QIODevice::ReadOnly)) {
                    read_plist = true;
                    QTextStream stream(&plist);
                    while(!stream.eof())
                        t << stream.readLine().replace('"', "\\\"") << endl;
                }
            }
            if(!read_plist) {
                t << "<?xml version="
                  << "\\\"1.0\\\" encoding=" << "\\\"UTF-8\\\"" << "?>" << "\n"
                  << "\t\t\t\t" << "<!DOCTYPE plist SYSTEM \\\"file://localhost/System/"
                  << "Library/DTDs/PropertyList.dtd\\\">" << "\n"
                  << "\t\t\t\t" << "<plist version=\\\"0.9\\\">" << "\n"
                  << "\t\t\t\t" << "<dict>" << "\n"
                  << "\t\t\t\t\t" << "<key>CFBundleDevelopmentRegion</key>" << "\n"
                  << "\t\t\t\t\t" << "<string>English</string>" << "\n"
                  << "\t\t\t\t\t" << "<key>CFBundleExecutable</key>" << "\n"
                  << "\t\t\t\t\t" << "<string>" << project->first("QMAKE_ORIG_TARGET") << "</string>" << "\n"
                  << "\t\t\t\t\t" << "<key>CFBundleIconFile</key>" << "\n"
                  << "\t\t\t\t\t" << "<string>" << var("ICON").section(Option::dir_sep, -1) << "</string>" << "\n"
                  << "\t\t\t\t\t" << "<key>CFBundleInfoDictionaryVersion</key>"  << "\n"
                  << "\t\t\t\t\t" << "<string>6.0</string>" << "\n"
                  << "\t\t\t\t\t" << "<key>CFBundlePackageType</key>" << "\n"
                  << "\t\t\t\t\t" << "<string>APPL</string>" << "\n"
                  << "\t\t\t\t\t" << "<key>CFBundleSignature</key>" << "\n"
                  << "\t\t\t\t\t" << "<string>"
                  << (project->isEmpty("QMAKE_PKGINFO_TYPEINFO") ? QString::fromLatin1("????") :
                      project->first("QMAKE_PKGINFO_TYPEINFO").left(4)) << "</string>" << "\n"
                  << "\t\t\t\t\t" << "<key>CFBundleVersion</key>" << "\n"
                  << "\t\t\t\t\t" << "<string>0.1</string>" << "\n"
                  << "\t\t\t\t\t" << "<key>CSResourcesFileMapped</key>" << "\n"
                  << "\t\t\t\t\t" << "<true/>" << "\n"
                  << "\t\t\t\t" << "</dict>" << "\n"
                  << "\t\t\t\t" << "</plist>";
            }
            t << "\";" << "\n";
        }
        t << "\t\t\t" << "name = \"" << project->first("QMAKE_ORIG_TARGET") << "\";" << "\n"
          << "\t\t\t" << "productName = " << project->first("QMAKE_ORIG_TARGET") << ";" << "\n";
    } else {
        QString lib = project->first("QMAKE_ORIG_TARGET");
        if(!project->isActiveConfig("frameworklib") && !project->isActiveConfig("staticlib"))
           lib.prepend("lib");
        t << "\t\t\t" << "name = \"" << lib << "\";" << "\n"
          << "\t\t\t" << "productName = " << lib << ";" << "\n";
        if(ideType() == MAC_XCODE) {
            if(project->isActiveConfig("staticlib"))
                t << "\t\t\t" << "productType = \"com.apple.product-type.library.static\";" << "\n";
            else
                t << "\t\t\t" << "productType = \"com.apple.product-type.library.dynamic\";" << "\n";
        } else {
            t << "\t\t\t" << "isa = PBXLibraryTarget;" << "\n";
        }
    }
    t << "\t\t\t" << "startupPath = \"<<ProjectDirectory>>\";" << "\n";
    if(!project->isEmpty("DESTDIR"))
        t << "\t\t\t" << "productInstallPath = \"" << project->first("DESTDIR") << "\";" << "\n";
    t << "\t\t" << "};" << "\n";
    //DEBUG/RELEASE
    QString active_buildstyle;
#if 0
    for(int as_release = 0; as_release < 2; as_release++)
#else
        bool as_release = !project->isActiveConfig("debug");
#endif
    {
        QString key = keyFor("QMAKE_PBX_" + QString(as_release ? "RELEASE" : "DEBUG"));
        if(project->isActiveConfig("debug") != as_release)
            active_buildstyle = key;
        project->variables()["QMAKE_PBX_BUILDSTYLES"].append(key);
        t << "\t\t" << key << " = {" << "\n"
          << "\t\t\t" << "buildRules = (" << "\n"
          << "\t\t\t" << ");" << "\n"
          << "\t\t\t" << "buildSettings = {" << "\n"
          << "\t\t\t\t" << "COPY_PHASE_STRIP = " << (as_release ? "YES" : "NO") << ";" << "\n";
        if(as_release) {
            t << "\t\t\t\t" << "DEBUGGING_SYMBOLS = NO;" << "\n";
        } else {
            t << "\t\t\t\t" << "GCC_ENABLE_FIX_AND_CONTINUE = "
              << (project->isActiveConfig("no_fix_and_continue") ? "NO" : "YES") << ";" << "\n"
              << "\t\t\t\t" << "GCC_GENERATE_DEBUGGING_SYMBOLS = YES;" << "\n"
              << "\t\t\t\t" << "GCC_OPTIMIZATION_LEVEL = 0;" << "\n"
              << "\t\t\t\t" << "ZERO_LINK ="
              << (project->isActiveConfig("no_zero_link") ? "NO" : "YES") << ";" << "\n";
        }
        t << "\t\t\t" << "};" << "\n"
          << "\t\t\t" << "isa = PBXBuildStyle;" << "\n"
          << "\t\t\t" << "name = " << (as_release ? "Deployment" : "Development") << ";" << "\n"
          << "\t\t" << "};" << "\n";
    }
    //ROOT
    t << "\t\t" << keyFor("QMAKE_PBX_ROOT") << " = {" << "\n"
      << "\t\t\t" << "buildStyles = (" << "\n"
      << varGlue("QMAKE_PBX_BUILDSTYLES", "\t\t\t\t", ",\n\t\t\t\t", "\n")
      << "\t\t\t" << ");" << "\n"
      << "\t\t\t" << "hasScannedForEncodings = 1;" << "\n"
      << "\t\t\t" << "isa = PBXProject;" << "\n"
      << "\t\t\t" << "mainGroup = " << keyFor("QMAKE_PBX_ROOT_GROUP") << ";" << "\n"
      << "\t\t\t" << "projectDirPath = \"\";" << "\n"
      << "\t\t\t" << "targets = (" << "\n"
      << varGlue("QMAKE_PBX_TARGETS", "\t\t\t\t", ",\n\t\t\t\t", "\n")
      << "\t\t\t" << ");" << "\n"
      << "\t\t" << "};" << "\n";

    //FOOTER
    t << "\t" << "};" << "\n"
      << "\t" << "rootObject = " << keyFor("QMAKE_PBX_ROOT") << ";" << "\n"
      << "}" << endl;

    if(project->isActiveConfig("generate_pbxbuild_makefile")) {
        QString mkwrap = fileFixify(pbx_dir + Option::dir_sep + ".." + Option::dir_sep + project->first("MAKEFILE"),
                                    QDir::currentPath());
        QFile mkwrapf(mkwrap);
        if(mkwrapf.open(QIODevice::WriteOnly | QIODevice::Translate)) {
            debug_msg(1, "pbuilder: Creating file: %s", mkwrap.latin1());
            QTextStream mkwrapt(&mkwrapf);
            writeHeader(mkwrapt);
            const char *cleans = "mocclean preprocess_clean ";
            mkwrapt << "#This is a makefile wrapper for PROJECT BUILDER\n"
                    << "all:" << "\n\t"
                    << "cd " << project->first("QMAKE_ORIG_TARGET") << projectSuffix() << "/ && " << pbxbuild() << "\n"
                    << "install: all" << "\n\t"
                    << "cd " << project->first("QMAKE_ORIG_TARGET") << projectSuffix() << "/ && " << pbxbuild() << " install\n"
                    << "distclean clean: preprocess_clean" << "\n\t"
                    << "cd " << project->first("QMAKE_ORIG_TARGET") << projectSuffix() << "/ && " << pbxbuild() << " clean" << "\n"
                    << (!did_preprocess ? cleans : "") << ":" << "\n";
            if(did_preprocess)
                mkwrapt << cleans << ":" << "\n\t"
                        << "make -f "
                        << pbx_dir << Option::dir_sep << "qt_preprocess.mak $@" << endl;
        }
    }
    return true;
}

QString
ProjectBuilderMakefileGenerator::fixForOutput(const QString &values)
{
    //get the environment variables references
    QRegExp reg_var("\\$\\((.*)\\)");
    for(int rep = 0; (rep = reg_var.indexIn(values, rep)) != -1;) {
        if(project->variables()["QMAKE_PBX_VARS"].indexOf(reg_var.cap(1)) == -1)
            project->variables()["QMAKE_PBX_VARS"].append(reg_var.cap(1));
        rep += reg_var.matchedLength();
    }
    QString ret = values;
    ret = ret.replace(QRegExp("('|\")"), "\\\\1"); //fix quotes
    ret = ret.replace("\t", "    "); //fix tabs
    return ret;
}

QString
ProjectBuilderMakefileGenerator::fixListForOutput(const QString &where)
{
    QString ret;
    const QStringList &l = project->variables()[where];
    for(int i = 0; i < l.count(); i++) {
        if(!ret.isEmpty())
            ret += " ";
        ret += fixForOutput(l[i]);
    }
    return ret;
}

QString
ProjectBuilderMakefileGenerator::keyFor(const QString &block)
{
#if 1 //This make this code much easier to debug..
    if(project->isActiveConfig("no_pb_munge_key"))
       return block;
#endif
    QString ret;
    if(!keys.contains(block)) {
        ret = qtMD5(block.utf8()).left(24).toUpper();
        keys.insert(block, ret);
    } else {
        ret = keys[block];
    }
    return ret;
}

bool
ProjectBuilderMakefileGenerator::openOutput(QFile &file, const QString &build) const
{
    if(QDir::isRelativePath(file.fileName()))
        file.setFileName(Option::output_dir + file.fileName()); //pwd when qmake was run
    QFileInfo fi(file);
    if(fi.suffix() != "pbxproj" || file.fileName().isEmpty()) {
        QString output = file.fileName();
        if(fi.isDir())
            output += QDir::separator();
        if(!output.endsWith(projectSuffix())) {
            if(file.fileName().isEmpty() || fi.isDir())
                output += project->first("QMAKE_ORIG_TARGET");
            output += projectSuffix() + QDir::separator();
        } else if(output[(int)output.length() - 1] != QDir::separator()) {
            output += QDir::separator();
        }
        output += QString("project.pbxproj");
        file.setFileName(output);
    }
    bool ret = UnixMakefileGenerator::openOutput(file, build);
    ((ProjectBuilderMakefileGenerator*)this)->pbx_dir = Option::output_dir.section(Option::dir_sep, 0, -1);
    Option::output_dir = pbx_dir.section(Option::dir_sep, 0, -2);
    return ret;
}

/* This function is such a hack it is almost pointless, but it
   eliminates the warning message from ProjectBuilder that the project
   file is for an older version. I guess this could be used someday if
   the format of the output is dependant upon the version of
   ProjectBuilder as well.
*/
int
ProjectBuilderMakefileGenerator::pbuilderVersion() const
{
    QString ret;
    if(project->isEmpty("QMAKE_PBUILDER_VERSION")) {
        QString version, version_plist = project->first("QMAKE_PBUILDER_VERSION_PLIST");
        if(version_plist.isEmpty()) {
            if(ideType() == MAC_XCODE && QFile::exists("/Developer/Applications/Xcode.app/Contents/version.plist"))
                version_plist = "/Developer/Applications/Xcode.app/Contents/version.plist";
            else
                version_plist = "/Developer/Applications/Project Builder.app/Contents/version.plist";
        } else {
            version_plist = version_plist.replace(QRegExp("\""), "");
        }
        QFile version_file(version_plist);
        if(version_file.open(QIODevice::ReadOnly)) {
            debug_msg(1, "pbuilder: version.plist: Reading file: %s", version_plist.latin1());
            QTextStream plist(&version_file);

            bool in_dict = false;
            QString current_key;
            QRegExp keyreg("^<key>(.*)</key>$"), stringreg("^<string>(.*)</string>$");
            while(!plist.eof()) {
                QString line = plist.readLine().trimmed();
                if(line == "<dict>")
                    in_dict = true;
                else if(line == "</dict>")
                    in_dict = false;
                else if(in_dict) {
                    if(keyreg.exactMatch(line))
                        current_key = keyreg.cap(1);
                    else if(current_key == "CFBundleShortVersionString" && stringreg.exactMatch(line))
                        version = stringreg.cap(1);
                }
            }
            version_file.close();
        } else { debug_msg(1, "pbuilder: version.plist: Failure to open %s", version_plist.latin1()); }
        if(version_plist.contains("Xcode")) {
            ret = "39";
        } else {
            if(version.startsWith("2."))
                ret = "38";
            else if(version == "1.1")
                ret = "34";
        }
    } else {
        ret = project->first("QMAKE_PBUILDER_VERSION");
    }
    if(!ret.isEmpty()) {
        bool ok;
        int int_ret = ret.toInt(&ok);
        if(ok) {
            debug_msg(1, "pbuilder: version.plist: Got version: %d", int_ret);
            return int_ret;
        }
    }
    debug_msg(1, "pbuilder: version.plist: Fallback to default version");
    return 34; //my fallback
}

int
ProjectBuilderMakefileGenerator::reftypeForFile(const QString &where)
{
    int ret = 0; //absolute is the default..
    if(QDir::isRelativePath(where))
        ret = 4; //relative
    return ret;
}

ProjectBuilderMakefileGenerator::IDE_TYPE
ProjectBuilderMakefileGenerator::ideType() const
{
    if(!project->isActiveConfig("no_pbx_xcode") &&
       (QFile::exists("/Developer/Applications/Xcode.app") || project->isActiveConfig("pbx_xcode")))
        return ProjectBuilderMakefileGenerator::MAC_XCODE;
    return ProjectBuilderMakefileGenerator::MAC_PBUILDER;
}

QString
ProjectBuilderMakefileGenerator::projectSuffix() const
{
    if(ideType() == MAC_XCODE)
        return ".xcode";
    return ".pbproj";
}

QString
ProjectBuilderMakefileGenerator::pbxbuild()
{
    if(QFile::exists("/usr/bin/pbbuild"))
        return "pbbuild";
    if(QFile::exists("/usr/bin/xcodebuild"))
       return "xcodebuild";
    return (ideType() == MAC_XCODE ? "xcodebuild" : "pbxbuild");
}

