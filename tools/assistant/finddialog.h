#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include "ui_finddialog.h"

class MainWindow;
class QStatusBar;
class QTextBrowser;

class FindDialog : public QDialog
{
    Q_OBJECT
public:
    FindDialog(MainWindow *parent);
    virtual ~FindDialog();

    MainWindow *mainWindow() const;
    bool hasFindExpression() const;

public slots:
    void reset();
    void doFind(bool forward);
    void statusMessage(const QString &message);

private slots:
    void on_findButton_clicked();
    void on_closeButton_clicked();

private:
    Ui::FindDialog gui;

    QStatusBar *sb;
    bool onceFound;
    QString findExpr;
    QTextBrowser *lastBrowser;
};

#endif

