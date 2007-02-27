#include <QtGui>

#include "echowindow.h"

EchoWindow::EchoWindow()
{
    createGUI();
    setLayout(layout);
    setWindowTitle("Echo Plugin Example");

    if (!loadPlugin()) {
	QMessageBox::information(this, "Error", "Could not load the plugin");
	lineEdit->setEnabled(false);
	button->setEnabled(false);
    }
}

void EchoWindow::sendEcho()
{
    QString text = echoInterface->echo(lineEdit->text());
    label->setText(text);
}

void EchoWindow::createGUI()
{
    lineEdit = new QLineEdit; 
    label = new QLabel;
    label->setFrameStyle(QFrame::Box | QFrame::Plain);
    button = new QPushButton(tr("Send Message"));

    connect(lineEdit, SIGNAL(editingFinished()),
	    this, SLOT(sendEcho()));
    connect(button, SIGNAL(clicked()),
	    this, SLOT(sendEcho()));  

    layout = new QGridLayout;
    layout->addWidget(new QLabel(tr("Message:")), 0, 0);
    layout->addWidget(lineEdit, 0, 1);
    layout->addWidget(new QLabel(tr("Answer:")), 1, 0);
    layout->addWidget(label, 1, 1);
    layout->addWidget(button, 2, 1, Qt::AlignRight);
    layout->setSizeConstraint(QLayout::SetFixedSize);
}

bool EchoWindow::loadPlugin()
{
    QDir pluginDirectory(qApp->applicationDirPath() + "/../plugin/");
    QString fileName = pluginDirectory.entryList(
			QStringList() << "libpnp_echoplugin*").first();

    QPluginLoader pluginLoader(pluginDirectory.absoluteFilePath(fileName));
    QObject *plugin = pluginLoader.instance();
    if (plugin) {
	echoInterface = qobject_cast<EchoInterface *>(plugin);
	if(echoInterface)
	    return true;   
    }
    return false;
}
