#include "quuidgen.h"
#include <qbuttongroup.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qlineedit.h>

#if defined(Q_WS_WIN32)
#include <objbase.h>
#else
extern "C" {
#include <uuid/uuid.h>
}
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
    result.clear();

#if defined(Q_WS_WIN32)
    GUID guid;
    HRESULT h = CoCreateGuid( &guid );
    if ( h != S_OK )
	resultLabel->setText( tr("Failed to create Uuid") );
    result = QString::number( guid.Data1, 16 ).rightJustify( 8, '0' ) + "-";
    result += QString::number( guid.Data2, 16 ).rightJustify( 4, '0' ) + "-";
    result += QString::number( guid.Data3, 16 ).rightJustify( 4, '0' ) + "-";
    result += QString::number( guid.Data4[0], 16 ).rightJustify( 2, '0' );
    result += QString::number( guid.Data4[1], 16 ).rightJustify( 2, '0' ) + "-";
    for ( int i = 2; i < 8; i++ )
	result += QString::number( guid.Data4[i], 16 ).rightJustify( 2, '0' );
#else
    uuid_t uuid;
    uuid_generate( uuid );

    result = QString::number( (int)uuid[ 0 ], 16 ).rightJustify( 2, '0' ) + QString::number( (int)uuid[ 1 ], 16 ).rightJustify( 2, '0' ) + 
	     QString::number( (int)uuid[ 2 ], 16 ).rightJustify( 2, '0' ) + QString::number( (int)uuid[ 3 ], 16 ).rightJustify( 2, '0' ) + "-" +
	     QString::number( (int)uuid[ 4 ], 16 ).rightJustify( 2, '0' ) + QString::number( (int)uuid[ 5 ], 16 ).rightJustify( 2, '0' ) + "-" +
	     QString::number( (int)uuid[ 6 ], 16 ).rightJustify( 2, '0' ) + QString::number( (int)uuid[ 7 ], 16 ).rightJustify( 2, '0' ) + "-" +
	     QString::number( (int)uuid[ 8 ], 16 ).rightJustify( 2, '0' ) + QString::number( (int)uuid[ 9 ], 16 ).rightJustify( 2, '0' ) + "-";
    for ( int i = 10; i < 16; ++i )
	result += QString::number( (int)uuid[ i ], 16 ).rightJustify( 2, '0' );
	
#endif

    formatChanged();
}

void QUuidGen::copyUuid()
{
    QApplication::clipboard()->setText( resultLabel->text() );
}

void QUuidGen::formatChanged()
{
    int f = formats->id( formats->selected() );
    QString text;
    QString name = nameEdit->text();
    if ( name.isEmpty() )
	name = "<<name>>";

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
    case 0: // define a macro
	text = "// {" + result.upper() + "} \n#ifndef " + name + "\n#define " + name + " QUuid( " + l + ", " + i1 +", " + i2 + ", "+b1+", "+b2+", "+b3+", "+b4+", "+b5+", "+b6+", "+b7+", "+b8+")\n#endif";
	break;
    case 1: // static const QUuid ...
	text = "// {" + result.upper() + "} \nstatic const QUuid " + name + " = QUuid( "+ l + ", " + i1 + ", " + i2 + ", "+b1+", "+b2+", "+b3+", "+b4+", "+b5+", "+b6+", "+b7+", "+b8+" );";
	break;
    case 2: // Registry Entry
	text = "{" + result.upper() + "}";
	break;
    }

    resultLabel->setText( text );
}
