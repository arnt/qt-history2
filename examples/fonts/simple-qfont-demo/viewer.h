/* $Id$ */

#ifndef VIEWER_H
#define VIEWER_H     


#include <qwidget.h>
#include <qfont.h>

class QTextView;
class QPushButton;

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
    void layout();
    void showFontInfo( QFont & );

    QTextView * textView; 
    QTextView * fontInfo;

    QPushButton * defaultButton;
    QPushButton * sansSerifButton;
    QPushButton * italicsButton;
};

#endif
