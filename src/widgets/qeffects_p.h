#ifndef QEFFECTS_H
#define QEFFECTS_H

#include "qnamespace.h"

class QWidget;

extern void Q_EXPORT scrollEffect( QWidget*, Qt::Orientation orient = Qt::Vertical, int time = 150 );
extern void Q_EXPORT fadeEffect( QWidget*, int time = 150 );

#endif // QEFFECTS_H
