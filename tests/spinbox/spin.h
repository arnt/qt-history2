/****************************************************************************
**
** Definition of .
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** EDITIONS: UNKNOWN
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef SPIN_H
#define SPIN_H

#include <qdialog.h>
class QSpinBox;
class QCheckBox;
class QLineEdit;
class QLabel;
class QSlider;

class Main : public QDialog {
    Q_OBJECT
public:
    Main(QWidget* parent=0, const char* name=0, int f=0);
	
    QSlider* tslider;

    QSpinBox* mainBox;
    QSpinBox* decBox;
    QSpinBox* stepBox;
    QSpinBox* minBox;
    QSpinBox* maxBox;
    QSpinBox* valBox;
    
    QCheckBox* wrapCheck;
    QCheckBox* palCheck;
    QCheckBox* disableCheck;
    QCheckBox* styleCheck;
    QCheckBox* symbolCheck;
    QLineEdit* suffixEd;
    QLineEdit* prefixEd;
    QLineEdit* minTxtEd;
    QLabel* textLb;

public slots:

    void updateValue( int i );
    void updateWrap();
    void updateStep();
    void updateRange();
    void updatePalette();
    void updateDisabled();
    void updateStyle();
    void updateSymbols();
    void updateSpecValTxt( const QString& s );
    void showValue( int i );

};

class MainParent : public QWidget {
    Q_OBJECT
public:
    MainParent( QWidget* parent = 0, const char* name = 0, int f = 0 );
protected:
    void mousePressEvent( QMouseEvent *e );
};

#endif
