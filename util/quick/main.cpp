#include "quick.h"
#include <qapp.h>
#include <qlist.h>

class WindowSet : public QObject {
    Q_OBJECT
public:
    WindowSet()
    {
    }

    ~WindowSet()
    {
    }

    void newWindow(const char* file)
    {
	Quick *w = new Quick(0,"quick",WDestructiveClose);
	quicks.append(w);
	connect(w,SIGNAL(newQuick()),this,SLOT(newWindow()));
	connect(w,SIGNAL(closeAll()),this,SLOT(closeAll()));
	connect(w,SIGNAL(destroyed()),this,SLOT(removeSenderFromList()));
	qApp->setMainWidget( w ); // Just geometry, etc.
	qApp->setMainWidget( 0 ); // Not Close==Quit
	if (file)
	    w->open(file,FALSE);
	w->show();
    }

signals:
    void lastWindowClosed();

public slots:
    void newWindow()
    {
	newWindow(0);
    }

    void closeAll()
    {
	QListIterator<Quick> i(quicks);
	while (Quick* q = i.current()) {
	    ++i;
	    q->closeWindow();
	}
    }

private slots:
    void removeSenderFromList()
    {
	// Safe cast - we know who we connected to this private slot
	quicks.remove((Quick*)sender());
    }

private:
    QList<Quick> quicks;
};

int main(int argc, char** argv)
{
    QApplication::setFont(QFont("Helvetica"));
    QApplication app(argc,argv);
    WindowSet windows;
    if (argc==1) {
	windows.newWindow();
    } else {
	for (int arg=1; arg<argc; arg++)
	    windows.newWindow(argv[arg]);
    }
    QObject::connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));
    return app.exec();
}

#include "main.moc"
