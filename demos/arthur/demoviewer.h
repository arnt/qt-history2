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
class QListView;
class QStackedBox;

class DemoViewer : public QWidget
{
    Q_OBJECT
public:
    DemoViewer(QWidget *parent = 0);

    void addDemoWidget(const QString &name, DemoWidget *demoWidget);

    QSize sizeHint() const;

    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);

public slots:
    void itemSelected();
    void antialiasChanged(bool);
    void alphaChanged(bool);
    void fillModeChanged(int);

private:
    QListView *listView;
    QStackedBox *widgets;
    QHash<QString, DemoWidget *> widgetByName;
    Attributes *attributes;

    QCheckBox *antialias;
    QCheckBox *alpha;
    QComboBox *bgMode;
};

#endif // DEMOVIEWER_H
