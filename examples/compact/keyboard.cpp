/****************************************************************************
** $Id: //depot/qt/main/examples/compact/keyboard.cpp#4 $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/


#include "keyboard.h"

#ifdef Q_WS_QWS

#include <qwindowsystem_qws.h>
#include <qpainter.h>
#include <qfontmetrics.h>



Keyboard::Keyboard(QWidget* parent, const char* name, int f) :
    QWidget(parent, name, f),  shift(FALSE), lock(FALSE), ctrl(FALSE),
    alt(FALSE), pressedKey(-1)
{
    setPalette(QPalette(QColor(240,240,230))); // Beige!
    //    setFont( QFont( "Helvetica", 8 ) );
}



//PC keyboard layout and scancodes

/*
  Format: length, code, length, code, ..., 0
  
  length is measured in half the width of a standard key.
  If code < 0x80 we have length/2 consecutive standard keys,
  starting with scancode code.
  
  Special keys are hardcoded, one at a time, with length of key
  and code >= 0x80, these are NOT standard PC scancodes, but are looked
  up in specialM[]. (The special keys are not keymappable.)
  
 */
static const uchar * const keyboard[5] = { 
    (const uchar *const)"\002\051\030\002\003\200",//\002\214\002\215\002\216",
    //~ + 123...+ BACKSPACE //+ INSERT + HOME + PGUP

    (const uchar *const)"\003\201\030\20\002\053",//\002\217\002\220\002\221",
    //TAB + qwerty..  + backslash //+ DEL + END + PGDN

    (const uchar *const)"\004\202\026\036\003\203",
    //CAPS + asdf.. + RETURN 

    (const uchar *const)"\005\204\024\054",//\002\210",
    //SHIFT + zxcv... //+ UP

    (const uchar *const)"\003\205\003\206\021\207",//\002\211\002\212\002\213" 
    //CTRL + ALT + SPACE //+ LEFT + DOWN + RIGHT
    
};


enum { BSCode = 0x80, TabCode, CapsCode, RetCode, 
       ShiftCode, CtrlCode, AltCode, SpaceCode,
       UpCode, LeftCode, DownCode, RightCode };

typedef struct SpecialMap {
    int qcode;
    ushort unicode;
    const char * label;
};

static const SpecialMap specialM[] = {
    {	Qt::Key_Backspace,	8,	"BS"  },
    {	Qt::Key_Tab,		9,	"Tab"  },
    {	Qt::Key_CapsLock,	0,	"Caps"  },
    {	Qt::Key_Return,		13,	"Ret"  },
    {	Qt::Key_Shift,		0,	"Shift"  },
    {	Qt::Key_Control,	0,	"Ctrl"  },
    {	Qt::Key_Alt,		0,	"Alt"  },
    {	Qt::Key_Space,		' ',	""  },

    // Need images?
    {	Qt::Key_Up,		0,	"^" },
    {	Qt::Key_Left,		0,	"<" },
    {	Qt::Key_Down,		0,	"v" },
    {	Qt::Key_Right,		0,	">" },
    {	Qt::Key_Insert,		0,	"I" },
    {	Qt::Key_Home,		0,	"H" },
    {	Qt::Key_PageUp,		0,	"U" },
    {	Qt::Key_End,		0,	"E" },
    {	Qt::Key_Delete,		0,	"X" },
    {	Qt::Key_PageDown,	0,	"D" }
};



static int keycode( int i2, int j )
{
    if ( j <0 || j >= 5 )
	return 0;
    const uchar *row = keyboard[j];
    
    while ( *row && *row <= i2 ) {
	i2 -= *row;
	row += 2;
    }

    if ( !*row ) return 0;
    
    int k;
    if ( row[1] >= 0x80 ) {
	k = row[1];
    } else {
	k = row[1]+i2/2;
    }
    
    return k;
}


/*
  return scancode and width of first key in row \a j if \a j >= 0,
  or next key on current row if \a j < 0.
  
*/

int Keyboard::getKey( int &w, int j ) {
    static const uchar *row = 0;
    static int key_i = 0;
    static int scancode = 0;
    if ( j >= 0 )
	row = keyboard[j];
    
    if ( !row || !*row )
	return 0;    
    if ( row[1] >= 0x80 ) {
	scancode = row[1];
	w = row[0];
	row += 2;
	return scancode;
    }    
    if ( key_i <= 0 ) {
	key_i = row[0]/2;
	scancode = row[1];
    }
    key_i--;
    w = 2;
    if ( key_i <= 0 )
	row += 2;
    return scancode++;
}
    

void Keyboard::paintEvent(QPaintEvent* e)
{
    QPainter painter(this);
    painter.setClipRect(e->rect());
    drawKeyboard( painter );
}

/*
  Draw the keyboard.

  If key >= 0, only the specified key is drawn.
*/
void Keyboard::drawKeyboard( QPainter &p, int key )
{
    QFontMetrics fm=fontMetrics();
    int d = fm.lineSpacing() - 1;
    
    //    int h = d*5;
    //int wid = 240;

    const bool threeD = TRUE;
    const QColorGroup& cg = colorGroup();
    QColor keycolor = cg.background();
    QColor keycolor_pressed = cg.mid();
    QColor keycolor_lo = cg.dark();
    QColor keycolor_hi = cg.light();
    QColor textcolor = cg.text();

    for ( int j = 0; j < 5; j++ ) {
	int y = j*d;
	int x = 0;
	int kw;
	int k= getKey( kw, j );
	while ( k ) {
	    int ww = kw*d/2;
	    if ( key < 0 || k == key ) {
		QString s;
		bool pressed = (k == pressedKey);
		if ( k >= 0x80 ) {
		    s = specialM[k - 0x80].label;
			
		    if ( k == ShiftCode ) {
			pressed = shift;
		    } else if ( k == CapsCode ) {
			pressed = lock;
		    } else if ( k == CtrlCode ) {
			pressed = ctrl;
		    } else if ( k == AltCode ) {
			pressed = alt;
		    } 
		} else {
		    s = QChar( shift^lock ? QWSServer::keyMap()[k].shift_unicode : 
			       QWSServer::keyMap()[k].unicode);
		}
		if ( pressed )
		    p.fillRect( x+1, y+1, ww-1, d-2, keycolor_pressed );
		else
		    p.fillRect( x+1, y+1, ww-1, d-2, keycolor );

		if ( threeD ) {
		    p.setPen(pressed ? keycolor_lo : keycolor_hi);
		    p.drawLine( x, y+1, x, y+d-2 );
		    p.drawLine( x+1, y+1, x+1, y+d-3 );
		    p.drawLine( x+1, y+1, x+1+ww-2, y+1 );
		}

		p.setPen(pressed ? keycolor_hi : keycolor_lo);
		p.drawLine( x+ww-1, y+1, x+ww-1, y+d-2 );

		if ( threeD ) {
		    p.setPen(keycolor_lo.light());
		    p.drawLine( x+ww-2, y+d-2, x+ww-2, y+1 );
		    p.drawLine( x+ww-2, y+d-2, x+1, y+d-2 );
		}

		p.setPen(textcolor);
		p.drawText( x, y, ww, d-2, AlignCenter, s ); 
	    }

	    x += ww;
	    k = getKey( kw );
	}
	if ( key < 0 ) {
	    if ( threeD ) {
		p.setPen(keycolor_hi);
		p.drawLine( 0, y, x-1, y );
	    }
	    p.setPen(keycolor_lo);
	    p.drawLine( 0, y+d-1, x-1, y+d-1 );
	}
    }
}



void Keyboard::mousePressEvent(QMouseEvent *e)
{
    QFontMetrics fm=fontMetrics();
    int d = fm.lineSpacing() - 1;

    int i2 = (e->x() * 2) / d;
    int j = e->y() / d;
    

    int k = keycode( i2, j );
    bool need_repaint = FALSE;
    int u = -1;
    int qk = 0;
    if ( k >= 0x80 ) {
	if ( k == ShiftCode ) {
	    shift = !shift;
	    need_repaint = TRUE;
	} else if ( k == AltCode ){
	    alt = !alt;
//	    need_repaint = TRUE;
	} else if ( k == CapsCode ) {
	    lock = !lock;
	    need_repaint = TRUE;
	} else if ( k == CtrlCode ) {
	    ctrl = !ctrl;
//	    need_repaint = TRUE;
	} else {
	    qk = specialM[ k - 0x80 ].qcode;
	    u = specialM[ k - 0x80 ].unicode;
	}
    } else {
	qk = QWSServer::keyMap()[k].key_code;
	if ( qk != Key_unknown ) {
		if ( ctrl )
		    u = QWSServer::keyMap()[k].ctrl_unicode;
		else if ( shift^lock )
		    u = QWSServer::keyMap()[k].shift_unicode;
		else 
		    u = QWSServer::keyMap()[k].unicode;
	}
    }
    if  ( u != -1 ) {
	int mod = (shift?Qt::ShiftButton:0)|(ctrl?Qt::ControlButton:0)|
		  (alt?Qt::AltButton:0);
	QWSServer::sendKeyEvent( u, qk, mod, TRUE, FALSE );
	QWSServer::sendKeyEvent( u, qk, mod, FALSE, FALSE );
	need_repaint = shift || alt || ctrl;
	shift = alt = ctrl = FALSE;
	qDebug( "pressed %d -> %04x ('%c')", k, u, u&0xffff < 256 ? u&0xff : 0 );
    }
    pressedKey = k;
    if ( need_repaint )
	repaint( FALSE );
    else {
	QPainter p(this);
	drawKeyboard( p, pressedKey );
    }
}

void Keyboard::mouseReleaseEvent(QMouseEvent*)
{
    if ( pressedKey >= 0 ) {
	int tmp = pressedKey;
	pressedKey = -1;
	QPainter p(this);
	drawKeyboard( p, tmp );
    }
}


QSize Keyboard::sizeHint() const
{
    QFontMetrics fm=fontMetrics();
    int d = fm.lineSpacing() - 1;
    return QSize( 1000, 5*d );
}

#endif // Q_WS_QWS
