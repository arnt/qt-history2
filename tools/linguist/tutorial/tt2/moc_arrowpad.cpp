/****************************************************************************
** ArrowPad meta object code from reading C++ file 'arrowpad.h'
**
** Created: Fri Feb 2 17:04:58 2001
**      by: The Qt MOC ($Id: //depot/qt/main/src/moc/moc.y#221 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "arrowpad.h"
#include <qmetaobject.h>
#include <qapplication.h>

#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 12)
#error "This file was generated using the moc from 3.0.0-snapshot. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *ArrowPad::className() const
{
    return "ArrowPad";
}

QMetaObject *ArrowPad::metaObj = 0;

static QMetaObjectCleanUp cleanUp_ArrowPad = QMetaObjectCleanUp();

#ifndef QT_NO_TRANSLATION

QString ArrowPad::tr(const char* s)
{
    return qApp->translate( "ArrowPad", s, 0 );
}

QString ArrowPad::tr(const char* s, const char * c)
{
    return qApp->translate( "ArrowPad", s, c );
}

#endif // QT_NO_TRANSLATION

QMetaObject* ArrowPad::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QGrid::staticMetaObject();
#ifndef QT_NO_PROPERTIES
#endif // QT_NO_PROPERTIES
    metaObj = QMetaObject::new_metaobject(
	"ArrowPad", parentObject,
	0, 0,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
#ifndef QT_NO_PROPERTIES
#endif // QT_NO_PROPERTIES
    cleanUp_ArrowPad.setMetaObject( metaObj );
    return metaObj;
}
