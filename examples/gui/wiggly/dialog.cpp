#include "dialog.h"

Dialog::Dialog()
    : QDialog(0)
{
    WigglyWidget *wiggly = new WigglyWidget(this);
    QLineEdit *lineEdit = new QLineEdit(this);

    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->addWidget(wiggly, 1);
    vbox->addWidget(lineEdit);

    connect(lineEdit, SIGNAL(textChanged(const QString&)),
            wiggly, SLOT(setText(const QString&)));

    setWindowTitle(tr("Wiggly"));
    resize(320, 200);

    lineEdit->setText(tr("Hello World!"));
}


