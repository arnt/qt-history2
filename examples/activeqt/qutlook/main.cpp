/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_COMMERCIAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "addressview.h"
#include <QApplication>

int main(int argc, char ** argv)
{
    QApplication a(argc, argv);

    AddressView view;
    view.setWindowTitle("Qt Example - Looking at Outlook");
    view.show();

    return a.exec();
}
