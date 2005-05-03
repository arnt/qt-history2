#ifndef VERSIONDIALOG_H
#define VERSIONDIALOG_H

#include <QtGui/QDialog>

class VersionDialog : public QDialog
{
    Q_OBJECT
public:
    VersionDialog(QWidget *parent);
    ~VersionDialog();
};

#endif
