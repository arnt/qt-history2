/****************************************************************************
** CDialogWidget meta object code from reading C++ file 'dialogwidget.h'
**
** Created: Thu Nov 16 14:27:41 2000
**      by: The Qt MOC ($Id: //depot/qt/main/src/moc/moc.y#209 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#define Q_MOC_CDialogWidget
#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 12
#elif Q_MOC_OUTPUT_REVISION != 12
#error "Moc format conflict - please regenerate all moc files"
#endif

#include "dialogwidget.h"
#include <qmetaobject.h>
#include <qapplication.h>


const char *CDialogWidget::className() const
{
    return "CDialogWidget";
}

QMetaObject *CDialogWidget::metaObj = 0;

static QMetaObjectCleanUp cleanUp_CDialogWidget = QMetaObjectCleanUp();

#ifndef QT_NO_TRANSLATION

QString CDialogWidget::tr(const char* s)
{
    return qApp->translate( "CDialogWidget", s, 0 );
}

QString CDialogWidget::tr(const char* s, const char * c)
{
    return qApp->translate( "CDialogWidget", s, c );
}

#endif // QT_NO_TRANSLATION

QMetaObject* CDialogWidget::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QWidget::staticMetaObject();
#ifndef QT_NO_PROPERTIES
#endif // QT_NO_PROPERTIES
    typedef void(CDialogWidget::*m1_t0)();
    typedef void(CDialogWidget::*m1_t1)(int);
    typedef void(CDialogWidget::*m1_t2)(int);
    m1_t0 v1_0 = &CDialogWidget::generate;
    m1_t1 v1_1 = &CDialogWidget::clickedLib;
    m1_t2 v1_2 = &CDialogWidget::clickedThread;
    QMetaData *slot_tbl = QMetaObject::new_metadata(3);
    slot_tbl[0].name = "generate()";
    slot_tbl[0].ptr = (QMember)v1_0;
    slot_tbl[0].access = QMetaData::Public;
    slot_tbl[1].name = "clickedLib(int)";
    slot_tbl[1].ptr = (QMember)v1_1;
    slot_tbl[1].access = QMetaData::Public;
    slot_tbl[2].name = "clickedThread(int)";
    slot_tbl[2].ptr = (QMember)v1_2;
    slot_tbl[2].access = QMetaData::Public;
    metaObj = QMetaObject::new_metaobject(
	"CDialogWidget", parentObject,
	slot_tbl, 3,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
#ifndef QT_NO_PROPERTIES
#endif // QT_NO_PROPERTIES
    cleanUp_CDialogWidget.setMetaObject( metaObj );
    return metaObj;
}
