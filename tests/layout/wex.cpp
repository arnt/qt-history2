/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** EDITIONS: UNKNOWN
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include <qapplication.h>
#include <qlabel.h>
#include <qcolor.h>
#include <qgroupbox.h>
#include <qpushbutton.h>

#include <qlayout.h>


class HFWLabel : public QLabel {
public:
    HFWLabel( QWidget *parent );
    QSizePolicy sizePolicy() const;
    int heightForWidth(int) const;

};

HFWLabel::HFWLabel( QWidget *parent ) : QLabel( parent )
{
    setBackgroundColor( cyan.dark() );

    setText( "heightforwidth" );
}

int HFWLabel::heightForWidth( int w ) const
{
    if ( w < 10 )
	w = 10;
    return 10000 / w;
}

QSizePolicy HFWLabel::sizePolicy() const
{
    return QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred,
			TRUE );
}

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    QGroupBox *f = new QGroupBox;
    f->setFrameStyle( QFrame::Panel  | QFrame::Sunken );

    QBoxLayout *gm = new QBoxLayout( f, QBoxLayout::TopToBottom, 5 );
#if 1
    QBoxLayout *b1 = new QBoxLayout( QBoxLayout::LeftToRight);
    gm->addLayout( b1/*, 10 */);

    QWidget *urk = 0;
    for ( int i=0; i<4; i++ ) {
	QLabel* lab = new QLabel(f);
	lab->setText("Testing");
	lab->setFrameStyle( QFrame::Panel   | QFrame::Plain );
	lab->setAlignment( Qt::AlignTop | Qt::AlignHCenter );
	lab->setBackgroundColor(Qt::yellow);

	b1->addWidget( lab, 20 );
	if ( i == 2 ) urk = lab;
    }
    b1->setStretchFactor( urk, 10 );

    {
	QLabel* l = new QLabel( f );
	l->setQML("<h1>heading</h1><p>Yet another <em>Paragraph</em> that contains lot of text that will probably need several <b>lines</b> in this label. Paul asked me to add even more text to test his new feature. Jeg skal skrive grammatisk riktig norsk i stedet for engelsk. That should be enough, das sollte hinreichend sein.</p>");
	b1->addWidget( l, 20 );
    }

    QPushButton* qb = new QPushButton( "Quit", f );
    a.connect( qb, SIGNAL(clicked()), SLOT(quit()) );
    b1->addWidget( qb, 0, Qt::AlignTop );

#endif
    QLabel* large = new QLabel(f);
    large->setText("This is supposed to be a large window\n you know.");
    large->setFrameStyle( QFrame::Panel | QFrame::Plain );
    large->setAlignment( Qt::AlignVCenter | Qt::AlignHCenter );
    large->setBackgroundColor(Qt::white);

    QBoxLayout *b2 = new QBoxLayout( QBoxLayout::LeftToRight);
    gm->addLayout( b2, 50 );

    b2->addWidget( large, 100 ); // hstretch

    {
	QLabel* s = new QLabel(f);
	s->setText("This\n is\n supposed\n to be\n centered\n relatively.");
	s->setFrameStyle( QFrame::Panel | QFrame::Plain );
	s->setAlignment( Qt::AlignVCenter | Qt::AlignHCenter );
	s->setBackgroundColor(Qt::cyan);
	b2->addWidget( s, 10, Qt::AlignCenter );
    }

    {
	QLabel* s = new QLabel(f);
	s->setText("This is a widget inside the outermost box");
	s->setFrameStyle( QFrame::Panel | QFrame::Plain );
	s->setAlignment( Qt::AlignVCenter | Qt::AlignHCenter );
	s->setBackgroundColor(Qt::red);
	gm->addWidget( s, 1 );
    }

    f->show();

    a.setMainWidget(f);
    return a.exec();
}
