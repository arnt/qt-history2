#ifndef RAWTEST_H
#define RAWTEST_H

#include <qapplication.h>
#include <qlistbox.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qbuttongroup.h>
#include <qlayout.h>
#include <qdialog.h>
#include <qpushbutton.h>
#include "lcdrange.h"
#include "qt_x11.h"

class RawTest : public QDialog
{
    Q_OBJECT;
public:
    RawTest( QWidget *parent = 0, const char *name = 0 );

private slots:
    void fontSelected( const QString &s );
    void boldToggled( bool on );
    void italicToggled( bool on );
    void underlineToggled( bool on );
    void sizeChanged( int sz );


private:
    void updateSample();

    QListBox *listB;
    QCheckBox *bold;
    QCheckBox *italic;
    QCheckBox *underline;
    
    LCDRange *range;
    QLineEdit *sample;

    QButtonGroup *group;

    QFont font;

};

#endif
