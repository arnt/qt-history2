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

#undef QCOMPARE
#define QCOMPARE(actual, expected) \
do { \
    bool ok = QTest::qCompare(actual, expected, #actual, #expected, filename.toLatin1(), 0);\
    if (ok) ++m_passedCount; else ++m_failedCount; \
} while(0)

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
    int m_passedCount;
    int m_failedCount;
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
            QString text = readFile(filename);

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
            if (count > 0) {
                for (int i = 0; i < count; ++i) {
                    QScriptValue kase = testcases.property(i);
                    Q_ASSERT(kase.isValid());
                    QString description = kase.property("description").toString();
                    QString expect = kase.property("expect").toString();
                    QString actual = kase.property("actual").toString();
                    QTest::newRow(description.toLatin1())
                        << kase.property("expect").toString()
                        << kase.property("actual").toString();
                }
            } else {
                QTest::newRow("")
                    << QString()
                    << QString();
            }
        } else {
            // test
            QString filename = m_testFiles.at(_id / 2);
            QFETCH(QString, actual);
            QFETCH(QString, expect);
            QCOMPARE(actual, expect);
        }
        _id -= m_testFiles.size() * 2;
    }
    return _id;
}

tst_Suite::tst_Suite()
    : m_passedCount(0), m_failedCount(0)
{
    // get the list of testfiles
    QDir dir(testsDir());
    QFileInfoList subDirs = dir.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot);
    foreach (QFileInfo testDir, subDirs) {
        QDir d(testDir.absoluteFilePath());
        QFileInfoList testFiles = d.entryInfoList(QStringList() << "*.js", QDir::Files);
        foreach (QFileInfo fi, testFiles) {
            QString name = fi.fileName();
            if (!name.at(0).isDigit())
                continue;
            m_testFiles << fi.absoluteFilePath();
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
    qDebug() << "passed:" << m_passedCount << "failed:" << m_failedCount;
}

QTEST_MAIN(tst_Suite)
