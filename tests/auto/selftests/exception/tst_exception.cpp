/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

class tst_Exception: public QObject
{
    Q_OBJECT

private slots:
    void throwException() const;
};

/*!
 \internal

 We simply throw an exception to check that we get sane output/reporting.
 */
void tst_Exception::throwException() const
{
    /* When exceptions are disabled, some compilers, at least linux-g++, treat
     * exception clauses as hard errors. */
#ifndef QT_NO_EXCEPTIONS
    throw 3;
#endif
}

QTEST_MAIN(tst_Exception)

#include "tst_exception.moc"
