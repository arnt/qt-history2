#ifndef QDEFAULTINTERFACE_H
#define QDEFAULTINTERFACE_H

#include "qplugininterface.h"
#include <qstringlist.h>

class QWidgetInterface : public QPlugInInterface
{
public:
    virtual QStringList widgets() = 0;
    virtual QWidget* create( const QString&, QWidget* parent = 0, const char* name = 0 ) = 0;
};

class QCustomWidgetInterface : public QWidgetInterface
{
public:
    virtual QString iconSet( const QString& ) = 0;
    virtual QCString includeFile( const QString& ) = 0;
    virtual QString group( const QString & ) = 0;
    virtual QString toolTip( const QString & ) { return QString::null; }
    virtual QString whatsThis( const QString & ) { return QString::null; }
    virtual bool isContainer( const QString & ) = 0;
};

#endif
