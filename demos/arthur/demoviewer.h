#ifndef DEMOVIEWER_H
#define DEMOVIEWER_H

#include <qhash.h>
#include <qwidget.h>

class Attributes;
class DemoWidget;
class QCheckBox;
class QComboBox;
class QGenericListView;
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
    QGenericListView *listView;
    QStackedBox *widgets;
    QHash<QString, DemoWidget *> widgetByName;
    Attributes *attributes;

    QCheckBox *antialias;
    QCheckBox *alpha;
    QComboBox *bgMode;
};

#endif // DEMOVIEWER_H
