#ifndef LAUNCHERPANEL_H
#define LAUNCHERPANEL_H

#include <QDir>
#include <QMap>
#include <QWidget>

class QAssistantClient;
class QCheckBox;
class QStringList;
class QVBoxLayout;

class LauncherPanel : public QWidget
{
    Q_OBJECT

public:
    LauncherPanel();

private slots:
    void launchExample(QAction *action);

private:
    void findAllExamples();
    void fillFriendlyStringMap();
    void createPopups();
    QStringList subDirs(const QDir &dir);
    QString canonicalName(const QString &name);
    QString friendlyName(const QString &name);
    QString extractWindowTitle(const QString &fileName);

    QDir qtDir;
    QDir examplesDir;
    QMap<QString, QStringList> examplesForCategoryMap;
    QMap<QString, QString> friendlyStringMap;
    QMap<QAction *, QString> programForActionMap;
    QMap<QAction *, QString> helpPageForActionMap;

    QVBoxLayout *popupButtonLayout;
    QCheckBox *showDocumentationCheckBox;
    QAssistantClient *assistantClient;
};

#endif
