#ifndef QWIDGETINTERFACE_H
#define QWIDGETINTERFACE_H

#include <qcomponentinterface.h>
#include <qiconset.h>

class QWidget;

class WidgetInterface : public QUnknownInterface
{
public:
    virtual QStringList featureList() = 0;

    virtual QWidget* create( const QString&, QWidget* parent = 0, const char* name = 0 ) = 0;

    virtual QString group( const QString& ) = 0;
    virtual QString iconSet( const QString& ) = 0;
    virtual QIconSet iconset( const QString& ) = 0;
    virtual QString includeFile( const QString& ) = 0;
    virtual QString toolTip( const QString& ) = 0;
    virtual QString whatsThis( const QString& ) = 0;
    virtual bool isContainer( const QString& ) = 0;

    static QString interfaceID() { return "WidgetInterface_QtDesigner_Trolltech_05102000_0516"; }
};

#endif //QWIDGETINTERFACE_H
