#include <QtGui>
#include <QtNetwork>

#include <stdlib.h>

#include "dialog.h"
#include "fortuneserver.h"

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
{
    statusLabel = new QLabel(this);
    quitButton = new QPushButton(tr("Quit"), this);
    quitButton->setAutoDefault(false);

    if (!server.listen()) {
        QMessageBox::critical(this, tr("Threaded Fortune Server"),
                              tr("Unable to start the server: %1.")
                              .arg(server.errorString()));
        close();
        return;
    }

    statusLabel->setText(tr("The server is running on port %1.\n"
                            "Run the Fortune Client example now.")
                         .arg(server.serverPort()));

    connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(quitButton);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(statusLabel);
    mainLayout->addLayout(buttonLayout);

    setWindowTitle(tr("Threaded Fortune Server"));
}
