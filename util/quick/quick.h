#ifndef QUICK_H
#define QUICK_H

#include <qwidget.h>
#include "quickpad.h"

class QuickEditedWidget;
class QTLWidget;

class Quick : public QWidget {
    Q_OBJECT
public:
    Quick(QWidget* parent=0, const char* name=0, WFlags f=0);
    ~Quick();
    void open(const char*, bool complain);
    void saveAs(const char*, bool overwrite);

public slots:
    void closeWindow();

protected:
    void closeEvent( QCloseEvent* );

private: // functions
    bool deleteOkay();
    static QString nextUntitled();
    void setDetailedCaption(const char* filename);
    void setFileName(const char*);

private: // members
    QTLWidget *border;
    QuickEditedWidget *edited_widget;
    QString filename;
    bool changed;

signals:
    void newQuick();
    void closeAll();

private slots:
    void flagChange();
    void open();
    void save();
    void saveAs();
    void dragWidget(QWidgetFactory&);
};

#endif
