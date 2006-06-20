/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <qdebug.h>
#include <qmetatype.h>

#include <qcoreapplication.h>
#include <private/qobject_p.h>
#include <qvarlengtharray.h>
#include <qrect.h>

Q_DECLARE_METATYPE(QRect)
Q_DECLARE_METATYPE(quint64)
Q_DECLARE_METATYPE(qint64)

class tst_QMetaCall: public QObject
{
Q_OBJECT

private slots:
    void invokeMetaMember();
    void invokeNonVoidMetaMember();
    void invokeMetaMemberQueued();
};



struct QVoidArgument
{
#ifdef Q_CC_XLC
    template<class T>
    inline operator T() const { return T(); }
#endif
};

template<> struct QMetaTypeId<QVoidArgument>
{ static inline int qt_metatype_id() { return QMetaType::Void; } };

static QByteArray qt_signature(int t0, int t1, int t2, int t3, int t4, int t5, int t6, int t7,
                               int t8, int t9)
{
    int types[] = { t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, QMetaType::Void };
    QVarLengthArray<char, 512> sig;

    sig.append('(');
    int i = 0;
    while (types[i] != QMetaType::Void) {
        const char *typeName = QMetaType::typeName(types[i]);
        sig.append(typeName, qstrlen(typeName));
        sig.append(',');
        ++i;
    }
    if (i == 0)
        sig.append(')');
    else
        sig[sig.size() - 1] = ')';
    sig.append('\0');

    return QMetaObject::normalizedSignature(sig.constData());
}

static int qt_metacall_index(QObject *obj, const char *member, const QByteArray &params)
{
    QVarLengthArray<char, 512> sig;
    int len = qstrlen(member);
    if (len <= 0)
        return false;
    sig.append(member, len);
    sig.append(params.constData(), params.length() + 1); // trailing 0

    return obj->metaObject()->indexOfMethod(sig.constData());
}

static bool qt_metacall_helper(QObject *obj, const char *member, const QByteArray &params,
                               QVariant *retValue, const void **args)
{
    int idx = qt_metacall_index(obj, member, params);
    if (idx < 0)
         return false;

    // return type
    if (retValue) {
        int tp = QMetaType::type(obj->metaObject()->method(idx).typeName());
        if (tp == 0)
            return false;
        *retValue = QVariant(QMetaType::type(obj->metaObject()->method(idx).typeName()), (void*)0);
        args[0] = retValue->data();
    }

    return obj->qt_metacall(QMetaObject::InvokeMetaMethod, idx, const_cast<void **>(args)) < 0;
}

static bool qt_metacall_queued_helper(QObject *obj, const char *member, const QByteArray &params,
                                      const int *types, const void **args)
{
    enum { ParamCount = 10 };

    int idx = qt_metacall_index(obj, member, params);
    if (idx < 0)
        return false;

    void **pa = static_cast<void **>(qMalloc(ParamCount * sizeof(void *)));
    int *pt = static_cast<int *>(qMalloc(ParamCount * sizeof(int)));
    pa[0] = 0; // return type
    pt[0] = 0;

    int i;
    for (i = 1; i < ParamCount; ++i) {
        if (types[i] == QMetaType::Void)
            break;
        pt[i] = types[i];
        pa[i] = QMetaType::construct(pt[i], args[i]);
    }
    QCoreApplication::postEvent(obj, new QMetaCallEvent(idx, 0, i, pt, pa));
    return true;
}

template<typename T0 = QVoidArgument, typename T1 = QVoidArgument, typename T2 = QVoidArgument,
         typename T3 = QVoidArgument, typename T4 = QVoidArgument, typename T5 = QVoidArgument,
         typename T6 = QVoidArgument, typename T7 = QVoidArgument, typename T8 = QVoidArgument,
         typename T9 = QVoidArgument>
class QMetaCall
{
public:
    static bool invokeMember(QObject *obj, const char *member,
            const T0 &t0 = QVoidArgument(), const T1 &t1 = QVoidArgument(),
            const T2 &t2 = QVoidArgument(), const T3 &t3 = QVoidArgument(),
            const T4 &t4 = QVoidArgument(), const T5 &t5 = QVoidArgument(),
            const T6 &t6 = QVoidArgument(), const T7 &t7 = QVoidArgument(),
            const T8 &t8 = QVoidArgument(), const T9 &t9 = QVoidArgument())
    {
        const void *args[] = { 0, &t0, &t1, &t2, &t3, &t4, &t5, &t6, &t7, &t8, &t9 };
        return qt_metacall_helper(obj, member, params(), 0, args);
    }

    static bool invokeNonVoidMember(QObject *obj, const char *member, QVariant &returnValue,
            const T0 &t0 = QVoidArgument(), const T1 &t1 = QVoidArgument(),
            const T2 &t2 = QVoidArgument(), const T3 &t3 = QVoidArgument(),
            const T4 &t4 = QVoidArgument(), const T5 &t5 = QVoidArgument(),
            const T6 &t6 = QVoidArgument(), const T7 &t7 = QVoidArgument(),
            const T8 &t8 = QVoidArgument(), const T9 &t9 = QVoidArgument())
    {
        const void *args[] = { 0, &t0, &t1, &t2, &t3, &t4, &t5, &t6, &t7, &t8, &t9 };
        return qt_metacall_helper(obj, member, params(), &returnValue, args);
    }

    static bool invokeMemberQueued(QObject *obj, const char *member,
            const T0 &t0 = QVoidArgument(), const T1 &t1 = QVoidArgument(),
            const T2 &t2 = QVoidArgument(), const T3 &t3 = QVoidArgument(),
            const T4 &t4 = QVoidArgument(), const T5 &t5 = QVoidArgument(),
            const T6 &t6 = QVoidArgument(), const T7 &t7 = QVoidArgument(),
            const T8 &t8 = QVoidArgument(), const T9 &t9 = QVoidArgument())
    {
        const void *args[] = { 0, &t0, &t1, &t2, &t3, &t4, &t5, &t6, &t7, &t8, &t9 };
        const int types[] = { QMetaType::Void,
                              QMetaTypeId<T0>::qt_metatype_id(), QMetaTypeId<T1>::qt_metatype_id(),
                              QMetaTypeId<T2>::qt_metatype_id(), QMetaTypeId<T3>::qt_metatype_id(),
                              QMetaTypeId<T4>::qt_metatype_id(), QMetaTypeId<T5>::qt_metatype_id(),
                              QMetaTypeId<T6>::qt_metatype_id(), QMetaTypeId<T7>::qt_metatype_id(),
                              QMetaTypeId<T8>::qt_metatype_id(), QMetaTypeId<T9>::qt_metatype_id(),
                              QMetaType::Void };
        return qt_metacall_queued_helper(obj, member, params(), types, args);
    }

private:
    static const QByteArray &params()
    {
        static const QByteArray ba = qt_signature(QMetaTypeId<T0>::qt_metatype_id(),
                QMetaTypeId<T1>::qt_metatype_id(), QMetaTypeId<T2>::qt_metatype_id(),
                QMetaTypeId<T3>::qt_metatype_id(), QMetaTypeId<T4>::qt_metatype_id(),
                QMetaTypeId<T5>::qt_metatype_id(), QMetaTypeId<T6>::qt_metatype_id(),
                QMetaTypeId<T7>::qt_metatype_id(), QMetaTypeId<T8>::qt_metatype_id(),
                QMetaTypeId<T9>::qt_metatype_id());
        return ba;
    }
};

class InvokeTester: public QObject
{
    Q_OBJECT
public slots:
    void foo1() { lastInvoked = "foo1"; }
    void foo2(int i) { lastInvoked = "foo2 " + QString::number(i); }
    void foo3(QString s, short i) { lastInvoked = "foo3 " + s + QString::number(i); }
    void foo4(QString s, short i) { lastInvoked = "foo4 " + s + QString::number(i); }
    void foo9(const QString &s1, int i, double d, long l, float f, QRect r,
              qint64 ll, quint64 ull, uint ui, ulong ul)
    {
        lastInvoked = QString("foo9 %1 %2 %3 %4 %5 %6 %7 %8 %9 ").arg(s1).arg(i).arg(d).arg(l).arg(f).arg(r.height()).arg(ll).arg(ull).arg(ui);
        lastInvoked += QString::number(ul);
    }

    void foo5(QString &s) { lastInvoked = "foo5" + s; }

    int bar1() { return 42; }
    QString bar2(const QString &s) { return s.toUpper(); }

public:
    QString lastInvoked;
};

void tst_QMetaCall::invokeMetaMember()
{
    InvokeTester obj;

    QVERIFY( (QMetaCall<>::invokeMember(&obj, "foo1")) );
    QCOMPARE(obj.lastInvoked, QString("foo1"));
    QVERIFY(QMetaCall<int>::invokeMember(&obj, "foo2", 42));
    QCOMPARE(obj.lastInvoked, QString("foo2 42"));
    QVERIFY( (QMetaCall<QString, short>::invokeMember(&obj, "foo3", "foo", 42)) );
    QCOMPARE(obj.lastInvoked, QString("foo3 foo42"));
    QVERIFY( (QMetaCall<QString, short>::invokeMember(&obj, "foo4", "foo", 42)) );
    QCOMPARE(obj.lastInvoked, QString("foo4 foo42"));
    QVERIFY( (QMetaCall<QString, int, double, long, float, QRect, qint64, quint64, uint, ulong>::invokeMember(&obj, "foo9", "blah", 1, 2.2, 3, 4.4, QRect(1, 2, 3, 4), 5, 6, 7, 8)));
    QCOMPARE(obj.lastInvoked, QString("foo9 blah 1 2.2 3 4.4 4 5 6 7 8"));

    QVERIFY( !(QMetaCall<QString>::invokeMember(&obj, "nada", "nono")) );
}

void tst_QMetaCall::invokeNonVoidMetaMember()
{
    InvokeTester obj;

    QVariant v = 43;

    QVERIFY((QMetaCall<>::invokeNonVoidMember(&obj, "bar1", v)));
    QCOMPARE(v.toInt(), 42);

    QVERIFY((QMetaCall<QString>::invokeNonVoidMember(&obj, "bar2", v, QString("hello"))));
    QCOMPARE(v.toString(), QString("HELLO"));
}

void tst_QMetaCall::invokeMetaMemberQueued()
{
    InvokeTester obj;

    QVERIFY((QMetaCall<>::invokeMemberQueued(&obj, "foo1")));
    QCOMPARE(obj.lastInvoked, QString());
    QCoreApplication::instance()->processEvents(QEventLoop::AllEvents);
    QCOMPARE(obj.lastInvoked, QString("foo1"));

    obj.lastInvoked.clear();

    QVERIFY((QMetaCall<QString, short>::invokeMemberQueued(&obj, "foo3", QString("foo"), 42)));
    QCOMPARE(obj.lastInvoked, QString());
    QCoreApplication::instance()->processEvents(QEventLoop::AllEvents);
    QCOMPARE(obj.lastInvoked, QString("foo3 foo42"));
}

QTEST_MAIN(tst_QMetaCall)
#include "tst_qmetacall.moc"
