/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/****************************************************************************
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

#ifndef QXIMINPUTCONTEXT_P_H
#define QXIMINPUTCONTEXT_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#if !defined(Q_NO_IM)

#include "qglobal.h"
#include <qinputcontext.h>
#include <qfont.h>
#include <qhash.h>

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
    struct ICData {
        XIC ic;
        XFontSet fontset;
        QWidget *widget;
        QString text;
        QBitArray selectedChars;
        bool composing;
        void clear();
    };

    QXIMInputContext();
    ~QXIMInputContext();

    QString identifierName();
    QString language();

    void reset();

    void mouseHandler( int x, QMouseEvent *event);
    bool isComposing() const;

    void setFocusWidget( QWidget *w );
    void widgetDestroyed(QWidget *w);

    void create_xim();
    void close_xim();

    void update();

    ICData *icData() const;
protected:
    bool x11FilterEvent( QWidget *keywidget, XEvent *event );

private:
    static XIMStyle xim_style;

    QString _language;
    XIM xim;
    QHash<QWidget *, ICData *> ximData;

    ICData *createICData(QWidget *w);
};

#endif //Q_NO_IM

#endif // QXIMINPUTCONTEXT_P_H
