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

#include "metrowerks_xml.h"
#include "option.h"
#include <qdir.h>
#include <qhash.h>
#include <qregexp.h>
#include <stdlib.h>
#include <time.h>
#if !defined(QWS) && defined(Q_OS_MAC)
#include <Carbon/Carbon.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

MetrowerksMakefileGenerator::MetrowerksMakefileGenerator() : MakefileGenerator(), init_flag(false)
{

}

bool
MetrowerksMakefileGenerator::writeMakefile(QTextStream &t)
{
    if(!project->variables()["QMAKE_FAILED_REQUIREMENTS"].isEmpty()) {
        /* for now just dump, I need to generated an empty xml or something.. */
        fprintf(stderr, "Project file not generated because all requirements not met:\n\t%s\n",
                var("QMAKE_FAILED_REQUIREMENTS").toLatin1().constData());
        return true;
    }

    if(project->first("TEMPLATE") == "app" ||
       project->first("TEMPLATE") == "lib") {
        return writeMakeParts(t);
    }
    else if(project->first("TEMPLATE") == "subdirs") {
        writeHeader(t);
        qDebug("Not supported!");
        return true;
    }
    return false;
}

bool
MetrowerksMakefileGenerator::writeMakeParts(QTextStream &t)
{
    //..grrr.. libs!
    QStringList extra_objs;
    bool do_libs = true;
    if(project->first("TEMPLATE") == "app")
        extra_objs += project->variables()["QMAKE_CRT_OBJECTS"];
    else if(project->first("TEMPLATE") == "lib" && project->isActiveConfig("staticlib"))
        do_libs = false;
    if(do_libs)
        extra_objs += project->variables()["QMAKE_LIBS"];
    for(QStringList::Iterator val_it = extra_objs.begin();
        val_it != extra_objs.end(); ++val_it) {
        if((*val_it).startsWith("-L")) {
            QString dir = fixEnvVariables((*val_it).right((*val_it).length() - 2));
            if(project->variables()["DEPENDPATH"].indexOf(dir) == -1 &&
               project->variables()["INCLUDEPATH"].indexOf(dir) == -1)
                project->variables()["INCLUDEPATH"].append(dir);
        } else if((*val_it).startsWith("-l")) {
            QString lib("lib" + (*val_it).right((*val_it).length() - 2)  + "." +
                        project->first("QMAKE_EXTENSION_SHLIB"));
            if(project->variables()["LIBRARIES"].indexOf(lib) == -1)
                project->variables()["LIBRARIES"].append(lib);
        } else
            if((*val_it) == "-framework") {
            ++val_it;
            if(val_it == extra_objs.end())
                break;
            QString frmwrk = (*val_it) + ".framework";
            if(project->variables()["FRAMEWORKS"].indexOf(frmwrk) == -1)
                project->variables()["FRAMEWORKS"].append(frmwrk);
        } else if((*val_it).left(1) != "-") {
            QString lib=(*val_it);
            int s = lib.lastIndexOf('/');
            if(s != -1) {
                QString dir = fixEnvVariables(lib.left(s));
                lib = lib.right(lib.length() - s - 1);
                if(project->variables()["DEPENDPATH"].indexOf(dir) == -1 &&
                   project->variables()["INCLUDEPATH"].indexOf(dir) == -1)
                    project->variables()["INCLUDEPATH"].append(dir);
            }
            project->variables()["LIBRARIES"].append(lib);
        }
    }
    //let metrowerks find the files & set the files to the type I expect
    QHash<QString, bool> seen;
    QString paths[] = { QString("SRCMOC"), QString("SOURCES"), QString("HEADERS"),
                        QString() };
    for(int y = 0; !paths[y].isNull(); y++) {
        QStringList &l = project->variables()[paths[y]];
        for(QStringList::Iterator val_it = l.begin(); val_it != l.end(); ++val_it) {
            //establish file types
            seen.insert((*val_it), true);
            createFork((*val_it)); //the file itself
            QStringList &d = findDependencies((*val_it)); //depends
            for(QStringList::Iterator dep_it = d.begin(); dep_it != d.end(); ++dep_it) {
                if(!seen.find((*dep_it))) {
                    seen.insert((*dep_it), true);
                    createFork((*dep_it));
                }
            }
            //now chop it
            int s = (*val_it).lastIndexOf('/');
            if(s != -1) {
                QString dir = (*val_it).left(s);
                (*val_it) = (*val_it).right((*val_it).length() - s - 1);
                QString tmpd=dir, tmpv;
                if(fixifyToMacPath(tmpd, tmpv)) {
                    bool add_in = true;
                    QString deps[] = { QString("DEPENDPATH"),
                                     QString("INCLUDEPATH"), QString() },
                                     dd, dv;
                    for(int yy = 0; !deps[yy].isNull(); yy++) {
                        QStringList &l2 = project->variables()[deps[yy]];
                        for(QStringList::Iterator val_it2 = l2.begin();
                            val_it2 != l2.end(); ++val_it2) {
                            QString dd= (*val_it2), dv;
                            if(!fixifyToMacPath(dd, dv))
                                continue;
                            if(dd == tmpd && tmpv == dv) {
                                add_in = false;
                                break;
                            }
                        }
                    }
                    if(add_in)
                        project->variables()["INCLUDEPATH"].append(dir);
                }
            }
        }
    }
    //need a defines file
    if(!project->isEmpty("DEFINES")) {
        QString pre_pref = project->first("TARGET_STEM");
        if(project->first("TEMPLATE") == "lib")
            pre_pref += project->isActiveConfig("staticlib") ? "_static" : "_shared";
        project->variables()["CODEWARRIOR_PREFIX_HEADER"].append(pre_pref + "_prefix.h");
    }

    QString xmlfile = findTemplate(project->first("QMAKE_XML_TEMPLATE"));
    QFile file(xmlfile);
    if(!file.open(QIODevice::ReadOnly)) {
        fprintf(stderr, "Cannot open XML file: %s\n",
                project->first("QMAKE_XML_TEMPLATE").toLatin1().constData());
        return false;
    }
    QTextStream xml(&file);
    createFork(Option::output.fileName());

    int rep;
    QString line;
    while (!xml.atEnd()) {
        line = xml.readLine();
        while((rep = line.indexOf(QRegExp("\\$\\$[!a-zA-Z0-9_-]*"))) != -1) {
            QString torep = line.mid(rep, line.indexOf(QRegExp("[^\\$!a-zA-Z0-9_-]"), rep) - rep);
            QString variable = torep.right(torep.length()-2);

            t << line.left(rep); //output the left side
            line = line.right(line.length() - (rep + torep.length())); //now past the variable
            if(variable == "CODEWARRIOR_HEADERS" || variable == "CODEWARRIOR_SOURCES" ||
               variable == "CODEWARRIOR_LIBRARIES" || variable == "CODEWARRIOR_QPREPROCESS" ||
                variable == "CODEWARRIOR_QPREPROCESSOUT") {
                QString outcmd=variable.right(variable.length() - variable.lastIndexOf('_') - 1);
                QStringList args;
                if(outcmd == "QPREPROCESS")
                    args << "UICS" << "MOCS";
                else if(outcmd == "QPREPROCESSOUT")
                    args << "SRCMOC" << "UICIMPLS" << "UICDELCS";
                else
                    args << outcmd;
                for(QStringList::Iterator arit = args.begin(); arit != args.end(); ++arit) {
                    QString arg = (*arit);
                    QString kind = "Text";
                    if(arg == "LIBRARIES")
                        kind = "Library";
                    if(!project->variables()[arg].isEmpty()) {
                        QStringList &list = project->variables()[arg];
                        for(QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
                            QString flag;
                            if(project->isActiveConfig("debug")) {
                                bool debug = true;
                                if(outcmd == "QPREPROCESS") {
                                    debug = false;
                                } else {
                                    for(QStringList::Iterator hit = Option::h_ext.begin(); hit != Option::h_ext.end(); ++hit) {
                                        if((*it).endsWith((*hit))) {
                                            debug = false;
                                            break;
                                        }
                                    }
                                }
                                if(debug)
                                    flag = "Debug";
                            }
                            t << "\t\t\t\t<FILE>" << endl
                              << "\t\t\t\t\t<PATHTYPE>Name</PATHTYPE>" << endl
                              << "\t\t\t\t\t<PATH>" << (*it) << "</PATH>" << endl
                              << "\t\t\t\t\t<PATHFORMAT>MacOS</PATHFORMAT>" << endl
                              << "\t\t\t\t\t<FILEKIND>" << kind << "</FILEKIND>" << endl
                              << "\t\t\t\t\t<FILEFLAGS>" << flag << "</FILEFLAGS>" << endl
                              << "\t\t\t\t</FILE>" << endl;
                        }
                    }
                }
            } else if(variable == "CODEWARRIOR_SOURCES_LINKORDER" ||
                      variable == "CODEWARRIOR_HEADERS_LINKORDER" ||
                      variable == "CODEWARRIOR_LIBRARIES_LINKORDER" ||
                      variable == "CODEWARRIOR_QPREPROCESS_LINKORDER" ||
                      variable == "CODEWARRIOR_QPREPROCESSOUT_LINKORDER") {
                QString outcmd=variable.mid(variable.indexOf('_')+1,
                                            variable.lastIndexOf('_')-(variable.indexOf('_')+1));
                QStringList args;
                if(outcmd == "QPREPROCESS")
                    args << "UICS" << "MOCS";
                else if(outcmd == "QPREPROCESSOUT")
                    args << "SRCMOC" << "UICIMPLS" << "UICDELCS";
                else
                    args << outcmd;
                for(QStringList::Iterator arit = args.begin(); arit != args.end(); ++arit) {
                    QString arg = (*arit);
                    if(!project->variables()[arg].isEmpty()) {
                        QStringList &list = project->variables()[arg];
                        for(QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
                            t << "\t\t\t\t<FILEREF>" << endl
                              << "\t\t\t\t\t<PATHTYPE>Name</PATHTYPE>" << endl
                              << "\t\t\t\t\t<PATH>" << (*it) << "</PATH>" << endl
                              << "\t\t\t\t\t<PATHFORMAT>MacOS</PATHFORMAT>" << endl
                              << "\t\t\t\t</FILEREF>" << endl;
                        }
                    }
                }
            } else if(variable == "CODEWARRIOR_HEADERS_GROUP" ||
                      variable == "CODEWARRIOR_SOURCES_GROUP" ||
                      variable == "CODEWARRIOR_LIBRARIES_GROUP" ||
                      variable == "CODEWARRIOR_QPREPROCESS_GROUP" ||
                      variable == "CODEWARRIOR_QPREPROCESSOUT_GROUP") {
                QString outcmd = variable.mid(variable.indexOf('_')+1,
                                              variable.lastIndexOf('_')-(variable.indexOf('_')+1));
                QStringList args;
                if(outcmd == "QPREPROCESS")
                    args << "UICS" << "MOCS";
                else if(outcmd == "QPREPROCESSOUT")
                    args << "SRCMOC" << "UICIMPLS" << "UICDELCS";
                else
                    args << outcmd;
                for(QStringList::Iterator arit = args.begin(); arit != args.end(); ++arit) {
                    QString arg = (*arit);
                    if(!project->variables()[arg].isEmpty()) {
                        QStringList &list = project->variables()[arg];
                        for(QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
                            t << "\t\t\t\t<FILEREF>" << endl
                              << "\t\t\t\t\t<TARGETNAME>" << var("TARGET_STEM") << "</TARGETNAME>"
                              << endl
                              << "\t\t\t\t\t<PATHTYPE>Name</PATHTYPE>" << endl
                              << "\t\t\t\t\t<PATH>" << (*it) << "</PATH>" << endl
                              << "\t\t\t\t\t<PATHFORMAT>MacOS</PATHFORMAT>" << endl
                              << "\t\t\t\t</FILEREF>" << endl;
                        }
                    }
                }
            } else if(variable == "CODEWARRIOR_FRAMEWORKS") {
                if(!project->isEmpty("FRAMEWORKS")) {
                    QStringList &list = project->variables()["FRAMEWORKS"];
                    for(QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
                        t << "\t\t\t\t<FRAMEWORK>" << endl
                          << "\t\t\t\t\t<FILEREF>" << endl
                          << "\t\t\t\t\t\t<PATHTYPE>Name</PATHTYPE>" << endl
                          << "\t\t\t\t\t\t<PATH>" << (*it) << "</PATH>" << endl
                          << "\t\t\t\t\t\t<PATHFORMAT>MacOS</PATHFORMAT>" << endl
                          << "\t\t\t\t\t</FILEREF>" << endl
                          << "\t\t\t\t</FRAMEWORK>" << endl;
                    }
                }
            } else if(variable == "CODEWARRIOR_DEPENDPATH" || variable == "CODEWARRIOR_INCLUDEPATH" ||
                variable == "CODEWARRIOR_FRAMEWORKPATH") {
                QString arg=variable.right(variable.length()-variable.indexOf('_')-1);
                QStringList list;
                if(arg == "INCLUDEPATH") {
                    list = project->variables()[arg];
                    list << Option::mkfile::qmakespec;
                    list << qmake_getpwd();

                    QStringList &l = project->variables()["QMAKE_LIBS_PATH"];
                    for(QStringList::Iterator val_it = l.begin(); val_it != l.end(); ++val_it) {
                        QString p = (*val_it), v;
                        if(!fixifyToMacPath(p, v))
                            continue;

                        t << "\t\t\t\t\t<SETTING>" << endl
                          << "\t\t\t\t\t\t<SETTING><NAME>SearchPath</NAME>" << endl
                          << "\t\t\t\t\t\t\t<SETTING><NAME>Path</NAME>"
                          << "<VALUE>" << p << "</VALUE></SETTING>" << endl
                          << "\t\t\t\t\t\t\t<SETTING><NAME>PathFormat</NAME><VALUE>MacOS</VALUE></SETTING>" << endl
                          << "\t\t\t\t\t\t\t<SETTING><NAME>PathRoot</NAME><VALUE>CodeWarrior</VALUE></SETTING>" << endl
                          << "\t\t\t\t\t\t</SETTING>" << endl
                          << "\t\t\t\t\t\t<SETTING><NAME>Recursive</NAME><VALUE>true</VALUE></SETTING>" << endl
                          << "\t\t\t\t\t\t<SETTING><NAME>HostFlags</NAME><VALUE>All</VALUE></SETTING>" << endl
                          << "\t\t\t\t\t</SETTING>" << endl;
                    }
                } else if(variable == "DEPENDPATH") {
                    QStringList &l = project->variables()[arg];
                    for(QStringList::Iterator val_it = l.begin(); val_it != l.end(); ++val_it)
                    {
                        //apparently tmake used colon separation...
                        QStringList damn = (*val_it).split(':');
                        if(!damn.isEmpty())
                            list += damn;
                        else
                            list.append((*val_it));
                    }
                } else {
                    list = project->variables()[arg];
                }
                for(QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
                    QString p = (*it), v, recursive = "false", framework = "false";
                    if(p.startsWith("recursive--")) {
                        p = p.right(p.length() - 11);
                        recursive = "true";
                    }
                    if(!fixifyToMacPath(p, v))
                        continue;
                    if(arg == "FRAMEWORKPATH")
                        framework = "true";

                    t << "\t\t\t\t\t<SETTING>" << endl
                      << "\t\t\t\t\t\t<SETTING><NAME>SearchPath</NAME>" << endl
                      << "\t\t\t\t\t\t\t<SETTING><NAME>Path</NAME>"
                      << "<VALUE>" << p << "</VALUE></SETTING>" << endl
                      << "\t\t\t\t\t\t\t<SETTING><NAME>PathFormat</NAME><VALUE>MacOS</VALUE></SETTING>" << endl
                      << "\t\t\t\t\t\t\t<SETTING><NAME>PathRoot</NAME><VALUE>" << v << "</VALUE></SETTING>" << endl
                      << "\t\t\t\t\t\t</SETTING>" << endl
                      << "\t\t\t\t\t\t<SETTING><NAME>Recursive</NAME><VALUE>" << recursive << "</VALUE></SETTING>" << endl
                    << "\t\t\t\t\t\t<SETTING><NAME>FrameworkPath</NAME><VALUE>" << framework << "</VALUE></SETTING>" << endl
                    << "\t\t\t\t\t\t<SETTING><NAME>HostFlags</NAME><VALUE>All</VALUE></SETTING>" << endl
                      << "\t\t\t\t\t</SETTING>" << endl;
                }
            } else if(variable == "CODEWARRIOR_WARNING" || variable == "!CODEWARRIOR_WARNING") {
                bool b = ((!project->isActiveConfig("warn_off")) &&
                          project->isActiveConfig("warn_on"));
                if(variable.startsWith("!"))
                    b = !b;
                t << (int)b;
            } else if(variable == "CODEWARRIOR_TEMPLATE") {
                if(project->first("TEMPLATE") == "app") {
                    t << "Executable";
                } else if(project->first("TEMPLATE") == "lib") {
                    if(project->isActiveConfig("staticlib"))
                       t << "Library";
                    else
                        t << "SharedLibrary";
                }
            } else if(variable == "CODEWARRIOR_OUTPUT_DIR") {
                QString outdir = "{Project}/", volume;
                if(!project->isEmpty("DESTDIR"))
                    outdir = project->first("DESTDIR");
                if(project->first("TEMPLATE") == "app" && !project->isActiveConfig("console"))
                    outdir += var("TARGET") + ".app/Contents/MacOS/";
                if(fixifyToMacPath(outdir, volume, false)) {
                    t << "\t\t\t<SETTING><NAME>Path</NAME><VALUE>" << outdir << "</VALUE></SETTING>"
                      << endl
                      << "\t\t\t<SETTING><NAME>PathFormat</NAME><VALUE>MacOS</VALUE></SETTING>" << endl
                      << "\t\t\t<SETTING><NAME>PathRoot</NAME><VALUE>" << volume << "</VALUE></SETTING>"
                      << endl;
                }
            } else if(variable == "CODEWARRIOR_PACKAGER_PANEL") {
                if(project->first("TEMPLATE") == "app" && !project->isActiveConfig("console")) {
                    QString outdir = "{Project}/", volume;
                    if(!project->isEmpty("DESTDIR"))
                        outdir = project->first("DESTDIR");
                    outdir += var("TARGET") + ".app";
                    if(fixifyToMacPath(outdir, volume, false)) {
                        t << "\t\t<SETTING><NAME>MWMacOSPackager_UsePackager</NAME>"
                          << "<VALUE>1</VALUE></SETTING>" << "\n"
                          << "\t\t<SETTING><NAME>MWMacOSPackager_FolderToPackage</NAME>" << "\n"
                          << "\t\t\t<SETTING><NAME>Path</NAME><VALUE>" << outdir
                          << "</VALUE></SETTING>" << "\n"
                          << "\t\t\t<SETTING><NAME>PathFormat</NAME><VALUE>MacOS</VALUE></SETTING>"
                          << "\n"
                          << "\t\t\t<SETTING><NAME>PathRoot</NAME><VALUE>" << volume
                          << "</VALUE></SETTING>" << "\n"
                          << "\t\t</SETTING>" << "\n"
                          << "\t\t<SETTING><NAME>MWMacOSPackager_CreateClassicAlias</NAME>"
                          << "<VALUE>0</VALUE></SETTING>" << "\n"
                          << "\t\t<SETTING><NAME>MWMacOSPackager_ClassicAliasMethod</NAME>"
                          << "<VALUE>UseTargetOutput</VALUE></SETTING>" << "\n"
                          << "\t\t<SETTING><NAME>MWMacOSPackager_ClassicAliasPath</NAME>"
                          << "<VALUE></VALUE></SETTING>" << "\n"
                          << "\t\t<SETTING><NAME>MWMacOSPackager_CreatePkgInfo</NAME>"
                          << "<VALUE>1</VALUE></SETTING>" << "\n"
                          << "\t\t<SETTING><NAME>MWMacOSPackager_PkgCreatorType</NAME>"
                          << "<VALUE>CUTE</VALUE></SETTING>" << "\n"
                          << "\t\t<SETTING><NAME>MWMacOSPackager_PkgFileType</NAME>"
                          << "<VALUE>APPL</VALUE></SETTING>" << endl;
                    }
                }
            } else if(variable == "CODEWARRIOR_FILETYPE") {
                if(project->first("TEMPLATE") == "lib")
                        t << "MYDL";
                else
                        t << "MEXE";
            } else if(variable == "CODEWARRIOR_CACHEMODDATES") {
                t << "true";
            } else {
                t << var(variable);
            }
        }
        t << line << endl;
    }
    t << endl;
    t.flush();
    file.close();

    if(!project->isEmpty("CODEWARRIOR_PREFIX_HEADER")) {
        QFile prefixfile(project->first("CODEWARRIOR_PREFIX_HEADER"));
        if(!prefixfile.open(QIODevice::WriteOnly)) {
            fprintf(stderr, "Cannot open PREFIX file: %s\n", prefixfile.fileName().toLatin1().constData());
        } else {
            createFork(project->first("CODEWARRIOR_PREFIX_HEADER"));
            QTextStream prefix(&prefixfile);
            QStringList &list = project->variables()["DEFINES"];
            for(QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
                if((*it).indexOf('=') != -1) {
                    int x = (*it).indexOf('=');
                    prefix << "#define " << (*it).left(x) << " " << (*it).right((*it).length() - x - 1) << endl;
                } else {
                    prefix << "#define " << (*it) << endl;
                }
            }
            prefix.flush();
            prefixfile.close();
        }
    }
    return true;
}



void
MetrowerksMakefileGenerator::init()
{
    if(init_flag)
        return;
    init_flag = true;

    if (project->isEmpty("QMAKE_XML_TEMPLATE"))
        project->variables()["QMAKE_XML_TEMPLATE"].append("mwerkstmpl.xml");

    QStringList &configs = project->variables()["CONFIG"];
    if(project->isActiveConfig("qt")) {
        if(configs.indexOf("moc")) configs.append("moc");
        if (!((project->first("TARGET") == "qt") || (project->first("TARGET") == "qte") ||
                (project->first("TARGET") == "qt-mt")))
            project->variables()["QMAKE_LIBS"] += project->variables()["QMAKE_LIBS_QT"];
        if(configs.indexOf("moc"))
            configs.append("moc");
        if (!project->isActiveConfig("debug"))
            project->variables()["DEFINES"].append("QT_NO_DEBUG");
    }

    //version handling
    if(project->variables()["VERSION"].isEmpty())
        project->variables()["VERSION"].append("1.0." +
                                               (project->isEmpty("VER_PAT") ? QString("0") :
                                                project->first("VER_PAT")));
    QStringList ver = project->first("VERSION").split('.');
    ver << "0" << "0"; //make sure there are three
    project->variables()["VER_MAJ"].append(ver[0]);
    project->variables()["VER_MIN"].append(ver[1]);
    project->variables()["VER_PAT"].append(ver[2]);

    if(!project->isEmpty("LIBS"))
        project->variables()["QMAKE_LIBS"] += project->variables()["LIBS"];
    if(project->variables()["QMAKE_EXTENSION_SHLIB"].isEmpty())
        project->variables()["QMAKE_EXTENSION_SHLIB"].append("dylib");

    if (project->isActiveConfig("moc")) {
        QString mocfile = project->first("TARGET");
        if(project->first("TEMPLATE") == "lib")
            mocfile += project->isActiveConfig("staticlib") ? "_static" : "_shared";
        project->variables()["MOCS"].append(mocfile + ".mocs");
    }
    if(!project->isEmpty("FORMS")) {
        QString uicfile = project->first("TARGET");
        if(project->first("TEMPLATE") == "lib")
            uicfile += project->isActiveConfig("staticlib") ? "_static" : "_shared";
        project->variables()["UICS"].append(uicfile + ".uics");
    }
    if(project->isEmpty("DESTDIR"))
        project->variables()["DESTDIR"].append(qmake_getpwd());
    MakefileGenerator::init();

    if (project->isActiveConfig("opengl")) {
        project->variables()["INCLUDEPATH"] += project->variables()["QMAKE_INCDIR_OPENGL"];
        if ((project->first("TARGET") == "qt") || (project->first("TARGET") == "qte") ||
             (project->first("TARGET") == "qt-mt"))
            project->variables()["QMAKE_LIBS"] += project->variables()["QMAKE_LIBS_OPENGL_QT"];
        else
            project->variables()["QMAKE_LIBS"] += project->variables()["QMAKE_LIBS_OPENGL"];
    }

    if(project->isActiveConfig("qt"))
        project->variables()["INCLUDEPATH"] += project->variables()["QMAKE_INCDIR_QT"];
    if(project->isEmpty("FRAMEWORKPATH"))
        project->variables()["FRAMEWORKPATH"].append("/System/Library/Frameworks/");

    //set the target up
    project->variables()["TARGET_STEM"] = project->variables()["TARGET"];
    if(project->first("TEMPLATE") == "lib") {
        if(project->isActiveConfig("staticlib"))
            project->variables()["TARGET"].first() =  "lib" + project->first("TARGET") + ".lib";
        else
            project->variables()["TARGET"].first() =  "lib" + project->first("TARGET") + "." +
                                                      project->first("QMAKE_EXTENSION_SHLIB");

        project->variables()["CODEWARRIOR_VERSION"].append(project->first("VER_MAJ") +
                                                          project->first("VER_MIN") +
                                                          project->first("VER_PAT"));
    } else {
        project->variables()["CODEWARRIOR_VERSION"].append("0");
        if(project->isEmpty("QMAKE_ENTRYPOINT"))
           project->variables()["QMAKE_ENTRYPOINT"].append("start");
        project->variables()["CODEWARRIOR_ENTRYPOINT"].append(
            project->first("QMAKE_ENTRYPOINT"));
    }
}


QString
MetrowerksMakefileGenerator::findTemplate(const QString &file)
{
    QString ret;
    if(!exists(ret = file) &&
       !exists((ret = Option::mkfile::qmakespec + QDir::separator() + file)) &&
       !exists((ret = QString(QLibraryInfo::location(QLibraryInfo::DataPath) + "/mac-mwerks/" + file))) &&
       !exists((ret = (QString(qgetenv("HOME")) + "/.tmake/" + file))))
        return "";
    return ret;
}

bool
MetrowerksMakefileGenerator::createFork(const QString &f)
{
#if !defined(QWS) && defined(Q_OS_MAC)
    FSRef fref;
    FSSpec fileSpec;
    if(exists(f)) {
        mode_t perms = 0;
        {
            struct stat s;
            stat(f.toLocal8Bit().constData(), &s);
            if(!(s.st_mode & S_IWUSR)) {
                perms = s.st_mode;
                chmod(f.toLatin1().constData(), perms | S_IWUSR);
            }
        }
        FILE *o = fopen(f.toLatin1().constData(), "a");
        if(!o)
            return false;
        if(FSPathMakeRef((const UInt8 *)f.toLatin1().constData(), &fref, NULL) == noErr) {
            if(FSGetCatalogInfo(&fref, kFSCatInfoNone, NULL, NULL, &fileSpec, NULL) == noErr)
                FSpCreateResFile(&fileSpec, 'CUTE', 'TEXT', smSystemScript);
            else
                qDebug("bogus %d", __LINE__);
        } else
            qDebug("bogus %d", __LINE__);
        fclose(o);
        if(perms)
            chmod(f.toLatin1().constData(), perms);
    }
#else
    Q_UNUSED(f)
#endif
    return true;
}

bool
MetrowerksMakefileGenerator::fixifyToMacPath(QString &p, QString &v, bool)
{
    v = "Absolute";
    if(p.indexOf(':') != -1) //guess its macish already
        return true;

    static QString st_volume;
    if(st_volume.isEmpty()) {
        st_volume = var("QMAKE_VOLUMENAME");
#if !defined(QWS) && defined(Q_OS_MAC)
        if(st_volume.isEmpty()) {
            uchar foo[512];
            HVolumeParam pb;
            memset(&pb, '\0', sizeof(pb));
            pb.ioVRefNum = 0;
            pb.ioNamePtr = foo;
            if(PBHGetVInfoSync((HParmBlkPtr)&pb) == noErr) {
                int len = foo[0];
                memcpy(foo,foo+1, len);
                foo[len] = '\0';
                st_volume = (char *)foo;
            }
        }
#endif
    }
    QString volume = st_volume;

    p = fixEnvVariables(p);
    if(p.startsWith("\"") && p.endsWith("\""))
        p = p.mid(1, p.length() - 2);
    if(p.isEmpty())
        return false;
    if(!p.endsWith("/"))
        p += "/";
    if(QDir::isRelativePath(p)) {
        if(p.startsWith("{")) {
            int eoc = p.indexOf('}');
            if(eoc == -1)
                return false;
            volume = p.mid(1, eoc - 1);
            p = p.right(p.length() - eoc - 1);
        } else {
            QFileInfo fi(fileInfo(p));
            if(fi.makeAbsolute()) //strange
                return false;
            p = fi.filePath();
        }
    }
    p = QDir::cleanPath(p);
    if(!volume.isEmpty())
        v = volume;
    p.replace("/", ":");
    if(p.right(1) != ":")
        p += ':';
    return true;
}

void
MetrowerksMakefileGenerator::processPrlFiles()
{
    QList<QMakeLocalFileName> libdirs;
    const QString lflags[] = { "QMAKE_LIBS", QString() };
    for(int i = 0; !lflags[i].isNull(); i++) {
        for(bool ret = false; true; ret = false) {
            QStringList l_out;
            QStringList &l = project->variables()[lflags[i]];
            for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
                QString opt = (*it);
                if(opt.startsWith("-")) {
                    if(opt.startsWith("-L")) {
                        libdirs.append(QMakeLocalFileName(opt.right(opt.length()-2)));
                    } else if(opt.left(2) == "-l") {
                        QString lib = opt.right(opt.length() - 2), prl;
                        for(QList<QMakeLocalFileName>::Iterator dep_it = libdirs.begin(); dep_it != libdirs.end(); ++dep_it) {
                            prl = (*dep_it).local() + Option::dir_sep + "lib" + lib;
                            if(processPrlFile(prl)) {
                                if(prl.startsWith((*dep_it).local()))
                                    prl.replace(0, (*dep_it).local().length(), (*dep_it).real());
                                QRegExp reg("^.*lib(" + lib + "[^.]*)\\." +
                                            project->first("QMAKE_EXTENSION_SHLIB") + "$");
                                if(reg.exactMatch(prl))
                                    prl = "-l" + reg.cap(1);
                                opt = prl;
                                ret = true;
                                break;
                            }
                        }
                    } else if(opt == "-framework") {
                        l_out.append(opt);
                        ++it;
                        opt = (*it);
                        QString prl = "/System/Library/Frameworks/" + opt +
                                      ".framework/" + opt;
                        if(processPrlFile(prl))
                            ret = true;
                    }
                    if(!opt.isEmpty())
                        l_out.append(opt);
                } else {
                    if(processPrlFile(opt))
                        ret = true;
                    if(!opt.isEmpty())
                        l_out.append(opt);
                }
            }
            if(ret)
                l = l_out;
            else
                break;
        }
    }
}

void
MetrowerksMakefileGenerator::processPrlVariable(const QString &var, const QStringList &l)
{
    if(var == "QMAKE_PRL_LIBS") {
        QStringList &out = project->variables()["QMAKE_LIBS"];
        for(QStringList::ConstIterator it = l.begin(); it != l.end(); ++it) {
            bool append = true;
            if((*it).startsWith("-")) {
                if((*it).startsWith("-l") || (*it).startsWith("-L")) {
                    append = out.indexOf((*it)) == -1;
                } else if((*it).startsWith("-framework")) {
                    ++it;
                    for(QStringList::ConstIterator outit = out.begin();
                        outit != out.end(); ++it) {
                        if((*outit) == "-framework") {
                            ++outit;
                            if((*outit) == (*it)) {
                                append = false;
                                break;
                            }
                        }
                    }
                }
            } else if(exists((*it))) {
                append = out.indexOf((*it));
            }
            if(append)
                out.append((*it));
        }
    } else {
        MakefileGenerator::processPrlVariable(var, l);
    }
}


bool
MetrowerksMakefileGenerator::openOutput(QFile &file, const QString &build) const
{
    QString outdir;
    if(!file.fileName().isEmpty()) {
        QFileInfo fi(fileInfo(file.fileName()));
        if(fi.isDir())
            outdir = file.fileName() + QDir::separator();
    }
    if(!outdir.isEmpty() || file.fileName().isEmpty())
        file.setFileName(outdir + "/" + project->first("QMAKE_ORIG_TARGET") + ".xml");
    return MakefileGenerator::openOutput(file, build);
}
