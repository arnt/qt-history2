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
#include <qbuffer.h>
#define QMAKE_EOL(x) (x == '\r' || x == '\n')

//#define QMAKE_USE_CACHE

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
    SourceFile() : deps(0), type(QMakeSourceFileInfo::TYPE_UNKNOWN),
                   mocable(0), traversed(0), exists(1),
                   moc_checked(0), dep_checked(0), included_count(0) { }
    QMakeLocalFileName file;
    SourceDependChildren *deps;
    QMakeSourceFileInfo::SourceFileType type;
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
    int hash(const char *);
public:
    SourceFiles();
    ~SourceFiles();

    SourceFile *lookupFile(const char *);
    inline SourceFile *lookupFile(const QString &f) { return lookupFile(f.toLatin1().constData()); }
    inline SourceFile *lookupFile(const QMakeLocalFileName &f) { return lookupFile(f.local().toLatin1().constData()); }
    void addFile(SourceFile *, const char *k=0);

    struct SourceFileNode {
        ~SourceFileNode() { free(key); }
        char *key;
        SourceFileNode *next;
        SourceFile *file;
    } **nodes;
    int num_nodes;
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

void SourceFiles::addFile(SourceFile *p, const char *k)
{
    QByteArray ba = p->file.local().toLatin1();
    if(!k)
        k = ba;
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
    if(!files)
        return ret;

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
                    ret.append(place.children[i]->file.real());
                }
           }
       }
    }
    return ret;
}

int
QMakeSourceFileInfo::included(const QString &file)
{
    if (!files)
        return 0;

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

QMakeSourceFileInfo::QMakeSourceFileInfo(const QString &cf)
{
    //quick project lookups
    files = 0;
    files_changed = false;

    //buffer
    spare_buffer = 0;
    spare_buffer_size = 0;

    //cache
    cachefile = cf;
    if(!cachefile.isEmpty())
        loadCache(cachefile);
}

QMakeSourceFileInfo::~QMakeSourceFileInfo()
{
    //cache
    if(!cachefile.isEmpty() /*&& files_changed*/)
        saveCache(cachefile);

    //buffer
    if(spare_buffer) {
        free(spare_buffer);
        spare_buffer = 0;
        spare_buffer_size = 0;
    }

    //quick project lookup
    delete files;
}

void QMakeSourceFileInfo::setCacheFile(const QString &cf)
{
    cachefile = cf;
    loadCache(cachefile);
}

void QMakeSourceFileInfo::addSourceFiles(const QStringList &l, uchar seek,
                                         QMakeSourceFileInfo::SourceFileType type)
{
    for(QStringList::ConstIterator it = l.begin(); it != l.end(); ++it)
        addSourceFile((*it), seek, type);
}
void QMakeSourceFileInfo::addSourceFile(const QString &f, uchar seek,
                                        QMakeSourceFileInfo::SourceFileType type)
{
    if(!files)
        files = new SourceFiles;

    QMakeLocalFileName fn(f);
    SourceFile *file = files->lookupFile(fn);
    if(!file) {
        file = new SourceFile;
        file->file = fn;
        files->addFile(file);
    } else {
        if(file->type != type && file->type != TYPE_UNKNOWN)
            warn_msg(WarnLogic, "%s is marked as %d, then %d!", f.toLatin1().constData(),
                     file->type, type);
    }
    file->type = type;

/* Do moc before dependency checking since some includes can come from
   moc_*.cpp files */
    if(seek & ADD_MOC) {
        files_changed = true;
        file->mocable = true;
    } else if(seek & SEEK_MOCS) {
        findMocs(file);
    }
    if(seek & SEEK_DEPS)
        findDeps(file);
}

char *QMakeSourceFileInfo::getBuffer(int s) {
    if(!spare_buffer || spare_buffer_size < s)
        spare_buffer = (char *)realloc(spare_buffer, spare_buffer_size=s);
    return spare_buffer;
}

#ifdef Q_WS_WIN
#define S_ISDIR(x) (x & _S_IFDIR)
#endif

QMakeLocalFileName QMakeSourceFileInfo::fixPathForFile(const QMakeLocalFileName &f, bool)
{
    return f;
}

QMakeLocalFileName QMakeSourceFileInfo::findFileForDep(const QMakeLocalFileName &/*dep*/,
                                                       const QMakeLocalFileName &/*file*/)
{
    return QMakeLocalFileName();
}

bool QMakeSourceFileInfo::findDeps(SourceFile *file)
{
    if(file->dep_checked)
        return true;
    files_changed = true;
    file->dep_checked = true;

    struct stat fst;
    char *buffer = 0;
    int buffer_len = 0;
    {
        int fd = open(fixPathForFile(file->file, true).local().toLatin1().constData(), O_RDONLY);
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
        if(file->type == QMakeSourceFileInfo::TYPE_UI) {
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
        } else if(file->type == QMakeSourceFileInfo::TYPE_QRC) {
        } else if(file->type == QMakeSourceFileInfo::TYPE_C) {
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
                    debug_msg(0, "%s:%d qmake_warning -- %s", file->file.local().toLatin1().constData(), line_count, buffer+x);
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
                        dir.prepend(qmake_getpwd() + "/");
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
                        if(!stat(f.local().toLocal8Bit().constData(), &fst) && !S_ISDIR(fst.st_mode)) {
                            lfn = fixPathForFile(f);
                            exists = true;
                        }
                    }
                }
                if(!exists) { //heuristic lookup
                    lfn = findFileForDep(QMakeLocalFileName(inc), file->file);
                    if((exists = !lfn.isNull()))
                        lfn = fixPathForFile(lfn);
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
                    dep->type = QMakeSourceFileInfo::TYPE_C;
                    files->addFile(dep);
                } else {
                    dep->exists = exists;
#if 0
                    warn_msg(WarnLogic, "%s is found to exist after not existing before!",
                             lfn.local().toLatin1());
#endif
                }
                dep->included_count++;
                if(dep->exists) {
                    debug_msg(5, "%s:%d Found dependency to %s", file->file.real().toLatin1().constData(),
                              line_count, dep->file.local().toLatin1().constData());
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
    files_changed = true;
    file->moc_checked = true;

    int buffer_len;
    char *buffer = 0;
    {
        struct stat fst;
        int fd = open(fixPathForFile(file->file, true).local().toLocal8Bit().constData(), O_RDONLY);
        if(fd == -1 || fstat(fd, &fst) || S_ISDIR(fst.st_mode))
            return false; //shouldn't happen
        buffer = getBuffer(fst.st_size);
        for(int have_read = buffer_len = 0;
            (have_read = read(fd, buffer + buffer_len, fst.st_size - buffer_len));
            buffer_len += have_read);
        close(fd);
    }

    debug_msg(2, "findMocs: %s", file->file.local().toLatin1().constData());
    int line_count = 1;
    bool ignore_qobject = false, ignore_qgadget = false;
 /* qmake ignore Q_GADGET */
#define Q_GADGET_LEN 8 //strlen("Q_GADGET")
 /* qmake ignore Q_OBJECT */
#define Q_OBJECT_LEN 8 //strlen("Q_OBJECT")
    for(int x = 0; x < (buffer_len-Q_OBJECT_LEN); x++) {
        if(*(buffer + x) == '/') {
            x++;
            if(buffer_len >= x) {
                if(*(buffer + x) == '/') { //c++ style comment
                    for(;x < buffer_len && !QMAKE_EOL(*(buffer + x)); x++);
                    line_count++;
                } else if(*(buffer + x) == '*') { //c style comment
                    for(;x < buffer_len; x++) {
                        if(*(buffer + x) == 't' || *(buffer + x) == 'q') { //ignore
                            if(buffer_len >= (x + 20) &&
                               !strncmp(buffer + x + 1, "make ignore Q_OBJECT", 20)) {
                                debug_msg(2, "Mocgen: %s:%d Found \"qmake ignore Q_OBJECT\"",
                                          file->file.real().toLatin1().constData(), line_count);
                                x += 20;
                                ignore_qobject = true;
                            } else if(buffer_len >= (x + 20) &&
                                      !strncmp(buffer + x + 1, "make ignore Q_GADGET", 20)) {
                                debug_msg(2, "Mocgen: %s:%d Found \"qmake ignore Q_GADGET\"",
                                          file->file.real().toLatin1().constData(), line_count);
                                x += 20;
                                ignore_qgadget = true;
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

        bool interesting = *(buffer+x) == 'Q' &&
                           (!strncmp(buffer+x, "Q_OBJECT", Q_OBJECT_LEN) ||
                            !strncmp(buffer+x, "Q_GADGET", Q_GADGET_LEN));
        if(interesting) {
            int len = 0;
            if(!strncmp(buffer+x, "Q_OBJECT", Q_OBJECT_LEN)) {
                if(ignore_qobject) {
                    debug_msg(2, "Mocgen: %s:%d Ignoring Q_OBJECT", file->file.real().toLatin1().constData(), line_count);
                    interesting = false;
                }
                len=Q_OBJECT_LEN;
            } else if(!strncmp(buffer+x, "Q_GADGET", Q_GADGET_LEN)) {
                if(ignore_qgadget) {
                    debug_msg(2, "Mocgen: %s:%d Ignoring Q_GADGET", file->file.real().toLatin1().constData(), line_count);
                    interesting = false;
                }
                len=Q_GADGET_LEN;
            }

            if(SYMBOL_CHAR(*(buffer+x+len)))
                interesting = false;
            if(interesting) {
                *(buffer+x+len) = '\0';
                debug_msg(2, "Mocgen: %s:%d Found MOC symbol %s", file->file.real().toLatin1().constData(),
                          line_count, buffer+x);
                file->mocable = true;
                break;
            }
        }

        while(x < buffer_len && SYMBOL_CHAR(*(buffer+x)))
            x++;
        if(QMAKE_EOL(*(buffer+x)))
            line_count++;
    }
    return true;
}


void QMakeSourceFileInfo::saveCache(const QString &cf)
{
#ifdef QMAKE_USE_CACHE
    if(cf.isEmpty())
        return;

    QFile file(QMakeLocalFileName(cf).local());
    if(file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
        stream << qmake_version() << endl << endl; //version
        { //cache verification
            QMap<QString, QStringList> verify = getCacheVerification();
             stream << verify.count() << endl;
             for(QMap<QString, QStringList>::iterator it = verify.begin();
                 it != verify.end(); ++it) {
                 stream << it.key() << endl << it.value().join(";") << endl;
             }
             stream << endl;
        }
        if(files->nodes) {
            for(int file = 0; file < files->num_nodes; ++file) {
                for(SourceFiles::SourceFileNode *node = files->nodes[file];
                    node; node = node->next) {
                    stream << node->file->file.local() << endl; //source
                    stream << node->file->type << endl; //type

                    //depends
                    stream << ";";
                    if(node->file->deps) {
                        for(int depend = 0; depend < node->file->deps->used_nodes; ++depend) {
                            if(depend)
                                stream << ";";
                            stream << node->file->deps->children[depend]->file.local();
                        }
                    }
                    stream << endl;

                    stream << node->file->mocable << endl; //mocable
                    stream << endl; //just for human readability
                }
            }
        }
        file.close();
    }
#endif
}

void QMakeSourceFileInfo::loadCache(const QString &cf)
{
    if(cf.isEmpty())
        return;

#ifdef QMAKE_USE_CACHE
    struct stat cache_st;
    int fd = open(QMakeLocalFileName(cf).local().toLatin1(), O_RDONLY);
    if(fd == -1 || fstat(fd, &cache_st) || S_ISDIR(cache_st.st_mode))
        return;

    QFile file;
    if(!file.open(QIODevice::ReadOnly, fd))
        return;
    QTextStream stream(&file);

    if(stream.readLine() == qmake_version()) { //version check
        stream.skipWhiteSpace();

        bool verified = true;
        { //cache verification
            QMap<QString, QStringList> verify;
            int len = stream.readLine().toInt();
            for(int i = 0; i < len; ++i) {
                QString var = stream.readLine();
                QString val = stream.readLine();
                verify.insert(var, val.split(';', QString::SkipEmptyParts));
            }
            verified = verifyCache(verify);
        }
        if(verified) {
            stream.skipWhiteSpace();
            if(!files)
                files = new SourceFiles;
            while(!stream.atEnd()) {
                QString source = stream.readLine();
                QString type = stream.readLine();
                QString depends = stream.readLine();
                QString mocable = stream.readLine();
                stream.skipWhiteSpace();

                QMakeLocalFileName fn(source);
                struct stat source_st;
                bool source_exists = !stat(fn.local().toLatin1(), &source_st);

                SourceFile *file = files->lookupFile(fn);
                if(!file) {
                    file = new SourceFile;
                    file->file = fn;
                    files->addFile(file);
                    file->type = (SourceFileType)type.toInt();
                    file->exists = source_exists;
                }
                if(source_exists && source_st.st_mtime < cache_st.st_mtime) {
                    if(!file->dep_checked) { //get depends
                        if(!file->deps)
                            file->deps = new SourceDependChildren;
                        file->dep_checked = true;
                        QStringList depend_list = depends.split(";", QString::SkipEmptyParts);
                        for(int depend = 0; depend < depend_list.size(); ++depend) {
                            QMakeLocalFileName dep_fn(depend_list.at(depend));
                            struct stat dep_st;
                            bool dep_exists = !stat(dep_fn.local().toLatin1(), &dep_st);
                            SourceFile *dep = files->lookupFile(dep_fn);
                            if(!dep) {
                                dep = new SourceFile;
                                dep->file = dep_fn;
                                dep->exists = dep_exists;
                                dep->type = QMakeSourceFileInfo::TYPE_UNKNOWN;
                                files->addFile(dep);
                            }
                            dep->included_count++;
                            file->deps->addChild(dep);
                        }
                    }
                    if(!file->moc_checked) { //get mocs
                        file->moc_checked = true;
                        file->mocable = mocable.toInt();
                    }
                }
            }
        }
    }
#endif
}

QMap<QString, QStringList> QMakeSourceFileInfo::getCacheVerification()
{
    return QMap<QString, QStringList>();
}

bool QMakeSourceFileInfo::verifyCache(const QMap<QString, QStringList> &v)
{
    return v == getCacheVerification();
}
