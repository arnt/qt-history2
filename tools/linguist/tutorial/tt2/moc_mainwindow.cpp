/****************************************************************************
** MainWindow meta object code from reading C++ file 'mainwindow.h'
**
** Created: Fri Feb 2 17:04:59 2001
**      by: The Qt MOC ($Id: //depot/qt/main/src/moc/moc.y#221 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "mainwindow.h"
#include <qmetaobject.h>
#include <qapplication.h>

#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 12)
#error "This file was generated using the moc from 3.0.0-snapshot. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *MainWindow::className() const
{
    return "MainWindow";
}

QMetaObject *MainWindow::metaObj = 0;

static QMetaObjectCleanUp cleanUp_MainWindow = QMetaObjectCleanUp();

#ifndef QT_NO_TRANSLATION

QString MainWindow::tr(const char* s)
{
    return qApp->translate( "MainWindow", s, 0 );
}

QString MainWindow::tr(const char* s, const char * c)
{
    return qApp->translate( "MainWindow", s, c );
}

#endif // QT_NO_TRANSLATION

QMetaObject* MainWindow::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QMainWindow::staticMetaObject();
#ifndef QT_NO_PROPERTIES
#endif // QT_NO_PROPERTIES
    metaObj = QMetaObject::new_metaobject(
	"MainWindow", parentObject,
	0, 0,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
#ifndef QT_NO_PROPERTIES
#endif // QT_NO_PROPERTIES
    cleanUp_MainWindow.setMetaObject( metaObj );
    return metaObj;
}
