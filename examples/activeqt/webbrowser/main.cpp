/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qapplication.h>
#include <qmessagebox.h>
#include <qprogressbar.h>
#include <qstatusbar.h>

#include <qmainwindow.h>
#include "ui_mainwindow.h"

class MainWindow : public QMainWindow, public Ui::MainWindow
{
    Q_OBJECT
public:
    MainWindow();

public slots:
    void on_WebBrowser_TitleChange(const QString &title);
    void on_WebBrowser_ProgressChange(int a, int b);
    void on_WebBrowser_CommandStateChange(int cmd, bool on);
    void on_WebBrowser_BeforeNavigate();
    void on_WebBrowser_NavigateComplete(QString);

    void on_actionGo_triggered();
    void on_actionNewWindow_triggered();
    void on_actionAbout_triggered();
    void on_actionAboutQt_triggered();

private:
    QProgressBar *pb;
};

MainWindow::MainWindow()
{
    setupUi(this);

    connect(addressEdit, SIGNAL(returnPressed()), actionGo, SLOT(trigger()));
    connect(actionBack, SIGNAL(triggered()), WebBrowser, SLOT(GoBack()));
    connect(actionForward, SIGNAL(triggered()), WebBrowser, SLOT(GoForward()));
    connect(actionStop, SIGNAL(triggered()), WebBrowser, SLOT(Stop()));
    connect(actionRefresh, SIGNAL(triggered()), WebBrowser, SLOT(Refresh()));
    connect(actionHome, SIGNAL(triggered()), WebBrowser, SLOT(GoHome()));
    connect(actionSearch, SIGNAL(triggered()), WebBrowser, SLOT(GoSearch()));

    pb = new QProgressBar(statusBar());
    pb->setTextVisible(false);
    pb->hide();
    statusBar()->addPermanentWidget(pb);

    WebBrowser->dynamicCall("GoHome()");
}

void MainWindow::on_WebBrowser_TitleChange(const QString &title)
{
    setWindowTitle("Qt WebBrowser - " + title);
}

void MainWindow::on_WebBrowser_ProgressChange(int a, int b)
{
    if (a <= 0 || b <= 0) {
	pb->hide();
	return;
    }
    pb->show();
    pb->setRange(0, b);
    pb->setValue(a);
}


void MainWindow::on_WebBrowser_CommandStateChange(int cmd, bool on)
{
    switch (cmd) {
    case 1:
	actionForward->setEnabled(on);
	break;
    case 2:
	actionBack->setEnabled(on);
	break;
    }
}

void MainWindow::on_WebBrowser_BeforeNavigate()
{
    actionStop->setEnabled(true);
}

void MainWindow::on_WebBrowser_NavigateComplete(QString)
{
    actionStop->setEnabled(false);
}

void MainWindow::on_actionGo_triggered()
{
    WebBrowser->dynamicCall("Navigate(const QString&)", addressEdit->text());
}


void MainWindow::on_actionNewWindow_triggered()
{
    MainWindow *window = new MainWindow;
    window->show();
    if (addressEdit->text().isEmpty())
	return;
    window->addressEdit->setText(addressEdit->text());
    window->actionStop->setEnabled(true);
    window->on_actionGo_triggered();
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, tr("About WebBrowser"),
		tr("This Example has been created using the ActiveQt integration into Qt Designer.\n"
		   "It demonstrates the use of QAxWidget to embed the Internet Explorer ActiveX\n"
		   "control into a Qt application."));
}

void MainWindow::on_actionAboutQt_triggered()
{
    QMessageBox::aboutQt(this, tr("About Qt"));
}


#include "main.moc"

int main(int argc, char ** argv)
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
