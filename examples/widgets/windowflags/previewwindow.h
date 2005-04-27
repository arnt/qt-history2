#ifndef PREVIEWWINDOW_H
#define PREVIEWWINDOW_H

#include <QWidget>

class QPushButton;
class QTextEdit;

class PreviewWindow : public QWidget
{
    Q_OBJECT

public:
    PreviewWindow(QWidget *parent = 0);

    void setWindowFlags(Qt::WindowFlags flags);

private:
    QTextEdit *textEdit;
    QPushButton *closeButton;
};

#endif
