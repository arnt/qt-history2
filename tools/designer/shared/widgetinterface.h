#ifndef WIDGETINTERFACE_H
#define WIDGETINTERFACE_H

#include <qcomponentinterface.h>
#include <qiconset.h>
#include <qstringlist.h>

class QWidget;

// {55184143-F18F-42c0-A8EB-71C01516019A}
Q_GUID(IID_WidgetInterface, 
0x55184143, 0xf18f, 0x42c0, 0xa8, 0xeb, 0x71, 0xc0, 0x15, 0x16, 0x1, 0x9a);

class WidgetInterface : public QUnknownInterface
{
public:
    virtual QStringList featureList() const = 0;
    virtual QWidget* create( const QString&, QWidget* parent = 0, const char* name = 0 ) = 0;

    virtual QString group( const QString& ) const = 0;
    virtual QString iconSet( const QString& ) const = 0;
    virtual QIconSet iconset( const QString& ) const = 0;
    virtual QString includeFile( const QString& ) const = 0;
    virtual QString toolTip( const QString& ) const = 0;
    virtual QString whatsThis( const QString& ) const = 0;
    virtual bool isContainer( const QString& ) const = 0;
};

#endif
