#ifndef DIALOG_H
#define DIALOG_H

#include <QtGui>

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = 0);

    void setIconFile(const QString filename) { m_filename = filename; }
    const QString& iconFile() const { return m_filename; }

protected:
    void paintEvent(QPaintEvent *);

private slots:
    void chooseIcon();

private:
    QString m_filename;
};

#endif
