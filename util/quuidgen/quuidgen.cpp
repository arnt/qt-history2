#include "quuidgen.h"
#include <qbuttongroup.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <qapplication.h>
#include <qclipboard.h>

#if defined(Q_WS_WIN32)
#include <objbase.h>
#else
#endif

QUuidGen::QUuidGen()
: QUuidBase( 0, 0, TRUE )
{
    formatMacro->setChecked( TRUE );
    newUuid();
}

void QUuidGen::newUuid()
{
    QString temp;
    result = QString::null;

#if defined(Q_WS_WIN32)
    GUID guid;
    HRESULT h = CoCreateGuid( &guid );
    if ( h != S_OK )
	resultLabel->setText( tr("Failed to create Uuid") );
    result = temp.setNum( guid.Data1, 16 ) + "-";
    result += temp.setNum( guid.Data2, 16 ) + "-";
    result += temp.setNum( guid.Data3, 16 ) + "-";
    result += temp.setNum( guid.Data4[0], 16 );
    result += temp.setNum( guid.Data4[1], 16 ) + "-";
    for ( int i = 2; i < 8; i++ )
	result += temp.setNum( guid.Data4[i], 16 );
#else

#endif

    formatChanged( formats->id( formats->selected() ) );
}

void QUuidGen::copyUuid()
{
    QApplication::clipboard()->setText( resultLabel->text() );
}

void QUuidGen::formatChanged( int f )
{
    QString text;

    QString l = "0x" + result.left( 8 ).lower();
    QString i1 = "0x" + result.mid( 9, 4 ).lower();
    QString i2 = "0x" + result.mid( 14, 4 ).lower();
    QString b1 = "0x" + result.mid( 19, 2 ).lower();
    QString b2 = "0x" + result.mid( 21, 2 ).lower();
    QString b3 = "0x" + result.mid( 24, 2 ).lower();
    QString b4 = "0x" + result.mid( 26, 2 ).lower();
    QString b5 = "0x" + result.mid( 28, 2 ).lower();
    QString b6 = "0x" + result.mid( 30, 2 ).lower();
    QString b7 = "0x" + result.mid( 32, 2 ).lower();
    QString b8 = "0x" + result.mid( 34, 2 ).lower();

    switch ( f ) {
    case 0: // use the Q_UUID macro
	text = "// {" + result.upper() + "} \nQ_UUID(<<name>>, " + l + ", " + i1 +", " + i2 + ", "+b1+", "+b2+", "+b3+", "+b4+", "+b5+", "+b6+", "+b7+", "+b8+");";
	break;
    case 1: // static const QUuid ...
	text = "// {" + result.upper() + "} \nstatic const QUuid <<name>> = { "+ l + ", " + i1 + ", " + i2 + ", { "+b1+", "+b2+", "+b3+", "+b4+", "+b5+", "+b6+", "+b7+", "+b8+" } };";
	break;
    case 2: // Registry Entry
	text = "{" + result.upper() + "}";
	break;
    }

    resultLabel->setText( text );
}
