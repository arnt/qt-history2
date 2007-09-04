/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <qapplication.h>
#include <qdebug.h>
#include <qsvgrenderer.h>


//TESTED_CLASS=
//TESTED_FILES=qsvgrenderer.h

class tst_QSvgRenderer : public QObject
{
Q_OBJECT

public:
    tst_QSvgRenderer();
    virtual ~tst_QSvgRenderer();

private slots:
    void getSetCheck();
    void inexistentUrl();
};

tst_QSvgRenderer::tst_QSvgRenderer()
{
}

tst_QSvgRenderer::~tst_QSvgRenderer()
{
}

// Testing get/set functions
void tst_QSvgRenderer::getSetCheck()
{
    QSvgRenderer obj1;
    // int QSvgRenderer::framesPerSecond()
    // void QSvgRenderer::setFramesPerSecond(int)
    obj1.setFramesPerSecond(20);
    QCOMPARE(20, obj1.framesPerSecond());
    obj1.setFramesPerSecond(0);
    QCOMPARE(0, obj1.framesPerSecond());
    obj1.setFramesPerSecond(INT_MIN);
    QCOMPARE(0, obj1.framesPerSecond()); // Can't have a negative framerate
    obj1.setFramesPerSecond(INT_MAX);
    QCOMPARE(INT_MAX, obj1.framesPerSecond());
}

void tst_QSvgRenderer::inexistentUrl()
{
    const char *src = "<svg><g><path d=\"\" style=\"stroke:url(#inexistent)\"/></g></svg>";

    QByteArray data(src);
    QSvgRenderer renderer(data);

    QVERIFY(renderer.isValid());
}

QTEST_MAIN(tst_QSvgRenderer)
#include "tst_qsvgrenderer.moc"
