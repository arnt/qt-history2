#ifndef QEFFECTS_H
#define QEFFECTS_H

#include "qnamespace.h"

class QWidget;

extern void Q_EXPORT qScrollEffect( QWidget*, int orient = 1, int time = -1 );
extern void Q_EXPORT qFadeEffect( QWidget*, int time = 200 );

#endif // QEFFECTS_H
