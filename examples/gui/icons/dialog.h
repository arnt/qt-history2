#ifndef DIALOG_H
#define DIALOG_H

#include <QtGui>

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = 0);

    void setIconFile(const QString fileName) { m_fileName = fileName; }
    QString iconFile() const { return m_fileName; }

protected:
    void paintEvent(QPaintEvent *);

private slots:
    void chooseIcon();

private:
    QString m_fileName;
};

#endif
