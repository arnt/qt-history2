#ifndef QTGRADIENTDIALOG_H
#define QTGRADIENTDIALOG_H

#include <QDialog>

class QtGradientDialog : public QDialog
{
    Q_OBJECT
public:
    QtGradientDialog(QWidget *parent = 0);
    ~QtGradientDialog();

    void setGradient(const QGradient &gradient);
    QGradient gradient() const;

private:
    class QtGradientDialogPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QtGradientDialog)
    Q_DISABLE_COPY(QtGradientDialog)
};

#endif
