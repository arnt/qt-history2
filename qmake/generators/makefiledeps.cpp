#include "makefiledeps.h"
#include "option.h"
#include <qdir.h>
#include <qfileinfo.h>
#if defined(Q_OS_UNIX)
# include <unistd.h>
#else
# include <io.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#define QMAKE_EOL(x) (x == '\r' || x == '\n')

QMakeLocalFileName::QMakeLocalFileName(const QString &name) : is_null(name.isNull())
{
    if(!is_null) {
        real_name = name;
        real_name.replace("\"","");
        local_name = Option::fixPathToLocalOS(real_name);
        fixEnvVariables(local_name);
    }
}

struct SourceDependChildren;
struct SourceFile {
    SourceFile() : deps(0), uifile(0), mocable(0), traversed(0), exists(1),
                   moc_checked(0), dep_checked(0), included_count(0) { }
    QMakeLocalFileName file, mocfile;
    SourceDependChildren *deps;
    uint uifile : 1;
    uint mocable : 1, traversed : 1, exists : 1;
    uint moc_checked : 1,  dep_checked : 1;
    uchar included_count;
};
struct SourceDependChildren {
    SourceFile **children;
    int num_nodes, used_nodes;
    SourceDependChildren() : children(0), num_nodes(0), used_nodes(0) { }
    ~SourceDependChildren() { if(children) free(children); }
    void addChild(SourceFile *s) {
        if(num_nodes <= used_nodes) {
            num_nodes += 200;
            children = (SourceFile**)realloc(children, sizeof(SourceFile*)*(num_nodes));
        }
        children[used_nodes++] = s;
    }
};
class SourceFiles {
    struct SourceFileNode {
        ~SourceFileNode() { free(key); }
        char *key;
        SourceFileNode *next;
        SourceFile *file;
    } **nodes;
    int num_nodes;
    int hash(const char *);
public:
    SourceFiles();
    ~SourceFiles();

    SourceFile *lookupMocFile(const QString &mocfile);
    SourceFile *lookupFile(const char *);
    inline SourceFile *lookupFile(const QString &f) { return lookupFile(f.latin1()); }
    inline SourceFile *lookupFile(const QMakeLocalFileName &f) { return lookupFile(f.local().latin1()); }
    void addFile(SourceFile *, const char *k=0);
};
SourceFiles::SourceFiles()
{
    nodes = (SourceFileNode**)malloc(sizeof(SourceFileNode*)*(num_nodes=3037));
    for(int n = 0; n < num_nodes; n++)
        nodes[n] = 0;
}

SourceFiles::~SourceFiles()
{
    for(int n = 0; n < num_nodes; n++) {
        if(nodes[n]) {
            for(SourceFileNode *next = nodes[n]->next; next;) {
                SourceFileNode *next_next = next->next;
                delete next;
                next = next_next;
            }
            delete nodes[n];
        }
    }
}

int SourceFiles::hash(const char *file)
{
    uint h = 0, g;
    while (*file) {
        h = (h << 4) + *file;
        if ((g = (h & 0xf0000000)) != 0)
            h ^= g >> 23;
        h &= ~g;
        file++;
    }
    return h;
}

SourceFile *SourceFiles::lookupFile(const char *file)
{
    int h = hash(file) % num_nodes;
    for(SourceFileNode *p = nodes[h]; p; p = p->next) {
        if(!strcmp(p->key, file)) 
            return p->file;
    }
    return 0;
}

SourceFile *SourceFiles::lookupMocFile(const QString &mocfile)
{
    for(register int n = 0; n < num_nodes; n++) {
        if(nodes[n]) {
            for(SourceFileNode *next = nodes[n]; next; ) {
                if (next->file->mocable && mocfile == next->file->mocfile.local()) 
                    return next->file;
                next = next->next;
            }
        }
    }
    return 0;
}

void SourceFiles::addFile(SourceFile *p, const char *k)
{
    if(!k)
        k = p->file.local();
    int h = hash(k) % num_nodes;
    SourceFileNode *pn = new SourceFileNode;
    pn->key = strdup(k);
    pn->file = p;
    pn->next = nodes[h];
    nodes[h] = pn;
}

void QMakeSourceFileInfo::dependTreeWalker(SourceFile *node, SourceDependChildren *place)
{
    if(node->traversed)
        return;
    place->addChild(node);
    node->traversed = true; //set flag
    if(node->deps) {
        for(int i = 0; i < node->deps->used_nodes; i++)
            dependTreeWalker(node->deps->children[i], place);
    }
}

void QMakeSourceFileInfo::setDependencyPaths(const QList<QMakeLocalFileName> &l)
{
    depdirs = l;
}

QStringList QMakeSourceFileInfo::dependencies(const QString &file)
{
    QStringList ret;
    if(SourceFile *node = files->lookupFile(file)) {
        if(node->deps) {
            /* I stick them into a SourceDependChildren here because it is faster to just
               iterate over the list to stick them in the list, and reset the flag, then it is
               to loop over the tree (about 50% faster I saw) --Sam */
            SourceDependChildren place;
            for(int i = 0; i < node->deps->used_nodes; i++)
                dependTreeWalker(node->deps->children[i], &place);
            if(place.children) {
                for(int i = 0; i < place.used_nodes; i++) {
                    place.children[i]->traversed = false; //reset flag
                    ret.append(place.children[i]->file.local());
                }
           }
       }
    }
    return ret;
}

int
QMakeSourceFileInfo::included(const QString &file)
{
    if(SourceFile *node = files->lookupFile(file))
        return node->included_count;
    return 0;
}

bool QMakeSourceFileInfo::mocable(const QString &file)
{
    if(SourceFile *node = files->lookupFile(file))
        return node->mocable;
    return false;
}

QString QMakeSourceFileInfo::mocFile(const QString &file)
{
    if (!files)
        return QString();
    if(SourceFile *node = files->lookupFile(file)) 
        return node->mocfile.local();
    return QString();
}

QString QMakeSourceFileInfo::mocSource(const QString &mocfile)
{
    if (!files)
        return QString();
    if (SourceFile *node = files->lookupMocFile(mocfile))
        return node->file.local();
    return QString();
}

QMakeSourceFileInfo::QMakeSourceFileInfo()
{
    files = 0;
    spare_buffer = 0;
    spare_buffer_size = 0;
}

QMakeSourceFileInfo::~QMakeSourceFileInfo()
{
    delete files;
    if(spare_buffer) {
        free(spare_buffer);
        spare_buffer = 0;
        spare_buffer_size = 0;
    }
}

void QMakeSourceFileInfo::addSourceFiles(const QStringList &l, uchar seek, bool uifile)
{
    if(!files)
        files = new SourceFiles;
    for(QStringList::ConstIterator it = l.begin(); it != l.end(); ++it) {
        QMakeLocalFileName fn((*it));
        SourceFile *file = files->lookupFile(fn);
        if(!file) {
            file = new SourceFile;
            file->file = fixPathForFile(fn);
            files->addFile(file);
        } else {
            if(file->uifile != uifile)
                warn_msg(WarnLogic, "%s is marked as UI, then not!", (*it).latin1());
        }
        file->uifile = uifile;

        /* Do moc before dependency checking since some includes can come from
           moc_*.cpp files */
        if(seek & ADD_MOC) {
            file->mocable = true;
            file->mocfile = findFileForMoc(file->file);
        } else if(seek & SEEK_MOCS) {
            findMocs(file);
        }
        if(seek & SEEK_DEPS)
            findDeps(file);
    }
}

char *QMakeSourceFileInfo::getBuffer(int s) {
    if(!spare_buffer || spare_buffer_size < s)
        spare_buffer = (char *)realloc(spare_buffer, spare_buffer_size=s);
    return spare_buffer;
}

#ifdef Q_WS_WIN
#define S_ISDIR(x) (x & _S_IFDIR)
#endif

QMakeLocalFileName QMakeSourceFileInfo::findFileForMoc(const QMakeLocalFileName &)
{
    return QMakeLocalFileName();
}

QMakeLocalFileName QMakeSourceFileInfo::fixPathForFile(const QMakeLocalFileName &f)
{
    return f;
}

QMakeLocalFileName QMakeSourceFileInfo::findFileForDep(const QMakeLocalFileName &/*file*/)
{
    return QMakeLocalFileName();
}

bool QMakeSourceFileInfo::findDeps(SourceFile *file)
{
    if(file->dep_checked)
        return true;
    file->dep_checked = true;

    struct stat fst;
    char *buffer = 0;
    int buffer_len = 0;
    {
        int fd = open(file->file.local(), O_RDONLY);
        if(fd == -1 || fstat(fd, &fst) || S_ISDIR(fst.st_mode))
            return false;
        buffer = getBuffer(fst.st_size);
        for(int have_read = 0;
            (have_read = read(fd, buffer + buffer_len, fst.st_size - buffer_len));
            buffer_len += have_read);
        close(fd);
    }
    if(!buffer)
        return false;
    if(!file->deps)
        file->deps = new SourceDependChildren;

    int line_count = 0;
    for(int x = 0; x < buffer_len; x++) {
        bool try_local = true;
        char *inc = 0;
        if(file->uifile) {
            // skip whitespaces
            while(x < buffer_len && (*(buffer+x) == ' ' || *(buffer+x) == '\t'))
                x++;
            if(*(buffer + x) == '<') {
                x++;
                if(buffer_len >= x + 12 && !strncmp(buffer + x, "includehint", 11) &&
                   (*(buffer + x + 11) == ' ' || *(buffer + x + 11) == '>')) {
                    for(x += 11; *(buffer + x) != '>'; x++);
                    int inc_len = 0;
                    for(x += 1 ; *(buffer + x + inc_len) != '<'; inc_len++);
                    *(buffer + x + inc_len) = '\0';
                    inc = buffer + x;
                } else if(buffer_len >= x + 13 && !strncmp(buffer + x, "customwidget", 12) &&
                          (*(buffer + x + 12) == ' ' || *(buffer + x + 12) == '>')) {
                    for(x += 13; *(buffer + x) != '>'; x++); //skip up to >
                    while(x < buffer_len) {
                        for(x++; *(buffer + x) != '<'; x++); //skip up to <
                        x++;
                        if(buffer_len >= x + 7 && !strncmp(buffer+x, "header", 6) &&
                           (*(buffer + x + 6) == ' ' || *(buffer + x + 6) == '>')) {
                            for(x += 7; *(buffer + x) != '>'; x++); //skip up to >
                            int inc_len = 0;
                            for(x += 1 ; *(buffer + x + inc_len) != '<'; inc_len++);
                            *(buffer + x + inc_len) = '\0';
                            inc = buffer + x;
                            break;
                        } else if(buffer_len >= x + 14 && !strncmp(buffer+x, "/customwidget", 13) &&
                                  (*(buffer + x + 13) == ' ' || *(buffer + x + 13) == '>')) {
                            x += 14;
                            break;
                        }
                    }
                } else if(buffer_len >= x + 8 && !strncmp(buffer + x, "include", 7) &&
                          (*(buffer + x + 7) == ' ' || *(buffer + x + 7) == '>')) {
                    for(x += 8; *(buffer + x) != '>'; x++) {
                        if(buffer_len >= x + 9 && *(buffer + x) == 'i' &&
                           !strncmp(buffer + x, "impldecl", 8)) {
                            for(x += 8; *(buffer + x) != '='; x++);
                            if(*(buffer + x) != '=')
                                continue;
                            for(x++; *(buffer+x) == '\t' || *(buffer+x) == ' '; x++);
                            char quote = 0;
                            if(*(buffer+x) == '\'' || *(buffer+x) == '"') {
                                quote = *(buffer + x);
                                x++;
                            }
                            int val_len;
                            for(val_len = 0; true; val_len++) {
                                if(quote) {
                                    if(*(buffer+x+val_len) == quote)
                                        break;
                                } else if(*(buffer + x + val_len) == '>' ||
                                          *(buffer + x + val_len) == ' ') {
                                    break;
                                }
                            }
//?                            char saved = *(buffer + x + val_len);
                            *(buffer + x + val_len) = '\0';
                            if(!strcmp(buffer+x, "in implementation")) {
                                //### do this
                            }
                        }
                    }
                    int inc_len = 0;
                    for(x += 1 ; *(buffer + x + inc_len) != '<'; inc_len++);
                    *(buffer + x + inc_len) = '\0';
                    inc = buffer + x;
                }
            }
        } else {
            if(*(buffer + x) == '/') {
                x++;
                if(buffer_len >= x) {
                    if(*(buffer + x) == '/') { //c++ style comment
                        for(; x < buffer_len && !QMAKE_EOL(*(buffer + x)); x++);
                    } else if(*(buffer + x) == '*') { //c style comment
                        for(; x < buffer_len; x++) {
                            if(*(buffer + x) == '*') {
                                if(buffer_len >= (x+1) && *(buffer + (x+1)) == '/') {
                                    x += 2;
                                    break;
                                }
                            } else if(QMAKE_EOL(*(buffer + x))) {
                                line_count++;
                            }
                        }
                    }
                }
            }
            while(x < buffer_len && //Skip spaces
                  (*(buffer+x) == ' ' || *(buffer+x) == '\t'))
                x++;
            if(*(buffer + x) == '#') {
                x++;
                while(x < buffer_len && //Skip spaces after hash
                      (*(buffer+x) == ' ' || *(buffer+x) == '\t'))
                    x++;

                int keyword_len = 0;
                const char *keyword = buffer+x;
                while(x+keyword_len < buffer_len) {
                    if((*(buffer+x+keyword_len) == ' ' || *(buffer+x+keyword_len) == '\t')) {
                        for(x+=keyword_len; //skip spaces after keyword
                            x < buffer_len && (*(buffer+x) == ' ' || *(buffer+x) == '\t');
                            x++);
                        break;
                    } else if(QMAKE_EOL(*(buffer+x+keyword_len))) {
                        x += keyword_len;
                        keyword_len = 0;
                        break;
                    }
                    keyword_len++;
                }

                if(keyword_len == 7 && !strncmp(keyword, "include", keyword_len)) {
                    char term = *(buffer + x);
                    if(term == '<') {
                        try_local = false;
                        term = '>';
                    } else if(term != '"') { //wtf?
                        continue; 
                    }
                    x++;

                    int inc_len;
                    for(inc_len = 0; *(buffer + x + inc_len) != term && !QMAKE_EOL(*(buffer + x + inc_len)); inc_len++);
                    *(buffer + x + inc_len) = '\0';
                    inc = buffer + x;
                } else if(buffer_len >= x + 14 && !strncmp(buffer + x,  "qmake_warning ", 14)) {
                    for(x+=14; //skip spaces after keyword
                        x < buffer_len && (*(buffer+x) == ' ' || *(buffer+x) == '\t');
                        x++);
                    char term = 0;
                    if(*(buffer + x) == '"')
                        term = '"';
                    if(*(buffer + x) == '\'')
                        term = '\'';
                    if(term)
                        x++;

                    int msg_len;
                    for(msg_len = 0; (term && *(buffer + x + msg_len) != term) && !QMAKE_EOL(*(buffer + x + msg_len)); msg_len++);
                    *(buffer + x + msg_len) = '\0';
                    debug_msg(0, "%s:%d qmake_warning -- %s", file->file.local().latin1(), line_count, buffer+x);
                }
            }
        }

        if(inc) {
            bool exists = false;
            QMakeLocalFileName lfn(inc);
            if(QDir::isRelativePath(lfn.real())) {
                if(try_local) {
                    QString dir = QFileInfo(file->file.local()).path();
                    if(QDir::isRelativePath(dir)) 
                        dir.prepend(QDir::currentPath() + "/");
                    if(!dir.endsWith("/"))
                        dir += "/";
                    QMakeLocalFileName f(dir + lfn.local());
                    if(QFile::exists(f.real())) {
                        lfn = fixPathForFile(f);
                        exists = true;
                    }
                } 
                if(!exists) { //path lookup
                    for(QList<QMakeLocalFileName>::Iterator it = depdirs.begin(); it != depdirs.end(); ++it) {
                        QMakeLocalFileName f((*it).real() + Option::dir_sep + lfn.real());
                        if(!stat(f.local(), &fst) && !S_ISDIR(fst.st_mode)) {
                            lfn = fixPathForFile(f);
                            exists = true;
                        }
                    }
                }
                if(!exists) { //heuristic lookup
                    lfn = findFileForDep(QMakeLocalFileName(inc));
                    exists = !lfn.isNull();
                }
            } else {
                exists = QFile::exists(lfn.real());
            }
            if(!lfn.isNull()) {
                SourceFile *dep = files->lookupFile(lfn);
                if(!dep) {
                    dep = new SourceFile;
                    dep->file = lfn;
                    dep->exists = exists;
                    files->addFile(dep);
                } else if(dep->exists != exists) {//not really possible, but seems dangerous -Sam
                    warn_msg(WarnLogic, "%s is found to exist after not existing before!", 
                             lfn.local().latin1());
                }
                dep->included_count++;
                if(dep->exists) {
                    debug_msg(5, "%s:%d Found dependency to %s", file->file.real().latin1(),
                              line_count, dep->file.local().latin1());
                    file->deps->addChild(dep);
                }
            }
        }
        //read past new line now..
        for(; x < buffer_len && !QMAKE_EOL(*(buffer + x)); x++);
        line_count++;
    }
    //done last because buffer is shared
    for(int i = 0; i < file->deps->used_nodes; i++) { //now recurse
        if(!file->deps->children[i]->deps)
            findDeps(file->deps->children[i]);
    }
    return true;
}

bool QMakeSourceFileInfo::findMocs(SourceFile *file)
{
    if(file->moc_checked)
        return true;
    file->moc_checked = true;

    int buffer_len;
    char *buffer = 0;
    {
        struct stat fst;
        int fd = open(file->file.local().latin1(), O_RDONLY);
        if(fd == -1 || fstat(fd, &fst) || S_ISDIR(fst.st_mode))
            return false; //shouldn't happen
        buffer = getBuffer(fst.st_size);
        for(int have_read = buffer_len = 0;
            (have_read = read(fd, buffer + buffer_len, fst.st_size - buffer_len));
            buffer_len += have_read);
        close(fd);
    }

    debug_msg(2, "findMocs: %s", file->file.local().latin1());
    int line_count = 1;
    bool ignore_qobject = false;
 /* qmake ignore Q_OBJECT */
#define OBJ_LEN 8 //strlen("Q_OBJECT")
    for(int x = 0; x < (buffer_len-OBJ_LEN); x++) {
        if(*(buffer + x) == '/') {
            x++;
            if(buffer_len >= x) {
                if(*(buffer + x) == '/') { //c++ style comment
                    for(;x < buffer_len && !QMAKE_EOL(*(buffer + x)); x++);
                    line_count++;
                } else if(*(buffer + x) == '*') { //c style comment
                    for(;x < buffer_len; x++) {
                        if(*(buffer + x) == 't' || *(buffer + x) == 'q') { //ignore
                            if(buffer_len >= (x + 20)) {
                                if(!strncmp(buffer + x + 1, "make ignore Q_OBJECT", 20)) {
                                    debug_msg(2, "Mocgen: %s:%d Found \"qmake ignore Q_OBJECT\"",
                                              file->file.real().latin1(), line_count);
                                    x += 20;
                                    ignore_qobject = true;
                                }
                            }
                        } else if(*(buffer + x) == '*') {
                            if(buffer_len >= (x+1) && *(buffer + (x+1)) == '/') {
                                x += 2;
                                break;
                            }
                        } else if(QMAKE_EOL(*(buffer + x))) {
                            line_count++;
                        }
                    }
                }
            }
        }
#define SYMBOL_CHAR(x) ((x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z') || \
                        (x >= '0' && x <= '9') || x == '_')

        bool interesting = *(buffer+x) == 'Q' && (!strncmp(buffer+x, "Q_OBJECT", OBJ_LEN));
        if(interesting) {
            int len = 0;
            if(!strncmp(buffer+x, "Q_OBJECT", OBJ_LEN)) {
                if(ignore_qobject) {
                    debug_msg(2, "Mocgen: %s:%d Ignoring Q_OBJECT", file->file.real().latin1(), line_count);
                    interesting = false;
                }
                len=OBJ_LEN;
            }
            if(SYMBOL_CHAR(*(buffer+x+len)))
                interesting = false;
            if(interesting) {
                *(buffer+x+len) = '\0';
                debug_msg(2, "Mocgen: %s:%d Found MOC symbol %s", file->file.real().latin1(),
                          line_count, buffer+x);
                file->mocable = true;
                file->mocfile = findFileForMoc(file->file);
                break;
            }
        }

        while(x < buffer_len && SYMBOL_CHAR(*(buffer+x)))
            x++;
        if(QMAKE_EOL(*(buffer+x)))
            line_count++;
    }
#undef OBJ_LEN
    return true;
}


