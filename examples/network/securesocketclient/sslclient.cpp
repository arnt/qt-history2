/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "certificateinfo.h"
#include "sslclient.h"
#include "ui_sslclient.h"
#include "ui_sslerrors.h"

#include <QtGui/QScrollBar>
#include <QtGui/QStyle>
#include <QtGui/QToolButton>
#include <QtNetwork/QSslCipher>

SslClient::SslClient(QWidget *parent)
    : QWidget(parent), socket(0), padLock(0)
{
    form = new Ui_Form;
    form->setupUi(this);
    form->hostNameEdit->setSelection(0, form->hostNameEdit->text().size());
    form->sessionOutput->setHtml(tr("&lt;not connected&gt;"));

    connect(form->hostNameEdit, SIGNAL(textChanged(QString)),
            this, SLOT(updateEnabledState()));
    connect(form->connectButton, SIGNAL(clicked()),
            this, SLOT(secureConnect()));
    connect(form->sendButton, SIGNAL(clicked()),
            this, SLOT(sendData()));
}

SslClient::~SslClient()
{
    delete form;
}

void SslClient::updateEnabledState()
{
    bool unconnected = !socket || socket->state() == QAbstractSocket::UnconnectedState;

    form->hostNameEdit->setReadOnly(!unconnected);
    form->hostNameEdit->setFocusPolicy(unconnected ? Qt::StrongFocus : Qt::NoFocus);

    form->hostNameLabel->setEnabled(unconnected);
    form->portBox->setEnabled(unconnected);
    form->portLabel->setEnabled(unconnected);
    form->connectButton->setEnabled(unconnected
                                    && !form->hostNameEdit->text().isEmpty()
                                    && form->hostNameEdit->text() != "imap.example.com");

    bool connected = socket && socket->state() == QAbstractSocket::ConnectedState;
    form->sessionBox->setEnabled(connected);
    form->sessionOutput->setEnabled(connected);
    form->sessionInput->setEnabled(connected);
    form->sessionInputLabel->setEnabled(connected);
    form->sendButton->setEnabled(connected);
}

void SslClient::secureConnect()
{
    if (!socket) {
        socket = new QSslSocket(this);
        connect(socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
                this, SLOT(socketStateChanged(QAbstractSocket::SocketState)));
        connect(socket, SIGNAL(encrypted()),
                this, SLOT(socketEncrypted()));
        connect(socket, SIGNAL(sslErrors(QList<QSslError>)),
                this, SLOT(sslErrors(QList<QSslError>)));
        connect(socket, SIGNAL(readyRead()),
                this, SLOT(socketReadyRead()));
    }

    socket->connectToHostEncrypted(form->hostNameEdit->text(), form->portBox->value());
    updateEnabledState();
}

void SslClient::socketStateChanged(QAbstractSocket::SocketState state)
{
    updateEnabledState();
    if (state == QAbstractSocket::UnconnectedState) {
        form->hostNameEdit->setPalette(QPalette());
        form->hostNameEdit->setFocus();
        form->cipherLabel->setText(tr("<none>"));
        if (padLock)
            padLock->hide();
        socket->deleteLater();
        socket = 0;
    }
}

void SslClient::socketEncrypted()
{
    form->sessionOutput->clear();
    form->sessionInput->setFocus();

    QPalette palette;
    palette.setColor(QPalette::Base, QColor(255, 255, 192));
    form->hostNameEdit->setPalette(palette);

    QSslCipher ciph = socket->sessionCipher();
    QString cipher = QString("%1, %2 (%3/%4)").arg(ciph.authenticationMethod())
                     .arg(ciph.name()).arg(ciph.usedBits()).arg(ciph.supportedBits());;
    form->cipherLabel->setText(cipher);

    if (!padLock) {
        padLock = new QToolButton;
        padLock->setIcon(QIcon(":/encrypted.png"));
        padLock->setCursor(Qt::ArrowCursor);
        padLock->setToolTip(tr("Display encryption details."));

        int extent = form->hostNameEdit->height() - 2;
        padLock->resize(extent, extent);
        padLock->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);

        QHBoxLayout *layout = new QHBoxLayout(form->hostNameEdit);
        layout->setMargin(form->hostNameEdit->style()->pixelMetric(QStyle::PM_DefaultFrameWidth));
        layout->setSpacing(0);
        layout->addStretch();
        layout->addWidget(padLock);

        form->hostNameEdit->setLayout(layout);

        connect(padLock, SIGNAL(clicked()),
                this, SLOT(displayCertificateInfo()));
    } else {
        padLock->show();
    }
}

void SslClient::socketReadyRead()
{
    appendString(QString::fromUtf8(socket->readAll()));
}

void SslClient::sendData()
{
    QString input = form->sessionInput->text();
    appendString(input + "\n");
    socket->write(input.toUtf8() + "\r\n");
    form->sessionInput->clear();
}

void SslClient::sslErrors(const QList<QSslError> &errors)
{
    QDialog errorDialog;
    Ui_SslErrors ui;
    ui.setupUi(&errorDialog);
    connect(ui.certificateChainButton, SIGNAL(clicked()),
            this, SLOT(displayCertificateInfo()));

    foreach (const QSslError &error, errors)
        ui.sslErrorList->addItem(error.errorString());

    if (errorDialog.exec() == QDialog::Accepted)
        socket->ignoreSslErrors();
}

void SslClient::displayCertificateInfo()
{
    CertificateInfo *info = new CertificateInfo(this);
    info->setCertificateChain(socket->peerCertificateChain());
    info->exec();
    info->deleteLater();
}

void SslClient::appendString(const QString &line)
{
    QTextCursor cursor(form->sessionOutput->textCursor());
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(line);
    form->sessionOutput->verticalScrollBar()->setValue(form->sessionOutput->verticalScrollBar()->maximum());
}
