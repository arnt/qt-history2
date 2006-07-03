#include "framework.h"

#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QStringList>
#include <QtDebug>

Framework::Framework()
    : qsettings(0)
{
}

Framework::Framework(const QString &file)
    : qsettings(0)
{
    load(file);
}

Framework::~Framework()
{
    delete qsettings;
    qsettings = 0;
}

QString Framework::basePath() const
{
    if (!qsettings)
        return QString();

    QFileInfo fi(qsettings->fileName());
    return fi.absolutePath();
}


QStringList Framework::suites() const
{
    if (!qsettings)
        return QStringList();

    QStringList tests = qsettings->childGroups();
    qDebug()<<"here suites "<<tests;
    tests.removeAll("General");
    tests.removeAll("Blacklist");
    return tests;
}


bool Framework::isTestBlacklisted(const QString &engineName,
                                  const QString &testcase) const
{
    return m_blacklist[engineName].contains(testcase);
}

bool Framework::isValid() const
{
    return qsettings;
}

void Framework::load(const QString &file)
{
    if (qsettings) {
        delete qsettings;
        qsettings = 0;
    }
    if (QFile::exists(file)) {
        qsettings = new QSettings(file, QSettings::IniFormat);
        qsettings->beginGroup(QString("Blacklist"));
        QStringList engines = qsettings->childKeys();
        foreach(QString engineName, engines) {
            QStringList testcases = qsettings->value(engineName).toStringList();
            m_blacklist.insert(engineName, testcases);
            qDebug()<<"Blacklists for "<<testcases;
        }
        qsettings->endGroup();
    }
}

QString Framework::outputDir() const
{
    qsettings->beginGroup("General");
    QString outputDirName = qsettings->value("outputDir").toString();
    qsettings->endGroup();
    return outputDirName;
}

QSettings * Framework::settings() const
{
    return qsettings;
}
