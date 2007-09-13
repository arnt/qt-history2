/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <qcoreapplication.h>
#include <qdebug.h>
#ifndef Q_WS_QWS
#include <qgl.h>
#endif

//TESTED_CLASS=
//TESTED_FILES=qgl.h

class tst_QGL : public QObject
{
Q_OBJECT

public:
    tst_QGL();
    virtual ~tst_QGL();

private slots:
    void getSetCheck();
    void openGLVersionCheck();
};

tst_QGL::tst_QGL()
{
}

tst_QGL::~tst_QGL()
{
}

#ifndef Q_WS_QWS
class MyGLContext : public QGLContext
{
public:
    MyGLContext(const QGLFormat& format) : QGLContext(format) {}
    bool windowCreated() const { return QGLContext::windowCreated(); }
    void setWindowCreated(bool on) { QGLContext::setWindowCreated(on); }
    bool initialized() const { return QGLContext::initialized(); }
    void setInitialized(bool on) { QGLContext::setInitialized(on); }
};

class MyGLWidget : public QGLWidget
{
public:
    MyGLWidget() : QGLWidget() {}
    bool autoBufferSwap() const { return QGLWidget::autoBufferSwap(); }
    void setAutoBufferSwap(bool on) { QGLWidget::setAutoBufferSwap(on); }
};
#endif
// Testing get/set functions
void tst_QGL::getSetCheck()
{
#ifdef Q_WS_QWS
    QSKIP("QGL not yet supported on QWS", SkipAll);
#else
    if (!QGLFormat::hasOpenGL())
        QSKIP("QGL not supported on this platform", SkipAll);

    QGLFormat obj1;
    // int QGLFormat::depthBufferSize()
    // void QGLFormat::setDepthBufferSize(int)
    obj1.setDepthBufferSize(0);
    QCOMPARE(0, obj1.depthBufferSize());
    obj1.setDepthBufferSize(INT_MIN);
    QCOMPARE(0, obj1.depthBufferSize()); // Makes no sense with a negative buffer size
    obj1.setDepthBufferSize(INT_MAX);
    QCOMPARE(INT_MAX, obj1.depthBufferSize());

    // int QGLFormat::accumBufferSize()
    // void QGLFormat::setAccumBufferSize(int)
    obj1.setAccumBufferSize(0);
    QCOMPARE(0, obj1.accumBufferSize());
    obj1.setAccumBufferSize(INT_MIN);
    QCOMPARE(0, obj1.accumBufferSize()); // Makes no sense with a negative buffer size
    obj1.setAccumBufferSize(INT_MAX);
    QCOMPARE(INT_MAX, obj1.accumBufferSize());

    // int QGLFormat::alphaBufferSize()
    // void QGLFormat::setAlphaBufferSize(int)
    obj1.setAlphaBufferSize(0);
    QCOMPARE(0, obj1.alphaBufferSize());
    obj1.setAlphaBufferSize(INT_MIN);
    QCOMPARE(0, obj1.alphaBufferSize()); // Makes no sense with a negative buffer size
    obj1.setAlphaBufferSize(INT_MAX);
    QCOMPARE(INT_MAX, obj1.alphaBufferSize());

    // int QGLFormat::stencilBufferSize()
    // void QGLFormat::setStencilBufferSize(int)
    obj1.setStencilBufferSize(0);
    QCOMPARE(0, obj1.stencilBufferSize());
    obj1.setStencilBufferSize(INT_MIN);
    QCOMPARE(0, obj1.stencilBufferSize()); // Makes no sense with a negative buffer size
    obj1.setStencilBufferSize(INT_MAX);
    QCOMPARE(INT_MAX, obj1.stencilBufferSize());

    // bool QGLFormat::sampleBuffers()
    // void QGLFormat::setSampleBuffers(bool)
    obj1.setSampleBuffers(false);
    QCOMPARE(false, obj1.sampleBuffers());
    obj1.setSampleBuffers(true);
    QCOMPARE(true, obj1.sampleBuffers());

    // int QGLFormat::samples()
    // void QGLFormat::setSamples(int)
    obj1.setSamples(0);
    QCOMPARE(0, obj1.samples());
    obj1.setSamples(INT_MIN);
    QCOMPARE(0, obj1.samples());  // Makes no sense with a negative sample size
    obj1.setSamples(INT_MAX);
    QCOMPARE(INT_MAX, obj1.samples());

    // bool QGLFormat::doubleBuffer()
    // void QGLFormat::setDoubleBuffer(bool)
    obj1.setDoubleBuffer(false);
    QCOMPARE(false, obj1.doubleBuffer());
    obj1.setDoubleBuffer(true);
    QCOMPARE(true, obj1.doubleBuffer());

    // bool QGLFormat::depth()
    // void QGLFormat::setDepth(bool)
    obj1.setDepth(false);
    QCOMPARE(false, obj1.depth());
    obj1.setDepth(true);
    QCOMPARE(true, obj1.depth());

    // bool QGLFormat::rgba()
    // void QGLFormat::setRgba(bool)
    obj1.setRgba(false);
    QCOMPARE(false, obj1.rgba());
    obj1.setRgba(true);
    QCOMPARE(true, obj1.rgba());

    // bool QGLFormat::alpha()
    // void QGLFormat::setAlpha(bool)
    obj1.setAlpha(false);
    QCOMPARE(false, obj1.alpha());
    obj1.setAlpha(true);
    QCOMPARE(true, obj1.alpha());

    // bool QGLFormat::accum()
    // void QGLFormat::setAccum(bool)
    obj1.setAccum(false);
    QCOMPARE(false, obj1.accum());
    obj1.setAccum(true);
    QCOMPARE(true, obj1.accum());

    // bool QGLFormat::stencil()
    // void QGLFormat::setStencil(bool)
    obj1.setStencil(false);
    QCOMPARE(false, obj1.stencil());
    obj1.setStencil(true);
    QCOMPARE(true, obj1.stencil());

    // bool QGLFormat::stereo()
    // void QGLFormat::setStereo(bool)
    obj1.setStereo(false);
    QCOMPARE(false, obj1.stereo());
    obj1.setStereo(true);
    QCOMPARE(true, obj1.stereo());

    // bool QGLFormat::directRendering()
    // void QGLFormat::setDirectRendering(bool)
    obj1.setDirectRendering(false);
    QCOMPARE(false, obj1.directRendering());
    obj1.setDirectRendering(true);
    QCOMPARE(true, obj1.directRendering());

    // int QGLFormat::plane()
    // void QGLFormat::setPlane(int)
    obj1.setPlane(0);
    QCOMPARE(0, obj1.plane());
    obj1.setPlane(INT_MIN);
    QCOMPARE(INT_MIN, obj1.plane());
    obj1.setPlane(INT_MAX);
    QCOMPARE(INT_MAX, obj1.plane());

    MyGLContext obj2(obj1);
    // bool QGLContext::windowCreated()
    // void QGLContext::setWindowCreated(bool)
    obj2.setWindowCreated(false);
    QCOMPARE(false, obj2.windowCreated());
    obj2.setWindowCreated(true);
    QCOMPARE(true, obj2.windowCreated());

    // bool QGLContext::initialized()
    // void QGLContext::setInitialized(bool)
    obj2.setInitialized(false);
    QCOMPARE(false, obj2.initialized());
    obj2.setInitialized(true);
    QCOMPARE(true, obj2.initialized());

    MyGLWidget obj3;
    // bool QGLWidget::autoBufferSwap()
    // void QGLWidget::setAutoBufferSwap(bool)
    obj3.setAutoBufferSwap(false);
    QCOMPARE(false, obj3.autoBufferSwap());
    obj3.setAutoBufferSwap(true);
    QCOMPARE(true, obj3.autoBufferSwap());
#endif
}

QT_BEGIN_NAMESPACE
extern QGLFormat::OpenGLVersionFlags qOpenGLVersionFlagsFromString(const QString &versionString);
QT_END_NAMESPACE

void tst_QGL::openGLVersionCheck()
{
#ifdef Q_WS_QWS
    QSKIP("QGL not yet supported on QWS", SkipAll);
#else
    if (!QGLFormat::hasOpenGL())
        QSKIP("QGL not supported on this platform", SkipAll);

    QString versionString;
    QGLFormat::OpenGLVersionFlags expectedFlag;
    QGLFormat::OpenGLVersionFlags versionFlag;

    versionString = "1.1 Irix 6.5";
    expectedFlag = QGLFormat::OpenGL_Version_1_1;
    versionFlag = qOpenGLVersionFlagsFromString(versionString);
    QCOMPARE(versionFlag, expectedFlag);

    versionString = "1.2 Microsoft";
    expectedFlag = QGLFormat::OpenGL_Version_1_2 | QGLFormat::OpenGL_Version_1_1;
    versionFlag = qOpenGLVersionFlagsFromString(versionString);
    QCOMPARE(versionFlag, expectedFlag);

    versionString = "1.2.1";
    expectedFlag = QGLFormat::OpenGL_Version_1_2 | QGLFormat::OpenGL_Version_1_1;
    versionFlag = qOpenGLVersionFlagsFromString(versionString);
    QCOMPARE(versionFlag, expectedFlag);

    versionString = "1.3 NVIDIA";
    expectedFlag = QGLFormat::OpenGL_Version_1_3 | QGLFormat::OpenGL_Version_1_2 | QGLFormat::OpenGL_Version_1_1;
    versionFlag = qOpenGLVersionFlagsFromString(versionString);
    QCOMPARE(versionFlag, expectedFlag);

    versionString = "1.4";
    expectedFlag = QGLFormat::OpenGL_Version_1_4 | QGLFormat::OpenGL_Version_1_3 | QGLFormat::OpenGL_Version_1_2 | QGLFormat::OpenGL_Version_1_1;
    versionFlag = qOpenGLVersionFlagsFromString(versionString);
    QCOMPARE(versionFlag, expectedFlag);

    versionString = "1.5 NVIDIA";
    expectedFlag = QGLFormat::OpenGL_Version_1_5 | QGLFormat::OpenGL_Version_1_4 | QGLFormat::OpenGL_Version_1_3 | QGLFormat::OpenGL_Version_1_2 | QGLFormat::OpenGL_Version_1_1;
    versionFlag = qOpenGLVersionFlagsFromString(versionString);
    QCOMPARE(versionFlag, expectedFlag);

    versionString = "2.0.2 NVIDIA 87.62";
    expectedFlag = QGLFormat::OpenGL_Version_2_0 | QGLFormat::OpenGL_Version_1_5 | QGLFormat::OpenGL_Version_1_4 | QGLFormat::OpenGL_Version_1_3 | QGLFormat::OpenGL_Version_1_2 | QGLFormat::OpenGL_Version_1_1;
    versionFlag = qOpenGLVersionFlagsFromString(versionString);
    QCOMPARE(versionFlag, expectedFlag);

    versionString = "2.1 NVIDIA";
    expectedFlag = QGLFormat::OpenGL_Version_2_1 | QGLFormat::OpenGL_Version_2_0 | QGLFormat::OpenGL_Version_1_5 | QGLFormat::OpenGL_Version_1_4 | QGLFormat::OpenGL_Version_1_3 | QGLFormat::OpenGL_Version_1_2 | QGLFormat::OpenGL_Version_1_1;
    versionFlag = qOpenGLVersionFlagsFromString(versionString);
    QCOMPARE(versionFlag, expectedFlag);

    versionString = "2.1";
    expectedFlag = QGLFormat::OpenGL_Version_2_1 | QGLFormat::OpenGL_Version_2_0 | QGLFormat::OpenGL_Version_1_5 | QGLFormat::OpenGL_Version_1_4 | QGLFormat::OpenGL_Version_1_3 | QGLFormat::OpenGL_Version_1_2 | QGLFormat::OpenGL_Version_1_1;
    versionFlag = qOpenGLVersionFlagsFromString(versionString);
    QCOMPARE(versionFlag, expectedFlag);

    versionString = "OpenGL ES-CM 1.0 ATI";
    expectedFlag = QGLFormat::OpenGL_ES_Common_Version_1_0 | QGLFormat::OpenGL_ES_CommonLite_Version_1_0;
    versionFlag = qOpenGLVersionFlagsFromString(versionString);
    QCOMPARE(versionFlag, expectedFlag);

    versionString = "OpenGL ES-CL 1.0 ATI";
    expectedFlag = QGLFormat::OpenGL_ES_CommonLite_Version_1_0;
    versionFlag = qOpenGLVersionFlagsFromString(versionString);
    QCOMPARE(versionFlag, expectedFlag);

    versionString = "OpenGL ES-CM 1.1 ATI";
    expectedFlag = QGLFormat::OpenGL_ES_Common_Version_1_1 | QGLFormat::OpenGL_ES_CommonLite_Version_1_1 | QGLFormat::OpenGL_ES_Common_Version_1_0 | QGLFormat::OpenGL_ES_CommonLite_Version_1_0;
    versionFlag = qOpenGLVersionFlagsFromString(versionString);
    QCOMPARE(versionFlag, expectedFlag);

    versionString = "OpenGL ES-CL 1.1 ATI";
    expectedFlag = QGLFormat::OpenGL_ES_CommonLite_Version_1_1 | QGLFormat::OpenGL_ES_CommonLite_Version_1_0;
    versionFlag = qOpenGLVersionFlagsFromString(versionString);
    QCOMPARE(versionFlag, expectedFlag);

    versionString = "OpenGL ES 2.0 ATI";
    expectedFlag = QGLFormat::OpenGL_ES_Version_2_0;
    versionFlag = qOpenGLVersionFlagsFromString(versionString);
    QCOMPARE(versionFlag, expectedFlag);

    QGLWidget glWidget;
    glWidget.show();
    glWidget.makeCurrent();

    // This is unfortunately the only test we can make on the actual openGLVersionFlags()
    // However, the complicated parts are in openGLVersionFlags(const QString &versionString)
    // tested above

    QVERIFY(QGLFormat::openGLVersionFlags() & QGLFormat::OpenGL_Version_1_1);
#endif
}

QTEST_MAIN(tst_QGL)
#include "tst_qgl.moc"
