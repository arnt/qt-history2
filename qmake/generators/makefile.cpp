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
                     path.left(3).toLatin1().data(), path.toLatin1().data());
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


MakefileGenerator::MakefileGenerator() : 
    init_opath_already(false), init_already(false), moc_aware(false),
    no_io(false), project(0)
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
                if(!fi.makeAbsolute()) {
                    QString cache_r = fi.path(), pwd = Option::output_dir;
                    if(pwd.startsWith(cache_r) && !pwd.startsWith(root)) {
                        pwd = Option::fixPathToTargetOS(root + pwd.mid(cache_r.length()));
                        if(QFile::exists(pwd))
                            v.insert("QMAKE_ABSOLUTE_SOURCE_PATH", QStringList(pwd));
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

    QString currentDir = QDir::currentPath(); //just to go back to

    //some builtin directories
    QString dirs[] = { QString("OBJECTS_DIR"), QString("MOC_DIR"), QString("DESTDIR"),
                       QString("SUBLIBS_DIR"), QString("DLLDESTDIR"), QString("UI_DIR"),
                       QString::null };
    for(int x = 0; true; x++) {
        if(dirs[x].isNull())
            break;
        if(!v[dirs[x]].isEmpty()) {
            const QString orig_path = v[dirs[x]].first();
#ifdef Q_WS_WIN
            // We don't want to add a separator for DLLDESTDIR on Windows
            if(!(dirs[x] == "DLLDESTDIR"))
#endif
            {
                QString &path = v[dirs[x]].first();
                path = fileFixify(path, Option::output_dir, Option::output_dir);
                if(path.right(Option::dir_sep.length()) != Option::dir_sep)
                    path += Option::dir_sep;
            }
            if(noIO())
                continue;

            QString path = project->first(dirs[x]); //not to be changed any further
            path = Option::fixPathToLocalOS(fileFixify(path, currentDir, Option::output_dir));
            debug_msg(3, "Fixed output_dir %s (%s) into %s", dirs[x].toLatin1().constData(), 
                      orig_path.toLatin1().constData(), path.toLatin1().constData());
            if(!createDir(path))
                warn_msg(WarnLogic, "%s: Cannot access directory '%s'", dirs[x].toLatin1().constData(), path.toLatin1().constData());
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
                    if(path != "." &&
                       !createDir(fileFixify(path, QDir::currentPath(), Option::output_dir)))
                        warn_msg(WarnLogic, "%s: Cannot access directory '%s'",
                                 (*it).toLatin1().constData(), path.toLatin1().constData());
                }
            }
        }
    }

    if(!v["DESTDIR"].isEmpty()) {
        QDir d(v["DESTDIR"].first());
        if(Option::fixPathToLocalOS(d.absolutePath()) == Option::fixPathToLocalOS(Option::output_dir))
            v.remove("DESTDIR");
    }
    QDir::current().cd(currentDir);
}

QMakeProject
*MakefileGenerator::projectFile() const
{
    return project;
}

void
MakefileGenerator::setProjectFile(QMakeProject *p)
{
    if(project)
        return;
    project = p;
    init();
    usePlatformDir();
    findLibraries();
    if(Option::qmake_mode == Option::QMAKE_GENERATE_MAKEFILE &&
       project->isActiveConfig("link_prl")) //load up prl's'
        processPrlFiles();
}

void
MakefileGenerator::init()
{
    initOutPaths();
    if(init_already)
        return;
    init_already = true;

    QMap<QString, QStringList> &v = project->variables();
    const QStringList &quc = v["QMAKE_EXTRA_COMPILERS"];
    
    if(!project->isEmpty("QMAKE_SUBSTITUTES")) {
        const QStringList &subs = v["QMAKE_SUBSTITUTES"];
        for(int i = 0; i < subs.size(); ++i) {
            if(!subs.at(i).endsWith(".in")) {
                warn_msg(WarnLogic, "Substitute '%s' does not end with '.in'", 
                         subs.at(i).toLatin1().constData());
                continue;
            }
            QFile in(fileFixify(subs.at(i))), out(subs.at(i).left(subs.at(i).length()-3));
            if(in.open(QFile::ReadOnly)) {
                QString contents;
                QStack<int> state;
                enum { IN_CONDITION, MET_CONDITION, PENDING_CONDITION };
                for(int count = 1; !in.atEnd(); ++count) {
                    QString line = QString::fromUtf8(in.readLine());
                    if(line.startsWith("!!IF ")) {
                        if(state.isEmpty() || state.top() == IN_CONDITION) {
                            QString test = line.mid(5, line.length()-(5+1));
                            if(project->test(test))
                                state.push(IN_CONDITION);
                            else
                                state.push(PENDING_CONDITION);
                        } else {
                            state.push(MET_CONDITION);
                        }
                    } else if(line.startsWith("!!ELIF ")) {
                        if(state.isEmpty()) {
                            warn_msg(WarnLogic, "(%s:%d): Unexpected else condition",
                                     in.fileName().toLatin1().constData(), count);
                        } else if(state.top() == PENDING_CONDITION) {
                            QString test = line.mid(7, line.length()-(7+1));
                            if(project->test(test))  {
                                state.pop();
                                state.push(IN_CONDITION);
                            }
                        } else if(state.top() == IN_CONDITION) {
                            state.pop();
                            state.push(MET_CONDITION);
                        }
                    } else if(line.startsWith("!!ELSE")) {
                        if(state.isEmpty()) {
                            warn_msg(WarnLogic, "(%s:%d): Unexpected else condition",
                                     in.fileName().toLatin1().constData(), count);
                        } else if(state.top() == PENDING_CONDITION) {
                            state.pop();
                            state.push(IN_CONDITION);
                        } else if(state.top() == IN_CONDITION) {
                            state.pop();
                            state.push(MET_CONDITION);
                        }
                    } else if(line.startsWith("!!ENDIF")) {
                        if(state.isEmpty()) 
                            warn_msg(WarnLogic, "(%s:%d): Unexpected endif",
                                     in.fileName().toLatin1().constData(), count);
                        else 
                            state.pop();
                    } else if(state.isEmpty() || state.top() == IN_CONDITION) {
                        contents += project->expand(line);
                    }
                }
                if(out.exists() && out.open(QFile::ReadOnly)) {
                    QString old = QString::fromUtf8(out.readAll());
                    if(contents == old) {
                        v["QMAKE_INTERNAL_INCLUDED_FILES"].append(subs.at(i));
                        continue;
                    }
                    out.close();
                    if(!out.remove()) {
                        warn_msg(WarnLogic, "Cannot clear substitute '%s'", 
                                 out.fileName().toLatin1().constData());
                        continue;
                    }
                }
                if(out.open(QFile::WriteOnly)) {
                    v["QMAKE_INTERNAL_INCLUDED_FILES"].append(subs.at(i));
                    out.write(contents.toUtf8());
                } else {
                    warn_msg(WarnLogic, "Cannot open substitute for output '%s'", 
                             out.fileName().toLatin1().constData());
                }
            } else {
                warn_msg(WarnLogic, "Cannot open substitute for input '%s'", 
                         in.fileName().toLatin1().constData());
            }
        }
    }

    QStringList paths;
    paths << "SOURCES" << "FORMS" << "YACCSOURCES" << "INCLUDEPATH" << "HEADERS" 
          << "HEADERS_ORIG" << "LEXSOURCES" << "QMAKE_INTERNAL_INCLUDED_FILES" 
          << "PRECOMPILED_HEADER";
    for(QStringList::ConstIterator it = quc.begin(); it != quc.end(); ++it) {
        if(!v[(*it) + ".output"].isEmpty())
            paths += v[(*it) + ".input"];
    }
    for(int y = 0; y < paths.count(); y++) {
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
        Option::mkfile::do_deps || Option::mkfile::do_mocs)
       && !noIO() && !project->isActiveConfig("no_fileio")) {
        depHeuristics.clear();
        if((Option::qmake_mode == Option::QMAKE_GENERATE_PROJECT || 
            Option::mkfile::do_deps) && 
           doDepends()) {
            //dependency paths
            QStringList incDirs = v["DEPENDPATH"] + v["QMAKE_ABSOLUTE_SOURCE_PATH"];
            if(project->isActiveConfig("depend_includepath"))
                incDirs += v["INCLUDEPATH"];
            QList<QMakeLocalFileName> deplist;
            for(QStringList::Iterator it = incDirs.begin(); it != incDirs.end(); ++it)
                deplist.append(QMakeLocalFileName((*it)));
            setDependencyPaths(deplist);
            debug_msg(1, "Dependency Directories: %s", incDirs.join(" :: ").toLatin1().constData());
            //cache info
            if(project->isActiveConfig("qmake_cache")) {
                QString cache_file;
                if(!project->isEmpty("QMAKE_INTERNAL_CACHE_FILE")) {
                    cache_file = Option::fixPathToLocalOS(project->first("QMAKE_INTERNAL_CACHE_FILE"));
                } else {
                    cache_file = ".qmake.internal.cache";
                    if(project->isActiveConfig("build_pass")) 
                        cache_file += ".BUILD." + project->first("BUILD_PASS");
                }
                if(cache_file.indexOf(QDir::separator()) == -1) 
                    cache_file.prepend(Option::output_dir + QDir::separator());
                setCacheFile(cache_file);
            }
        }
        {
            QStringList sources;
            sources << "OBJECTS" << "LEXSOURCES" << "YACCSOURCES" << "HEADERS" 
                    << "SOURCES" << "FORMS" << "PRECOMPILED_HEADER";
            for(QStringList::ConstIterator it = quc.begin(); it != quc.end(); ++it) {
                if(!v[(*it) + ".output"].isEmpty())
                    sources += v[(*it) + ".input"];
            }
            int x;
            for(x = 0; x < sources.count(); x++) {
                QStringList &l = v[sources[x]], vpath;
                for(int val_it = 0; val_it < l.count(); val_it++) {
                    QString &val = l[val_it];
                    if(!val.isEmpty()) {
                        QString file = fileFixify(val, QDir::currentPath(), 
                                                  Option::output_dir);
                        if (file.at(0) == '\"' && file.at(file.length() - 1) == '\"')
                            file = file.mid(1, file.length() - 2);

                        if(QFile::exists(file))
                            continue;
                        bool found = false;
                        if(QDir::isRelativePath(val)) {
                            if(vpath.isEmpty())
                                vpath = v["VPATH_" + sources[x]] + v["VPATH"] +
                                        v["QMAKE_ABSOLUTE_SOURCE_PATH"] + v["DEPENDPATH"];

                            for(QStringList::Iterator vpath_it = vpath.begin();
                                vpath_it != vpath.end(); ++vpath_it) {
                                QString real_dir = Option::fixPathToLocalOS((*vpath_it));
                                if(QFile::exists(real_dir + QDir::separator() + val)) {
                                    QString dir = (*vpath_it);
                                    if(dir.right(Option::dir_sep.length()) != Option::dir_sep)
                                        dir += Option::dir_sep;
                                    val = fileFixify(dir + val);
                                    found = true;
                                    debug_msg(1, "Found file through vpath %s -> %s",
                                              file.toLatin1().constData(), val.toLatin1().constData());
                                    break;
                                }
                            }
                        }
                        if(!found) {
                            QString dir, regex = val, real_dir;
                            if(regex.lastIndexOf(Option::dir_sep) != -1) {
                                dir = regex.left(regex.lastIndexOf(Option::dir_sep) + 1);
                                real_dir = fileFixify(Option::fixPathToLocalOS(dir),
                                                      QDir::currentPath(), Option::output_dir);
                                regex = regex.right(regex.length() - dir.length());
                            }
                            if(real_dir.isEmpty() || QFile::exists(real_dir)) {
                                QStringList files = QDir(real_dir).entryList(QStringList(regex));
                                if(files.isEmpty()) {
                                    debug_msg(1, "%s:%d Failure to find %s in vpath (%s)",
                                              __FILE__, __LINE__,
                                              val.toLatin1().constData(), vpath.join("::").toLatin1().constData());
                                    warn_msg(WarnLogic, "Failure to find: %s", val.toLatin1().constData());
                                    continue;
                                } else {
                                    for(int i = (int)files.count()-1; i >= 0; i--) {
                                        QString file = fileFixify(dir + files[i]);
                                        if(!i)
                                            val = file;
                                        else
                                            l.insert(val_it+1, file);
                                    }
                                    val_it += files.count();
                                }
                            } else {
                                debug_msg(1, "%s:%d Cannot match %s%c%s, as %s does not exist.",
                                          __FILE__, __LINE__, real_dir.toLatin1().constData(),
                                          QDir::separator().latin1(), 
                                          regex.toLatin1().constData(), real_dir.toLatin1().constData());
                                warn_msg(WarnLogic, "Failure to find: %s", val.toLatin1().constData());
                            }
                        }
                    }
                }
            }
            for(x = 0; x < sources.count(); x++) {
                uchar seek = 0;
                if(mocAware() && (sources[x] == "SOURCES" || sources[x] == "HEADERS") &&
                   (Option::qmake_mode == Option::QMAKE_GENERATE_PROJECT || Option::mkfile::do_mocs))
                    seek |= QMakeSourceFileInfo::SEEK_MOCS;
                if(sources[x] != "OBJECTS")
                    seek |= QMakeSourceFileInfo::SEEK_DEPS;
                if(seek) {
                    QMakeSourceFileInfo::SourceFileType type = QMakeSourceFileInfo::TYPE_C;
                    if(sources[x] == "FORMS")
                        type = QMakeSourceFileInfo::TYPE_UI;
                    else if(sources[x] == "RESOURCES")
                        type = QMakeSourceFileInfo::TYPE_QRC;
                    addSourceFiles(v[sources[x]], seek, type);
                }
            }
        }
    }

    //extra compilers (done here so it ends up in the variables post-fixified)
    for(QStringList::ConstIterator it = quc.begin(); it != quc.end(); ++it) {
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
    v["OBJECTS"] = createObjectList(v["SOURCES"]) + v["OBJECTS"]; // init variables

    //lex files
    {
        QStringList &impls = v["LEXIMPLS"];
        QStringList &l = v["LEXSOURCES"];
        for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
            QString dir;
            QFileInfo fi((*it));
            if(fi.path() != ".")
                dir = fi.path() + Option::dir_sep;
            dir = fileFixify(dir, QDir::currentPath(), Option::output_dir);
            if(!dir.isEmpty() && dir.right(Option::dir_sep.length()) != Option::dir_sep)
                dir += Option::dir_sep;
            QString impl = dir + fi.completeBaseName() + Option::lex_mod + Option::cpp_ext.first();
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
            v["OBJECTS"] += (v["LEXOBJECTS"] = createObjectList(impls));
    }
    //yacc files
    {
        QStringList &decls = v["YACCCDECLS"], &impls = v["YACCIMPLS"];
        QStringList &l = v["YACCSOURCES"];
        for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
            QString dir;
            QFileInfo fi((*it));
            if(fi.path() != ".")
                dir = fi.path() + Option::dir_sep;
            dir = fileFixify(dir, QDir::currentPath(), Option::output_dir);
            if(!dir.isEmpty() && dir.right(Option::dir_sep.length()) != Option::dir_sep)
                dir += Option::dir_sep;
            QString impl = dir + fi.completeBaseName() + Option::yacc_mod + Option::cpp_ext.first();
            checkMultipleDefinition(impl, "SOURCES");
            QString decl = dir + fi.completeBaseName() + Option::yacc_mod + Option::h_ext.first();
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
                QString lexsrc = fi.completeBaseName() + Option::lex_ext;
                if(fi.path() != ".")
                    lexsrc.prepend(fi.path() + Option::dir_sep);
                if(v["LEXSOURCES"].indexOf(lexsrc) != -1) {
                    QString trg = dir + fi.completeBaseName() + Option::lex_mod + Option::cpp_ext.first();
                    impldeps.append(trg);
                    impldeps += findDependencies(lexsrc);
                    depends[lexsrc].clear();
                }
            }
            yaccdeps.clear();
        }
        v["OBJECTS"] += (v["YACCOBJECTS"] = createObjectList(impls));
    }

    //Translation files
    if(!project->isEmpty("TRANSLATIONS")) {
        QStringList &trf = project->variables()["TRANSLATIONS"];
        for(QStringList::Iterator it = trf.begin(); it != trf.end(); ++it) {
            (*it) = Option::fixPathToLocalOS((*it));
        }
    }

    if(Option::output_dir != QDir::currentPath())
        project->variables()["INCLUDEPATH"].append(fileFixify(Option::output_dir, Option::output_dir,
                                                              Option::output_dir));

    //moc files
    if(mocAware()) {
        if(!project->isEmpty("MOC_DIR"))
            project->variables()["INCLUDEPATH"].append(project->first("MOC_DIR").replace(QRegExp("\\\\$"),""));
        QStringList &srcmoc = v["SRCMOC"], &hdrmoc = v["HDRMOC"];
        const QStringList &mocables = v["MOCABLES"];
        for(int i = 0; i < mocables.count(); i++) {
            QString moc = Option::fixPathToTargetOS(createMocFileName(mocables[i]), false);
            if(QMakeSourceFileInfo::included(moc))
                srcmoc += moc;
            else if(moc.endsWith(Option::cpp_moc_ext))
                warn_msg(WarnLogic, "File %s[%s] considered mocable but will not be linked into TARGET!",
                         mocables[i].toLatin1().constData(), moc.toLatin1().constData());
            else
                hdrmoc += moc;
        }
        if(Option::h_moc_ext == Option::cpp_ext.first())
            v["OBJMOC"] = createObjectList(hdrmoc);
        srcmoc = hdrmoc + srcmoc;
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
                        QStringList files = QDir(dir).entryList(QStringList(regex));
                        if(files.isEmpty()) {
                            warn_msg(WarnLogic, "Dependency for [%s]: Not found %s", (*file_it).toLatin1().constData(),
                                     (*dep_it).toLatin1().constData());
                        } else {
                            for(int i = 0; i < files.count(); i++)
                                out_deps.append(dir + files[i]);
                        }
                    }
                }
            }
        }
    }
    if (project->isEmpty("QMAKE_QMAKE")) {
        if (!Option::qmake_abslocation.isNull())
            project->variables()["QMAKE_QMAKE"].append(Option::qmake_abslocation);
        else
            project->variables()["QMAKE_QMAKE"].append("qmake");
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
//    meta_file = fileFixify(meta_file);
    QString real_meta_file = Option::fixPathToLocalOS(meta_file);
    if(project->variables()["QMAKE_PRL_INTERNAL_FILES"].indexOf(QMakeMetaInfo::findLib(meta_file)) != -1) {
        ret = true;
    } else if(!meta_file.isEmpty()) {
        QString f = fileFixify(real_meta_file, QDir::currentPath(), Option::output_dir);
        if(QMakeMetaInfo::libExists(f)) {
            QMakeMetaInfo libinfo;
            debug_msg(1, "Processing PRL file: %s", real_meta_file.toLatin1().constData());
            if(!libinfo.readLib(f)) {
                fprintf(stderr, "Error processing meta file: %s\n", real_meta_file.toLatin1().constData());
            } else if(project->isActiveConfig("no_read_prl_" + libinfo.type().toLower())) {
                debug_msg(2, "Ignored meta file %s [%s]", real_meta_file.toLatin1().constData(), libinfo.type().toLatin1().constData());
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
        warn_msg(WarnLogic, "Found prl [%s] file with no target [%s]!", meta_file.toLatin1().constData(),
                 orig_file.toLatin1().constData());
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
        bdir = QDir::currentPath();
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
    if(project->isActiveConfig("build_all")) {
        t << "first: all" << endl;
        QList<SubTarget*>::Iterator it;

        //install
        t << "install: ";
        for(it = targets.begin(); it != targets.end(); ++it)
            t << (*it)->target << "-install ";
        t << endl;

        //uninstall
        t << "uninstall: ";
        for(it = targets.begin(); it != targets.end(); ++it)
            t << (*it)->target << "-uninstall ";
        t << endl;
    } else {
        t << "first: " << targets.first()->target << endl
          << "install: " << targets.first()->target << "-install" << endl
          << "uninstall: " << targets.first()->target << "-uinstall" << endl;
    }
    writeSubTargets(t, targets, SubTargetsNoFlags);
    return true;
}

bool
MakefileGenerator::write()
{
    if(!project)
        return false;
    writePrlFile();
    if(Option::qmake_mode == Option::QMAKE_GENERATE_MAKEFILE || //write makefile
       Option::qmake_mode == Option::QMAKE_GENERATE_PROJECT) {
        QTextStream t(&Option::output);
        writeMakefile(t);
    }
    return true;
}

void
MakefileGenerator::writePrlFile()
{
    if((Option::qmake_mode == Option::QMAKE_GENERATE_MAKEFILE ||
	Option::qmake_mode == Option::QMAKE_GENERATE_PRL)
       && project->variables()["QMAKE_FAILED_REQUIREMENTS"].isEmpty()
       && project->isActiveConfig("create_prl")
       && (project->first("TEMPLATE") == "lib"
	   || project->first("TEMPLATE") == "vclib")
       && !project->isActiveConfig("plugin")) { //write prl file

	QString prl = project->first("TARGET");
	int slsh = prl.lastIndexOf(Option::dir_sep);
	if(slsh != -1)
	    prl = prl.right(prl.length() - slsh);
	int dot = prl.indexOf('.');
	if(dot != -1)
	    prl = prl.left(dot);
	prl += Option::prl_ext;
	if(!project->isEmpty("DESTDIR"))
	    prl.prepend(var("DESTDIR"));
	QString local_prl = Option::fixPathToLocalOS(fileFixify(prl, QDir::currentPath(), Option::output_dir));
	QFile ft(local_prl);
	if(ft.open(QIODevice::WriteOnly)) {
	    project->variables()["ALL_DEPS"].append(prl);
	    project->variables()["QMAKE_INTERNAL_PRL_FILE"].append(prl);
	    QTextStream t(&ft);
	    writePrlFile(t);
	    ft.close();
	}
    }
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
    project->variables()["DESTDIR"] = QStringList(filePath + (filePath.isEmpty() ? pltDir : slashPltDir));

    filePath = project->first("DLLDESTDIR");
    project->variables()["DLLDESTDIR"] = QStringList(filePath + (filePath.isEmpty() ? pltDir : slashPltDir));

    filePath = project->first("OBJECTS_DIR");
    project->variables()["OBJECTS_DIR"] = QStringList(filePath + (filePath.isEmpty() ? pltDir : slashPltDir));

    filePath = project->first("QMAKE_LIBDIR_QT");
    project->variables()["QMAKE_LIBDIR_QT"] = QStringList(filePath + (filePath.isEmpty() ? pltDir : slashPltDir));

    filePath = project->first("QMAKE_LIBS_QT");
    int fpi = filePath.lastIndexOf(sep);
    if(fpi == -1)
        project->variables()["QMAKE_LIBS_QT"].prepend(pltDir + sep);
    else
        project->variables()["QMAKE_LIBS_QT"] = QStringList(filePath.left(fpi) + slashPltDir + filePath.mid(fpi));

    filePath = project->first("QMAKE_LIBS_QT_THREAD");
    fpi = filePath.lastIndexOf(sep);
    if(fpi == -1)
        project->variables()["QMAKE_LIBS_QT_THREAD"].prepend(pltDir + sep);
    else
        project->variables()["QMAKE_LIBS_QT_THREAD"] = QStringList(filePath.left(fpi) + slashPltDir + filePath.mid(fpi));

    filePath = project->first("QMAKE_LIBS_QT_ENTRY");
    fpi = filePath.lastIndexOf(sep);
    if(fpi == -1)
        project->variables()["QMAKE_LIBS_QT_ENTRY"].prepend(pltDir + sep);
    else
        project->variables()["QMAKE_LIBS_QT_ENTRY"] = QStringList(filePath.left(fpi) + slashPltDir + filePath.mid(fpi));
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
        if(fi.path() != ".")
            dir = fi.path() + Option::dir_sep;
        dir = fileFixify(dir, QDir::currentPath(), Option::output_dir);
        if(!dir.isEmpty() && dir.right(Option::dir_sep.length()) != Option::dir_sep)
            dir += Option::dir_sep;

        QString impl = dir + fi.completeBaseName() + Option::yacc_mod + Option::cpp_ext.first();
        QString decl = dir + fi.completeBaseName() + Option::yacc_mod + Option::h_ext.first();

        QString yaccflags = "$(YACCFLAGS)", mangle = "y";
        if(!project->isActiveConfig("yacc_no_name_mangle")) {
            mangle = fi.completeBaseName();
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
        if(fi.path() != ".")
            dir = fi.path() + Option::dir_sep;
        dir = fileFixify(dir, QDir::currentPath(), Option::output_dir);
        if(!dir.isEmpty() && dir.right(Option::dir_sep.length()) != Option::dir_sep)
            dir += Option::dir_sep;
        QString impl = dir + fi.completeBaseName() + Option::lex_mod + Option::cpp_ext.first();

        QString lexflags = "$(LEXFLAGS)", stub="yy";
        if(!project->isActiveConfig("yacc_no_name_mangle")) {
            stub = fi.completeBaseName();
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
            warn_msg(WarnLogic, "%s is not defined: install target not created\n", pvar.toLatin1().constData());
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
                QString dirstr = QDir::currentPath(), filestr = wild;
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
                QStringList files = QDir(local_dirstr).entryList(QStringList(filestr));
                for(int x = 0; x < files.count(); x++) {
                    QString file = files[x];
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
            if(project->isActiveConfig("build_all"))
                t << "install_" << (*it) << ": all ";
            else
                t << "install_" << (*it) << ": first ";
            const QStringList &deps = project->variables()[(*it) + ".depends"];
            if(!deps.isEmpty()) {
                for(QStringList::ConstIterator dep_it = deps.begin(); dep_it != deps.end(); ++dep_it) {
                    QString targ = var((*dep_it) + ".target");
                    if(targ.isEmpty())
                        targ = (*dep_it);
                    t << targ;
                }
            }
            if(project->isEmpty("QMAKE_NOFORCE"))
                t <<  " FORCE";
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
                t << "uninstall_" << (*it) << ": ";
                if(project->isEmpty("QMAKE_NOFORCE"))
                    t <<  " FORCE";
                t << "\n\t"
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
            debug_msg(1, "no definition for install %s: install target not created",(*it).toLatin1().constData());
        }
    }
    t << "install: " << all_installs << " " << var("INSTALLDEPS");
    if(project->isEmpty("QMAKE_NOFORCE"))
        t <<  " FORCE";
    t << "\n\n";
    t << "uninstall: " << all_uninstalls << " " << var("UNINSTALLDEPS");
    if(project->isEmpty("QMAKE_NOFORCE"))
        t <<  " FORCE";
    t << "\n\n";
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
MakefileGenerator::createObjectList(const QStringList &sources)
{
    QStringList ret;
    QString objdir, dir;
    if(!project->variables()["OBJECTS_DIR"].isEmpty())
        objdir = project->first("OBJECTS_DIR");
    for(QStringList::ConstIterator it = sources.begin(); it != sources.end(); ++it) {
        QFileInfo fi(Option::fixPathToLocalOS((*it)));
        if(objdir.isEmpty() && project->isActiveConfig("object_with_source")) {
            QString fName = Option::fixPathToTargetOS((*it), false);
            int dl = fName.lastIndexOf(Option::dir_sep);
            if(dl != -1)
                dir = fName.left(dl + 1);
        } else {
            dir = objdir;
        }
        ret.append(dir + fi.completeBaseName() + Option::obj_ext);
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
    return ret;
}

QString
MakefileGenerator::replaceExtraCompilerVariables(const QString &var, const QString &in, const QString &out)
{
    QString ret = var;
    if(!in.isNull()) {
        QFileInfo fi(Option::fixPathToLocalOS(in));
        ret.replace("${QMAKE_FILE_BASE}", fi.completeBaseName());
        ret.replace("${QMAKE_FILE_IN_BASE}", fi.completeBaseName());
        ret.replace("${QMAKE_FILE_NAME}", fi.filePath());
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
        if(project->isEmpty("QMAKE_NOFORCE") &&
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
        QString tmp_out = fileFixify(project->variables()[(*it) + ".output"].first(),
                                     Option::output_dir, Option::output_dir);
        QString tmp_cmd = project->variables()[(*it) + ".commands"].join(" ");
        QStringList tmp_dep = project->variables()[(*it) + ".depends"];
        QString tmp_dep_cmd = project->variables()[(*it) + ".depend_command"].join(" ");
        if(!tmp_dep_cmd.isEmpty()) {
            QString argv0 = Option::fixPathToLocalOS(tmp_dep_cmd.split(' ').first());
            if(!QFile::exists(argv0))
                tmp_dep_cmd = QString();
        }
        QStringList &vars = project->variables()[(*it) + ".variables"];
        if(tmp_out.isEmpty() || tmp_cmd.isEmpty())
            continue;
        const QStringList &tmp_inputs = project->variables()[(*it) + ".input"];
	{
	    int input_count = 0;
            for(QStringList::ConstIterator it2 = tmp_inputs.begin();
		it2 != tmp_inputs.end(); ++it2) {
                const QStringList &tmp = project->variables()[(*it2)];
                for(QStringList::ConstIterator input = tmp.begin();
		    input != tmp.end(); ++input)
		    input_count++;
	    }
	    if(!input_count)
		continue;
	}

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
	    tmp_clean = Option::fixPathToTargetOS(tmp_clean);
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
                         (*it).toLatin1().constData());
                continue;
            }
            QString inputs, deps;
            if(!tmp_dep.isEmpty()) {
                QStringList dep_fixed = fileFixify(tmp_dep, Option::output_dir, Option::output_dir);
                deps = " " + dep_fixed.join(" ");
            }
            for(QStringList::ConstIterator it2 = tmp_inputs.begin(); it2 != tmp_inputs.end(); ++it2) {
                const QStringList &tmp = project->variables()[(*it2)];
                for(QStringList::ConstIterator input = tmp.begin(); input != tmp.end(); ++input) {
                    deps += " " + findDependencies((*input)).join(" ");
                    inputs += " " + Option::fixPathToTargetOS((*input), false);
                    if(!tmp_dep_cmd.isEmpty() && doDepends()) {
                        char buff[256];
                        QString dep_cmd = replaceExtraCompilerVariables(tmp_dep_cmd, (*input),
                                                                        tmp_out);
                        dep_cmd = Option::fixPathToLocalOS(dep_cmd);
                        if(FILE *proc = QT_POPEN(dep_cmd.toLatin1().constData(), "r")) {
                            QString indeps;
                            while(!feof(proc)) {
                                int read_in = (int)fread(buff, 1, 255, proc);
                                if(!read_in)
                                    break;
                                indeps += QByteArray(buff, read_in);
                            }
                            fclose(proc);
                            if(!indeps.isEmpty())
                                deps += " " + fileFixify(indeps.replace('\n', ' ').simplified().split(' ')).join(" ");
                        }
                    }
                }
            }
            if (inputs.isEmpty())
                continue;
            QString cmd = replaceExtraCompilerVariables(tmp_cmd, QString::null, tmp_out);
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
                if(!tmp_dep.isEmpty()) {
                    QStringList dep_fixed = fileFixify(tmp_dep, Option::output_dir, Option::output_dir);
                    deps = " " + dep_fixed.join(" ");
                }
                QString out = replaceExtraCompilerVariables(tmp_out, (*input), QString::null);
                QString cmd = replaceExtraCompilerVariables(tmp_cmd, (*input), out);
                for(QStringList::Iterator it3 = vars.begin(); it3 != vars.end(); ++it3)
                    cmd.replace("$(" + (*it3) + ")", "$(QMAKE_COMP_" + (*it3)+")");
                if(!tmp_dep_cmd.isEmpty() && doDepends()) {
                    char buff[256];
                    QString dep_cmd = replaceExtraCompilerVariables(tmp_dep_cmd, (*input), out);
                    dep_cmd = Option::fixPathToLocalOS(dep_cmd);
                    if(FILE *proc = QT_POPEN(dep_cmd.toLatin1().constData(), "r")) {
	                QString indeps;
                        while(!feof(proc)) {
                            int read_in = (int)fread(buff, 1, 255, proc);
                            if(!read_in)
                                break;
                            indeps += QByteArray(buff, read_in);
                        }
                        fclose(proc);
                        if(!indeps.isEmpty())
                            deps += " " + fileFixify(indeps.replace('\n', ' ').simplified().split(' ')).join(" ");
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
    const QStringList &quc = project->variables()["QMAKE_EXTRA_COMPILERS"];
    for(QStringList::ConstIterator it = quc.begin(); it != quc.end(); ++it) {
        const QStringList &vars = project->variables()[(*it) + ".variables"];
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
            QRegExp rx((*exp_it), Qt::CaseInsensitive, QRegExp::Wildcard);
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

    if(project->isEmpty("QMAKE_NOFORCE"))
        t << "FORCE:" << endl << endl;
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

    //configs
    for(QStringList::Iterator it = Option::user_configs.begin();
        it != Option::user_configs.end(); ++it)
        ret += " -config " + (*it);
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
    QString ofile = Option::fixPathToTargetOS(fileFixify(Option::output.fileName()));
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
    if(!project->isActiveConfig("build_pass"))
        t << "# Command: " << build_args().replace("$(QMAKE)",
                      (project->isEmpty("QMAKE_QMAKE") ? QString("qmake") : var("QMAKE_QMAKE"))) << endl;
    t << "#############################################################################" << endl;
    t << endl;
}

void
MakefileGenerator::writeSubDirs(QTextStream &t)
{
    QList<SubTarget*> targets;
    {
        QStringList subdirs = project->variables()["SUBDIRS"];
        for(int subdir = 0; subdir < subdirs.size(); ++subdir) {
            // Required since dir_sep is '\\'.
            QString file = Option::fixPathToLocalOS(subdirs[subdir]);
            SubTarget *st = new SubTarget;
            targets.append(st);
            st->makefile = "$(MAKEFILE)";
            if(file.endsWith(Option::pro_ext)) {
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
            st->target = "sub-" + file;
            st->target.replace('/', '-');
            st->target.replace('.', '_');
        }
    }
    if(project->isActiveConfig("build_all"))
        t << "first: all" << endl;
    else
        t << "first: make_first" << endl;
    int flags = SubTargetInstalls;
    if(project->isActiveConfig("ordered"))
        flags |= SubTargetOrdered;
    writeSubTargets(t, targets, flags);
}

void
MakefileGenerator::writeSubTargets(QTextStream &t, QList<MakefileGenerator::SubTarget*> targets, int flags)
{
    // blasted includes
    QStringList &qeui = project->variables()["QMAKE_EXTRA_INCLUDES"];
    for(QStringList::Iterator qeui_it = qeui.begin(); qeui_it != qeui.end(); ++qeui_it)
        t << "include " << (*qeui_it) << endl;

    QString ofile = Option::fixPathToTargetOS(Option::output.fileName());
    if(ofile.lastIndexOf(Option::dir_sep) != -1)
        ofile = ofile.right(ofile.length() - ofile.lastIndexOf(Option::dir_sep) -1);
    t << "MAKEFILE      = " << ofile << endl;
    t << "QMAKE         = " << var("QMAKE_QMAKE") << endl;
    t << "DEL_FILE      = " << var("QMAKE_DEL_FILE") << endl;
    t << "CHK_DIR_EXISTS= " << var("QMAKE_CHK_DIR_EXISTS") << endl;
    t << "MKDIR         = " << var("QMAKE_MKDIR") << endl;
    t << "COPY          = " << var("QMAKE_COPY") << endl;
    t << "COPY_FILE     = " << var("QMAKE_COPY_FILE") << endl;
    t << "COPY_DIR      = " << var("QMAKE_COPY_DIR") << endl;
    t << "INSTALL_FILE  = " << var("QMAKE_INSTALL_FILE") << endl;
    t << "INSTALL_DIR   = " << var("QMAKE_INSTALL_DIR") << endl;
    t << "DEL_FILE      = " << var("QMAKE_DEL_FILE") << endl;
    t << "SYMLINK       = " << var("QMAKE_SYMBOLIC_LINK") << endl;
    t << "DEL_DIR       = " << var("QMAKE_DEL_DIR") << endl;
    t << "MOVE          = " << var("QMAKE_MOVE") << endl;
    t << "CHK_DIR_EXISTS= " << var("QMAKE_CHK_DIR_EXISTS") << endl;
    t << "MKDIR         = " << var("QMAKE_MKDIR") << endl;
    writeExtraVariables(t);
    t << "SUBTARGETS    = ";     // subtargets are sub-directory
    for(int target = 0; target < targets.size(); ++target)
        t << " \\\n\t\t" << targets.at(target)->target;
    t << endl << endl;

    QStringList targetSuffixes;
    targetSuffixes << "make_first" << "all" << "clean" << "distclean" << "mocables"
                   << QString((flags & SubTargetInstalls) ? "install_subtargets" : "install")
                   << QString((flags & SubTargetInstalls) ? "uninstall_subtargets" : "uninstall")
                   << "mocclean";

    // generate target rules
    for(int target = 0; target < targets.size(); ++target) {
        SubTarget *subtarget = targets.at(target);
        bool have_dir = !subtarget->directory.isEmpty();
        QString mkfile = subtarget->makefile, cdin, cdout;
        if(have_dir) {
            mkfile.prepend(targets.at(target)->directory + Option::dir_sep);
            if(project->isActiveConfig("cd_change_global")) {
                cdin = "\n\tcd " + subtarget->directory + "\n\t";

		QDir pwd(Option::output_dir);
		QStringList in = subtarget->directory.split(Option::dir_sep), out;
		for(int i = 0; i < in.size(); i++) {
		    if(in.at(i) == "..")
			out.prepend(QFileInfo(pwd.path()).fileName());
		    else if(in.at(i) != ".")
			out.prepend("..");
		    pwd.cd(in.at(i));
		}
                cdout = "\n\t@cd " + out.join(Option::dir_sep);
            } else {
                cdin = "\n\tcd " + subtarget->directory + " && ";
            }
        } else {
            cdin = "\n\t";
        }

        //qmake it
        if(!subtarget->profile.isEmpty()) {
            QString out, in = fileFixify(subtarget->directory + Option::dir_sep + subtarget->profile);
            if(subtarget->makefile != "$(MAKEFILE)")
                out = " -o " + subtarget->makefile;
            if(in.startsWith(subtarget->directory + Option::dir_sep))
                in = in.mid(subtarget->directory.length() + 1);
            t << mkfile << ": " << "\n\t";
            if(have_dir) {
                t << mkdir_p_asstring(subtarget->directory)
                  << cdin
                  << "$(QMAKE) " << in << buildArgs() << out
                  << cdout << endl;
            } else {
                t << "$(QMAKE) " << in << buildArgs() << out << endl;
            }
            t << subtarget->target << "-qmake_all: ";
            if(project->isEmpty("QMAKE_NOFORCE"))
                t <<  " FORCE";
            t << "\n\t";
            if(have_dir) {
                t << mkdir_p_asstring(subtarget->directory)
                  << cdin
                  << "$(QMAKE) " << in << buildArgs() << out
                  << cdout << endl;
            } else {
                t << "$(QMAKE) " << in << buildArgs() << out << endl;
            }
        }

        //don't need the makefile arg if it isn't changed
        QString makefilein;
        if(subtarget->makefile != "$(MAKEFILE)")
            makefilein = " -f " + subtarget->makefile;

        //actually compile
        t << subtarget->target << ": " << mkfile;
        if(project->isEmpty("QMAKE_NOFORCE"))
            t <<  " FORCE";
        t << "\n\t";
        if(have_dir)
            t << "cd " << subtarget->directory << " && ";
        t << "$(MAKE)" << makefilein << endl;
        for(int suffix = 0; suffix < targetSuffixes.size(); ++suffix) {
            QString s = targetSuffixes.at(suffix);
            if(s == "install_subtargets")
                s = "install";
            else if(s == "uninstall_subtargets")
                s = "uninstall";
            else if(s == "make_first")
                s = "first";
            if(flags & SubTargetOrdered) {
                t << subtarget->target << "-" << targetSuffixes.at(suffix) << "-ordered: " << mkfile;
                if(target)
                    t << " " << targets.at(target-1)->target << "-" << targetSuffixes.at(suffix) << "-ordered ";
                if(project->isEmpty("QMAKE_NOFORCE"))
                    t <<  " FORCE";
                t << cdin
                  << "$(MAKE)" << makefilein << " " << s
                  << cdout << endl;
            }
            t << subtarget->target << "-" << targetSuffixes.at(suffix) << ": " << mkfile;
            if(project->isEmpty("QMAKE_NOFORCE"))
                t <<  " FORCE";
            t << cdin
              << "$(MAKE)" << makefilein << " " << s
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
    }
    if(project->isEmpty("QMAKE_NOFORCE"))
        t <<  " FORCE";
    if(project->isActiveConfig("no_empty_targets"))
        t << "\t" << "@cd .";
    t << endl << endl;

    for(int s = 0; s < targetSuffixes.size(); ++s) {
        QString suffix = targetSuffixes.at(s);
        if(!(flags & SubTargetInstalls) && suffix.endsWith("install"))
            continue;

        t << suffix << ":";
        for(int target = 0; target < targets.size(); ++target) {
            QString targetRule = targets.at(target)->target + "-" + suffix;
            if(flags & SubTargetOrdered)
                targetRule += "-ordered";
            t << " " << targetRule;
        }
        if(suffix == "all")
            t << varGlue("ALL_DEPS"," "," ","");
        if(project->isEmpty("QMAKE_NOFORCE"))
            t <<  " FORCE";
        t << endl;
        if(suffix == "clean") {
            t << varGlue("QMAKE_CLEAN","\t-$(DEL_FILE) ","\n\t-$(DEL_FILE) ", "\n");
        } else if(suffix == "distclean") {
            QString ofile = Option::fixPathToTargetOS(fileFixify(Option::output.fileName()));
            if(!ofile.isEmpty())
                t << "\t-$(DEL_FILE) " << ofile << endl;
        } else if(project->isActiveConfig("no_empty_targets")) {
            t << "\t" << "@cd ." << endl;
        }
    }

    // user defined targets
    QStringList &qut = project->variables()["QMAKE_EXTRA_TARGETS"];
    for(QStringList::Iterator qut_it = qut.begin(); qut_it != qut.end(); ++qut_it) {
        QString targ = var((*qut_it) + ".target"),
                 cmd = var((*qut_it) + ".commands"), deps;
        if(targ.isEmpty())
            targ = (*qut_it);
        t << endl;

        QStringList &deplist = project->variables()[(*qut_it) + ".depends"];
        for(QStringList::Iterator dep_it = deplist.begin(); dep_it != deplist.end(); ++dep_it) {
            QString dep = var((*dep_it) + ".target");
            if(dep.isEmpty())
                dep = (*dep_it);
            deps += " " + dep;
        }
        if(project->variables()[(*qut_it) + ".CONFIG"].indexOf("recursive") != -1) {
            for(int target = 0; target < targets.size(); ++target) {
                SubTarget *subtarget = targets.at(target);
                bool have_dir = !subtarget->directory.isEmpty();
                QString mkfile = subtarget->makefile, cdin, cdout;
                if(have_dir) {
                    mkfile.prepend(targets.at(target)->directory + Option::dir_sep);
                    if(project->isActiveConfig("cd_change_global")) {
                        cdin = "\n\tcd " + subtarget->directory + "\n\t";

                        QDir pwd(Option::output_dir);
                        QStringList in = subtarget->directory.split(Option::dir_sep), out;
                        for(int i = 0; i < in.size(); i++) {
                            if(in.at(i) == "..")
                                out.prepend(QFileInfo(pwd.path()).fileName());
                            else if(in.at(i) != ".")
                                out.prepend("..");
                            pwd.cd(in.at(i));
                        }
                        cdout = "\n\t@cd " + out.join(Option::dir_sep);
                    } else {
                        cdin = "\n\tcd " + subtarget->directory + " && ";
                    }
                } else {
                    cdin = "\n\t";
                }

                //don't need the makefile arg if it isn't changed
                QString makefilein;
                if(subtarget->makefile != "$(MAKEFILE)")
                    makefilein = " -f " + subtarget->makefile;

                //write the rule/depends
                if(flags & SubTargetOrdered) {
                    const QString dep = subtarget->target + "-" + (*qut_it) + "-ordered";
                    t << dep << ": " << mkfile;
                    if(target)
                        t << " " << targets.at(target-1)->target << "-" << (*qut_it) << "-ordered ";
                    deps += " " + dep;
                } else {
                    const QString dep = subtarget->target + "-" + (*qut_it);
                    t << dep << ": " << mkfile;
                    deps += " " + dep;
                }

                //write the commands
                if(have_dir) {
                    t << cdin
                      << "$(MAKE)" << makefilein << " " << (*qut_it)
                      << cdout << endl;
                } else {
                    t << "$(MAKE)" << makefilein << " " << (*qut_it) << endl;
                }
            }
        }
        if(project->isEmpty("QMAKE_NOFORCE") &&
           project->variables()[(*qut_it) + ".CONFIG"].indexOf("phony") != -1)
            deps += " FORCE";
        t << targ << ":" << deps << "\n\t"
          << cmd << endl;
    }

    if(flags & SubTargetInstalls) {
        project->variables()["INSTALLDEPS"]   += "install_subtargets";
        project->variables()["UNINSTALLDEPS"] += "uninstall_subtargets";
        writeInstalls(t, "INSTALLS");
    }

    if(project->isEmpty("QMAKE_NOFORCE"))
        t << "FORCE:" << endl << endl;
}

void
MakefileGenerator::writeMakeQmake(QTextStream &t)
{
    QString ofile = Option::fixPathToTargetOS(fileFixify(Option::output.fileName()));
    if(project->isEmpty("QMAKE_FAILED_REQUIREMENTS") && !project->isEmpty("QMAKE_INTERNAL_PRL_FILE")) {
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
            for(int include = 0; include < included.size(); ++include)
                t << included.at(include) << ":" << endl;
        }
        if(project->first("QMAKE_ORIG_TARGET") != "qmake") {
            t << "qmake: " <<
                project->variables()["QMAKE_INTERNAL_QMAKE_DEPS"].join(" \\\n\t\t");
            if(project->isEmpty("QMAKE_NOFORCE"))
                t <<  " FORCE";
            t << "\n\t" << "@" << qmake << endl << endl;
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

struct FileFixifyCacheKey
{
    QString in_d, out_d;
    QString file, pwd;
    MakefileGenerator::FileFixifyType fixType;
    bool canonicalize;
    FileFixifyCacheKey(const QString &f, const QString &od, const QString &id,
                   MakefileGenerator::FileFixifyType ft, bool c)
    {
        pwd = QDir::currentPath();
        file = f;
        if(od.isNull())
            out_d = Option::output_dir;
        else
            out_d = od;
        if(id.isNull())
            in_d = QDir::currentPath();
        else
            in_d = id;
        fixType = ft;
        canonicalize = c;
    }
    QString toString() const {
        return file + "--" + in_d + "--" + out_d + "--" + pwd + "--" + 
            QString::number(fixType) + "--" + QString::number(canonicalize);
    }
    bool operator==(const FileFixifyCacheKey &f) const
    {
        return (f.canonicalize == canonicalize &&
                f.fixType == fixType &&
                f.file == file &&
                f.in_d == in_d &&
                f.out_d == out_d &&
                f.pwd == pwd);
    }
};

uint qHash(const FileFixifyCacheKey &f) { 
    return (uint(f.canonicalize) |
            uint(f.fixType) |
            qHash(f.file) |
            qHash(f.pwd) |
            qHash(f.in_d) |
            qHash(f.out_d));
}

QString
MakefileGenerator::fileFixify(const QString& file, const QString &out_d, const QString &in_d,
                              FileFixifyType fix, bool canon) const
{
    if(file.isEmpty())
        return file;
    QString ret = file;

    //remove the quotes
    QChar quote;
    if((ret[0] == QLatin1Char('\'') || ret[0] == QLatin1Char('\"')) && 
       ret[ret.length()-1] == ret[0]) {
        quote = ret[0];
        ret = ret.mid(1, ret.length() - 2);
    }

    //setup the cache
    static QHash<FileFixifyCacheKey, QString> *cache = 0;
    if(!cache) 
        cache = new QHash<FileFixifyCacheKey, QString>;
    FileFixifyCacheKey cacheKey(ret, out_d, in_d, fix, canon);
    if(cache->contains(cacheKey)) 
        return cache->value(cacheKey);

    //do the fixin'
    const QString pwd = QDir::currentPath() + "/";
    QString orig_file = ret;
    if(fix == FileFixifyAbsolute || (fix == FileFixifyDefault && project->isActiveConfig("no_fixpath"))) {
        if(!project->isEmpty("QMAKE_ABSOLUTE_SOURCE_PATH")) { //absoluteify it
            if(QDir::isRelativePath(ret)) //already absolute
                ret.prepend(pwd);
        }
        ret = Option::fixPathToTargetOS(ret, false, canon);
    } else { //fix it..
        QString qfile(Option::fixPathToLocalOS(ret, true, canon)), in_dir(in_d), out_dir(out_d);
        {
            if(out_dir.isNull() || QDir::isRelativePath(out_dir))
                out_dir.prepend(Option::output_dir + "/");
            else if(out_dir == ".")
                out_dir = pwd;
            if(in_dir.isEmpty() || QDir::isRelativePath(in_dir))
                in_dir.prepend(pwd);
            else if(in_dir == ".")
                in_dir = pwd;
            QFileInfo in_fi(in_dir);
            if(in_fi.exists())
                in_dir = in_fi.canonicalFilePath();
            QFileInfo out_fi(out_dir);
            if(out_fi.exists())
                out_dir = out_fi.canonicalFilePath();
        }
        QFileInfo qfileinfo(qfile);
        if(out_dir != in_dir || !qfileinfo.isRelative()) {
            if(qfileinfo.isRelative()) {
                ret = in_dir + "/" + qfile;
                qfileinfo.setFile(ret);
            }
            ret = Option::fixPathToTargetOS(ret, false, canon);
            if(canon && qfileinfo.exists() && 
               file == Option::fixPathToTargetOS(ret, true, canon))
                ret = qfileinfo.canonicalFilePath();
            QString match_dir = Option::fixPathToTargetOS(out_dir, false, canon);
            if(ret == match_dir) {
                ret = "";
            } else if(ret.startsWith(match_dir + Option::dir_sep)) {
                ret = ret.mid(match_dir.length() + Option::dir_sep.length());
            } else {
                //figure out the depth
                int depth = 4;
                if(Option::qmake_mode == Option::QMAKE_GENERATE_MAKEFILE ||
                   Option::qmake_mode == Option::QMAKE_GENERATE_PRL) {
                    if(project && !project->isEmpty("QMAKE_PROJECT_DEPTH"))
                        depth = project->first("QMAKE_PROJECT_DEPTH").toInt();
                    else if(Option::mkfile::cachefile_depth != -1)
                        depth = Option::mkfile::cachefile_depth;
                }
                //calculate how much can be removed
                QString dot_prefix;
                for(int i = 1; i <= depth; i++) {
                    int sl = match_dir.lastIndexOf(Option::dir_sep);
                    if(sl == -1)
                        break;
                    match_dir = match_dir.left(sl);
                    if(match_dir.isEmpty())
                        break;
                    if(ret.startsWith(match_dir + Option::dir_sep)) {
                        //concat
                        int remlen = ret.length() - (match_dir.length() + 1);
                        if(remlen < 0)
                            remlen = 0;
                        ret = ret.right(remlen);
                        //prepend
                        for(int o = 0; o < i; o++)
                            dot_prefix += ".." + Option::dir_sep; 
                    }
                }
                ret.prepend(dot_prefix);
            }
        } else {
            ret = Option::fixPathToTargetOS(ret, false, canon);
        }
    }
    if(ret.isEmpty())
        ret = ".";
    if(!quote.isNull()) //put the quotes back
        ret = quote + ret + quote;
    debug_msg(3, "Fixed %s :: to :: %s [%s::%s] [%s::%s]", orig_file.toLatin1().constData(), 
              ret.toLatin1().constData(), in_d.toLatin1().constData(), out_d.toLatin1().constData(), 
              pwd.toLatin1().constData(), Option::output_dir.toLatin1().constData());
    cache->insert(cacheKey, ret);
    return ret;
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
                     file.toLatin1().constData(), (*val_it).toLatin1().constData(), w.toLatin1().constData());
            break;
        }
    }
}

QMakeLocalFileName
MakefileGenerator::fixPathForFile(const QMakeLocalFileName &file, bool forOpen)
{
    if(forOpen)
        return QMakeLocalFileName(fileFixify(file.real(), QDir::currentPath(), Option::output_dir));
    return QMakeLocalFileName(fileFixify(file.real()));
}

QMakeLocalFileName
MakefileGenerator::findFileForMoc(const QMakeLocalFileName &file)
{
    project->variables()["MOCABLES"].append(file.local());
    QMakeLocalFileName mocfile(createMocFileName(file.local()));
    debug_msg(2, "findFileForMoc: %s considered mocable (%s)", file.local().toLatin1().constData(), mocfile.local().toLatin1().constData());
    return mocfile;
}

QMakeLocalFileName MakefileGenerator::findFileForDep(const QMakeLocalFileName &dep, const QMakeLocalFileName &file)
{
    QMakeLocalFileName ret;
    if(!project->isEmpty("SKIP_DEPENDS")) {
        bool found = false;
        QStringList &nodeplist = project->values("SKIP_DEPENDS");
        for(QStringList::Iterator it = nodeplist.begin();
            it != nodeplist.end(); ++it) {
            QRegExp regx((*it));
            if(regx.indexIn(dep.local()) != -1) {
                found = true;
                break;
            }
        }
        if(found)
            return ret;
    }

    ret = QMakeSourceFileInfo::findFileForDep(dep, file);
    if(!ret.isNull())
        return ret;

    //these are some hacky heuristics it will try to do on an include
    //however these can be turned off at runtime, I'm not sure how
    //reliable these will be, most likely when problems arise turn it off
    //and see if they go away..
    if(Option::mkfile::do_dep_heuristics) {
        if(depHeuristics.contains(dep.real()))
            return depHeuristics[dep.real()];

        if(Option::output_dir != QDir::currentPath()
           && QDir::isRelativePath(dep.real())) { //is it from the shadow tree
            QList<QMakeLocalFileName> depdirs = QMakeSourceFileInfo::dependencyPaths();
            depdirs.prepend(QFileInfo(file.real()).absoluteDir().path());
            QString pwd = QDir::currentPath();
            if(pwd.at(pwd.length()-1) != '/')
                pwd += '/';
            for(int i = 0; i < depdirs.count(); i++) {
                QString dir = depdirs.at(i).real();
                if(!QDir::isRelativePath(dir) && dir.startsWith(pwd))
                    dir = dir.mid(pwd.length());
                if(QDir::isRelativePath(dir)) {
                    if(!dir.endsWith(Option::dir_sep))
                        dir += Option::dir_sep;
                    QString shadow = fileFixify(dir + dep.local(), pwd, Option::output_dir);
                    if(QFile::exists(shadow)) {
                        ret = QMakeLocalFileName(shadow);
                        goto found_dep_from_heuristic;
                    }
                }
            }
        }
        { //is it form an EXTRA_TARGET
            QStringList &qut = project->variables()["QMAKE_EXTRA_TARGETS"];
            for(QStringList::Iterator it = qut.begin(); it != qut.end(); ++it) {
                QString targ = var((*it) + ".target");
                if(targ.isEmpty())
                    targ = (*it);
                if(targ.endsWith(dep.real())) {
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
                        if(out == dep.real() || out.endsWith("/" + dep.real())) {
                            ret = QMakeLocalFileName(fileFixify(out, QDir::currentPath(), Option::output_dir));
                            goto found_dep_from_heuristic;
                        }
                    }
                }
            }
        }
        if(project->isActiveConfig("lex_included")) { //is this the lex file?
            QString rhs = Option::lex_mod + Option::cpp_ext.first();
            if(dep.real().endsWith(rhs)) {
                QString lhs = dep.real().left(dep.real().length() - rhs.length()) + Option::lex_ext;
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
                        QString ret_name = d + dep.real();
                        ret_name = fileFixify(ret_name, QDir::currentPath(), Option::output_dir);
                        ret = QMakeLocalFileName(ret_name);
                        goto found_dep_from_heuristic;
                    }
                }
            }
        }
        { //is it from a .y?
            QString rhs = Option::yacc_mod + Option::h_ext.first();
            if(dep.real().endsWith(rhs)) {
                QString lhs = dep.local().left(dep.local().length() - rhs.length()) + Option::yacc_ext;
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
                        QString ret_name = d + dep.local();
                        ret_name = fileFixify(ret_name, QDir::currentPath(), Option::output_dir);
                        ret = QMakeLocalFileName(ret_name);
                        goto found_dep_from_heuristic;
                    }
                }
            }
        }
        if(mocAware() &&                    //is it a moc file?
           (dep.local().endsWith(Option::cpp_ext.first()) || dep.local().endsWith(Option::cpp_moc_ext))
           || ((Option::cpp_ext.first() != Option::h_moc_ext) && dep.local().endsWith(Option::h_moc_ext))) {
            QStringList &l = project->variables()["MOCABLES"];
            for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
                const QString moc = createMocFileName((*it));
                QString fixed_moc = Option::fixPathToTargetOS(moc);
                if(fixed_moc.section(Option::dir_sep, -(dep.local().count('/')+1)) == dep.local()) {
                    QString ret_name = fileFixify(moc, QDir::currentPath(), Option::output_dir);
                    ret = QMakeLocalFileName(ret_name);
                    goto found_dep_from_heuristic;
                }
            }
        }
    found_dep_from_heuristic:
        depHeuristics.insert(dep.real(), ret);
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
    if(const char *d = qgetenv("QTDIR")) {
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
        if(!file.fileName().isEmpty()) {
            if(QDir::isRelativePath(file.fileName()))
                file.setFileName(Option::output_dir + file.fileName()); //pwd when qmake was run
            QFileInfo fi(file);
            if(fi.isDir())
                outdir = file.fileName() + QDir::separator();
        }
        if(!outdir.isEmpty() || file.fileName().isEmpty()) {
            QString fname = "Makefile";
            if(!project->isEmpty("MAKEFILE"))
               fname = project->first("MAKEFILE");
            file.setFileName(outdir + fname);
        }
    }
    if(QDir::isRelativePath(file.fileName()))
        file.setFileName(Option::output_dir + file.fileName()); //pwd when qmake was run
    if(!build.isEmpty())
        file.setFileName(file.fileName() + "." + build);
    if(project->isEmpty("QMAKE_MAKEFILE"))
        project->variables()["QMAKE_MAKEFILE"].append(file.fileName());
    int slsh = file.fileName().lastIndexOf(Option::dir_sep);
    if(slsh != -1)
        createDir(file.fileName().left(slsh));
    if(file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QFileInfo fi(Option::output);
        QString od = Option::fixPathToTargetOS((fi.isSymLink() ? fi.readLink() : fi.path()));
        if(QDir::isRelativePath(od))
            od.prepend(Option::output_dir);
        Option::output_dir = od;
        return true;
    }
    return false;
}



