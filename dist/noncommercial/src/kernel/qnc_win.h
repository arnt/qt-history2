#if defined(QT_NON_COMMERCIAL)

#include "qmessagebox.h"

#define IDM_ABOUTQT 1

static char* ForK( const char *f ) {
    char *res = new char[strlen(f)+1];
    int i = 0;
    while ( f[i] ) {
	res[i] = f[i] ^ 5;
	i++;
    }
    res[i] = '\0';
    return res;
}

static char* unForK( const char *f ) {
    char *res = new char[strlen(f)+1];
    int i = 0;
    while ( f[i] ) {
	res[i] = f[i] ^ 5;
	i++;
    }
    res[i] = '\0';
    return res;
}

#define QT_NC_WNDPROC \
    if ( msg.message == WM_SETTEXT )\
    {\
	QString qstr;\
	if ( qt_winver & Qt::WV_NT_based ) {\
	    LPCTSTR *str = (LPCTSTR *)msg.lParam;\
	    qstr = qt_winQString( str );\
	} else {\
	    qstr = QString::fromLocal8Bit( (const char*)msg.lParam );\
	}\
	char* q = ForK("Tq"); /* "Qt" */\
	char* t = ForK("Qwjiiq`fm"); /* "Trolltech" */\
	char* f = ForK("^Cw``rdw`X%(%"); /* "[Freeware] - " */\
	if ( qstr.find( QString(f) ) == -1 && qstr.find( QString(t) ) == -1 && qstr.find( QString(q) ) == -1 ) {\
	    widget = (QETWidget*)QWidget::find( hwnd );\
	    if ( ! (widget->parentWidget() && widget->parentWidget()->caption().find( QString(t) ) != -1\
		&& widget->parentWidget()->caption().find( QString(q) ) != -1 ) ) {\
		if ( widget->caption().find( QString(q) + QString("Example") ) == -1 ) {\
		    if ( ! ( widget->inherits("QFileDialog") || widget->inherits("QMessageBox")\
			|| widget->inherits("QFontDialog") || widget->inherits("QColorDialog") ) ) {\
			widget->setCaption( qstr );\
			return NULL;\
		    }\
		}\
	    }\
	}\
	delete[] q;\
	delete[] t;\
	delete[] f;\
    }


#define QT_NC_SYSCOMMAND \
		} else if ( wParam == IDM_ABOUTQT ) {\
		    QMessageBox::aboutQt( widget->topLevelWidget(), "About Qt" );



#define QT_NC_WIDGET_CREATE \
    HMENU menu = GetSystemMenu( winId(), FALSE );\
    AppendMenuA( menu, MF_SEPARATOR, NULL, NULL ); \
    AppendMenuA( menu, MF_STRING, IDM_ABOUTQT, "About Qt" );



#define QT_NC_CAPTION \
    QString cap;\
    char* t = ForK("Qwjiiq`fm"); /* "Trolltech" */\
    char* q = ForK("Tq"); /* "Qt" */\
    char* f = ForK("^Cw``rdw`X%(%"); /* "[Freeware] - " */\
    if ( caption.find( QString(t) ) != -1 && caption.find( QString(q) ) != -1 )\
	cap = caption;\
    else if ( caption.find( QString(q) + " Example" ) != -1 )\
	cap = caption;\
    else if ( parentWidget() && parentWidget()->caption().find( QString(t) ) != -1 && parentWidget()->caption().find( QString(q) ) != -1 )\
	cap = caption;\
    else if ( inherits("QFileDialog") || inherits("QMessageBox") || inherits("QFontDialog") || inherits("QColorDialog") )\
	cap = caption;\
    else\
	cap = QString(f) + caption;\
    delete[] t;\
    delete[] q;\
    delete[] f;


#define QT_NC_SHOW_WINDOW \
	if ( isTopLevel() && caption() == QString::null\
	    && ! ( inherits("QFileDialog") || inherits("QMessageBox")\
	    || inherits("QFontDialog") || inherits("QColorDialog") ) ) {\
	    char* f = ForK("^Cw``rdw`X%(%"); /* "[Freeware] - " */\
	    setCaption( QString(f) + QString(qApp->name()) );\
	    delete[] f;\
	}


#define QT_NC_MSGBOX \
    	char* q = unForK("\x54\x71"); \
	char* t = unForK("\x51\x77\x6a\x69\x69\x71\x60\x66\x6d"); \
	char* line1 = unForK("\x39\x6d\x36\x3b\x44\x67\x6a\x70\x71\x25\x54\x71\x39\x2a\x6d\x36\x3b"); \
	char* line2 = unForK("\x39\x67\x3b\x39\x63\x6a\x6b\x71\x25\x66\x6a\x69\x6a\x77\x38\x77\x60\x61\x3b\x51\x6d\x6c\x76\x25\x64\x75\x75\x69\x6c\x66\x64\x71\x6c\x6a\x6b\x25\x6c\x76\x25\x6b\x6a\x6b\x28\x66\x6a\x68\x68\x60\x77\x66\x6c\x64\x69\x25"); \
	char* line3 = unForK("\x76\x6a\x63\x71\x72\x64\x77\x60\x39\x2a\x63\x6a\x6b\x71\x3b\x39\x2a\x67\x3b"); \
	char* line4 = unForK("\x39\x75\x3b\x51\x6d\x6c\x76\x25\x64\x75\x75\x69\x6c\x66\x64\x71\x6c\x6a\x6b\x25\x72\x64\x76\x25\x66\x77\x60\x64\x71\x60\x61\x25\x72\x6c\x71\x6d\x25\x6b\x6a\x6b\x28\x66\x6a\x68\x68\x60\x77\x66\x6c\x64\x69\x25\x54\x71\x25\x73\x60\x77\x76\x6c\x6a\x6b"); \
	char* line5 = unForK("\x25\x20\x34\x2b\x39\x2a\x75\x3b"); \
	if ( !parentWidget() || ( parentWidget()->caption().find( QString(q) ) == -1 && parentWidget()->caption().find( QString(t) ) == -1 ) ) { \
	    *translatedTextAboutQt = tr( QString(line1) +  \
		  QString(line2) +  \
		  QString(line3) +  \
		  QString(line4) + QString(line5) ).arg( QT_VERSION_STR ); \
	} else { \
	    *translatedTextAboutQt = tr( "<h3>About Qt</h3>" \
		  "<p>This application was created with Qt version %1.</p>" ).arg( QT_VERSION_STR ); \
	} \
	*translatedTextAboutQt += "<p>Qt is a multi-platform C++ GUI application framework from " \
				  "Trolltech. Qt provides single-source portability across Windows " \
				  "95/98/NT4/Me/2000, Linux, Solaris, HP-UX and many other versions of " \
				  "Unix with X11. Qt is also available for embedded devices.</p>" \
				  "<p>See <tt>http://www.trolltech.com/qt/</tt> for more information.</p>"; \
	delete[] q; \
	delete[] t; \
	delete[] line1; \
	delete[] line2; \
	delete[] line3; \
	delete[] line4; \
	delete[] line5;

#endif
