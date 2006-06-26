/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "about.h"

#include <QPixmap>
#include <QStyle>

AboutDialog::AboutDialog(QWidget* parent, Qt::WindowFlags fl)
    : QDialog(parent, fl)
{
    setupUi(this);
    
    PixmapLabel1->setPixmap(QPixmap(":/images/splash.png"));
    Qt::TextInteractionFlags f(style()->styleHint(QStyle::SH_MessageBox_TextInteractionFlags));
    versionLabel->setTextInteractionFlags(f);
    infoText->setTextInteractionFlags(f);
    connect(PushButton1, SIGNAL(clicked()), this, SLOT(accept()));
}

void AboutDialog::languageChange()
{
    retranslateUi(this);
}
