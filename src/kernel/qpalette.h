/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpalette.h#38 $
**
** Definition of QColorGroup and QPalette classes
**
** Created : 950323
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
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


    // Do not change the order, the serialization format depends on it
    enum ColorRole { Foreground, Button, Light, Midlight, Dark, Mid,
                     Text, BrightText, ButtonText, Base, Background, Shadow,
                     Highlight, HighlightedText,
		     MaxColorRole = HighlightedText };

    const QColor &color( ColorRole ) const;
    const QBrush &brush( ColorRole ) const;
    void setColor( ColorRole, const QColor & );
    void setBrush( ColorRole, const QBrush & );

    const QColor &foreground()	const	{ return br[Foreground].color(); }
    const QColor &button()	const	{ return br[Button].color(); }
    const QColor &light()	const	{ return br[Light].color(); }
    const QColor &dark()	const	{ return br[Dark].color(); }
    const QColor &mid()		const	{ return br[Mid].color(); }
    const QColor &text()	const	{ return br[Text].color(); }
    const QColor &base()	const	{ return br[Base].color(); }
    const QColor &background()	const	{ return br[Background].color(); }

    const QColor &midlight()	const	{ return br[Midlight].color(); }
    const QColor &brightText()	const	{ return br[BrightText].color(); }
    const QColor &buttonText()	const	{ return br[ButtonText].color(); }
    const QColor &shadow()	const	{ return br[Shadow].color(); }
    const QColor &highlight()	const	{ return br[Highlight].color(); }
    const QColor &highlightedText() const{return br[HighlightedText].color(); }

#if 0
    void setForeground( const QBrush& b) { foreground_brush = b; }
    void setButton( const QBrush& b) { button_brush = b; }
    void setLight( const QBrush& b) { light_brush = b; }
    void setMidlight( const QBrush& b) { midlight_brush = b; }
    void setDark( const QBrush& b) { dark_brush = b; }
    void setMid( const QBrush& b) { mid_brush = b; }
    void setText( const QBrush& b) { text_brush = b; }
    void setBrightText( const QBrush& b) { bright_text_brush = b; }
    void setButtonText( const QBrush& b) { button_text_brush = b; }
    void setBase( const QBrush& b) { base_brush = b; }
    void setBackground( const QBrush& b) { background_brush = b; }
    void setShadow( const QBrush& b) { shadow_brush = b; }
    void setHighlight( const QBrush& b) { highlight_brush = b; }
    void setHighlightedText( const QBrush& b) { highlighted_text_brush = b; }
#endif

    bool	operator==( const QColorGroup &g ) const;
    bool	operator!=( const QColorGroup &g ) const
	{ return !(operator==(g)); }

private:
    QBrush br[MaxColorRole + 1];

    friend class QPalette;
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

    enum ColorGroup { Normal, Disabled, Active, MaxColorGroup = Active };

    const QColor &color( ColorGroup, QColorGroup::ColorRole ) const;
    const QBrush &brush( ColorGroup, QColorGroup::ColorRole ) const;
    void setColor( ColorGroup, QColorGroup::ColorRole, const QColor & );
    void setBrush( ColorGroup, QColorGroup::ColorRole, const QBrush & );

    void setColor( QColorGroup::ColorRole, const QColor & );
    void setBrush( QColorGroup::ColorRole, const QBrush & );

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
    QBrush &directBrush( ColorGroup, QColorGroup::ColorRole ) const;

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
