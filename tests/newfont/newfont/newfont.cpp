#include <qapplication.h>
#include <qfont.h>
#include <qfontdatabase.h>
#include <qlayout.h>
#include <qwidget.h>
#include <qpushbutton.h>
#include <qtextview.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qfontdialog.h>


class NewFont : public QWidget
{
    Q_OBJECT
    
public:
    NewFont(QWidget *parent = 0, const char *name = 0)
	: QWidget(parent, name)
    {
	textview = new QTextView(this, "new font testing textview");

	QFile file("arabic.html");
	if (file.open(IO_ReadOnly)) {
	    QTextStream stream(&file);
            stream.setEncoding(QTextStream::UnicodeUTF8);
	    textview->setText(stream.read());
	}

	fbutton = new QPushButton("&Font Dialog", this, "newfont testing font button");
	connect(fbutton, SIGNAL(clicked()), this, SLOT(showFD()));
	
        qbutton = new QPushButton("&Quit", this, "new font testing QUITBUTTON");
        connect(qbutton, SIGNAL(clicked()), qApp, SLOT(quit()));

        QVBoxLayout *hbox = new QVBoxLayout(this, 4, 4, "layout");
        hbox->addWidget(textview);
        hbox->addWidget(fbutton);
        hbox->addWidget(qbutton);
    }
    
    
public slots:
    void showFD();


private:
    QTextView *textview;
    QPushButton *qbutton, *fbutton;
};


void NewFont::showFD()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, QApplication::font(), this);
    
    if (ok) {
	textview->setFont(font);
    }
}


int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QFont f;
    f.setFamily( "nakshi" );
    f.setPointSize(18);
    app.setFont(f);
    NewFont newfont(0, "newfont testing widget");
    
    newfont.resize(600, 1000);
    app.setMainWidget(&newfont);
    newfont.show();

    return app.exec();
}


#include "newfont.moc"
