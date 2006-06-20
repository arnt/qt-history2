/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <qpolygon.h>
#include <qpainterpath.h>
#include <math.h>

#include <qpainter.h>
#include <qdialog.h>



//TESTED_CLASS=
//TESTED_FILES=gui/painting/qpolygon.h gui/painting/qpolygon.cpp

class tst_QPolygon : public QObject
{
    Q_OBJECT

public:
    tst_QPolygon();

private slots:
    void makeEllipse();
};

tst_QPolygon::tst_QPolygon()
{
}

void tst_QPolygon::makeEllipse()
{
    // create an ellipse with R1 = R2 = R, i.e. a circle
    QPolygon pa;
    const int R = 50; // radius
    QPainterPath path;
    path.addEllipse(0, 0, 2*R, 2*R);
    pa = path.toSubpathPolygons().at(0).toPolygon();

    int i;
    // make sure that all points are R+-1 away from the center
    bool err = FALSE;
    for (i = 1; i < pa.size(); i++) {
	QPoint p = pa.at( i );
	double r = sqrt( pow( double(p.x() - R), 2.0 ) + pow( double(p.y() - R), 2.0 ) );
	// ### too strict ? at least from visual inspection it looks
	// quite buggy around the main axes. 2.0 passes easily.
	err |= ( qAbs( r - double(R) ) > 2.0 );
    }
    QVERIFY( !err );
}

QTEST_APPLESS_MAIN(tst_QPolygon)
#include "tst_qpointarray.moc"
