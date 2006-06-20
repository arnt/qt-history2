/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QApplication>
#include <QComboBox>

int main(int argc, char **argv) {
    QApplication::setDesktopSettingsAware(false);
    QApplication app(argc, argv);
    QComboBox box;
    box.insertItem(0, "foo");
    box.setEditable(true);
    box.show();
    return 0;
}
