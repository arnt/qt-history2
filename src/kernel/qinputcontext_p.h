#ifndef QINPUTCONTEXT_P_H
#define QINPUTCONTEXT_P_H

#include "qwindowdefs.h"
#include "qt_x11.h"

class QKeyEvent;
class QWidget;


class QInputContext
{
public:
    QInputContext(QWidget *); // should be a toplevel widget
    ~QInputContext();

    void setFocus();
    void setComposePosition(int, int);
    void setComposeArea(int, int, int, int);
    void reset();

#if defined(Q_WS_X11)
    int lookupString(XKeyEvent *, QCString &, KeySym *, Status *) const;
    void setXFontSet(QFont *);
#endif // Q_WS_X11

    void *ic;
    QString text, lastcompose;
    QWidget *focusWidget;
    bool composing;
#ifdef    Q_WS_X11
    QFont font;
    XFontSet fontset;
#endif // Q_WS_X11
};


#endif // QINPUTCONTEXT_P_H
