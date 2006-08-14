/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <qdir.h>
#include <qlibrary.h>
#include <QtCore/QRegExp>

// Helper macros to let us know if some suffixes are valid
#define bundle_VALID    false
#define dylib_VALID     false
#define sl_VALID        false
#define a_VALID         false
#define so_VALID        false
#define dll_VALID       false

#if defined(Q_OS_DARWIN)
# undef bundle_VALID
# undef dylib_VALID
# undef so_VALID
# define bundle_VALID   true
# define dylib_VALID    true
# define so_VALID       true
# define SUFFIX         ".dylib"

#elif defined(Q_OS_HPUX) && !defined(__ia64)
# undef sl_VALID
# define sl_VALID       true
# define SUFFIX         ".sl"

#elif defined(Q_OS_AIX)
# undef a_VALID
# undef so_VALID
# define a_VALID        true
# define so_VALID       true
# define SUFFIX         ".a"

#elif defined(Q_OS_WIN)
# undef dll_VALID
# define dll_VALID      true
# define SUFFIX         ".dll"

#else  // all other Unix
# undef so_VALID
# define so_VALID       true
# define SUFFIX         ".so"
#endif


//TESTED_CLASS=
//TESTED_FILES=corelib/plugin/qlibrary.h corelib/plugin/qlibrary.cpp

class QLibrary;
class tst_QLibrary : public QObject
{
    Q_OBJECT

public:
    tst_QLibrary();
    virtual ~tst_QLibrary();

enum QLibraryOperation {
    Load,
    Unload,
    Resolve
};
private slots:
    void load();
    void load_data();
    void library_data();
    void resolve_data();
    void resolve();
    void unload_data();
    void unload();
    void isLibrary_data();
    void isLibrary();
    void version_data();
    void version();
    void errorString_data();
    void errorString();

};

tst_QLibrary::tst_QLibrary()

{
}

tst_QLibrary::~tst_QLibrary()
{
}


typedef int (*VersionFunction)(void);

void tst_QLibrary::version_data()
{
    QTest::addColumn<QString>("lib");
    QTest::addColumn<int>("loadversion");
    QTest::addColumn<int>("resultversion");
 
    QTest::newRow( "ok00, version 1" ) << "mylib" << 1 << 1;
    QTest::newRow( "ok00, version 2" ) << "mylib" << 2 << 2;
    QTest::newRow( "ok00, default to last version" ) << "mylib" << -1 << 2;
}

void tst_QLibrary::version()
{
    QFETCH( QString, lib );
    QFETCH( int, loadversion );
    QFETCH( int, resultversion );

#if QT_VERSION >= 0x040200 && !defined(Q_OS_AIX) && !defined(Q_OS_WIN)
    QString currDir = QDir::currentPath();
    QLibrary library( currDir + QLatin1Char('/') + lib, loadversion );
    bool ok = library.load();
    QVERIFY(ok);

    VersionFunction fnVersion = (VersionFunction)library.resolve("version");
    QVERIFY(fnVersion);
    QCOMPARE(fnVersion(), resultversion);
#else
    Q_UNUSED(lib);
    Q_UNUSED(loadversion);
    Q_UNUSED(resultversion);
#endif
    
}

void tst_QLibrary::load_data()
{
    QTest::addColumn<QString>("lib");
    QTest::addColumn<bool>("result");

    QString currDir = QDir::currentPath();
    QTest::newRow( "ok00" ) << currDir + "/mylib" << (bool)TRUE;
    QTest::newRow( "bad00" ) << currDir + "/nolib" << (bool)FALSE;

#ifdef Q_OS_MAC
    QTest::newRow("ok (libmylib ver. 1)") << currDir + "/libmylib" <<(bool)TRUE;
#endif

#if QT_VERSION >= 0x040103
# if defined Q_OS_WIN32
    QTest::newRow( "ok01 (with suffix)" ) << currDir + "/mylib.dll" << (bool)TRUE;
    QTest::newRow( "ok02 (with non-standard suffix)" ) << currDir + "/mylib.dl2" << (bool)TRUE;
    QTest::newRow( "ok03 (with many dots)" ) << currDir + "/system.trolltech.test.mylib.dll" << (bool)TRUE;
# elif defined Q_OS_UNIX
    QTest::newRow( "ok01 (with suffix)" ) << currDir + "/libmylib" SUFFIX << (bool)TRUE;
    QTest::newRow( "ok02 (with non-standard suffix)" ) << currDir + "/libmylib.so2" << (bool)TRUE;
    QTest::newRow( "ok03 (with non-standard suffix)" ) << currDir + "/system.trolltech.test.mylib.so" << (bool)TRUE;
# endif  // Q_OS_UNIX
#endif   // QT_VERSION
}

void tst_QLibrary::load()
{
    QFETCH( QString, lib );
    QFETCH( bool, result );

    QLibrary library( lib );
    bool ok = library.load();
    if ( result ) {
	QVERIFY( ok );
    } else {
	QVERIFY( !ok );
    }
}

void tst_QLibrary::unload_data()
{
    QTest::addColumn<QString>("lib");
    QTest::addColumn<bool>("result");

    QString currDir = QDir::currentPath();
    QTest::newRow( "ok00" ) << currDir + "/mylib" << (bool)TRUE;

    QTest::newRow( "ok01" ) << currDir + "/nolib" << (bool)TRUE;
}

void tst_QLibrary::unload()
{
    QFETCH( QString, lib );
    QFETCH( bool, result );

    QLibrary library( lib );
    library.load();
#ifdef Q_WS_MAC
    QEXPECT_FAIL("ok00", "dlcompat cannot unload libraries", Continue);
#endif
    bool ok = library.unload();
    if ( result ) {
	QVERIFY( ok );
    } else {
	QVERIFY( !ok );
    }
}

void tst_QLibrary::resolve_data()
{
    QTest::addColumn<QString>("lib");
    QTest::addColumn<QString>("symbol");
    QTest::addColumn<bool>("goodPointer");

    QString currDir = QDir::currentPath();
    QTest::newRow( "ok00" ) << currDir + "/mylib" << QString("version") << (bool)TRUE;
    QTest::newRow( "bad00" ) << currDir + "/mylib" << QString("nosym") << (bool)FALSE;
    QTest::newRow( "bad01" ) << currDir + "/nolib" << QString("nosym") << (bool)FALSE;
}

void tst_QLibrary::resolve()
{
    typedef int (*testFunc)();
    QFETCH( QString, lib );
    QFETCH( QString, symbol );
    QFETCH( bool, goodPointer );

    QLibrary library( lib );
    testFunc func = (testFunc) library.resolve( symbol.toLatin1() );
    if ( goodPointer ) {
	QVERIFY( func != 0 );
    } else {
	QVERIFY( func == 0 );
    }
}

void tst_QLibrary::library_data()
{
    QTest::addColumn<QString>("lib");
}

void tst_QLibrary::isLibrary_data()
{
    QTest::addColumn<QString>("filename");
    QTest::addColumn<bool>("valid");

    // use the macros #defined at the top of the file
    QTest::newRow("bad") << QString("mylib.bad") << false;
    QTest::newRow(".a") << QString("mylib.a") << a_VALID;
    QTest::newRow(".bundle") << QString("mylib.bundle") << bundle_VALID;
    QTest::newRow(".dll") << QString("mylib.dll") << dll_VALID;
    QTest::newRow(".dl2" ) << QString("mylib.dl2") << false;
    QTest::newRow(".dylib") << QString("mylib.dylib") << dylib_VALID;
    QTest::newRow(".sl") << QString("mylib.sl") << sl_VALID;
    QTest::newRow(".so") << QString("mylib.so") << so_VALID;

    // special tests:
#ifdef Q_OS_MAC
    QTest::newRow("good (libmylib.1.0.0.dylib)") << QString("libmylib.1.0.0.dylib") << true;
    QTest::newRow("good (libmylib.dylib)") << QString("libmylib.dylib") << true;
    QTest::newRow("good (libmylib.so)") << QString("libmylib.so") << true;
    QTest::newRow("good (libmylib.so.1.0.0)") << QString("libmylib.so.1.0.0") << true;

    QTest::newRow("bad (libmylib.1.0.0.foo)") << QString("libmylib.1.0.0.foo") << false;
#elif defined(Q_OS_WIN)
    QTest::newRow("good (with many dots)" ) << "/system.trolltech.test.mylib.dll" << true;
#endif    
}

void tst_QLibrary::isLibrary()
{
    QFETCH( QString, filename );
    QFETCH( bool, valid );

    QCOMPARE(QLibrary::isLibrary(filename), valid);
}

void tst_QLibrary::errorString_data()
{
    QTest::addColumn<int>("operation");
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<bool>("success");
    QTest::addColumn<QString>("errorString");

    QString currDir = QDir::currentPath();

    QTest::newRow("bad load()") << (int)Load << QString("nosuchlib") << false << QString("QLibrary::load_sys: Cannot load nosuchlib \\(.*\\)");
    QTest::newRow("bad resolve") << (int)Resolve << currDir + "/mylib" << false << QString("QLibrary::resolve_sys: Symbol \"nosuchsymbol\" undefined in \\S+ \\(.*\\)");
    QTest::newRow("good resolve") << (int)Resolve << currDir + "/mylib" << true << QString("Unknown error");


#ifdef Q_OS_WIN
    QTest::newRow("bad load()") << (int)Load << QString("nosuchlib.dll") << false << QString("QLibrary::load_sys: Cannot load nosuchlib.dll \\(The specified module could not be found.\\)");
//    QTest::newRow("bad unload") << (int)Unload << QString("nosuchlib.dll") << false << QString("QLibrary::unload_sys: Cannot unload nosuchlib.dll (The specified module could not be found.)");
#elif Q_OS_MAC

#else

#endif
}

void tst_QLibrary::errorString()
{
    QFETCH(int, operation); 
    QFETCH(QString, fileName);
    QFETCH(bool, success);
    QFETCH(QString, errorString);
    
    
    QLibrary lib(fileName);

    bool ok = false;
    switch (operation) {
        case Load:
            ok = lib.load();
            break;
        case Unload:
            ok = lib.load();    //###
            ok = lib.unload();
            break;
        case Resolve: {
            ok = lib.load();
            QCOMPARE(ok, true);
            if (success) {
                ok = lib.resolve("version");
            } else {
                ok = lib.resolve("nosuchsymbol");
            }
            break;}
        default:
            Q_ASSERT(0);
            break;
    }
    QRegExp re(errorString);
    QVERIFY(re.exactMatch(lib.errorString()));
    QCOMPARE(ok, success);
}

QTEST_APPLESS_MAIN(tst_QLibrary)
#include "tst_qlibrary.moc"
