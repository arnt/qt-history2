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

#ifndef CERTIFICATEINFO_H
#define CERTIFICATEINFO_H

#include <QtGui/QDialog>
#include <QtNetwork/QSslCertificate>

class Ui_CertificateInfo;

class CertificateInfo : public QDialog
{
    Q_OBJECT
public:
    CertificateInfo(QWidget *parent = 0);
    ~CertificateInfo();

    void setCertificateChain(const QList<QSslCertificate> &chain);

private slots:
    void updateCertificateInfo(int index);

private:
    Ui_CertificateInfo *form;
    QList<QSslCertificate> chain;
};

#endif
