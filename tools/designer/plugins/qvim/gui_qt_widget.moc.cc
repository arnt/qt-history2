/****************************************************************************
** VimMainWindow meta object code from reading C++ file 'gui_qt_widget.h'
**
** Created: Sun Jan 14 22:38:29 2001
**      by: The Qt MOC ($Id: $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "gui_qt_widget.h"
#include <qmetaobject.h>
#include <qapplication.h>

#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 12)
#error "This file was generated using the moc from 3.0.0-snapshot. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *VimMainWindow::className() const
{
    return "VimMainWindow";
}

QMetaObject *VimMainWindow::metaObj = 0;

static QMetaObjectCleanUp cleanUp_VimMainWindow = QMetaObjectCleanUp();

#ifndef QT_NO_TRANSLATION

QString VimMainWindow::tr(const char* s)
{
    return qApp->translate( "VimMainWindow", s, 0 );
}

QString VimMainWindow::tr(const char* s, const char * c)
{
    return qApp->translate( "VimMainWindow", s, c );
}

#endif // QT_NO_TRANSLATION

QMetaObject* VimMainWindow::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QMainWindow::staticMetaObject();
#ifndef QT_NO_PROPERTIES
#endif // QT_NO_PROPERTIES
    typedef void (VimMainWindow::*m1_t0)(int);
    typedef void (QObject::*om1_t0)(int);
    typedef void (VimMainWindow::*m1_t1)();
    typedef void (QObject::*om1_t1)();
    m1_t0 v1_0 = &VimMainWindow::menu_activated;
    om1_t0 ov1_0 = (om1_t0)v1_0;
    m1_t1 v1_1 = &VimMainWindow::blink_cursor;
    om1_t1 ov1_1 = (om1_t1)v1_1;
    QMetaData *slot_tbl = QMetaObject::new_metadata(2);
    slot_tbl[0].name = "menu_activated(int)";
    slot_tbl[0].ptr = (QMember)ov1_0;
    slot_tbl[0].access = QMetaData::Public;
    slot_tbl[1].name = "blink_cursor()";
    slot_tbl[1].ptr = (QMember)ov1_1;
    slot_tbl[1].access = QMetaData::Public;
    metaObj = QMetaObject::new_metaobject(
	"VimMainWindow", parentObject,
	slot_tbl, 2,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
#ifndef QT_NO_PROPERTIES
#endif // QT_NO_PROPERTIES
    cleanUp_VimMainWindow.setMetaObject( metaObj );
    return metaObj;
}


const char *SBPool::className() const
{
    return "SBPool";
}

QMetaObject *SBPool::metaObj = 0;

static QMetaObjectCleanUp cleanUp_SBPool = QMetaObjectCleanUp();

#ifndef QT_NO_TRANSLATION

QString SBPool::tr(const char* s)
{
    return qApp->translate( "SBPool", s, 0 );
}

QString SBPool::tr(const char* s, const char * c)
{
    return qApp->translate( "SBPool", s, c );
}

#endif // QT_NO_TRANSLATION

QMetaObject* SBPool::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QObject::staticMetaObject();
#ifndef QT_NO_PROPERTIES
#endif // QT_NO_PROPERTIES
    typedef void (SBPool::*m1_t0)(int);
    typedef void (QObject::*om1_t0)(int);
    m1_t0 v1_0 = &SBPool::sbUsed;
    om1_t0 ov1_0 = (om1_t0)v1_0;
    QMetaData *slot_tbl = QMetaObject::new_metadata(1);
    slot_tbl[0].name = "sbUsed(int)";
    slot_tbl[0].ptr = (QMember)ov1_0;
    slot_tbl[0].access = QMetaData::Public;
    metaObj = QMetaObject::new_metaobject(
	"SBPool", parentObject,
	slot_tbl, 1,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
#ifndef QT_NO_PROPERTIES
#endif // QT_NO_PROPERTIES
    cleanUp_SBPool.setMetaObject( metaObj );
    return metaObj;
}
