/****************************************************************************
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

int main(int argc, char **argv)
{
    // parse command line
    if(!Option::parseCommandLine(argc, argv))
        return 666;

    QDir sunworkshop42workaround = QDir::current();
    QString oldpwd = sunworkshop42workaround.currentDirPath();
#ifdef Q_WS_WIN
    if(!(oldpwd.length() == 3 && oldpwd[0].isLetter() && oldpwd.endsWith(":/")))
#endif
    {
        if(oldpwd.right(1) != QString(QChar(QDir::separator())))
            oldpwd += QDir::separator();
    }
    Option::output_dir = oldpwd; //for now this is the output dir

    if(Option::output.name() != "-") {
        QFileInfo fi(Option::output);
        QString dir;
        if(fi.isDir()) {
            dir = fi.filePath();
        } else {
            QString tmp_dir = fi.dirPath();
            if(!tmp_dir.isEmpty() && QFile::exists(tmp_dir))
                dir = tmp_dir;
        }
        if(!dir.isNull() && dir != ".")
            Option::output_dir = dir;
        if(QDir::isRelativePath(Option::output_dir))
            Option::output_dir.prepend(oldpwd);
    }

    QMakeProperty prop;
    if(Option::qmake_mode == Option::QMAKE_QUERY_PROPERTY || Option::qmake_mode == Option::QMAKE_SET_PROPERTY)
        return prop.exec() ? 0 : 101;

    QMakeProject proj(&prop);
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

            //setup pwd properly
            debug_msg(1, "Resetting dir to: %s", oldpwd.latin1());
            QDir::setCurrent(oldpwd); //reset the old pwd
            int di = fn.lastIndexOf(Option::dir_sep);
            if(di != -1) {
                debug_msg(1, "Changing dir to: %s", fn.left(di).latin1());
                if(!QDir::setCurrent(fn.left(di)))
                    fprintf(stderr, "Cannot find directory: %s\n", fn.left(di).latin1());
                fn = fn.right(fn.length() - di - 1);
            }

            // read project..
            if(!proj.read(fn)) {
                fprintf(stderr, "Error processing project file: %s\n",
                        fn == "-" ? "(stdin)" : (*pfile).latin1());
                exit_val = 2;
                continue;
            }
            if(Option::mkfile::do_preprocess) //no need to create makefile
                continue;

            // let Option post-process
            if(!Option::postProcessProject(&proj)) {
                fprintf(stderr, "Error post-processing project file: %s",
                        fn == "-" ? "(stdin)" : (*pfile).latin1());
                exit_val = 8;
                continue;
            }
        }

        MetaMakefileGenerator *mkfile = MetaMakefileGenerator::createMetaGenerator(&proj);
        if(mkfile && !mkfile->write(oldpwd)) {
            if(Option::qmake_mode == Option::QMAKE_GENERATE_PROJECT)
                fprintf(stderr, "Unable to generate project file.\n");
            else
                fprintf(stderr, "Unable to generate makefile for: %s\n", (*pfile).latin1());
            exit_val = 6;
        }
        delete mkfile;
        mkfile = NULL;
    }
    return exit_val;
}
