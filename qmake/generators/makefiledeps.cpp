#include "makefiledeps.h"
#include "option.h"
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
    SourceFile() : deps(0), mocable(0), traversed(0), exists(1) { }
    QMakeLocalFileName file;
    SourceDependChildren *deps;
    uint mocable : 1, traversed : 1, exists : 1;
};
struct SourceDependChildren {
    SourceFile **children;
    int num_nodes, used_nodes;
    SourceDependChildren() : children(0), num_nodes(0), used_nodes(0) { }
    ~SourceDependChildren() { if(children) free(children); }
    void addChild(SourceFile *s) {
	if(num_nodes >= used_nodes) {
	    num_nodes += 200;
	    children = (SourceFile**)realloc(children, sizeof(SourceFile*)*(num_nodes));
	}
	children[used_nodes++] = s;
    }
};
class SourceFiles {
    struct SourceFileNode {
	const char *key;
	SourceFileNode *next;
	SourceFile *file;
    } **nodes;
    int num_nodes;
    int hash(const char *);
    const char *key(const char *);
public:
    SourceFiles();
    ~SourceFiles();

    SourceFile *lookupFile(const char *);
    inline SourceFile *lookupFile(const QString &f) { return lookupFile(f.latin1()); }
    inline SourceFile *lookupFile(const QMakeLocalFileName &f) { return lookupFile(f.local().latin1()); }
    void addFile(SourceFile *);
};
SourceFiles::SourceFiles()
{
    nodes = (SourceFileNode**)malloc(sizeof(SourceFileNode*)*(num_nodes=1024));
    for(int n = 0; n < num_nodes; n++)
	nodes[n] = 0;
}
 
SourceFiles::~SourceFiles()
{
    for(int n = 0; n < num_nodes; n++) {
	if(nodes[n]) {
	    for(SourceFileNode *next = nodes[n]->next; next; ) {
		SourceFileNode *next_next = next->next;
		delete next;
		next = next_next;
	    }
	    free(nodes[n]);
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

const char *SourceFiles::key(const char *file)
{
    const char *ret = file;
    for( ; (*file); ++file) {
	if((*file) == '/' && *(file+1))
	    ret = file+1;
    }
    return ret;
}

SourceFile *SourceFiles::lookupFile(const char *file)
{
    const char *k = key(file);
    int h = hash(k) % num_nodes;
    for(SourceFileNode *p = nodes[h]; p; p = p->next) {
	if(!strcmp(p->key, k)) {
	    return p->file;
	}
    }
    return 0;
}

void SourceFiles::addFile(SourceFile *p)
{
    const char *k = key(p->file.local());
    int h = hash(k) % num_nodes;
    SourceFileNode *pn = new SourceFileNode;
    pn->next = nodes[h];
    pn->key = k;
    pn->file = p;
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

bool QMakeSourceFileInfo::mocable(const QString &file)
{
    if(SourceFile *node = files->lookupFile(file)) 
	return node->mocable;
    return false;
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

void QMakeSourceFileInfo::addSourceFiles(const QStringList &l, uchar seek)
{
    if(!files)
	files = new SourceFiles;
    for(QStringList::ConstIterator it = l.begin(); it != l.end(); ++it) {
	QMakeLocalFileName fn((*it));
	SourceFile *file = files->lookupFile(fn);
	if(!file) {
	    file = new SourceFile;
	    file->file = fn;
	    /* Do moc before dependency checking since some includes can come from
	       moc_*.cpp files */
	    if(seek & SEEK_MOCS)
		findMocs(file);
	    if(seek & SEEK_DEPS)
		findDeps(file);
	    files->addFile(file);
	}
    }
}

char *QMakeSourceFileInfo::getBuffer(int s) {
    if(!spare_buffer || spare_buffer_size < s)
	spare_buffer = (char *)realloc(spare_buffer, spare_buffer_size=s);
    return spare_buffer;
}

void QMakeSourceFileInfo::setFileMocable(const QMakeLocalFileName &)
{

}

#ifdef Q_WS_WIN
#define S_ISDIR(x) (x == _S_IFDIR)
#endif

QMakeLocalFileName QMakeSourceFileInfo::findFileForDep(const QMakeLocalFileName &file)
{
    struct stat fst;
    for(QList<QMakeLocalFileName>::Iterator it = depdirs.begin(); it != depdirs.end(); ++it) {
	QMakeLocalFileName file((*it).real() + Option::dir_sep + file.real());
	if(!stat(file.local(), &fst) && !S_ISDIR(fst.st_mode))
	    return file;
    }
    return QMakeLocalFileName();
}

bool QMakeSourceFileInfo::findDeps(SourceFile *file)
{
    if(!file->deps)
	file->deps = new SourceDependChildren;
    struct stat fst;
    char *buffer = 0;
    int buffer_len = 0;
    {
	int fd = open(file->file.local(), O_RDONLY);
	if(fd == -1)
	    return false;
	if(fstat(fd, &fst) || S_ISDIR(fst.st_mode))
	    return false;

	buffer = getBuffer(fst.st_size);
	for(int have_read = 0;
	    (have_read = read(fd, buffer + buffer_len, fst.st_size - buffer_len));
	    buffer_len += have_read);
	close(fd);
    }
    if(!buffer)
	return false;

    int line_count = 0;
    bool ui_file = false;
    for(int x = 0; x < buffer_len; x++) {
	char *inc = 0;
	if(ui_file) {
	    // skip whitespaces
	    while(x < buffer_len && (*(buffer+x) == ' ' || *(buffer+x) == '\t'))
		x++;
	    if(*(buffer + x) == '<') {
		x++;
		if(buffer_len >= x + 12 && !strncmp(buffer + x, "includehint", 11) &&
		   (*(buffer + x + 11) == ' ' || *(buffer + x + 11) == '>')) {
		    for(x += 12; *(buffer + x) != '>'; x++);
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
			    for(val_len = 0; TRUE; val_len++) {
				if(quote) {
				    if(*(buffer+x+val_len) == quote)
					break;
				} else if(*(buffer + x + val_len) == '>' ||
					  *(buffer + x + val_len) == ' ') {
				    break;
				}
			    }
			    char saved = *(buffer + x + val_len);
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
			for(; x < buffer_len && *(buffer + x) != '\n'; x++);
		    } else if(*(buffer + x) == '*') { //c style comment
			for(; x < buffer_len; x++) {
			    if(*(buffer + x) == '*') {
				if(buffer_len >= (x+1) && *(buffer + (x+1)) == '/') {
				    x += 2;
				    break;
				}
			    } else if(*(buffer + x) == '\n') {
				line_count++;
			    }
			}
		    }
		}
	    }
	    if(*(buffer + x) == '#') {
		x++;
		while(x < buffer_len && //Skip spaces after hash
		      (*(buffer+x) == ' ' || *(buffer+x) == '\t'))
		    x++;
		if(buffer_len >= x + 8 && !strncmp(buffer + x, "include", 7) &&
		   (*(buffer + x + 7) == ' ' || *(buffer + x + 7) == '\t' ||
		    *(buffer + x + 7) == '<' || *(buffer + x + 7) == '"')) {
		    for(x+=7; //skip spaces after keyword
			x < buffer_len && (*(buffer+x) == ' ' || *(buffer+x) == '\t');
			x++);
		    char term = *(buffer + x);
		    if(term == '"');
		    else if(term == '<')
			term = '>';
		    else
			continue; //wtf?
		    x++;

		    int inc_len;
		    for(inc_len = 0; *(buffer + x + inc_len) != term && *(buffer + x + inc_len) != '\n'; inc_len++);
		    *(buffer + x + inc_len) = '\0';
		    inc = buffer + x;
		} else if(buffer_len >= x + 14 && !strncmp(buffer + x,  "qmake_warning ", 14)) {
		    for(x+=14; //skip spaces after keyword
			x < buffer_len && (*(buffer+x) == ' ' || *(buffer+x) == '\t');
			x++);
		    char term = '\n';
		    if(*(buffer + x) == '"')
			term = '"';
		    if(*(buffer + x) == '\'')
			term = '\'';
		    if(term != '\n')
			x++;
		
		    int msg_len;
		    for(msg_len = 0; *(buffer + x + msg_len) != term && *(buffer + x + msg_len) != '\n'; msg_len++);
		    *(buffer + x + msg_len) = '\0';
		    debug_msg(0, "%s:%d qmake_warning -- %s", file->file.local().latin1(), line_count, buffer+x);
		}
	    }
	}

	if(inc) {
	    bool found = true;
	    QMakeLocalFileName lfn(inc);
	    SourceFile *dep = files->lookupFile(lfn);
	    if(!dep) {
		dep = new SourceFile;
		if(stat(lfn.local(), &fst)) {
		    dep->file = findFileForDep(lfn);
		    dep->exists = !dep->file.isNull();
		} else {
		    dep->file = lfn;
		}
		files->addFile(dep);
	    }

	    if(dep->exists) {
		debug_msg(5, "%s:%d Found dependency to %s", file->file.real().latin1(), 
			  line_count, dep->file.local().latin1());
		file->deps->addChild(dep);
	    }
	}
	//read past new line now..
	for(; x < buffer_len && (*(buffer + x) != '\n'); x++);
        line_count++;
    }
    for(int i = 0; i < file->deps->used_nodes; i++) { //now recurse
	if(!file->deps->children[i]->deps) 
	    findDeps(file->deps->children[i]);
    }
    return true;
}

bool QMakeSourceFileInfo::findMocs(SourceFile *file)
{
    int buffer_len;
    char *buffer = 0;
    {
	int fd = open(file->file.local().latin1(), O_RDONLY);
	if(fd == -1)
	    return false;
	struct stat fst;
	if(fstat(fd, &fst) || S_ISDIR(fst.st_mode))
	    return false; //shouldn't happen

	buffer = getBuffer(fst.st_size);
	for(int have_read = buffer_len = 0;
	    (have_read = read(fd, buffer + buffer_len, fst.st_size - buffer_len));
	    buffer_len += have_read);
	close(fd);
    }

    int line_count = 1;
    bool ignore_qobject = false;
 /* qmake ignore Q_OBJECT */
#define COMP_LEN 8 //strlen("Q_OBJECT")
#define OBJ_LEN 8 //strlen("Q_OBJECT")
#define DIS_LEN 10 //strlen("Q_DISPATCH")
    for(int x = 0; x < (buffer_len-COMP_LEN); x++) {
	if(*(buffer + x) == '/') {
	    x++;
	    if(buffer_len >= x) {
		if(*(buffer + x) == '/') { //c++ style comment
		    for(;x < buffer_len && *(buffer + x) != '\n'; x++);
		    line_count++;
		} else if(*(buffer + x) == '*') { //c style comment
		    for(;x < buffer_len; x++) {
			if(*(buffer + x) == 't' || *(buffer + x) == 'q') { //ignore
			    if(buffer_len >= (x + 20)) {
				if(!strncmp(buffer + x + 1, "make ignore Q_OBJECT", 20)) {
				    debug_msg(2, "Mocgen: %s:%d Found \"qmake ignore Q_OBJECT\"",
					      file->file.real().latin1(), line_count);
				    x += 20;
				    ignore_qobject = TRUE;
				}
			    }
			} else if(*(buffer + x) == '*') {
			    if(buffer_len >= (x+1) && *(buffer + (x+1)) == '/') {
				x += 2;
				break;
			    }
			} else if(*(buffer + x) == '\n') {
			    line_count++;
			}
		    }
		}
	    }
	}
#define SYMBOL_CHAR(x) ((x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z') || \
			(x <= '0' && x >= '9') || x == '_')

	bool interesting = *(buffer+x) == 'Q' && (!strncmp(buffer+x, "Q_OBJECT", OBJ_LEN) ||
						      !strncmp(buffer+x, "Q_DISPATCH", DIS_LEN));
	if(interesting) {
	    int len = 0;
	    if(!strncmp(buffer+x, "Q_OBJECT", OBJ_LEN)) {
		if(ignore_qobject) {
		    debug_msg(2, "Mocgen: %s:%d Ignoring Q_OBJECT", file->file.real().latin1(), line_count);
		    interesting = FALSE;
		}
		len=OBJ_LEN;
	    } else if(!strncmp(buffer+x, "Q_DISPATCH", DIS_LEN)) {
		len=DIS_LEN;
	    }
	    if(SYMBOL_CHAR(*(buffer+x+len)))
		interesting = FALSE;
	    if(interesting) {
		*(buffer+x+len) = '\0';
		debug_msg(2, "Mocgen: %s:%d Found MOC symbol %s", file->file.real().latin1(),
			  line_count, buffer+x);
		file->mocable = true;
		setFileMocable(file->file);
		break;
	    }
	}

	while(x < buffer_len && SYMBOL_CHAR(*(buffer+x)))
	    x++;
	if(*(buffer+x) == '\n')
	    line_count++;
    }
#undef OBJ_LEN
#undef DIS_LEN
    return true;
}
