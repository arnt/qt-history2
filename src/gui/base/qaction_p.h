#ifndef QACTION_P_H
#define QACTION_P_H

#include "qaction.h"
#include <private/qobject_p.h>
#include "qaccel.h"
#include "qmenu.h"

class QActionPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QAction);
public:
    QActionPrivate() : group(0), icons(0), accel(-1), enabled(1), visible(1), checkable(0), checked(0), separator(0) {}
    ~QActionPrivate() {
	delete icons;
	if(menu)
	    delete menu;
	if(actionAccels && accel != -1) {
	    actionAccels->removeItem(accel);
	    if(!actionAccels->count()) {
		delete actionAccels;
		actionAccels = 0;
	    }
	}
    }

    QPointer<QActionGroup> group;
    QString text;
    QIconSet *icons;
    QString tooltip;
    QString statustip;
    QString whatsthis;
#ifndef QT_NO_ACCEL
    static QAccel *actionAccels;
    int accel;
#endif
    QPointer<Q4Menu> menu;
    uint enabled : 1;
    uint visible : 1;
    uint checkable : 1;
    uint checked : 1;
    uint separator : 1;
};

class QActionGroupPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QActionGroup);
public:
    QActionGroupPrivate() : exclusive(1)  { }
    QList<QAction *> actions;
    QPointer<QAction> current;
    uint exclusive : 1;
    uint enabled : 1;
    uint visible : 1;
};

#endif /* QACTION_P_H */
