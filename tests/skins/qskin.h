#ifndef QSKIN_H
#define QSKIN_H

#include <qwindowsstyle.h>
#include <qtextstream.h>
#include <qlayout.h>
#include <qptrlist.h>
#include <qdial.h>
#include <qpainter.h>

class QSkinStylePrivate;

class QSkinStyle : public QWindowsStyle
{
    Q_OBJECT

public:
    QSkinStyle();
    QSkinStyle( QTextStream &desc );
    ~QSkinStyle();

    void setSkin( QTextStream &desc );

    QString skinName();
    
    bool defined(const QWidget *) const;
    bool defined(const QLayout *) const;
    bool defined(const QLayout *, const QWidget *) const;
    QRect getGeometry(const QWidget *) const;
    QRect getGeometry(const QLayout *) const;
    QRect getGeometry(const QLayout *, const QWidget *) const;

    int countImages(const QWidget *) const;
    QPixmap image(const QWidget *, const QString &) const;
    void drawImage(QPainter *p, const QWidget *, const QString &) const;

#if 0
    void drawPrimitive(PrimitiveElement pe,
	    QPainter *p,
	    const QRect &r,
	    const QColorGroup &cg,
	    SFlags flags = Style_Default,
	    QStyleOption& = QStyleOption::Default ) const;
#endif

    void drawControl(ControlElement element,
	    QPainter *p,
	    const QWidget *wiget,
	    const QRect &r,
	    const QColorGroup &cg,
	    SFlags flags = Style_Default,
	    const QStyleOption& = QStyleOption::Default ) const;

    void drawComplexControl(ComplexControl element,
	    QPainter *p,
	    const QWidget *widet,
	    const QRect &r,
	    const QColorGroup &cg,
	    SFlags flags = Style_Default,
	    SCFlags sub = SC_All,
	    SCFlags subActive = SC_None,
	    const QStyleOption& = QStyleOption::Default ) const;

    int pixelMetric( PixelMetric, const QWidget *widget = 0) const;
    
    QRect querySubControlMetrics( ComplexControl, 
	    			  const QWidget *widget,
				  SubControl sc,
				  const QStyleOption& = QStyleOption::Default) const;

    SubControl querySubControl( ComplexControl,
	    			const QWidget *widget,
				const QPoint &pos,
				const QStyleOption& = QStyleOption::Default) const;

    void polish(QWidget *);
    void polish(QApplication *);
private:
    // disable copy 
    QSkinStyle( const QSkinStyle & );
    QSkinStyle& operator=( const QSkinStyle & );

private:
    bool eventFilter(QObject *o, QEvent *e);

    QSkinStylePrivate *d;
};

class QSkinLayout : public QLayout
{
public:
    QSkinLayout( QWidget *parent, const char *name = 0);
    QSkinLayout( QLayout *parent, const char *name = 0);

    ~QSkinLayout();

    void addItem(QLayoutItem *item);
    QSize sizeHint() const;
    QSize minimumSize() const;
    QLayoutIterator iterator();
    void setGeometry(const QRect &rect);
private:
    QPtrList<QLayoutItem> list;
};

/****************************
 * Below are classes for widgets that just won't give up their draw
 * functions or perhaps need to bend in a way that was never intended.
 ***************************/

class QSkinDial : public QDial
{
    Q_OBJECT

public:
    QSkinDial(QWidget* parent = 0, const char *name = 0, WFlags f = 0)
	: QDial(parent, name, f) {}
    ~QSkinDial() {}

protected:
    virtual void repaintScreen( const QRect *cr = 0 );
};

#endif
