#ifndef WIDGETINTERFACE_H
#define WIDGETINTERFACE_H

#include <qcomponentinterface.h>
#include <qiconset.h>

class QWidget;

class WidgetInterface : public QUnknownInterface
{
public:
    WidgetInterface( QUnknownInterface *parent = 0 )
	: QUnknownInterface( parent ) {}
    QString interfaceId() const { return createId( QUnknownInterface::interfaceId(), "WidgetInterface" ); }

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
