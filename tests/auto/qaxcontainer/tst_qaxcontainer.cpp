/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#if defined(Q_OS_WIN) && !defined(NO_DUMPCPP)

#include <qapplication.h>
#include <quuid.h>

#include <qaxwidget.h>
#include <qaxobject.h>

#include "dmocx.h"

#undef max
#undef min

#ifdef TEST_WMP
#include "wmp.h"
#endif

#ifdef TEST_EXCEL
#include "excel.h"
#endif

#include <windows.h> // IUnknown

//TESTED_CLASS=QAxBase QAxWidget QAxObject
//TESTED_FILES=extensions/activeqt/container/qaxbase.cpp extensions/activeqt/container/qaxwidget.cpp extensions/activeqt/container/qaxobject.cpp extensions/activeqt/container/qaxtypes.cpp

class tst_QAxContainer : public QObject
{
    Q_OBJECT
public:
    tst_QAxContainer();
    virtual ~tst_QAxContainer();

public slots:
    void init();
    void cleanup();


protected slots:
    void outShortSignal_slot(int &p);

private slots:
    void casting();
    void metaObject();

    void setControl();
    void clear();
    void multiParameterProperties();
    void IDispatchOut();
    void floatSupport();
    void noMetaObject();
    void outWithoutOut();
    void outOnlyOut();
    void enumProperty();
    void outShortSignal();

    void dumpcppExcel();

private:
    bool outShortSignal_called;
};

tst_QAxContainer::tst_QAxContainer()
{
    outShortSignal_called = false;
}

tst_QAxContainer::~tst_QAxContainer()
{

}

void tst_QAxContainer::init()
{
// This will be executed immediately before each test is run.
}

void tst_QAxContainer::cleanup()
{
// This will be executed immediately after each test is run.
}


void tst_QAxContainer::casting()
{
    QObject *widget = new QAxWidget();
    QVERIFY(qobject_cast<QObject*>(widget));
    QVERIFY(qobject_cast<QWidget*>(widget));
    QVERIFY(qobject_cast<QAxBase*>(widget));
    QVERIFY(qobject_cast<QAxWidget*>(widget));
    QVERIFY(!qobject_cast<QAxObject*>(widget));
    delete widget;

    QObject *object = new QAxObject();
    QVERIFY(qobject_cast<QObject*>(object));
    QVERIFY(qobject_cast<QAxBase*>(object));
    QVERIFY(qobject_cast<QAxObject*>(object));
    QVERIFY(!qobject_cast<QAxWidget*>(object));
    delete object;

    QObject *tv = new CTVLib::TV();
    QVERIFY(qobject_cast<CTVLib::TV*>(tv));
    QVERIFY(tv->qt_metacast("CTVLib::TV"));
    QVERIFY(qobject_cast<QObject*>(tv));
    QVERIFY(tv->qt_metacast("QObject"));
    QVERIFY(qobject_cast<QWidget*>(tv));
    QVERIFY(tv->qt_metacast("QWidget"));
    QVERIFY(qobject_cast<QAxBase*>(tv));
    QVERIFY(tv->qt_metacast("QAxBase"));
    QVERIFY(qobject_cast<QAxWidget*>(tv));
    QVERIFY(tv->qt_metacast("QAxWidget"));
    QVERIFY(!qobject_cast<QAxObject*>(tv));
    QVERIFY(!tv->qt_metacast("QAxObject"));
    delete tv;
}

void tst_QAxContainer::metaObject()
{
#ifdef TEST_EXCEL
    const QMetaObject metaObject = Excel::WorksheetFunction::staticMetaObject;

    QCOMPARE(metaObject.className(), "Excel::WorksheetFunction");

    QCOMPARE(metaObject.methodCount(), 1058 + metaObject.methodOffset());

    QMetaMethod method;
    for (int i = 0; i < metaObject.methodCount(); ++i) {
        method = metaObject.method(i);
        QVERIFY(!QByteArray(method.signature()).isEmpty());
    }

    method = metaObject.method(metaObject.methodOffset());
    QCOMPARE(method.signature(), "exception(int,QString,QString,QString)");
    method = metaObject.method(metaObject.methodOffset() + 3);
    QCOMPARE(method.signature(), "Acos(double)");

    QCOMPARE(metaObject.propertyCount(), 4 + metaObject.propertyOffset());
    QMetaProperty metaProp;
    for (int i = 0; i < metaObject.propertyCount(); ++i) {
        metaProp = metaObject.property(i);
        QVERIFY(!QByteArray(metaProp.name()).isEmpty());
        QVERIFY(!QByteArray(metaProp.typeName()).isEmpty());
    }
    
    metaProp = metaObject.property(metaObject.propertyOffset());
    QCOMPARE(metaProp.name(), "Application");
    QCOMPARE(metaProp.typeName(), "Application*");
   
    QCOMPARE(metaObject.enumeratorCount(), 186 + metaObject.enumeratorOffset());
    QMetaEnum metaEnum;
    for (int i = 0; i < metaObject.enumeratorCount(); ++i) {
        metaEnum = metaObject.enumerator(i);
        QVERIFY(!QByteArray(metaEnum.name()).isEmpty());
    }

    metaEnum = metaObject.enumerator(metaObject.enumeratorOffset());
    QCOMPARE(metaEnum.name(), "Constants");
#endif
}

class SignalSpy : public QObject
{
    Q_OBJECT
public:
    SignalSpy()
    {
        reset();
    }

    void reset()
    {
        NewWindow2Called = false;
        NavigateComplete2Called = false;
        NavigateCompleteCalled = false;
    }

    QString lastSignal()
    {
        QString ls = last_signal;
        last_signal = QString();
        return ls;
    }

    bool NewWindow2Called;
    bool NavigateComplete2Called;
    bool NavigateCompleteCalled;

public slots:
    void signal(const QString &s)
    {
        last_signal = s;
    }

    void propertyChanged(const QString &s)
    {
        last_signal = s + "Changed";
    }

    void NavigateComplete(const QString &url)
    {
        NavigateCompleteCalled = true;
    }

    void NavigateComplete2(IDispatch *disp, const QVariant &var)
    {
        Q_ASSERT(disp);
        NavigateComplete2Called = true;
    }

    void NewWindow2(IDispatch **pdisp, bool &cancel)
    {
        Q_ASSERT(pdisp);
        Q_ASSERT(!*pdisp);
        cancel = true;
        NewWindow2Called = true;
    }
private:
    QString last_signal;
};

void tst_QAxContainer::setControl()
{
    {
        QAxObject ie;
        ie.setProperty("control", "{8856F961-340A-11D0-A96B-00C04FD705A2}");
        QVERIFY(!ie.isNull());
    }

    QAxObject control;

    control.setControl("{ff5b1dca-8ffa-4317-85fb-63ed8407d024}");
    if (control.isNull()) {
        QSKIP("testcontrol40.QTestControl not available", SkipAll);
    }

    IUnknown *firstUnknown = 0;
    IUnknown *otherUnknown = 0;
    bool equal = false;

    control.queryInterface(IID_IUnknown, (void**)&firstUnknown);
    QVERIFY(firstUnknown);

    control.clear();
    control.setControl("{ff5b1dca-8ffa-4317-85fb-63ed8407d024}");
    firstUnknown->Release();

    control.queryInterface(IID_IUnknown, (void**)&otherUnknown);
    QVERIFY(firstUnknown != otherUnknown);
    firstUnknown = otherUnknown;

    control.setControl("{FF5B1DCA-8FFA-4317-85FB-63ED8407D024}");
    otherUnknown->Release();

    control.queryInterface(IID_IUnknown, (void**)&otherUnknown);
    QVERIFY(firstUnknown == otherUnknown);

    control.setControl("testcontrol40.QTestControl");
    otherUnknown->Release();
    control.queryInterface(IID_IUnknown, (void**)&otherUnknown);
    QVERIFY(firstUnknown == otherUnknown);

    otherUnknown->Release();
}

void tst_QAxContainer::clear()
{
    QAxObject control;
    SignalSpy spy;
    control.setControl("{ff5b1dca-8ffa-4317-85fb-63ed8407d024}");

    if (control.isNull()) {
        QSKIP("testcontrol40.QTestControl not available", SkipAll);
    }

    QObject::connect(&control, SIGNAL(signal(QString, int, void*)), &spy, SLOT(signal(QString)));
    QObject::connect(&control, SIGNAL(propertyChanged(QString)), &spy, SLOT(propertyChanged(QString)));

    control.dynamicCall("setUnicode(\"foo\")");
    QCOMPARE(spy.lastSignal(), QString("unicodeChanged"));
    QCOMPARE(spy.lastSignal(), QString());

    control.clear();

    control.setControl("{ff5b1dca-8ffa-4317-85fb-63ed8407d024}");
    control.dynamicCall("setUnicode(\"bar\")");
    QCOMPARE(spy.lastSignal(), QString("unicodeChanged"));
    QCOMPARE(spy.lastSignal(), QString());
}

void tst_QAxContainer::multiParameterProperties()
{
    QAxWidget mc("{4C9FB737-A83C-4A3B-8C1D-EF4FC180F300}");
    if (mc.isNull())
        QSKIP("Control {4C9FB737-A83C-4A3B-8C1D-EF4FC180F300} not registered", SkipAll);

    const QMetaObject *mo = mc.metaObject();
    QCOMPARE(mo->indexOfProperty("Accel"), -1);

    QVERIFY(mo->indexOfMethod("Accel(int)") != -1);
    QVERIFY(mo->indexOfMethod("SetAccel(int,double)") != -1);
#if 0
    double accel0 = mc.dynamicCall("Accel(int)", 0).toDouble();
    mc.dynamicCall("SetAccel(int, double)", 0, accel0);
#endif
}

void tst_QAxContainer::IDispatchOut()
{
    Q_ASSERT(qApp);

    QAxWidget ie("{8856F961-340A-11D0-A96B-00C04FD705A2}");
    if (ie.isNull())
        QSKIP("InternetExplorer is not installed", SkipAll);

    SignalSpy sink;
    QVERIFY(QObject::connect(&ie, SIGNAL(NavigateComplete(const QString&)), 
        &sink, SLOT(NavigateComplete(const QString&))));
    QVERIFY(QObject::connect(&ie, SIGNAL(NavigateComplete2(IDispatch*, const QVariant&)), 
        &sink, SLOT(NavigateComplete2(IDispatch*, const QVariant&))));
    QVERIFY(QObject::connect(&ie, SIGNAL(NewWindow2(IDispatch**, bool&)), 
        &sink, SLOT(NewWindow2(IDispatch**, bool&))));

    ie.dynamicCall("GoHome()");
    while (!sink.NavigateCompleteCalled)
        qApp->processEvents();

    QVERIFY(sink.NavigateComplete2Called);
    QVERIFY(!sink.NewWindow2Called);
    sink.reset();
    ie.dynamicCall("Navigate2(const QString&, const QVariant&, const QVariant&)", "www.trolltech.com", 0, "New Window");
    ie.dynamicCall("Navigate2(const QString&)", "www.trolltech.com");
    while (!sink.NavigateCompleteCalled)
        qApp->processEvents();

    QVERIFY(sink.NavigateComplete2Called);
    QVERIFY(sink.NewWindow2Called);
}


void tst_QAxContainer::floatSupport()
{
    QAxWidget kodak("{6D940280-9F11-11CE-83FD-02608C3EC08A}");
    if (kodak.isNull())
        QSKIP("Kodak Image Control not installed", SkipAll);

    QVariant zoom = kodak.property("Zoom");
    QVERIFY(!zoom.isNull());
    QCOMPARE(zoom.toDouble(), 100.);

    QVERIFY(kodak.setProperty("Zoom", 50.));
    QCOMPARE(kodak.property("Zoom"), QVariant(50.));

    kodak.dynamicCall("setZoom(double)", 75.);
    QCOMPARE(kodak.property("Zoom"), QVariant(75.));

    kodak.dynamicCall("setZoom(100)");
    QCOMPARE(kodak.property("Zoom"), QVariant(100.));
}

void tst_QAxContainer::noMetaObject()
{
    QAxWidget kodak("{6D940280-9F11-11CE-83FD-02608C3EC08A}");
    if (kodak.isNull())
        QSKIP("Kodak Image Control not installed", SkipAll);

    kodak.disableMetaObject();
    kodak.dynamicCall("setZoom(double)", 75.);
    QCOMPARE(kodak.dynamicCall("Zoom"), QVariant(75.));

    kodak.dynamicCall("setZoom(50)");
    QCOMPARE(kodak.dynamicCall("Zoom"), QVariant(50.0));

    kodak.dynamicCall("Zoom", 100.0);
    QCOMPARE(kodak.dynamicCall("Zoom"), QVariant(100.0));

    kodak.dynamicCall("GetVersion()");
}

void tst_QAxContainer::outWithoutOut()
{
    // qDebug("Testing regressions of task #46389");
    QAxWidget homatic("{EC784803-6E51-11D2-8979-00A0244FC9C0}");
    if (homatic.isNull())
        QSKIP("{EC784803-6E51-11D2-8979-00A0244FC9C0} not installed", SkipAll);

    int lFlags = 0;
    int plOutVal = 0;
    QVariantList params;
    params << lFlags;
    params << plOutVal;

    QVariant res = homatic.dynamicCall("GetLongCaps(int, int&)", params);
    QVERIFY(res.type() != QVariant::Invalid);
}

void tst_QAxContainer::outOnlyOut()
{
    QAxObject task79423("{485C6972-80E5-4405-9C95-3E008F9557C5}");
    if (task79423.isNull())
        QSKIP("COM object not installed; check mails in 78130", SkipAll);

    QVariantList params;
    params << -1;

    task79423.dynamicCall("GetOutParam(int&)", params);
    QCOMPARE(params.at(0).toInt(), 123);

    task79423.dynamicCall("GetInOutParam(int&)", params);
    QCOMPARE(params.at(0).toInt(), 456);

    QVariant ret = task79423.dynamicCall("GetRetVal()");
    QCOMPARE(ret.toInt(), 789);
}

void tst_QAxContainer::enumProperty()
{
    // qDebug("Testing of advanced properties using DBGrid Control");
    QAxWidget dbgrid("{00028C00-0000-0000-0000-000000000046}");

    if (dbgrid.isNull())
        QSKIP("DBGrid not installed", SkipAll);

    QVERIFY(dbgrid.metaObject()->indexOfProperty("Columns") != -1);
    QVariant leftCol = dbgrid.property("LeftCol");
    QCOMPARE(leftCol, QVariant(0));

    QCOMPARE(dbgrid.property("BorderStyle"), QVariant(1));

    QVERIFY(dbgrid.setProperty("BorderStyle", 0));
    QCOMPARE(dbgrid.property("BorderStyle"), QVariant(0));

    QVERIFY(dbgrid.setProperty("BorderStyle", "_FixedSingle"));
    QCOMPARE(dbgrid.property("BorderStyle"), QVariant(1));

    QVERIFY(dbgrid.setProperty("BorderStyle", "_None"));
    QCOMPARE(dbgrid.property("BorderStyle"), QVariant(0));

    dbgrid.dynamicCall("BorderStyle", "_FixedSingle");
    QCOMPARE(dbgrid.property("BorderStyle"), QVariant(1));
    QCOMPARE(dbgrid.dynamicCall("BorderStyle"), QVariant(1));

    dbgrid.dynamicCall("SetBorderStyle(enumBorderStyleConstants)", "_None");
    QCOMPARE(dbgrid.property("BorderStyle"), QVariant(0));
    QCOMPARE(dbgrid.dynamicCall("BorderStyle"), QVariant(0));

    QAxObject *columns = dbgrid.querySubObject("Columns()");
    QVERIFY(columns);

    QCOMPARE(columns->property("Count"), QVariant(2));
    columns->dynamicCall("Add(int)", 2);
    QCOMPARE(columns->property("Count"), QVariant(3));

    delete columns;
}

void tst_QAxContainer::outShortSignal_slot(int &)
{
    outShortSignal_called = true;
}

void tst_QAxContainer::outShortSignal()
{
    QAxWidget mscal("MSCAL.Calendar");
    if (mscal.isNull())
        QSKIP("MSCAL.Calendar not installed", SkipAll);

    connect(&mscal, SIGNAL(BeforeUpdate(int&)), this, SLOT(outShortSignal_slot(int&)));
    mscal.show();
    outShortSignal_called = false;
    mscal.dynamicCall("NextDay()");
    QVERIFY(outShortSignal_called);
}

void tst_QAxContainer::dumpcppExcel()
{
#ifdef TEST_EXCEL
    Excel::Application* xlApp= new Excel::Application();
    QVERIFY(xlApp);
    Excel::Workbooks* wbs=xlApp->Workbooks();
    QVERIFY(wbs);
    Excel::Workbook *book = wbs->Add();
    QVERIFY(book);
    Excel::Range *range = xlApp->Range("A4");
    QVERIFY(range);

    xlApp->Quit();
    delete range;
    delete book;
    delete wbs;
    delete xlApp;
#else
    QSKIP("Excel not installed", SkipAll);
#endif
}

QTEST_MAIN(tst_QAxContainer)
#include "tst_qaxcontainer.moc"

#else // non-windows systems
QTEST_NOOP_MAIN
#endif
