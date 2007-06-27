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
#include "ui_certificateinfo.h"

CertificateInfo::CertificateInfo(QWidget *parent)
    : QDialog(parent)
{
    form = new Ui_CertificateInfo;
    form->setupUi(this);

    connect(form->certificationPathView, SIGNAL(currentRowChanged(int)),
            this, SLOT(updateCertificateInfo(int)));
}

CertificateInfo::~CertificateInfo()
{
    delete form;
}

void CertificateInfo::setCertificateChain(const QList<QSslCertificate> &chain)
{
    this->chain = chain;

    form->certificationPathView->clear();

    for (int i = 0; i < chain.size(); ++i) {
        const QSslCertificate &cert = chain.at(i);
        form->certificationPathView->addItem(tr("%1%2 (%3)").arg(!i ? QString() : tr("Issued by: "))
                                             .arg(cert.subjectInfo(QSslCertificate::Organization))
                                             .arg(cert.subjectInfo(QSslCertificate::CommonName)));
    }

    form->certificationPathView->setCurrentRow(0);
}

void CertificateInfo::updateCertificateInfo(int index)
{
    form->certificateInfoView->clear();
    if (index >= 0 && index < chain.size()) {
        const QSslCertificate &cert = chain.at(index);
        QStringList lines;
        lines << tr("Organization: %1").arg(cert.subjectInfo(QSslCertificate::Organization))
              << tr("Subunit: %1").arg(cert.subjectInfo(QSslCertificate::OrganizationalUnitName))
              << tr("Country: %1").arg(cert.subjectInfo(QSslCertificate::CountryName))
              << tr("Locality: %1").arg(cert.subjectInfo(QSslCertificate::LocalityName))
              << tr("State/Province: %1").arg(cert.subjectInfo(QSslCertificate::StateOrProvinceName))
              << tr("Common Name: %1").arg(cert.subjectInfo(QSslCertificate::CommonName))
              << QString()
              << tr("Issuer Organization: %1").arg(cert.issuerInfo(QSslCertificate::Organization))
              << tr("Issuer Unit Name: %1").arg(cert.issuerInfo(QSslCertificate::OrganizationalUnitName))
              << tr("Issuer Country: %1").arg(cert.issuerInfo(QSslCertificate::CountryName))
              << tr("Issuer Locality: %1").arg(cert.issuerInfo(QSslCertificate::LocalityName))
              << tr("Issuer State/Province: %1").arg(cert.issuerInfo(QSslCertificate::StateOrProvinceName))
              << tr("Issuer Common Name: %1").arg(cert.issuerInfo(QSslCertificate::CommonName));
        foreach (QString line, lines)
            form->certificateInfoView->addItem(line);
    } else {
        form->certificateInfoView->clear();
    }
}
