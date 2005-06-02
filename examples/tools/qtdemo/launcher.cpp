/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGui>
#include <QDomDocument>
#include <QtAssistant/QAssistantClient>

#include "displayshape.h"
#include "displaywidget.h"
#include "launcher.h"

Launcher::Launcher(QWidget *parent)
    : QMainWindow(parent)
{
    titleFont = font();
    titleFont.setWeight(QFont::Bold);
    buttonFont = font();
    fontRatio = 0.8;
    inFullScreenResize = false;
    currentCategory = "[starting]";

    QAction *parentPageAction1 = new QAction(tr("Show Parent Page"), this);
    QAction *parentPageAction2 = new QAction(tr("Show Parent Page"), this);
    QAction *parentPageAction3 = new QAction(tr("Show Parent Page"), this);
    parentPageAction1->setShortcut(QKeySequence(tr("Escape")));
    parentPageAction2->setShortcut(QKeySequence(tr("Backspace")));
    parentPageAction3->setShortcut(QKeySequence(tr("Alt+Left")));

    QAction *fullScreenAction = new QAction(tr("Toggle &Full Screen"), this);
    fullScreenAction->setShortcut(QKeySequence(tr("Ctrl+F")));

    QAction *exitAction = new QAction(tr("E&xit"), this);
    exitAction->setShortcut(QKeySequence(tr("Ctrl+Q")));

    connect(parentPageAction1, SIGNAL(triggered()), this, SIGNAL(showPage()));
    connect(parentPageAction2, SIGNAL(triggered()), this, SIGNAL(showPage()));
    connect(parentPageAction3, SIGNAL(triggered()), this, SIGNAL(showPage()));
    connect(fullScreenAction, SIGNAL(triggered()),
            this, SLOT(toggleFullScreen()));
    connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));

    display = new DisplayWidget;

    addAction(parentPageAction1);
    addAction(parentPageAction2);
    addAction(parentPageAction3);
    addAction(fullScreenAction);
    addAction(exitAction);

    slideshowTimer = new QTimer(this);
    resizeTimer = new QTimer(this);
    resizeTimer->setSingleShot(true);
    connect(resizeTimer, SIGNAL(timeout()), this, SLOT(redisplayWindow()));

    assistant = new QAssistantClient("assistant", this);

    connect(display, SIGNAL(actionRequested(const QString &)),
            this, SLOT(executeAction(const QString &)));
    connect(display, SIGNAL(categoryRequested(const QString &)),
            this, SLOT(showExamples(const QString &)));
    connect(display, SIGNAL(documentationRequested(const QString &)),
            this, SLOT(showExampleDocumentation(const QString &)));
    connect(display, SIGNAL(exampleRequested(const QString &)),
            this, SLOT(showExampleSummary(const QString &)));

    connect(display, SIGNAL(launchRequested(const QString &)),
            this, SLOT(launchExample(const QString &)));

    connect(this, SIGNAL(showPage()), this, SLOT(showParentPage()),
            Qt::QueuedConnection);
    connect(this, SIGNAL(windowResized()), this, SLOT(redisplayWindow()),
            Qt::QueuedConnection);

    setCentralWidget(display);
    setMaximumSize(QApplication::desktop()->screenGeometry().size());
    setWindowTitle(tr("Qt Examples and Demos"));
}

bool Launcher::setup()
{
    documentationDir = QDir(QLibraryInfo::location(
                            QLibraryInfo::DocumentationPath));

    if (!documentationDir.cd("html")) {
        // Failed to find the HTML documentation.
        // We can continue without it.
        QMessageBox::warning(this, tr("No Documentation Found"),
            tr("I could not find the Qt documentation."),
            QMessageBox::Cancel, QMessageBox::NoButton);
    }

    imagesDir = documentationDir;
    if (!imagesDir.cd("images")) {
        // Failed to find the accompanying images for the documentation.
        // We can continue without them.
        QMessageBox::warning(this, tr("No Images Found"),
            tr("I could not find any images for the Qt documentation."),
            QMessageBox::Cancel, QMessageBox::NoButton);
    }

    maximumLabels = 0;

    demosDir = QDir(QLibraryInfo::location(QLibraryInfo::DemosPath));
    int demoCategories = readInfo(":/demos.xml", demosDir);

    examplesDir = QDir(QLibraryInfo::location(QLibraryInfo::ExamplesPath));
    int exampleCategories = readInfo(":/examples.xml", examplesDir);

    if (demoCategories + exampleCategories <= 0) {
        // Failed to find the examples.
        QMessageBox::warning(this, tr("No Examples or Demos found"),
            tr("I could not find any Qt examples or demos.\n"
               "Please ensure that Qt is installed correctly."),
            QMessageBox::Cancel, QMessageBox::NoButton);
        return false;
    }

    maximumLabels = qMax(demoCategories + exampleCategories, maximumLabels);

    QString mainDescription = categoryDescriptions["[main]"];
    if (!mainDescription.isEmpty())
        mainDescription += tr("\n");

    categoryDescriptions["[main]"] = mainDescription + tr(
        "Press Escape, Backspace, or %1 to return to a previous menu.\n"
        "Press %2 to switch between normal and full screen modes.\n"
        "Use %3 to exit the launcher.").arg(QString(
            QKeySequence(tr("Alt+Left")))).arg(QString(
            QKeySequence(tr("Ctrl+F")))).arg(QString(
            QKeySequence(tr("Ctrl+Q"))));

    emit showPage();
    return true;
}

void Launcher::findDescriptionAndImages(const QString &exampleName,
                                        const QString &docName)
{
    if (documentationDir.exists(docName)) {
        documentPaths[exampleName] = \
            documentationDir.absoluteFilePath(docName);

        QDomDocument exampleDoc;
        QFile exampleFile(documentPaths[exampleName]);
        exampleDoc.setContent(&exampleFile);

        QDomNodeList paragraphs = exampleDoc.elementsByTagName("p");
        QString description;

        for (int p = 0; p < int(paragraphs.length()); ++p) {
            QDomNode descriptionNode = paragraphs.item(p);
            description = readExampleDescription(descriptionNode);

            if (description.indexOf(QRegExp(QString(
                "((The|This) )?(%1 )?.*(example|demo)").arg(exampleName),
                Qt::CaseInsensitive)) != -1) {
                exampleDescriptions[exampleName] = description;
                break;
            }
        }

        QDomNodeList images = exampleDoc.elementsByTagName("img");
        QStringList imageFiles;

        for (int i = 0; i < int(images.length()); ++i) {
            QDomElement imageElement = images.item(i).toElement();
            QString source = imageElement.attribute("src");
            if (!source.contains("-logo"))
                imageFiles.append(documentationDir.absoluteFilePath(source));
        }

        if (imageFiles.size() > 0)
            imagePaths[exampleName] = imageFiles;
    }
}

int Launcher::readInfo(const QString &resource, const QDir &dir)
{
    QFile categoriesFile(resource);
    QDomDocument document;
    document.setContent(&categoriesFile);
    QDomElement documentElement = document.documentElement();
    QDomNodeList categoryNodes = documentElement.elementsByTagName("category");

    readCategoryDescription(dir, "[main]");
    qtLogo.load(imagesDir.absoluteFilePath(":/images/qt-logo.png"));
    trolltechLogo.load(imagesDir.absoluteFilePath(":/images/trolltech-logo.png"));

    for (int i = 0; i < int(categoryNodes.length()); ++i) {

        QDomNode categoryNode = categoryNodes.item(i);
        QDomElement element = categoryNode.toElement();
        QString categoryName = element.attribute("name");
        QString categoryDirName = element.attribute("dirname");
        QString categoryDocName = element.attribute("docname");
        QString categoryColor = element.attribute("color", "#f0f0f0");

        QDir categoryDir = dir;
        if (categoryDir.cd(categoryDirName)) {

            readCategoryDescription(categoryDir, categoryName);

            examples[categoryName] = QStringList();

            QDomNodeList exampleNodes = element.elementsByTagName("example");
            maximumLabels = qMax(maximumLabels, int(exampleNodes.length()));

            for (int j = 0; j < int(exampleNodes.length()); ++j) {

                QDir exampleDir = categoryDir;

                QDomNode exampleNode = exampleNodes.item(j);
                element = exampleNode.toElement();
                QString exampleName = element.attribute("name");
                QString exampleFileName = element.attribute("filename");
                QString exampleDirName = element.attribute("dirname");
                QString exampleColor = element.attribute("color", "#f0f0f0");

                examples[categoryName].append(exampleName);

                QString docName;
                if (!categoryDocName.isEmpty())
                    docName = categoryDocName+"-"+exampleFileName+".html";
                else
                    docName = categoryDirName+"-"+exampleFileName+".html";

                exampleColors[exampleName] = exampleColor;
                findDescriptionAndImages(exampleName, docName);

                if (!exampleDirName.isEmpty() && !exampleDir.cd(exampleDirName))
                    continue;

                if (exampleDir.cd(exampleFileName)) {
                    QString examplePath = findExecutable(exampleDir);
                    if (!examplePath.isNull())
                        examplePaths[exampleName] = QPair<QString,QString>(exampleDir.absolutePath(), examplePath);
                }
            }

            categories.append(categoryName);
            categoryColors[categoryName] = categoryColor;
        }
    }

    return categories.size();
}

QString Launcher::findExecutable(const QDir &dir) const
{
    QDir parentDir = dir;
    parentDir.cdUp();

    foreach (QFileInfo info, dir.entryInfoList(QDir::Dirs | QDir::Files)) {
        if (info.isFile() && info.isExecutable())
            return info.absoluteFilePath();
        else if (info.isDir()) {
            QDir currentDir(info.absoluteFilePath());
            if (currentDir != dir && currentDir != parentDir) {
                QString path = findExecutable(currentDir);
                if (!path.isNull())
                    return path;
            }
        }
    }
    return QString();
}

void Launcher::readCategoryDescription(const QDir &categoryDir,
                                       const QString &categoryName)
{
    if (categoryDir.exists("README")) {
        QFile file(categoryDir.absoluteFilePath("README"));
        if (!file.open(QFile::ReadOnly))
            return;

        QTextStream inputStream(&file);

        QStringList paragraphs;
        QStringList currentPara;
        bool openQuote = true;

        while (!inputStream.atEnd()) {
            QString line = inputStream.readLine();
            int at = 0;
            while ((at = line.indexOf("\"", at)) != -1) {
                if (openQuote)
                    line[at] = QChar::Punctuation_InitialQuote;
                else
                    line[at] = QChar::Punctuation_FinalQuote;
                openQuote = !openQuote;
            }

            if (!line.trimmed().isEmpty()) {
                currentPara.append(line.trimmed());
            } else if (!currentPara.isEmpty()) {
                paragraphs.append(currentPara.join(" "));
                currentPara.clear();
            } else
                break;
        }

        if (!currentPara.isEmpty())
            paragraphs.append(currentPara.join(" "));

        categoryDescriptions[categoryName] = paragraphs.join("\n");
    }
}

QString Launcher::readExampleDescription(const QDomNode &parentNode) const
{
    QString description;
    QDomNode node = parentNode.firstChild();

    while (!node.isNull()) {
        if (node.isText())
            description += node.nodeValue();
        else if (node.hasChildNodes())
            description += readExampleDescription(node);

        node = node.nextSibling();
    }

    return description;
}

void Launcher::launchExample(const QString &example)
{
    if (runningExamples.contains(example))
        return;

    QProcess *process = new QProcess(this);
    connect(process, SIGNAL(finished(int)),
            this, SLOT(enableLaunching()));

#ifdef Q_OS_WIN
    //make sure it finds the dlls on windows
    QString curpath = QString::fromLocal8Bit(qgetenv("PATH").constData());
    QString newpath = QString("PATH=%1;%2").arg(
        QLibraryInfo::location(QLibraryInfo::BinariesPath), curpath);
    process->setEnvironment(QStringList(newpath));
#endif

    runningExamples.append(example);
    runningProcesses[process] = example;
    process->setWorkingDirectory(examplePaths[example].first);
    process->start(examplePaths[example].second);
}

void Launcher::enableLaunching()
{
    QProcess *process = static_cast<QProcess*>(sender());
    QString example = runningProcesses.take(process);
    delete process;
    runningExamples.removeAll(example);

    if (example == currentExample) {
        for (int i = 0; i < display->shapesCount(); ++i) {
            DisplayShape *shape = display->shape(i);
            if (shape->metaData("launch").toString() == example) {
                shape->setMetaData("fade", 15);
                display->enableUpdates();
            }
        }
    }
}

void Launcher::executeAction(const QString &action)
{
    if (action == "parent")
        showParentPage();
    else if (action == "exit") {
        if (runningExamples.size() == 0) {
            connect(display, SIGNAL(displayEmpty()), this, SLOT(close()));
            display->reset();
        } else
            close();
    }
}

void Launcher::closeEvent(QCloseEvent *event)
{
    if (runningExamples.size() > 0) {
        if (QMessageBox::warning(this, tr("Examples Running"),
                tr("There are examples running. Do you really want to exit?"),
                QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
            event->ignore();
            return;
    }

    foreach (QProcess *example, runningProcesses.keys()) {
        example->terminate();
        example->waitForFinished(1000);
    }

    qDeleteAll(runningProcesses.keys());
    resizeTimer->stop();
    resizeTimer->deleteLater();
    slideshowTimer->stop();
    slideshowTimer->deleteLater();
}

void Launcher::showParentPage()
{
    slideshowTimer->stop();
    disconnect(slideshowTimer, SIGNAL(timeout()), this, 0);

    if (!currentExample.isEmpty()) {
        currentExample = "";
        redisplayWindow();
    } else if (!currentCategory.isEmpty()) {
        currentCategory = "";
        redisplayWindow();
    }
}

void Launcher::newPage()
{
    slideshowTimer->stop();
    disconnect(slideshowTimer, SIGNAL(timeout()), this, 0);
    disconnect(display, SIGNAL(displayEmpty()), this, 0);
}

void Launcher::showCategories()
{
    newPage();
    currentCategory = "";
    currentExample = "";

    for (int i = 0; i < display->shapesCount(); ++i) {
        DisplayShape *shape = display->shape(i);

        shape->setMetaData("fade", -15);
        shape->setMetaData("fade minimum", 0);
    }

    qreal horizontalMargin = 0.025*width();
    qreal verticalMargin = 0.025*height();

    DisplayShape *title = new TitleShape(tr("Qt Examples and Demos"),
        titleFont, QPen(QColor("#a6ce39")), QPointF(),
        QSizeF(0.5*width(), 4*verticalMargin));

    title->setPosition(QPointF(width()/2 - title->rect().width()/2,
                               -title->rect().height()));
    title->setTarget(QPointF(title->position().x(), verticalMargin));

    display->appendShape(title);

    QFontMetrics buttonMetrics(buttonFont);
    qreal topMargin = 6*verticalMargin;
    qreal bottomMargin = height() - 3.2*verticalMargin;
    qreal space = bottomMargin - topMargin;
    qreal step = qMin(title->rect().height() / fontRatio,
                      space/qreal(maximumLabels));
    qreal textHeight = fontRatio * step;

    QPointF startPosition = QPointF(0.0, topMargin);
    QSizeF maxSize(10.8*horizontalMargin, textHeight);
    qreal maxWidth = 0.0;

    QList<DisplayShape*> newShapes;

    foreach (QString category, categories) {

        DisplayShape *caption = new TitleShape(category, font(), QPen(),
            startPosition, maxSize);
        caption->setPosition(QPointF(-caption->rect().width(),
                                     caption->position().y()));
        caption->setTarget(QPointF(2*horizontalMargin, caption->position().y()));

        newShapes.append(caption);

        startPosition += QPointF(0.0, step);
        maxWidth = qMax(maxWidth, caption->rect().width());
    }

    DisplayShape *exitButton = new TitleShape(tr("Exit"), font(),
        QPen(Qt::white), startPosition, maxSize);

    exitButton->setTarget(QPointF(2*horizontalMargin, exitButton->position().y()));
    newShapes.append(exitButton);

    startPosition = QPointF(width(), topMargin);
    qreal extra = (step - textHeight)/4;

    QPainterPath backgroundPath;
    backgroundPath.addRect(-2*extra, -extra, maxWidth + 4*extra, textHeight + 2*extra);

    foreach (QString category, categories) {

        DisplayShape *background = new PanelShape(backgroundPath,
            QBrush(categoryColors[category]), QBrush(QColor("#e0e0ff")),
            Qt::NoPen, startPosition,
            QSizeF(maxWidth + 4*extra, textHeight + 2*extra));

        background->setMetaData("category", category);
        background->setInteractive(true);
        background->setTarget(QPointF(2*horizontalMargin,
                                      background->position().y()));
        display->insertShape(0, background);
        startPosition += QPointF(0.0, step);
    }

    QPainterPath exitPath;
    exitPath.moveTo(-2*extra, -extra);
    exitPath.lineTo(-8*extra, textHeight/2);
    exitPath.lineTo(-extra, textHeight + extra);
    exitPath.lineTo(maxWidth + 2*extra, textHeight + extra);
    exitPath.lineTo(maxWidth + 2*extra, -extra);
    exitPath.closeSubpath();

    DisplayShape *exitBackground = new PanelShape(exitPath,
        QBrush(QColor("#a6ce39")), QBrush(QColor("#c7f745")), Qt::NoPen,
        startPosition, QSizeF(maxWidth + 10*extra, textHeight + 2*extra));

    exitBackground->setMetaData("action", "exit");
    exitBackground->setInteractive(true);
    exitBackground->setTarget(QPointF(2*horizontalMargin,
                                      exitBackground->position().y()));
    display->insertShape(0, exitBackground);

    foreach (DisplayShape *caption, newShapes) {
        QPointF position = caption->target();
        QSizeF size = caption->rect().size();
        caption->setPosition(QPointF(-maxWidth, position.y()));
        display->appendShape(caption);
    }

    qreal leftMargin = 3*horizontalMargin + maxWidth;
    qreal rightMargin = width() - 3*horizontalMargin;

    DocumentShape *description = new DocumentShape(categoryDescriptions["[main]"],
        font(), QPen(QColor(0,0,0,0)), QPointF(leftMargin, topMargin),
        QSizeF(rightMargin - leftMargin, space));

    description->setMetaData("fade", 10);
    display->appendShape(description);

    qreal imageHeight = title->rect().height() + verticalMargin;

    qreal qtLength = qMin(imageHeight, title->rect().left()-3*horizontalMargin);
    QSizeF qtMaxSize = QSizeF(qtLength, qtLength);

    DisplayShape *qtShape = new ImageShape(qtLogo,
        QPointF(2*horizontalMargin-extra, -imageHeight), qtMaxSize, 0,
        Qt::AlignLeft | Qt::AlignTop);

    qtShape->setMetaData("fade", 15);
    qtShape->setTarget(QPointF(qtShape->rect().x(), verticalMargin));
    display->insertShape(0, qtShape);

    QSizeF trolltechMaxSize = QSizeF(
        width()-3*horizontalMargin-title->rect().right(), imageHeight);

    DisplayShape *trolltechShape = new ImageShape(trolltechLogo,
        QPointF(width()-2*horizontalMargin-trolltechMaxSize.width()+extra,
                -imageHeight),
        trolltechMaxSize, 0, Qt::AlignRight | Qt::AlignTop);

    trolltechShape->setMetaData("fade", 15);
    trolltechShape->setTarget(QPointF(trolltechShape->rect().x(),
                                      verticalMargin));
    display->insertShape(0, trolltechShape);

    DisplayShape *versionCaption = new TitleShape(
        QString("Qt %1").arg(QT_VERSION_STR), font(),
        QPen(QColor(0,0,0,0)),
        QPointF(0.5*width(), height() - verticalMargin - textHeight),
        QSizeF(0.5*width()-2*horizontalMargin, textHeight),
        Qt::AlignRight | Qt::AlignVCenter);

    versionCaption->setMetaData("fade", 15);
    display->appendShape(versionCaption);

    DisplayShape *copyrightCaption = new TitleShape(
        QString("Copyright \xa9 2005 Trolltech"), font(),
        QPen(QColor(0,0,0,0)),
        QPointF(2*horizontalMargin, height() - verticalMargin - textHeight),
        QSizeF(0.5*width()-2*horizontalMargin, textHeight),
        Qt::AlignLeft | Qt::AlignVCenter);

    copyrightCaption->setMetaData("fade", 15);
    display->appendShape(copyrightCaption);
}

void Launcher::showExamples(const QString &category)
{
    newPage();
    currentCategory = category;
    currentExample = "";

    for (int i = 0; i < display->shapesCount(); ++i) {
        DisplayShape *shape = display->shape(i);

        shape->setMetaData("fade", -15);
        shape->setMetaData("fade minimum", 0);
    }

    qreal horizontalMargin = 0.025*width();
    qreal verticalMargin = 0.025*height();

    QPointF titlePosition = QPointF(0.0, 2*verticalMargin);

    DisplayShape *newTitle = new TitleShape(category, titleFont,
        QPen(Qt::white), titlePosition,
        QSizeF(0.5 * width(), 2*verticalMargin),
        Qt::AlignHCenter | Qt::AlignTop);

    newTitle->setPosition(QPointF(-newTitle->rect().width(), titlePosition.y()));
    newTitle->setTarget(QPointF(0.25*width(), titlePosition.y()));
    newTitle->setMetaData("fade", 15);

    QPainterPath backgroundPath;
    backgroundPath.addRect(0, -newTitle->rect().height()*0.3,
                           width(), newTitle->rect().height()*1.6);

    DisplayShape *titleBackground = new PanelShape(backgroundPath,
        QBrush(QColor("#a6ce39")), QBrush(QColor("#a6ce39")), Qt::NoPen,
            QPointF(width(), titlePosition.y()),
            backgroundPath.boundingRect().size());

    titleBackground->setTarget(QPointF(0.0, titlePosition.y()));

    display->insertShape(0, titleBackground);
    display->appendShape(newTitle);

    qreal topMargin = 6*verticalMargin;
    qreal bottomMargin = height() - 3.2*verticalMargin;
    qreal space = bottomMargin - topMargin;
    //qreal topMargin = 0.075 * height() + titleBackground->rect().bottom();
    //qreal space = 0.95*height() - topMargin;
    qreal step = qMin(newTitle->rect().height() / fontRatio,
                      space/qreal(maximumLabels));
    qreal textHeight = fontRatio * step;

    QPointF startPosition = QPointF(2*horizontalMargin, height() + topMargin);
    QPointF finishPosition = QPointF(2*horizontalMargin, topMargin);
    QSizeF maxSize(32*horizontalMargin, textHeight);
    qreal maxWidth = 0.0;

    foreach (QString example, examples[currentCategory]) {

        DisplayShape *caption = new TitleShape(example, font(), QPen(),
            startPosition, maxSize);
        caption->setTarget(finishPosition);

        display->appendShape(caption);

        startPosition += QPointF(0.0, step);
        finishPosition += QPointF(0.0, step);
        maxWidth = qMax(maxWidth, caption->rect().width());
    }

    DisplayShape *menuButton = new TitleShape(tr("Main Menu"), font(),
        QPen(Qt::white), startPosition, maxSize);
    menuButton->setTarget(finishPosition);

    display->appendShape(menuButton);
    maxWidth = qMax(maxWidth, menuButton->rect().width());

    startPosition = QPointF(width(), topMargin);
    qreal extra = (step - textHeight)/4;

    foreach (QString example, examples[currentCategory]) {

        QPainterPath path;
        path.addRect(-2*extra, -extra, maxWidth + 4*extra, textHeight + 2*extra);

        DisplayShape *background = new PanelShape(path,
            QBrush(exampleColors[example]), QBrush(QColor("#e0e0ff")),
            Qt::NoPen, startPosition,
            QSizeF(maxWidth + 4*extra, textHeight + 2*extra));

        background->setMetaData("example", example);
        background->setInteractive(true);
        background->setTarget(QPointF(2*horizontalMargin,
                                      background->position().y()));
        display->insertShape(0, background);
        startPosition += QPointF(0.0, step);
    }

    QPainterPath backPath;
    backPath.moveTo(-2*extra, -extra);
    backPath.lineTo(-8*extra, textHeight/2);
    backPath.lineTo(-extra, textHeight + extra);
    backPath.lineTo(maxWidth + 2*extra, textHeight + extra);
    backPath.lineTo(maxWidth + 2*extra, -extra);
    backPath.closeSubpath();

    DisplayShape *buttonBackground = new PanelShape(backPath,
        QBrush(QColor("#a6ce39")), QBrush(QColor("#c7f745")), Qt::NoPen,
        startPosition, QSizeF(maxWidth + 10*extra, textHeight + 2*extra));

    buttonBackground->setMetaData("action", "parent");
    buttonBackground->setInteractive(true);
    buttonBackground->setTarget(QPointF(2*horizontalMargin,
                                        buttonBackground->position().y()));
    display->insertShape(0, buttonBackground);

    qreal leftMargin = 3*horizontalMargin + maxWidth;
    qreal rightMargin = width() - 3*horizontalMargin;

    DocumentShape *description = new DocumentShape(
        categoryDescriptions[currentCategory], font(),
        QPen(QColor(0,0,0,0)), QPointF(leftMargin, topMargin),
        QSizeF(rightMargin - leftMargin, space));

    description->setMetaData("fade", 10);

    display->appendShape(description);

    DisplayShape *versionCaption = new TitleShape(
        QString("Qt %1").arg(QT_VERSION_STR), font(),
        QPen(QColor(0,0,0,0)),
        QPointF(0.5*width(), height() - verticalMargin - textHeight),
        QSizeF(0.5*width()-2*horizontalMargin, textHeight),
        Qt::AlignRight | Qt::AlignVCenter);

    versionCaption->setMetaData("fade", 15);
    display->appendShape(versionCaption);

    DisplayShape *copyrightCaption = new TitleShape(
        QString("Copyright \xa9 2005 Trolltech"), font(),
        QPen(QColor(0,0,0,0)),
        QPointF(2*horizontalMargin, height() - verticalMargin - textHeight),
        QSizeF(0.5*width()-2*horizontalMargin, textHeight),
        Qt::AlignLeft | Qt::AlignVCenter);

    copyrightCaption->setMetaData("fade", 15);
    display->appendShape(copyrightCaption);
}

void Launcher::showExampleDocumentation(const QString &example)
{
    disconnect(display, SIGNAL(displayEmpty()), this, 0);
    currentExample = example;

    assistant->showPage(documentPaths[example]);
}

void Launcher::showExampleSummary(const QString &example)
{
    newPage();
    currentExample = example;

    for (int i = 0; i < display->shapesCount(); ++i) {
        DisplayShape *shape = display->shape(i);

        shape->setMetaData("fade", -15);
        shape->setMetaData("fade minimum", 0);
    }

    qreal horizontalMargin = 0.025*width();
    qreal verticalMargin = 0.025*height();

    QPointF titlePosition = QPointF(0.0, 2*verticalMargin);

    DisplayShape *newTitle = new TitleShape(example, titleFont,
        QPen(Qt::white), titlePosition,
        QSizeF(0.5 * width(), 2*verticalMargin),
        Qt::AlignHCenter | Qt::AlignTop);

    newTitle->setPosition(QPointF(-newTitle->rect().width(), titlePosition.y()));
    newTitle->setTarget(QPointF(0.25*width(), titlePosition.y()));
    newTitle->setMetaData("fade", 15);

    QPainterPath backgroundPath;
    backgroundPath.addRect(0, -newTitle->rect().height()*0.3,
                           width(), newTitle->rect().height()*1.6);

    DisplayShape *titleBackground = new PanelShape(backgroundPath,
        QBrush(QColor("#a6ce39")), QBrush(QColor("#a6ce39")), Qt::NoPen,
            QPointF(width(), titlePosition.y()),
            backgroundPath.boundingRect().size());

    titleBackground->setTarget(QPointF(0.0, titlePosition.y()));
    display->insertShape(0, titleBackground);
    display->appendShape(newTitle);

    qreal topMargin = 2*verticalMargin + titleBackground->rect().bottom();
    qreal bottomMargin = height() - 8*verticalMargin;
    qreal space = bottomMargin - topMargin;
    qreal step = qMin(newTitle->rect().height() / fontRatio,
                      (bottomMargin + 4.8*verticalMargin - topMargin)
                      /qreal(maximumLabels));
    qreal textHeight = fontRatio * step;

    qreal leftMargin = 3*horizontalMargin;
    qreal rightMargin = width() - 3*horizontalMargin;

    if (exampleDescriptions.contains(example)) {
        DocumentShape *description = new DocumentShape(
            exampleDescriptions[currentExample], font(),
            QPen(QColor(0,0,0,0)), QPointF(leftMargin, topMargin),
            QSizeF(rightMargin-leftMargin, space));

        description->setMetaData("fade", 10);

        description->setPosition(QPointF(description->position().x(),
            0.8*height() - description->rect().height()));

        display->appendShape(description);
        space = description->position().y() - topMargin - 2*verticalMargin;
    }

    if (imagePaths.contains(example)) {

        QImage image(imagePaths[example][0]);

        QSizeF imageMaxSize = QSizeF(width() - 8*horizontalMargin, space);

        currentFrame = new ImageShape(image,
            QPointF(width() - imageMaxSize.width()/2, topMargin),
            imageMaxSize);

        currentFrame->setMetaData("fade", 15);
        currentFrame->setTarget(QPointF(width()/2 - imageMaxSize.width()/2,
                                        topMargin));

        display->appendShape(currentFrame);

        if (imagePaths[example].size() > 1) {
            connect(slideshowTimer, SIGNAL(timeout()),
                    this, SLOT(updateExampleSummary()));

            slideshowFrame = 0;
            slideshowTimer->start(5000);
        }
    }

    QSizeF maxSize(0.3 * width(), 2*verticalMargin);
    leftMargin = 0.0;
    rightMargin = 0.0;

    if (true) {
        DisplayShape *backButton = new TitleShape(currentCategory, font(),
            QPen(Qt::white), QPointF(0.1*width(), height()), maxSize,
            Qt::AlignLeft | Qt::AlignTop);
        backButton->setTarget(QPointF(backButton->position().x(),
                                      height() - 5.2*verticalMargin));

        display->appendShape(backButton);

        qreal maxWidth = backButton->rect().width();
        qreal textHeight = backButton->rect().height();
        qreal extra = (3*verticalMargin - textHeight)/4;

        QPainterPath path;
        path.moveTo(-extra, -extra);
        path.lineTo(-4*extra, textHeight/2);
        path.lineTo(-extra, textHeight + extra);
        path.lineTo(maxWidth + 2*extra, textHeight + extra);
        path.lineTo(maxWidth + 2*extra, -extra);
        path.closeSubpath();

        DisplayShape *buttonBackground = new PanelShape(path,
            QBrush(QColor("#a6ce39")), QBrush(QColor("#c7f745")), Qt::NoPen,
            backButton->position(),
            QSizeF(maxWidth + 6*extra, textHeight + 2*extra));

        buttonBackground->setMetaData("category", currentCategory);
        buttonBackground->setInteractive(true);
        buttonBackground->setTarget(backButton->target());

        display->insertShape(0, buttonBackground);

        leftMargin = buttonBackground->rect().right();
    }

    if (examplePaths.contains(example)) {

        DisplayShape *launchCaption = new TitleShape(tr("Launch"),
            font(), QPen(Qt::white), QPointF(0.0, 0.0), maxSize,
            Qt::AlignLeft | Qt::AlignTop);
        launchCaption->setPosition(QPointF(
            0.9*width() - launchCaption->rect().width(), height()));
        launchCaption->setTarget(QPointF(launchCaption->position().x(),
                                         height() - 5.2*verticalMargin));

        display->appendShape(launchCaption);

        qreal maxWidth = launchCaption->rect().width();
        qreal textHeight = launchCaption->rect().height();
        qreal extra = (3*verticalMargin - textHeight)/4;

        QPainterPath path;
        path.addRect(-2*extra, -extra, maxWidth + 4*extra, textHeight + 2*extra);

        QColor backgroundColor = QColor("#a63e39");
        QColor highlightedColor = QColor("#f95e56");
        if (runningExamples.contains(example)) {
            backgroundColor.setAlpha(15);
            highlightedColor.setAlpha(15);
        }

        DisplayShape *background = new PanelShape(path,
            QBrush(backgroundColor), QBrush(highlightedColor), Qt::NoPen,
            launchCaption->position(),
            QSizeF(maxWidth + 4*extra, textHeight + 2*extra));

        background->setMetaData("fade minimum", 120);
        background->setMetaData("launch", example);
        background->setInteractive(true);
        background->setTarget(launchCaption->target());

        display->insertShape(0, background);

        rightMargin = background->rect().left();
    }

    if (documentPaths.contains(example)) {

        DisplayShape *documentCaption = new TitleShape(tr("Show Documentation"),
            font(), QPen(Qt::white), QPointF(0.0, 0.0), maxSize,
            Qt::AlignLeft | Qt::AlignTop);

        if (rightMargin == 0.0) {
            documentCaption->setPosition(QPointF(
                0.9*width() - documentCaption->rect().width(), height()));
        } else {
            documentCaption->setPosition(QPointF(
                leftMargin/2 + rightMargin/2 - documentCaption->rect().width()/2,
                height()));
        }
        documentCaption->setTarget(QPointF(documentCaption->position().x(),
                                           height() - 5.2*verticalMargin));

        display->appendShape(documentCaption);

        qreal maxWidth = documentCaption->rect().width();
        qreal textHeight = documentCaption->rect().height();
        qreal extra = (3*verticalMargin - textHeight)/4;

        QPainterPath path;
        path.addRect(-2*extra, -extra, maxWidth + 4*extra, textHeight + 2*extra);

        DisplayShape *background = new PanelShape(path,
            QBrush(QColor("#9c9cff")), QBrush(QColor("#cfcfff")), Qt::NoPen,
            documentCaption->position(),
            QSizeF(maxWidth + 4*extra, textHeight + 2*extra));

        background->setMetaData("fade minimum", 120);
        background->setMetaData("documentation", example);
        background->setInteractive(true);
        background->setTarget(documentCaption->target());

        display->insertShape(0, background);
    }

    DisplayShape *versionCaption = new TitleShape(
        QString("Qt %1").arg(QT_VERSION_STR), font(),
        QPen(QColor(0,0,0,0)),
        QPointF(0.5*width(), height() - verticalMargin - textHeight),
        QSizeF(0.5*width()-2*horizontalMargin, textHeight),
        Qt::AlignRight | Qt::AlignVCenter);

    versionCaption->setMetaData("fade", 15);
    display->appendShape(versionCaption);

    DisplayShape *copyrightCaption = new TitleShape(
        QString("Copyright \xa9 2005 Trolltech"), font(),
        QPen(QColor(0,0,0,0)),
        QPointF(2*horizontalMargin, height() - verticalMargin - textHeight),
        QSizeF(0.5*width()-2*horizontalMargin, textHeight),
        Qt::AlignLeft | Qt::AlignVCenter);

    copyrightCaption->setMetaData("fade", 15);
    display->appendShape(copyrightCaption);
}

void Launcher::updateExampleSummary()
{
    if (imagePaths.contains(currentExample)) {

        currentFrame->setMetaData("fade", -15);
        currentFrame->setTarget(currentFrame->position() - QPointF(0.5*width(),
                                0));

        slideshowFrame = (slideshowFrame+1) % imagePaths[currentExample].size();
        QImage image(imagePaths[currentExample][slideshowFrame]);

        QSizeF imageSize = currentFrame->size();
        QPointF imagePosition = QPointF(width() - imageSize.width()/2,
                                        currentFrame->position().y());

        currentFrame = new ImageShape(image, imagePosition, imageSize);
        currentFrame->setMetaData("fade", 15);
        currentFrame->setTarget(QPointF(width()/2 - imageSize.width()/2,
                                        imagePosition.y()));

        display->appendShape(currentFrame);
    }
}

void Launcher::toggleFullScreen()
{
    if (inFullScreenResize)
        return;

    inFullScreenResize = true;

    connect(display, SIGNAL(displayEmpty()), this, SLOT(resizeWindow()),
            Qt::QueuedConnection);
    display->reset();
}

void Launcher::resizeWindow()
{
    disconnect(display, SIGNAL(displayEmpty()), this, 0);

    if (isFullScreen())
        showNormal();
    else
        showFullScreen();
}

void Launcher::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    if (inFullScreenResize) {
        emit windowResized();
        inFullScreenResize = false;
    } else if (currentCategory != "[starting]") {
        resizeTimer->start(500);
    }
}

void Launcher::redisplayWindow()
{
    if (!currentExample.isEmpty())
        showExampleSummary(currentExample);
    else if (!currentCategory.isEmpty())
        showExamples(currentCategory);
    else
        showCategories();
}
