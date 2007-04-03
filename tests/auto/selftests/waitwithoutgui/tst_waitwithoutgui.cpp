/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QTest>
#include <QTextStream>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    QTextStream out(stdout);

    out << "We just output something such that there's a baseline to compare against." << endl;

    /* Simply call qWait(). */
    QTest::qWait(100);

    out << "Finished waiting!" << endl;

    return 0;
}

