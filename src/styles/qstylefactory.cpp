#include "qstylefactory.h"

#ifndef QT_H
#include "qwindowsstyle.h"
#include "qmotifstyle.h"
#include "qinterfacemanager.h"
#include "qstyleinterface.h"
#endif // QT_H

static QInterfaceManager<QStyleInterface> *manager = 0;

QStyle *QStyleFactory::create( const QString& style )
{
    if ( style == "Windows" )
	return new QWindowsStyle;
    else if ( style == "Motif" )
	return new QMotifStyle;
#if defined(Q_STYLE_CDE)
    else if ( style == "CDE" )
	return new QCDEStyle;
#endif
#if defined(Q_STYLE_MOTIFPLUS)
    else if ( style == "MotifPlus" )
	return new QMotifPlusStyle;
#endif
#if defined(Q_STYLE_PLATINUM)
    else if ( style == "Platinum" )
	return new QPlatinumStyle;
#endif
#if defined(Q_STYLE_SGI)
    else if ( style == "SGI")
	return new QSGIStyle;
#endif

    QInterfaceManager<QStyleInterface> manager( "QStyleInterface" );

    QStyleInterface *iface = manager.queryInterface( style );
    
    return iface ? iface->create( style ) : 0;
}
