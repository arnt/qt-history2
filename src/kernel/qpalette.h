/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpalette.h#34 $
**
** Definition of QColorGroup and QPalette classes
**
** Created : 950323
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of Troll Tech's internal development tree for Qt.
**
** This header text will be replaced by an appropriate text by the
** mkdist script which generates external distributions.
**
** If you are using the Qt Professional Edition or the Qt Free Edition,
** please notify Troll Tech at <info@troll.no> if you see this text.
**
** To Troll Tech developers: This header was generated by the script
** fixcopyright-int. It has the same number of text lines as the free
** and professional editions to avoid line number inconsistency.
**
*****************************************************************************/

#ifndef QPALETTE_H
#define QPALETTE_H

#ifndef QT_H
#include "qwindowdefs.h"
#include "qcolor.h"
#include "qshared.h"
#include "qbrush.h"
#endif // QT_H


class Q_EXPORT QColorGroup				// color group class
{
public:
    QColorGroup();				// all colors black
    QColorGroup( const QColor &foreground, const QColor &button,
		 const QColor &light, const QColor &dark, const QColor &mid,
		 const QColor &text, const QColor &base );

    QColorGroup( const QBrush &foreground, const QBrush &button,
		 const QBrush &light, const QBrush &dark, const QBrush &mid,
		 const QBrush &text,  const QBrush &bright_text, const QBrush &base,
		 const QBrush &background);
   ~QColorGroup();

    const QColor &foreground()	const	{ return foreground_brush.color(); }
    const QColor &button()	const	{ return button_brush.color(); }
    const QColor &light()	const	{ return light_brush.color(); }
    const QColor &midlight()	const	{ return midlight_brush.color(); }
    const QColor &dark()	const	{ return dark_brush.color(); }
    const QColor &mid()		const	{ return mid_brush.color(); }
    const QColor &text()	const	{ return text_brush.color(); }
    const QColor &brightText()	const	{ return bright_text_brush.color(); }
    const QColor &base()	const	{ return base_brush.color(); }
    const QColor &background()	const	{ return background_brush.color(); }

    const QBrush &fillForeground() const {return foreground_brush; }
    const QBrush &fillButton() const {return button_brush; }
    const QBrush &fillLight() const {return light_brush; }
    const QBrush &fillMidlight() const {return midlight_brush; }
    const QBrush &fillDark() const {return dark_brush; }
    const QBrush &fillMid() const {return mid_brush; }
    const QBrush &fillText() const {return text_brush; }
    const QBrush &fillBrightText() const {return bright_text_brush; }
    const QBrush &fillBase() const {return base_brush; }
    const QBrush &fillBackground() const {return background_brush; }

    bool	operator==( const QColorGroup &g ) const;
    bool	operator!=( const QColorGroup &g ) const
					{ return !(operator==(g)); }
private:
    QBrush foreground_brush;
    QBrush button_brush;
    QBrush light_brush;
    QBrush dark_brush;
    QBrush mid_brush;
    QBrush text_brush;
    QBrush bright_text_brush;
    QBrush base_brush;
    QBrush background_brush;
    QBrush midlight_brush;
};


class Q_EXPORT QPalette					// palette class
{
public:
    QPalette();
    QPalette( const QColor &button );
    QPalette( const QColor &button, const QColor &background );
    QPalette( const QColorGroup &normal, const QColorGroup &disabled,
	      const QColorGroup &active );
    QPalette( const QPalette & );
   ~QPalette();
    QPalette &operator=( const QPalette & );

    QPalette	copy() const;

    const QColorGroup &normal()	  const { return data->normal; }
    const QColorGroup &disabled() const { return data->disabled; }
    const QColorGroup &active()	  const { return data->active; }

    void	setNormal( const QColorGroup & );
    void	setDisabled( const QColorGroup & );
    void	setActive( const QColorGroup & );

    bool	operator==( const QPalette &p ) const;
    bool	operator!=( const QPalette &p ) const
					{ return !(operator==(p)); }
    bool	isCopyOf( const QPalette & );

    int		serialNumber() const	{ return data->ser_no; }

private:
    void	detach();

    struct QPalData : public QShared {
	QColorGroup normal;
	QColorGroup disabled;
	QColorGroup active;
	int	    ser_no;
    } *data;
};


/*****************************************************************************
  QColorGroup/QPalette stream functions
 *****************************************************************************/

Q_EXPORT QDataStream &operator<<( QDataStream &, const QColorGroup & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QColorGroup & );

Q_EXPORT QDataStream &operator<<( QDataStream &, const QPalette & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QPalette & );


#endif // QPALETTE_H
