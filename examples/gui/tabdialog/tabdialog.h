#ifndef TABDIALOG_H
#define TABDIALOG_H

#include <QDialog>
#include <QFileInfo>
#include <QString>
#include <QTabWidget>

class GeneralTab : public QWidget
{
    Q_OBJECT

public:
    GeneralTab(QWidget *parent, const QFileInfo &fileInfo);
};


class PermissionsTab : public QWidget
{
    Q_OBJECT

public:
    PermissionsTab(QWidget *parent, const QFileInfo &fileInfo);
};


class ApplicationsTab : public QWidget
{
    Q_OBJECT

public:
    ApplicationsTab(QWidget *parent, const QFileInfo &fileInfo);
};


class TabDialog : public QDialog
{
    Q_OBJECT

public:
    TabDialog(QWidget *parent, const QString &fileName);

private:
    QTabWidget *tabWidget;
};

#endif
