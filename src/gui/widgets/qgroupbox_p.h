#ifndef QGROUPBOX_P_H
#define QGROUPBOX_P_H

#include "qframe_p.h"

class QCheckBox;
class QSpacerItem;
class QVBoxLayout;
class QGridLayout;

class QGroupBoxPrivate : public QFramePrivate
{
public:
    Q_DECL_PUBLIC(QGroupBox);

    QGroupBoxPrivate():
	spacer( 0 ),
	checkbox( 0 ) {}

    void skip();
    void init();
    void calculateFrame();
    void insertWid( QWidget* );
    void setTextSpacer();
#ifndef QT_NO_CHECKBOX
    void updateCheckBoxGeometry();
#endif
    QString str;
    int align;
    int lenvisible;
#ifndef QT_NO_ACCEL
    QAccel * accel;
#endif

    QVBoxLayout *vbox;
    QGridLayout *grid;
    int row;
    int col : 30;
    uint bFlat : 1;
    int nRows, nCols;
    Orientation dir;
    int spac, marg;

    QSpacerItem *spacer;
    QCheckBox *checkbox;
};


#endif
