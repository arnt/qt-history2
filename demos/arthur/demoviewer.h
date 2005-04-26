/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef DEMOVIEWER_H
#define DEMOVIEWER_H

#include <qhash.h>
#include <qwidget.h>

class Attributes;
class DemoWidget;
class QCheckBox;
class QComboBox;
class QListWidget;
class QPushButton;
class QStackedWidget;
class QTextEdit;

class DemoViewer : public QWidget
{
    Q_OBJECT
public:
    DemoViewer(QWidget *parent = 0);
    ~DemoViewer();

    void addDemoWidget(const QString &name, DemoWidget *demoWidget, const QString &file);

    QSize sizeHint() const;

    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);

public slots:
    void itemSelected();
    void antialiasChanged(bool);
    void alphaChanged(bool);
    void fillModeChanged(int);
    void openSource(bool);

private:
    QListWidget *listWidget;
    QStackedWidget *widgets;
    QHash<QString, DemoWidget *> widgetByName;
    QHash<QString, QString> fileByName;
    Attributes *attributes;
    QTextEdit *sourceViewer;
    QPushButton *viewSourceButton;

    QCheckBox *antialias;
    QCheckBox *alpha;
    QComboBox *bgMode;
};

#endif
