/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

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

    QTextView * greetings; 
    QTextView * fontInfo;

    QPushButton * defaultButton;
    QPushButton * sansSerifButton;
    QPushButton * italicsButton;
};

#endif
