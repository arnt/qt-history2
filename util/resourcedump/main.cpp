#include <qresource.h>
#include <qfile.h>
#include <qstack.h>

#ifdef Q_OS_UNIX
# include <util.h>
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
        return 666;
    }
    QFile fi(argv[1]);
    if(!fi.open(IO_ReadOnly)) {
        fprintf(stderr, "Failure to open: %s\n", argv[1]);
        return 666;
    }
    const QByteArray bytes = fi.readAll();
    fi.close();
    for(int i = 0; i <= bytes.size() - 4; i++) {
        if(bytes[i] == 0x12 && bytes[i+1] == 0x15 && bytes[i+2] == 0x19 && bytes[i+3] == 0x78)
            (void)new QMetaResource((const uchar *)bytes.data()+i);
    }
    if(!QResource::find("/")) {
        fprintf(stderr, "** No resources!!!\n");
        return 666;
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
            return 666;
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
