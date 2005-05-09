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

    findResources();

    QAction *restartAction = new QAction(tr("Restart"), this);
    restartAction->setShortcut(QKeySequence(tr("Escape")));

    QAction *fullScreenAction = new QAction(tr("Toggle &Full Screen"), this);
    fullScreenAction->setShortcut(QKeySequence(tr("Ctrl+F")));

    QAction *exitAction = new QAction(tr("E&xit"), this);
    exitAction->setShortcut(QKeySequence(tr("Ctrl+Q")));

    connect(restartAction, SIGNAL(triggered()), this, SIGNAL(restart()));
    connect(fullScreenAction, SIGNAL(triggered()),
            this, SLOT(toggleFullScreen()));
    connect(exitAction, SIGNAL(triggered()), qApp, SLOT(quit()));

    display = new DisplayWidget;

    addAction(restartAction);
    addAction(fullScreenAction);
    addAction(exitAction);

    slideshowTimer = new QTimer(this);

    assistant = new QAssistantClient("assistant", this);

    connect(display, SIGNAL(categoryRequested(const QString &)),
            this, SLOT(showExamples(const QString &)));
    connect(display, SIGNAL(documentationRequested(const QString &)),
            this, SLOT(showExampleDocumentation(const QString &)));
    connect(display, SIGNAL(exampleRequested(const QString &)),
            this, SLOT(showExampleSummary(const QString &)));

    launched = false;
    connect(display, SIGNAL(launchRequested(const QString &)),
            this, SLOT(launchExample(const QString &)));

    connect(this, SIGNAL(restart()), this, SLOT(reset()),
            Qt::QueuedConnection);
    connect(this, SIGNAL(windowResized()), this, SLOT(redisplayWindow()),
            Qt::QueuedConnection);

    loadExampleInfo();

    setCentralWidget(display);
    layout()->setSizeConstraint(QLayout::SetFixedSize);
    setWindowTitle(tr("Launcher"));
}

void Launcher::findResources()
{
    documentationDir = QDir(QLibraryInfo::location(
                            QLibraryInfo::DocumentationPath));

    if (!documentationDir.cd("html")) {
        // Failed to find the HTML documentation.
        // We can continue without it.
        QMessageBox::warning(this, tr("No Examples Found"),
            tr("I could not find the Qt documentation."),
            QMessageBox::Cancel, QMessageBox::NoButton);
    }

    imagesDir = documentationDir;
    if (!imagesDir.cd("images")) {
        // Failed to find the accompanying images for the documentation.
        // We can continue without them.
        QMessageBox::warning(this, tr("No Examples Found"),
            tr("I could not find any images for the Qt documentation."),
            QMessageBox::Cancel, QMessageBox::NoButton);
    }

    // Assume that the current example resides on a path ending with
    // .../examples/tools/launcher and try to locate the main examples
    // directory. We record the names of the subdirectories as we
    // move up the directory tree so that we can find the other example
    // executables later.
    QString launcherPath = qApp->applicationFilePath();

    int index = launcherPath.lastIndexOf("examples");
    if (index == -1) {
        // Failed to find the examples.
        QMessageBox::warning(this, tr("No Examples Found"),
            tr("I could not find any Qt examples."),
            QMessageBox::Cancel, QMessageBox::NoButton);
        close();
    }

    examplesDir = QDir(launcherPath.left(index));
    examplesDir.cd("examples");

    QStringList subDirs = launcherPath.mid(index).split("/");
    subDirs.pop_front(); // We should now be in the examples directory.
    subDirs.pop_front(); // We should now be in the tools directory.

    foreach (QString subDir, subDirs) {
        int launchIndex = subDir.indexOf("launcher");
        if (launchIndex != -1)
            examplePieces.append(subDir.left(launchIndex).replace("%", "%?")
                + "%1" + subDir.mid(launchIndex+8).replace("%", "%?"));
        else
            examplePieces.append(subDir.replace("%", "%?"));
    }
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

        if (paragraphs.length() >= 2) {
            QDomNode descriptionNode = paragraphs.item(1);
            exampleDescriptions[exampleName] = readExampleDescription(
                descriptionNode);
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

void Launcher::loadExampleInfo()
{
    QFile categoriesFile(":/information.xml");
    QDomDocument document;
    document.setContent(&categoriesFile);
    QDomElement documentElement = document.documentElement();
    QDomNodeList categoryNodes = documentElement.elementsByTagName("category");
    QDomElement descriptionElement = documentElement.elementsByTagName(
        "description").item(0).toElement();

    mainDescription = descriptionElement.childNodes(
        ).item(0).toCDATASection().nodeValue();

    maximumLabels = int(categoryNodes.length()+1);

    for (int i = 0; i < int(categoryNodes.length()); ++i) {

        QDomNode categoryNode = categoryNodes.item(i);
        QDomElement element = categoryNode.toElement();
        QString categoryName = element.attribute("name");
        QString categoryDirName = element.attribute("dirname");

        categoryDescriptions[categoryName] = readCategoryDescription(
            element.elementsByTagName("description").item(0));

        QDir categoryDir = examplesDir;
        if (categoryDir.cd(categoryDirName)) {

            examples[categoryName] = QStringList();

            QDomNodeList exampleNodes = element.elementsByTagName("example");
            maximumLabels = qMax(maximumLabels, int(exampleNodes.length()));

            for (int j = 0; j < int(exampleNodes.length()); ++j) {

                QDomNode exampleNode = exampleNodes.item(j);
                element = exampleNode.toElement();
                QString exampleName = element.attribute("name");
                QString exampleFileName = element.attribute("filename");

                QDir exampleDir = categoryDir;

                for (int p = 0; p < examplePieces.size(); ++p) {
                    QString name = examplePieces[p].arg(exampleFileName
                                        ).replace("%?", "%");

                    if (p == examplePieces.size()-1) {
                        QFileInfo binary(exampleDir.absoluteFilePath(name));
                        if (binary.exists() && binary.isExecutable()) {
                            examples[categoryName].append(exampleName);
                            examplePaths[exampleName] = binary.absoluteFilePath();
                        }
                    } else {
                        if (!exampleDir.cd(name))
                            break;
                    }
                }

                QString docName = categoryDirName+"-"+exampleFileName+".html";
                findDescriptionAndImages(exampleName, docName);
            }

            categories.append(categoryName);
        }
    }

    if (categories.size() == 0) {
        QMessageBox::warning(this, tr("No Examples Found"),
                             tr("I could not find any Qt examples."),
                                QMessageBox::Cancel, QMessageBox::NoButton);
    } else {
        emit restart();
    }
}

QString Launcher::readCategoryDescription(const QDomNode &parentNode) const
{
    QString text;
    QDomNode node = parentNode.firstChild();

    while (!node.isNull()) {
        text += node.toCDATASection().nodeValue();
        node = node.nextSibling();
    }

    QStringList lines = text.split("\n", QString::KeepEmptyParts);
    QStringList newLines;
    foreach (QString line, lines) {
        if (line.trimmed().size() == 0)
            newLines.append("\n");
        else
            newLines.append(line);
    }

    return newLines.join("");
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
    if (launched)  // Don't launch if one is already out there.
        return;

    launched = QProcess::startDetached(examplePaths[example]);
}

void Launcher::reset()
{
    slideshowTimer->stop();
    disconnect(slideshowTimer, SIGNAL(timeout()), this, 0);
    launched = false;

    if (!currentExample.isEmpty())
        showExamples(currentCategory);
    else if (!currentCategory.isEmpty())
        showCategories();
    else
        showCategories();
}

void Launcher::showCategories()
{
    disconnect(display, SIGNAL(displayEmpty()), this, 0);
    currentCategory = "";
    currentExample = "";
    connect(display, SIGNAL(displayEmpty()), this, SLOT(createCategories()),
            Qt::QueuedConnection);
    display->reset();
}

void Launcher::createCategories()
{
    disconnect(display, SIGNAL(displayEmpty()), this, 0);

    DisplayShape *title = new TitleShape(tr("Qt Examples"), titleFont,
        QPen(QColor("#a6ce39")), QPointF(),
        QSizeF(0.5 * width(), 0.1 * height()));

    title->setPosition(QPointF(width()/2 - title->rect().width()/2,
                               -title->rect().height()));
    title->setMetaData("target", QPointF(title->position().x(), 0.05 * height()));

    display->appendShape(title);

    QFontMetrics buttonMetrics(buttonFont);
    qreal topMargin = 0.1 * height() + title->rect().height();
    qreal space = 0.95*height() - topMargin;
    qreal step = qMin(title->rect().height() / fontRatio,
                      space/qreal(maximumLabels));
    qreal textHeight = fontRatio * step;

    QPointF startPosition = QPointF(0.0, topMargin);
    QSizeF maxSize(0.25 * width(), textHeight);
    qreal maxWidth = 0.0;

    QList<DisplayShape*> newShapes;

    foreach (QString category, categories) {

        DisplayShape *caption = new TitleShape(category, font(), QPen(),
            startPosition, maxSize);
        caption->setPosition(QPointF(-caption->rect().width(),
                                     caption->position().y()));
        caption->setMetaData("target", QPointF(0.05 * width(),
                                               caption->position().y()));

        newShapes.append(caption);

        startPosition += QPointF(0.0, step);
        maxWidth = qMax(maxWidth, caption->rect().width());
    }

    startPosition = QPointF(width(), topMargin);
    qreal extra = (step - textHeight)/4;

    foreach (QString category, categories) {

        QPainterPath path;
        path.setFillRule(Qt::WindingFill);
        path.addRect(-2*extra, -extra, maxWidth + 4*extra, textHeight + 2*extra);

        DisplayShape *background = new PathShape(path,
            QBrush(QColor(240, 240, 240, 255)), Qt::NoPen, startPosition,
            QSizeF(maxWidth + 4*extra, textHeight + 2*extra));

        background->setMetaData("category", category);
        background->setMetaData("target", QPointF(0.05 * width(),
                                                  background->position().y()));
        display->insertShape(0, background);
        startPosition += QPointF(0.0, step);
    }

    foreach (DisplayShape *caption, newShapes) {
        QPointF position = caption->metaData("target").toPointF();
        QSizeF size = caption->rect().size();
        caption->setPosition(
            QPointF(-maxWidth - size.width()/2, position.y()));
        caption->setMetaData("target",
            QPointF(position.x() + (maxWidth - size.width())/2, position.y()));
        display->appendShape(caption);
    }

    qreal leftMargin = 0.075*width() + maxWidth;

    DocumentShape *description = new DocumentShape(mainDescription, font(),
        QPen(QColor(0,0,0,0)), QPointF(leftMargin, topMargin),
        QSizeF(0.9*width() - maxWidth, space));

    description->setMetaData("fade", 10);

    display->appendShape(description);
}

void Launcher::showExamples(const QString &category)
{
    disconnect(display, SIGNAL(displayEmpty()), this, 0);
    currentCategory = category;
    currentExample = "";

    for (int i = 0; i < display->shapesCount(); ++i) {
        DisplayShape *shape = display->shape(i);

        shape->setMetaData("fade", -15);
    }

    QPointF titlePosition = QPointF(0.0, 0.05*height());

    DisplayShape *newTitle = new TitleShape(category, titleFont,
        QPen(Qt::white), titlePosition,
        QSizeF(0.5 * width(), 0.05 * height()));

    newTitle->setPosition(QPointF(-newTitle->rect().width(), titlePosition.y()));
    newTitle->setMetaData("target",
        QPointF(width()/2 - newTitle->rect().width()/2, titlePosition.y()));
    newTitle->setMetaData("fade", 15);

    QPainterPath backgroundPath;
    backgroundPath.addRect(0, -newTitle->rect().height()*0.3,
                           width(), newTitle->rect().height()*1.6);

    DisplayShape *titleBackground = new PathShape(backgroundPath,
        QBrush(QColor("#a6ce39")), Qt::NoPen,
            QPointF(width(), titlePosition.y()),
            backgroundPath.boundingRect().size());

    titleBackground->setMetaData("target", QPointF(0.0, titlePosition.y()));

    display->insertShape(0, titleBackground);
    display->appendShape(newTitle);

    qreal topMargin = 0.05 * height() + titleBackground->rect().bottom();
    qreal space = 0.95*height() - topMargin;
    qreal step = qMin(newTitle->rect().height() / fontRatio,
                      space/qreal(maximumLabels));
    qreal textHeight = fontRatio * step;

    QPointF startPosition = QPointF(0.05*width(), height() + topMargin);
    QPointF finishPosition = QPointF(0.05*width(), topMargin);
    QSizeF maxSize(0.25 * width(), textHeight);
    qreal maxWidth = 0.0;

    QList<DisplayShape*> newShapes;

    foreach (QString example, examples[currentCategory]) {
        DisplayShape *caption = new TitleShape(example, font(), QPen(),
            startPosition, maxSize);
        caption->setMetaData("target", finishPosition);

        newShapes.append(caption);

        startPosition += QPointF(0.0, step);
        finishPosition += QPointF(0.0, step);
        maxWidth = qMax(maxWidth, caption->rect().width());
    }

    startPosition = QPointF(width(), topMargin);
    qreal extra = (step - textHeight)/4;

    foreach (QString example, examples[currentCategory]) {

        QPainterPath path;
        path.setFillRule(Qt::WindingFill);
        path.addRect(-2*extra, -extra, maxWidth + 4*extra, textHeight + 2*extra);

        DisplayShape *background = new PathShape(path,
            QBrush(QColor(240, 240, 240, 255)), Qt::NoPen, startPosition,
            QSizeF(maxWidth + 4*extra, textHeight + 2*extra));

        background->setMetaData("example", example);
        background->setMetaData("target", QPointF(0.05 * width(),
                                                  background->position().y()));
        display->insertShape(0, background);
        startPosition += QPointF(0.0, step);
    }

    foreach (DisplayShape *caption, newShapes) {
        QPointF position = caption->metaData("target").toPointF();
        QSizeF size = caption->rect().size();
        caption->setPosition(QPointF(
            position.x() + (maxWidth - size.width())/2,
            caption->position().y()));
        caption->setMetaData("target",
            QPointF(position.x() + (maxWidth - size.width())/2, position.y()));
        display->appendShape(caption);
    }

    qreal leftMargin = 0.075*width() + maxWidth;

    DocumentShape *description = new DocumentShape(
        categoryDescriptions[currentCategory], font(),
        QPen(QColor(0,0,0,0)), QPointF(leftMargin, topMargin),
        QSizeF(0.9*width() - maxWidth, space));

    description->setMetaData("fade", 10);

    display->appendShape(description);
}

void Launcher::showExampleDocumentation(const QString &example)
{
    disconnect(display, SIGNAL(displayEmpty()), this, 0);
    currentExample = example;

    assistant->showPage(documentPaths[example]);
/*    connect(display, SIGNAL(displayEmpty()),
            this, SLOT(createExampleDocumentation()));
    display->reset();*/
}

void Launcher::showExampleSummary(const QString &example)
{
    disconnect(display, SIGNAL(displayEmpty()), this, 0);
    currentExample = example;

    for (int i = 0; i < display->shapesCount(); ++i) {
        DisplayShape *shape = display->shape(i);

        shape->setMetaData("fade", -15);
    }

    QPointF titlePosition = QPointF(0.0, 0.05*height());

    DisplayShape *newTitle = new TitleShape(example, titleFont,
        QPen(Qt::white), titlePosition,
        QSizeF(0.5 * width(), 0.05 * height()));

    newTitle->setPosition(QPointF(-newTitle->rect().width(), titlePosition.y()));
    newTitle->setMetaData("target",
        QPointF(width()/2 - newTitle->rect().width()/2, titlePosition.y()));
    newTitle->setMetaData("fade", 15);

    QPainterPath backgroundPath;
    backgroundPath.addRect(0, -newTitle->rect().height()*0.3,
                           width(), newTitle->rect().height()*1.6);

    DisplayShape *titleBackground = new PathShape(backgroundPath,
        QBrush(QColor("#a6ce39")), Qt::NoPen,
            QPointF(width(), titlePosition.y()),
            backgroundPath.boundingRect().size());
    titleBackground->setMetaData("target", QPointF(0.0, titlePosition.y()));
    display->insertShape(0, titleBackground);
    display->appendShape(newTitle);

    qreal topMargin = 0.05 * height() + titleBackground->rect().bottom();
    qreal space = 0.8 * height() - topMargin;

    qreal leftMargin = 0.075*width();
    qreal rightMargin = 0.925*width();

    if (exampleDescriptions.contains(example)) {
        DocumentShape *description = new DocumentShape(
            exampleDescriptions[currentExample], font(),
            QPen(QColor(0,0,0,0)), QPointF(leftMargin, topMargin),
            QSizeF(rightMargin-leftMargin, space));

        description->setMetaData("fade", 10);

        description->setPosition(QPointF(description->position().x(),
            0.8*height() - description->rect().height()));

        display->appendShape(description);
        space = description->position().y() - topMargin - 0.05 * height();
    }

    if (imagePaths.contains(example)) {

        QImage image(imagePaths[example][0]);

        QSizeF imageMaxSize = QSizeF(0.8*width(), space);

        currentFrame = new ImageShape(image,
            QPointF(width() - imageMaxSize.width()/2, topMargin),
            imageMaxSize);

        currentFrame->setMetaData("fade", 15);
        currentFrame->setMetaData("target", QPointF(
            width()/2 - imageMaxSize.width()/2, topMargin));

        display->appendShape(currentFrame);

        if (imagePaths[example].size() > 1) {
            connect(slideshowTimer, SIGNAL(timeout()),
                    this, SLOT(updateExampleSummary()));

            slideshowFrame = 0;
            slideshowTimer->start(5000);
        }
    }

    QSizeF maxSize(0.45 * width(), 0.05 * height());
    qreal margin = 0.0;

    if (examplePaths.contains(example)) {

        DisplayShape *launchCaption = new TitleShape(tr("Launch Example"),
            font(), QPen(Qt::white), QPointF(0.0, 0.0), maxSize);
        launchCaption->setPosition(QPointF(
            0.23*width() - launchCaption->rect().width()/2, height()));
        launchCaption->setMetaData("target", QPointF(
            launchCaption->position().x(), 0.85 * height()));

        display->appendShape(launchCaption);

        qreal maxWidth = launchCaption->rect().width();
        qreal textHeight = launchCaption->rect().height();
        qreal extra = (0.075*height() - textHeight)/4;

        QPainterPath path;
        path.setFillRule(Qt::WindingFill);
        path.addRect(-2*extra, -extra, maxWidth + 4*extra, textHeight + 2*extra);

        DisplayShape *background = new PathShape(path,
            QBrush(QColor("#a6ce39")), Qt::NoPen,
            QPointF(0.23*width() - maxWidth/2, height()),
            QSizeF(maxWidth + 4*extra, textHeight + 2*extra));
        background->setMetaData("target", launchCaption->metaData("target"));
        background->setMetaData("launch", example);

        margin = background->rect().left() - 0.05 * width();
        display->insertShape(0, background);
    }

    if (documentPaths.contains(example)) {

        DisplayShape *documentCaption = new TitleShape(tr("Show Documentation"),
            font(), QPen(Qt::white), QPointF(0.0, 0.0), maxSize);
        documentCaption->setPosition(QPointF(
            qMax(qMin(0.77*width() - documentCaption->rect().width()/2,
                      0.95*width() - margin - documentCaption->rect().width()),
                 0.55*width()), height()));
        documentCaption->setMetaData("target", QPointF(
            documentCaption->position().x(), 0.85 * height()));

        display->appendShape(documentCaption);

        qreal maxWidth = documentCaption->rect().width();
        qreal textHeight = documentCaption->rect().height();
        qreal extra = (0.075*height() - textHeight)/4;

        QPainterPath path;
        path.setFillRule(Qt::WindingFill);
        path.addRect(-2*extra, -extra, maxWidth + 4*extra, textHeight + 2*extra);

        DisplayShape *background = new PathShape(path,
            QBrush(QColor("#9c9cff")), Qt::NoPen,
            QPointF(0.77*width() - documentCaption->rect().width()/2,
                    height()),
            QSizeF(maxWidth + 4*extra, textHeight + 2*extra));
        background->setMetaData("target", documentCaption->metaData("target"));
        background->setMetaData("documentation", example);

        display->insertShape(0, background);
    }
}

void Launcher::updateExampleSummary()
{
    if (imagePaths.contains(currentExample)) {

        currentFrame->setMetaData("fade", -15);
        currentFrame->setMetaData("target",
            currentFrame->position() - QPointF(0.5*width(), 0));

        slideshowFrame = (slideshowFrame+1) % imagePaths[currentExample].size();
        QImage image(imagePaths[currentExample][slideshowFrame]);

        QSizeF imageSize = currentFrame->size();
        QPointF imagePosition = QPointF(width() - imageSize.width()/2,
                                        currentFrame->position().y());

        currentFrame = new ImageShape(image, imagePosition, imageSize);
        currentFrame->setMetaData("fade", 15);
        currentFrame->setMetaData("target", QPointF(
            width()/2 - imageSize.width()/2, imagePosition.y()));

        display->appendShape(currentFrame);
    }
}

void Launcher::toggleFullScreen()
{
    connect(display, SIGNAL(displayEmpty()), this, SLOT(resizeWindow()),
            Qt::QueuedConnection);
    display->reset();
}

void Launcher::resizeEvent(QResizeEvent *event)
{
    if (inFullScreenResize) {
        emit windowResized();
        inFullScreenResize = false;
    }
}

void Launcher::resizeWindow()
{
    disconnect(display, SIGNAL(displayEmpty()), this, 0);

    inFullScreenResize = true;

    if (isFullScreen())
        showNormal();
    else
        showFullScreen();
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
