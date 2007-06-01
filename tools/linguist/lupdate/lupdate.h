#include "metatranslator.h"
#include "qconsole.h"
#include <QtCore>
#include <stdio.h>

class lupdateApplication : public QCoreApplication
{
    Q_OBJECT
public:
    lupdateApplication(int argc, char **argv) 
        : QCoreApplication(argc, argv), 
          m_defaultExtensions("ui,c,c++,cc,cpp,cxx,ch,h,h++,hh,hpp,hxx")
    { }

    int start();

    // defined in fetchtr.cpp
    void fetchtr_cpp( const char *fileName, MetaTranslator *tor,
                      const char *defaultContext, bool mustExist, const QByteArray &codecForSource );
    void fetchtr_ui( const char *fileName, MetaTranslator *tor,
                     const char *defaultContext, bool mustExist );

    // defined in fetchtrjava.cpp
    void fetchtr_java( const char *fileName, MetaTranslator *tor,
                       const char *defaultContext, bool mustExist, const QByteArray &codecForSource );
    // defined in merge.cpp
    void merge( const MetaTranslator *tor, const MetaTranslator *virginTor, MetaTranslator *out,
                bool verbose, bool noObsolete );

    void recursiveFileInfoList( const QDir &dir, const QStringList &nameFilters, 
                                QDir::Filters filter, bool recursive, QFileInfoList *fileinfolist);
    void printUsage();
    void updateTsFiles( const MetaTranslator& fetchedTor,
                               const QStringList& tsFileNames, const QString& codecForTr,
                               bool noObsolete, bool verbose );

private:
    const char *m_defaultExtensions;

};

