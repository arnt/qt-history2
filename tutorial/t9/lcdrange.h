/****************************************************************
**
** Definition of LCDRange class, Qt tutorial 9
**
****************************************************************/

#ifndef LCDRANGE_H
#define LCDRANGE_H

#include <qwidget.h>

class QScrollBar;


class LCDRange : public QWidget
{
    Q_OBJECT
public:
    LCDRange( QWidget *parent=0, const char *name=0 );

    int value() const;
public slots:
    void setValue( int );
    void setRange( int minVal, int maxVal );
signals:
    void valueChanged( int );
private:
    QScrollBar  *sBar;
};

#endif // LCDRANGE_H
