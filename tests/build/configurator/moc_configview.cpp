/****************************************************************************
** CConfigView meta object code from reading C++ file 'configview.h'
**
** Created: Thu Nov 16 14:27:41 2000
**      by: The Qt MOC ($Id: //depot/qt/main/src/moc/moc.y#209 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#define Q_MOC_CConfigView
#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 12
#elif Q_MOC_OUTPUT_REVISION != 12
#error "Moc format conflict - please regenerate all moc files"
#endif

#include "configview.h"
#include <qmetaobject.h>
#include <qapplication.h>


const char *CConfigView::className() const
{
    return "CConfigView";
}

QMetaObject *CConfigView::metaObj = 0;

static QMetaObjectCleanUp cleanUp_CConfigView = QMetaObjectCleanUp();

#ifndef QT_NO_TRANSLATION

QString CConfigView::tr(const char* s)
{
    return qApp->translate( "CConfigView", s, 0 );
}

QString CConfigView::tr(const char* s, const char * c)
{
    return qApp->translate( "CConfigView", s, c );
}

#endif // QT_NO_TRANSLATION

QMetaObject* CConfigView::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QScrollView::staticMetaObject();
#ifndef QT_NO_PROPERTIES
#endif // QT_NO_PROPERTIES
    typedef void(CConfigView::*m1_t0)(const QString&);
    m1_t0 v1_0 = &CConfigView::configToggled;
    QMetaData *slot_tbl = QMetaObject::new_metadata(1);
    slot_tbl[0].name = "configToggled(const QString&)";
    slot_tbl[0].ptr = (QMember)v1_0;
    slot_tbl[0].access = QMetaData::Public;
    metaObj = QMetaObject::new_metaobject(
	"CConfigView", parentObject,
	slot_tbl, 1,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
#ifndef QT_NO_PROPERTIES
#endif // QT_NO_PROPERTIES
    cleanUp_CConfigView.setMetaObject( metaObj );
    return metaObj;
}
