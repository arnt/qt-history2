#include "qmenu.h"
#include "qhash.h"
#include "qt_mac.h"

#include "private/qmenu_p.h"
#define d d_func()
#define q q_func()

QHash<QWidget *, Q4MenuBar *> Q4MenuBarPrivate::QMacMenuBarPrivate::menubars;

