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

#include <qresource.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qstack.h>

#ifdef Q_OS_DARWIN
# include <util.h>
#elif defined(Q_OS_UNIX)
# include <termio.h>
#endif
#ifdef Q_OS_UNIX
# include <sys/ioctl.h>
# include <stdio.h>
#endif

int
main(int argc, char **argv)
{
    int terminal_width = 80;
#ifdef Q_OS_UNIX
    struct winsize wsz;
    if(ioctl(1, TIOCGWINSZ, &wsz) == 0 && wsz.ws_col > 0)
        terminal_width = wsz.ws_col;
#endif
    const uint line_width = (terminal_width / 4) - 5;


    if(argc == 1) {
        fprintf(stderr, "%s <binary> [resources]\n", argv[0]);
        return 1;
    }
    QString fileName = argv[1];
#ifdef Q_OS_MAC
    if(!QFile::exists(fileName)) {
        QString exe = fileName + ".app/Contents/MacOS/" + fileName.section('/', -1);
        if(QFile::exists(exe))
            fileName = exe;
    } else if(fileName.endsWith(".app") && QFileInfo(fileName).isDir()) {
        QString exe = fileName + "/Contents/MacOS/" + 
                      fileName.left(fileName.length()-4).section('/', -1);
        if(QFile::exists(exe))
            fileName = exe;
    }
#endif
    QFile file(fileName);
    if(!file.open(QFile::ReadOnly)) {
        fprintf(stderr, "Failure to open: %s\n", fileName.latin1());
        return 1;
    }
    const QByteArray bytes = file.readAll();
    file.close();
    for(int i = 0; i <= bytes.size() - 4; i++) {
        if(bytes[i] == 0x12 && bytes[i+1] == 0x15 && 
           bytes[i+2] == 0x19 && bytes[i+3] == 0x78)
            (void)new QMetaResource((const uchar *)bytes.data()+i);
    }
    if(!QResource::find("/")) {
        fprintf(stderr, "** No resources!!!\n");
        return 1;
    }
    if(argc == 2) {
        QList<const QResource*> files;
        QStack<const QResource*> containers;
        containers.push(QResource::find("/")); //root
        while(!containers.isEmpty()) {
            const QResource *resource = containers.pop();
            QList<QResource *> children = resource->children();
            for(int i = 0; i < children.size(); i++) {
                if(children[i]->isContainer())
                    containers.push(children[i]);
                else 
                    files.append(children[i]);
            }
        } 
        if(files.isEmpty()) {
            fprintf(stderr, "** No resources!!!\n");
            return 1;
        } else {
            printf("************ Resources *************\n");
            for(int i = 0; i < files.count(); i++) {
                QString file;
                for(const QResource *res = files[i]; res; res = res->parent()) {
                    if(file.isEmpty())
                        file = res->name();
                    else if(res->name() == "/")
                        file.prepend("/");
                    else
                        file.prepend(res->name() + "/");
                }
                printf("%s\n", file.latin1());
            }
        }
    } else {
        for(int file = 2; file < argc; file++) {
            QResource *resource = QResource::find(argv[file]);
            if(!resource) {
                fprintf(stderr, "*** %s does not exist!\n", argv[file]);
                continue;
            }
            printf("****************** %s [%d] ****************\n", argv[file], resource->size());
            if(resource->isContainer()) {
                QList<QResource *> children = resource->children();
                for(int i = 0; i < children.size(); i++) 
                    printf("**Child: %s\n", children[i]->name().latin1());
                continue;
            }
            const uint datalen = resource->size();
            const uchar *data = resource->data();
            for(uint i = 0, off; i < datalen; i+=line_width) {
                uint chunks = qMin(line_width, datalen-i);
                for(off = 0; off < chunks; off++) {
                    if(off == line_width / 2)
                        printf(" ");
                    printf("%02x ", data[i+off]);
                }
                if(chunks < line_width/2) 
                    printf(" ");
                for(off = 0; off < line_width-chunks; off++) 
                    printf("   ");
                printf(" |");
                for(off = 0; off < chunks; off++)
                    printf("%c", (data[i+off] > 31 && data[i+off] < 126) ? data[i+off] : '.');
                for(off = 0; off < line_width-chunks; off++)
                    printf(" ");
                printf("|\n");
            }
        }
    }
    return 0;
}
