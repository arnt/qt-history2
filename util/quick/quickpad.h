#ifndef QUICKPAD_H
#define QUICKPAD_H

#include <qtabdialog.h>

class QWidgetFactory {
public:
    virtual QWidget* instance(QWidget *parent, const char *name=0)=0;
};

class QuickPad : public QTabDialog {
    Q_OBJECT
public:
    QuickPad(QWidget *parent=0, const char *name=0, bool modal=FALSE,
             WFlags f=0 );
    ~QuickPad();
    void addTab( QWidget* child, QPixmap pm, const char* text );
    void addTab( QWidget* child, const char* text );

signals:
    void pulled(QWidgetFactory&);

public slots:
    void dropWidget(QWidget*);

private slots:
    void pullWidgetClassed( const char* );
};

#endif
