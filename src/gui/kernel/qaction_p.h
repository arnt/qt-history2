#ifndef QACTION_P_H
#define QACTION_P_H

#include "qaction.h"
#include <private/qobject_p.h>
#include "qmenu.h"

#ifdef QT_COMPAT
class QMenuItemEmitter;
#endif

class QActionPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QAction)
public:
    QActionPrivate();
    ~QActionPrivate();

    QPointer<QActionGroup> group;
    QString text;
    QString menuText;
    QIconSet *icons;
    QString tooltip;
    QString statustip;
    QString whatsthis;
    QKeySequence shortcut;
    int shortcutId;
    QFont font;
    QPointer<QMenu> menu;
    uint enabled : 1, forceDisabled : 1;
    uint visible : 1, forceInvisible : 1;
    uint checkable : 1;
    uint checked : 1;
    uint separator : 1;

#ifdef QT_COMPAT //for menubar/menu compat
    QMenuItemEmitter *act_signal;
    int id, param;
#endif
    void sendDataChanged();
};

#endif // QACTION_P_H
