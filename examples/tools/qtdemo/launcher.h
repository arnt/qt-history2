/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <QDir>
#include <QDomNode>
#include <QImage>
#include <QMap>
#include <QMainWindow>
#include <QString>
#include <QStringList>

class DisplayShape;
class DisplayWidget;
class QAssistantClient;
class QProcess;

class Launcher : public QMainWindow
{
    Q_OBJECT

public:
    Launcher(QWidget *parent = 0);
    bool setup();

public slots:
    void enableLaunching();
    void executeAction(const QString &action);
    void launchExample(const QString &uniqueName);
    void showCategories();
    void showExampleDocumentation(const QString &uniqueName);
    void showExamples(const QString &category);
    void showExampleSummary(const QString &uniqueName);
    void showParentPage();
    void updateExampleSummary();

protected:
    void closeEvent(QCloseEvent *event);
    void resizeEvent(QResizeEvent *event);

signals:
    void showPage();
    void windowResized();

private slots:
    void toggleFullScreen();
    void redisplayWindow();
    void resizeWindow();

private:
    DisplayShape *addTitle(const QString &title, qreal verticalMargin);
    DisplayShape *addTitleBackground(DisplayShape *titleShape);

    QString readExampleDescription(const QDomNode &parentNode) const;
    QString findExecutable(const QDir &dir) const;

    int readInfo(const QString &resource, const QDir &dir);

    void addVersionAndCopyright(const QRectF &rect);
    void fadeShapes();
    void findDescriptionAndImages(const QString &exampleName,
                                  const QString &docName);
    void newPage();
    void readCategoryDescription(const QDir &categoryDir,
                                 const QString &categoryName);

    bool inFullScreenResize;
    int maximumLabels;
    int slideshowFrame;
    qreal fontRatio;
    DisplayShape *currentFrame;
    DisplayWidget *display;
    QAssistantClient *assistant;
    QDir documentationDir;
    QDir imagesDir;
    QDir demosDir;
    QDir examplesDir;
    QFont buttonFont;
    QFont documentFont;
    QFont textFont;
    QFont titleFont;
    QImage qtLogo;
    QImage trolltechLogo;
    QMap<QString,QColor> categoryColors;
    QMap<QProcess*,QString> runningProcesses;
    QMap<QString,QString> categoryDescriptions;
    QMap<QString,QMap<QString,QString> > exampleOptions;
    QMap<QString,QMap<QString,QString> > exampleDetails;
    QMap<QString,QStringList> examples;
    QMap<QString,QStringList> imagePaths;
    QString currentCategory;
    QString currentExample;
    QStringList runningExamples;
    QStringList categories;
    QTimer *resizeTimer;
    QTimer *slideshowTimer;
};

#endif
