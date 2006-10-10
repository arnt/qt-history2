#ifndef INTERACTIVEWIDGET_H
#define INTERACTIVEWIDGET_H


#include "widgets.h"
#include "paintcommands.h"

#include <QWidget>
#include <QGLWidget>
#include <QSettings>
#include <QFileInfo>
#include <QPainter>
#include <QPaintEvent>
#include <QListWidgetItem>
#include <QTextEdit>
#include <QHBoxLayout>
#include <QSplitter>
#include <QPushButton>
#include <QFileDialog>
#include <QTextStream>

#include <private/qmath_p.h>


class InteractiveWidget : public QWidget
{
    Q_OBJECT
public:
    InteractiveWidget();

public slots:
    void run();
    void cmdSelected(QListWidgetItem *item);
    void load();
    void load(const QString &fname);
    void save();

protected:
    bool eventFilter(QObject *o, QEvent *e);

private:
    QMap<QString, QString> cmdMap;
    QListWidget *cmds;
    OnScreenWidget<QWidget> *osw;
    QTextEdit *te;
    QString filename;
};


#endif
