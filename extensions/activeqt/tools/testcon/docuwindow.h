#ifndef DOCUWINDOW_H
#define DOCUWINDOW_H

#include <qmainwindow.h>

class QTextBrowser;

class DocuWindow : public QMainWindow
{
    Q_OBJECT
public:
    DocuWindow( const QString& docu, QWidget *parent, QWidget *source );

public slots:
    void save();
    void print();

private:
    QTextBrowser *browser;
};

#endif // DOCUWINDOW_H
