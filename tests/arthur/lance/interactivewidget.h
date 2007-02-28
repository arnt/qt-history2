#ifndef INTERACTIVEWIDGET_H
#define INTERACTIVEWIDGET_H

#include "widgets.h"
#include "paintcommands.h"

#include <QWidget>
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

class QToolBox;

class InteractiveWidget : public QWidget
{
    Q_OBJECT
public:
    InteractiveWidget();

public slots:
    void run();
    void load();
    void load(const QString &fname);
    void save();

protected:
    bool eventFilter(QObject *o, QEvent *e);

protected slots:
    void cmdSelected(QListWidgetItem *item);
    void enumSelected(QListWidgetItem *item);

private:
    QToolBox *m_commandsToolBox;
    QToolBox *m_enumsToolBox;
    OnScreenWidget<QWidget> *m_onScreenWidget;
    QTextEdit *ui_textEdit;
    QString m_filename;
};

#endif
