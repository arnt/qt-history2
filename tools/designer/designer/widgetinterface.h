#ifndef QWIDGETINTERFACE_H
#define QWIDGETINTERFACE_H

#include <qplugininterface.h>

class QWidget;

class WidgetInterface : public QPlugInInterface
{
public:
    QCString queryPlugInInterface() const { return "WidgetInterface"; }

    virtual QWidget* create( const QString&, QWidget* parent = 0, const char* name = 0 ) = 0;

    virtual QString group( const QString& ) = 0;
    virtual QString iconSet( const QString& ) = 0;
    virtual QString includeFile( const QString& ) = 0;
    virtual QString toolTip( const QString& ) = 0;
    virtual QString whatsThis( const QString& ) = 0;
    virtual bool isContainer( const QString& ) = 0;
};

#endif //QWIDGETINTERFACE_H
