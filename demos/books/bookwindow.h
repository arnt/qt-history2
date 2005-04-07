#ifndef BOOKWINDOW_H
#define BOOKWINDOW_H

#include <QtGui>
#include <QtSql>

#include "ui_bookwindow.h"


class BookWindow: public QMainWindow
{
    Q_OBJECT
public:
    BookWindow();

private slots:
    void currentBookChanged(const QModelIndex &index);
    void on_authorEdit_activated(const QString &text);
    void on_genreEdit_activated(const QString &text);
    void on_titleEdit_textChanged(const QString &text);
    void on_yearEdit_valueChanged(int value);

private:
    void showError(const QSqlError &err);
    Ui::BookWindow ui;
    QSqlRelationalTableModel *model;
    int authorIdx, genreIdx;
};

#endif

