/****************************************************************************
**
** Copyright (C) 2007-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGui>

#include "stylewindow.h"

StyleWindow::StyleWindow()
{    
    QPushButton *styledButton = new QPushButton(tr("Big Red Button"));

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(styledButton);

    QGroupBox *styleBox = new QGroupBox(tr("A simple style button"));
    styleBox->setLayout(layout);    

    QGridLayout *outerLayout = new QGridLayout;
    outerLayout->addWidget(styleBox, 0, 0);
    setLayout(outerLayout);

    setWindowTitle(tr("Style Plugin Example"));
}
