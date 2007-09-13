#ifndef FRAMEWORK_H
#define FRAMEWORK_H

#include <QMap>
#include <QStringList>
#include <QString>

QT_DECLARE_CLASS(QSettings)

class Framework
{
public:
    Framework();
    Framework(const QString &file);
    ~Framework();

    bool isValid() const;

    void load(const QString &file);

    QSettings *settings() const;


    QString basePath() const;
    QString outputDir() const;

    QStringList suites() const;

    bool isTestBlacklisted(const QString &engineName,
                           const QString &testcase) const;
private:
    QSettings *qsettings;
    QMap<QString, QStringList> m_blacklist;
};

#endif
