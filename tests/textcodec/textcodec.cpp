#include "textcodec.h"
#include "qtextcodec.h"
#include <qpainter.h>
#include <qapplication.h>
#include <qfile.h>
#include <qkeycode.h>
#include <ctype.h>
#include <stdlib.h>

Main::Main(const char* enc, QWidget* parent, const char* name, int f) :
    QLabel("Type hex",parent, name, f)
{
    codec = QTextCodec::codecForName(enc);
    if ( !codec )
	fatal("No codec for \"%s\"",enc);
    setFocus();
}

void Main::keyPressEvent(QKeyEvent* e)
{
    char ch = e->ascii();
    if ( isalnum(ch) && toupper(ch) <= 'F' ) {
	hex += toupper(ch);
    } else if ( e->key() == Key_Backspace ) {
	hex.truncate(QMAX(1,hex.length())-1);
    } else
	return;

    QString t;
    QString uhex;
    QString xhex;
    QString uc;
    Q1String xc;
    for (uint i=0; i+4<=hex.length(); i+=4) {
	char u[5];
	for (int j=0; j<4; j++)
	    u[j] = hex[i+j];
	u[4]=0;
	ushort code = strtol(u,0,16);
	uhex += u;
	uhex += " ";
	uc += QChar(code);
    }
    for (uint i=0; i+2<=hex.length(); i+=2) {
	char x[3];
	for (int j=0; j<2; j++)
	    x[j] = hex[i+j];
	x[2]=0;
	xhex += x;
	xhex += " ";
	uchar code = strtol(x,0,16);
	xc += code;
    }

    int l=uc.length();
    uchar* encoded = (uchar*)codec->fromUnicode(uc,l);
    QString xxhex;
    while (l--) {
	QString h;
	h.sprintf("%02x ",*encoded++);
	xxhex += h;
    }

    QString decoded = codec->toUnicode(xc,xc.length());
    QString uuhex;
    for (uint z=0; z<decoded.length(); z++) {
	QString h;
	h.sprintf("%02x%02x ",decoded[z].row,decoded[z].cell);
	uuhex += h;
    }

    t += hex;
    t += "\n";
    t += "\n";

    t += codec->name();
    t += " -> Unicode\n";
    t += "  x: "; t += xhex; t += "\n";
    t += "  U: "; t += uuhex;
    t += " \""; t += decoded; t += "\"\n";
    t += "\n";

    t += "Unicode -> ";
    t += codec->name();
    t += "\n";
    t += "  U: "; t += uhex;
    t += " \""; t += uc; t += "\"\n";
    t += "  x: "; t += xxhex; t += "\n";

    setText(t);
}

main(int argc, char** argv)
{
    QApplication::setColorSpec( QApplication::ManyColor );
    QApplication app(argc, argv);
    QApplication::setFont( QFont("Helvetica",24) );

    char* enc = argv[1];
    if (QFile::exists(enc)) {
	QFile f(enc);
	f.open(IO_ReadOnly);
	enc = QTextCodec::loadCharmap(&f)->name();
    }

    Main m(argv[1]);
    m.show();

    QObject::connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));

    return app.exec();
}
