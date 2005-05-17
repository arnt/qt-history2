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

#include "unixmake.h"
#include "option.h"
#include <qregexp.h>
#include <qfile.h>
#include <qhash.h>
#include <qdir.h>
#include <time.h>


void
UnixMakefileGenerator::init()
{
    if(init_flag)
        return;
    init_flag = true;

    if(!project->isEmpty("QMAKE_FAILED_REQUIREMENTS")) /* no point */
        return;

    QStringList &configs = project->variables()["CONFIG"];

    //defaults
    if(project->isEmpty("ICON") && !project->isEmpty("RC_FILE"))
        project->variables()["ICON"] = project->variables()["RC_FILE"];
    if(project->isEmpty("QMAKE_EXTENSION_SHLIB")) {
        if(project->isEmpty("QMAKE_CYGWIN_SHLIB")) {
            project->variables()["QMAKE_EXTENSION_SHLIB"].append("so");
        } else {
            project->variables()["QMAKE_EXTENSION_SHLIB"].append("dll");
        }
    }
    if(project->isEmpty("QMAKE_CFLAGS_PRECOMPILE"))
        project->variables()["QMAKE_CFLAGS_PRECOMPILE"].append("-x c-header -c");
    if(project->isEmpty("QMAKE_CXXFLAGS_PRECOMPILE"))
        project->variables()["QMAKE_CXXFLAGS_PRECOMPILE"].append("-x c++-header -c");
    if(project->isEmpty("QMAKE_CFLAGS_USE_PRECOMPILE"))
        project->variables()["QMAKE_CFLAGS_USE_PRECOMPILE"].append("-include");
    if(project->isEmpty("QMAKE_EXTENSION_PLUGIN"))
        project->variables()["QMAKE_EXTENSION_PLUGIN"].append(project->first("QMAKE_EXTENSION_SHLIB"));
    if(project->isEmpty("QMAKE_COPY_FILE"))
        project->variables()["QMAKE_COPY_FILE"].append("$(COPY)");
    if(project->isEmpty("QMAKE_COPY_DIR"))
        project->variables()["QMAKE_COPY_DIR"].append("$(COPY) -R");
    if(project->isEmpty("QMAKE_INSTALL_FILE"))
        project->variables()["QMAKE_INSTALL_FILE"].append("$(COPY_FILE)");
    if(project->isEmpty("QMAKE_INSTALL_DIR"))
        project->variables()["QMAKE_INSTALL_DIR"].append("$(COPY_DIR)");
    if(project->isEmpty("QMAKE_LIBTOOL"))
        project->variables()["QMAKE_LIBTOOL"].append("libtool --silent");
    if(project->isEmpty("QMAKE_SYMBOLIC_LINK"))
        project->variables()["QMAKE_SYMBOLIC_LINK"].append("ln -sf");

    /* this should probably not be here, but I'm using it to wrap the .t files */
    if(project->first("TEMPLATE") == "app")
        project->variables()["QMAKE_APP_FLAG"].append("1");
    else if(project->first("TEMPLATE") == "lib")
        project->variables()["QMAKE_LIB_FLAG"].append("1");
    else if(project->first("TEMPLATE") == "subdirs") {
        MakefileGenerator::init();
        if(project->isEmpty("MAKEFILE"))
            project->variables()["MAKEFILE"].append("Makefile");
        if(project->isEmpty("QMAKE_QMAKE"))
            project->variables()["QMAKE_QMAKE"].append("qmake");
        if(project->variables()["QMAKE_INTERNAL_QMAKE_DEPS"].indexOf("qmake_all") == -1)
            project->variables()["QMAKE_INTERNAL_QMAKE_DEPS"].append("qmake_all");
        return; /* subdirs is done */
    }

    //If the TARGET looks like a path split it into DESTDIR and the resulting TARGET
    if(!project->isEmpty("TARGET")) {
        QString targ = project->first("TARGET");
        int slsh = qMax(targ.lastIndexOf('/'), targ.lastIndexOf(Option::dir_sep));
        if(slsh != -1) {
            if(project->isEmpty("DESTDIR"))
                project->values("DESTDIR").append("");
            else if(project->first("DESTDIR").right(1) != Option::dir_sep)
                project->variables()["DESTDIR"] = QStringList(project->first("DESTDIR") + Option::dir_sep);
            project->variables()["DESTDIR"] = QStringList(project->first("DESTDIR") + targ.left(slsh+1));
            project->variables()["TARGET"] = QStringList(targ.mid(slsh+1));
        }
    }

    project->variables()["QMAKE_ORIG_TARGET"] = project->variables()["TARGET"];
    project->variables()["QMAKE_ORIG_DESTDIR"] = project->variables()["DESTDIR"];
    project->variables()["QMAKE_LIBS"] += project->variables()["LIBS"];
    if((!project->isEmpty("QMAKE_LIB_FLAG") && !project->isActiveConfig("staticlib")) ||
         (project->isActiveConfig("qt") &&  project->isActiveConfig("plugin"))) {
        if(configs.indexOf("dll") == -1) configs.append("dll");
    } else if(!project->isEmpty("QMAKE_APP_FLAG") || project->isActiveConfig("dll")) {
        configs.removeAll("staticlib");
    }
    if(!project->isEmpty("QMAKE_INCREMENTAL"))
        project->variables()["QMAKE_LFLAGS"] += project->variables()["QMAKE_LFLAGS_INCREMENTAL"];
    else if(!project->isEmpty("QMAKE_LFLAGS_PREBIND") &&
            !project->variables()["QMAKE_LIB_FLAG"].isEmpty() &&
            project->isActiveConfig("dll"))
        project->variables()["QMAKE_LFLAGS"] += project->variables()["QMAKE_LFLAGS_PREBIND"];
    if(!project->isEmpty("QMAKE_INCDIR"))
        project->variables()["INCLUDEPATH"] += project->variables()["QMAKE_INCDIR"];
    if(!project->isEmpty("QMAKE_LIBDIR")) {
        const QStringList &libdirs = project->values("QMAKE_LIBDIR");
        for(QStringList::ConstIterator it = libdirs.begin(); it != libdirs.end(); ++it) {
            if(!project->isEmpty("QMAKE_LFLAGS_RPATH") && project->isActiveConfig("rpath_libdirs"))
                project->variables()["QMAKE_LFLAGS"] += var("QMAKE_LFLAGS_RPATH") + (*it);
            project->variables()["QMAKE_LIBDIR_FLAGS"] += "-L" + (*it);
        }
    }
    if(!project->isEmpty("QMAKE_RPATHDIR")) {
        const QStringList &rpathdirs = project->values("QMAKE_RPATHDIR");
        for(QStringList::ConstIterator it = rpathdirs.begin(); it != rpathdirs.end(); ++it) {
            if(!project->isEmpty("QMAKE_LFLAGS_RPATH"))
                project->variables()["QMAKE_LFLAGS"] += var("QMAKE_LFLAGS_RPATH") + QFileInfo((*it)).absoluteFilePath();
        }
    }
    QString compile_flag = var("QMAKE_COMPILE_FLAG");
    if(compile_flag.isEmpty())
        compile_flag = "-c";
    if(doPrecompiledHeaders() && !project->isEmpty("PRECOMPILED_HEADER")) {
        QString prefix_flags = project->first("QMAKE_CFLAGS_PREFIX_INCLUDE");
        if(prefix_flags.isEmpty())
            prefix_flags = "-include";
        compile_flag += " " + prefix_flags + " " + project->first("QMAKE_ORIG_TARGET");
    }
    if(project->isEmpty("QMAKE_RUN_CC"))
        project->variables()["QMAKE_RUN_CC"].append("$(CC) " + compile_flag + " $(CFLAGS) $(INCPATH) -o $obj $src");
    if(project->isEmpty("QMAKE_RUN_CC_IMP"))
        project->variables()["QMAKE_RUN_CC_IMP"].append("$(CC) " + compile_flag + " $(CFLAGS) $(INCPATH) -o $@ $<");
    if(project->isEmpty("QMAKE_RUN_CXX"))
        project->variables()["QMAKE_RUN_CXX"].append("$(CXX) " + compile_flag + " $(CXXFLAGS) $(INCPATH) -o $obj $src");
    if(project->isEmpty("QMAKE_RUN_CXX_IMP"))
        project->variables()["QMAKE_RUN_CXX_IMP"].append("$(CXX) " + compile_flag + " $(CXXFLAGS) $(INCPATH) -o $@ $<");

    project->variables()["QMAKE_FILETAGS"] << "SOURCES" << "GENERATED_SOURCES" << "TARGET" << "DESTDIR";
    if(!project->isEmpty("QMAKE_EXTRA_COMPILERS")) {
        const QStringList &quc = project->variables()["QMAKE_EXTRA_COMPILERS"];
        for(QStringList::ConstIterator it = quc.begin(); it != quc.end(); ++it)
            project->variables()["QMAKE_FILETAGS"] += project->variables()[(*it)+".input"];
    }

    if(project->isActiveConfig("GNUmake") && !project->isEmpty("QMAKE_CFLAGS_DEPS"))
        include_deps = true; //do not generate deps
    if(project->isActiveConfig("compile_libtool"))
        Option::obj_ext = ".lo"; //override the .o

    MakefileGenerator::init();

    if(project->isActiveConfig("macx") &&
       !project->isEmpty("TARGET") && !project->isActiveConfig("compile_libtool") &&
       ((project->first("TEMPLATE") == "app" && project->isActiveConfig("app_bundle")) ||
        (project->first("TEMPLATE") == "lib" && project->isActiveConfig("lib_bundle") &&
         !project->isActiveConfig("staticlib") && !project->isActiveConfig("plugin")))) {
        if(project->first("TEMPLATE") == "app") {
            QString bundle = project->first("TARGET");
            if(!project->isEmpty("QMAKE_APPLICATION_BUNDLE_NAME"))
                bundle = project->first("QMAKE_APPLICATION_BUNDLE_NAME");
            if(!bundle.endsWith(".app"))
                bundle += ".app";
            project->variables()["QMAKE_BUNDLE_NAME"] = QStringList(bundle);
            project->variables()["QMAKE_PKGINFO"].append(project->first("DESTDIR") + bundle + "/PkgInfo");
            project->variables()["ALL_DEPS"] += project->first("QMAKE_PKGINFO");
        } else if(project->first("TEMPLATE") == "lib") {
            QString bundle = project->first("TARGET");
            if(!project->isEmpty("QMAKE_FRAMEWORK_BUNDLE_NAME"))
                bundle = project->first("QMAKE_FRAMEWORK_BUNDLE_NAME");
            if(!bundle.endsWith(".framework"))
                bundle += ".framework";
            project->variables()["QMAKE_BUNDLE_NAME"] = QStringList(bundle);
        }
    } else { //no bundling here
        project->variables()["QMAKE_BUNDLE_NAME"].clear();
    }

    if(!project->isEmpty("QMAKE_INTERNAL_INCLUDED_FILES"))
        project->variables()["DISTFILES"] += project->variables()["QMAKE_INTERNAL_INCLUDED_FILES"];
    project->variables()["DISTFILES"] += project->projectFile();

    init2();
    project->variables()["QMAKE_INTERNAL_PRL_LIBS"] << "QMAKE_LIBDIR_FLAGS" << "QMAKE_LIBS";
    if(!project->isEmpty("QMAKE_MAX_FILES_PER_AR")) {
        bool ok;
        int max_files = project->first("QMAKE_MAX_FILES_PER_AR").toInt(&ok);
        QStringList ar_sublibs, objs = project->variables()["OBJECTS"];
        if(ok && max_files > 5 && max_files < (int)objs.count()) {
            int obj_cnt = 0, lib_cnt = 0;
            QString lib;
            for(QStringList::Iterator objit = objs.begin(); objit != objs.end(); ++objit) {
                if((++obj_cnt) >= max_files) {
                    if(lib_cnt) {
                        lib.sprintf("lib%s-tmp%d.a",
                                    project->first("QMAKE_ORIG_TARGET").toLatin1().constData(), lib_cnt);
                        ar_sublibs << lib;
                        obj_cnt = 0;
                    }
                    lib_cnt++;
                }
            }
        }
        if(!ar_sublibs.isEmpty()) {
            project->variables()["QMAKE_AR_SUBLIBS"] = ar_sublibs;
            project->variables()["QMAKE_INTERNAL_PRL_LIBS"] << "QMAKE_AR_SUBLIBS";
        }
    }

    if(project->isActiveConfig("compile_libtool")) {
        const QString libtoolify[] = { "QMAKE_RUN_CC", "QMAKE_RUN_CC_IMP",
                                       "QMAKE_RUN_CXX", "QMAKE_RUN_CXX_IMP",
                                       "QMAKE_LINK_THREAD", "QMAKE_LINK", "QMAKE_AR_CMD", "QMAKE_LINK_SHLIB_CMD",
                                       QString() };
        for(int i = 0; !libtoolify[i].isNull(); i++) {
            QStringList &l = project->variables()[libtoolify[i]];
            if(!l.isEmpty()) {
                QString libtool_flags, comp_flags;
                if(libtoolify[i].startsWith("QMAKE_LINK") || libtoolify[i] == "QMAKE_AR_CMD") {
                    libtool_flags += " --mode=link";
                    if(project->isActiveConfig("staticlib")) {
                        libtool_flags += " -static";
                    } else {
                        if(!project->isEmpty("QMAKE_LIB_FLAG")) {
                            int maj = project->first("VER_MAJ").toInt();
                            int min = project->first("VER_MIN").toInt();
                            int pat = project->first("VER_PAT").toInt();
                            comp_flags += " -version-info " + QString::number(10*maj + min) +
                                          ":" + QString::number(pat) + ":0";
                            if(libtoolify[i] != "QMAKE_AR_CMD") {
                                QString rpath = Option::output_dir;
                                if(!project->isEmpty("DESTDIR")) {
                                    rpath = project->first("DESTDIR");
                                    if(QDir::isRelativePath(rpath))
                                        rpath.prepend(Option::output_dir + Option::dir_sep);
                                }
                                comp_flags += " -rpath " + Option::fixPathToTargetOS(rpath, false);
                            }
                        }
                    }
                    if(project->isActiveConfig("plugin"))
                        libtool_flags += " -module";
                } else {
                    libtool_flags += " --mode=compile";
                }
                l.first().prepend("$(LIBTOOL)" + libtool_flags + " ");
                if(!comp_flags.isEmpty())
                    l.first() += comp_flags;
            }
        }
    }
}

void
UnixMakefileGenerator::processPrlVariable(const QString &var, const QStringList &l)
{
    if(var == "QMAKE_PRL_LIBS") {
        project->variables()["QMAKE_CURRENT_PRL_LIBS"] += l;
    } else
        MakefileGenerator::processPrlVariable(var, l);
}

QStringList
&UnixMakefileGenerator::findDependencies(const QString &file)
{
    QStringList &ret = MakefileGenerator::findDependencies(file);
    // Note: The QMAKE_IMAGE_COLLECTION file have all images
    // as dependency, so don't add precompiled header then
    if(doPrecompiledHeaders() && !project->isEmpty("PRECOMPILED_HEADER")
       && file != project->first("QMAKE_IMAGE_COLLECTION")) {
        QString header_prefix = project->first("QMAKE_ORIG_TARGET") + ".gch" + Option::dir_sep;
        header_prefix += project->first("QMAKE_PRECOMP_PREFIX");
        if(file.endsWith(".c")) {
            QString precomp_h = header_prefix + "c";
            if(!ret.contains(precomp_h))
                ret += precomp_h;
        } else {
            for(QStringList::Iterator it = Option::cpp_ext.begin(); it != Option::cpp_ext.end(); ++it) {
                if(file.endsWith(*it)) {
                    QString precomp_h = header_prefix + "c++";
                    if(!ret.contains(precomp_h))
                        ret += precomp_h;
                    break;
                }
            }
        }
    }
    return ret;
}

bool
UnixMakefileGenerator::findLibraries()
{
    QList<QMakeLocalFileName> libdirs, frameworkdirs;
    frameworkdirs.append(QMakeLocalFileName("/System/Library/Frameworks"));
    frameworkdirs.append(QMakeLocalFileName("/Library/Frameworks"));
    const QString lflags[] = { "QMAKE_LIBDIR_FLAGS", "QMAKE_LIBS", QString() };
    for(int i = 0; !lflags[i].isNull(); i++) {
        QStringList &l = project->variables()[lflags[i]];
        for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
            bool do_suffix = true;
            QString stub, dir, extn, opt = (*it).trimmed();
            if(opt.startsWith("-")) {
                if(opt.startsWith("-L")) {
                    libdirs.append(QMakeLocalFileName(opt.right(opt.length()-2)));
                } else if(opt.startsWith("-l")) {
                    stub = opt.mid(2);
                } else if(Option::target_mode == Option::TARG_MACX_MODE && opt.startsWith("-F")) {
                    frameworkdirs.append(QMakeLocalFileName(opt.right(opt.length()-2)));
                } else if(Option::target_mode == Option::TARG_MACX_MODE && opt.startsWith("-framework")) {
                    if(opt.length() > 11) {
                        opt = opt.mid(11);
                    } else {
                        ++it;
                        opt = (*it);
                    }
                    do_suffix = false;
                    extn = "";
                    dir = "/System/Library/Frameworks/" + opt + ".framework/";
                    stub = opt;
                }
            } else {
                extn = dir = "";
                stub = opt;
                int slsh = opt.lastIndexOf(Option::dir_sep);
                if(slsh != -1) {
                    dir = opt.left(slsh);
                    stub = opt.mid(slsh+1);
                }
                QRegExp stub_reg("^.*lib(" + stub + "[^./=]*)\\.(.*)$");
                if(stub_reg.exactMatch(stub)) {
                    stub = stub_reg.cap(1);
                    extn = stub_reg.cap(2);
                }
            }
            if(!stub.isEmpty()) {
                if(do_suffix && !project->isEmpty("QMAKE_" + stub.toUpper() + "_SUFFIX"))
                    stub += project->first("QMAKE_" + stub.toUpper() + "_SUFFIX");
                bool found = false;
                QStringList extens;
                if(!extn.isNull())
                    extens << extn;
                else
                    extens << project->variables()["QMAKE_EXTENSION_SHLIB"].first() << "a";
                for(QStringList::Iterator extit = extens.begin(); extit != extens.end(); ++extit) {
                    if(dir.isNull()) {
                        QString lib_stub;
                        for(QList<QMakeLocalFileName>::Iterator dep_it = libdirs.begin(); dep_it != libdirs.end(); ++dep_it) {
                            if(exists((*dep_it).local() + Option::dir_sep + "lib" + stub +
                                             "." + (*extit))) {
                                lib_stub = stub;
                                break;
                            }
                        }
                        if(!lib_stub.isNull()) {
                            (*it) = "-l" + lib_stub;
                            found = true;
                            break;
                        }
                    } else {
                        if(exists("lib" + stub + "." + (*extit))) {
                            (*it) = "lib" + stub + "." + (*extit);
                            found = true;
                            break;
                        }
                    }
                }
                if(!found && project->isActiveConfig("compile_libtool")) {
                    for(QList<QMakeLocalFileName>::Iterator dep_it = libdirs.begin(); dep_it != libdirs.end(); ++dep_it) {
                        if(exists((*dep_it).local() + Option::dir_sep + "lib" + stub + Option::libtool_ext)) {
                            (*it) = (*dep_it).real() + Option::dir_sep + "lib" + stub + Option::libtool_ext;
                            found = true;
                            break;
                        }
                    }
                }
            }
        }
    }
    return false;
}

QString linkLib(const QString &file, const QString &libName) {
  QString ret;
  QRegExp reg("^.*lib(" + QRegExp::escape(libName) + "[^./=]*).*$");
  if(reg.exactMatch(file))
    ret = "-l" + reg.cap(1);
  return ret;
}

void
UnixMakefileGenerator::processPrlFiles()
{
    QList<QMakeLocalFileName> libdirs, frameworkdirs;
    frameworkdirs.append(QMakeLocalFileName("/System/Library/Frameworks"));
    frameworkdirs.append(QMakeLocalFileName("/Library/Frameworks"));
    const QString lflags[] = { "QMAKE_LIBDIR_FLAGS", "QMAKE_LIBS", QString() };
    for(int i = 0; !lflags[i].isNull(); i++) {
            QStringList &l = project->variables()[lflags[i]];
        for(int lit = 0; lit < l.size(); ++lit) {
            QString opt = l.at(lit).trimmed();
                if(opt.startsWith("-")) {
                    if(opt.startsWith("-L")) {
                        libdirs.append(QMakeLocalFileName(opt.right(opt.length()-2)));
                } else if(opt.startsWith("-l")) {
                        QString lib = opt.right(opt.length() - 2);
                        for(QList<QMakeLocalFileName>::Iterator dep_it = libdirs.begin(); dep_it != libdirs.end(); ++dep_it) {
                            if(!project->isActiveConfig("compile_libtool")) { //give them the .libs..
                                QString la = (*dep_it).local() + Option::dir_sep + "lib" + lib + Option::libtool_ext;
                                if(exists(la) && QFile::exists((*dep_it).local() + Option::dir_sep + ".libs")) {
                                    QString dot_libs = (*dep_it).real() + Option::dir_sep + ".libs";
                                l.append("-L" + dot_libs);
                                    libdirs.append(QMakeLocalFileName(dot_libs));
                                }
                            }

                            QString prl = (*dep_it).local() + Option::dir_sep + "lib" + lib;
                            if(!project->isEmpty("QMAKE_" + lib.toUpper() + "_SUFFIX"))
                                prl += project->first("QMAKE_" + lib.toUpper() + "_SUFFIX");
                            if(processPrlFile(prl)) {
                                if(prl.startsWith((*dep_it).local()))
                                    prl.replace(0, (*dep_it).local().length(), (*dep_it).real());
                                opt = linkLib(prl, lib);
                                break;
                            }
                        }
                    } else if(Option::target_mode == Option::TARG_MACX_MODE && opt.startsWith("-F")) {
                        frameworkdirs.append(QMakeLocalFileName(opt.right(opt.length()-2)));
                    } else if(Option::target_mode == Option::TARG_MACX_MODE && opt.startsWith("-framework")) {
                    if(opt.length() > 11)
                            opt = opt.mid(11);
                    else
                        opt = l.at(++lit);
                    opt = opt.trimmed();
                        for(QList<QMakeLocalFileName>::Iterator dep_it = frameworkdirs.begin();
                            dep_it != frameworkdirs.end(); ++dep_it) {
                            QString prl = (*dep_it).local() + "/" + opt + ".framework/" + opt + Option::prl_ext;
                        if(processPrlFile(prl))
                                break;
                            }
                    }
                } else if(!opt.isNull()) {
                    QString lib = opt;
                processPrlFile(lib);
#if 0
                    if(ret)
                      opt = linkLib(lib, "");
#endif
                    if(!opt.isEmpty())
                    l.replaceInStrings(lib, opt);
            }

            QStringList &prl_libs = project->variables()["QMAKE_CURRENT_PRL_LIBS"];
            if(!prl_libs.isEmpty()) {
                for(int prl = 0; prl < prl_libs.size(); ++prl)
                    l.insert(lit+prl+1, prl_libs.at(prl));
                prl_libs.clear();
                }
            }

        //merge them into a logical order
        if(!project->isActiveConfig("no_smart_library_merge") && !project->isActiveConfig("no_lflags_merge")) {
            QStringList lflags;
            for(int lit = 0; lit < l.size(); ++lit) {
                QString opt = l.at(lit).trimmed();
                if(opt.startsWith("-")) {
                    if(opt.startsWith("-L") ||
                       (Option::target_mode == Option::TARG_MACX_MODE && opt.startsWith("-F"))) {
                        if(lit == 0 || l.lastIndexOf(opt, lit-1) == -1)
                            lflags.append(opt);
                    } else if(opt.startsWith("-l")) {
                        if(lit == l.size()-1 || l.indexOf(opt, lit+1) == -1)
                            lflags.append(opt);
                    } else if(Option::target_mode == Option::TARG_MACX_MODE && opt.startsWith("-framework")) {
                        if(opt.length() > 11)
                            opt = opt.mid(11);
                        else
                            opt = l.at(++lit);
                        bool found = false;
                        for(int x = lit+1; x < l.size(); ++x) {
                            QString xf = l.at(x);
                            if(xf.startsWith("-framework")) {
                                QString framework;
                                if(xf.length() > 11)
                                    framework = xf.mid(11);
            else
                                    framework = l.at(++x);
                                if(framework == opt) {
                                    found = true;
                break;
        }
    }
                        }
                        if(!found) {
                            lflags.append("-framework");
                            lflags.append(opt);
                        }
                    } else {
                        lflags.append(opt);
                    }
                } else if(!opt.isNull()) {
                    if(lit == 0 || l.lastIndexOf(opt, lit-1) == -1)
                        lflags.append(opt);
                }
            }
            l = lflags;
        }
    }
}

QString
UnixMakefileGenerator::defaultInstall(const QString &t)
{
    if(t != "target" || project->first("TEMPLATE") == "subdirs")
        return QString();

    bool bundle = false;
    const QString root = "$(INSTALL_ROOT)";
    QStringList &uninst = project->variables()[t + ".uninstall"];
    QString ret, destdir=project->first("DESTDIR");
    QString targetdir = Option::fixPathToTargetOS(project->first("target.path"), false);
    if(!destdir.isEmpty() && destdir.right(1) != Option::dir_sep)
        destdir += Option::dir_sep;
    targetdir = fileFixify(targetdir, FileFixifyAbsolute);
    if(targetdir.right(1) != Option::dir_sep)
        targetdir += Option::dir_sep;

    QStringList links;
    QString target="$(TARGET)";
    if(!project->isEmpty("QMAKE_BUNDLE_NAME")) {
        target = project->first("QMAKE_BUNDLE_NAME");
        bundle = true;
    } else if(project->first("TEMPLATE") == "app") {
        target = "$(QMAKE_TARGET)";
    } else if(project->first("TEMPLATE") == "lib") {
        if(project->isActiveConfig("create_prl") && !project->isActiveConfig("no_install_prl") &&
           !project->isEmpty("QMAKE_INTERNAL_PRL_FILE")) {
            QString dst_prl = project->first("QMAKE_INTERNAL_PRL_FILE");
            int slsh = dst_prl.lastIndexOf('/');
            if(slsh != -1)
                dst_prl = dst_prl.right(dst_prl.length() - slsh - 1);
            dst_prl = filePrefixRoot(root, targetdir + dst_prl);
            ret += "-$(INSTALL_FILE) \"" + project->first("QMAKE_INTERNAL_PRL_FILE") + "\" \"" + dst_prl + "\"";
            if(!uninst.isEmpty())
                uninst.append("\n\t");
            uninst.append("-$(DEL_FILE) \"" + dst_prl + "\"");
        }
        if(project->isActiveConfig("create_libtool") && !project->isActiveConfig("compile_libtool")) {
            QString src_lt = var("QMAKE_ORIG_TARGET");
            int slsh = src_lt.lastIndexOf(Option::dir_sep);
            if(slsh != -1)
                src_lt = src_lt.right(src_lt.length() - slsh);
            int dot = src_lt.indexOf('.');
            if(dot != -1)
                src_lt = src_lt.left(dot);
            src_lt += Option::libtool_ext;
            src_lt.prepend("lib");
            QString dst_lt = filePrefixRoot(root, targetdir + src_lt);
            if(!project->isEmpty("DESTDIR")) {
                src_lt.prepend(var("DESTDIR"));
                src_lt = Option::fixPathToLocalOS(fileFixify(src_lt,
                                                             qmake_getpwd(), Option::output_dir, FileFixifyAbsolute));
            }
            if(!ret.isEmpty())
                ret += "\n\t";
            ret += "-$(INSTALL_FILE) \"" + src_lt + "\" \"" + dst_lt + "\"";
            if(!uninst.isEmpty())
                uninst.append("\n\t");
            uninst.append("-$(DEL_FILE) \"" + dst_lt + "\"");
        }
        if(project->isActiveConfig("create_pc")) {
            QString src_pc = var("QMAKE_ORIG_TARGET");
            int slsh = src_pc.lastIndexOf(Option::dir_sep);
            if(slsh != -1)
                src_pc = src_pc.right(src_pc.length() - slsh);
            int dot = src_pc.indexOf('.');
            if(dot != -1)
                src_pc = src_pc.left(dot);
            src_pc += ".pc";
            QString d = filePrefixRoot(root, targetdir + "pkgconfig" + Option::dir_sep);
            QString dst_pc = d + src_pc;
            if(!project->isEmpty("DESTDIR")) {
                src_pc.prepend(var("DESTDIR"));
                src_pc = Option::fixPathToLocalOS(fileFixify(src_pc,
                                                             qmake_getpwd(), Option::output_dir, FileFixifyAbsolute));
            }
            if(!ret.isEmpty())
                ret += "\n\t";
            ret += mkdir_p_asstring(d) + "\n\t";
            ret += "-$(INSTALL_FILE) \"" + src_pc + "\" \"" + dst_pc + "\"";
            if(!uninst.isEmpty())
                uninst.append("\n\t");
            uninst.append("-$(DEL_FILE) \"" + dst_pc + "\"");
        }
        if(project->isEmpty("QMAKE_CYGWIN_SHLIB")) {
            if(!project->isActiveConfig("staticlib") && !project->isActiveConfig("plugin")) {
                if(project->isEmpty("QMAKE_HPUX_SHLIB")) {
                    links << "$(TARGET0)" << "$(TARGET1)" << "$(TARGET2)";
                } else {
                    links << "$(TARGET0)";
                }
            }
        }
    }

    if(!bundle && project->isActiveConfig("compile_libtool")) {
        QString src_targ = target;
        if(src_targ == "$(TARGET)")
            src_targ = "$(TARGETL)";
        QString dst_dir = fileFixify(targetdir, FileFixifyAbsolute);
        if(QDir::isRelativePath(dst_dir))
            dst_dir = Option::fixPathToTargetOS(Option::output_dir + Option::dir_sep + dst_dir);
        ret = "-$(LIBTOOL) --mode=install cp \"" + src_targ + "\" \"" + filePrefixRoot(root, dst_dir) + "\"";
        uninst.append("-$(LIBTOOL) --mode=uninstall \"" + src_targ + "\"");
    } else {
        QString src_targ = target;
        if(!destdir.isEmpty())
            src_targ = Option::fixPathToTargetOS(destdir + target, false);
        QString dst_targ = filePrefixRoot(root, fileFixify(targetdir + target, FileFixifyAbsolute));
        if(bundle) {
            if(!ret.isEmpty())
                ret += "\n\t";
            ret += "$(DEL_FILE) -r \"" + dst_targ + "\"\n\t";
        }
        if(!ret.isEmpty())
            ret += "\n\t";

        const QString copy_cmd = QString(bundle ? "-$(INSTALL_DIR)" : "-$(INSTALL_FILE)") + " \"" +
                                 src_targ + "\" \"" + dst_targ + "\"";
        if(project->first("TEMPLATE") == "lib" && !project->isActiveConfig("staticlib")
           && project->values(t + ".CONFIG").indexOf("fix_rpath") != -1) {
            if(!project->isEmpty("QMAKE_FIX_RPATH")) {
                ret += copy_cmd;
                ret += "\n\t-" + var("QMAKE_FIX_RPATH") + " \"" +
                       dst_targ + "\" \"" + dst_targ + "\"";
            } else if(!project->isEmpty("QMAKE_LFLAGS_RPATH")) {
                ret += "-$(LINK) $(LFLAGS) " + var("QMAKE_LFLAGS_RPATH") + targetdir + " -o \"" +
                       dst_targ + "\" $(OBJECTS) $(LIBS) $(OBJCOMP)";
            } else {
                ret += copy_cmd;
            }
        } else {
            ret += copy_cmd;
        }

        if(!project->isActiveConfig("debug") && !project->isEmpty("QMAKE_STRIP") &&
           (project->first("TEMPLATE") != "lib" || !project->isActiveConfig("staticlib"))) {
            ret += "\n\t-" + var("QMAKE_STRIP");
            if(project->first("TEMPLATE") == "lib" && !project->isEmpty("QMAKE_STRIPFLAGS_LIB"))
                ret += " " + var("QMAKE_STRIPFLAGS_LIB");
            else if(project->first("TEMPLATE") == "app" && !project->isEmpty("QMAKE_STRIPFLAGS_APP"))
                ret += " " + var("QMAKE_STRIPFLAGS_APP");
            if(bundle)
                ret = " \"" + dst_targ + "/Contents/MacOS/$(QMAKE_TARGET)\"";
            else
                ret += " \"" + dst_targ + "\"";
        }
        if(!uninst.isEmpty())
            uninst.append("\n\t");
        if(bundle)
            uninst.append("-$(DEL_FILE) -r \"" + dst_targ + "\"");
        else
            uninst.append("-$(DEL_FILE) \"" + dst_targ + "\"");
        if(!links.isEmpty()) {
            for(QStringList::Iterator it = links.begin(); it != links.end(); it++) {
                if(Option::target_mode == Option::TARG_WIN_MODE ||
                   Option::target_mode == Option::TARG_MAC9_MODE) {
                } else if(Option::target_mode == Option::TARG_UNIX_MODE ||
                          Option::target_mode == Option::TARG_MACX_MODE) {
                    QString link = Option::fixPathToTargetOS(destdir + (*it), false);
                    int lslash = link.lastIndexOf(Option::dir_sep);
                    if(lslash != -1)
                        link = link.right(link.length() - (lslash + 1));
                    QString dst_link = filePrefixRoot(root, fileFixify(targetdir + link, FileFixifyAbsolute));
                    ret += "\n\t-$(SYMLINK) \"$(TARGET)\" \"" + dst_link + "\"";
                    if(!uninst.isEmpty())
                        uninst.append("\n\t");
                    uninst.append("-$(DEL_FILE) \"" + dst_link + "\"");
                }
            }
        }
    }
    return ret;
}
