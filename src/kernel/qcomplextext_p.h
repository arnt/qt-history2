/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QCOMPLEXTEXT_P_H
#define QCOMPLEXTEXT_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Remote Control. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//
//

#ifndef QT_H
#include "qstring.h"
#include "qpointarray.h"
#include "qfont.h"
#include "qpainter.h"
#include "qptrlist.h"
#include "qshared.h"
#endif // QT_H

#ifndef QT_NO_COMPLEXTEXT

// bidi helper classes. Internal to Qt
struct Q_EXPORT QBidiStatus {
    QBidiStatus() {
	eor = QChar::DirON;
	lastStrong = QChar::DirON;
	last = QChar:: DirON;
    }
    QChar::Direction eor;
    QChar::Direction lastStrong;
    QChar::Direction last;
};

struct Q_EXPORT QBidiContext : public QShared {
    // ### ref and deref parent?
    QBidiContext( uchar level, QChar::Direction embedding, QBidiContext *parent = 0, bool override = FALSE );
    ~QBidiContext();

    unsigned char level;
    bool override : 1;
    QChar::Direction dir : 5;

    QBidiContext *parent;
};

struct Q_EXPORT QBidiControl {
    QBidiControl() { context = 0; }
    QBidiControl( QBidiContext *c, QBidiStatus s)
    { context = c; if( context ) context->ref(); status = s; }
    ~QBidiControl() { if ( context && context->deref() ) delete context; }
    void setContext( QBidiContext *c ) { if ( context == c ) return; if ( context && context->deref() ) delete context; context = c; context->ref(); }
    QBidiContext *context;
    QBidiStatus status;
};

struct Q_EXPORT QTextRun {
    QTextRun(int _start, int _stop, QBidiContext *context, QChar::Direction dir);

    int start;
    int stop;
    // explicit + implicit levels here
    uchar level;
};

class Q_EXPORT QComplexText {
public:
    static QPtrList<QTextRun> *bidiReorderLine( QBidiControl *control, const QString &str, int start, int len,
						QChar::Direction basicDir = QChar::DirON );
    static QString bidiReorderString( const QString &str, QChar::Direction basicDir = QChar::DirON );
};


#endif //QT_NO_COMPLEXTEXT

#endif
