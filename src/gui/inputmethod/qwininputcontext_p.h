#ifndef QWININPUTCONTEXT_P_H
#define QWININPUTCONTEXT_P_H

#include <qinputcontext.h>
#include "qt_windows.h"

class QWinInputContext : public QInputContext
{
    Q_OBJECT
public:
    explicit QWinInputContext(QObject* parent = 0);
    virtual ~QWinInputContext();

    virtual QString identifierName() { return QLatin1String("win"); }
    virtual QString language();

    virtual void reset();
    virtual void update();

    virtual void mouseHandler(int x, QMouseEvent *event);
    virtual bool isComposing() const;

    virtual void setFocusWidget(QWidget *w);

    bool startComposition();
    bool endComposition();
    bool composition(LPARAM lparam);

    static void enable(QWidget *w, bool e);
    static void TranslateMessage(const MSG *msg);
    static LRESULT DefWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
private:
    void init();
};

#endif
