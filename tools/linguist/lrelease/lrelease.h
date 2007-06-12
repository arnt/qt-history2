#include <QtCore/QCoreApplication>
#include <metatranslator.h>
#include <qconsole.h>

class LRelease : public QCoreApplication
{
    Q_OBJECT
public:
    LRelease(int &argc, char **argv) : QCoreApplication(argc, argv)
    {
    
    }

    void releaseMetaTranslator( const MetaTranslator& tor,
                                const QString& qmFileName, bool verbose,
                                bool ignoreUnfinished, bool trimmed );

};

