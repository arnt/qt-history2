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

#include "statistics.h"

#include <qvariant.h>
#include <qimage.h>
#include <qpixmap.h>

Statistics::Statistics(QWidget* parent, Qt::WFlags fl)
: QDialog(parent, fl)
{
    setupUi(this);
    connect(closeBtn, SIGNAL(clicked()), this, SLOT(close()));
}

void Statistics::languageChange()
{
    retranslateUi(this);
}

void Statistics::updateStats(int sW,int sC,int sCS,int trW,int trC,int trCS)
{
    untrWords->setText(QString::number(sW));
    untrChars->setText(QString::number(sC)); 
    untrCharsSpc->setText(QString::number(sCS));
    trWords->setText(QString::number(trW));
    trChars->setText(QString::number(trC));
    trCharsSpc->setText(QString::number(trCS));
}
