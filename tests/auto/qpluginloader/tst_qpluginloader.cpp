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

class QPluginLoader;
class tst_QPluginLoader : public QObject
{
    Q_OBJECT

public:
    tst_QPluginLoader();
    virtual ~tst_QPluginLoader();

private slots:
    void errorString();
    void errorString_data();

};

tst_QPluginLoader::tst_QPluginLoader()

{
}

tst_QPluginLoader::~tst_QPluginLoader()
{
}


typedef int (*VersionFunction)(void);


void tst_QPluginLoader::errorString_data()
{
    QTest::addColumn<QString>("lib");
    QTest::addColumn<bool>("result");
    QTest::addColumn<QString>("error");

    QString currDir = QDir::currentPath();
#ifdef Q_OS_WIN32
    //### Fix prefix/suffix problem for all platforms
    QTest::newRow( "ok00" ) << currDir + "/mylib.dll" << false << "'.*' is not a plugin";
#endif
}

void tst_QPluginLoader::errorString()
{
#ifdef Q_OS_WIN32
    QFETCH( QString, lib );
    QFETCH( bool, result );
    QFETCH( QString, error );

    QPluginLoader loader( lib );
    bool ok = loader.load();
    QCOMPARE(ok, result);

    QRegExp re(error);
    QVERIFY(re.exactMatch(loader.errorString()));
#endif
}

QTEST_APPLESS_MAIN(tst_QPluginLoader)
#include "tst_qpluginloader.moc"
