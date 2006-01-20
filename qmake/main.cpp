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

#include "project.h"
#include "property.h"
#include "option.h"
#include "metamakefile.h"
#include <qnamespace.h>
#include <qregexp.h>
#include <qdir.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

// for Borland, main is defined to qMain which breaks qmake
#undef main
#ifdef Q_OS_MAC
#endif

/* This is to work around lame implementation on Darwin. It has been noted that the getpwd(3) function
   is much too slow, and called much too often inside of Qt (every fileFixify). With this we use a locally
   cached copy because I can control all the times it is set (because Qt never sets the pwd under me).
*/
static QString pwd;
QString qmake_getpwd()
{
    if(pwd.isNull())
        pwd = QDir::currentPath();
    return pwd;
}
bool qmake_setpwd(const QString &p)
{
    if(QDir::setCurrent(p)) {
        pwd = QDir::currentPath();
        return true;
    }
    return false;
}

int main(int argc, char **argv)
{
    // parse command line
    int ret = Option::init(argc, argv);
    if(ret != Option::QMAKE_CMDLINE_SUCCESS) {
        if ((ret & Option::QMAKE_CMDLINE_ERROR) != 0)
            return 1;
        return 0;
    }

    QString oldpwd = qmake_getpwd();
#ifdef Q_WS_WIN
    if(!(oldpwd.length() == 3 && oldpwd[0].isLetter() && oldpwd.endsWith(":/")))
#endif
    {
        if(oldpwd.right(1) != QString(QChar(QDir::separator())))
            oldpwd += QDir::separator();
    }
    Option::output_dir = oldpwd; //for now this is the output dir

    if(Option::output.fileName() != "-") {
        QFileInfo fi(Option::output);
        QString dir;
        if(fi.isDir()) {
            dir = fi.filePath();
        } else {
            QString tmp_dir = fi.path();
            if(!tmp_dir.isEmpty() && QFile::exists(tmp_dir))
                dir = tmp_dir;
        }
        if(!dir.isNull() && dir != ".")
            Option::output_dir = dir;
        if(QDir::isRelativePath(Option::output_dir))
            Option::output_dir.prepend(oldpwd);
        Option::output_dir = QDir::cleanPath(Option::output_dir);
    }

    QMakeProperty prop;
    if(Option::qmake_mode == Option::QMAKE_QUERY_PROPERTY || Option::qmake_mode == Option::QMAKE_SET_PROPERTY)
        return prop.exec() ? 0 : 101;

    QMakeProject project(&prop);
    int exit_val = 0;
    QStringList files;
    if(Option::qmake_mode == Option::QMAKE_GENERATE_PROJECT)
        files << "(*hack*)"; //we don't even use files, but we do the for() body once
    else
        files = Option::mkfile::project_files;
    for(QStringList::Iterator pfile = files.begin(); pfile != files.end(); pfile++) {
        if(Option::qmake_mode == Option::QMAKE_GENERATE_MAKEFILE ||
           Option::qmake_mode == Option::QMAKE_GENERATE_PRL) {
            QString fn = Option::fixPathToLocalOS((*pfile));
            if(!QFile::exists(fn)) {
                fprintf(stderr, "Cannot find file: %s.\n", fn.toLatin1().constData());
                exit_val = 2;
                continue;
            }

            //setup pwd properly
            debug_msg(1, "Resetting dir to: %s", oldpwd.toLatin1().constData());
            qmake_setpwd(oldpwd); //reset the old pwd
            int di = fn.lastIndexOf(Option::dir_sep);
            if(di != -1) {
                debug_msg(1, "Changing dir to: %s", fn.left(di).toLatin1().constData());
                if(!qmake_setpwd(fn.left(di)))
                    fprintf(stderr, "Cannot find directory: %s\n", fn.left(di).toLatin1().constData());
                fn = fn.right(fn.length() - di - 1);
            }

            // read project..
            if(!project.read(fn)) {
                fprintf(stderr, "Error processing project file: %s\n",
                        fn == "-" ? "(stdin)" : (*pfile).toLatin1().constData());
                exit_val = 3;
                continue;
            }
            if(Option::mkfile::do_preprocess) //no need to create makefile
                continue;
        }

        MetaMakefileGenerator *mkfile = MetaMakefileGenerator::createMetaGenerator(&project, false);
        if(mkfile && !mkfile->write(oldpwd)) {
            if(Option::qmake_mode == Option::QMAKE_GENERATE_PROJECT)
                fprintf(stderr, "Unable to generate project file.\n");
            else
                fprintf(stderr, "Unable to generate makefile for: %s\n", (*pfile).toLatin1().constData());
            exit_val = 5;
        }
        delete mkfile;
        mkfile = NULL;
    }
    qmakeClearCaches();
    return exit_val;
}
