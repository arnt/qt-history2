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
#define S_ISDIR(m)        (((m) & S_IFMT) == S_IFDIR)
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

static bool createDir(const QString& fullPath)
{
    if(QFile::exists(fullPath))
        return false;
    QDir dirTmp;
    bool ret = true;
    QString pathComponent, tmpPath;
    QStringList hierarchy = fullPath.split(QString(Option::dir_sep));
    for(QStringList::Iterator it = hierarchy.begin(); it != hierarchy.end(); ++it) {
        pathComponent = *it + QDir::separator();
        tmpPath += pathComponent;
        if(!dirTmp.mkdir(tmpPath)) {
            ret = false;
//            break;
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
    QString currentDir = QDir::currentDirPath();
    QString dirs[] = { QString("OBJECTS_DIR"), QString("MOC_DIR"), QString("UI_HEADERS_DIR"),
                       QString("UI_SOURCES_DIR"), QString("UI_DIR"), QString("DESTDIR"),
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
            path = Option::fixPathToTargetOS(fileFixify(path, QDir::currentDirPath(), Option::output_dir));
            debug_msg(3, "Fixed output_dir %s (%s) into %s (%s)", dirs[x].latin1(), orig_path.latin1(),
                      v[dirs[x]].join("::").latin1(), path.latin1());

            QDir d;
            if(path.startsWith(Option::dir_sep)) {
                d.cd(Option::dir_sep);
                path = path.right(path.length() - 1);
            }
#ifdef Q_WS_WIN
            bool driveExists = true;
            if(!QDir::isRelativePath(path)) {
                if(QFile::exists(path.left(3))) {
                    d.cd(path.left(3));
                    path = path.right(path.length() - 3);
                } else {
                    warn_msg(WarnLogic, "%s: Cannot access drive '%s' (%s)", dirs[x].latin1(),
                             path.left(3).latin1(), path.latin1());
                    driveExists = false;
                }
            }
            if(driveExists) {
#endif
                QStringList subs = path.split(Option::dir_sep);
                for(QStringList::Iterator subit = subs.begin(); subit != subs.end(); ++subit) {
                    if(!d.cd(*subit)) {
                        d.mkdir((*subit));
                        if(d.exists((*subit)))
                            d.cd((*subit));
                        else {
                            warn_msg(WarnLogic, "%s: Cannot access directory '%s' (%s)", dirs[x].latin1(),
                                     (*subit).latin1(), path.latin1());
                            break;
                        }
                    }
                }
#ifdef Q_WS_WIN
            }
#endif
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
                                          __FILE__, __LINE__,
                                          real_dir.latin1(), QDir::separator(), regex.latin1(),
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

    //UI files
    {
        QStringList &includepath = project->variables()["INCLUDEPATH"];
        if(!project->isEmpty("UI_DIR"))
            includepath.append(project->first("UI_DIR"));
        else if(!project->isEmpty("UI_HEADERS_DIR"))
            includepath.append(project->first("UI_HEADERS_DIR"));
        QStringList &decls = v["UICDECLS"], &impls = v["UICIMPLS"];
        QStringList &l = v["FORMS"];
        for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
            QString impl, decl;
            QFileInfo fi(Option::fixPathToLocalOS((*it)));
            if(!project->isEmpty("UI_DIR")) {
                impl = decl = project->first("UI_DIR");
                QString d = fi.dirPath();
                if(d == ".")
                    d = QDir::currentDirPath();
                d = fileFixify(d, QDir::currentDirPath(), Option::output_dir);
                if(!includepath.contains(d))
                    includepath.append(d);
            } else {
                if(decl.isEmpty() && !project->isEmpty("UI_HEADERS_DIR"))
                    decl = project->first("UI_HEADERS_DIR");
                if(!decl.isEmpty() || (project->isEmpty("UI_HEADERS_DIR") &&
                                       !project->isEmpty("UI_SOURCES_DIR"))) {
                    QString d = fi.dirPath();
                    if(d == ".")
                        d = QDir::currentDirPath();
                    d = fileFixify(d, QDir::currentDirPath(), Option::output_dir);
                    if(includepath.contains(d))
                        includepath.append(d);
                }
                if(impl.isEmpty() && !project->isEmpty("UI_SOURCES_DIR"))
                    impl = project->first("UI_SOURCES_DIR");
                if(fi.dirPath() != ".") {
                    if(impl.isEmpty())
                        impl = fi.dirPath() + Option::dir_sep;
                    if(decl.isEmpty())
                        decl = fi.dirPath() + Option::dir_sep;
                }
            }
            impl = fileFixify(impl, QDir::currentDirPath(), Option::output_dir);
            if(!impl.isEmpty() && !impl.endsWith(Option::dir_sep))
                impl += Option::dir_sep;
            impl += fi.baseName(true) + Option::cpp_ext.first();
            if(Option::output_dir != QDir::currentDirPath() &&
               project->isEmpty("UI_DIR") && project->isEmpty("UI_HEADERS_DIR")) {
                QString decl_fixed = fileFixify(decl, QDir::currentDirPath(), Option::output_dir);
                if(!includepath.contains(decl_fixed))
                    includepath.append(decl_fixed);
                if(!includepath.contains(decl))
                    project->variables()["INCLUDEPATH"].append(decl);
            }
            decl = fileFixify(decl, QDir::currentDirPath(), Option::output_dir);
            if(!decl.isEmpty() && !decl.endsWith(Option::dir_sep))
                decl += Option::dir_sep;
            decl += fi.baseName(true) + Option::h_ext.first();
            checkMultipleDefinition(impl, "SOURCES");
            checkMultipleDefinition(decl, "HEADERS");
            decls.append(decl);
            impls.append(impl);
            findDependencies(impl).append(decl);

            QString mocable = createMocFileName((*it));
            checkMultipleDefinition(mocable, "SOURCES");
        }
        addSourceFiles(v["UICDECLS"], QMakeSourceFileInfo::ADD_MOC);
        v["OBJECTS"] += (v["UICOBJECTS"] = createObjectList("UICDECLS"));
    }

    //Translation files
    if(!project->isEmpty("TRANSLATIONS")) {
        QStringList &trf = project->variables()["TRANSLATIONS"];
        for(QStringList::Iterator it = trf.begin(); it != trf.end(); ++it) {
            (*it) = Option::fixPathToLocalOS((*it));
        }
    }

    //Image files
    if(!project->isEmpty("IMAGES")) {
        if(project->isEmpty("QMAKE_IMAGE_COLLECTION"))
            v["QMAKE_IMAGE_COLLECTION"].append("qmake_image_collection" + Option::cpp_ext.first());
        QString imgfile = project->first("QMAKE_IMAGE_COLLECTION");
        Option::fixPathToTargetOS(imgfile);
        if(!project->isEmpty("UI_DIR") || !project->isEmpty("UI_SOURCES_DIR")) {
            if(imgfile.indexOf(Option::dir_sep) != -1)
                imgfile = imgfile.right(imgfile.lastIndexOf(Option::dir_sep) + 1);
            imgfile.prepend((project->isEmpty("UI_DIR") ? project->first("UI_SOURCES_DIR") :
                            project->first("UI_DIR")));
            v["QMAKE_IMAGE_COLLECTION"] = QStringList(imgfile);
        }
        checkMultipleDefinition(imgfile, "SOURCES");
        if(!noIO()) {
            QStringList &l = v["IMAGES"];
            for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
                if(!QFile::exists((*it))) {
                    warn_msg(WarnLogic, "Failure to open: %s", (*it).latin1());
                    continue;
                }
                findDependencies(imgfile).append(fileFixify((*it)));
            }
        }
        v["OBJECTS"] += (v["IMAGEOBJECTS"] = createObjectList("QMAKE_IMAGE_COLLECTION"));
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
MakefileGenerator::write()
{
    usePlatformDir();
    init();
    findLibraries();
    if((Option::qmake_mode == Option::QMAKE_GENERATE_MAKEFILE || //write prl
       Option::qmake_mode == Option::QMAKE_GENERATE_PRL) &&
       project->variables()["QMAKE_FAILED_REQUIREMENTS"].isEmpty() &&
       project->isActiveConfig("create_prl") && project->first("TEMPLATE") == "lib" &&
       !project->isActiveConfig("plugin")) {
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
    char sep = QDir::separator();
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
MakefileGenerator::writeUicSrc(QTextStream &t, const QString &ui)
{
    QStringList &uil = project->variables()[ui];
    for(QStringList::Iterator it = uil.begin(); it != uil.end(); it++) {
        QString decl, impl;
        {
            QString tmp = (*it), impl_dir, decl_dir;
            decl = tmp.replace(QRegExp("\\" + Option::ui_ext + "$"), Option::h_ext.first());
            int dlen = decl.lastIndexOf(Option::dir_sep) + 1;
            tmp = (*it);
            impl = tmp.replace(QRegExp("\\" + Option::ui_ext + "$"), Option::cpp_ext.first());
            int ilen = decl.lastIndexOf(Option::dir_sep) + 1;
            if(!project->isEmpty("UI_DIR")) {
                impl_dir = project->first("UI_DIR");
                decl = project->first("UI_DIR") + decl.right(decl.length() - dlen);
                impl = project->first("UI_DIR") + impl.right(impl.length() - ilen);
            } else {
                if(!project->isEmpty("UI_HEADERS_DIR")) {
                    decl_dir = project->first("UI_HEADERS_DIR");
                    decl = project->first("UI_HEADERS_DIR") + decl.right(decl.length() - dlen);
                }
                if(!project->isEmpty("UI_SOURCES_DIR")) {
                    impl_dir = project->first("UI_SOURCES_DIR");
                    impl = project->first("UI_SOURCES_DIR") + impl.right(impl.length() - ilen);
                }
            }
            impl = fileFixify(impl, QDir::currentDirPath(), Option::output_dir);
            decl = fileFixify(decl, QDir::currentDirPath(), Option::output_dir);
            if(decl_dir.isEmpty())
                decl_dir = decl.section(Option::dir_sep,0,-2);
            if(impl_dir.isEmpty())
                impl_dir = impl.section(Option::dir_sep,0,-2);
            if(QDir::isRelativePath(impl_dir))
                impl_dir.prepend(Option::output_dir + Option::dir_sep);
            if(QDir::isRelativePath(decl_dir))
                decl_dir.prepend(Option::output_dir + Option::dir_sep);
            createDir(impl_dir);
            createDir(decl_dir);
        }
        QStringList deps = findDependencies((*it));
        deps.removeAll(decl); //avoid circular dependencies..
        t << decl << ": " << (*it) << " " << deps.join(" \\\n\t\t") << "\n\t"
          << "$(UIC) " << (*it) << " -o " << decl << endl << endl;

        QString mildDecl = decl;
        int k = mildDecl.lastIndexOf(Option::dir_sep);
        if(k != -1)
            mildDecl = mildDecl.mid(k + 1);
        t << impl << ": " << decl << " " << (*it) << " " << deps.join(" \\\n\t\t") << "\n\t"
          << "$(UIC)";
        t << " " << (*it) << " -i " << mildDecl << " -o " << impl << endl << endl;
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
        QString hdr = QMakeSourceFileInfo::mocFile((*sit));
        t << (*oit) << ": "
          << (*sit) << " " << findDependencies((*sit)).join(" \\\n\t\t") << " "
          << hdr << " " << findDependencies(hdr).join(" \\\n\t\t");
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
            deps += (*it);
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

void
MakefileGenerator::writeImageObj(QTextStream &t, const QString &obj)
{
    QStringList &objl = project->variables()[obj];
    QString stringSrc("$src");
    QString stringObj("$obj");

    QString uidir;
    for(QStringList::Iterator oit = objl.begin(); oit != objl.end(); oit++) {
        QString src(project->first("QMAKE_IMAGE_COLLECTION"));
        t << (*oit) << ": " << src;
        bool use_implicit_rule = !project->isEmpty("QMAKE_RUN_CXX_IMP");
        if(use_implicit_rule) {
            if(!project->isEmpty("OBJECTS_DIR") || !project->isEmpty("UI_DIR") || !project->isEmpty("UI_SOURCES_DIR")) {
                use_implicit_rule = false;
            } else {
                int dot = src.lastIndexOf('.');
                if(dot == -1 || (src.left(dot) + Option::obj_ext != (*oit)))
                    use_implicit_rule = false;
            }
        }
        if(!use_implicit_rule && !project->isEmpty("QMAKE_RUN_CXX")) {
            QString p = var("QMAKE_RUN_CXX"), srcf(src);
            p.replace(stringSrc, srcf);
            p.replace(stringObj, (*oit));
            t << "\n\t" << p;
        }
        t << endl << endl;
    }
}


void
MakefileGenerator::writeImageSrc(QTextStream &t, const QString &src)
{
    QStringList &l = project->variables()[src];
    for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
        QString gen = project->first("MAKEFILE_GENERATOR");
        if(gen == "MSVC") {
            t << (*it) << ": " << findDependencies((*it)).join(" \\\n\t\t") << "\n\t"
                << "$(UIC)  -o " << (*it) << " -embed " << project->first("QMAKE_ORIG_TARGET")
                << " -f <<\n" << findDependencies((*it)).join(" ") << "\n<<" << endl << endl;
        } else if(gen == "BMAKE") {
            t << (*it) << ": " << findDependencies((*it)).join(" \\\n\t\t") << "\n\t"
                << "$(UIC) " << " -embed " << project->first("QMAKE_ORIG_TARGET")
                << " -f &&|\n" << findDependencies((*it)).join(" ") << "\n| -o " << (*it) << endl << endl;
        } else {
            t << (*it) << ": " << findDependencies((*it)).join(" \\\n\t\t") << "\n\t"
                << "$(UIC) " << " -embed " << project->first("QMAKE_ORIG_TARGET")
                << " " << findDependencies((*it)).join(" ") << " -o " << (*it) << endl << endl;
        }
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

bool
MakefileGenerator::writeMakefile(QTextStream &t)
{
    t << "####### Compile" << endl << endl;
    writeObj(t, "OBJECTS", "SOURCES");
    writeUicSrc(t, "FORMS");
    writeObj(t, "UICOBJECTS", "UICIMPLS");
    writeMocObj(t, "OBJMOC", "SRCMOC");
    writeMocSrc(t, "HEADERS");
    writeMocSrc(t, "SOURCES");
    writeMocSrc(t, "UICDECLS");
    writeYaccSrc(t, "YACCSOURCES");
    writeLexSrc(t, "LEXSOURCES");
    writeImageObj(t, "IMAGEOBJECTS");
    writeImageSrc(t, "QMAKE_IMAGE_COLLECTION");

    t << "####### Install" << endl << endl;
    writeInstalls(t, "INSTALLS");
    return true;
}

QString MakefileGenerator::buildArgs()
{
    static QString ret;
    if(ret.isEmpty()) {
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
    }
    return ret;
}

//could get stored argv, but then it would have more options than are
//probably necesary this will try to guess the bare minimum..
QString MakefileGenerator::build_args()
{
    static QString ret;
    if(ret.isEmpty()) {
        ret = "$(QMAKE)";

        // general options and arguments
        ret += buildArgs();

        //output
        QString ofile = Option::fixPathToTargetOS(fileFixify(Option::output.name()));
        if(!ofile.isEmpty() && ofile != project->first("QMAKE_MAKEFILE"))
            ret += " -o " + ofile;

        //inputs
        QStringList files = fileFixify(Option::mkfile::project_files);
        ret += " " + files.join(" ");
    }
    return ret;
}

bool
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
    return true;
}


//makes my life easier..
bool
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
            t << project->variables()["QMAKE_INTERNAL_INCLUDED_FILES"].join(" \\\n\t\t") << "\n\t"
              << qmake <<endl;
        }
        if(project->first("QMAKE_ORIG_TARGET") != "qmake") {
            t << "qmake: " <<
                project->variables()["QMAKE_INTERNAL_QMAKE_DEPS"].join(" \\\n\t\t") << "\n\t"
              << "@" << qmake << endl << endl;
        }
    }
    return true;
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
                        break;
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
                            break;
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
                            break;
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
                            break;
                        }
                    }
                    if(!ret.isNull())
                        break;
                }
            }
        }
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
MakefileGenerator::openOutput(QFile &file) const
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
    if(project->isEmpty("QMAKE_MAKEFILE"))
        project->variables()["QMAKE_MAKEFILE"].append(file.name());
    int slsh = file.name().lastIndexOf(Option::dir_sep);
    if(slsh != -1)
        createDir(file.name().left(slsh));
    if(file.open(IO_WriteOnly | IO_Translate)) {
        QFileInfo fi(Option::output);
        QString od = Option::fixPathToTargetOS((fi.isSymLink() ? fi.readLink() : fi.dirPath()));
        if(QDir::isRelativePath(od))
            od.prepend(Option::output_dir);
        Option::output_dir = od;
        return true;
    }
    return false;
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
MakefileGenerator::create(QMakeProject *proj)
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

