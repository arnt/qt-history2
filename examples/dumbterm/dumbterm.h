#ifndef DUMBTERM_H
#define DUMBTERM_H

#include <qmultilineedit.h>
#include <qsocket.h>

class DumbTerminal : public QMultiLineEdit {
    Q_OBJECT

    int ptyfd;
    QSocket socket;
    int cpid;

public:
    DumbTerminal(QWidget* parent=0, const char* name=0);
    ~DumbTerminal();

    QSize sizeHint() const;

signals:
    void closed();

protected:
    void keyPressEvent(QKeyEvent*);

private slots:
    void error();
    void readPty();
};

#endif
