#include <QtGui>

#include <../../tools/assistant/lib/qassistantclient.h> // ###

#include "launcherpanel.h"

LauncherPanel::LauncherPanel()
{
    examplesDir = QDir::current(); // QCoreApplication::applicationDirPath();
    examplesDir.cdUp();
    examplesDir.cdUp();

    qtDir = examplesDir;
    qtDir.cdUp();

    popupButtonLayout = new QVBoxLayout(this);

    findAllExamples();
    fillFriendlyStringMap();
    createPopups();

    showDocumentationCheckBox = new QCheckBox(tr("Show documentation"));
    popupButtonLayout->addWidget(showDocumentationCheckBox);

    assistantClient = new QAssistantClient("", this);

    setWindowTitle(tr("Launcher"));
}

void LauncherPanel::launchExample(QAction *action)
{
    QString program = programForActionMap.value(action);
    QProcess::startDetached(program);

    if (showDocumentationCheckBox->isChecked())
        assistantClient->showPage(helpPageForActionMap.value(action));
}
    
void LauncherPanel::findAllExamples()
{
    static const char *exceptionTable[] = {
        "customwidget",
        "launcher",
        "semaphores",
        "waitconditions",
        "worldtimeclockplugin",
        0
    };

    foreach (QString category, subDirs(examplesDir)) {
        QDir dir(examplesDir);
        dir.cd(category);

        QStringList examples = subDirs(dir);
        for (int i = 0; exceptionTable[i]; ++i)
            examples.removeAll(exceptionTable[i]);

        examplesForCategoryMap[category] = examples;
    }
}

void LauncherPanel::fillFriendlyStringMap()
{
    static const char *exceptionTable[] = {
        "Drag and Drop",
        "Item Views",
        "Main Windows",
        "OpenGL",
        "Query Model",
        "Relational Table Model",
        "Rich Text",
        "Table Model",
        0
    };

    QStringList fileFilters;
    fileFilters << "*.cpp" << "*.ui";

    QMapIterator<QString, QStringList> i(examplesForCategoryMap);
    while (i.hasNext()) {
        i.next();

        QDir dir(examplesDir);
        dir.cd(i.key());

        foreach (QString example, i.value()) {
            dir.cd(example);

            QStringList sourceFiles = dir.entryList(fileFilters);
            QString windowTitle;

            foreach (QString sourceFile, sourceFiles) {
                windowTitle = extractWindowTitle(dir.filePath(sourceFile));
                if (!windowTitle.isEmpty()) {
                    QStringList candidates = windowTitle.split(" - ");
                    foreach (QString candidate, candidates) {
                        if (canonicalName(candidate) == example) {
                            friendlyStringMap[example] = candidate;
                            break;
                        }
                    }
                }
            }
            dir.cdUp();
        }
    }

    for (int i = 0; exceptionTable[i]; ++i) {
        QString name = exceptionTable[i];
        friendlyStringMap[canonicalName(name)] = name;
    }
}

void LauncherPanel::createPopups()
{
    QMapIterator<QString, QStringList> i(examplesForCategoryMap);
    while (i.hasNext()) {
        i.next();

        QMenu *menu = new QMenu(friendlyName(i.key()));
        connect(menu, SIGNAL(triggered(QAction *)),
                this, SLOT(launchExample(QAction *)));

        foreach (QString example, i.value()) {
            QAction *action = new QAction(friendlyName(example));
            menu->addAction(action);
            programForActionMap[action] =
                examplesDir.filePath(i.key() + "/" + example + "/" + example);
            helpPageForActionMap[action] =
                qtDir.filePath("doc/html/" + i.key() + "-" + example + ".html");
        }

        QPushButton *button = new QPushButton(friendlyName(i.key()));
        button->setMenu(menu);

        popupButtonLayout->addWidget(button);
    }
}

QStringList LauncherPanel::subDirs(const QDir &dir)
{
    QStringList result = dir.entryList(QDir::Dirs, QDir::Name);
    result.removeAll(".");
    result.removeAll("..");
    return result;
}

QString LauncherPanel::canonicalName(const QString &name)
{
    QString result;
    for (int i = 0; i < name.size(); ++i) {
        if (name[i].isLetterOrNumber())
            result += name[i].toLower();
    }
    return result;
}

QString LauncherPanel::friendlyName(const QString &name)
{
    QString defaultName = name;
    if (defaultName.size() <= 3) {
        defaultName = defaultName.toUpper();
    } else {
        defaultName[0] = defaultName[0].toUpper();
    }
    return friendlyStringMap.value(name, defaultName);
}

QString LauncherPanel::extractWindowTitle(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        return QString();

    if (fileName.endsWith(".ui")) {
        QRegExp propertyRegExp("<property\\s+name=\"windowTitle\"\\s*>");
        QRegExp stringRegExp("<string>(.*)</string>");
        QTextStream in(&file);

        while (!in.atEnd()) {
            QString line = in.readLine();
            if (line.indexOf(propertyRegExp) != -1) {
                QString nextLine = in.readLine();
                if (nextLine.indexOf(stringRegExp) != -1)
                    return stringRegExp.cap(1);
            }
        }
    } else {
        QRegExp funcCallRegExp("setWindowTitle\\(.*\"([^\"]+)\"");
        QTextStream in(&file);

        while (!in.atEnd()) {
            QString line = in.readLine();
            if (line.indexOf(funcCallRegExp) != -1)
                return funcCallRegExp.cap(1);
        }
    }

    return QString();
}
