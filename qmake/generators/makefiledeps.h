#ifndef __MAKEFILEDEPS_H__
#define __MAKEFILEDEPS_H__

#include "qstringlist.h"

class SourceFile;
class SourceDependChildren;
class SourceFiles;

class QMakeLocalFileName {
    uint is_null : 1;
    QString real_name, local_name;    
public:
    QMakeLocalFileName() : is_null(1) { }
    QMakeLocalFileName(const QString &);
    bool isNull() const { return is_null; }
    const QString &real() const { return real_name; }
    const QString &local() const { return local_name; }
};

class QMakeSourceFileInfo
{
private:    
    //quick project lookups
    SourceFiles *files;
    QList<QMakeLocalFileName> depdirs;

    //sleezy buffer code
    char *spare_buffer;
    int   spare_buffer_size;
    char *getBuffer(int s);

    //actual guts
    bool findMocs(SourceFile *);
    bool findDeps(SourceFile *);
    void dependTreeWalker(SourceFile *, SourceDependChildren *);

protected:
    virtual QMakeLocalFileName findFileForDep(const QMakeLocalFileName &);
    virtual void setFileMocable(const QMakeLocalFileName &);

public:
    QMakeSourceFileInfo();
    ~QMakeSourceFileInfo();
    void setDependencyPaths(const QList<QMakeLocalFileName> &);
    enum SourceFileSeek { SEEK_DEPS=0x01, SEEK_MOCS=0x02 };
    void addSourceFiles(const QStringList &, uchar);

    QStringList dependencies(const QString &file);
    bool mocable(const QString &file);
};

#endif /* __MAKEFILEDEPS_H__ */
