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

#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <QDir>
#include <QDomNode>
#include <QMap>
#include <QMainWindow>
#include <QString>
#include <QStringList>

class DisplayShape;
class DisplayWidget;
class QAssistantClient;

class Launcher : public QMainWindow
{
    Q_OBJECT

public:
    Launcher(QWidget *parent = 0);

public slots:
    void createCategories();
    void launchExample(const QString &example);
    void reset();
    void showCategories();
    void showExampleDocumentation(const QString &example);
    void showExamples(const QString &category);
    void showExampleSummary(const QString &example);
    void updateExampleSummary();

protected:
    void resizeEvent(QResizeEvent *event);

signals:
    void restart();
    void windowResized();

private slots:
    void toggleFullScreen();
    void redisplayWindow();
    void resizeWindow();

private:
    QString readCategoryDescription(const QDomNode &parentNode) const;
    QString readExampleDescription(const QDomNode &parentNode) const;
    void findDescriptionAndImages(const QString &exampleName,
                                  const QString &docName);
    void findResources();
    void loadExampleInfo();

    bool launched;
    bool inFullScreenResize;
    int maximumLabels;
    int slideshowFrame;
    qreal fontRatio;
    DisplayShape *currentFrame;
    DisplayWidget *display;
    QAssistantClient *assistant;
    QDir documentationDir;
    QDir imagesDir;
    QDir examplesDir;
    QFont buttonFont;
    QFont textFont;
    QFont titleFont;
    QMap<QString,QString> categoryDescriptions;
    QMap<QString,QString> exampleDescriptions;
    QMap<QString,QString> documentPaths;
    QMap<QString,QString> examplePaths;
    QMap<QString,QStringList> examples;
    QMap<QString,QStringList> imagePaths;
    QString currentCategory;
    QString currentExample;
    QString mainDescription;
    QStringList categories;
    QStringList examplePieces;
    QTimer *slideshowTimer;
};

#endif
