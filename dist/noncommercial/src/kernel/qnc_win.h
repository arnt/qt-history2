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
	char* q = ForK("Tq");\
	char* t = ForK("Qwjiiq`fm");\
	char* f = ForK("^Cw``rdw`X%(%");\
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
    char* t = ForK("Qwjiiq`fm");\
    char* q = ForK("Tq");\
    char* f = ForK("^Cw``rdw`X%(%");\
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
	    char* f = ForK("^Cw``rdw`X%(%");\
	    setCaption( QString(f) + QString(qApp->name()) );\
	    delete[] f;\
	}



#endif
