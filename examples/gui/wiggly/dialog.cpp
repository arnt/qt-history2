#include <QtGui>

#include "dialog.h"
#include "wigglywidget.h"

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
{
    WigglyWidget *wigglyWidget = new WigglyWidget(this);
    QLineEdit *lineEdit = new QLineEdit(this);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(wigglyWidget);
    layout->addWidget(lineEdit);

    connect(lineEdit, SIGNAL(textChanged(QString)),
            wigglyWidget, SLOT(setText(QString)));

    lineEdit->setText(tr("Hello world!"));

    setWindowTitle(tr("Wiggly"));
    resize(320, 145);
}
