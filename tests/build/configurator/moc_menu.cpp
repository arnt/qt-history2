/****************************************************************************
** CConfiguratorMenu meta object code from reading C++ file 'menu.h'
**
** Created: Thu Nov 16 14:27:42 2000
**      by: The Qt MOC ($Id: //depot/qt/main/src/moc/moc.y#209 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#define Q_MOC_CConfiguratorMenu
#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 12
#elif Q_MOC_OUTPUT_REVISION != 12
#error "Moc format conflict - please regenerate all moc files"
#endif

#include "menu.h"
#include <qmetaobject.h>
#include <qapplication.h>


const char *CConfiguratorMenu::className() const
{
    return "CConfiguratorMenu";
}

QMetaObject *CConfiguratorMenu::metaObj = 0;

static QMetaObjectCleanUp cleanUp_CConfiguratorMenu = QMetaObjectCleanUp();

#ifndef QT_NO_TRANSLATION

QString CConfiguratorMenu::tr(const char* s)
{
    return qApp->translate( "CConfiguratorMenu", s, 0 );
}

QString CConfiguratorMenu::tr(const char* s, const char * c)
{
    return qApp->translate( "CConfiguratorMenu", s, c );
}

#endif // QT_NO_TRANSLATION

QMetaObject* CConfiguratorMenu::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QMenuBar::staticMetaObject();
#ifndef QT_NO_PROPERTIES
#endif // QT_NO_PROPERTIES
    typedef void(CConfiguratorMenu::*m2_t0)();
    typedef void(CConfiguratorMenu::*m2_t1)();
    m2_t0 v2_0 = &CConfiguratorMenu::fileOpen;
    m2_t1 v2_1 = &CConfiguratorMenu::fileSave;
    QMetaData *signal_tbl = QMetaObject::new_metadata(2);
    signal_tbl[0].name = "fileOpen()";
    signal_tbl[0].ptr = (QMember)v2_0;
    signal_tbl[0].access = QMetaData::Public;
    signal_tbl[1].name = "fileSave()";
    signal_tbl[1].ptr = (QMember)v2_1;
    signal_tbl[1].access = QMetaData::Public;
    metaObj = QMetaObject::new_metaobject(
	"CConfiguratorMenu", parentObject,
	0, 0,
	signal_tbl, 2,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
#ifndef QT_NO_PROPERTIES
#endif // QT_NO_PROPERTIES
    cleanUp_CConfiguratorMenu.setMetaObject( metaObj );
    return metaObj;
}

// SIGNAL fileOpen
void CConfiguratorMenu::fileOpen()
{
    activate_signal( staticMetaObject()->signalOffset() + 0 );
}

// SIGNAL fileSave
void CConfiguratorMenu::fileSave()
{
    activate_signal( staticMetaObject()->signalOffset() + 1 );
}
