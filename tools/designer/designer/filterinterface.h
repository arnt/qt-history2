#ifndef FILTERINTERFACE_H
#define FILTERINTERFACE_H

#include <qcomponentinterface.h>

// {EA8CB381-59B5-44a8-BAE5-9BEA8295762A}
Q_GUID(IID_ImportFilterInterface, 
0xea8cb381, 0x59b5, 0x44a8, 0xba, 0xe5, 0x9b, 0xea, 0x82, 0x95, 0x76, 0x2a);

struct ImportFilterInterface : public QUnknownInterface
{
    virtual QStringList featureList() const = 0;
    virtual QStringList import( const QString& filter, const QString& filename ) = 0;
};

// {C32A07E0-B006-471e-AFCA-D227457A1280}
Q_GUID(IID_ExportFilterInterface, 
0xc32a07e0, 0xb006, 0x471e, 0xaf, 0xca, 0xd2, 0x27, 0x45, 0x7a, 0x12, 0x80);

struct ExportFilterInterface : public QUnknownInterface
{
    virtual QStringList featureList() const = 0;
//    virtual QStringList export( const QString& filter, const QString& filename ) = 0;
};

#endif
