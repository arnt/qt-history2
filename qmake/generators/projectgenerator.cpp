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

#include "projectgenerator.h"
#include "option.h"
#include <qdatetime.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qregexp.h>

QString project_builtin_regx() //calculate the builtin regular expression..
{
    QString ret;
    QStringList builtin_exts(".c");
    builtin_exts << Option::ui_ext << Option::yacc_ext << Option::lex_ext << ".ts" << ".qrc";
    builtin_exts += Option::h_ext + Option::cpp_ext;
    for(QStringList::Iterator ext_it = builtin_exts.begin();
        ext_it != builtin_exts.end(); ++ext_it) {
        if(!ret.isEmpty())
            ret += "; ";
        ret += QString("*") + (*ext_it);
    }
    return ret;
}

ProjectGenerator::ProjectGenerator() : MakefileGenerator(), init_flag(false)
{
}

void
ProjectGenerator::init()
{
    if(init_flag)
        return;
    int file_count = 0;
    init_flag = true;

    QMap<QString, QStringList> &v = project->variables();
    QString templ = Option::user_template.isEmpty() ? QString("app") : Option::user_template;
    if(!Option::user_template_prefix.isEmpty())
        templ.prepend(Option::user_template_prefix);
    v["TEMPLATE_ASSIGN"] += templ;

    //figure out target
    if(Option::output.fileName() == "-")
        v["TARGET"] = QStringList("unknown");

    //the scary stuff
    if(project->first("TEMPLATE_ASSIGN") != "subdirs") {
        QString builtin_regex = project_builtin_regx();
        QStringList dirs = Option::projfile::project_dirs;
        if(Option::projfile::do_pwd) {
            if(!v["INCLUDEPATH"].contains("."))
                v["INCLUDEPATH"] += ".";
            dirs.prepend(qmake_getpwd());
        }

        for(int i = 0; i < dirs.count(); ++i) {
            QString dir, regex, pd = dirs.at(i);
            bool add_depend = false;
            if(exists(pd)) {
                QFileInfo fi(fileInfo(pd));
                if(fi.isDir()) {
                    dir = pd;
                    add_depend = true;
                    if(dir.right(1) != Option::dir_sep)
                        dir += Option::dir_sep;
                    if(Option::recursive) {
                        QStringList files = QDir(dir).entryList(QDir::Files);
                        for(int i = 0; i < (int)files.count(); i++) {
                            if(files[i] != "." && files[i] != "..")
                                dirs.append(dir + files[i] + QDir::separator() + builtin_regex);
                        }
                    }
                    regex = builtin_regex;
                } else {
                    QString file = pd;
                    int s = file.lastIndexOf(Option::dir_sep);
                    if(s != -1)
                        dir = file.left(s+1);
                    if(addFile(file)) {
                        add_depend = true;
                        file_count++;
                    }
                }
            } else { //regexp
                regex = pd;
            }
            if(!regex.isEmpty()) {
                int s = regex.lastIndexOf(Option::dir_sep);
                if(s != -1) {
                    dir = regex.left(s+1);
                    regex = regex.right(regex.length() - (s+1));
                }
                if(Option::recursive) {
                    QStringList entries = QDir(dir).entryList(QDir::Dirs);
                    for(int i = 0; i < (int)entries.count(); i++) {
                        if(entries[i] != "." && entries[i] != "..") {
                            dirs.append(dir + entries[i] + QDir::separator() + regex);
                        }
                    }
                }
                QStringList files = QDir(dir).entryList(QDir::nameFiltersFromString(regex));
                for(int i = 0; i < (int)files.count(); i++) {
                    QString file = dir + files[i];
                    if (addFile(file)) {
                        add_depend = true;
                        file_count++;
                    }
                }
            }
            if(add_depend && !dir.isEmpty() && !v["DEPENDPATH"].contains(dir)) {
                QFileInfo fi(fileInfo(dir));
                if(fi.absoluteFilePath() != qmake_getpwd())
                    v["DEPENDPATH"] += fileFixify(dir);
            }
        }
    }
    if(!file_count) { //shall we try a subdir?
        QStringList knownDirs = Option::projfile::project_dirs;
        if(Option::projfile::do_pwd)
            knownDirs.prepend(".");
        const QString out_file = fileFixify(Option::output.fileName());
        for(int i = 0; i < knownDirs.count(); ++i) {
            QString pd = knownDirs.at(i);
            if(exists(pd)) {
                QString newdir = pd;
                QFileInfo fi(fileInfo(newdir));
                if(fi.isDir()) {
                    newdir = fileFixify(newdir);
                    QStringList &subdirs = v["SUBDIRS"];
                    if(exists(fi.filePath() + QDir::separator() + fi.fileName() + Option::pro_ext) &&
                       !subdirs.contains(newdir)) {
                        subdirs.append(newdir);
                    } else {
                        QStringList profiles = QDir(newdir).entryList(QStringList("*" + Option::pro_ext), QDir::Files);
                        for(int i = 0; i < (int)profiles.count(); i++) {
                            QString nd = newdir;
                            if(nd == ".")
                                nd = "";
                            else if(!nd.isEmpty() && !nd.endsWith(QString(QChar(QDir::separator()))))
                                nd += QDir::separator();
                            nd += profiles[i];
                            fileFixify(nd);
                            if(profiles[i] != "." && profiles[i] != ".." &&
                               !subdirs.contains(nd) && !out_file.endsWith(nd))
                                subdirs.append(nd);
                        }
                    }
                    if(Option::recursive) {
                        QStringList dirs = QDir(newdir).entryList(QDir::Dirs);
                        for(int i = 0; i < (int)dirs.count(); i++) {
                            QString nd = fileFixify(newdir + QDir::separator() + dirs[i]);
                            if(dirs[i] != "." && dirs[i] != ".." && !knownDirs.contains(nd))
                                knownDirs.append(nd);
                        }
                    }
                }
            } else { //regexp
                QString regx = pd, dir;
                int s = regx.lastIndexOf(Option::dir_sep);
                if(s != -1) {
                    dir = regx.left(s+1);
                    regx = regx.right(regx.length() - (s+1));
                }
                QStringList files = QDir(dir).entryList(QDir::nameFiltersFromString(regx), QDir::Dirs);
                QStringList &subdirs = v["SUBDIRS"];
                for(int i = 0; i < (int)files.count(); i++) {
                    QString newdir(dir + files[i]);
                    QFileInfo fi(fileInfo(newdir));
                    if(fi.fileName() != "." && fi.fileName() != "..") {
                        newdir = fileFixify(newdir);
                        if(exists(fi.filePath() + QDir::separator() + fi.fileName() + Option::pro_ext) &&
                           !subdirs.contains(newdir)) {
                           subdirs.append(newdir);
                        } else {
                            QStringList profiles = QDir(newdir).entryList(QStringList("*" + Option::pro_ext), QDir::Files);
                            for(int i = 0; i < (int)profiles.count(); i++) {
                                QString nd = newdir + QDir::separator() + files[i];
                                fileFixify(nd);
                                if(files[i] != "." && files[i] != ".." && !subdirs.contains(nd)) {
                                    if(newdir + files[i] != Option::output_dir + Option::output.fileName())
                                        subdirs.append(nd);
                                }
                            }
                        }
                        if(Option::recursive && !knownDirs.contains(newdir))
                            knownDirs.append(newdir);
                    }
                }
            }
        }
        v["TEMPLATE_ASSIGN"] = QStringList("subdirs");
        return;
    }

    //setup deplist
    QList<QMakeLocalFileName> deplist;
    {
        QStringList &d = v["DEPENDPATH"];
        for(QStringList::Iterator it = d.begin(); it != d.end(); ++it)
            deplist.append(QMakeLocalFileName((*it)));
    }
    setDependencyPaths(deplist);

    QStringList &h = v["HEADERS"];
    bool no_qt_files = true;
    QString srcs[] = { "SOURCES", "YACCSOURCES", "LEXSOURCES", "FORMS", QString() };
    for(int i = 0; !srcs[i].isNull(); i++) {
        QStringList &l = v[srcs[i]];
        QMakeSourceFileInfo::SourceFileType type = QMakeSourceFileInfo::TYPE_C;
        QMakeSourceFileInfo::addSourceFiles(l, QMakeSourceFileInfo::SEEK_DEPS, type);
        for(QStringList::Iterator val_it = l.begin(); val_it != l.end(); ++val_it) {
            QStringList tmp = QMakeSourceFileInfo::dependencies((*val_it));
            if(!tmp.isEmpty()) {
                for(QStringList::Iterator dep_it = tmp.begin(); dep_it != tmp.end(); ++dep_it) {
                    QString file_dir = (*dep_it).section(Option::dir_sep, 0, -2),
                        file_no_path = (*dep_it).section(Option::dir_sep, -1);
                    if(!file_dir.isEmpty()) {
                        for(QList<QMakeLocalFileName>::Iterator it = deplist.begin(); it != deplist.end(); ++it) {
                            if((*it).local() == file_dir && !v["INCLUDEPATH"].contains((*it).real()))
                                v["INCLUDEPATH"] += (*it).real();
                        }
                    }
                    if(no_qt_files && file_no_path.indexOf(QRegExp("^q[a-z_0-9].h$")) != -1)
                        no_qt_files = false;
                    QString h_ext;
                    for(QStringList::Iterator hit = Option::h_ext.begin();
                        hit != Option::h_ext.end(); ++hit) {
                        if((*dep_it).endsWith((*hit))) {
                            h_ext = (*hit);
                            break;
                        }
                    }
                    if(!h_ext.isEmpty()) {
                        for(QStringList::Iterator cppit = Option::cpp_ext.begin();
                            cppit != Option::cpp_ext.end(); ++cppit) {
                            QString src((*dep_it).left((*dep_it).length() - h_ext.length()) +
                                        (*cppit));
                            if(exists(src)) {
                                bool exists = false;
                                QStringList &srcl = v["SOURCES"];
                                for(QStringList::Iterator src_it = srcl.begin();
                                    src_it != srcl.end(); ++src_it) {
                                    if((*src_it).toLower() == src.toLower()) {
                                        exists = true;
                                        break;
                                    }
                                }
                                if(!exists)
                                    srcl.append(src);
                            }
                        }
                    } else if((*dep_it).endsWith(Option::lex_ext) &&
                              file_no_path.startsWith(Option::lex_mod)) {
                        addConfig("lex_included");
                    }
                    if(!h.contains((*dep_it)))
                        h += (*dep_it);
                }
            }
        }
    }

#if 0
    //if we find a file that matches an forms it needn't be included in the project
    QStringList &u = v["FORMS"];
    QString no_ui[] = { "SOURCES", "HEADERS", QString() };
    {
        for(int i = 0; !no_ui[i].isNull(); i++) {
            QStringList &l = v[no_ui[i]];
            for(QStringList::Iterator val_it = l.begin(); val_it != l.end();) {
                bool found = false;
                for(QStringList::Iterator ui_it = u.begin(); ui_it != u.end(); ++ui_it) {
                    QString s1 = (*val_it).right((*val_it).length() - ((*val_it).lastIndexOf(Option::dir_sep) + 1));
                    if(s1.lastIndexOf('.') != -1)
                        s1 = s1.left(s1.lastIndexOf('.')) + Option::ui_ext;
                    QString u1 = (*ui_it).right((*ui_it).length() - ((*ui_it).lastIndexOf(Option::dir_sep) + 1));
                    if(s1 == u1) {
                        found = true;
                        break;
                    }
                }
                if(!found && (*val_it).endsWith(Option::cpp_moc_ext))
                    found = true;
                if(found)
                    val_it = l.erase(val_it);
                else
                    ++val_it;
            }
        }
    }
#endif
}

bool
ProjectGenerator::writeMakefile(QTextStream &t)
{
    t << "######################################################################" << endl;
    t << "# Automatically generated by qmake (" << qmake_version() << ") " << QDateTime::currentDateTime().toString() << endl;
    t << "######################################################################" << endl << endl;
    if(!Option::user_configs.isEmpty())
        t << "CONFIG += " << Option::user_configs.join(" ") << endl;
    QStringList::Iterator it;
    for(it = Option::before_user_vars.begin(); it != Option::before_user_vars.end(); ++it)
        t << (*it) << endl;
    t << getWritableVar("TEMPLATE_ASSIGN", false);
    if(project->first("TEMPLATE_ASSIGN") == "subdirs") {
        t << endl << "# Directories" << "\n"
          << getWritableVar("SUBDIRS");
    } else {
        t << getWritableVar("TARGET")
          << getWritableVar("CONFIG", false)
          << getWritableVar("CONFIG_REMOVE", false)
          << getWritableVar("DEPENDPATH")
          << getWritableVar("INCLUDEPATH") << endl;

        t << "# Input" << "\n";
        t << getWritableVar("HEADERS")
          << getWritableVar("FORMS")
          << getWritableVar("LEXSOURCES")
          << getWritableVar("YACCSOURCES")
          << getWritableVar("SOURCES")
          << getWritableVar("RESOURCES")
          << getWritableVar("TRANSLATIONS");
    }
    for(it = Option::after_user_vars.begin(); it != Option::after_user_vars.end(); ++it)
        t << (*it) << endl;
    return true;
}

bool
ProjectGenerator::addConfig(const QString &cfg, bool add)
{
    QString where = "CONFIG";
    if(!add)
        where = "CONFIG_REMOVE";
    if(!project->variables()[where].contains(cfg)) {
        project->variables()[where] += cfg;
        return true;
    }
    return false;
}

bool
ProjectGenerator::addFile(QString file)
{
    file = fileFixify(file, qmake_getpwd());
    QString dir;
    int s = file.lastIndexOf(Option::dir_sep);
    if(s != -1)
        dir = file.left(s+1);
    if(file.mid(dir.length(), Option::h_moc_mod.length()) == Option::h_moc_mod)
        return false;

    QString where;
    for(QStringList::Iterator cppit = Option::cpp_ext.begin(); cppit != Option::cpp_ext.end(); ++cppit) {
        if(file.endsWith((*cppit))) {
            if(exists(file.left(file.length() - (*cppit).length()) + Option::ui_ext))
                return false;
            else
                where = "SOURCES";
            break;
        }
    }
    if(where.isEmpty()) {
        for(QStringList::Iterator hit = Option::h_ext.begin(); hit != Option::h_ext.end(); ++hit) {
            if(file.endsWith((*hit))) {
                where = "HEADERS";
                break;
            }
        }
    }
    if(where.isEmpty()) {
        if(file.endsWith(Option::ui_ext))
            where = "FORMS";
        else if(file.endsWith(".c"))
            where = "SOURCES";
        else if(file.endsWith(Option::lex_ext))
            where = "LEXSOURCES";
        else if(file.endsWith(Option::yacc_ext))
            where = "YACCSOURCES";
        else if(file.endsWith(".ts"))
            where = "TRANSLATIONS";
        else if(file.endsWith(".qrc"))
            where = "RESOURCES";
    }

    QString newfile = fileFixify(file);
    if(Option::dir_sep != QLatin1String("/"))
        newfile = newfile.replace(Option::dir_sep, QLatin1String("/"));
    if(!where.isEmpty() && !project->variables()[where].contains(file)) {
        project->variables()[where] += newfile;
        return true;
    }
    return false;
}

QString
ProjectGenerator::getWritableVar(const QString &v, bool fixPath)
{
    QStringList &vals = project->variables()[v];
    if(vals.isEmpty())
        return "";

    QString ret;
    if(v.endsWith("_REMOVE"))
        ret = v.left(v.length() - 7) + " -= ";
    else if(v.endsWith("_ASSIGN"))
        ret = v.left(v.length() - 7) + " = ";
    else
        ret = v + " += ";
    QString join = vals.join(" ");
    if(ret.length() + join.length() > 80) {
        QString spaces;
        for(int i = 0; i < ret.length(); i++)
            spaces += " ";
        join = vals.join(" \\\n" + spaces);
    }
#if 0
    // ### Commented out for now so that project generation works.
    // Sam: it had to do with trailing \'s (ie considered continuation lines)
    if(fixPath)
        join = join.replace("\\", "/");
#else
    Q_UNUSED(fixPath);
#endif
    return ret + join + "\n";
}

bool
ProjectGenerator::openOutput(QFile &file, const QString &build) const
{
    QString outdir;
    if(!file.fileName().isEmpty()) {
        QFileInfo fi(fileInfo(file.fileName()));
        if(fi.isDir())
            outdir = fi.path() + QDir::separator();
    }
    if(!outdir.isEmpty() || file.fileName().isEmpty()) {
        QString dir = qmake_getpwd();
        int s = dir.lastIndexOf('/');
        if(s != -1)
            dir = dir.right(dir.length() - (s + 1));
        file.setFileName(outdir + dir + Option::pro_ext);
    }
    return MakefileGenerator::openOutput(file, build);
}
