#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>
#include <QTextDocumentFragment>

class QAction;
class QTextDocument;
class QTextEdit;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

public slots:
    void insertList();
    void showListItems();

private:
    QString currentFile;
    QTextEdit *editor;
    QTextDocument *document;
};

#endif
