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
  If code is > 0x80, then we have one key of the given length
  with keycode = code & 0x7f
  Otherwise we have length/2 standard keys starting with code.
  
 */
static const uchar * const keyboard[5] = { 
    "\002\251\030\002\003\216", // ~ + 123...+ BACKSPACE
    "\003\217\030\20\002\053", //TAB + qwerty..  + backslash
    "\004\272\026\036\003\234", //CAPS + asdf.. + RETURN 
    "\005\252\024\054", //SHIFT + zxcv...
    "\003\235\003\270\020\271" // CTRL + ALT + SPACE
};

static const int capscode = 072;
static const int shiftcode = 052;
static const int ctrlcode = 035;
static const int altcode = 070;
static const int retcode = 034;
static const int bscode = 016;
static const int tabcode = 017;


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
	k = row[1]&0x7f;
    } else {
	k = row[1]+i2/2;
    }
    
    return k;
}


/*
  return scancode and width of first key in row \a j if \a j >= 0,
  or next key on current row if \a j < 0.
  
*/

int Keyboard::getKey( int &w, int j = -1 ) {
    static const uchar *row = 0;
    static int key_i = 0;
    static int scancode = 0;
    if ( j >= 0 )
	row = keyboard[j];
    
    if ( !row || !*row )
	return 0;    
    if ( row[1] > 0x80 ) {
	scancode = row[1] & 0x7f;
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

    
    int d = fm.lineSpacing();
    
    //    int h = d*5;
    //int wid = 240;

    for ( int j = 0; j <= 5; j++ ) {
	int y = j*d;
	int x = 0;
	if ( j < 5 ) {
	    int kw;
	    int k= getKey( kw, j );
	    while ( k ) {
		int ww = kw*d/2;
		QString s;
		bool reverse = FALSE;
		if ( k == shiftcode ) {
		    s = "Shift";
		    reverse = shift;
		} else if ( k == capscode ) {
		    s = "Caps";
		    reverse = lock;
		} else if ( k == ctrlcode ) {
		    s = "Ctrl";
		    reverse = ctrl;
		} else if ( k == altcode ) {
		    s = "Alt";
		    reverse = alt;
		} else if ( k == retcode ) {
		    s = "Ret";
		} else if ( k == bscode ) {
		    s = "BS";
		} else if ( k == tabcode ) {
		    s = "Tab";
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
	}
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
    int d = fm.lineSpacing();

    int i2 = (e->x() * 2) / d;
    int j = e->y() / d;
    
    int k = keycode( i2, j );
    bool need_repaint = FALSE;
    int qk = QWSServer::keyMap()[k].key_code;
    if ( qk == Key_Shift ) {
	shift = !shift;
	need_repaint = TRUE;
    } else if ( qk == Key_Alt ){
	alt = !alt;
	need_repaint = TRUE;
    } else if ( qk == Key_CapsLock ) {
	lock = !lock;
	need_repaint = TRUE;
    } else if ( qk == Key_Control ) {
	ctrl = !ctrl;
	need_repaint = TRUE;
    } else if ( qk != Key_unknown ) {
	int u = shift^lock ? QWSServer::keyMap()[k].shift_unicode : 
		QWSServer::keyMap()[k].unicode;
	u |= qk << 16;
	doKey( u );
	if ( shift ) {
	    shift = FALSE;
	    need_repaint = TRUE;
	}
	alt = FALSE;
	ctrl = FALSE;
	qDebug( "pressed %d -> %04x ('%c')", k, u, u&0xffff < 256 ? u&0xff : 0 );
    }
    if ( need_repaint )
	repaint( FALSE );
}

void Keyboard::mouseReleaseEvent(QMouseEvent*)
{
}
