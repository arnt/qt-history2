/****************************************************************************
**
** Implementation of Win32MakefileGenerator class.
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

#include "winmakefile.h"
#include "option.h"
#include "project.h"
#include "meta.h"
#include <qtextstream.h>
#include <qstring.h>
#include <qhash.h>
#include <qregexp.h>
#include <qstringlist.h>
#include <qdir.h>
#include <stdlib.h>

Win32MakefileGenerator::Win32MakefileGenerator(QMakeProject *p) : MakefileGenerator(p)
{

}

int
Win32MakefileGenerator::findHighestVersion(const QString &d, const QString &stem)
{
    QString bd = Option::fixPathToLocalOS(d, true);
    if(!QFile::exists(bd))
        return -1;
    if(!project->variables()["QMAKE_" + stem.toUpper() + "_VERSION_OVERRIDE"].isEmpty())
        return project->variables()["QMAKE_" + stem.toUpper() + "_VERSION_OVERRIDE"].first().toInt();

    QDir dir(bd);
    int biggest=-1;
    QStringList entries = dir.entryList();
    QString dllStem = stem + QTDLL_POSTFIX;
    QRegExp regx("(" + dllStem + "([0-9]*)).(lib|prl)$", Qt::CaseInsensitive);
    for(QStringList::Iterator it = entries.begin(); it != entries.end(); ++it) {
        if(regx.exactMatch((*it)))
            biggest = qMax(biggest, (regx.cap(1) == dllStem ||
                                     regx.cap(2).isEmpty()) ? -1 : regx.cap(2).toInt());
    }
    QMakeMetaInfo libinfo;
    if(libinfo.readLib(bd + dllStem)) {
        if(!libinfo.isEmpty("QMAKE_PRL_VERSION"))
            biggest = qMax(biggest, libinfo.first("QMAKE_PRL_VERSION").replace(".", "").toInt());
    }
    return biggest;
}

bool
Win32MakefileGenerator::findLibraries(const QString &where)
{
    QStringList &l = project->variables()[where];
    QList<QMakeLocalFileName> dirs;
    {
        QStringList &libpaths = project->variables()["QMAKE_LIBDIR"];
        for(QStringList::Iterator libpathit = libpaths.begin(); libpathit != libpaths.end(); ++libpathit)
            dirs.append(QMakeLocalFileName((*libpathit)));
    }
    for(QStringList::Iterator it = l.begin(); it != l.end();) {
        QChar quote;
        bool modified_opt = false, remove = false;
        QString opt = (*it).trimmed();
        if((opt[0] == '\'' || opt[0] == '"') && opt[(int)opt.length()-1] == opt[0]) {
            quote = opt[0];
            opt = opt.mid(1, opt.length()-2);
        }
        if(opt.startsWith("/LIBPATH:")) {
            dirs.append(QMakeLocalFileName(opt.mid(9)));
        } else if(opt.startsWith("-L") || opt.startsWith("/L")) {
            QString libpath = opt.mid(2);
            dirs.append(QMakeLocalFileName(libpath));
            modified_opt = true;
            (*it) = "/LIBPATH:" + libpath;
        } else if(opt.startsWith("-l") || opt.startsWith("/l")) {
            QString lib = opt.right(opt.length() - 2), out;
            if(!lib.isEmpty()) {
                for(QList<QMakeLocalFileName>::Iterator it = dirs.begin(); it != dirs.end(); ++it) {
                    QString extension;
                    int ver = findHighestVersion((*it).local(), lib);
                    if(ver > 0)
                        extension += QString::number(ver);
                    extension += ".lib";
                    if(QMakeMetaInfo::libExists((*it).local() + Option::dir_sep + lib) ||
                       QFile::exists((*it).local() + Option::dir_sep + lib + extension)) {
                        out = (*it).real() + Option::dir_sep + lib + extension;
                        break;
                    }
                }
            }
            if(out.isEmpty()) {
                remove = true; //just eat it since we cannot find one..
            } else {
                modified_opt = true;
                (*it) = out;
            }
        } else if(!QFile::exists(Option::fixPathToLocalOS(opt))) {
            QList<QMakeLocalFileName> lib_dirs;
            QString file = opt;
            int slsh = file.lastIndexOf(Option::dir_sep);
            if(slsh != -1) {
                lib_dirs.append(QMakeLocalFileName(file.left(slsh+1)));
                file = file.right(file.length() - slsh - 1);
            } else {
                lib_dirs = dirs;
            }
            if(file.endsWith(".lib")) {
                file = file.left(file.length() - 4);
                if(!file.at(file.length()-1).isNumber()) {
                    for(QList<QMakeLocalFileName>::Iterator dep_it = lib_dirs.begin(); dep_it != lib_dirs.end(); ++dep_it) {
                        QString lib_tmpl(file + "%1" + ".lib");
                        int ver = findHighestVersion((*dep_it).local(), file);
                        if(ver != -1) {
                            if(ver)
                                lib_tmpl = lib_tmpl.arg(ver);
                            else
                                lib_tmpl = lib_tmpl.arg("");
                            if(slsh != -1) {
                                QString dir = (*dep_it).real();
                                if(!dir.endsWith(Option::dir_sep))
                                    dir += Option::dir_sep;
                                lib_tmpl.prepend(dir);
                            }
                            modified_opt = true;
                            (*it) = lib_tmpl;
                            break;
                        }
                    }
                }
            }
        }
        if(remove) {
            it = l.erase(it);
        } else {
            if(!quote.isNull() && modified_opt)
                (*it) = quote + (*it) + quote;
            ++it;
        }
    }
    return true;
}

void
Win32MakefileGenerator::processPrlFiles()
{
    QHash<QString, bool> processed;
    QList<QMakeLocalFileName> libdirs;
    {
        QStringList &libpaths = project->variables()["QMAKE_LIBDIR"];
        for(QStringList::Iterator libpathit = libpaths.begin(); libpathit != libpaths.end(); ++libpathit)
            libdirs.append(QMakeLocalFileName((*libpathit)));
    }
    for(bool ret = false; true; ret = false) {
        //read in any prl files included..
        QStringList l_out;
        QString where = "QMAKE_LIBS";
        if(!project->isEmpty("QMAKE_INTERNAL_PRL_LIBS"))
            where = project->first("QMAKE_INTERNAL_PRL_LIBS");
        QStringList l = project->variables()[where];
        for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
            QString opt = (*it).trimmed();
            if((opt[0] == '\'' || opt[0] == '"') && opt[(int)opt.length()-1] == opt[0])
                opt = opt.mid(1, opt.length()-2);
            if(opt.startsWith("/")) {
                if(opt.startsWith("/LIBPATH:"))
                    libdirs.append(QMakeLocalFileName(opt.mid(9)));
            } else if(!processed.contains(opt)) {
                if(processPrlFile(opt)) {
                    processed.insert(opt, true);
                    ret = true;
                } else if(QDir::isRelativePath(opt)) {
                    for(QList<QMakeLocalFileName>::Iterator it = libdirs.begin(); it != libdirs.end(); ++it) {
                        QString prl = (*it).local() + Option::dir_sep + opt;
                        if(processed.contains(prl)) {
                            break;
                        } else if(processPrlFile(prl)) {
                            processed.insert(prl, true);
                            ret = true;
                            break;
                        }
                    }
                }
            }
            if(!opt.isEmpty())
                l_out.append(opt);
        }
        if(ret)
            l = l_out;
        else
            break;
    }
}


void Win32MakefileGenerator::processVars()
{
    project->variables()["QMAKE_ORIG_TARGET"] = project->variables()["TARGET"];
    if (!project->variables()["QMAKE_INCDIR"].isEmpty())
        project->variables()["INCLUDEPATH"] += project->variables()["QMAKE_INCDIR"];
    if (!project->variables()["VERSION"].isEmpty()) {
        QStringList l = project->first("VERSION").split('.');
        project->variables()["VER_MAJ"].append(l[0]);
        project->variables()["VER_MIN"].append(l[1]);
    }
    if(project->isEmpty("QMAKE_COPY_FILE"))
        project->variables()["QMAKE_COPY_FILE"].append("$(COPY)");
    if(project->isEmpty("QMAKE_COPY_DIR"))
        project->variables()["QMAKE_COPY_DIR"].append("xcopy /s /q /y /i");
    if(project->isEmpty("QMAKE_INSTALL_FILE"))
        project->variables()["QMAKE_INSTALL_FILE"].append("$(COPY_FILE)");
    if(project->isEmpty("QMAKE_INSTALL_DIR"))
        project->variables()["QMAKE_INSTALL_DIR"].append("$(COPY_DIR)");

    fixTargetExt();
    processLibsVar();
    processRcFileVar();
    processFileTagsVar();
    processMocConfig();
    processQtConfig();
    processDllConfig();
}

void Win32MakefileGenerator::processLibsVar()
{
    project->variables()["QMAKE_LIBS"] += project->variables()["LIBS"];
    QStringList &libList = project->variables()["QMAKE_LIBS"];
    for (QStringList::Iterator stIt = libList.begin(); stIt != libList.end() ;) {
        QString s = *stIt;
        if (s.startsWith("-l")) {
            stIt = libList.erase(stIt);
            stIt = libList.insert(stIt, s.mid(2) + ".lib");
        } else if (s.startsWith("-L")) {
            stIt = libList.erase(stIt);
            project->variables()["QMAKE_LIBDIR"].append(QDir::convertSeparators(s.mid(2)));
        } else {
            stIt++;
        }
    }
}

void Win32MakefileGenerator::fixTargetExt()
{
    if (project->isActiveConfig("dll")) {
        if (!project->variables()["QMAKE_LIB_FLAG"].isEmpty()) {
            project->variables()["TARGET_EXT"].append(project->first("VERSION").replace(".", "") + ".dll");
        } else {
            project->variables()["TARGET_EXT"].append(".dll");
        }
    } else {
        if (!project->variables()["QMAKE_APP_FLAG"].isEmpty()) {
            project->variables()["TARGET_EXT"].append(".exe");
        } else {
            project->variables()["TARGET_EXT"].append(".lib");
        }
    }
}

void Win32MakefileGenerator::processRcFileVar()
{
    const bool mingw = (project->first("MAKEFILE_GENERATOR") == "MINGW");
    if (!project->variables()["RC_FILE"].isEmpty()) {
        if (!project->variables()["RES_FILE"].isEmpty()) {
            fprintf(stderr, "Both .rc and .res file specified.\n");
            fprintf(stderr, "Please specify one of them, not both.");
            exit(666);
        }
        project->variables()["RES_FILE"] = project->variables()["RC_FILE"];
        project->variables()["RES_FILE"].first().replace(".rc", mingw ? ".o" : ".res");
        project->variables()["POST_TARGETDEPS"] += project->variables()["RES_FILE"];
        project->variables()["CLEAN_FILES"] += project->variables()["RES_FILE"];
    }
    if(!project->variables()["RES_FILE"].isEmpty())
        project->variables()["QMAKE_LIBS"] += project->variables()["RES_FILE"];
}

void Win32MakefileGenerator::processQtConfig()
{
    if (project->isActiveConfig("qt")) {
        if (!(project->isActiveConfig("target_qt") && !project->variables()["QMAKE_LIB_FLAG"].isEmpty())) {
            if (!project->variables()["QMAKE_QT_DLL"].isEmpty()) {
                int hver = findHighestVersion(project->first("QMAKE_LIBDIR_QT"), "qt");
                if(hver != -1) {
                    QString ver;
                    ver.sprintf("qt" QTDLL_POSTFIX "%d.lib", hver);
                    QStringList &libs = project->variables()["QMAKE_LIBS"];
                    for(QStringList::Iterator libit = libs.begin(); libit != libs.end(); ++libit)
                        (*libit).replace(QRegExp("qt\\.lib"), ver);
                }
            }
            if (!project->isActiveConfig("dll") && !project->isActiveConfig("plugin"))
                project->variables()["QMAKE_LIBS"] += project->variables()["QMAKE_LIBS_QT_ENTRY"];
        }
    }
}

void Win32MakefileGenerator::processDllConfig()
{
    if(project->isActiveConfig("dll") || !project->variables()["QMAKE_APP_FLAG"].isEmpty()) {
        project->variables()["CONFIG"].removeAll("staticlib");
        project->variables()["QMAKE_APP_OR_DLL"].append("1");
    } else {
        project->variables()["CONFIG"].append("staticlib");
    }
}

void Win32MakefileGenerator::processFileTagsVar()
{
    char *filetags[] = { "HEADERS", "SOURCES", "DEF_FILE", "RC_FILE", "TARGET", "QMAKE_LIBS", "DESTDIR", "DLLDESTDIR", "INCLUDEPATH", NULL };
    for(int i = 0; filetags[i]; i++) {
        project->variables()["QMAKE_FILETAGS"] << filetags[i];
        //clean path
        QStringList &gdmf = project->variables()[filetags[i]];
        for(QStringList::Iterator it = gdmf.begin(); it != gdmf.end(); ++it)
            (*it) = Option::fixPathToTargetOS((*it), false);
    }
}

void Win32MakefileGenerator::writeCleanParts(QTextStream &t)
{
    QString mocclean = varGlue("SRCMOC" ,"\n\t-$(DEL_FILE) ","\n\t-$(DEL_FILE) ","") +
                       varGlue("OBJMOC" ,"\n\t-$(DEL_FILE) ","\n\t-$(DEL_FILE) ","");
    t << "mocclean:" << mocclean << endl;

    t << "clean: compiler_clean mocclean"
        << varGlue("OBJECTS","\n\t-$(DEL_FILE) ","\n\t-$(DEL_FILE) ","")
        << varGlue("QMAKE_CLEAN","\n\t-$(DEL_FILE) ","\n\t-$(DEL_FILE) ","\n")
        << varGlue("CLEAN_FILES","\n\t-$(DEL_FILE) ","\n\t-$(DEL_FILE) ","\n");

    t << endl;

    t << "distclean: clean"
        << "\n\t-$(DEL_FILE) $(TARGET)"
        << endl << endl;
}

void Win32MakefileGenerator::writeStandardParts(QTextStream &t)
{
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
    t << "INCPATH       = ";

    QStringList &incs = project->variables()["INCLUDEPATH"];
    for(QStringList::Iterator incit = incs.begin(); incit != incs.end(); ++incit) {
        QString inc = (*incit);
        inc.replace(QRegExp("\\\\$"), "\\\\");
        inc.replace(QRegExp("\""), "");
        t << "-I" << "\"" << inc << "\" ";
    }
    t << "-I\"" << specdir() << "\""
      << endl;

    writeLibsPart(t);

    t << "MOC           = " << (project->isEmpty("QMAKE_MOC") ? QString("moc") :
                              Option::fixPathToTargetOS(var("QMAKE_MOC"), false)) << endl;
    t << "UIC3          = " << (project->isEmpty("QMAKE_UIC3") ? QString("uic3") :
                              Option::fixPathToTargetOS(var("QMAKE_UIC3"), false)) << endl;
    t << "UIC           = " << (project->isEmpty("QMAKE_UIC") ? QString("uic") :
                              Option::fixPathToTargetOS(var("QMAKE_UIC"), false)) << endl;
    t << "QMAKE         = " << (project->isEmpty("QMAKE_QMAKE") ? QString("qmake") :
                              Option::fixPathToTargetOS(var("QMAKE_QMAKE"), false)) << endl;
    t << "IDC           = " << (project->isEmpty("QMAKE_IDC") ? QString("idc") :
                              Option::fixPathToTargetOS(var("QMAKE_IDC"), false)) << endl;
    t << "IDL           = " << (project->isEmpty("QMAKE_IDL") ? QString("midl") :
                              Option::fixPathToTargetOS(var("QMAKE_IDL"), false)) << endl;
    t << "ZIP           = " << var("QMAKE_ZIP") << endl;
    t << "DEF_FILE      = " << varList("DEF_FILE") << endl;
    t << "RES_FILE      = " << varList("RES_FILE") << endl; // Not on mingw, can't see why not though...
    t << "COPY          = " << var("QMAKE_COPY") << endl;
    t << "COPY_FILE     = " << var("QMAKE_COPY_FILE") << endl;
    t << "COPY_DIR      = " << var("QMAKE_COPY_DIR") << endl;
    t << "DEL_FILE      = " << var("QMAKE_DEL_FILE") << endl;
    t << "DEL_DIR       = " << var("QMAKE_DEL_DIR") << endl;
    t << "MOVE          = " << var("QMAKE_MOVE") << endl;
    t << "CHK_DIR_EXISTS= " << var("QMAKE_CHK_DIR_EXISTS") << endl;
    t << "MKDIR         = " << var("QMAKE_MKDIR") << endl;
    t << "INSTALL_FILE  = " << var("QMAKE_INSTALL_FILE") << endl;
    t << "INSTALL_DIR   = " << var("QMAKE_INSTALL_DIR") << endl;
    t << endl;

    t << "####### Output directory" << endl << endl;
    if(!project->variables()["OBJECTS_DIR"].isEmpty())
        t << "OBJECTS_DIR   = " << var("OBJECTS_DIR").replace(QRegExp("\\\\$"),"") << endl;
    else
        t << "OBJECTS_DIR   = . " << endl;
    if(!project->variables()["MOC_DIR"].isEmpty())
        t << "MOC_DIR       = " << var("MOC_DIR").replace(QRegExp("\\\\$"),"") << endl;
    else
        t << "MOC_DIR       = . " << endl;
    t << endl;

    t << "####### Files" << endl << endl;
    t << "HEADERS       = " << varList("HEADERS") << endl;
    t << "SOURCES       = " << varList("SOURCES") << endl;

    writeObjectsPart(t);

    t << "SRCMOC        = " << varList("SRCMOC") << endl;

    writeObjMocPart(t);
    writeExtraCompilerVariables(t);
    writeExtraVariables(t);

    t << "DIST          = " << varList("DISTFILES") << endl;
    t << "TARGET        = ";
    if(!project->variables()["DESTDIR"].isEmpty())
        t << varGlue("TARGET", project->first("DESTDIR"),"",project->first("TARGET_EXT"));
    else
        t << project->variables()["TARGET"].value(0) << project->variables()["TARGET_EXT"].value(0);
    t << endl << endl;

    t << "####### Implicit rules" << endl << endl;
    writeImplicitRulesPart(t);

    t << "####### Build rules" << endl << endl;
    writeBuildRulesPart(t);

    if (!project->variables()["QMAKE_POST_LINK"].isEmpty())
        t << "\t" <<var("QMAKE_POST_LINK") << endl;

    if(project->isActiveConfig("dll") && !project->variables()["DLLDESTDIR"].isEmpty()) {
        QStringList dlldirs = project->variables()["DLLDESTDIR"];
        for (QStringList::Iterator dlldir = dlldirs.begin(); dlldir != dlldirs.end(); ++dlldir) {
            t << "\n\t" << "-$(COPY_FILE) \"$(TARGET)\" " << *dlldir;
        }
    }

    t << endl << endl;

    writeRcFilePart(t);

    t << "mocables: $(SRCMOC)" << endl;

    writeMakeQmake(t);

    QStringList dist_files = Option::mkfile::project_files;
    if(!project->isEmpty("QMAKE_INTERNAL_INCLUDED_FILES"))
        dist_files += project->variables()["QMAKE_INTERNAL_INCLUDED_FILES"];
    if(!project->isEmpty("TRANSLATIONS"))
        dist_files << var("TRANSLATIONS");
    if(!project->isEmpty("FORMS")) {
        QStringList &forms = project->variables()["FORMS"];
        for(QStringList::Iterator formit = forms.begin(); formit != forms.end(); ++formit) {
            QString ui_h = fileFixify((*formit) + Option::h_ext.first());
            if(QFile::exists(ui_h))
                dist_files << ui_h;
        }
    }
    t << "dist:" << "\n\t"
      << "$(ZIP) " << var("QMAKE_ORIG_TARGET") << ".zip " << "$(SOURCES) $(HEADERS) $(DIST) $(FORMS) "
      << dist_files.join(" ") << " " << var("TRANSLATIONS") << " " << var("IMAGES") << endl << endl;

    writeCleanParts(t);
    writeExtraTargets(t);
    writeExtraCompilerTargets(t);
    t << endl << endl;
}

void Win32MakefileGenerator::writeLibsPart(QTextStream &t)
{
    if(!project->variables()["QMAKE_APP_OR_DLL"].isEmpty()) {
        t << "LINK          = " << var("QMAKE_LINK") << endl;
        t << "LFLAGS        = ";
        if(!project->variables()["QMAKE_LIBDIR"].isEmpty())
            writeLibDirPart(t);
        t << var("QMAKE_LFLAGS") << endl;
        t << "LIBS          = " << var("QMAKE_LIBS") << endl;
    } else {
        t << "LIB           = " << var("QMAKE_LIB") << endl;
    }
}

void Win32MakefileGenerator::writeLibDirPart(QTextStream &t)
{
    t << varGlue("QMAKE_LIBDIR","-L\"","\" -L\"","\"") << " ";
}

void Win32MakefileGenerator::processMocConfig()
{
    if(project->isActiveConfig("moc"))
        setMocAware(true);
}

void Win32MakefileGenerator::writeObjectsPart(QTextStream &t)
{
    t << "OBJECTS       = " << varList("OBJECTS") << endl;
}

void Win32MakefileGenerator::writeObjMocPart(QTextStream &t)
{
    t << "OBJMOC        = " << varList("OBJMOC") << endl;
}

void Win32MakefileGenerator::writeImplicitRulesPart(QTextStream &t)
{
    t << ".SUFFIXES: .c";
    QStringList::Iterator cppit;
    for(cppit = Option::cpp_ext.begin(); cppit != Option::cpp_ext.end(); ++cppit)
        t << " " << (*cppit);
    t << endl << endl;
    for(cppit = Option::cpp_ext.begin(); cppit != Option::cpp_ext.end(); ++cppit)
        t << (*cppit) << Option::obj_ext << ":\n\t" << var("QMAKE_RUN_CXX_IMP") << endl << endl;
    t << ".c" << Option::obj_ext << ":\n\t" << var("QMAKE_RUN_CC_IMP") << endl << endl;
}

void Win32MakefileGenerator::writeBuildRulesPart(QTextStream &)
{
}

void Win32MakefileGenerator::writeRcFilePart(QTextStream &t)
{
    if(!project->variables()["RC_FILE"].isEmpty()) {
        t << var("RES_FILE") << ": " << var("RC_FILE") << "\n\t"
            << var("QMAKE_RC") << " " << var("RC_FILE") << endl << endl;
    }
}

QString Win32MakefileGenerator::defaultInstall(const QString &t)
{
    if(t != "target" || project->first("TEMPLATE") == "subdirs")
        return QString();

    const QString root = "$(INSTALL_ROOT)";
    QStringList &uninst = project->variables()[t + ".uninstall"];
    QString ret;
    QString targetdir = Option::fixPathToTargetOS(project->first("target.path"), false);
    targetdir = fileFixify(targetdir, FileFixifyAbsolute);
    if(targetdir.right(1) != Option::dir_sep)
        targetdir += Option::dir_sep;

    QStringList links;
    QString target="$(TARGET)";
    if(project->first("TEMPLATE") == "lib") {
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
    }

    QString src_targ = target;
    QString dst_targ = filePrefixRoot(root, fileFixify(targetdir + target, FileFixifyAbsolute));
    if(!ret.isEmpty())
        ret += "\n\t";
    ret += QString("-$(INSTALL_FILE)") + " \"" + src_targ + "\" \"" + dst_targ + "\"";
    if(!uninst.isEmpty())
        uninst.append("\n\t");
    uninst.append("-$(DEL_FILE) \"" + dst_targ + "\"");
    return ret;
}
