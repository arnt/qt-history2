#ifndef QINPUTCONTEXT_P_H
#define QINPUTCONTEXT_P_H

#include "qwindowdefs.h"
#include "qt_x11.h"

class QKeyEvent;


class QInputContext
{
public:
    QInputContext(QWidget *); // should be a toplevel widget
    ~QInputContext();

    bool isComposing() const;

    void setFocus();
    void setComposePosition(int, int);
    void setComposeArea(int, int, int, int);
    void reset();
    bool keyPressEvent(QKeyEvent *);

    void setText(const QString &);
    const QString &text() const;

#if defined(Q_WS_X11)
    int lookupString(XKeyEvent *, QCString &, KeySym *, Status *) const;
#endif // Q_WS_X11

private:
    void *ic;
    QString txt;
};


// Inline methods

inline void QInputContext::setText(const QString &t)
{ txt = t; }

inline const QString &QInputContext::text() const
{ return txt; }

#endif // QINPUTCONTEXT_P_H
