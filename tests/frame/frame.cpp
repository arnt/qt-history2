#include "frame.h"
#include <qpainter.h>
#include <qapplication.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qimage.h>
#include <unistd.h>

Main::Main(QWidget* parent, const char* name, int f) :
    QWidget(parent, name, f)
{
    setBackgroundMode(PaletteBase);
    QGridLayout* grid = new QGridLayout(this, 38, 17, 3, 3);
    for (int y=0; y<38; y++) {
	for (int x=0; x<17; x++) {
	    if ( y == 0 ) {
		if ( x == 16 ) {
		    QLabel* l = new QLabel("lineWidth()",this);
		    l->setBackgroundMode(PaletteBase);
		    grid->addWidget(l,y,x);
		} else {
		    if ( (x%4) == 0 ) {
			QString txt;
			txt.setNum(x/4+1);
			QLabel* l = new QLabel(txt,this);
			l->setAlignment(AlignCenter);
			l->setBackgroundColor(QColor( "#D0D0FF" ));
			grid->addMultiCellWidget(l,y,y,x,x+3);
		    }
		}
	    } else if ( y == 1 ) {
		if ( x == 16 ) {
		    QLabel* l = new QLabel("midLineWidth()",this);
		    l->setBackgroundMode(PaletteBase);
		    grid->addWidget(l,y,x);
		} else {
		    QString txt;
		    txt.setNum(x%4);
		    QLabel* l = new QLabel(txt,this);
		    l->setBackgroundColor(QColor( "#EEEEFF" ));
		    l->setAlignment(AlignCenter);
		    grid->addWidget(l,y,x);
		}
	    } else {
		int shape=(y-2)/3+1;
		int shadow=((y-2)%3+1)*16;
		if ( x == 16 ) {
		    QString txt;
		    //txt += "QFrame::";
		    txt +=
			  shape == QFrame::Box ? "Box"
			: shape == QFrame::Panel ? "Panel"
			: shape == QFrame::WinPanel ? "WinPanel"
			: shape == QFrame::HLine ? "HLine"
			: shape == QFrame::VLine ? "VLine"
			: shape == QFrame::StyledPanel ? "StyledPanel"
			: shape == QFrame::PopupPanel ? "PopupPanel"
			: shape == QFrame::MenuBarPanel ? "MenuBarPanel"
			: shape == QFrame::ToolBarPanel ? "ToolBarPanel"
			: shape == QFrame::LineEditPanel ? "LineEditPanel"
			: shape == QFrame::TabWidgetPanel ? "TabWidgetPanel"
			: shape == QFrame::GroupBoxPanel ? "GroupBoxPanel"
			: "???";
		    txt += " + ";
		    //txt += "QFrame::";
		    txt +=
			  shadow == QFrame::Plain ? "Plain"
			: shadow == QFrame::Raised ? "Raised"
			: shadow == QFrame::Sunken ? "Sunken"
			: "???";
		    QLabel* l = new QLabel(txt,this);
		    l->setBackgroundMode(PaletteBase);
		    grid->addWidget(l,y,x);
		} else {
		    QFrame* f = new QFrame(this);
		    f->setFixedSize(28,28);
		    f->setLineWidth(x/4+1);
		    f->setMidLineWidth(x%4);
		    f->setFrameStyle(shape+shadow);
		    grid->addWidget(f,y,x);
		}
	    }
	}
    }
}

main(int argc, char** argv)
{
    QApplication app(argc, argv);
    QApplication::setFont( QFont("Helvetica") );

    Main m;
    app.setMainWidget(&m);
    m.show();
    app.processEvents();
    sleep(1);
    app.processEvents();
    QPixmap pm = QPixmap::grabWidget(&m);
    pm.convertToImage().convertDepth(8).save("frames.png","PNG");
}
