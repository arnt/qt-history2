/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <private/qtextengine_p.h>

#include "bidireorderstring.h"


//TESTED_CLASS=
//TESTED_FILES=gui/widgets/qcombobox.h gui/widgets/qcombobox.cpp

class tst_QComplexText : public QObject
{
Q_OBJECT

public:
    tst_QComplexText();
    virtual ~tst_QComplexText();


public slots:
    void init();
    void cleanup();
private slots:
    void bidiReorderString_data();
    void bidiReorderString();
};

tst_QComplexText::tst_QComplexText()
{
}

tst_QComplexText::~tst_QComplexText()
{

}

void tst_QComplexText::init()
{
// This will be executed immediately before each test is run.
// TODO: Add initialization code here.
}

void tst_QComplexText::cleanup()
{
// This will be executed immediately after each test is run.
// TODO: Add cleanup code here.
}

void tst_QComplexText::bidiReorderString_data()
{
    QTest::addColumn<QString>("logical");
    QTest::addColumn<QString>("VISUAL");
    QTest::addColumn<int>("basicDir");

    const LV *data = logical_visual;
    while ( data->name ) {
	//next we fill it with data
	QTest::newRow( data->name )
	    << QString::fromUtf8( data->logical )
	    << QString::fromUtf8( data->visual )
	    << (int) data->basicDir;
	data++;
    }
}

void tst_QComplexText::bidiReorderString()
{
    QFETCH( QString, logical );
    QFETCH( int,  basicDir );

    // replace \n with Unicode newline. The new algorithm ignores \n
    logical.replace(QChar('\n'), QChar(0x2028));

    QTextEngine e(logical, QFont());
    e.option.setTextDirection((QChar::Direction)basicDir == QChar::DirL ? Qt::LeftToRight : Qt::RightToLeft);
    e.itemize();
    quint8 levels[256];
    int visualOrder[256];
    int nitems = e.layoutData->items.size();
    int i;
    for (i = 0; i < nitems; ++i) {
	//qDebug("item %d bidiLevel=%d", i,  e.items[i].analysis.bidiLevel);
	levels[i] = e.layoutData->items[i].analysis.bidiLevel;
    }
    e.bidiReorder(nitems, levels, visualOrder);

    QString visual;
    for (i = 0; i < nitems; ++i) {
	QScriptItem &si = e.layoutData->items[visualOrder[i]];
	QString sub = logical.mid(si.position, e.length(visualOrder[i]));
	if (si.analysis.bidiLevel % 2) {
	    // reverse sub
	    QChar *a = (QChar *)sub.unicode();
	    QChar *b = a + sub.length() - 1;
	    while (a < b) {
		QChar tmp = *a;
		*a = *b;
		*b = tmp;
		++a;
		--b;
	    }
	    a = (QChar *)sub.unicode();
	    b = a + sub.length();
	    while (a<b) {
		*a = a->mirroredChar();
		++a;
	    }
	}
	visual += sub;
    }
    // replace Unicode newline back with  \n to compare.
    visual.replace(QChar(0x2028), QChar('\n'));

    QTEST(visual, "VISUAL");
}


QTEST_MAIN(tst_QComplexText)
#include "tst_qcomplextext.moc"
