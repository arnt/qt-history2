#ifndef PRINTPREVIEW_H
#define PRINTPREVIEW_H

#include <QMainWindow>

class PreviewView;
class QTextDocument;

class PrintPreview : public QMainWindow
{
    Q_OBJECT
public:
    PrintPreview(const QTextDocument *document, QWidget *parent);
    virtual ~PrintPreview();

private slots:
    void print();

private:
    QTextDocument *doc;
    PreviewView *view;
};

#endif // PRINTPREVIEW_H

