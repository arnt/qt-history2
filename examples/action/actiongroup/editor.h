#ifndef EDITOR_H
#define EDITOR_H

#include <qmainwindow.h>


class QTextEdit;
class QAction;


class Editor : public QMainWindow
{
    Q_OBJECT

public:
    Editor();

private slots:
    void setFontColor( QAction * );

private:
    QTextEdit * editor;
    QAction * setRedFont;
};

#endif

