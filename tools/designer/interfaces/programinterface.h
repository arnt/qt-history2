#ifndef PROGRAMINTERFACE_H
#define PROGRAMINTERFACE_H

#include <qcomponentinterface.h>
#include <qstringlist.h>
#include <qmap.h>

// {87ced303-884f-449a-881a-ae8104932e3e}
#ifndef IID_ProgramInterface
#define IID_ProgramInterface QUuid( 0x87ced303, 0x884f, 0x449a, 0x88, 0x1a, 0xae, 0x81, 0x04, 0x93, 0x2e, 0x3e )
#endif

struct ProgramInterface : public QUnknownInterface
{
    virtual QStringList featureList() const = 0;
    virtual bool check( const QString &, QStringList &errors, QValueList<int> &line ) = 0;
    virtual bool build( const QString &projectFile, QMap< QString, QMap<QStringList, int > > &errors ) = 0;
    virtual int run( const QStringList &projectFile ) = 0;
};

#endif
