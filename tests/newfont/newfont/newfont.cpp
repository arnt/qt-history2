#include <qapplication.h>
#include <qfont.h>
#include <qlayout.h>
#include <qwidget.h>
#include <qpushbutton.h>
#include <qtextview.h>
#include <qfile.h>
#include <qtextstream.h>


class NewFont : public QWidget
{
public:
    NewFont(QWidget *parent = 0, const char *name = 0)
	: QWidget(parent, name)
    {
	textview = new QTextView(this, "new font testing textview");

	QFile file("x-utf8.html");
	if (file.open(IO_ReadOnly)) {
	    QTextStream stream(&file);
            stream.setEncoding(QTextStream::UnicodeUTF8);
	    textview->setText(stream.read());
	}

        button = new QPushButton("&Quit", this, "new font testing QUITBUTTON");
        connect(button, SIGNAL(clicked()), qApp, SLOT(quit()));

        QVBoxLayout *hbox = new QVBoxLayout(this, 4, 4, "layout");
        hbox->addWidget(textview);
        hbox->addWidget(button);
    }


private:
    QTextView *textview;
    QPushButton *button;
};


int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    NewFont newfont(0, "newfont testing widget");

    app.setMainWidget(&newfont);
    newfont.resize(650, 1000);
    newfont.show();

    return app.exec();
}
