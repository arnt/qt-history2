/* $Id$ */

#ifndef VIEWER_H
#define VIEWER_H     


#include <qwidget.h>

class QTextView;

class Viewer : public QWidget
{
Q_OBJECT

public:   
    Viewer();

private slots:
    void setDefault();
    void setSansSerif();
    void setItalics();

private:
    void setFontSubstitutions();
    void showFontInfo( QFont & );

    QTextView * textView; 
    QTextView * fontInfo;
};

#endif
