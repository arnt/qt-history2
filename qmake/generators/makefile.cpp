/****************************************************************************
**
** Implementation of MakefileGenerator class.
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

#include "makefile.h"
#include "option.h"
#include "meta.h"
#include <qdir.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <qhash.h>
#if defined(Q_OS_UNIX)
#include <unistd.h>
#else
#include <io.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

// Well, Windows doesn't have this, so here's the macro
#ifndef S_ISDIR
#  define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif

QString mkdir_p_asstring(const QString &dir)
{
    QString ret =  "@$(CHK_DIR_EXISTS) \"" + dir + "\" ";
    if(Option::target_mode == Option::TARG_WIN_MODE)
        ret += "$(MKDIR)";
    else
        ret += "|| $(MKDIR)";
    ret += " \"" + dir + "\"";
    return ret;
}

static bool createDir(QString path)
{
    if(QFile::exists(path))
        return true;

    QDir d;
    if(path.startsWith(Option::dir_sep)) {
        d.cd(Option::dir_sep);
        path = path.right(path.length() - 1);
    }
    bool ret = true;
#ifdef Q_WS_WIN
    bool driveExists = true;
    if(!QDir::isRelativePath(path)) {
        if(QFile::exists(path.left(3))) {
            d.cd(path.left(3));
            path = path.right(path.length() - 3);
        } else {
            warn_msg(WarnLogic, "Cannot access drive '%s' (%s)",
                     path.left(3).latin1(), path.latin1());
            driveExists = false;
        }
    }
    if(driveExists)
#endif
    {
        QStringList subs = path.split(Option::dir_sep);
        for(QStringList::Iterator subit = subs.begin(); subit != subs.end(); ++subit) {
            if(!d.cd(*subit)) {
                d.mkdir((*subit));
                if(d.exists((*subit))) {
                    d.cd((*subit));
                } else {
                    ret = false;
                    break;
                }
            }
        }
    }
    return ret;
}


MakefileGenerator::MakefileGenerator(QMakeProject *p) : init_opath_already(false),
                                                        init_already(false), moc_aware(false),
                                                        no_io(false), project(p)
{
}


void
MakefileGenerator::initOutPaths()
{
    if(init_opath_already)
        return;
    init_opath_already = true;
    QMap<QString, QStringList> &v = project->variables();
    //for shadow builds
    if(!v.contains("QMAKE_ABSOLUTE_SOURCE_PATH")) {
        if(Option::mkfile::do_cache && !Option::mkfile::cachefile.isEmpty() &&
           v.contains("QMAKE_ABSOLUTE_SOURCE_ROOT")) {
            QString root = v["QMAKE_ABSOLUTE_SOURCE_ROOT"].first();
            root = Option::fixPathToTargetOS(root);
            if(!root.isEmpty()) {
                QFileInfo fi(Option::mkfile::cachefile);
                if(!fi.convertToAbs()) {
                    QString cache_r = fi.dirPath(), pwd = Option::output_dir;
                    if(pwd.startsWith(cache_r) && !pwd.startsWith(root)) {
                        pwd = Option::fixPathToTargetOS(root + pwd.mid(cache_r.length()));
                        if(QFile::exists(pwd))
                            v.insert("QMAKE_ABSOLUTE_SOURCE_PATH", pwd);
                    }
                }
            }
        }
    }
    if(!v["QMAKE_ABSOLUTE_SOURCE_PATH"].isEmpty()) {
        QString &asp = v["QMAKE_ABSOLUTE_SOURCE_PATH"].first();
        asp = Option::fixPathToTargetOS(asp);
        if(asp.isEmpty() || asp == Option::output_dir) //if they're the same, why bother?
            v["QMAKE_ABSOLUTE_SOURCE_PATH"].clear();
    }

    QString currentDir = QDir::currentDirPath(); //just to go back to

    //some builtin directories
    QString dirs[] = { QString("OBJECTS_DIR"), QString("MOC_DIR"), QString("DESTDIR"),
                       QString("SUBLIBS_DIR"), QString("DLLDESTDIR"), QString::null };
    for(int x = 0; true; x++) {
        if(dirs[x] == QString::null)
            break;
        if(!v[dirs[x]].isEmpty()) {
            QString orig_path = v[dirs[x]].first();
#ifdef Q_WS_WIN
            // We don't want to add a separator for DLLDESTDIR on Windows
            if(!(dirs[x] == "DLLDESTDIR"))
#endif
            {
                QString &path = v[dirs[x]].first();
                QString op = path;
                path = fileFixify(path, Option::output_dir, Option::output_dir);
                if(path.right(Option::dir_sep.length()) != Option::dir_sep)
                    path += Option::dir_sep;
            }
            if(noIO())
                continue;

            QString path = project->first(dirs[x]); //not to be changed any further
            path = Option::fixPathToLocalOS(fileFixify(path, currentDir, Option::output_dir));
            debug_msg(3, "Fixed output_dir %s (%s) into %s (%s)", dirs[x].latin1(), orig_path.latin1(),
                      v[dirs[x]].join("::").latin1(), path.latin1());
            if(!createDir(path))
                warn_msg(WarnLogic, "%s: Cannot access directory '%s'", dirs[x].latin1(), path.latin1());
        }
    }

    //out paths from the extra compilers
    const QStringList &quc = project->variables()["QMAKE_EXTRA_COMPILERS"];
    for(QStringList::ConstIterator it = quc.begin(); it != quc.end(); ++it) {
        QString tmp_out = project->variables()[(*it) + ".output"].first();
        if(tmp_out.isEmpty())
            continue;
        const QStringList &tmp = project->variables()[(*it) + ".input"];
        for(QStringList::ConstIterator it2 = tmp.begin(); it2 != tmp.end(); ++it2) {
            const QStringList &inputs = project->variables()[(*it2)];
            for(QStringList::ConstIterator input = inputs.begin(); input != inputs.end(); ++input) {
                QString path = replaceExtraCompilerVariables(tmp_out, (*input), QString::null);
                int slash = path.lastIndexOf(Option::dir_sep);
                if(slash != -1) {
                    path = path.left(slash);
                    if(path != "." && !createDir(path))
                        warn_msg(WarnLogic, "%s: Cannot access directory '%s'", (*it).latin1(), path.latin1());
                }
            }
        }
    }

    if(!v["DESTDIR"].isEmpty()) {
        QDir d(v["DESTDIR"].first());
        if(Option::fixPathToLocalOS(d.absPath()) == Option::fixPathToLocalOS(Option::output_dir))
            v.remove("DESTDIR");
    }
    QDir::current().cd(currentDir);
}

void
MakefileGenerator::init()
{
    initOutPaths();
    if(init_already)
        return;
    init_already = true;

    QMap<QString, QStringList> &v = project->variables();
    QString paths[] = { QString("SOURCES"), QString("FORMS"), QString("YACCSOURCES"), QString("INCLUDEPATH"),
                        QString("HEADERS"), QString("HEADERS_ORIG"), QString("LEXSOURCES"),
                        QString("QMAKE_INTERNAL_INCLUDED_FILES"),
                        QString("PRECOMPILED_HEADER"), QString::null };
    for(int y = 0; paths[y] != QString::null; y++) {
        QStringList &l = v[paths[y]];
        for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
            if((*it).isEmpty())
                continue;
            if(QFile::exists((*it)))
                (*it) = fileFixify((*it));
        }
    }

    /* get deps and mocables */
    if((Option::qmake_mode == Option::QMAKE_GENERATE_PROJECT ||
        Option::mkfile::do_deps || Option::mkfile::do_mocs) && !noIO()) {
        depHeuristics.clear();
        if((Option::qmake_mode == Option::QMAKE_GENERATE_PROJECT || Option::mkfile::do_deps) &&
           doDepends()) {
            QStringList incDirs = v["DEPENDPATH"] + v["QMAKE_ABSOLUTE_SOURCE_PATH"];
            if(project->isActiveConfig("depend_includepath"))
                incDirs += v["INCLUDEPATH"];
            QList<QMakeLocalFileName> deplist;
            for(QStringList::Iterator it = incDirs.begin(); it != incDirs.end(); ++it)
                deplist.append(QMakeLocalFileName((*it)));
            setDependencyPaths(deplist);
            debug_msg(1, "Dependency Directories: %s", incDirs.join(" :: ").latin1());
        }
        if(!noIO()) {
            QString sources[] = { QString("OBJECTS"), QString("LEXSOURCES"), QString("YACCSOURCES"),
                                  QString("HEADERS"), QString("SOURCES"), QString("FORMS"),
                                  QString("PRECOMPILED_HEADER"), QString::null };
            int x;
            for(x = 0; sources[x] != QString::null; x++) {
                QStringList &l = v[sources[x]], vpath;
                for(QStringList::Iterator val_it = l.begin(); val_it != l.end(); ++val_it) {
                    if(!(*val_it).isEmpty()) {
                        QString file = fileFixify((*val_it), QDir::currentDirPath(), Option::output_dir);
                        if(QFile::exists(file))
                            continue;
                        bool found = false;
                        if(QDir::isRelativePath((*val_it))) {
                            if(vpath.isEmpty())
                                vpath = v["VPATH_" + sources[x]] + v["VPATH"] +
                                        v["QMAKE_ABSOLUTE_SOURCE_PATH"] + v["DEPENDPATH"];

                            for(QStringList::Iterator vpath_it = vpath.begin();
                                vpath_it != vpath.end(); ++vpath_it) {
                                QString real_dir = Option::fixPathToLocalOS((*vpath_it));
                                if(QFile::exists(real_dir + QDir::separator() + (*val_it))) {
                                    QString dir = (*vpath_it);
                                    if(dir.right(Option::dir_sep.length()) != Option::dir_sep)
                                        dir += Option::dir_sep;
                                    (*val_it) = fileFixify(dir + (*val_it));
                                    found = true;
                                    debug_msg(1, "Found file through vpath %s -> %s",
                                              file.latin1(), (*val_it).latin1());
                                    break;
                                }
                            }
                        }
                        if(!found) {
                            QString dir, regex = (*val_it), real_dir;
                            if(regex.lastIndexOf(Option::dir_sep) != -1) {
                                dir = regex.left(regex.lastIndexOf(Option::dir_sep) + 1);
                                real_dir = fileFixify(Option::fixPathToLocalOS(dir),
                                                      QDir::currentDirPath(), Option::output_dir);
                                regex = regex.right(regex.length() - dir.length());
                            }
                            if(real_dir.isEmpty() || QFile::exists(real_dir)) {
                                QDir d(real_dir, regex);
                                if(!d.count()) {
                                    debug_msg(1, "%s:%d Failure to find %s in vpath (%s)",
                                              __FILE__, __LINE__,
                                              (*val_it).latin1(), vpath.join("::").latin1());
                                    warn_msg(WarnLogic, "Failure to find: %s", (*val_it).latin1());
                                    continue;
                                } else {
                                    for(int i = 0; i < (int)d.count(); i++) {
                                        QString file = fileFixify(dir + d[i]);
                                        if(i == (int)d.count() - 1)
                                            (*val_it) = file;
                                        else
                                            l.insert(val_it, file);
                                    }
                                }
                            } else {
                                debug_msg(1, "%s:%d Cannot match %s%c%s, as %s does not exist.",
                                          __FILE__, __LINE__, real_dir.latin1(),
                                          QDir::separator().latin1(), regex.latin1(),
                                          real_dir.latin1());
                                warn_msg(WarnLogic, "Failure to find: %s", (*val_it).latin1());
                            }
                        }
                    }
                }
            }
            for(x = 0; sources[x] != QString::null; x++) {
                uchar seek = 0;
                if(mocAware() && (sources[x] == "SOURCES" || sources[x] == "HEADERS") &&
                   (Option::qmake_mode == Option::QMAKE_GENERATE_PROJECT || Option::mkfile::do_mocs))
                    seek |= QMakeSourceFileInfo::SEEK_MOCS;
                if(sources[x] != "OBJECTS")
                    seek |= QMakeSourceFileInfo::SEEK_DEPS;
                if(seek)
                    addSourceFiles(v[sources[x]], seek, sources[x] == "FORMS");
            }
        }
    }

    //extra compilers (done here so it ends up in the variables post-fixified)
    QStringList &quc = project->variables()["QMAKE_EXTRA_COMPILERS"];
    for(QStringList::Iterator it = quc.begin(); it != quc.end(); ++it) {
        QString tmp_out = project->variables()[(*it) + ".output"].first();
        if(tmp_out.isEmpty())
            continue;
        if(project->variables()[(*it) + ".CONFIG"].indexOf("combine") != -1) {
            // Don't generate compiler output if it doesn't have input.
            QStringList &compilerInputs = project->variables()[(*it) + ".input"];
            if (compilerInputs.isEmpty() || project->variables()[compilerInputs.first()].isEmpty())
                continue;
            if(tmp_out.indexOf("$") == -1) {
                if(project->variables().contains((*it) + ".variable_out"))
                    project->variables()[project->variables().value((*it) + ".variable_out").first()] += Option::fixPathToTargetOS(tmp_out, false);
                else if(project->variables()[(*it) + ".CONFIG"].indexOf("no_link") == -1)
                    project->variables()["OBJECTS"] += tmp_out; //auto link it in
            }
            continue;
        }
        QStringList &tmp = project->variables()[(*it) + ".input"];
        for(QStringList::Iterator it2 = tmp.begin(); it2 != tmp.end(); ++it2) {
            QStringList &inputs = project->variables()[(*it2)];
            for(QStringList::Iterator input = inputs.begin(); input != inputs.end(); ++input) {
                if((*input).isEmpty())
                    continue;
                if(QFile::exists((*input)))
                    (*input) = fileFixify((*input));
                QFileInfo fi(Option::fixPathToLocalOS((*input)));
                QString in = fileFixify(Option::fixPathToTargetOS((*input), false));
                QString out = replaceExtraCompilerVariables(tmp_out, (*input), QString::null);
                if(project->variables().contains((*it) + ".variable_out"))
                    project->variables()[project->variables().value((*it) + ".variable_out").first()] += Option::fixPathToTargetOS(out, false);
                else if(project->variables()[(*it) + ".CONFIG"].indexOf("no_link") == -1)
                    project->variables()["OBJECTS"] += out; //auto link it in
            }
        }
    }

    //SOURCES (as objects) get built first!
    v["OBJECTS"] = createObjectList("SOURCES") + v["OBJECTS"]; // init variables

    //lex files
    {
        QStringList &impls = v["LEXIMPLS"];
        QStringList &l = v["LEXSOURCES"];
        for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
            QString dir;
            QFileInfo fi((*it));
            if(fi.dirPath() != ".")
                dir = fi.dirPath() + Option::dir_sep;
            dir = fileFixify(dir, QDir::currentDirPath(), Option::output_dir);
            if(!dir.isEmpty() && dir.right(Option::dir_sep.length()) != Option::dir_sep)
                dir += Option::dir_sep;
            QString impl = dir + fi.baseName(true) + Option::lex_mod + Option::cpp_ext.first();
            checkMultipleDefinition(impl, "SOURCES");
            checkMultipleDefinition(impl, "SOURCES");
            impls.append(impl);
            if(!project->isActiveConfig("lex_included")) {
                v["SOURCES"].append(impl);
                // attribute deps of lex file to impl file
                QStringList &lexdeps = findDependencies((*it));
                QStringList &impldeps = findDependencies(impl);
                for(QStringList::ConstIterator d = lexdeps.begin(); d != lexdeps.end(); ++d) {
                    if(!impldeps.contains(*d))
                        impldeps.append(*d);
                }
                lexdeps.clear();
            }
        }
        if(!project->isActiveConfig("lex_included"))
            v["OBJECTS"] += (v["LEXOBJECTS"] = createObjectList("LEXIMPLS"));
    }
    //yacc files
    {
        QStringList &decls = v["YACCCDECLS"], &impls = v["YACCIMPLS"];
        QStringList &l = v["YACCSOURCES"];
        for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
            QString dir;
            QFileInfo fi((*it));
            if(fi.dirPath() != ".")
                dir = fi.dirPath() + Option::dir_sep;
            dir = fileFixify(dir, QDir::currentDirPath(), Option::output_dir);
            if(!dir.isEmpty() && dir.right(Option::dir_sep.length()) != Option::dir_sep)
                dir += Option::dir_sep;
            QString impl = dir + fi.baseName(true) + Option::yacc_mod + Option::cpp_ext.first();
            checkMultipleDefinition(impl, "SOURCES");
            QString decl = dir + fi.baseName(true) + Option::yacc_mod + Option::h_ext.first();
            checkMultipleDefinition(decl, "HEADERS");

            decls.append(decl);
            impls.append(impl);
            v["SOURCES"].append(impl);
            QStringList &impldeps = findDependencies(impl);
            impldeps.append(decl);
            // attribute deps of yacc file to impl file
            QStringList &yaccdeps = findDependencies((*it));
            for(QStringList::ConstIterator d = yaccdeps.begin(); d != yaccdeps.end(); ++d) {
                if(!impldeps.contains(*d))
                    impldeps.append(*d);
            }
            if(project->isActiveConfig("lex_included")) {
                // is there a matching lex file ? Transfer its dependencies.
                QString lexsrc = fi.baseName(true) + Option::lex_ext;
                if(fi.dirPath() != ".")
                    lexsrc.prepend(fi.dirPath() + Option::dir_sep);
                if(v["LEXSOURCES"].indexOf(lexsrc) != -1) {
                    QString trg = dir + fi.baseName(true) + Option::lex_mod + Option::cpp_ext.first();
                    impldeps.append(trg);
                    impldeps += findDependencies(lexsrc);
                    depends[lexsrc].clear();
                }
            }
            yaccdeps.clear();
        }
        v["OBJECTS"] += (v["YACCOBJECTS"] = createObjectList("YACCIMPLS"));
    }

    //Translation files
    if(!project->isEmpty("TRANSLATIONS")) {
        QStringList &trf = project->variables()["TRANSLATIONS"];
        for(QStringList::Iterator it = trf.begin(); it != trf.end(); ++it) {
            (*it) = Option::fixPathToLocalOS((*it));
        }
    }

    if(Option::output_dir != QDir::currentDirPath())
        project->variables()["INCLUDEPATH"].append(fileFixify(Option::output_dir, Option::output_dir,
                                                              Option::output_dir));

    //moc files
    if(mocAware()) {
        if(!project->isEmpty("MOC_DIR"))
            project->variables()["INCLUDEPATH"].append(project->first("MOC_DIR"));

        if(Option::h_moc_ext == Option::cpp_ext.first())
            v["OBJMOC"] = createObjectList("_HDRMOC");

        QStringList &l = v["SRCMOC"];
        l = v["_HDRMOC"] + v["_SRCMOC"];
        for(QStringList::Iterator val_it = l.begin(); val_it != l.end(); ++val_it) {
            if(!(*val_it).isEmpty())
                (*val_it) = Option::fixPathToTargetOS((*val_it), false);
        }
    }

    //fix up the target deps
    QString fixpaths[] = { QString("PRE_TARGETDEPS"), QString("POST_TARGETDEPS"), QString::null };
    for(int path = 0; !fixpaths[path].isNull(); path++) {
        QStringList &l = v[fixpaths[path]];
        for(QStringList::Iterator val_it = l.begin(); val_it != l.end(); ++val_it) {
            if(!(*val_it).isEmpty())
                (*val_it) = Option::fixPathToTargetOS((*val_it), false);
        }
    }

    //extra depends
    if(!project->isEmpty("DEPENDS")) {
        QStringList &l = v["DEPENDS"];
        for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
            QStringList files = v[(*it) + ".file"] + v[(*it) + ".files"]; //why do I support such evil things?
            for(QStringList::Iterator file_it = files.begin(); file_it != files.end(); ++file_it) {
                QStringList &out_deps = findDependencies(*file_it);
                QStringList &in_deps = v[(*it) + ".depends"]; //even more evilness..
                for(QStringList::Iterator dep_it = in_deps.begin(); dep_it != in_deps.end(); ++dep_it) {
                    if(QFile::exists(*dep_it)) {
                        out_deps.append(*dep_it);
                    } else {
                        QString dir, regex = Option::fixPathToLocalOS((*dep_it));
                        if(regex.lastIndexOf(Option::dir_sep) != -1) {
                            dir = regex.left(regex.lastIndexOf(Option::dir_sep) + 1);
                            regex = regex.right(regex.length() - dir.length());
                        }
                        QDir qdir(dir, regex);
                        if(qdir.count()) {
                            for(uint i = 0; i < qdir.count(); i++)
                                out_deps.append(dir + qdir[i]);
                        } else {
                            warn_msg(WarnLogic, "Dependency for [%s]: Not found %s", (*file_it).latin1(),
                                     (*dep_it).latin1());
                        }
                    }
                }
            }
        }
    }
}

bool
MakefileGenerator::processPrlFile(QString &file)
{
    bool ret = false, try_replace_file=false;
    QString meta_file, orig_file = file;
    if(QMakeMetaInfo::libExists(file)) {
        try_replace_file = true;
        meta_file = file;
        file = "";
    } else {
        QString tmp = file;
        int ext = tmp.lastIndexOf('.');
        if(ext != -1)
            tmp = tmp.left(ext);
        meta_file = tmp;
    }
    meta_file = fileFixify(meta_file);
    QString real_meta_file = Option::fixPathToLocalOS(meta_file);
    if(project->variables()["QMAKE_PRL_INTERNAL_FILES"].indexOf(QMakeMetaInfo::findLib(meta_file)) != -1) {
        ret = true;
    } else if(!meta_file.isEmpty()) {
        QString f = fileFixify(real_meta_file, QDir::currentDirPath(), Option::output_dir);
        if(QMakeMetaInfo::libExists(f)) {
            QMakeMetaInfo libinfo;
            debug_msg(1, "Processing PRL file: %s", real_meta_file.latin1());
            if(!libinfo.readLib(f)) {
                fprintf(stderr, "Error processing meta file: %s\n", real_meta_file.latin1());
            } else if(project->isActiveConfig("no_read_prl_" + libinfo.type().toLower())) {
                debug_msg(2, "Ignored meta file %s [%s]", real_meta_file.latin1(), libinfo.type().latin1());
            } else {
                ret = true;
                QMap<QString, QStringList> &vars = libinfo.variables();
                for(QMap<QString, QStringList>::Iterator it = vars.begin(); it != vars.end(); ++it)
                    processPrlVariable(it.key(), it.value());
                if(try_replace_file && !libinfo.isEmpty("QMAKE_PRL_TARGET")) {
                    QString dir;
                    int slsh = real_meta_file.lastIndexOf(Option::dir_sep);
                    if(slsh != -1)
                        dir = real_meta_file.left(slsh+1);
                    file = libinfo.first("QMAKE_PRL_TARGET");
                    if(QDir::isRelativePath(file))
                        file.prepend(dir);
                }
            }
        }
        if(ret) {
            QString mf = QMakeMetaInfo::findLib(meta_file);
            project->variables()["QMAKE_PRL_INTERNAL_FILES"].append(mf);
            project->variables()["QMAKE_INTERNAL_INCLUDED_FILES"].append(mf);
        }
    }
    if(try_replace_file && file.isEmpty()) {
#if 0
        warn_msg(WarnLogic, "Found prl [%s] file with no target [%s]!", meta_file.latin1(),
                 orig_file.latin1());
#endif
        file = orig_file;
    }
    return ret;
}

void
MakefileGenerator::processPrlVariable(const QString &var, const QStringList &l)
{
    if(var == "QMAKE_PRL_LIBS") {
        QString where = "QMAKE_LIBS";
        if(!project->isEmpty("QMAKE_INTERNAL_PRL_LIBS"))
            where = project->first("QMAKE_INTERNAL_PRL_LIBS");
        QStringList &out = project->variables()[where];
        for(QStringList::ConstIterator it = l.begin(); it != l.end(); ++it) {
            if(out.indexOf((*it)) == -1)
                out.append((*it));
        }
    } else if(var == "QMAKE_PRL_DEFINES") {
        QStringList &out = project->variables()["DEFINES"];
        for(QStringList::ConstIterator it = l.begin(); it != l.end(); ++it) {
            if(out.indexOf((*it)) == -1 &&
               project->variables()["PRL_EXPORT_DEFINES"].indexOf((*it)) == -1)
                out.append((*it));
        }
    }
}

void
MakefileGenerator::processPrlFiles()
{
    QHash<QString, bool> processed;
    for(bool ret = false; true; ret = false) {
        //read in any prl files included..
        QStringList l_out;
        QString where = "QMAKE_LIBS";
        if(!project->isEmpty("QMAKE_INTERNAL_PRL_LIBS"))
            where = project->first("QMAKE_INTERNAL_PRL_LIBS");
        QStringList &l = project->variables()[where];
        for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
            QString file = (*it);
            if(!processed.contains(file) && processPrlFile(file)) {
                processed.insert(file, true);
                ret = true;
            }
            if(!file.isEmpty())
                l_out.append(file);
        }
        if(ret)
            l = l_out;
        else
            break;
    }
}

void
MakefileGenerator::writePrlFile(QTextStream &t)
{
    QString target = project->first("TARGET");
    int slsh = target.lastIndexOf(Option::dir_sep);
    if(slsh != -1)
        target = target.right(target.length() - slsh - 1);
    QString bdir = Option::output_dir;
    if(bdir.isEmpty())
        bdir = QDir::currentDirPath();
    t << "QMAKE_PRL_BUILD_DIR = " << bdir << endl;

    if(!project->projectFile().isEmpty() && project->projectFile() != "-")
        t << "QMAKE_PRO_INPUT = " << project->projectFile().section('/', -1) << endl;

    if(!project->isEmpty("QMAKE_ABSOLUTE_SOURCE_PATH"))
        t << "QMAKE_PRL_SOURCE_DIR = " << project->first("QMAKE_ABSOLUTE_SOURCE_PATH") << endl;
    t << "QMAKE_PRL_TARGET = " << target << endl;
    if(!project->isEmpty("PRL_EXPORT_DEFINES"))
        t << "QMAKE_PRL_DEFINES = " << project->variables()["PRL_EXPORT_DEFINES"].join(" ") << endl;
    if(!project->isEmpty("PRL_EXPORT_CFLAGS"))
        t << "QMAKE_PRL_CFLAGS = " << project->variables()["PRL_EXPORT_CFLAGS"].join(" ") << endl;
    if(!project->isEmpty("PRL_EXPORT_CXXFLAGS"))
        t << "QMAKE_PRL_CXXFLAGS = " << project->variables()["PRL_EXPORT_CXXFLAGS"].join(" ") << endl;
    if(!project->isEmpty("CONFIG"))
        t << "QMAKE_PRL_CONFIG = " << project->variables()["CONFIG"].join(" ") << endl;
    if(!project->isEmpty("VERSION"))
        t << "QMAKE_PRL_VERSION = " << project->first("VERSION") << endl;
    if(project->isActiveConfig("staticlib") || project->isActiveConfig("explicitlib")) {
        QStringList libs;
        if(!project->isEmpty("QMAKE_INTERNAL_PRL_LIBS"))
            libs = project->variables()["QMAKE_INTERNAL_PRL_LIBS"];
        else
            libs << "QMAKE_LIBS"; //obvious one
        t << "QMAKE_PRL_LIBS = ";
        for(QStringList::Iterator it = libs.begin(); it != libs.end(); ++it)
            t << project->variables()[(*it)].join(" ") << " ";
        t << endl;
    }
}

bool
MakefileGenerator::writeProjectMakefile()
{
    usePlatformDir();
    QTextStream t(&Option::output);

    //header
    writeHeader(t);

    QList<SubTarget*> targets;
    {
        QStringList builds = project->variables()["BUILDS"];
        for(QStringList::Iterator it = builds.begin(); it != builds.end(); ++it) {
            SubTarget *st = new SubTarget;
            targets.append(st);
            st->makefile = "$(MAKEFILE)." + (*it);
            st->target = project->isEmpty((*it) + ".target") ? (*it) : project->first((*it) + ".target");
        }
    }
    t << "first: " << targets.first()->target << endl;
    writeSubTargets(t, targets, false);
    return true;
}

bool
MakefileGenerator::write()
{
    usePlatformDir();
    init();
    findLibraries();
    if((Option::qmake_mode == Option::QMAKE_GENERATE_MAKEFILE || Option::qmake_mode == Option::QMAKE_GENERATE_PRL)
       && project->variables()["QMAKE_FAILED_REQUIREMENTS"].isEmpty()
       && project->isActiveConfig("create_prl")
       && (project->first("TEMPLATE") == "lib" || project->first("TEMPLATE") == "vclib")
       && !project->isActiveConfig("plugin")) {
        QString prl = var("TARGET");
        int slsh = prl.lastIndexOf(Option::dir_sep);
        if(slsh != -1)
            prl = prl.right(prl.length() - slsh);
        int dot = prl.indexOf('.');
        if(dot != -1)
            prl = prl.left(dot);
        prl += Option::prl_ext;
        if(!project->isEmpty("DESTDIR"))
            prl.prepend(var("DESTDIR"));
        QString local_prl = Option::fixPathToLocalOS(fileFixify(prl, QDir::currentDirPath(), Option::output_dir));
        QFile ft(local_prl);
        if(ft.open(IO_WriteOnly)) {
            project->variables()["ALL_DEPS"].append(prl);
            project->variables()["QMAKE_INTERNAL_PRL_FILE"].append(prl);
            QTextStream t(&ft);
            writePrlFile(t);
            ft.close();
        }
    }
    if(Option::qmake_mode == Option::QMAKE_GENERATE_MAKEFILE &&
       project->isActiveConfig("link_prl")) //load up prl's'
        processPrlFiles();

    if(Option::qmake_mode == Option::QMAKE_GENERATE_MAKEFILE || //write prl file
       Option::qmake_mode == Option::QMAKE_GENERATE_PROJECT) {
        QTextStream t(&Option::output);
        writeMakefile(t);
    }
    return true;
}

// Manipulate directories, so it's possible to build
// several cross-platform targets concurrently
void
MakefileGenerator::usePlatformDir()
{
    QString pltDir(project->first("QMAKE_PLATFORM_DIR"));
    if(pltDir.isEmpty())
        return;
    QChar sep = QDir::separator();
    QString slashPltDir = sep + pltDir;

    QString filePath = project->first("DESTDIR");
    project->variables()["DESTDIR"] = filePath
                                    + (filePath.isEmpty() ? pltDir : slashPltDir);

    filePath = project->first("DLLDESTDIR");
    project->variables()["DLLDESTDIR"] = filePath
                                       + (filePath.isEmpty() ? pltDir : slashPltDir);

    filePath = project->first("OBJECTS_DIR");
    project->variables()["OBJECTS_DIR"] = filePath
                                        + (filePath.isEmpty() ? pltDir : slashPltDir);

    filePath = project->first("QMAKE_LIBDIR_QT");
    project->variables()["QMAKE_LIBDIR_QT"] = filePath
                                            + (filePath.isEmpty() ? pltDir : slashPltDir);

    filePath = project->first("QMAKE_LIBS_QT");
    int fpi = filePath.lastIndexOf(sep);
    if(fpi == -1)
        project->variables()["QMAKE_LIBS_QT"].prepend(pltDir + sep);
    else
        project->variables()["QMAKE_LIBS_QT"] = filePath.left(fpi)
                                              + slashPltDir
                                              + filePath.mid(fpi);

    filePath = project->first("QMAKE_LIBS_QT_THREAD");
    fpi = filePath.lastIndexOf(sep);
    if(fpi == -1)
        project->variables()["QMAKE_LIBS_QT_THREAD"].prepend(pltDir + sep);
    else
        project->variables()["QMAKE_LIBS_QT_THREAD"] = filePath.left(fpi)
                                                     + slashPltDir
                                                     + filePath.mid(fpi);

    filePath = project->first("QMAKE_LIBS_QT_ENTRY");
    fpi = filePath.lastIndexOf(sep);
    if(fpi == -1)
        project->variables()["QMAKE_LIBS_QT_ENTRY"].prepend(pltDir + sep);
    else
        project->variables()["QMAKE_LIBS_QT_ENTRY"] = filePath.left(fpi)
                                                    + slashPltDir
                                                    + filePath.mid(fpi);
}

void
MakefileGenerator::writeObj(QTextStream &t, const QString &obj, const QString &src)
{
    QStringList &objl = project->variables()[obj];
    QStringList &srcl = project->variables()[src];

    QStringList::Iterator oit = objl.begin();
    QStringList::Iterator sit = srcl.begin();
    QString stringSrc("$src");
    QString stringObj("$obj");
    for(;sit != srcl.end() && oit != objl.end(); oit++, sit++) {
        if((*sit).isEmpty())
            continue;

        if(!doDepends()) {
            QString sdep, odep = (*sit) + " ";
            QStringList deps = findDependencies((*sit));
            for(QStringList::Iterator dit = deps.begin(); dit != deps.end(); dit++) {
                if((*dit).endsWith(Option::cpp_moc_ext))
                    odep += (*dit) + " ";
                else
                    sdep += (*dit) + " ";
            }
            t << (*sit) << ": " << sdep << endl
              << (*oit) << ": " << odep ;
        } else {
            t << (*oit) << ": " << (*sit) << " " << findDependencies((*sit)).join(" \\\n\t\t");
        }

        QString comp, cimp;
        for(QStringList::Iterator cppit = Option::cpp_ext.begin(); cppit != Option::cpp_ext.end(); ++cppit) {
            if((*sit).endsWith((*cppit))) {
                comp = "QMAKE_RUN_CXX";
                cimp = "QMAKE_RUN_CXX_IMP";
                break;
            }
        }
        if(comp.isEmpty()) {
            comp = "QMAKE_RUN_CC";
            cimp = "QMAKE_RUN_CC_IMP";
        }
        bool use_implicit_rule = !project->isEmpty(cimp);
        if(use_implicit_rule) {
            if(!project->isEmpty("OBJECTS_DIR")) {
                use_implicit_rule = false;
            } else {
                int dot = (*sit).lastIndexOf('.');
                if(dot == -1 || ((*sit).left(dot) + Option::obj_ext != (*oit)))
                    use_implicit_rule = false;
            }
        }
        if (!use_implicit_rule && !project->isEmpty(comp)) {
            QString p = var(comp), srcf(*sit);
            p.replace(stringSrc, srcf);
            p.replace(stringObj, (*oit));
            t << "\n\t" << p;
        }
        t << endl << endl;
    }
}

void
MakefileGenerator::writeMocObj(QTextStream &t, const QString &obj, const QString &src)
{
    QStringList &objl = project->variables()[obj],
                &srcl = project->variables()[src];
    QStringList::Iterator oit = objl.begin(), sit = srcl.begin();
    QString stringSrc("$src"), stringObj("$obj");
    for(;sit != srcl.end() && oit != objl.end(); oit++, sit++) {
        QString hdr = QMakeSourceFileInfo::mocSource((*sit));
        t << (*oit) << ": " << (*sit) << " " << valList(findDependencies(*sit)) << " "
          << hdr << " " << valList(findDependencies(hdr));
        bool use_implicit_rule = !project->isEmpty("QMAKE_RUN_CXX_IMP");
        if(use_implicit_rule) {
            if(!project->isEmpty("OBJECTS_DIR") || !project->isEmpty("MOC_DIR")) {
                use_implicit_rule = false;
            } else {
                int dot = (*sit).lastIndexOf('.');
                if(dot == -1 || ((*sit).left(dot) + Option::obj_ext != (*oit)))
                    use_implicit_rule = false;
            }
        }
        if (!use_implicit_rule && !project->isEmpty("QMAKE_RUN_CXX")) {
            QString p = var("QMAKE_RUN_CXX"), srcf(*sit);
            p.replace(stringSrc, srcf);
            p.replace(stringObj, (*oit));
            t << "\n\t" << p;
        }
        t << endl << endl;
    }
}

void
MakefileGenerator::writeMocSrc(QTextStream &t, const QString &src)
{
    QStringList &l = project->variables()[src];

    for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
        QString m = QMakeSourceFileInfo::mocFile(*it);
        if(!m.isEmpty()) {
            QString deps;
            if(!project->isActiveConfig("no_mocdepend"))
                deps += "$(MOC) ";
            deps += (*it) + " " + findDependencies((m)).join(" ");
            t << m << ": " << deps << "\n\t"
              << "$(MOC)" << " $(DEFINES) $(INCPATH) " << varGlue("QMAKE_COMPILER_DEFINES","-D"," -D"," ")
              << (*it) << " -o " << m << endl << endl;
        }
    }
}

void
MakefileGenerator::writeYaccSrc(QTextStream &t, const QString &src)
{
    QStringList &l = project->variables()[src];
    if(project->isActiveConfig("yacc_no_name_mangle") && l.count() > 1)
        warn_msg(WarnLogic, "yacc_no_name_mangle specified, but multiple parsers expected."
                 "This can lead to link problems.\n");
    QString default_out_h = "y.tab.h", default_out_c = "y.tab.c";
    if(!project->isEmpty("QMAKE_YACC_HEADER"))
        default_out_h = project->first("QMAKE_YACC_HEADER");
    if(!project->isEmpty("QMAKE_YACC_SOURCE"))
        default_out_c = project->first("QMAKE_YACC_SOURCE");
    QString stringBase("$base");
    for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
        QFileInfo fi((*it));
        QString dir;
        if(fi.dirPath() != ".")
            dir = fi.dirPath() + Option::dir_sep;
        dir = fileFixify(dir, QDir::currentDirPath(), Option::output_dir);
        if(!dir.isEmpty() && dir.right(Option::dir_sep.length()) != Option::dir_sep)
            dir += Option::dir_sep;

        QString impl = dir + fi.baseName(true) + Option::yacc_mod + Option::cpp_ext.first();
        QString decl = dir + fi.baseName(true) + Option::yacc_mod + Option::h_ext.first();

        QString yaccflags = "$(YACCFLAGS)", mangle = "y";
        if(!project->isActiveConfig("yacc_no_name_mangle")) {
            mangle = fi.baseName(true);
            if(!project->isEmpty("QMAKE_YACCFLAGS_MANGLE"))
                yaccflags += " " + var("QMAKE_YACCFLAGS_MANGLE").replace(stringBase, mangle);
            else
                yaccflags += " -p " + mangle;
        }
        QString out_h = default_out_h, out_c = default_out_c;
        if(!mangle.isEmpty()) {
            out_h.replace(stringBase, mangle);
            out_c.replace(stringBase, mangle);
        }

        t << impl << ": " << (*it) << "\n\t"
          << "$(YACC) " << yaccflags << " " << (*it) << "\n\t"
          << "-$(DEL_FILE) " << impl << " " << decl << "\n\t"
          << "-$(MOVE) " << out_h << " " << decl << "\n\t"
          << "-$(MOVE) " << out_c << " " << impl << endl << endl;
        t << decl << ": " << impl << endl << endl;
    }
}

void
MakefileGenerator::writeLexSrc(QTextStream &t, const QString &src)
{
    QStringList &l = project->variables()[src];
    if(project->isActiveConfig("yacc_no_name_mangle") && l.count() > 1)
        warn_msg(WarnLogic, "yacc_no_name_mangle specified, but multiple parsers expected.\n"
                 "This can lead to link problems.\n");
    QString default_out_c = "lex.$base.c";
    if(!project->isEmpty("QMAKE_LEX_SOURCE"))
        default_out_c = project->first("QMAKE_LEX_SOURCE");
    QString stringBase("$base");
    for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
        QFileInfo fi((*it));
        QString dir;
        if(fi.dirPath() != ".")
            dir = fi.dirPath() + Option::dir_sep;
        dir = fileFixify(dir, QDir::currentDirPath(), Option::output_dir);
        if(!dir.isEmpty() && dir.right(Option::dir_sep.length()) != Option::dir_sep)
            dir += Option::dir_sep;
        QString impl = dir + fi.baseName(true) + Option::lex_mod + Option::cpp_ext.first();

        QString lexflags = "$(LEXFLAGS)", stub="yy";
        if(!project->isActiveConfig("yacc_no_name_mangle")) {
            stub = fi.baseName(true);
            lexflags += " -P" + stub;
        }
        QString out_c = default_out_c;
        if(!stub.isEmpty())
            out_c.replace(stringBase, stub);

        t << impl << ": " << (*it) << " " << findDependencies((*it)).join(" \\\n\t\t") << "\n\t"
          << ("$(LEX) " + lexflags + " ") << (*it) << "\n\t"
          << "-$(DEL_FILE) " << impl << " " << "\n\t"
          << "-$(MOVE) " << out_c << " " << impl << endl << endl;
    }
}

QString
MakefileGenerator::filePrefixRoot(const QString &root, const QString &path)
{
    if(path.length() > 2 && path[1] == ':') //c:\foo
        return path.mid(0, 2) + root + path.mid(2);
    return root + path;
}

void
MakefileGenerator::writeInstalls(QTextStream &t, const QString &installs)
{
    QString rm_dir_contents("-$(DEL_FILE)");
    if(Option::target_mode != Option::TARG_WIN_MODE) //ick
        rm_dir_contents = "-$(DEL_FILE) -r";

    QString all_installs, all_uninstalls;
    QStringList &l = project->variables()[installs];
    for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
        QString pvar = (*it) + ".path";
        if(project->variables()[(*it) + ".CONFIG"].indexOf("no_path") == -1 &&
           project->variables()[pvar].isEmpty()) {
            warn_msg(WarnLogic, "%s is not defined: install target not created\n", pvar.latin1());
            continue;
        }

        bool do_default = true;
        const QString root = "$(INSTALL_ROOT)";
        QString target, dst= fileFixify(project->variables()[pvar].first(), FileFixifyAbsolute, false);
        if(dst.right(1) != Option::dir_sep)
            dst += Option::dir_sep;
        QStringList tmp, uninst = project->variables()[(*it) + ".uninstall"];
        //other
        tmp = project->variables()[(*it) + ".extra"];
        if(tmp.isEmpty())
            tmp = project->variables()[(*it) + ".commands"]; //to allow compatible name
        if(!tmp.isEmpty()) {
            do_default = false;
            if(!target.isEmpty())
                target += "\n\t";
            target += tmp.join(" ");
        }
        //masks
        tmp = project->variables()[(*it) + ".files"];
        if(!tmp.isEmpty()) {
            if(!target.isEmpty())
                target += "\n";
            do_default = false;
            for(QStringList::Iterator wild_it = tmp.begin(); wild_it != tmp.end(); ++wild_it) {
                QString wild = Option::fixPathToLocalOS((*wild_it), false, false);
                QString dirstr = QDir::currentDirPath(), filestr = wild;
                int slsh = filestr.lastIndexOf(Option::dir_sep);
                if(slsh != -1) {
                    dirstr = filestr.left(slsh+1);
                    filestr = filestr.right(filestr.length() - slsh - 1);
                }
                if(dirstr.right(Option::dir_sep.length()) != Option::dir_sep)
                    dirstr += Option::dir_sep;
                if(QFile::exists(wild)) { //real file
                    QString file = wild;
                    QFileInfo fi(wild);
                    if(!target.isEmpty())
                        target += "\t";
                    QString dst_file = filePrefixRoot(root, dst);
                    if(fi.isDir() && project->isActiveConfig("copy_dir_files")) {
                        if(!dst_file.endsWith(Option::dir_sep))
                            dst_file += Option::dir_sep;
                        dst_file += fi.fileName();
                    }
                    QString cmd =  QString(fi.isDir() ? "-$(INSTALL_DIR)" : "-$(INSTALL_FILE)") + " \"" +
                                   Option::fixPathToTargetOS(fileFixify(wild, FileFixifyAbsolute, false), false, false) +
                                   "\" \"" + dst_file + "\"\n";
                    target += cmd;
                    if(!project->isActiveConfig("debug") &&
                       !fi.isDir() && fi.isExecutable() && !project->isEmpty("QMAKE_STRIP"))
                        target += QString("\t-") + var("QMAKE_STRIP") + " \"" +
                                  filePrefixRoot(root, fileFixify(dst + filestr, FileFixifyAbsolute, false)) +
                                  "\"\n";
                    if(!uninst.isEmpty())
                        uninst.append("\n\t");
                    uninst.append(rm_dir_contents + " \"" + filePrefixRoot(root, fileFixify(dst + filestr, FileFixifyAbsolute, false)) + "\"");
                    continue;
                }
                QString local_dirstr = dirstr;
                fixEnvVariables(local_dirstr);
                QDir dir(local_dirstr, filestr);
                for(uint x = 0; x < dir.count(); x++) {
                    QString file = dir[x];
                    if(file == "." || file == "..") //blah
                        continue;
                    if(!uninst.isEmpty())
                        uninst.append("\n\t");
                    uninst.append(rm_dir_contents + " \"" + filePrefixRoot(root, fileFixify(dst + file, FileFixifyAbsolute, false)) + "\"");
                    QFileInfo fi(Option::fixPathToTargetOS(fileFixify(dirstr + file, FileFixifyAbsolute, false), true, false));
                    if(!target.isEmpty())
                        target += "\t";
                    QString dst_file = filePrefixRoot(root, fileFixify(dst, FileFixifyAbsolute, false));
                    if(fi.isDir() && project->isActiveConfig("copy_dir_files")) {
                        if(!dst_file.endsWith(Option::dir_sep))
                            dst_file += Option::dir_sep;
                        dst_file += fi.fileName();
                    }
                    QString cmd = QString(fi.isDir() ? "-$(INSTALL_DIR)" : "-$(INSTALL_FILE)") + " \"" +
                                  Option::fixPathToTargetOS(fileFixify(dirstr + file, FileFixifyAbsolute, false), false) +
                                  "\" \"" + dst_file + "\"\n";
                    target += cmd;
                    if(!project->isActiveConfig("debug") &&
                       !fi.isDir() && fi.isExecutable() && !project->isEmpty("QMAKE_STRIP"))
                        target += QString("\t-") + var("QMAKE_STRIP") + " \"" +
                                  filePrefixRoot(root, fileFixify(dst + file, FileFixifyAbsolute, false)) +
                                  "\"\n";
                }
            }
        }
        //default?
        if(do_default) {
            target = defaultInstall((*it));
            uninst = project->variables()[(*it) + ".uninstall"];
        }

        if(!target.isEmpty()) {
            t << "install_" << (*it) << ": all ";
            const QStringList &deps = project->variables()[(*it) + ".depends"];
            if(!deps.isEmpty()) {
                for(QStringList::ConstIterator dep_it = deps.begin(); dep_it != deps.end(); ++dep_it) {
                    QString targ = var((*dep_it) + ".target");
                    if(targ.isEmpty())
                        targ = (*dep_it);
                    t << targ;
                }
            }
            t << "\n\t";
            const QStringList &dirs = project->variables()[pvar];
            for(QStringList::ConstIterator pit = dirs.begin(); pit != dirs.end(); ++pit) {
                QString tmp_dst = fileFixify((*pit), FileFixifyAbsolute, false);
                if(Option::target_mode != Option::TARG_WIN_MODE && tmp_dst.right(1) != Option::dir_sep)
                    tmp_dst += Option::dir_sep;
                t << mkdir_p_asstring(filePrefixRoot(root, tmp_dst)) << "\n\t";
            }
            t << target << endl << endl;
            if(!uninst.isEmpty()) {
                t << "uninstall_" << (*it) << ": " << "\n\t"
                  << uninst.join("") << "\n\t"
                  << "-$(DEL_DIR) \"" << filePrefixRoot(root, dst) << "\"" << endl << endl;
            }
            t << endl;

            if(project->variables()[(*it) + ".CONFIG"].indexOf("no_default_install") == -1) {
                all_installs += QString("install_") + (*it) + " ";
                if(!uninst.isEmpty())
                    all_uninstalls += "uninstall_" + (*it) + " ";
            }
        }   else {
            debug_msg(1, "no definition for install %s: install target not created",(*it).latin1());
        }
    }
    t << "install: " << all_installs << " " << var("INSTALLDEPS")   << "\n\n";
    t << "uninstall: " << all_uninstalls << " " << var("UNINSTALLDEPS") << "\n\n";
}

QString
MakefileGenerator::var(const QString &var)
{
    return val(project->variables()[var]);
}

QString
MakefileGenerator::val(const QStringList &varList)
{
    return valGlue(varList, "", " ", "");
}

QString
MakefileGenerator::varGlue(const QString &var, const QString &before, const QString &glue, const QString &after)
{
    return valGlue(project->variables()[var], before, glue, after);
}

QString
MakefileGenerator::valGlue(const QStringList &varList, const QString &before, const QString &glue, const QString &after)
{
    QString ret;
    for(QStringList::ConstIterator it = varList.begin(); it != varList.end(); ++it) {
        if(!(*it).isEmpty()) {
            if(!ret.isEmpty())
                ret += glue;
            ret += (*it);
        }
    }
    return ret.isEmpty() ? QString("") : before + ret + after;
}


QString
MakefileGenerator::varList(const QString &var)
{
    return valList(project->variables()[var]);
}

QString
MakefileGenerator::valList(const QStringList &varList)
{
    return valGlue(varList, "", " \\\n\t\t", "");
}


QStringList
MakefileGenerator::createObjectList(const QString &var)
{
    QStringList &l = project->variables()[var], ret;
    QString objdir, dir;
    if(!project->variables()["OBJECTS_DIR"].isEmpty())
        objdir = project->first("OBJECTS_DIR");
    for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
        QFileInfo fi(Option::fixPathToLocalOS((*it)));
        if(objdir.isEmpty() && project->isActiveConfig("object_with_source")) {
            QString fName = Option::fixPathToTargetOS((*it), false);
            int dl = fName.lastIndexOf(Option::dir_sep);
            if(dl != -1)
                dir = fName.left(dl + 1);
        } else {
            dir = objdir;
        }
        ret.append(fileFixify(dir + fi.baseName(true) + Option::obj_ext, QDir::currentDirPath(), Option::output_dir));
    }
    return ret;
}

QString
MakefileGenerator::createMocFileName(const QString &file)
{
    bool from_cpp = false;
    for(QStringList::Iterator it =  Option::cpp_ext.begin(); it !=  Option::cpp_ext.end(); ++it) {
        if(file.endsWith((*it))) {
            from_cpp = true;
            break;
        }
    }

    QString ret;
    int dir_pos = file.lastIndexOf(Option::dir_sep), ext_pos = file.indexOf('.', dir_pos == -1 ? 0 : dir_pos);
    if(!project->isEmpty("MOC_DIR"))
        ret = project->first("MOC_DIR");
    else if(dir_pos != -1)
        ret = file.left(dir_pos+1);

    if(from_cpp)
        ret += Option::cpp_moc_mod + file.mid(dir_pos+1, ext_pos - dir_pos-1) + Option::cpp_moc_ext;
    else
        ret += Option::h_moc_mod + file.mid(dir_pos+1, ext_pos - dir_pos-1) + Option::h_moc_ext;

    if(!ret.isNull())
        ret = Option::fixPathToTargetOS(fileFixify(ret, QDir::currentDirPath(), Option::output_dir));
    return ret;
}

QString
MakefileGenerator::replaceExtraCompilerVariables(const QString &var, const QString &in, const QString &out)
{
    QString ret = var;
    if(!in.isNull()) {
        QFileInfo fi(Option::fixPathToLocalOS(in));
        ret.replace("${QMAKE_FILE_BASE}", fi.baseName());
        ret.replace("${QMAKE_FILE_NAME}", fi.fileName());
        ret.replace("${QMAKE_FILE_IN}", fi.filePath());
    }
    if(!out.isNull()) {
        ret.replace("${QMAKE_FILE_OUT}", out);
    }
    return ret;
}

void
MakefileGenerator::writeExtraTargets(QTextStream &t)
{
    QStringList &qut = project->variables()["QMAKE_EXTRA_TARGETS"];
    for(QStringList::Iterator it = qut.begin(); it != qut.end(); ++it) {
        QString targ = var((*it) + ".target"),
                 cmd = var((*it) + ".commands"), deps;
        if(targ.isEmpty())
            targ = (*it);
        QStringList &deplist = project->variables()[(*it) + ".depends"];
        for(QStringList::Iterator dep_it = deplist.begin(); dep_it != deplist.end(); ++dep_it) {
            QString dep = var((*dep_it) + ".target");
            if(dep.isEmpty())
                dep = (*dep_it);
            deps += " " + dep;
        }
        if(!project->variables()["QMAKE_NOFORCE"].isEmpty() &&
           project->variables()[(*it) + ".CONFIG"].indexOf("phony") != -1)
            deps += QString(" ") + "FORCE";
        t << targ << ":" << deps << "\n\t"
          << cmd << endl << endl;
    }
}

void
MakefileGenerator::writeExtraCompilerTargets(QTextStream &t)
{
    QString clean_targets;
    const QStringList &quc = project->variables()["QMAKE_EXTRA_COMPILERS"];
    for(QStringList::ConstIterator it = quc.begin(); it != quc.end(); ++it) {
        QString tmp_out = project->variables()[(*it) + ".output"].first();
        QString tmp_cmd = project->variables()[(*it) + ".commands"].join(" ");
        QString tmp_dep = project->variables()[(*it) + ".depends"].join(" ");
        QString tmp_dep_cmd = project->variables()[(*it) + ".depend_command"].join(" ");
        QStringList &vars = project->variables()[(*it) + ".variables"];
        if(tmp_out.isEmpty() || tmp_cmd.isEmpty())
            continue;
        const QStringList &tmp_inputs = project->variables()[(*it) + ".input"];
        if(project->variables()[(*it) + ".CONFIG"].indexOf("no_clean") == -1) {
            QString tmp_clean = project->variables()[(*it) + ".clean"].join(" ");
            QString tmp_clean_cmds = project->variables()[(*it) + ".clean_commands"].join(" ");
            clean_targets += QString("compiler_" + (*it) + "_clean ");
            t << "compiler_" << (*it) << "_clean:";
            bool wrote_clean_cmds = false, wrote_clean = false;
            if(tmp_clean_cmds.isEmpty()) {
                wrote_clean_cmds = true;
            } else if(tmp_clean_cmds.indexOf("${QMAKE_") == -1) {
                t << "\n\t" << tmp_clean_cmds;
                wrote_clean_cmds = true;
            }
            if(tmp_clean.isEmpty())
                tmp_clean = tmp_out;
            if(tmp_clean.indexOf("${QMAKE_") == -1) {
                t << "\n\t" << "-$(DEL_FILE) " << tmp_clean;
                wrote_clean = true;
            }
            if(!wrote_clean_cmds || !wrote_clean) {
                QStringList cleans;
                for(QStringList::ConstIterator it2 = tmp_inputs.begin(); it2 != tmp_inputs.end(); ++it2) {
                    const QStringList &tmp = project->variables()[(*it2)];
                    for(QStringList::ConstIterator input = tmp.begin(); input != tmp.end(); ++input) {
                        if(!wrote_clean)
                            cleans.append("-$(DEL_FILE) " + replaceExtraCompilerVariables(tmp_clean, (*input),
                                          replaceExtraCompilerVariables(tmp_out, (*input), QString::null)));
                        if(!wrote_clean_cmds)
                            cleans.append(replaceExtraCompilerVariables(tmp_clean_cmds, (*input),
                                          replaceExtraCompilerVariables(tmp_out, (*input), QString::null)));
                    }
                }
                if(!cleans.isEmpty())
                    t << valGlue(cleans, "\n\t", "\n\t", "");
            }
            t << endl;
        }
        if(project->variables()[(*it) + ".CONFIG"].indexOf("combine") != -1) {
            if(tmp_out.indexOf("${QMAKE_") != -1) {
                warn_msg(WarnLogic, "QMAKE_EXTRA_COMPILERS(%s) with combine has variable output.",
                         (*it).latin1());
                continue;
            }
            QString inputs, deps;
            for(QStringList::ConstIterator it2 = tmp_inputs.begin(); it2 != tmp_inputs.end(); ++it2) {
                const QStringList &tmp = project->variables()[(*it2)];
                for(QStringList::ConstIterator input = tmp.begin(); input != tmp.end(); ++input) {
                    deps += " " + findDependencies((*input)).join(" ");
                    inputs += " " + Option::fixPathToTargetOS((*input), false);
                }
            }
            if (inputs.isEmpty())
                continue;
            QString cmd = replaceExtraCompilerVariables(tmp_cmd, QString::null, tmp_out);
            if(!tmp_dep.isEmpty())
                deps = " " + tmp_dep;
            if(!tmp_dep_cmd.isEmpty() && doDepends()) {
                char buff[256];
                QString dep_cmd = replaceExtraCompilerVariables(tmp_dep_cmd, QString::null, tmp_out);
                if(FILE *proc = QT_POPEN(dep_cmd.latin1(), "r")) {
                    while(!feof(proc)) {
                        int read_in = fread(buff, 1, 255, proc);
                        if(!read_in)
                            break;
                        int l = 0;
                        for(int i = 0; i < read_in; i++) {
                            if(buff[i] == '\n' || buff[i] == ' ') {
                                deps += " " + QByteArray(buff+l, (i - l) + 1);
                                l = i;
                            }
                        }
                    }
                    fclose(proc);
                }
            }
            deps = replaceExtraCompilerVariables(deps, QString::null, tmp_out);
            t << tmp_out << ": " << inputs << " " << deps << "\n\t"
              << cmd.replace("${QMAKE_FILE_IN}", inputs) << endl << endl;
            continue;
        }
        for(QStringList::ConstIterator it2 = tmp_inputs.begin(); it2 != tmp_inputs.end(); ++it2) {
            const QStringList &tmp = project->variables()[(*it2)];
            for(QStringList::ConstIterator input = tmp.begin(); input != tmp.end(); ++input) {
                QString in = Option::fixPathToTargetOS((*input), false),
                      deps = " " + findDependencies((*input)).join(" ");
                if(!tmp_dep.isEmpty())
                    deps = " " + tmp_dep;
                QString out = replaceExtraCompilerVariables(tmp_out, (*input), QString::null);
                QString cmd = replaceExtraCompilerVariables(tmp_cmd, (*input), out);
                for(QStringList::Iterator it3 = vars.begin(); it3 != vars.end(); ++it3)
                    cmd.replace("$(" + (*it3) + ")", "$(QMAKE_COMP_" + (*it3)+")");
                if(!tmp_dep_cmd.isEmpty() && doDepends()) {
                    char buff[256];
                    QString dep_cmd = replaceExtraCompilerVariables(tmp_dep_cmd, (*input), out);
                    if(FILE *proc = QT_POPEN(dep_cmd.latin1(), "r")) {
                        while(!feof(proc)) {
                            int read_in = fread(buff, 1, 255, proc);
                            if(!read_in)
                                break;
                            int l = 0;
                            for(int i = 0; i < read_in; i++) {
                                if(buff[i] == '\n' || buff[i] == ' ') {
                                    deps += " " + QByteArray(buff+l, (i - l) + 1);
                                    l = i;
                                }
                            }
                        }
                        fclose(proc);
                    }
                }
                deps = replaceExtraCompilerVariables(deps, (*input), out);
                t << out << ": " << in << deps << "\n\t"
                  << cmd << endl << endl;
            }
        }
    }
    t << "compiler_clean: " << clean_targets << endl << endl;
}

void
MakefileGenerator::writeExtraCompilerVariables(QTextStream &t)
{
    bool first = true;
    const QStringList &comps = project->variables()["QMAKE_EXTRA_COMPILERS"];
    for(QStringList::ConstIterator compit = comps.begin(); compit != comps.end(); ++compit) {
        const QStringList &vars = project->variables()[(*compit) + ".variables"];
        for(QStringList::ConstIterator varit = vars.begin(); varit != vars.end(); ++varit) {
            if(first) {
                t << "\n####### Custom Compiler Variables" << endl;
                first = false;
            }
            t << "QMAKE_COMP_" << (*varit) << " = "
              << valList(project->variables()[(*varit)]) << endl;
        }
    }
    if(!first)
        t << endl;
}

void
MakefileGenerator::writeExtraVariables(QTextStream &t)
{
    bool first = true;
    QMap<QString, QStringList> &vars = project->variables();
    QStringList &exports = project->variables()["QMAKE_EXTRA_VARIABLES"];
    for(QMap<QString, QStringList>::Iterator it = vars.begin(); it != vars.end(); ++it) {
        for(QStringList::Iterator exp_it = exports.begin(); exp_it != exports.end(); ++exp_it) {
            QRegExp rx((*exp_it), QString::CaseInsensitive, QRegExp::Wildcard);
            if(rx.exactMatch(it.key())) {
                if(first) {
                    t << "\n####### Custom Variables" << endl;
                    first = false;
                }
                t << "EXPORT_" << it.key() << " = " << it.value().join(" ") << endl;
            }
        }
    }
    if(!first)
        t << endl;
}

bool
MakefileGenerator::writeMakefile(QTextStream &t)
{
    t << "####### Compile" << endl << endl;
    writeObj(t, "OBJECTS", "SOURCES");
    writeMocObj(t, "OBJMOC", "SRCMOC");
    writeMocSrc(t, "HEADERS");
    writeMocSrc(t, "SOURCES");
    writeYaccSrc(t, "YACCSOURCES");
    writeLexSrc(t, "LEXSOURCES");

    t << "####### Install" << endl << endl;
    writeInstalls(t, "INSTALLS");
    return true;
}

QString MakefileGenerator::buildArgs()
{
    QString ret;
    //special variables
    if(!project->isEmpty("QMAKE_ABSOLUTE_SOURCE_PATH"))
        ret += " QMAKE_ABSOLUTE_SOURCE_PATH=\"" + project->first("QMAKE_ABSOLUTE_SOURCE_PATH") + "\"";

    //warnings
    else if(Option::warn_level == WarnNone)
        ret += " -Wnone";
    else if(Option::warn_level == WarnAll)
        ret += " -Wall";
    else if(Option::warn_level & WarnParser)
        ret += " -Wparser";
    //other options
    if(!Option::user_template.isEmpty())
        ret += " -t " + Option::user_template;
    if(!Option::mkfile::do_cache)
        ret += " -nocache";
    if(!Option::mkfile::do_deps)
        ret += " -nodepend";
    if(!Option::mkfile::do_mocs)
        ret += " -nomoc";
    if(!Option::mkfile::do_dep_heuristics)
        ret += " -nodependheuristics";
    if(!Option::mkfile::qmakespec_commandline.isEmpty())
        ret += " -spec " + Option::mkfile::qmakespec_commandline;

    //arguments
    for(QStringList::Iterator it = Option::before_user_vars.begin();
        it != Option::before_user_vars.end(); ++it) {
        if((*it).left(qstrlen("QMAKE_ABSOLUTE_SOURCE_PATH")) != "QMAKE_ABSOLUTE_SOURCE_PATH")
            ret += " \"" + (*it) + "\"";
    }
    if(Option::after_user_vars.count()) {
        ret += " -after ";
        for(QStringList::Iterator it = Option::after_user_vars.begin();
            it != Option::after_user_vars.end(); ++it) {
            if((*it).left(qstrlen("QMAKE_ABSOLUTE_SOURCE_PATH")) != "QMAKE_ABSOLUTE_SOURCE_PATH")
                ret += " \"" + (*it) + "\"";
        }
    }
    return ret;
}

//could get stored argv, but then it would have more options than are
//probably necesary this will try to guess the bare minimum..
QString MakefileGenerator::build_args()
{
    QString ret = "$(QMAKE)";

    // general options and arguments
    ret += buildArgs();

    //output
    QString ofile = Option::fixPathToTargetOS(fileFixify(Option::output.name()));
    if(!ofile.isEmpty() && ofile != project->first("QMAKE_MAKEFILE"))
        ret += " -o " + ofile;

    //inputs
    QStringList files = fileFixify(Option::mkfile::project_files);
    ret += " " + files.join(" ");
    return ret;
}

void
MakefileGenerator::writeHeader(QTextStream &t)
{
    time_t foo = time(NULL);
    t << "#############################################################################" << endl;
    t << "# Makefile for building: " << var("TARGET") << endl;
    t << "# Generated by qmake (" << qmake_version() << ") (Qt " << QT_VERSION_STR << ") on: " << ctime(&foo);
    t << "# Project:  " << fileFixify(project->projectFile()) << endl;
    t << "# Template: " << var("TEMPLATE") << endl;
    t << "# Command: " << build_args() << endl;
    t << "#############################################################################" << endl;
    t << endl;
}

void
MakefileGenerator::writeSubDirs(QTextStream &t)
{
    QList<SubTarget*> targets;
    {
        QStringList subdirs = project->variables()["SUBDIRS"];
        for(QStringList::Iterator it = subdirs.begin(); it != subdirs.end(); ++it) {
            QString file = (*it);
            SubTarget *st = new SubTarget;
            targets.append(st);
            st->makefile = "$(MAKEFILE)";
            if((*it).endsWith(Option::pro_ext)) {
                int slsh = file.lastIndexOf(Option::dir_sep);
                if(slsh != -1) {
                    st->directory = file.left(slsh+1);
                    st->profile = file.mid(slsh+1);
                } else {
                    st->profile = file;
                }
            } else {
                if(!file.isEmpty() && !project->isActiveConfig("subdir_first_pro"))
                    st->profile = file.section(Option::dir_sep, -1) + Option::pro_ext;
                st->directory = file;
            }
            while(st->directory.right(1) == Option::dir_sep)
                st->directory = st->directory.left(st->directory.length() - 1);
            if(!st->profile.isEmpty()) {
                QString basename = st->directory;
                int new_slsh = basename.lastIndexOf(Option::dir_sep);
                if(new_slsh != -1)
                    basename = basename.mid(new_slsh+1);
                if(st->profile != basename + Option::pro_ext)
                    st->makefile += "." + st->profile.left(st->profile.length() - Option::pro_ext.length()); //no need for the .pro
            }
            st->target = "sub-" + (*it);
            st->target.replace('/', '-');
            st->target.replace('.', '_');
        }
    }
    t << "first: make_first" << endl;
    writeSubTargets(t, targets, true);

    if (project->isActiveConfig("ordered")) {         // generate dependencies
        QStringList targs;
        targs << "" << "-make_first" << "-all" << "-install_subtargets" << "-uninstall_subtargets";

        for(QList<SubTarget*>::ConstIterator it = targets.constBegin(); it != targets.constEnd();) {
            QString tar = (*it)->target;
            ++it;
            for (QStringList::ConstIterator tit = targs.constBegin(); tit != targs.constEnd(); ++tit) {
                if (it != targets.end())
                    t << (*it)->target << *tit << ": " << tar << *tit << endl;
            }
        }
        t << endl;
    }
}

void
MakefileGenerator::writeSubTargets(QTextStream &t, QList<MakefileGenerator::SubTarget*> targets, bool installs)
{
    // blasted includes
    QStringList &qeui = project->variables()["QMAKE_EXTRA_INCLUDES"];
    for(QStringList::Iterator qeui_it = qeui.begin(); qeui_it != qeui.end(); ++qeui_it)
        t << "include " << (*qeui_it) << endl;

    QString ofile = Option::output.name();
    if(ofile.lastIndexOf(Option::dir_sep) != -1)
        ofile = ofile.right(ofile.length() - ofile.lastIndexOf(Option::dir_sep) -1);
    t << "MAKEFILE =        " << ofile << endl;
    t << "QMAKE    =        " << var("QMAKE_QMAKE") << endl;
    t << "DEL_FILE =    " << var("QMAKE_DEL_FILE") << endl;
    t << "CHK_DIR_EXISTS= " << var("QMAKE_CHK_DIR_EXISTS") << endl;
    t << "MKDIR    = " << var("QMAKE_MKDIR") << endl;
    t << "INSTALL_FILE= " << var("QMAKE_INSTALL_FILE") << endl;
    t << "INSTALL_DIR = " << var("QMAKE_INSTALL_DIR") << endl;
    writeExtraVariables(t);
    t << "SUBTARGETS =        ";     // subtargets are sub-directory
    for(QList<SubTarget*>::Iterator it = targets.begin(); it != targets.end(); ++it)
        t << " \\\n\t\t" << (*it)->target;
    t << endl << endl;

    QStringList targs;
    targs << "make_first" << "all" << "clean" << "distclean" << "mocables"
          << QString(installs ? "install_subtargets" : "install")
          << QString(installs ? "uninstall_subtargets" : "uninstall")
          << "mocclean";

    // generate target rules
    for(QList<SubTarget*>::Iterator it = targets.begin(); it != targets.end(); ++it) {
        bool have_dir = !(*it)->directory.isEmpty();
        QString mkfile = (*it)->makefile, cdin, cdout;
        if(have_dir) {
            mkfile.prepend((*it)->directory + Option::dir_sep);
            if(project->isActiveConfig("cd_change_global")) {
                cdin = "\n\t@cd " + (*it)->directory + "\n\t";
                cdout = "\n\t@cd ..";
                const int subLevels = (*it)->directory.count(Option::dir_sep);
                for(int i = 0; i < subLevels; i++)
                    cdout += Option::dir_sep + "..";
            } else {
                cdin = "\n\tcd " + (*it)->directory + " && ";
            }
        } else {
            cdin = "\n\t";
        }

        //qmake it
        if(!(*it)->profile.isEmpty()) {
            QString out, in = (*it)->profile;
            if((*it)->makefile != "$(MAKEFILE)")
                out = " -o " + (*it)->makefile;
            if(in.startsWith((*it)->directory + Option::dir_sep))
                in = in.mid((*it)->directory.length() + 1);
            t << mkfile << ": " << "\n\t"
              << mkdir_p_asstring((*it)->directory)
              << cdin
              << "$(QMAKE) " << in << buildArgs() << out
              << cdout << endl;
            t << (*it)->target << "-qmake_all: " << "\n\t"
              << mkdir_p_asstring((*it)->directory)
              << cdin
              << "$(QMAKE) " << in << buildArgs() << out
              << cdout << endl;
        }

        //actually compile
        t << (*it)->target << ": " << mkfile << "\n\t";
        if(have_dir)
            t << "cd " << (*it)->directory << " && ";
        t << "$(MAKE) -f " << (*it)->makefile << endl;
        for(QStringList::Iterator targ_it = targs.begin(); targ_it != targs.end(); ++targ_it) {
            QString targ = (*targ_it);
            if(targ == "install_subtargets")
                targ = "install";
            else if(targ == "uninstall_subtargets")
                targ = "uninstall";
            else if(targ == "make_first")
                targ = "first";
            t << (*it)->target << "-" << (*targ_it) << ": " << mkfile
              << cdin
              << "$(MAKE) -f " << (*it)->makefile << " " << targ
              << cdout << endl;
        }
    }
    t << endl;

    if(project->variables()["QMAKE_INTERNAL_QMAKE_DEPS"].indexOf("qmake_all") == -1)
        project->variables()["QMAKE_INTERNAL_QMAKE_DEPS"].append("qmake_all");

    writeMakeQmake(t);

    t << "qmake_all:";
    if(!targets.isEmpty()) {
        for(QList<SubTarget*>::Iterator it = targets.begin(); it != targets.end(); ++it) {
            if(!(*it)->profile.isEmpty())
                t << " " << (*it)->target << "-" << "qmake_all";
        }
        if(project->isActiveConfig("no_empty_targets"))
            t << "\t" << "@cd .";
    }
    t << endl << endl;

    for(QStringList::Iterator targ_it = targs.begin(); targ_it != targs.end(); ++targ_it) {
        t << (*targ_it) << ":";
        QString targ = (*targ_it);
        for(QList<SubTarget*>::Iterator it = targets.begin(); it != targets.end(); ++it)
            t << " " << (*it)->target << "-" << (*targ_it);
        t << endl;
        if(targ == "clean")
            t << varGlue("QMAKE_CLEAN","\t-$(DEL_FILE) ","\n\t-$(DEL_FILE) ", "\n");
        else if(project->isActiveConfig("no_empty_targets"))
            t << "\t" << "@cd ." << endl;
    }

    // user defined targets
    QStringList &qut = project->variables()["QMAKE_EXTRA_TARGETS"];
    for(QStringList::Iterator qut_it = qut.begin(); qut_it != qut.end(); ++qut_it) {
        QString targ = var((*qut_it) + ".target"),
                 cmd = var((*qut_it) + ".commands"), deps;
        if(targ.isEmpty())
            targ = (*qut_it);
        QStringList &deplist = project->variables()[(*qut_it) + ".depends"];
        for(QStringList::Iterator dep_it = deplist.begin(); dep_it != deplist.end(); ++dep_it) {
            QString dep = var((*dep_it) + ".target");
            if(dep.isEmpty())
                dep = (*dep_it);
            deps += " " + dep;
        }
        if(!project->variables()["QMAKE_NOFORCE"].isEmpty() &&
           project->variables()[(*qut_it) + ".CONFIG"].indexOf("phony") != -1)
            deps += " FORCE";
        t << "\n\n" << targ << ":" << deps << "\n\t"
          << cmd << endl;
    }

    if(installs) {
        project->variables()["INSTALLDEPS"]   += "install_subdirs";
        project->variables()["UNINSTALLDEPS"] += "uninstall_subdirs";
        writeInstalls(t, "INSTALLS");
    }

    if(project->variables()["QMAKE_NOFORCE"].isEmpty())
        t <<"FORCE:" << endl << endl;
}

void
MakefileGenerator::writeMakeQmake(QTextStream &t)
{
    QString ofile = Option::fixPathToTargetOS(fileFixify(Option::output.name()));
    if(project->isEmpty("QMAKE_FAILED_REQUIREMENTS") && !project->isActiveConfig("no_autoqmake") &&
       !project->isEmpty("QMAKE_INTERNAL_PRL_FILE")) {
        QStringList files = fileFixify(Option::mkfile::project_files);
        t << project->first("QMAKE_INTERNAL_PRL_FILE") << ": " << "\n\t"
          << "@$(QMAKE) -prl " << buildArgs() << " " << files.join(" ") << endl;
    }

    QString pfile = project->projectFile();
    if(pfile != "(stdin)") {
        QString qmake = build_args();
        if(!ofile.isEmpty() && !project->isActiveConfig("no_autoqmake")) {
            t << ofile << ": " << fileFixify(pfile) << " ";
            if(Option::mkfile::do_cache)
                t <<  fileFixify(Option::mkfile::cachefile) << " ";
            if(!specdir().isEmpty()) {
                if(QFile::exists(Option::fixPathToLocalOS(specdir()+QDir::separator()+"qmake.conf")))
                    t << specdir() << Option::dir_sep << "qmake.conf" << " ";
                else if(QFile::exists(Option::fixPathToLocalOS(specdir()+QDir::separator()+"tmake.conf")))
                    t << specdir() << Option::dir_sep << "tmake.conf" << " ";
            }
            const QStringList &included = project->variables()["QMAKE_INTERNAL_INCLUDED_FILES"];
            t << included.join(" \\\n\t\t") << "\n\t"
              << qmake << endl;
            for(QStringList::ConstIterator it = included.begin(); it != included.end(); ++it)
                t << (*it) << ":" << endl;
        }
        if(project->first("QMAKE_ORIG_TARGET") != "qmake") {
            t << "qmake: " <<
                project->variables()["QMAKE_INTERNAL_QMAKE_DEPS"].join(" \\\n\t\t") << "\n\t"
              << "@" << qmake << endl << endl;
        }
    }
}

QStringList
MakefileGenerator::fileFixify(const QStringList& files, const QString &out_dir, const QString &in_dir,
                              FileFixifyType fix, bool canon) const
{
    if(files.isEmpty())
        return files;
    QStringList ret;
    for(QStringList::ConstIterator it = files.begin(); it != files.end(); ++it) {
        if(!(*it).isEmpty())
            ret << fileFixify((*it), out_dir, in_dir, fix, canon);
    }
    return ret;
}

QString
MakefileGenerator::fileFixify(const QString& file0, const QString &out_d, const QString &in_d,
                              FileFixifyType fix, bool canon) const
{
    if(file0.isEmpty())
        return file0;
    QString key = file0;
    if(QDir::isRelativePath(file0))
        key.prepend(QDir::currentDirPath() + "--");
    if(!in_d.isEmpty() || !out_d.isEmpty() || fix != FileFixifyDefault || !canon)
        key.prepend(in_d + "--" + out_d + "--" + QString::number((int)fix) + "--" +
                    QString::number((int)canon) + "-");
    if(fileFixed.contains(key))
        return fileFixed[key];

    QString file = file0;
    int depth = 4;
    if(Option::qmake_mode == Option::QMAKE_GENERATE_MAKEFILE ||
       Option::qmake_mode == Option::QMAKE_GENERATE_PRL) {
        if(project && !project->isEmpty("QMAKE_PROJECT_DEPTH"))
            depth = project->first("QMAKE_PROJECT_DEPTH").toInt();
        else if(Option::mkfile::cachefile_depth != -1)
            depth = Option::mkfile::cachefile_depth;
    }

    QChar quote;
    if((file.startsWith("'") || file.startsWith("\"")) && file.startsWith(file.right(1))) {
        quote = file.at(0);
        file = file.mid(1, file.length() - 2);
    }
    QString orig_file = file;
    if(fix == FileFixifyAbsolute || (fix == FileFixifyDefault && project->isActiveConfig("no_fixpath"))) {
        if(!project->isEmpty("QMAKE_ABSOLUTE_SOURCE_PATH")) { //absoluteify it
            QString qfile = Option::fixPathToLocalOS(file, true, canon);
            if(QDir::isRelativePath(file)) { //already absolute
                QFileInfo fi(qfile);
                if(!fi.convertToAbs()) //strange
                    file = fi.filePath();
            }
        }
    } else { //fix it..
        QString qfile(Option::fixPathToLocalOS(file, true, canon)), in_dir(in_d), out_dir(out_d);
        {
            if(out_dir.isNull() || QDir::isRelativePath(out_dir))
                out_dir.prepend(Option::output_dir + QDir::separator());
            if(out_dir == ".")
                out_dir = QDir::currentDirPath();
            if(in_dir.isEmpty() || QDir::isRelativePath(in_dir))
                in_dir.prepend(QDir::currentDirPath() + QDir::separator());
            if(in_dir == ".")
                in_dir = QDir::currentDirPath();

            if(!QDir::isRelativePath(in_dir) || !QDir::isRelativePath(out_dir)) {
                QFileInfo in_fi(in_dir);
                if(!in_fi.convertToAbs())
                    in_dir = in_fi.filePath();
                QFileInfo out_fi(out_dir);
                if(!out_fi.convertToAbs())
                    out_dir = out_fi.filePath();
            }
            if(QFile::exists(in_dir))
                in_dir = QDir(in_dir).canonicalPath();
            if(QFile::exists(out_dir))
                out_dir = QDir(out_dir).canonicalPath();
        }
        if(out_dir != in_dir || !QDir::isRelativePath(qfile)) {
            if(QDir::isRelativePath(qfile)) {
                if(file.left(Option::dir_sep.length()) != Option::dir_sep &&
                   in_dir.right(Option::dir_sep.length()) != Option::dir_sep)
                    file.prepend(Option::dir_sep);
                file.prepend(in_dir);
            }
            file = Option::fixPathToTargetOS(file, false, canon);
            if(canon && QFile::exists(file) && file == Option::fixPathToTargetOS(file, true, canon)) {
                QString real_file = QDir(file).canonicalPath();
                if(!real_file.isEmpty())
                    file = real_file;
            }
            QString match_dir = Option::fixPathToTargetOS(out_dir, false, canon);
            if(file == match_dir) {
                file = "";
            } else if(file.startsWith(match_dir) &&
               file.mid(match_dir.length(), Option::dir_sep.length()) == Option::dir_sep) {
                file = file.right(file.length() - (match_dir.length() + 1));
            } else {
                for(int i = 1; i <= depth; i++) {
                    int sl = match_dir.lastIndexOf(Option::dir_sep);
                    if(sl == -1)
                        break;
                    match_dir = match_dir.left(sl);
                    if(match_dir.isEmpty())
                        break;
                    if(file.startsWith(match_dir) &&
                       file.mid(match_dir.length(), Option::dir_sep.length()) == Option::dir_sep) {
                        //concat
                        int remlen = file.length() - (match_dir.length() + 1);
                        if(remlen < 0)
                            remlen = 0;
                        file = file.right(remlen);
                        //prepend
                        for(int o = 0; o < i; o++)
                            file.prepend(".." + Option::dir_sep);
                    }
                }
            }
        }
    }
    file = Option::fixPathToTargetOS(file, false, canon);
    if(file.isEmpty())
        file = ".";
    if(!quote.isNull())
        file = quote + file + quote;
    debug_msg(3, "Fixed %s :: to :: %s (%d) [%s::%s]", orig_file.latin1(), file.latin1(), depth,
              in_d.latin1(), out_d.latin1());
    ((MakefileGenerator*)this)->fileFixed.insert(key, file);
    return file;
}

void
MakefileGenerator::checkMultipleDefinition(const QString &f, const QString &w)
{
    if(!(Option::warn_level & WarnLogic))
        return;
    QString file = f;
    int slsh = f.lastIndexOf(Option::dir_sep);
    if(slsh != -1)
        file = file.right(file.length() - slsh - 1);
    QStringList &l = project->variables()[w];
    for(QStringList::Iterator val_it = l.begin(); val_it != l.end(); ++val_it) {
        QString file2((*val_it));
        slsh = file2.lastIndexOf(Option::dir_sep);
        if(slsh != -1)
            file2 = file2.right(file2.length() - slsh - 1);
        if(file2 == file) {
            warn_msg(WarnLogic, "Found potential symbol conflict of %s (%s) in %s",
                     file.latin1(), (*val_it).latin1(), w.latin1());
            break;
        }
    }
}

QMakeLocalFileName
MakefileGenerator::fixPathForFile(const QMakeLocalFileName &file)
{
    return QMakeLocalFileName(fileFixify(file.real()));
}

QMakeLocalFileName
MakefileGenerator::findFileForMoc(const QMakeLocalFileName &file)
{
    QString ret = createMocFileName(file.local());
    if(ret.endsWith(Option::cpp_moc_ext)) { //.moc
        project->variables()["_SRCMOC"].append(ret);
    } else {
        checkMultipleDefinition(ret, "SOURCES");
        project->variables()["_HDRMOC"].append(ret);
    }
    return QMakeLocalFileName(ret);
}

QMakeLocalFileName MakefileGenerator::findFileForDep(const QMakeLocalFileName &file)
{
    QMakeLocalFileName ret;
    if(!project->isEmpty("SKIP_DEPENDS")) {
        bool found = false;
        QStringList &nodeplist = project->values("SKIP_DEPENDS");
        for(QStringList::Iterator it = nodeplist.begin();
            it != nodeplist.end(); ++it) {
            QRegExp regx((*it));
            if(regx.indexIn(file.local()) != -1) {
                found = true;
                break;
            }
        }
        if(found)
            return ret;
    }

    ret = QMakeSourceFileInfo::findFileForDep(file);
    if(!ret.isNull())
        return ret;

    //these are some hacky heuristics it will try to do on an include
    //however these can be turned off at runtime, I'm not sure how
    //reliable these will be, most likely when problems arise turn it off
    //and see if they go away..
    if(Option::mkfile::do_dep_heuristics) {
        if(depHeuristics.contains(file.real()))
            return depHeuristics[file.real()];

        { //is it form an EXTRA_TARGET
            QStringList &qut = project->variables()["QMAKE_EXTRA_TARGETS"];
            for(QStringList::Iterator it = qut.begin(); it != qut.end(); ++it) {
                QString targ = var((*it) + ".target");
                if(targ.isEmpty())
                    targ = (*it);
                if(targ.endsWith(file.real())) {
                    ret = QMakeLocalFileName(targ);
                    goto found_dep_from_heuristic;
                }
            }
        }
        { //is it from an EXTRA_COMPILER
            const QStringList &quc = project->variables()["QMAKE_EXTRA_COMPILERS"];
            for(QStringList::ConstIterator it = quc.begin(); it != quc.end(); ++it) {
                QString tmp_out = project->variables()[(*it) + ".output"].first();
                if(tmp_out.isEmpty())
                    continue;
                QStringList &tmp = project->variables()[(*it) + ".input"];
                for(QStringList::Iterator it2 = tmp.begin(); it2 != tmp.end(); ++it2) {
                    QStringList &inputs = project->variables()[(*it2)];
                    for(QStringList::Iterator input = inputs.begin(); input != inputs.end(); ++input) {
                        QString out = replaceExtraCompilerVariables(tmp_out, (*input), QString::null);
                        if(out == file.real() || out.endsWith("/" + file.real())) {
                            ret = QMakeLocalFileName(out);
                            goto found_dep_from_heuristic;
                        }
                    }
                }
            }
        }
        { //is it a file from a .ui?
            QString inc_file = file.real().section(Option::dir_sep, -1);
            int extn = inc_file.lastIndexOf('.');
            if(extn != -1 &&
               (inc_file.right(inc_file.length()-extn) == Option::cpp_ext.first() ||
                inc_file.right(inc_file.length()-extn) == Option::h_ext.first())) {
                QString uip = inc_file.left(extn) + Option::ui_ext;
                QStringList uil = project->variables()["FORMS"];
                for(QStringList::Iterator it = uil.begin(); it != uil.end(); ++it) {
                    if((*it).section(Option::dir_sep, -1) == uip) {
                        QString ret_name;
                        if(!project->isEmpty("UI_DIR"))
                            ret_name = project->first("UI_DIR");
                        else if(!project->isEmpty("UI_HEADERS_DIR"))
                            ret_name = project->first("UI_HEADERS_DIR");
                        else
                            ret_name = (*it).section(Option::dir_sep, 0, -2);
                        if(!ret_name.isEmpty() && !ret_name.endsWith(Option::dir_sep))
                            ret_name += Option::dir_sep;
                        ret_name += inc_file;
                        ret_name = fileFixify(ret_name, QDir::currentDirPath(), Option::output_dir);
                        ret = QMakeLocalFileName(ret_name);
                        goto found_dep_from_heuristic;
                    }
                }
            }
            if(project->isActiveConfig("lex_included")) { //is this the lex file?
                QString rhs = Option::lex_mod + Option::cpp_ext.first();
                if(file.real().endsWith(rhs)) {
                    QString lhs = file.real().left(file.real().length() - rhs.length()) + Option::lex_ext;
                    QStringList ll = project->variables()["LEXSOURCES"];
                    for(QStringList::Iterator it = ll.begin(); it != ll.end(); ++it) {
                        QString s = (*it), d;
                        int slsh = s.lastIndexOf(Option::dir_sep);
                        if(slsh != -1) {
                            d = s.left(slsh + 1);
                            s = s.right(s.length() - slsh - 1);
                        }
                        if(!project->isEmpty("QMAKE_ABSOLUTE_SOURCE_PATH"))
                            d = project->first("QMAKE_ABSOLUTE_SOURCE_PATH");
                        if(s == lhs) {
                            QString ret_name = d + file.real();
                            ret_name = fileFixify(ret_name, QDir::currentDirPath(), Option::output_dir);
                            ret = QMakeLocalFileName(ret_name);
                            goto found_dep_from_heuristic;
                        }
                    }
                }
            }
            { //is it from a .y?
                QString rhs = Option::yacc_mod + Option::h_ext.first();
                if(file.real().endsWith(rhs)) {
                    QString lhs = file.local().left(file.local().length() - rhs.length()) + Option::yacc_ext;
                    QStringList yl = project->variables()["YACCSOURCES"];
                    for(QStringList::Iterator it = yl.begin(); it != yl.end(); ++it) {
                        QString s = (*it), d;
                        int slsh = s.lastIndexOf(Option::dir_sep);
                        if(slsh != -1) {
                            d = s.left(slsh + 1);
                            s = s.right(s.length() - slsh - 1);
                        }
                        if(!project->isEmpty("QMAKE_ABSOLUTE_SOURCE_PATH"))
                            d = project->first("QMAKE_ABSOLUTE_SOURCE_PATH");
                        if(s == lhs) {
                            QString ret_name = d + file.local();
                            ret_name = fileFixify(ret_name, QDir::currentDirPath(), Option::output_dir);
                            ret = QMakeLocalFileName(ret_name);
                            goto found_dep_from_heuristic;
                        }
                    }
                }
            }
            if(mocAware() &&                    //is it a moc file?
               (file.local().endsWith(Option::cpp_ext.first()) || file.local().endsWith(Option::cpp_moc_ext))
               || ((Option::cpp_ext.first() != Option::h_moc_ext) && file.local().endsWith(Option::h_moc_ext))) {
                QString mocs[] = { QString("_HDRMOC"), QString("_SRCMOC"), QString::null };
                for(int moc = 0; !mocs[moc].isNull(); moc++) {
                    QStringList &l = project->variables()[mocs[moc]];
                    for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
                        QString fixed_it= Option::fixPathToTargetOS((*it));
                        if(fixed_it.section(Option::dir_sep, -(file.local().count('/')+1)) == file.local()) {
                            QString ret_name = (*it);
                            if(!moc) { //Since it is include, no need to link it in as well
                                project->variables()["_SRCMOC"].append((*it));
                                l.erase(it);
                            }
                            ret_name = fileFixify(ret_name, QDir::currentDirPath(), Option::output_dir);
                            ret = QMakeLocalFileName(ret_name);
                            goto found_dep_from_heuristic;
                        }
                    }
                }
            }
        }
    found_dep_from_heuristic:
        depHeuristics.insert(file.real(), ret);
    }
    return ret;
}

QStringList
&MakefileGenerator::findDependencies(const QString &file)
{
    if(!depends.contains(file))
        depends.insert(file, QMakeSourceFileInfo::dependencies(file));
    return depends[file];
}

QString
MakefileGenerator::specdir()
{
    if(!spec.isEmpty())
        return spec;
    spec = Option::mkfile::qmakespec;
#if 0
    if(const char *d = getenv("QTDIR")) {
        QString qdir = Option::fixPathToTargetOS(QString(d));
        if(qdir.endsWith(QString(QChar(QDir::separator()))))
            qdir.truncate(qdir.length()-1);
        //fix path
        QFileInfo fi(spec);
        QString absSpec(fi.absFilePath());
        absSpec = Option::fixPathToTargetOS(absSpec);
        //replace what you can
        if(absSpec.startsWith(qdir)) {
            absSpec.replace(0, qdir.length(), "$(QTDIR)");
            spec = absSpec;
        }
    }
#else
    spec = Option::fixPathToTargetOS(spec);
#endif
    return spec;
}

bool
MakefileGenerator::openOutput(QFile &file, const QString &build) const
{
    {
        QString outdir;
        if(!file.name().isEmpty()) {
            if(QDir::isRelativePath(file.name()))
                file.setName(Option::output_dir + file.name()); //pwd when qmake was run
            QFileInfo fi(file);
            if(fi.isDir())
                outdir = file.name() + QDir::separator();
        }
        if(!outdir.isEmpty() || file.name().isEmpty()) {
            QString fname = "Makefile";
            if(!project->isEmpty("MAKEFILE"))
               fname = project->first("MAKEFILE");
            file.setName(outdir + fname);
        }
    }
    if(QDir::isRelativePath(file.name()))
        file.setName(Option::output_dir + file.name()); //pwd when qmake was run
    if(!build.isEmpty())
        file.setName(file.name() + "." + build);
    if(project->isEmpty("QMAKE_MAKEFILE"))
        project->variables()["QMAKE_MAKEFILE"].append(file.name());
    int slsh = file.name().lastIndexOf(Option::dir_sep);
    if(slsh != -1)
        createDir(file.name().left(slsh));
    if(file.open(IO_WriteOnly | IO_Translate | IO_Truncate)) {
        QFileInfo fi(Option::output);
        QString od = Option::fixPathToTargetOS((fi.isSymLink() ? fi.readLink() : fi.dirPath()));
        if(QDir::isRelativePath(od))
            od.prepend(Option::output_dir);
        Option::output_dir = od;
        return true;
    }
    return false;
}



