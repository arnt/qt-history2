/****************************************************************************
** Main meta object code from reading C++ file 'textcodec.h'
**
** Created: Wed Nov 4 18:03:44 1998
**      by: The Qt Meta Object Compiler ($Revision: 1.1 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 3
#elif Q_MOC_OUTPUT_REVISION != 3
#error "Moc format conflict - please regenerate all moc files"
#endif

#include "textcodec.h"
#include <qmetaobject.h>
#include <qapplication.h>


const char *Main::className() const
{
    return "Main";
}

QMetaObject *Main::metaObj = 0;


#if QT_VERSION >= 200
static QMetaObjectInit init_Main(&Main::staticMetaObject);

#endif

void Main::initMetaObject()
{
    if ( metaObj )
	return;
    if ( strcmp(QLabel::className(), "QLabel") != 0 )
	badSuperclassWarning("Main","QLabel");

#if QT_VERSION >= 200
    staticMetaObject();
}

QString Main::tr(const char* s)
{
    return qApp->translate("Main",s);
}

void Main::staticMetaObject()
{
    if ( metaObj )
	return;
    QLabel::staticMetaObject();
#else

    QLabel::initMetaObject();
#endif

    metaObj = new QMetaObject( "Main", "QLabel",
	0, 0,
	0, 0 );
}
