#include "qpushbutton.h"
#include "qapplication.h"
#include "qmessagebox.h"

class SimpleManager
: public QObject
{
public:
    SimpleManager() {}

    virtual bool event(QEvent * e)
    {
        execute();
    }

protected:
    void execute()
    {
	QMessageBox::information(0, "SimpleManager", "executing function", QMessageBox::Ok);
    }
};

class MyButton : public QPushButton
{
	Q_OBJECT
public:
	MyButton(SimpleManager *m, QWidget * parent)
	: QPushButton("Send", parent), m_(m)
	{
		connect(this, SIGNAL(pressed()), this, SLOT(sendEvent()));
	}

public slots:
	void sendEvent()
	{
		QEvent *e = new QEvent(7890);
		QApplication::postEvent(m_, e);
	}

private:
	SimpleManager * m_;
};
