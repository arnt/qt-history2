#include "qstylefactory.h"

#include "qinterfacemanager.h"
#include "qstyleinterface.h"
#include "qwindowsstyle.h"
#include "qmotifstyle.h"
#if defined(QT_STYLE_CDE)
#include "qcdestyle.h"
#endif
#if defined(QT_STYLE_MOTIFPLUS)
#include "qmotifplusstyle.h"
#endif
#if defined(QT_STYLE_PLATINUM)
#include "qplatinumstyle.h"
#endif
#if defined(QT_STYLE_SGI)
#include "qsgistyle.h"
#endif
#include <stdlib.h>

static QInterfaceManager<QStyleInterface> *manager = 0;

QStyle *QStyleFactory::create( const QString& style )
{
    if ( !manager )
	manager = new QInterfaceManager<QStyleInterface>( "QStyleInterface", QString((char*)getenv( "QTDIR" )) + "/plugins" );

    QStyleInterface *iface = manager->queryInterface( style );

    if ( iface )
	return iface->create( style );

    if ( style == "Windows" )
	return new QWindowsStyle;
    else if ( style == "Motif" )
	return new QMotifStyle;
#if defined(QT_STYLE_CDE)
    else if ( style == "CDE" )
	return new QCDEStyle;
#endif
#if defined(QT_STYLE_MOTIFPLUS)
    else if ( style == "MotifPlus" )
	return new QMotifPlusStyle;
#endif
#if defined(QT_STYLE_PLATINUM)
    else if ( style == "Platinum" )
	return new QPlatinumStyle;
#endif
#if defined(QT_STYLE_SGI)
    else if ( style == "SGI")
	return new QSGIStyle;
#endif

    return 0;
}

QStringList QStyleFactory::styles()
{
    if ( !manager )
	manager = new QInterfaceManager<QStyleInterface>( "QStyleInterface", QString((char*)getenv( "QTDIR" )) + "/plugins" );

    QStringList list = manager->featureList();

    list << "Windows";
    list << "Motif";
#if defined(QT_STYLE_CDE)
    list << "CDE";
#endif
#if defined(QT_STYLE_MOTIFPLUS)
    list << "MotifPlus";
#endif
#if defined(QT_STYLE_PLATINUM)
    list << "Platinum";
#endif
#if defined(QT_STYLE_SGI)
    list << "SGI";
#endif

    return list;
}
