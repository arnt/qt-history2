/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpalette.h#8 $
**
** Definition of QColorGroup and QPalette classes
**
** Author  : Haavard Nord
** Created : 950323
**
** Copyright (C) 1995-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QPALETTE_H
#define QPALETTE_H

#include "qwindefs.h"
#include "qcolor.h"
#include "qshared.h"


class QColorGroup				// color group class
{
public:
    QColorGroup();				// all colors black
    QColorGroup( const QColor &foreground, const QColor &background,
		 const QColor &light, const QColor &dark, const QColor &mid,
		 const QColor &text, const QColor &base);
   ~QColorGroup();

    const QColor &foreground()	const	{ return fg_col; }
    const QColor &background()	const	{ return bg_col; }
    const QColor &light()	const	{ return light_col; }
    const QColor &dark()	const	{ return dark_col; }
    const QColor &mid()		const	{ return mid_col; }
    const QColor &text()	const	{ return text_col; }
    const QColor &base()	const	{ return base_col; }

    bool	operator==( const QColorGroup &g ) const;
    bool	operator!=( const QColorGroup &g ) const
					{ return !(operator==(g)); }
private:
    QColor fg_col;
    QColor bg_col;
    QColor light_col;
    QColor dark_col;
    QColor mid_col;
    QColor text_col;
    QColor base_col;
};


class QPalette					// palette class
{
public:
    QPalette();
    QPalette( const QColorGroup &normal, const QColorGroup &disabled,
	      const QColorGroup &active );
    QPalette( const QPalette & );
   ~QPalette();
    QPalette &operator=( const QPalette & );

    QPalette  copy() const;

    const QColorGroup &normal()	  const { return data->normal; }
    const QColorGroup &disabled() const { return data->disabled; }
    const QColorGroup &active()	  const { return data->active; }

    void  setNormal( const QColorGroup & );
    void  setDisabled( const QColorGroup & );
    void  setActive( const QColorGroup & );

    bool	operator==( const QPalette &p ) const;
    bool	operator!=( const QPalette &p ) const
					{ return !(operator==(p)); }

    long	serialNumber() const	{ return data->ser_num; }

private:
    struct QPalData : QShared {			// palette data
	QColorGroup normal;
	QColorGroup disabled;
	QColorGroup active;
	long	    ser_num;
    } *data;
};


// --------------------------------------------------------------------------
// QColorGroup/QPalette stream functions
//

QDataStream &operator<<( QDataStream &, const QColorGroup & );
QDataStream &operator>>( QDataStream &, QColorGroup & );

QDataStream &operator<<( QDataStream &, const QPalette & );
QDataStream &operator>>( QDataStream &, QPalette & );


#endif // QPALETTE_H
