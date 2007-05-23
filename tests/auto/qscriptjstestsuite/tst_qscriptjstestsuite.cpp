/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <qscriptengine.h>
#include <qscriptcontext.h>

//TESTED_CLASS=
//TESTED_FILES=

static QString readFile(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QFile::ReadOnly))
        return QString();
    QTextStream stream(&file);
    return stream.readAll();
}

static void appendCString(QVector<char> *v, const char *s)
{
    char c;
    do {
        c = *(s++);
        *v << c;
    } while (c != '\0');
}

class tst_Suite : public QObject
{

public:
    tst_Suite();
    virtual ~tst_Suite();

    static QMetaObject staticMetaObject;
    virtual const QMetaObject *metaObject() const;
    virtual void *qt_metacast(const char *);
    virtual int qt_metacall(QMetaObject::Call, int, void **argv);

    QString testsDir() const
    {
        QString path = qgetenv("QSCRIPT_JS_TESTS_DIR");

        if (path.isEmpty()) {
            qWarning() << "";
            qWarning() << "*** environment variable QSCRIPT_JS_TESTS_DIR is empty!!";
            qWarning() << "";
        }

        return path;
    }

private:
    QStringList m_testFiles;
    typedef QPair<int, QString> FailureCase;
    QHash<QString, QList<FailureCase> > m_expectedFailures;
    int m_passedCount;
};

QMetaObject tst_Suite::staticMetaObject;

Q_GLOBAL_STATIC(QVector<uint>, qt_meta_data_tst_Suite)
Q_GLOBAL_STATIC(QVector<char>, qt_meta_stringdata_tst_Suite)

const QMetaObject *tst_Suite::metaObject() const
{
    return &staticMetaObject;
}

void *tst_Suite::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_tst_Suite()->constData()))
        return static_cast<void*>(const_cast<tst_Suite*>(this));
    return QObject::qt_metacast(_clname);
}

static QScriptValue qscript_void(QScriptContext *, QScriptEngine *eng)
{
    return eng->undefinedValue();
}

int tst_Suite::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {

        if (!(_id & 1)) {
            // data
            QString filename = m_testFiles.at(_id / 2);
            QString text = readFile(testsDir() + QDir::separator() + filename);
            QList<FailureCase> expectedFailureCases = m_expectedFailures.value(filename);

            QScriptEngine eng;
            eng.globalObject().setProperty("print", eng.newFunction(qscript_void));
            QScriptValue ret;
            ret = eng.evaluate(readFile(testsDir() + QDir::separator() + QLatin1String("shell.js")));
            if (ret.isError())
                qWarning("%s", qPrintable(ret.toString()));
            eng.globalObject().setProperty("test", eng.newFunction(qscript_void));
            (void)eng.evaluate(text);

            QScriptValue testcases = eng.globalObject().property("testcases");
            int count = testcases.property("length").toInt32();
            QTest::addColumn<QString>("expect");
            QTest::addColumn<QString>("actual");
            QTest::addColumn<bool>("success");
            QTest::addColumn<bool>("expectFail");
            QTest::addColumn<QString>("expectFailReason");
            if (count > 0) {
                for (int i = 0; i < count; ++i) {
                    QScriptValue kase = testcases.property(i);
                    Q_ASSERT(kase.isValid());
                    QString description = kase.property("description").toString();
                    QScriptValue expect = kase.property("expect");
                    QScriptValue actual = kase.property("actual");
                    bool equal;
                    if (actual.isNumber() && expect.isNumber()) {
                        qsreal n1 = actual.toNumber();
                        qsreal n2 = expect.toNumber();
                        if (qIsNaN(n1) && qIsNaN(n2))
                            equal = true;
                        else
                            equal = qFuzzyCompare(n1, n2);
                    }
                    else if (actual.isDate() && expect.isDate())
                        equal = (actual.toDateTime() == expect.toDateTime());
                    else
                        equal = (actual.toString() == expect.toString());

                    bool expectFail = false;
                    QString expectFailReason;
                    if (!expectedFailureCases.isEmpty()) {
                        QList<FailureCase>::const_iterator lit;
                        lit = qLowerBound(expectedFailureCases, FailureCase(i, ""));
                        if (lit != expectedFailureCases.constEnd()
                            && ((*lit).first == i)) {
                            expectFail = true;
                            expectFailReason = (*lit).second;
                        }
                    }

                    QTest::newRow(description.toLatin1())
                        << expect.toString() << actual.toString()
                        << equal << expectFail << expectFailReason;
//                    if (!equal)
//                        qDebug("******* %d %s", i, qPrintable(description));
                }
            } else {
                QTest::newRow("") << QString() << QString() << false << false << QString();
            }
        } else {
            // test
            QString filename = m_testFiles.at(_id / 2);
            QFETCH(QString, expect);
            QFETCH(QString, actual);
            QFETCH(bool, success);
            QFETCH(bool, expectFail);
            QFETCH(QString, expectFailReason);
            if (expectFail) {
                if (expectFailReason.isEmpty())
                    expectFailReason = QLatin1String("Will fix in the next release");
                QTest::qExpectFail("", expectFailReason.toLatin1(), QTest::Continue, filename.toLatin1(), 0);
            }
            if (success) {
                QTest::qCompare("", "", "", "", "", 0);
                ++m_passedCount;
            } else {
                QTest::qCompare(actual, expect, "actual", "expect", filename.toLatin1(), 0);
            }
        }
        _id -= m_testFiles.size() * 2;
    }
    return _id;
}

tst_Suite::tst_Suite()
    : m_passedCount(0)
{
    // the test cases that we expect will fail
    {
        QList<FailureCase> cases;
        cases << FailureCase(72, "")
              << FailureCase(73, "")
              << FailureCase(75, "")
              << FailureCase(77, "")
              << FailureCase(83, "")
              << FailureCase(85, "");
        m_expectedFailures.insert("Date/15.9.3.8-1.js", cases);
    }
    {
        QList<FailureCase> cases;
        cases << FailureCase(0, "")
              << FailureCase(1, "")
              << FailureCase(4, "")
              << FailureCase(5, "")
              << FailureCase(6, "")
              << FailureCase(11, "")
              << FailureCase(13, "")
              << FailureCase(14, "")
              << FailureCase(18, "")
              << FailureCase(19, "")
              << FailureCase(21, "")
              << FailureCase(23, "")
              << FailureCase(29, "")
              << FailureCase(31, "")
              << FailureCase(54, "")
              << FailureCase(55, "")
              << FailureCase(57, "")
              << FailureCase(59, "")
              << FailureCase(65, "")
              << FailureCase(67, "")
              << FailureCase(72, "")
              << FailureCase(73, "")
              << FailureCase(75, "")
              << FailureCase(77, "")
              << FailureCase(78, "")
              << FailureCase(83, "")
              << FailureCase(85, "")
              << FailureCase(86, "");
        m_expectedFailures.insert("Date/15.9.3.8-2.js", cases);
    }
    {
        QList<FailureCase> cases;
        cases << FailureCase(36, "")
              << FailureCase(37, "")
              << FailureCase(39, "")
              << FailureCase(41, "")
              << FailureCase(47, "")
              << FailureCase(49, "")
              << FailureCase(54, "")
              << FailureCase(55, "")
              << FailureCase(57, "")
              << FailureCase(59, "")
              << FailureCase(65, "")
              << FailureCase(67, "")
              << FailureCase(72, "")
              << FailureCase(73, "")
              << FailureCase(75, "")
              << FailureCase(77, "")
              << FailureCase(78, "")
              << FailureCase(83, "")
              << FailureCase(85, "")
              << FailureCase(86, "")
              << FailureCase(90, "")
              << FailureCase(91, "")
              << FailureCase(94, "")
              << FailureCase(95, "")
              << FailureCase(96, "")
              << FailureCase(101, "")
              << FailureCase(103, "")
              << FailureCase(104, "");
        m_expectedFailures.insert("Date/15.9.3.8-3.js", cases);
    }
    {
        QList<FailureCase> cases;
        cases << FailureCase(36, "")
              << FailureCase(37, "")
              << FailureCase(39, "")
              << FailureCase(40, "")
              << FailureCase(42, "")
              << FailureCase(47, "")
              << FailureCase(49, "")
              << FailureCase(54, "")
              << FailureCase(55, "")
              << FailureCase(57, "")
              << FailureCase(59, "")
              << FailureCase(60, "")
              << FailureCase(65, "")
              << FailureCase(67, "")
              << FailureCase(72, "")
              << FailureCase(73, "")
              << FailureCase(75, "")
              << FailureCase(76, "")
              << FailureCase(78, "")
              << FailureCase(83, "")
              << FailureCase(85, "")
              << FailureCase(86, "")
              << FailureCase(90, "")
              << FailureCase(91, "")
              << FailureCase(93, "")
              << FailureCase(95, "")
              << FailureCase(96, "")
              << FailureCase(101, "")
              << FailureCase(103, "")
              << FailureCase(104, "");
        m_expectedFailures.insert("Date/15.9.3.8-4.js", cases);
    }
    {
        QList<FailureCase> cases;
        cases << FailureCase(36, "")
              << FailureCase(37, "")
              << FailureCase(39, "")
              << FailureCase(41, "")
              << FailureCase(47, "")
              << FailureCase(49, "")
              << FailureCase(54, "")
              << FailureCase(55, "")
              << FailureCase(57, "")
              << FailureCase(59, "")
              << FailureCase(65, "")
              << FailureCase(67, "")
              << FailureCase(72, "")
              << FailureCase(73, "")
              << FailureCase(76, "")
              << FailureCase(77, "")
              << FailureCase(78, "")
              << FailureCase(83, "")
              << FailureCase(85, "")
              << FailureCase(86, "")
              << FailureCase(90, "")
              << FailureCase(91, "")
              << FailureCase(93, "")
              << FailureCase(95, "")
              << FailureCase(96, "")
              << FailureCase(101, "")
              << FailureCase(103, "")
              << FailureCase(104, "");
        m_expectedFailures.insert("Date/15.9.3.8-5.js", cases);
    }
    {
        QList<FailureCase> cases;
        cases << FailureCase(0, "")
              << FailureCase(1, "")
              << FailureCase(2, "")
              << FailureCase(3, "");
        m_expectedFailures.insert("Date/15.9.4.2-1.js", cases);
    }
    {
        QList<FailureCase> cases;
        cases << FailureCase(2, "")
              << FailureCase(3, "")
              << FailureCase(6, "")
              << FailureCase(7, "")
              << FailureCase(10, "")
              << FailureCase(11, "")
              << FailureCase(14, "")
              << FailureCase(15, "")
              << FailureCase(18, "")
              << FailureCase(19, "")
              << FailureCase(22, "")
              << FailureCase(23, "")
              << FailureCase(26, "")
              << FailureCase(27, "")
              << FailureCase(30, "")
              << FailureCase(31, "")
              << FailureCase(34, "")
              << FailureCase(35, "")
              << FailureCase(38, "")
              << FailureCase(39, "")
              << FailureCase(42, "")
              << FailureCase(43, "")
              << FailureCase(46, "")
              << FailureCase(47, "")
              << FailureCase(50, "")
              << FailureCase(51, "")
              << FailureCase(54, "")
              << FailureCase(55, "")
              << FailureCase(58, "")
              << FailureCase(59, "")
              << FailureCase(62, "")
              << FailureCase(63, "")
              << FailureCase(66, "")
              << FailureCase(67, "");
        m_expectedFailures.insert("Date/15.9.4.2.js", cases);
    }
    {
        QList<FailureCase> cases;
        cases << FailureCase(1, "")
              << FailureCase(3, "")
              << FailureCase(4, "")
              << FailureCase(5, "")
              << FailureCase(6, "")
              << FailureCase(7, "")
              << FailureCase(8, "")
              << FailureCase(9, "")
              << FailureCase(10, "")
              << FailureCase(11, "")
              << FailureCase(12, "")
              << FailureCase(13, "")
              << FailureCase(14, "");
        m_expectedFailures.insert("Date/15.9.5.2-1.js", cases);
    }
    {
        QList<FailureCase> cases;
        cases << FailureCase(1, "")
              << FailureCase(3, "")
              << FailureCase(4, "")
              << FailureCase(5, "")
              << FailureCase(6, "")
              << FailureCase(7, "")
              << FailureCase(8, "")
              << FailureCase(9, "")
              << FailureCase(10, "")
              << FailureCase(11, "")
              << FailureCase(12, "")
              << FailureCase(13, "")
              << FailureCase(14, "");
        m_expectedFailures.insert("Date/15.9.5.2.js", cases);
    }
    {
        QList<FailureCase> cases;
        cases << FailureCase(8, "Task 161033");
        m_expectedFailures.insert("ExecutionContexts/10.1.6.js", cases);
    }
    {
        QList<FailureCase> cases;
        cases << FailureCase(2, "");
        m_expectedFailures.insert("GlobalObject/15.1.2.1-1.js", cases);
    }
    {
        QList<FailureCase> cases;
        cases << FailureCase(2, "")
              << FailureCase(315, "")
              << FailureCase(316, "")
              << FailureCase(317, "")
              << FailureCase(318, "");
        m_expectedFailures.insert("GlobalObject/15.1.2.2-1.js", cases);
    }
    {
        QList<FailureCase> cases;
        cases << FailureCase(23, "")
              << FailureCase(24, "")
              << FailureCase(25, "")
              << FailureCase(26, "")
              << FailureCase(27, "")
              << FailureCase(28, "")
              << FailureCase(30, "");
        m_expectedFailures.insert("GlobalObject/15.1.2.2-2.js", cases);
    }
    {
        QList<FailureCase> cases;
        cases << FailureCase(4, "");
        m_expectedFailures.insert("GlobalObject/15.1.2.3-1.js", cases);
    }
    {
        QList<FailureCase> cases;
        cases << FailureCase(4, "");
        m_expectedFailures.insert("GlobalObject/15.1.2.4.js", cases);
    }
    {
        QList<FailureCase> cases;
        cases << FailureCase(4, "");
        m_expectedFailures.insert("GlobalObject/15.1.2.5-1.js", cases);
    }
    {
        QList<FailureCase> cases;
        cases << FailureCase(1, "");
        m_expectedFailures.insert("GlobalObject/15.1.2.6.js", cases);
    }
    {
        QList<FailureCase> cases;
        cases << FailureCase(4, "");
        m_expectedFailures.insert("GlobalObject/15.1.2.7.js", cases);
    }
    {
        QList<FailureCase> cases;
        cases << FailureCase(0, "");
        m_expectedFailures.insert("Statements/12.6.3-12.js", cases);
    }
    {
        QList<FailureCase> cases;
        cases << FailureCase(0, "");
        m_expectedFailures.insert("Statements/12.9-1-n.js", cases);
    }
    {
        QList<FailureCase> cases;
        cases << FailureCase(0, "")
              << FailureCase(1, "")
              << FailureCase(2, "")
              << FailureCase(3, "")
              << FailureCase(4, "")
              << FailureCase(5, "")
              << FailureCase(6, "")
              << FailureCase(7, "")
              << FailureCase(8, "")
              << FailureCase(9, "")
              << FailureCase(10, "")
              << FailureCase(11, "")
              << FailureCase(12, "")
              << FailureCase(13, "")
              << FailureCase(14, "")
              << FailureCase(15, "")
              << FailureCase(16, "")
              << FailureCase(17, "")
              << FailureCase(18, "")
              << FailureCase(19, "")
              << FailureCase(20, "")
              << FailureCase(21, "")
              << FailureCase(22, "")
              << FailureCase(23, "")
              << FailureCase(24, "")
              << FailureCase(25, "")
              << FailureCase(26, "")
              << FailureCase(27, "")
              << FailureCase(28, "")
              << FailureCase(29, "")
              << FailureCase(30, "")
              << FailureCase(31, "")
              << FailureCase(32, "")
              << FailureCase(33, "")
              << FailureCase(34, "")
              << FailureCase(35, "")
              << FailureCase(36, "")
              << FailureCase(37, "");
        m_expectedFailures.insert("String/15.5.4.11-2.js", cases);
    }
    {
        QList<FailureCase> cases;
        cases << FailureCase(3, "")
              << FailureCase(16, "");
        m_expectedFailures.insert("String/15.5.4.11-5.js", cases);
    }
    {
        QList<FailureCase> cases;
        cases << FailureCase(184, "")
              << FailureCase(331, "");
        m_expectedFailures.insert("String/15.5.4.12-1.js", cases);
    }
    {
        QList<FailureCase> cases;
        cases << FailureCase(80, "")
              << FailureCase(93, "");
        m_expectedFailures.insert("String/15.5.4.12-4.js", cases);
    }
    {
        QList<FailureCase> cases;
        cases << FailureCase(87, "");
        m_expectedFailures.insert("String/15.5.4.12-5.js", cases);
    }
    {
        QList<FailureCase> cases;
        cases << FailureCase(0, "")
              << FailureCase(231, "");
        m_expectedFailures.insert("String/15.5.4.6-2.js", cases);
    }
    {
        QList<FailureCase> cases;
        cases << FailureCase(3, "")
              << FailureCase(7, "")
              << FailureCase(8, "")
              << FailureCase(12, "")
              << FailureCase(13, "")
              << FailureCase(14, "")
              << FailureCase(21, "")
              << FailureCase(25, "")
              << FailureCase(26, "")
              << FailureCase(27, "")
              << FailureCase(28, "")
              << FailureCase(29, "")
              << FailureCase(81, "")
              << FailureCase(84, "")
              << FailureCase(87, "")
              << FailureCase(90, "")
              << FailureCase(92, "")
              << FailureCase(93, "")
              << FailureCase(95, "")
              << FailureCase(109, "");
        m_expectedFailures.insert("TypeConversion/9.3.1-3.js", cases);
    }

    // get the list of testfiles
    QDir dir(testsDir());
    QFileInfoList subDirs = dir.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot);
    foreach (QFileInfo testDir, subDirs) {
        QDir d(testDir.absoluteFilePath());
        QString relativePath = dir.relativeFilePath(d.absolutePath());
        QFileInfoList testFiles = d.entryInfoList(QStringList() << "*.js", QDir::Files);
        foreach (QFileInfo fi, testFiles) {
            QString name = fi.fileName();
            if (!name.at(0).isDigit())
                continue;
            m_testFiles << (relativePath + QDir::separator() + name);
        }
    }

    QVector<uint> *data = qt_meta_data_tst_Suite();
    // content:
    *data << 1 // revision
          << 0 // classname
          << 0 << 0 // classinfo
          << (m_testFiles.size() * 2) << 10 // methods
          << 0 << 0 // properties
          << 0 << 0 // enums/sets
        ;

    QVector<char> *stringdata = qt_meta_stringdata_tst_Suite();
    appendCString(stringdata, "tst_Suite");
    appendCString(stringdata, "");

    // slots: signature, parameters, type, tag, flags
    foreach (QString testfile, m_testFiles) {
        QFileInfo fi(testfile);
        QString slot = fi.fileName();
        slot = slot.left(slot.length() - 3); // kill ".js"
        QString data_slot = slot + QLatin1String("_data()");
        slot = slot + QLatin1String("()");

        *data << stringdata->size() << 10 << 10 << 10 << 0x08;
        appendCString(stringdata, data_slot.toLatin1());

        *data << stringdata->size() << 10 << 10 << 10 << 0x08;
        appendCString(stringdata, slot.toLatin1());
    }

    *data << 0; // eod

    // initialize staticMetaObject
    staticMetaObject.d.superdata = &QObject::staticMetaObject;
    staticMetaObject.d.stringdata = stringdata->constData();
    staticMetaObject.d.data = data->constData();
    staticMetaObject.d.extradata = 0;
}

tst_Suite::~tst_Suite()
{
    qDebug() << "JS testcases passed:" << m_passedCount;
}

QTEST_MAIN(tst_Suite)
