#ifndef ARGHINTWIDGET_H
#define ARGHINTWIDGET_H

#include <qframe.h>

class QLabel;
class ArrowButton;

class ArgHintWidget : public QFrame
{
    Q_OBJECT

public:
    ArgHintWidget( QWidget *parent, const char*name );

    void setFunctionText( int func, const QString &text );
    void setNumFunctions( int num );

public slots:
    void relayout();
    void gotoPrev();
    void gotoNext();

private:
    void updateState();

private:
    int curFunc;
    int numFuncs;
    QMap<int, QString> funcs;
    QLabel *funcLabel;
    ArrowButton *prev, *next;

};

#endif
