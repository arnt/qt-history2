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

#ifndef QINPUTCONTEXT_P_H
#define QINPUTCONTEXT_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of internal files.  This header file may change from version to version
// without notice, or even be removed.
//
// We mean it.
//

#include "qglobal.h"

class QKeyEvent;
class QWidget;
class QFont;
class QString;


#ifdef Q_WS_X11
#include "qbitarray.h"
#include "qwindowdefs.h"
#include "qt_x11_p.h"
#endif

#ifdef Q_WS_WIN
#include "qt_windows.h"
#endif

#ifdef Q_WS_QWS
class QWSIMEvent;
#endif

#ifndef qdoc
class QInputContext
{
public:
#ifdef Q_WS_X11
    explicit QInputContext(QWidget *); // should be a toplevel widget
    ~QInputContext();

    void setFocus();
    void setComposePosition(int, int);
    void setComposeArea(int, int, int, int);
    void reset();

    int lookupString(XKeyEvent *, QByteArray &, KeySym *, Status *) const;
    void setXFontSet(const QFont &);

    void *ic;
    QString text;
    QWidget *focusWidget;
    bool composing;
    QFont font;
    XFontSet fontset;
    QBitArray selectedChars;
#endif // Q_WS_X11

#ifdef Q_WS_QWS
#ifndef QT_NO_QWS_IM

    static void translateIMEvent(QWSIMEvent *, QWidget *);
    static void reset(QWidget *focusW = 0);

    static void setMicroFocusWidget(QWidget *);
    static QWidget *microFocusWidget() {return activeWidget;}
    static void notifyWidgetDeletion(QWidget *);

private:
    static void retrieveMarkedText(QWidget *);
    static void cleanup();
    static QWidget* activeWidget;
    static QString* composition;
    static bool composeMode;
#endif //QT_NO_QWS_IM
#endif //Q_WS_QWS

#ifdef Q_WS_WIN
    static void init();
    static void shutdown();

    static void TranslateMessage(const MSG *msg);
    static LRESULT DefWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

    static void setFont(const QWidget *w, const QFont &);
    static void setFocusHint(int x, int y, int w, int h, const QWidget *widget);
    static bool startComposition();
    static bool endComposition(QWidget *fw = 0);
    static bool composition(LPARAM lparam);

    static void accept(QWidget *fw = 0);
    static void enable(QWidget *w, bool b);
#endif
};
#endif // qdoc

#endif // QINPUTCONTEXT_P_H
