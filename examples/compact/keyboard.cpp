#include "keyboard.h"
#include <qwindowsystem_qws.h>
#include <qpainter.h>
#include <qfontmetrics.h>



Keyboard::Keyboard(QWidget* parent, const char* name, int f) :
    QWidget(parent, name, f),  shift(FALSE), lock(FALSE), ctrl(FALSE), alt(FALSE)
{
    setBackgroundMode( PaletteBase ); 
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
    "\002\051\030\002\003\200", // ~ + 123...+ BACKSPACE
    "\003\201\030\20\002\053", //TAB + qwerty..  + backslash
    "\004\202\026\036\003\203", //CAPS + asdf.. + RETURN 
    "\005\204\024\054", //\002\210", //SHIFT + zxcv... //+ UP
    "\003\205\003\206\021\207" //\002\211\002\212\002\213" 
    // 			CTRL + ALT + SPACE //+ LEFT + DOWN + RIGHT
    
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
    {	Qt::Key_Up,		0,	0 },
    {	Qt::Key_Left,		0,	0 },
    {	Qt::Key_Down,		0,	0 },
    {	Qt::Key_Right,		0,	0 }
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
    QFontMetrics fm=fontMetrics();

    QPainter p(this);
    p.setClipRect(e->rect());

    
    int d = fm.lineSpacing() - 1;
    
    //    int h = d*5;
    //int wid = 240;

    for ( int j = 0; j < 5; j++ ) {
	int y = j*d;
	int x = 0;
	int kw;
	int k= getKey( kw, j );
	while ( k ) {
	    int ww = kw*d/2;
	    QString s;
	    bool reverse = FALSE;
	    if ( k >= 0x80 ) {
		s = specialM[k - 0x80].label;
		    
		if ( k == ShiftCode ) {
		    reverse = shift;
		} else if ( k == CapsCode ) {
		    reverse = lock;
		} else if ( k == CtrlCode ) {
		    reverse = ctrl;
		} else if ( k == AltCode ) {
		    reverse = alt;
		} 
	    } else {
		s = QChar( shift^lock ? QWSServer::keyMap()[k].shift_unicode : 
			   QWSServer::keyMap()[k].unicode);
	    }
	    if ( reverse )
		p.setPen( white );
	    p.fillRect( x+1, y+1, ww-1, d-1, reverse ? black : white );
	    p.drawText( x, y, ww, d, AlignCenter, s ); 
	    if ( reverse )
		p.setPen( black );

	    p.drawLine( x, y, x, y+d );
	    x += ww;
	    k = getKey( kw );
	}
	p.drawLine( x, y, x, y+d );
	if ( j == 0 )
	    p.drawLine( 0, y, x, y );
	p.drawLine( 0, y+d, x, y+d );
    }
}


static void doKey( unsigned int unicode, int mod = 0 )
{
    QWSServer::sendKeyEvent( unicode, mod, TRUE, FALSE );
    QWSServer::sendKeyEvent( unicode, mod, FALSE, FALSE );
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
    if ( k >= 0x80 ) {
	if ( k == ShiftCode ) {
	    shift = !shift;
	    need_repaint = TRUE;
	} else if ( k == AltCode ){
	    alt = !alt;
	    need_repaint = TRUE;
	} else if ( k == CapsCode ) {
	    lock = !lock;
	    need_repaint = TRUE;
	} else if ( k == CtrlCode ) {
	    ctrl = !ctrl;
	    need_repaint = TRUE;
	} else {
	    int qk = specialM[ k - 0x80 ].qcode;
	    u = specialM[ k - 0x80 ].unicode;
	    u |= qk << 16;
	}
    } else {
	int qk = QWSServer::keyMap()[k].key_code;
	if ( qk != Key_unknown ) {
	    u = shift^lock ? QWSServer::keyMap()[k].shift_unicode : 
		QWSServer::keyMap()[k].unicode;
	    u |= qk << 16;
	}
    }
    if  ( u != -1 ) {
	doKey( u );
	need_repaint = shift || alt || ctrl;
	shift = alt = ctrl = FALSE;
	qDebug( "pressed %d -> %04x ('%c')", k, u, u&0xffff < 256 ? u&0xff : 0 );
    }
    if ( need_repaint )
	repaint( FALSE );
}

void Keyboard::mouseReleaseEvent(QMouseEvent*)
{
}


QSize Keyboard::sizeHint() const
{
    QFontMetrics fm=fontMetrics();
    int d = fm.lineSpacing() - 1;
    return QSize( 15*d, 5*d );
}
