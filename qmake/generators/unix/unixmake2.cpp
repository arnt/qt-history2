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
#include "meta.h"
#include <qregexp.h>
#include <qbytearray.h>
#include <qfile.h>
#include <qdir.h>
#include <time.h>

QString mkdir_p_asstring(const QString &dir);

UnixMakefileGenerator::UnixMakefileGenerator() : MakefileGenerator(), init_flag(false), include_deps(false)
{

}

void
UnixMakefileGenerator::writePrlFile(QTextStream &t)
{
    MakefileGenerator::writePrlFile(t);
    // libtool support
    if(project->isActiveConfig("create_libtool") && project->first("TEMPLATE") == "lib") { //write .la
        if(project->isActiveConfig("compile_libtool"))
            warn_msg(WarnLogic, "create_libtool specified with compile_libtool can lead to conflicting .la\n"
                     "formats, create_libtool has been disabled\n");
        else
            writeLibtoolFile();
    }
    // pkg-config support
    if(project->isActiveConfig("create_pc") && project->first("TEMPLATE") == "lib")
        writePkgConfigFile();
}

bool
UnixMakefileGenerator::writeMakefile(QTextStream &t)
{

    writeHeader(t);
    if(!project->variables()["QMAKE_FAILED_REQUIREMENTS"].isEmpty()) {
        t << "QMAKE    = "        << (project->isEmpty("QMAKE_QMAKE") ? QString("qmake") : var("QMAKE_QMAKE")) << endl;
        QStringList &qut = project->variables()["QMAKE_EXTRA_TARGETS"];
        for(QStringList::ConstIterator it = qut.begin(); it != qut.end(); ++it)
            t << *it << " ";
        t << "first all clean install distclean mocables uninstall uicables:" << "\n\t"
          << "@echo \"Some of the required modules ("
          << var("QMAKE_FAILED_REQUIREMENTS") << ") are not available.\"" << "\n\t"
          << "@echo \"Skipped.\"" << endl << endl;
        writeMakeQmake(t);
        return true;
    }

    if (project->variables()["TEMPLATE"].first() == "app" ||
        project->variables()["TEMPLATE"].first() == "lib") {
        writeMakeParts(t);
        return MakefileGenerator::writeMakefile(t);
    } else if(project->variables()["TEMPLATE"].first() == "subdirs") {
        MakefileGenerator::writeSubDirs(t);
        return true;
    }
    return false;
}

void
UnixMakefileGenerator::writeMakeParts(QTextStream &t)
{
    QString deps = fileFixify(Option::output.fileName()), target_deps, prl;
    bool do_incremental = (project->isActiveConfig("incremental") &&
                           !project->variables()["QMAKE_INCREMENTAL"].isEmpty() &&
                           (!project->variables()["QMAKE_APP_FLAG"].isEmpty() ||
                            (!project->isActiveConfig("staticlib")))),
         src_incremental=false, moc_incremental=false;

    t << "####### Compiler, tools and options" << endl << endl;
    t << "CC            = " << var("QMAKE_CC") << endl;
    t << "CXX           = " << var("QMAKE_CXX") << endl;
    t << "LEX           = " << var("QMAKE_LEX") << endl;
    t << "YACC          = " << var("QMAKE_YACC") << endl;
    t << "DEFINES       = "
      << varGlue("PRL_EXPORT_DEFINES","-D"," -D"," ")
      << varGlue("DEFINES","-D"," -D","") << endl;
    t << "CFLAGS        = " << var("QMAKE_CFLAGS") << " $(DEFINES)" << endl;
    t << "CXXFLAGS      = " << var("QMAKE_CXXFLAGS") << " $(DEFINES)" << endl;
    t << "LEXFLAGS      = " << var("QMAKE_LEXFLAGS") << endl;
    t << "YACCFLAGS     = " << var("QMAKE_YACCFLAGS") << endl;
    t << "INCPATH       = " << "-I" << specdir();
    if(!project->isActiveConfig("no_include_pwd")) {
        QString pwd = fileFixify(QDir::currentPath());
        if(pwd.isEmpty())
            pwd = ".";
        t << " -I" << pwd;
    }
    t << varGlue("INCLUDEPATH"," -I", " -I", "") << endl;

    if(!project->isActiveConfig("staticlib")) {
        t << "LINK          = " << var("QMAKE_LINK") << endl;
        t << "LFLAGS        = " << var("QMAKE_LFLAGS") << endl;
        t << "LIBS          = " << "$(SUBLIBS) " << var("QMAKE_LIBDIR_FLAGS") << " " << var("QMAKE_LIBS") << endl;
    }

    t << "AR            = " << var("QMAKE_AR") << endl;
    t << "RANLIB        = " << var("QMAKE_RANLIB") << endl;
    t << "MOC           = " << var("QMAKE_MOC") << endl;
    t << "UIC3          = " << var("QMAKE_UIC3") << endl;
    t << "UIC           = " << var("QMAKE_UIC") << endl;
    t << "QMAKE         = " << (project->isEmpty("QMAKE_QMAKE") ? QString("qmake") : var("QMAKE_QMAKE")) << endl;
    t << "TAR           = " << var("QMAKE_TAR") << endl;
    t << "COMPRESS      = " << var("QMAKE_GZIP") << endl;
    if(project->isActiveConfig("compile_libtool"))
        t << "LIBTOOL       = " << var("QMAKE_LIBTOOL") << endl;
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
    if(!project->isEmpty("QMAKE_MACOSX_DEPLOYMENT_TARGET"))
        t << "export MACOSX_DEPLOYMENT_TARGET = " //exported to children processes
          << project->first("QMAKE_MACOSX_DEPLOYMENT_TARGET") << endl;
    t << endl;

    t << "####### Output directory" << endl << endl;
    if (! project->variables()["OBJECTS_DIR"].isEmpty())
        t << "OBJECTS_DIR   = " << var("OBJECTS_DIR") << endl;
    else
        t << "OBJECTS_DIR   = ./" << endl;
    t << endl;

    /* files */
    t << "####### Files" << endl << endl;
    t << "HEADERS       = " << varList("HEADERS") << endl;
    t << "SOURCES       = " << varList("SOURCES") << endl;
    if(do_incremental) {
        QStringList &objs = project->variables()["OBJECTS"], &incrs = project->variables()["QMAKE_INCREMENTAL"], incrs_out;
        t << "OBJECTS       = ";
        for(QStringList::Iterator objit = objs.begin(); objit != objs.end(); ++objit) {
            bool increment = false;
            for(QStringList::Iterator incrit = incrs.begin(); incrit != incrs.end(); ++incrit) {
                if((*objit).indexOf(QRegExp((*incrit), Qt::CaseSensitive,
                                    QRegExp::Wildcard)) != -1) {
                    increment = true;
                    incrs_out.append((*objit));
                    break;
                }
            }
            if(!increment)
                t << "\\\n\t\t" << (*objit);
        }
        if(incrs_out.count() == objs.count()) { //we just switched places, no real incrementals to be done!
            t << incrs_out.join(" \\\n\t\t") << endl;
        } else if(!incrs_out.count()) {
            t << endl;
        } else {
            src_incremental = true;
            t << endl;
            t << "INCREMENTAL_OBJECTS = " << incrs_out.join(" \\\n\t\t") << endl;
        }
    } else {
        t << "OBJECTS       = " << varList("OBJECTS") << endl;
    }
    QString srcMoc = varList("SRCMOC"), objMoc = varList("OBJMOC");
    t << "SRCMOC        = " << srcMoc << endl;
    if(do_incremental) {
        QStringList &objs = project->variables()["OBJMOC"],
                   &incrs = project->variables()["QMAKE_INCREMENTAL"], incrs_out;
        t << "OBJMOC        = ";
        for(QStringList::Iterator objit = objs.begin(); objit != objs.end(); ++objit) {
            bool increment = false;
            for(QStringList::Iterator incrit = incrs.begin(); incrit != incrs.end(); ++incrit) {
                if((*objit).indexOf(QRegExp((*incrit), Qt::CaseSensitive,
                                    QRegExp::Wildcard)) != -1) {
                    increment = true;
                    incrs_out.append((*objit));
                    break;
                }
            }
            if(!increment)
                t << "\\\n\t\t" << (*objit);
        }
        if(incrs_out.count() == objs.count()) { //we just switched places, no real incrementals to be done!
            t << incrs_out.join(" \\\n\t\t") << endl;
        } else if(!incrs_out.count()) {
            t << endl;
        } else {
            moc_incremental = true;
            t << endl;
            t << "INCREMENTAL_OBJMOC = " << incrs_out.join(" \\\n\t\t") << endl;
        }
    } else {
        t << "OBJMOC        = " << objMoc << endl;
    }
    if(do_incremental && !moc_incremental && !src_incremental)
        do_incremental = false;
    t << "DIST          = " << valList(fileFixify(project->variables()["DISTFILES"])) << endl;
    t << "QMAKE_TARGET  = " << var("QMAKE_ORIG_TARGET") << endl;
    t << "DESTDIR       = " << var("DESTDIR") << endl;
    if(project->isActiveConfig("compile_libtool"))
        t << "TARGETL       = " << var("TARGET_la") << endl;
    t << "TARGET        = " << var("TARGET") << endl;
    if(project->isActiveConfig("plugin")) {
        t << "TARGETD       = " << var("TARGET") << endl;
    } else if (!project->isActiveConfig("staticlib") && project->variables()["QMAKE_APP_FLAG"].isEmpty()) {
        t << "TARGETA       = " << var("TARGETA") << endl;
        if (project->isEmpty("QMAKE_HPUX_SHLIB")) {
            t << "TARGETD       = " << var("TARGET_x.y.z") << endl;
            t << "TARGET0       = " << var("TARGET_") << endl;
            t << "TARGET1       = " << var("TARGET_x") << endl;
            t << "TARGET2       = " << var("TARGET_x.y") << endl;
        } else {
            t << "TARGETD       = " << var("TARGET_x") << endl;
            t << "TARGET0       = " << var("TARGET_") << endl;
        }
    }
    writeExtraCompilerVariables(t);
    writeExtraVariables(t);
    t << endl;

    // blasted includes
    QStringList &qeui = project->variables()["QMAKE_EXTRA_INCLUDES"];
    QStringList::Iterator it;
    for(it = qeui.begin(); it != qeui.end(); ++it)
        t << "include " << (*it) << endl;

    /* rules */
    t << "first: all" << endl;
    t << "####### Implicit rules" << endl << endl;
    t << ".SUFFIXES: .c " << Option::obj_ext;
    QStringList::Iterator cppit;
    for(cppit = Option::cpp_ext.begin(); cppit != Option::cpp_ext.end(); ++cppit)
        t << " " << (*cppit);
    t << endl << endl;
    for(cppit = Option::cpp_ext.begin(); cppit != Option::cpp_ext.end(); ++cppit)
        t << (*cppit) << Option::obj_ext << ":\n\t" << var("QMAKE_RUN_CXX_IMP") << endl << endl;
    t << ".c" << Option::obj_ext << ":\n\t" << var("QMAKE_RUN_CC_IMP") << endl << endl;

    if(include_deps) {
        QString cmd=var("QMAKE_CFLAGS_DEPS") + " ";
        cmd += varGlue("DEFINES","-D"," -D","") + varGlue("PRL_EXPORT_DEFINES"," -D"," -D","");
        if(!project->isEmpty("QMAKE_ABSOLUTE_SOURCE_PATH"))
            cmd += " -I" + project->first("QMAKE_ABSOLUTE_SOURCE_PATH") + " ";
        cmd += " $(INCPATH) " + varGlue("DEPENDPATH", "-I", " -I", "");
        QString odir;
        if(!project->variables()["OBJECTS_DIR"].isEmpty())
            odir = project->first("OBJECTS_DIR");
        t << "###### Dependencies" << endl << endl;
        t << odir << ".deps/%.d: %.cpp\n\t"
          << "@echo Creating depend for $<" << "\n\t"
          << "@test -d $(@D) || mkdir -p $(@D)" << "\n\t"
          << "@$(CXX) " << cmd << " $< | sed \"s,^\\($(*F).o\\):," << odir << "\\1:,g\" >$@" << endl << endl;

        t << odir << ".deps/%.d: %.c\n\t"
          << "@echo Creating depend for $<" << "\n\t"
          << "@test -d $(@D) || mkdir -p $(@D)" << "\n\t"
          << "@$(CC) " << cmd << " $< | sed \"s,^\\($(*F).o\\):," << odir << "\\1:,g\" >$@" << endl << endl;

        QString src[] = { "SOURCES", "UICIMPLS", "SRCMOC", QString::null };
        for(int x = 0; !src[x].isNull(); x++) {
            QStringList &l = project->variables()[src[x]];
            for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
                if(!(*it).isEmpty()) {
                    QString d_file;
                    if((*it).endsWith(".c")) {
                        d_file = (*it).left((*it).length() - 2);
                    } else {
                        for(QStringList::Iterator cppit = Option::cpp_ext.begin();
                            cppit != Option::cpp_ext.end(); ++cppit) {
                            if((*it).endsWith((*cppit))) {
                                d_file = (*it).left((*it).length() - (*cppit).length());
                                break;
                            }
                        }
                    }
                    if(!d_file.isEmpty()) {
                        d_file = odir + ".deps/" + d_file + ".d";
                        QStringList deps = findDependencies((*it)).find(QRegExp(Option::cpp_moc_ext + "$"));
                        if(!deps.isEmpty())
                            t << d_file << ": " << deps.join(" ") << endl;
                        t << var("QMAKE_CFLAGS_USE_PRECOMPILE") << " " << d_file << endl;
                    }
                }
            }
        }
    }

    t << "####### Build rules" << endl << endl;
    if(!project->variables()["SUBLIBS"].isEmpty()) {
        QString libdir = "tmp/";
        if(!project->isEmpty("SUBLIBS_DIR"))
            libdir = project->first("SUBLIBS_DIR");
        t << "SUBLIBS       = ";
        QStringList &l = project->variables()["SUBLIBS"];
        for(QStringList::Iterator it = l.begin(); it != l.end(); ++it)
            t << libdir << "lib" << (*it) << ".a ";
        t << endl << endl;
    }
    if(project->isActiveConfig("depend_prl") && !project->isEmpty("QMAKE_PRL_INTERNAL_FILES")) {
        QStringList &l = project->variables()["QMAKE_PRL_INTERNAL_FILES"];
        QStringList::Iterator it;
        for(it = l.begin(); it != l.end(); ++it) {
            QMakeMetaInfo libinfo;
            if(libinfo.readLib((*it)) && !libinfo.isEmpty("QMAKE_PRL_BUILD_DIR")) {
                QString dir;
                int slsh = (*it).lastIndexOf(Option::dir_sep);
                if(slsh != -1)
                    dir = (*it).left(slsh + 1);
                QString targ = dir + libinfo.first("QMAKE_PRL_TARGET");
                deps += " " + targ;
                t << targ << ":" << "\n\t"
                  << "@echo \"Creating '" << targ << "'\"" << "\n\t"
                  << "(cd " << libinfo.first("QMAKE_PRL_BUILD_DIR") << ";"
                  << "$(MAKE))" << endl;
            }
        }
    }
    if(!project->variables()["QMAKE_APP_FLAG"].isEmpty()) {
        QString destdir = project->first("DESTDIR");
        if(do_incremental) {
            //incremental target
            QString incr_target = var("TARGET") + "_incremental";
            if(incr_target.indexOf(Option::dir_sep) != -1)
                incr_target = incr_target.right(incr_target.length() -
                                                (incr_target.lastIndexOf(Option::dir_sep) + 1));
            QString incr_deps, incr_objs;
            if(project->first("QMAKE_INCREMENTAL_STYLE") == "ld") {
                QString incr_target_dir = var("OBJECTS_DIR") + incr_target + Option::obj_ext;
                //actual target
                t << incr_target_dir << ": $(OBJECTS)" << "\n\t"
                  << "ld -r  -o "<< incr_target_dir << " $(OBJECTS)" << endl;
                //communicated below
                deps.prepend(incr_target_dir + " ");
                incr_deps = "$(INCREMENTAL_OBJECTS) $(INCREMENTAL_OBJMOC) $(OBJMOC)";
                if(!incr_objs.isEmpty())
                    incr_objs += " ";
                incr_objs += incr_target_dir;
            } else {
                //actual target
                QString incr_target_dir = var("DESTDIR") + "lib" + incr_target + "." +
                                          project->variables()["QMAKE_EXTENSION_SHLIB"].first();
                QString incr_lflags = var("QMAKE_LFLAGS_SHLIB") + " ";
                if(project->isActiveConfig("debug"))
                    incr_lflags += var("QMAKE_LFLAGS_DEBUG");
                else
                    incr_lflags += var("QMAKE_LFLAGS_RELEASE");
                t << incr_target_dir << ": $(INCREMENTAL_OBJECTS) $(INCREMENTAL_OBJMOC)" << "\n\t";
                if(!destdir.isEmpty())
                    t << "\n\t" << "test -d " << destdir << " || mkdir -p " << destdir << "\n\t";
                t << "$(LINK) " << incr_lflags << " -o "<< incr_target_dir <<
                    " $(INCREMENTAL_OBJECTS) $(INCREMENTAL_OBJMOC)" << endl;
                //communicated below
                if(!destdir.isEmpty()) {
                    if(!incr_objs.isEmpty())
                        incr_objs += " ";
                    incr_objs += "-L" + destdir;
                } else {
                    if(!incr_objs.isEmpty())
                        incr_objs += " ";
                    incr_objs += "-L" + QDir::currentPath();
                }
                if(!incr_objs.isEmpty())
                    incr_objs += " ";
                incr_objs += " -l" + incr_target;
                deps.prepend(incr_target_dir + " ");
                incr_deps = "$(OBJECTS) $(OBJMOC)";
            }
            t << "all: " << deps <<  " " << varGlue("ALL_DEPS",""," "," ") <<  "$(TARGET)"
              << endl << endl;

            //real target
            t << var("TARGET") << ": " << var("PRE_TARGETDEPS") << " " << incr_deps << " " << target_deps
              << " " << var("POST_TARGETDEPS") << "\n\t";
            if(!destdir.isEmpty())
                t << "\n\t" << "test -d " << destdir << " || mkdir -p " << destdir << "\n\t";
            if(!project->isEmpty("QMAKE_PRE_LINK"))
                t << var("QMAKE_PRE_LINK") << "\n\t";
            t << "$(LINK) $(LFLAGS) -o $(TARGET) " << incr_deps << " " << incr_objs << " $(OBJCOMP) $(LIBS)";
            if(!project->isEmpty("QMAKE_POST_LINK"))
                t << "\n\t" << var("QMAKE_POST_LINK");
            t << endl << endl;
        } else {
            t << "all: " << deps <<  " " << varGlue("ALL_DEPS",""," "," ") <<  "$(TARGET)"
              << endl << endl;

            t << "$(TARGET): " << var("PRE_TARGETDEPS") << " $(OBJECTS) $(OBJMOC) "
              << target_deps << " " << var("POST_TARGETDEPS") << "\n\t";
            if(!destdir.isEmpty())
                t << "test -d " << destdir << " || mkdir -p " << destdir << "\n\t";
            if(!project->isEmpty("QMAKE_PRE_LINK"))
                t << var("QMAKE_PRE_LINK") << "\n\t";
            t << "$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(OBJMOC) $(OBJCOMP) $(LIBS)";
            if(!project->isEmpty("QMAKE_POST_LINK"))
                t << "\n\t" << var("QMAKE_POST_LINK");
            t << endl << endl;
        }
    } else if(!project->isActiveConfig("staticlib")) {
        QString destdir = project->first("DESTDIR"), incr_deps;
        if(do_incremental) {
            QString s_ext = project->variables()["QMAKE_EXTENSION_SHLIB"].first();
            QString incr_target = var("QMAKE_ORIG_TARGET").replace(
                QRegExp("\\." + s_ext), "").replace(QRegExp("^lib"), "") + "_incremental";
            if(incr_target.indexOf(Option::dir_sep) != -1)
                incr_target = incr_target.right(incr_target.length() -
                                                (incr_target.lastIndexOf(Option::dir_sep) + 1));

            if(project->first("QMAKE_INCREMENTAL_STYLE") == "ld") {
                QString incr_target_dir = var("OBJECTS_DIR") + incr_target + Option::obj_ext;
                //actual target
                const QString link_deps = "$(OBJECTS) $(OBJMOC)";
                t << incr_target_dir << ": " << link_deps << "\n\t"
                  << "ld -r  -o " << incr_target_dir << " " << link_deps << endl;
                //communicated below
                QStringList &cmd = project->variables()["QMAKE_LINK_SHLIB_CMD"];
                cmd.first().replace("$(OBJECTS) $(OBJMOC)",
                                    "$(INCREMENTAL_OBJECTS) $(INCREMENTAL_OBJMOC)"); //ick
                cmd.append(incr_target_dir);
                deps.prepend(incr_target_dir + " ");
                incr_deps = "$(INCREMENTAL_OBJECTS) $(INCREMENTAL_OBJMOC)";
            } else {
                //actual target
                QString incr_target_dir = var("DESTDIR") + "lib" + incr_target + "." + s_ext;
                QString incr_lflags = var("QMAKE_LFLAGS_SHLIB") + " ";
                if(!project->isEmpty("QMAKE_LFLAGS_INCREMENTAL"))
                    incr_lflags += var("QMAKE_LFLAGS_INCREMENTAL") + " ";
                if(project->isActiveConfig("debug"))
                    incr_lflags += var("QMAKE_LFLAGS_DEBUG");
                else
                    incr_lflags += var("QMAKE_LFLAGS_RELEASE");
                t << incr_target_dir << ": $(INCREMENTAL_OBJECTS) $(INCREMENTAL_OBJMOC)" << "\n\t";
                if(!destdir.isEmpty())
                    t << "test -d " << destdir << " || mkdir -p " << destdir << "\n\t";
                t << "$(LINK) " << incr_lflags << " -o "<< incr_target_dir <<
                    " $(INCREMENTAL_OBJECTS) $(INCREMENTAL_OBJMOC)" << endl;
                //communicated below
                QStringList &cmd = project->variables()["QMAKE_LINK_SHLIB_CMD"];
                if(!destdir.isEmpty())
                    cmd.append(" -L" + destdir);
                cmd.append(" -l" + incr_target);
                deps.prepend(incr_target_dir + " ");
                incr_deps = "$(OBJECTS) $(OBJMOC)";
            }

            t << "all: " << " " << deps << " " << varGlue("ALL_DEPS",""," ","")
              << " " <<  var("DESTDIR_TARGET") << endl << endl;

            //real target
            t << var("DESTDIR_TARGET") << ": " << var("PRE_TARGETDEPS") << " "
              << incr_deps << " $(SUBLIBS) " << target_deps << " " << var("POST_TARGETDEPS");
        } else {
            t << "all: " << deps << " " << varGlue("ALL_DEPS",""," ","") << " " <<
                var("DESTDIR_TARGET") << endl << endl;
            t << var("DESTDIR_TARGET") << ": " << var("PRE_TARGETDEPS")
              << " $(OBJECTS) $(OBJMOC) $(SUBLIBS) $(OBJCOMP) " << target_deps
              << " " << var("POST_TARGETDEPS");
        }
        if(!destdir.isEmpty())
            t << "\n\t" << "test -d " << destdir << " || mkdir -p " << destdir;
        if(!project->isEmpty("QMAKE_PRE_LINK"))
            t << "\n\t" << var("QMAKE_PRE_LINK");

        if(project->isActiveConfig("compile_libtool")) {
            t << "\n\t"
              << var("QMAKE_LINK_SHLIB_CMD");
        } else if(project->isActiveConfig("plugin")) {
            t << "\n\t"
              << "-$(DEL_FILE) $(TARGET)" << "\n\t"
              << var("QMAKE_LINK_SHLIB_CMD");
            if(!destdir.isEmpty())
                t << "\n\t"
                  << "-$(MOVE) $(TARGET) " << var("DESTDIR");
            if(!project->isEmpty("QMAKE_POST_LINK"))
                t << "\n\t" << var("QMAKE_POST_LINK") << "\n\t";
            t << endl << endl;
        } else if(project->isEmpty("QMAKE_HPUX_SHLIB")) {
            t << "\n\t"
              << "-$(DEL_FILE) $(TARGET) $(TARGET0) $(TARGET1) $(TARGET2)" << "\n\t"
              << var("QMAKE_LINK_SHLIB_CMD") << "\n\t";
            t << varGlue("QMAKE_LN_SHLIB","-"," "," $(TARGET) $(TARGET0)")  << "\n\t"
              << varGlue("QMAKE_LN_SHLIB","-"," "," $(TARGET) $(TARGET1)") << "\n\t"
              << varGlue("QMAKE_LN_SHLIB","-"," "," $(TARGET) $(TARGET2)");
            if(!destdir.isEmpty())
                t << "\n\t"
                  << "-$(DEL_FILE) " << var("DESTDIR") << "$(TARGET)\n\t"
                  << "-$(DEL_FILE) " << var("DESTDIR") << "$(TARGET0)\n\t"
                  << "-$(DEL_FILE) " << var("DESTDIR") << "$(TARGET1)\n\t"
                  << "-$(DEL_FILE) " << var("DESTDIR") << "$(TARGET2)\n\t"
                  << "-$(MOVE) $(TARGET) $(TARGET0) $(TARGET1) $(TARGET2) " << var("DESTDIR");
            if(!project->isEmpty("QMAKE_POST_LINK"))
                t << "\n\t" << var("QMAKE_POST_LINK");
            t << endl << endl;
        } else {
            t << "\n\t"
              << "-$(DEL_FILE) $(TARGET) $(TARGET0)" << "\n\t"
              << var("QMAKE_LINK_SHLIB_CMD") << "\n\t";
            t << varGlue("QMAKE_LN_SHLIB",""," "," $(TARGET) $(TARGET0)");
            if(!destdir.isEmpty())
                t  << "\n\t"
                   << "-$(DEL_FILE) " << var("DESTDIR") << "$(TARGET)\n\t"
                   << "-$(DEL_FILE) " << var("DESTDIR") << "$(TARGET0)\n\t"
                   << "-$(MOVE) $(TARGET) $(TARGET0) " << var("DESTDIR");
            if(!project->isEmpty("QMAKE_POST_LINK"))
                t << "\n\t" << var("QMAKE_POST_LINK");
            t << endl << endl;
        }
        t << endl << endl;

        if (! project->isActiveConfig("plugin")) {
            t << "staticlib: $(TARGETA)" << endl << endl;
            t << "$(TARGETA): " << var("PRE_TARGETDEPS") << " $(OBJECTS) $(OBJMOC) $(OBJCOMP)";
            if(do_incremental)
                t << " $(INCREMENTAL_OBJECTS) $(INCREMENTAL_OBJMOC)";
            t << var("POST_TARGETDEPS") << "\n\t"
              << "-$(DEL_FILE) $(TARGETA) " << "\n\t"
              << var("QMAKE_AR_CMD");
            if(do_incremental)
                t << " $(INCREMENTAL_OBJECTS) $(INCREMENTAL_OBJMOC)";
            if(!project->isEmpty("QMAKE_RANLIB"))
                t << "\n\t" << "$(RANLIB) $(TARGETA)";
            t << endl << endl;
        }
    } else {
        t << "all: " << deps << " " << varGlue("ALL_DEPS",""," "," ") << var("DESTDIR") << "$(TARGET) "
          << varGlue("QMAKE_AR_SUBLIBS", var("DESTDIR"), " " + var("DESTDIR"), "") << "\n\n"
          << "staticlib: " << var("DESTDIR") << "$(TARGET)" << "\n\n";
        if(project->isEmpty("QMAKE_AR_SUBLIBS")) {
            t << var("DESTDIR") << "$(TARGET): " << var("PRE_TARGETDEPS")
              << " $(OBJECTS) $(OBJMOC) $(OBJCOMP) " << var("POST_TARGETDEPS") << "\n\t";
            if(!project->isEmpty("DESTDIR")) {
                QString destdir = project->first("DESTDIR");
                t << "test -d " << destdir << " || mkdir -p " << destdir << "\n\t";
            }
            t << "-$(DEL_FILE) $(TARGET)" << "\n\t"
              << var("QMAKE_AR_CMD") << "\n";
            if(!project->isEmpty("QMAKE_POST_LINK"))
                t << "\t" << var("QMAKE_POST_LINK") << "\n";
            if(!project->isEmpty("QMAKE_RANLIB"))
                t << "\t" << "$(RANLIB) $(TARGET)" << "\n";
            if(!project->isEmpty("DESTDIR"))
                t << "\t" << "-$(DEL_FILE) " << var("DESTDIR") << "$(TARGET)" << "\n"
                  << "\t" << "-$(MOVE) $(TARGET) " << var("DESTDIR") << "\n";
        } else {
            int max_files = project->first("QMAKE_MAX_FILES_PER_AR").toInt();
            QStringList objs = project->variables()["OBJECTS"] + project->variables()["OBJMOC"] +
                               project->variables()["OBJCOMP"],
                        libs = project->variables()["QMAKE_AR_SUBLIBS"];
            libs.prepend("$(TARGET)");
            for(QStringList::Iterator libit = libs.begin(), objit = objs.begin();
                libit != libs.end(); ++libit) {
                QStringList build;
                for(int cnt = 0; cnt < max_files && objit != objs.end(); ++objit, cnt++)
                    build << (*objit);
                QString ar;
                if((*libit) == "$(TARGET)") {
                    t << var("DESTDIR") << "$(TARGET): " << var("PRE_TARGETDEPS")
                      << " " << var("POST_TARGETDEPS") << valList(build) << "\n\t";
                    ar = project->variables()["QMAKE_AR_CMD"].first();
                    ar = ar.replace("$(OBJMOC)", "").replace("$(OBJECTS)",
                                                             build.join(" "));
                } else {
                    t << (*libit) << ": " << valList(build) << "\n\t";
                    ar = "$(AR) " + (*libit) + " " + build.join(" ");
                }
                if(!project->isEmpty("DESTDIR")) {
                    QString destdir = project->first("DESTDIR");
                    t << "test -d " << destdir << " || mkdir -p " << destdir << "\n\t";
                }
                t << "-$(DEL_FILE) " << (*libit) << "\n\t"
                  << ar << "\n";
                if(!project->isEmpty("QMAKE_POST_LINK"))
                    t << "\t" << var("QMAKE_POST_LINK") << "\n";
                if(!project->isEmpty("QMAKE_RANLIB"))
                    t << "\t" << "$(RANLIB) " << (*libit) << "\n";
                if(!project->isEmpty("DESTDIR"))
                    t << "\t" << "-$(DEL_FILE) " << var("DESTDIR") << (*libit) << "\n"
                      << "\t" << "-$(MOVE) " << (*libit) << " " << var("DESTDIR") << "\n";
            }
        }
        t << endl << endl;
    }

    t << "mocables: $(SRCMOC)" << endl;

    if(!project->isActiveConfig("no_mocdepend")) {
        //this is an implicity depend on moc, so it will be built if necesary, however
        //moc itself shouldn't have this dependency - this is a little kludgy but it is
        //better than the alternative for now.
        QString moc = project->first("QMAKE_MOC"), target = project->first("TARGET"),
            moc_dir = "$(QTDIR)/src/moc";
        if(!project->isEmpty("QMAKE_MOC_SRC"))
            moc_dir = project->first("QMAKE_MOC_SRC");
        fixEnvVariables(target);
        fixEnvVariables(moc);
        if(target != moc)
            t << "$(MOC): \n\t"
              << "(cd " << moc_dir << " && $(MAKE))"  << endl << endl;
    }

    writeMakeQmake(t);
    if(project->isEmpty("QMAKE_FAILED_REQUIREMENTS") && !project->isActiveConfig("no_autoqmake")) {
        QString meta_files;
        if(project->isActiveConfig("create_libtool") && project->first("TEMPLATE") == "lib" &&
           !project->isActiveConfig("compile_libtool")) { //libtool
            if(!meta_files.isEmpty())
                meta_files += " ";
            meta_files += libtoolFileName();
        }
        if(project->isActiveConfig("create_pc") && project->first("TEMPLATE") == "lib") { //pkg-config
            if(!meta_files.isEmpty())
                meta_files += " ";
            meta_files += pkgConfigFileName();
        }
        if(!meta_files.isEmpty()) {
            QStringList files = fileFixify(Option::mkfile::project_files);
            t << meta_files << ": " << "\n\t"
              << "@$(QMAKE) -prl " << buildArgs() << " " << files.join(" ") << endl;
        }
    }

    if(!project->first("QMAKE_PKGINFO").isEmpty()) {
        QString pkginfo = project->first("QMAKE_PKGINFO");
        QString destdir = project->first("DESTDIR");
        t << pkginfo << ": " << "\n\t";
        if(!destdir.isEmpty())
            t << "@test -d " << destdir << " || mkdir -p " << destdir << "\n\t";
        t << "@$(DEL_FILE) " << pkginfo << "\n\t"
          << "@echo \"APPL????\" >" << pkginfo << endl;
    }
    if(!project->isEmpty("QMAKE_INFO_PLIST")) {
        //copy the plist
        QString info_plist = project->first("QMAKE_INFO_PLIST"),
            info_plist_out = project->first("QMAKE_INFO_PLIST_OUT");
        QString destdir = project->first("DESTDIR");
        t << info_plist_out << ": " << "\n\t";
        if(!destdir.isEmpty())
            t << "@test -d " << destdir << " || mkdir -p " << destdir << "\n\t";
        t << "@$(DEL_FILE) " << info_plist_out << "\n\t"
          << "@sed -e \"s,@ICON@,application.icns,g\" -e \"s,@EXECUTABLE@," << var("QMAKE_ORIG_TARGET")
          << ",g\" \"" << info_plist << "\" >\"" << info_plist_out << "\"" << endl;
        //copy the icon
        if(!project->isEmpty("ICON")) {
            QString dir = project->first("DESTDIR") + "../Resources/";
            t << dir << "application.icns: " << fileFixify(var("ICON")) << "\n\t"
              << "@test -d " << dir << " || mkdir -p " << dir << "\n\t"
              << "@$(DEL_FILE) " << dir << "application.icns" << "\n\t"
              << "@$(COPY_FILE) " << fileFixify(var("ICON")) << " " << dir << "application.icns" << endl;
        }
        //copy other data
        if(!project->isEmpty("QMAKE_BUNDLE_DATA")) {
            const QStringList &bundle_data = project->variables()["QMAKE_BUNDLE_DATA"];
            for(int i = 0; i < bundle_data.count(); i++) {
                const QStringList &files = project->variables()[bundle_data[i] + ".files"];
                QString path = Option::fixPathToTargetOS(project->first("DESTDIR") +
                                                         "../" + project->first(bundle_data[i] + ".path"));
                for(int file = 0; file < files.count(); file++) {
                    QString dst = path + Option::dir_sep + QFileInfo(files[file]).fileName();
                    t << dst << ": " << files[file] << "\n\t"
                      << "@test -d " << path << " || mkdir -p " << path << "\n\t"
                      << "@$(DEL_FILE) " << dst << "\n\t"
                      << "@$(COPY_FILE) " << files[file] << " " << dst << endl;
                }
            }
        }
    }

    QString ddir;
    if (project->isEmpty("QMAKE_DISTDIR"))
        ddir = project->first("QMAKE_ORIG_TARGET");
    else
        ddir = project->first("QMAKE_DISTDIR");

    QString ddir_c = fileFixify((project->isEmpty("OBJECTS_DIR") ? QString(".tmp/") :
                                 project->first("OBJECTS_DIR")) + ddir);
    t << "dist: " << "\n\t"
      << "@mkdir -p " << ddir_c << " && "
      << "$(COPY_FILE) --parents $(SOURCES) $(HEADERS) $(FORMS) $(DIST) " << ddir_c << Option::dir_sep << " && ";
    if(!project->isEmpty("TRANSLATIONS"))
        t << "$(COPY_FILE) --parents " << var("TRANSLATIONS") << " " << ddir_c << Option::dir_sep << " && ";
    if(!project->isEmpty("IMAGES"))
        t << "$(COPY_FILE) --parents " << var("IMAGES") << " " << ddir_c << Option::dir_sep << " && ";
    if(!project->isEmpty("FORMS")) {
        QStringList &forms = project->variables()["FORMS"], ui_headers;
        for(QStringList::Iterator formit = forms.begin(); formit != forms.end(); ++formit) {
            QString ui_h = fileFixify((*formit) + Option::h_ext.first());
            if(QFile::exists(ui_h))
               ui_headers << ui_h;
        }
        if(!ui_headers.isEmpty())
            t << "$(COPY_FILE) --parents " << val(ui_headers) << " " << ddir_c << Option::dir_sep << " && ";
    }
    t << "(cd `dirname " << ddir_c << "` && "
      << "$(TAR) " << var("QMAKE_ORIG_TARGET") << ".tar " << ddir << " && "
      << "$(COMPRESS) " << var("QMAKE_ORIG_TARGET") << ".tar) && "
      << "$(MOVE) `dirname " << ddir_c << "`" << Option::dir_sep << var("QMAKE_ORIG_TARGET") << ".tar.gz . && "
      << "$(DEL_FILE) -r " << ddir_c
      << endl << endl;

    QString clean_targets = "compiler_clean";
    t << "mocclean:" << "\n";
    if(mocAware()) {
        if(!objMoc.isEmpty() || !srcMoc.isEmpty() || moc_incremental) {
            if(!objMoc.isEmpty())
                t << "\t-$(DEL_FILE) $(OBJMOC)" << '\n';
            if(!srcMoc.isEmpty())
                t << "\t-$(DEL_FILE) $(SRCMOC)" << '\n';
            if(moc_incremental)
                t << "\t-$(DEL_FILE) $(INCREMENTAL_OBJMOC)" << '\n';
            clean_targets += " mocclean";
        }
        t << endl;
    }
    t << endl;

    t << "yaccclean:" << "\n";
    if(!var("YACCSOURCES").isEmpty()) {
        QStringList clean, &l = project->variables()["YACCSOURCES"];
        for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
            QFileInfo fi((*it));
            QString dir;
            if(fi.path() != ".")
                dir = fi.path() + Option::dir_sep;
            dir = fileFixify(dir, QDir::currentPath(), Option::output_dir);
            if(!dir.isEmpty() && dir.right(Option::dir_sep.length()) != Option::dir_sep)
                dir += Option::dir_sep;
            clean << (dir + fi.completeBaseName() + Option::yacc_mod + Option::cpp_ext.first());
            clean << (dir + fi.completeBaseName() + Option::yacc_mod + Option::h_ext.first());
        }
        if(!clean.isEmpty()) {
            t << "\t-$(DEL_FILE) " << clean.join(" ") << "\n";
            clean_targets += " yaccclean";
        }
    }

    t << "lexclean:" << "\n";
    if(!var("LEXSOURCES").isEmpty()) {
        QStringList clean, &l = project->variables()["LEXSOURCES"];
        for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
            QFileInfo fi((*it));
            QString dir;
            if(fi.path() != ".")
                dir = fi.path() + Option::dir_sep;
            dir = fileFixify(dir, QDir::currentPath(), Option::output_dir);
            if(!dir.isEmpty() && dir.right(Option::dir_sep.length()) != Option::dir_sep)
                dir += Option::dir_sep;
            clean << (dir + fi.completeBaseName() + Option::lex_mod + Option::cpp_ext.first());
        }
        if(!clean.isEmpty()) {
            t << "\t-$(DEL_FILE) " << clean.join(" ") << "\n";
            clean_targets += " lexclean";
        }
    }

    if(do_incremental) {
        t << "incrclean:" << "\n";
        if(src_incremental)
            t << "\t-$(DEL_FILE) $(INCREMENTAL_OBJECTS)" << "\n";
        if(moc_incremental)
            t << "\t-$(DEL_FILE) $(INCREMENTAL_OBJMOC)" << '\n';
        t << endl;
    }

    t << "clean:" << clean_targets << "\n\t";
    if(!project->isEmpty("OBJECTS")) {
        if(project->isActiveConfig("compile_libtool"))
            t << "-$(LIBTOOL) --mode=clean $(DEL_FILE) $(OBJECTS)" << "\n\t";
        else
            t << "-$(DEL_FILE) $(OBJECTS)" << "\n\t";
    }
    if(doPrecompiledHeaders() && !project->isEmpty("PRECOMPILED_HEADER")) {
        QString header_prefix = project->first("QMAKE_PRECOMP_PREFIX");
        QString precomph_out_dir = project->first("QMAKE_ORIG_TARGET") + ".gch" + Option::dir_sep;
        t << "-$(DEL_FILE) " << precomph_out_dir << header_prefix + "c "
          << precomph_out_dir << header_prefix << "c++" << "\n\t";
    }
    if(!project->isEmpty("IMAGES"))
        t << varGlue("QMAKE_IMAGE_COLLECTION", "\t-$(DEL_FILE) ", " ", "") << "\n\t";
    if(src_incremental)
        t << "-$(DEL_FILE) $(INCREMENTAL_OBJECTS)" << "\n\t";
    t << varGlue("QMAKE_CLEAN","-$(DEL_FILE) "," ","\n\t")
      << "-$(DEL_FILE) *~ core *.core" << "\n"
      << varGlue("CLEAN_FILES","\t-$(DEL_FILE) "," ","") << endl << endl;
    t << "####### Sub-libraries" << endl << endl;
    if (!project->variables()["SUBLIBS"].isEmpty()) {
        QString libdir = "tmp/";
        if(!project->isEmpty("SUBLIBS_DIR"))
            libdir = project->first("SUBLIBS_DIR");
        QStringList &l = project->variables()["SUBLIBS"];
        for(it = l.begin(); it != l.end(); ++it)
            t << libdir << "lib" << (*it) << ".a" << ":\n\t"
              << var(QString("MAKELIB") + (*it)) << endl << endl;
    }

    QString destdir = project->first("DESTDIR");
    if(!destdir.isEmpty() && destdir.right(1) != Option::dir_sep)
        destdir += Option::dir_sep;
    t << "distclean: " << "clean\n";
    if(project->first("TEMPLATE") == "app" && project->isActiveConfig("resource_fork"))
        t << "\t-$(DEL_FILE) -r " << destdir.section(Option::dir_sep, 0, -4) << endl;
    else if(project->isActiveConfig("compile_libtool"))
        t << "\t-$(LIBTOOL) --mode=clean $(DEL_FILE) " << "$(TARGET)" << endl;
    else
        t << "\t-$(DEL_FILE) " << destdir << "$(TARGET)" << " " << endl;
    if(!project->isActiveConfig("staticlib") && project->variables()["QMAKE_APP_FLAG"].isEmpty() &&
       !project->isActiveConfig("plugin") && !project->isActiveConfig("compile_libtool"))
        t << "\t-$(DEL_FILE) " << destdir << "$(TARGET0) " << destdir << "$(TARGET1) "
          << destdir << "$(TARGET2) $(TARGETA)" << endl;
    {
        QString ofile = Option::fixPathToTargetOS(fileFixify(Option::output.fileName()));
        if(!ofile.isEmpty())
            t << "\t-$(DEL_FILE) " << ofile << endl;
    }
    t << endl << endl;

    if(doPrecompiledHeaders() && !project->isEmpty("PRECOMPILED_HEADER")) {
        QString precomph = fileFixify(project->first("PRECOMPILED_HEADER"));
        t << "###### Prefix headers" << endl;
        QString comps[] = { "C", "CXX", QString::null };
        for(int i = 0; !comps[i].isNull(); i++) {
            QString flags = var("QMAKE_" + comps[i] + "FLAGS_PRECOMPILE");
            flags += " $(" + comps[i] + "FLAGS)";

            QString header_prefix = project->first("QMAKE_PRECOMP_PREFIX");
            QString outdir = project->first("QMAKE_ORIG_TARGET") + ".gch" + Option::dir_sep, outfile = outdir;
            QString compiler;
            if(comps[i] == "C") {
                outfile += header_prefix + "c";
                compiler = "$(CC) ";
            } else {
                outfile += header_prefix + "c++";
                compiler = "$(CXX) ";
            }
            t << outfile << ": " << precomph << " " << findDependencies(precomph).join(" \\\n\t\t")
              << "\n\t" << "test -d " << outdir << " || mkdir -p " << outdir
              << "\n\t" << compiler << flags << " $(INCPATH) " << precomph << " -o " << outfile << endl << endl;
        }
    }
    if(!project->isEmpty("ALLMOC_HEADER")) {
        QString outdir = project->first("MOC_DIR");
        QString precomph = fileFixify(project->first("ALLMOC_HEADER"));
        t << "###### Combined headers" << endl << endl
          << outdir << "allmoc.cpp: " << precomph << " "
          << varList("HEADERS_ORIG") << "\n\t"
          << "echo '#include \"" << precomph << "\"' >" << outdir << "allmoc.cpp" << "\n\t"
          << "$(CXX) -E -DQT_MOC_CPP -DQT_NO_STL $(CXXFLAGS) $(INCPATH) >" << outdir << "allmoc.h "
          << outdir << "allmoc.cpp" << "\n\t"
          << "$(MOC) $(DEFINES) -o " << outdir << "allmoc.cpp " << outdir << "allmoc.h" << "\n\t"
          << "perl -pi -e 's{#include \"allmoc.h\"}{#define QT_H_CPP\\n#include \""
          << precomph << "\"}' " << outdir << "allmoc.cpp" << "\n\t"
          << "$(DEL_FILE) " << outdir << "allmoc.h" << endl << endl;
    }

    writeExtraTargets(t);
    writeExtraCompilerTargets(t);
    t <<"FORCE:" << endl << endl;
}

void UnixMakefileGenerator::init2()
{
    //version handling
    if(project->variables()["VERSION"].isEmpty())
        project->variables()["VERSION"].append("1.0." +
                                               (project->isEmpty("VER_PAT") ? QString("0") :
                                                project->first("VER_PAT")));
    QStringList l = project->first("VERSION").split('.');
    l << "0" << "0"; //make sure there are three
    project->variables()["VER_MAJ"].append(l[0]);
    project->variables()["VER_MIN"].append(l[1]);
    project->variables()["VER_PAT"].append(l[2]);

    if (!project->variables()["QMAKE_APP_FLAG"].isEmpty()) {
#if 0
        if (project->isActiveConfig("dll")) {
            project->variables()["TARGET"] += project->variables()["TARGET.so"];
            if(project->variables()["QMAKE_LFLAGS_SHAPP"].isEmpty())
                project->variables()["QMAKE_LFLAGS_SHAPP"] += project->variables()["QMAKE_LFLAGS_SHLIB"];
            if(!project->variables()["QMAKE_LFLAGS_SONAME"].isEmpty())
                project->variables()["QMAKE_LFLAGS_SONAME"].first() += project->first("TARGET");
        }
#endif
        if(!project->isEmpty("TARGET"))
            project->variables()["TARGET"].first().prepend(project->first("DESTDIR"));
       if (!project->variables()["QMAKE_CYGWIN_EXE"].isEmpty())
            project->variables()["TARGET_EXT"].append(".exe");
    } else if (project->isActiveConfig("staticlib")) {
        project->variables()["TARGET"].first().prepend("lib");
        project->variables()["TARGET"].first() += ".a";
        if(project->variables()["QMAKE_AR_CMD"].isEmpty())
            project->variables()["QMAKE_AR_CMD"].append("$(AR) $(TARGET) $(OBJECTS) $(OBJMOC)");
    } else {
        project->variables()["TARGETA"].append(project->first("DESTDIR") + "lib" + project->first("TARGET") + ".a");
        if(project->isActiveConfig("compile_libtool"))
            project->variables()["TARGET_la"] = QStringList(project->first("DESTDIR") + "lib" + project->first("TARGET") + Option::libtool_ext);

        if (!project->variables()["QMAKE_AR_CMD"].isEmpty())
            project->variables()["QMAKE_AR_CMD"].first().replace("(TARGET)","(TARGETA)");
        else
            project->variables()["QMAKE_AR_CMD"].append("$(AR) $(TARGETA) $(OBJECTS) $(OBJMOC)");
        if(project->isActiveConfig("compile_libtool")) {
            project->variables()["TARGET"] = project->variables()["TARGET_la"];
        } else if(project->isActiveConfig("plugin")) {
            QString prefix;
            if(!project->isActiveConfig("no_plugin_name_prefix"))
                prefix = "lib";
            project->variables()["TARGET_x.y.z"].append(prefix +
                                                        project->first("TARGET") + "." +
                                                        project->first("QMAKE_EXTENSION_PLUGIN"));
            if(project->isActiveConfig("lib_version_first"))
                project->variables()["TARGET_x"].append(prefix + project->first("TARGET") + "." +
                                                        project->first("VER_MAJ") + "." +
                                                        project->first("QMAKE_EXTENSION_PLUGIN"));
            else
                project->variables()["TARGET_x"].append(prefix + project->first("TARGET") + "." +
                                                        project->first("QMAKE_EXTENSION_PLUGIN") +
                                                        "." + project->first("VER_MAJ"));
            project->variables()["TARGET"] = project->variables()["TARGET_x.y.z"];
        } else if (!project->isEmpty("QMAKE_HPUX_SHLIB")) {
            project->variables()["TARGET_"].append("lib" + project->first("TARGET") + ".sl");
            if(project->isActiveConfig("lib_version_first"))
                project->variables()["TARGET_x"].append("lib" + project->first("VER_MAJ") + "." +
                                                        project->first("TARGET"));
            else
                project->variables()["TARGET_x"].append("lib" + project->first("TARGET") + "." +
                                                        project->first("VER_MAJ"));
            project->variables()["TARGET"] = project->variables()["TARGET_x"];
        } else if (!project->isEmpty("QMAKE_AIX_SHLIB")) {
            project->variables()["TARGET_"].append("lib" + project->first("TARGET") + ".a");
            if(project->isActiveConfig("lib_version_first")) {
                project->variables()["TARGET_x"].append("lib" + project->first("TARGET") + "." +
                                                        project->first("VER_MAJ") + "." +
                                                        project->first("QMAKE_EXTENSION_SHLIB"));
                project->variables()["TARGET_x.y"].append("lib" + project->first("TARGET") + "." +
                                                          project->first("VER_MAJ") +
                                                          "." + project->first("VER_MIN") + "." +
                                                          project->first("QMAKE_EXTENSION_SHLIB"));
                project->variables()["TARGET_x.y.z"].append("lib" + project->first("TARGET") + "." +
                                                            project->first("VER_MAJ") + "." +
                                                            project->first("VER_MIN") + "." +
                                                            project->first("VER_PAT") + "." +
                                                            project->first("QMAKE_EXTENSION_SHLIB"));
            } else {
                project->variables()["TARGET_x"].append("lib" + project->first("TARGET") + "." +
                                                        project->first("QMAKE_EXTENSION_SHLIB") +
                                                        "." + project->first("VER_MAJ"));
                project->variables()["TARGET_x.y"].append("lib" + project->first("TARGET") + "." +
                                                          project->first("QMAKE_EXTENSION_SHLIB") +
                                                          "." + project->first("VER_MAJ") +
                                                          "." + project->first("VER_MIN"));
                project->variables()["TARGET_x.y.z"].append("lib" + project->first("TARGET") + "." +
                                                            project->first("QMAKE_EXTENSION_SHLIB") + "." +
                                                            project->first("VER_MAJ") + "." +
                                                            project->first("VER_MIN") + "." +
                                                            project->first("VER_PAT"));
            }
            project->variables()["TARGET"] = project->variables()["TARGET_x.y.z"];
        } else {
            project->variables()["TARGET_"].append("lib" + project->first("TARGET") + "." +
                                                   project->first("QMAKE_EXTENSION_SHLIB"));
            if(project->isActiveConfig("lib_version_first")) {
                project->variables()["TARGET_x"].append("lib" + project->first("TARGET") + "." +
                                                        project->first("VER_MAJ") + "." +
                                                        project->first("QMAKE_EXTENSION_SHLIB"));
                project->variables()["TARGET_x.y"].append("lib" + project->first("TARGET") + "." +
                                                          project->first("VER_MAJ") +
                                                          "." + project->first("VER_MIN") + "." +
                                                          project->first("QMAKE_EXTENSION_SHLIB"));
                project->variables()["TARGET_x.y.z"].append("lib" + project->first("TARGET") + "." +
                                                            project->first("VER_MAJ") + "." +
                                                            project->first("VER_MIN") +  "." +
                                                            project->first("VER_PAT") + "." +
                                                            project->variables()["QMAKE_EXTENSION_SHLIB"].first());
            } else {
                project->variables()["TARGET_x"].append("lib" + project->first("TARGET") + "." +
                                                        project->first("QMAKE_EXTENSION_SHLIB") +
                                                        "." + project->first("VER_MAJ"));
                project->variables()["TARGET_x.y"].append("lib" + project->first("TARGET") + "." +
                                                      project->first("QMAKE_EXTENSION_SHLIB")
                                                      + "." + project->first("VER_MAJ") +
                                                      "." + project->first("VER_MIN"));
                project->variables()["TARGET_x.y.z"].append("lib" + project->first("TARGET") +
                                                            "." +
                                                            project->variables()[
                                                                "QMAKE_EXTENSION_SHLIB"].first() + "." +
                                                            project->first("VER_MAJ") + "." +
                                                            project->first("VER_MIN") +  "." +
                                                            project->first("VER_PAT"));
            }
            project->variables()["TARGET"] = project->variables()["TARGET_x.y.z"];
        }
        if(project->isEmpty("QMAKE_LN_SHLIB"))
            project->variables()["QMAKE_LN_SHLIB"].append("ln -s");
        project->variables()["DESTDIR_TARGET"].append("$(TARGET)");
        if (!project->variables()["DESTDIR"].isEmpty())
            project->variables()["DESTDIR_TARGET"].first().prepend(project->first("DESTDIR"));
        if (!project->variables()["QMAKE_LFLAGS_SONAME"].isEmpty()) {
            if(project->isActiveConfig("plugin")) {
                if(!project->variables()["TARGET"].isEmpty())
                    project->variables()["QMAKE_LFLAGS_SONAME"].first() += project->first("TARGET");
            } else {
                if(!project->variables()["TARGET_x"].isEmpty())
                    project->variables()["QMAKE_LFLAGS_SONAME"].first() += project->first("TARGET_x");
            }
        }
        if (project->variables()["QMAKE_LINK_SHLIB_CMD"].isEmpty())
            project->variables()["QMAKE_LINK_SHLIB_CMD"].append(
                "$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(OBJMOC) $(LIBS) $(OBJCOMP)");
    }
    if(project->isEmpty("QMAKE_SYMBOLIC_LINK"))
        project->variables()["QMAKE_SYMBOLIC_LINK"].append("ln -sf");
    if (!project->variables()["QMAKE_APP_FLAG"].isEmpty()) {
        project->variables()["QMAKE_CFLAGS"] += project->variables()["QMAKE_CFLAGS_APP"];
        project->variables()["QMAKE_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS_APP"];
        project->variables()["QMAKE_LFLAGS"] += project->variables()["QMAKE_LFLAGS_APP"];
    } else if (project->isActiveConfig("dll")) {
        if(!project->isActiveConfig("plugin") || !project->isActiveConfig("plugin_no_share_shlib_cflags")) {
            project->variables()["QMAKE_CFLAGS"] += project->variables()["QMAKE_CFLAGS_SHLIB"];
            project->variables()["QMAKE_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS_SHLIB"];
        }
        if (project->isActiveConfig("plugin")) {
            project->variables()["QMAKE_CFLAGS"] += project->variables()["QMAKE_CFLAGS_PLUGIN"];
            project->variables()["QMAKE_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS_PLUGIN"];
            project->variables()["QMAKE_LFLAGS"] += project->variables()["QMAKE_LFLAGS_PLUGIN"];
            if(project->isActiveConfig("plugin_with_soname") && !project->isActiveConfig("compile_libtool"))
                project->variables()["QMAKE_LFLAGS"] += project->variables()["QMAKE_LFLAGS_SONAME"];
        } else {
            project->variables()["QMAKE_LFLAGS"] += project->variables()["QMAKE_LFLAGS_SHLIB"];
            if(!project->isEmpty("QMAKE_LFLAGS_COMPAT_VERSION")) {
                if(project->isEmpty("COMPAT_VERSION"))
                    project->variables()["QMAKE_LFLAGS"] += QString(project->first("QMAKE_LFLAGS_COMPAT_VERSION") +
                                                                    project->first("VER_MAJ") + "." +
                                                                    project->first("VER_MIN"));
                else
                    project->variables()["QMAKE_LFLAGS"] += QString(project->first("QMAKE_LFLAGS_COMPAT_VERSION") +
                                                                    project->first("COMPATIBILITY_VERSION"));
            }
            if(!project->isEmpty("QMAKE_LFLAGS_VERSION")) {
                project->variables()["QMAKE_LFLAGS"] += QString(project->first("QMAKE_LFLAGS_VERSION") +
                                                                project->first("VER_MAJ") + "." +
                                                                project->first("VER_MIN") + "." +
                                                                project->first("VER_PAT"));
            }
            if(!project->isActiveConfig("compile_libtool"))
                project->variables()["QMAKE_LFLAGS"] += project->variables()["QMAKE_LFLAGS_SONAME"];
        }
        QString destdir = project->first("DESTDIR");
        if (!destdir.isEmpty() && !project->variables()["QMAKE_LFLAGS_RPATH"].isEmpty()) {
            QString rpath_destdir = destdir;
            if(QDir::isRelativePath(rpath_destdir)) {
                QFileInfo fi(Option::fixPathToLocalOS(rpath_destdir));
                if(fi.makeAbsolute())  //strange, shouldn't really happen
                    rpath_destdir = Option::fixPathToTargetOS(rpath_destdir, false);
                else
                    rpath_destdir = fi.filePath();
            } else {
                rpath_destdir = Option::fixPathToTargetOS(rpath_destdir, false);
            }
            project->variables()["QMAKE_LFLAGS"] += project->first("QMAKE_LFLAGS_RPATH") + rpath_destdir;
        }
    }
}

QString
UnixMakefileGenerator::libtoolFileName()
{
    QString ret = var("TARGET");
    int slsh = ret.lastIndexOf(Option::dir_sep);
    if(slsh != -1)
        ret = ret.right(ret.length() - slsh);
    int dot = ret.indexOf('.');
    if(dot != -1)
        ret = ret.left(dot);
    ret += Option::libtool_ext;
    if(!project->isEmpty("DESTDIR")) {
        ret.prepend(var("DESTDIR"));
        ret = Option::fixPathToLocalOS(fileFixify(ret, QDir::currentPath(), Option::output_dir));
    }
    return ret;
}

void
UnixMakefileGenerator::writeLibtoolFile()
{
    QString fname = libtoolFileName(), lname = fname;
    int slsh = lname.lastIndexOf(Option::dir_sep);
    if(slsh != -1)
        lname = lname.right(lname.length() - slsh - 1);
    QFile ft(fname);
    if(!ft.open(QIODevice::WriteOnly))
        return;
    project->variables()["ALL_DEPS"].append(fileFixify(fname));

    QTextStream t(&ft);
    t << "# " << lname << " - a libtool library file\n";
    time_t now = time(NULL);
    t << "# Generated by qmake/libtool (" << qmake_version() << ") (Qt "
      << QT_VERSION_STR << ") on: " << ctime(&now) << "\n";

    t << "# The name that we can dlopen(3).\n"
      << "dlname='" << var(project->isActiveConfig("plugin") ? "TARGET" : "TARGET_x")
      << "'\n\n";

    t << "# Names of this library.\n";
    t << "library_names='";
    if(project->isActiveConfig("plugin")) {
        t << var("TARGET");
    } else {
        if (project->isEmpty("QMAKE_HPUX_SHLIB"))
            t << var("TARGET_x.y.z") << " ";
        t << var("TARGET_x") << " " << var("TARGET_");
    }
    t << "'\n\n";

    t << "# The name of the static archive.\n"
      << "old_library='" << lname.left(lname.length()-Option::libtool_ext.length()) << ".a'\n\n";

    t << "# Libraries that this one depends upon.\n";
    QStringList libs;
    if(!project->isEmpty("QMAKE_INTERNAL_PRL_LIBS"))
        libs = project->variables()["QMAKE_INTERNAL_PRL_LIBS"];
    else
        libs << "QMAKE_LIBS"; //obvious one
    t << "dependency_libs='";
    for(QStringList::ConstIterator it = libs.begin(); it != libs.end(); ++it)
        t << project->variables()[(*it)].join(" ") << " ";
    t << "'\n\n";

    t << "# Version information for " << lname << "\n";
    int maj = project->first("VER_MAJ").toInt();
    int min = project->first("VER_MIN").toInt();
    int pat = project->first("VER_PAT").toInt();
    t << "current=" << (10*maj + min) << "\n" // best I can think of
      << "age=0\n"
      << "revision=" << pat << "\n\n";

    t << "# Is this an already installed library.\n"
        "installed=yes\n\n"; // ###

    t << "# Files to dlopen/dlpreopen.\n"
        "dlopen=''\n"
        "dlpreopen=''\n\n";

    QString install_dir = project->first("target.path");
    if(install_dir.isEmpty())
        install_dir = project->first("DESTDIR");
    t << "# Directory that this library needs to be installed in:\n"
        "libdir='" << Option::fixPathToTargetOS(install_dir, false) << "'\n";
}

QString
UnixMakefileGenerator::pkgConfigFileName()
{
    QString ret = var("TARGET");
    int slsh = ret.lastIndexOf(Option::dir_sep);
    if(slsh != -1)
        ret = ret.right(ret.length() - slsh);
    if(ret.startsWith("lib"))
        ret = ret.mid(3);
    int dot = ret.indexOf('.');
    if(dot != -1)
        ret = ret.left(dot);
    ret += Option::pkgcfg_ext;
    if(!project->isEmpty("DESTDIR")) {
        ret.prepend(var("DESTDIR"));
        ret = Option::fixPathToLocalOS(fileFixify(ret, QDir::currentPath(), Option::output_dir));
    }
    return ret;
}

QString
UnixMakefileGenerator::pkgConfigPrefix() const
{
    if(!project->isEmpty("QMAKE_PKGCONFIG_PREFIX"))
        return project->first("QMAKE_PKGCONFIG_PREFIX");
    return qInstallPath();
}

QString
UnixMakefileGenerator::pkgConfigFixPath(QString path) const
{
    QString prefix = pkgConfigPrefix();
    if(path.startsWith(prefix))
        path = path.replace(prefix, "${prefix}");
    return path;
}

void
UnixMakefileGenerator::writePkgConfigFile()     // ### does make sense only for libqt so far
{
    QString fname = pkgConfigFileName(), lname = fname;
    int slsh = lname.lastIndexOf(Option::dir_sep);
    if(slsh != -1)
        lname = lname.right(lname.length() - slsh - 1);
    QFile ft(fname);
    if(!ft.open(QIODevice::WriteOnly))
        return;
    project->variables()["ALL_DEPS"].append(fileFixify(fname));
    QTextStream t(&ft);

    QString prefix = pkgConfigPrefix();
    QString libDir = project->first("QMAKE_PKGCONFIG_LIBDIR");
    if(libDir.isEmpty())
        libDir = prefix + Option::dir_sep + "lib" + Option::dir_sep;
    QString includeDir = project->first("QMAKE_PKGCONFIG_INCDIR");
    if(includeDir.isEmpty())
        includeDir = prefix + "/include";

    t << "prefix=" << prefix << endl;
    t << "exec_prefix=${prefix}\n"
      << "libdir=" << pkgConfigFixPath(libDir) << "\n"
      << "includedir=" << pkgConfigFixPath(includeDir) << endl;
    // non-standard entry. Provides useful info normally only
    // contained in the internal .qmake.cache file
    t << varGlue("CONFIG", "qt_config=", " ", "") << endl << endl;

    QString name = project->first("QMAKE_PKGCONFIG_NAME");
    if(name.isEmpty()) {
        name = project->first("QMAKE_ORIG_TARGET").toLower();
        name.replace(0, 1, name[0].toUpper());
    }
    t << "Name: " << name << endl;
    QString desc = project->first("QMAKE_PKGCONFIG_DESCRIPTION");
    if(desc.isEmpty()) {
        if(name.isEmpty()) {
            desc = project->first("QMAKE_ORIG_TARGET").toLower();
            desc.replace(0, 1, desc[0].toUpper());
        } else {
            desc = name;
        }
        if(project->first("TEMPLATE") == "lib") {
            if(project->isActiveConfig("plugin"))
               desc += " Plugin";
            else
               desc += " Library";
        } else if(project->first("TEMPLATE") == "app") {
            desc += " Application";
        }
    }
    t << "Description: " << desc << endl;
    t << "Version: " << project->first("VERSION") << endl;

    // libs
    QStringList libs;
    if(!project->isEmpty("QMAKE_INTERNAL_PRL_LIBS"))
        libs = project->variables()["QMAKE_INTERNAL_PRL_LIBS"];
    else
        libs << "QMAKE_LIBS"; //obvious one
    libs << "QMAKE_LFLAGS_THREAD"; //not sure about this one, but what about things like -pthread?
    t << "Libs: -L${libdir} -l" << lname.left(lname.length()-Option::libtool_ext.length()) << " ";
    for(QStringList::ConstIterator it = libs.begin(); it != libs.end(); ++it)
        t << project->variables()[(*it)].join(" ") << " ";
    t << endl;

    // flags
    // ### too many
    t << "Cflags: "
        // << var("QMAKE_CXXFLAGS") << " "
      << varGlue("PRL_EXPORT_DEFINES","-D"," -D"," ")
      << project->variables()["PRL_EXPORT_CXXFLAGS"].join(" ")
        //      << varGlue("DEFINES","-D"," -D"," ")
      << " -I${includedir}";
}
