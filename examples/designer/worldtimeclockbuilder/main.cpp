/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtForm>
#include <QtGui>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QForm::Loader loader;

    QFile file(":/forms/form.ui");
    if (file.open(QFile::ReadOnly)) {
        QWidget *widget = loader.load(&file, 0);
        Q_ASSERT(widget != 0);
        widget->show();
    }

    return app.exec();
}
