/****************************************************************************
** $Id$
**
** Definition of QXIMInputContext class
**
** Copyright (C) 2003-2004 immodule for Qt Project.  All rights reserved.
**
** This file is written to contribute to Trolltech AS under their own
** licence. You may use this file under your Qt license. Following
** description is copied from their original file headers. Contact
** immodule-qt@freedesktop.org if any conditions of this licensing are
** not clear to you.
**
****************************************************************************/

/****************************************************************************
**
** Copyright (C) 1992-2004 Trolltech AS. All rights reserved.
**
** This file is part of the input method module of the Qt Toolkit.
**
** Licensees holding valid Qt Preview licenses may use this file in
** accordance with the Qt Preview License Agreement provided with the
** Software.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QXIMINPUTCONTEXT_H
#define QXIMINPUTCONTEXT_H

#if !defined(Q_NO_IM)

#include "qglobal.h"
#include <qinputcontext.h>
#include <qfont.h>

class QKeyEvent;
class QWidget;
class QFont;
class QString;


#ifdef Q_WS_X11
#include "qlist.h"
#include "qbitarray.h"
#include "qwindowdefs.h"
#include <private/qt_x11_p.h>
#endif

class QXIMInputContext : public QInputContext
{
    Q_OBJECT
public:
#ifdef Q_WS_X11
    QXIMInputContext();
    ~QXIMInputContext();

    QString identifierName();
    QString language();

    bool x11FilterEvent( QWidget *keywidget, XEvent *event );
    void reset();

    void setFocus();
    void unsetFocus();
    void setMicroFocus( const QRect &rect, const QFont &f );
    void mouseHandler( int x, QMouseEvent *event);
    bool isPreeditRelocationEnabled();

    void setHolderWidget( QWidget *widget );

    bool hasFocus() const;
    void resetClientState();
    void close( const QString &errMsg );

    void sendIMEvent( QEvent::Type type,
		      const QString &text = QString::null,
		      int cursorPosition = -1, int selLength = 0 );

    static void init_xim();
    static void create_xim();
    static void close_xim();

    void *ic;
    QString composingText;
    QFont font;
    XFontSet fontset;
    QBitArray selectedChars;

protected:
    virtual bool isPreeditPreservationEnabled();  // not a QInputContext func

    QString _language;

    static bool isInitialized;
    static XIM xim;
    static XIMStyle xim_style;
    static QList<QXIMInputContext *> *ximContexts;

private:
    void setComposePosition(int, int);
    void setComposeArea(int, int, int, int);
    void setXFontSet(const QFont &);

    int lookupString(XKeyEvent *, QByteArray &, KeySym *, Status *) const;

#endif // Q_WS_X11
};


#endif //Q_NO_IM

#endif // QXIMINPUTCONTEXT_H
