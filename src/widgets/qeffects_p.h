#ifndef QEFFECTS_H
#define QEFFECTS_H

#include "qnamespace.h"

class QWidget;

struct QEffects
{
    enum Direction {
	LeftScroll	= 0x0001,
	RightScroll	= 0x0002,
	UpScroll	= 0x0004,
	DownScroll	= 0x0008
    };

    typedef uint DirFlags;
};

extern void Q_EXPORT qScrollEffect( QWidget*, QEffects::DirFlags dir = QEffects::DownScroll, int time = -1 );
extern void Q_EXPORT qFadeEffect( QWidget*, int time = -1 );

#endif // QEFFECTS_H
