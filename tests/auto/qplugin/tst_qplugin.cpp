#include <QtTest/QtTest>

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QPluginLoader>

class tst_QPlugin : public QObject
{
    Q_OBJECT

    QDir dir;

public:
    tst_QPlugin();

private slots:
    void loadDebugPlugin();
    void loadReleasePlugin();
};

tst_QPlugin::tst_QPlugin()
    : dir("plugins")
{
}

void tst_QPlugin::loadDebugPlugin()
{
    foreach (QString fileName, dir.entryList(QStringList() << "*debug*", QDir::Files)) {
        if (!QLibrary::isLibrary(fileName))
            continue;
        QPluginLoader loader(dir.filePath(fileName));
#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
        // we can always load a plugin on unix
        QVERIFY(loader.load());
        QObject *object = loader.instance();
        QVERIFY(object != 0);
#else
        // loading a plugin is dependent on which lib we are running against
#  if defined(QT_NO_DEBUG)
        // release build, we cannot load debug plugins
        QVERIFY(!loader.load());
#  else
        // debug build, we can load debug plugins
        QVERIFY(loader.load());
        QObject *object = loader.instance();
        QVERIFY(object != 0);
#  endif
#endif
    }
}

void tst_QPlugin::loadReleasePlugin()
{
    foreach (QString fileName, dir.entryList(QStringList() << "*release*", QDir::Files)) {
        if (!QLibrary::isLibrary(fileName))
            continue;
        QPluginLoader loader(dir.filePath(fileName));
#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
        // we can always load a plugin on unix
        QVERIFY(loader.load());
        QObject *object = loader.instance();
        QVERIFY(object != 0);
#else
        // loading a plugin is dependent on which lib we are running against
#  if defined(QT_NO_DEBUG)
        // release build, we can load debug plugins
        QVERIFY(loader.load());
        QObject *object = loader.instance();
        QVERIFY(object != 0);
#  else
        // debug build, we cannot load debug plugins
        QVERIFY(!loader.load());
#  endif
#endif
    }
}

QTEST_MAIN(tst_QPlugin)
#include "tst_qplugin.moc"
