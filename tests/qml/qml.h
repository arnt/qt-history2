/****************************************************************************
 ** $Id: //depot/qt/main/tests/qml/qml.h#16 $
 **
 ** Definition of something or other
 **
 ** Created : 979899
 **
 ** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
 **
****************************************************************************/

#ifndef QML_H
#define QML_H


#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlist.h>
#include <qdict.h>
#include <qpixmap.h>
#include <qscrollview.h>
#include <qtimer.h>
#include <qcolor.h>

class QMLStyle
{
public:
    QMLStyle( const QString& name );
    ~QMLStyle();

    static const QMLStyle& nullStyle();

    QString name() const;

    enum Display {display_block, display_inline, display_list_item, display_none};
    Display display() const;
    void setDisplay(Display d);

    enum FontStyle {style_undefined, style_normal, style_italic, style_oblique};
    FontStyle fontStyle() const;
    void setFontStyle(FontStyle s);

    enum FontWeight {weight_undefined = -1,
		     weight_normal = QFont::Normal,
		     weight_bold = QFont::Bold,
		     weight_bolder = QFont::Black,
		     weight_lighter = QFont::Light};

    int fontWeight() const;
    void setFontWeight(int w);

    int fontSize() const;
    void setFontSize(int s);

    int numberOfColumns() const;
    void setNumberOfColumns(int ncols);

    QColor color( const QColor & ) const;
    void setColor( const QColor &);
    inline bool hasColor() const { return col != 0; }

    bool isActive() const;
    void setActive(bool act);


private:
    void init();
    Display disp;
    FontStyle fontstyle;
    int fontweight;
    int fontsize;
    QMLStyle *parentstyle;
    QString stylename;
    int ncolumns;
    QColor* col;
    bool active;
};

class QMLProvider
{
public:
    QMLProvider();
    virtual ~QMLProvider();

    static QMLProvider& defaultProvider();


    virtual QPixmap image(const QString &name) const;
    virtual QString document(const QString &name) const;

    virtual void setImage(const QString& name, const QPixmap& pm);
    virtual void setDocument(const QString& name, const QString& contents);

    virtual void setPath( const QString &path );
    QString path() const;
    
    // TODO add nifty pixmap cache stuff

private:
    QDict<QPixmap>images;
    QDict<QString>documents;
    QString searchPath;
};

class QMLNode;
class QMLContainer;
class QMLStyleSheet
{
public:
    QMLStyleSheet();
    virtual ~QMLStyleSheet();

    QMLStyle& defaultStyle() const;
    static QMLStyleSheet& defaultSheet();

    void insert( QMLStyle* style);
    const QMLStyle& style(const char* name) const;

    virtual QMLContainer* tag( const QMLStyle &,
			       const QDict<QString>&attr,
			       const QMLProvider& provider ) const;

    virtual QMLNode* emptyTag( const QMLStyle &,
			       const QDict<QString>&attr,
			       const QMLProvider& provider ) const;

private:
    void init();
    QDict <QMLStyle> styles;
    QMLStyle* defaultstyle;

};


class QMLContainer;
class QMLBox;

class QMLNode
{
public:
    QMLNode();
    ~QMLNode();
    QMLNode* next;

    QMLNode* depthFirstSearch(QMLNode* tag, QMLContainer* &parent, bool down = TRUE);
    QMLNode* nextLayout(QMLNode* tag, QMLContainer* &parent);
    QMLNode* nextLeaf(QMLNode* tag, QMLContainer* &parent);

    QChar c;

    inline bool isSpace() const {return c.isSpace();}
    inline bool isNull() const {return c == QChar::null;}

    QMLContainer* parent() const;
    QMLBox* box() const;
    QMLNode* previousSibling() const;
    QMLNode* lastSibling() const;
    QMLNode* nextSibling() const;

    uint isSimpleNode: 1;
    uint isLastSibling:1;
    uint isContainer:1;
    uint isBox:1;
    uint isSelected: 1;
    uint isSelectionDirty: 1;
};

class QMLCustomNode : public QMLNode
{
public:
    QMLCustomNode();
    virtual ~QMLCustomNode();

    virtual void draw(QPainter* p, int x, int y,
		      int ox, int oy, int cx, int cy, int cw, int ch) = 0;

    int width;
    int height;
};

class QMLImage : public QMLCustomNode
{
public:
    QMLImage(const QDict<QString>&attr, const QMLProvider& provider);
    ~QMLImage();

    void draw(QPainter* p, int x, int y,
	      int ox, int oy, int cx, int cy, int cw, int ch);
private:
    QPixmap pm;
};


// internal class for qmlbox, also used in qmlcursor.
class QMLRow
{
public:
    QMLRow();
    QMLRow(QMLContainer* box, QPainter* p, QMLNode* &t, QMLContainer* &par, int w);
    ~QMLRow();
    int x;
    int y;
    int width;
    int height;
    int base;
    bool intersects(int xr, int yr, int wr, int hr);
    void draw(QMLContainer* box, QPainter* p, int obx, int oby, int ox, int oy, int cx, int cy, int cw, int ch,
	      QRegion& backgroundRegion, const QColorGroup& cg,
	      bool onlyDirty = FALSE, bool onlySelection = FALSE);
    QMLNode* hitTest(QMLContainer* box, QPainter* p, int obx, int oby, int xarg, int yarg);


    bool locate(QMLContainer* box, QPainter* p, QMLNode* node, int &lx, int &ly, int &lh);

    bool dirty;

    QMLNode* start;
    QMLNode* end;
    QMLContainer* parent;

};


inline bool QMLRow::intersects(int xr, int yr, int wr, int hr)
{
    return ( QMAX( x, xr ) <= QMIN( x+width, xr+wr ) &&
	     QMAX( y, yr ) <= QMIN( y+height, yr+hr ) );

}

class QMLContainer : public QMLNode
{
public:
    QMLContainer( const QMLStyle &stl);
    QMLContainer( const QMLStyle &stl, const QDict<QString>& attr );
    virtual ~QMLContainer();
    inline QFont font() const;
    inline QColor color(const QColor&) const;
    QMLContainer* parent;
    const QMLStyle* style;
    QMLNode* child;

    QMLBox* box() const;
    QMLBox* parentBox() const;

    QMLNode* lastChild() const;

    void reparentSubtree();

    virtual QMLContainer* copy() const;

    void split(QMLNode* node);

    const QDict<QString>& attributes() const;

    const QMLContainer* activeContainer() const;

protected:
    void setAttributes(const QDict<QString>& attr );

private:
    int fontWeight() const;
    QMLStyle::FontStyle fontStyle() const;
    int fontSize() const;

    void createFont();

    QFont* fnt;
    QDict<QString>* attributes_;
};


inline QFont QMLContainer::font() const
{
    if (!fnt) {
	QMLContainer* that = (QMLContainer*) this;
	that->createFont();
    }
    return *fnt;
}


inline QColor QMLContainer::color(const QColor& c) const
{
    if (style->hasColor())
	return style->color(c);
    return parent?parent->color(c):c;
}

class QMLBox : public QMLContainer
{
public:
    QMLBox( const QMLStyle &stl);
    QMLBox( const QMLStyle &stl, const QDict<QString>& attr );
    ~QMLBox();

    void draw(QPainter* p, int obx, int oby, int ox, int oy, int cx, int cy, int cw, int ch,
	      QRegion& backgroundRegion, const QColorGroup& cg, bool onlyDirty = FALSE, bool onlySelection = FALSE);
    void resize (QPainter* p, int newWidth, bool forceResize = FALSE);

    void update(QPainter* p, QMLRow* r = 0);

    QMLContainer* copy() const;

    QList<QMLRow> rows;

    int width;
    int height;

    //    QMLNode* locate(int x, int y);
    QMLRow*  locate(QPainter* p, QMLNode* node, int &lx, int &ly, int &lh, int&lry, int &lrh);

    QMLNode* hitTest(QPainter* p, int obx, int oby, int xarg, int yarg);

};



class QMLDocument : public QMLBox
{
public:
    QMLDocument( const QString &doc);
    QMLDocument( const QString &doc, const QMLProvider& provider);
    QMLDocument( const QString &doc,  const QMLProvider& provider, const QMLStyleSheet& sheet);
    ~QMLDocument();


    bool isValid() const;

    void dump();

    static QString firstTag( const QString& doc);

private:
    void init( const QString& doc );

    void parse (QMLContainer* current, QMLNode* lastChild, const QString& doc, int& pos);
    bool eatSpace(const QString& doc, int& pos);
    bool eat(const QString& doc, int& pos, const QChar& c);
    bool lookAhead(const QString& doc, int& pos, const QChar& c);
    QString parseOpenTag(const QString& doc, int& pos, QDict<QString> &attr);
    bool eatCloseTag(const QString& doc, int& pos, const QString& open);
    QString parseWord(const QString& doc, int& pos, bool lower = FALSE);
    QString parsePlainText(const QString& doc, int& pos);
    bool hasPrefix(const QString& doc, int pos, const QChar& c);
    bool valid;
    QChar* openChar;
    QChar* closeChar;
    QChar* slashChar;
    QChar* quoteChar;
    QChar* equalChar;
    const QMLStyleSheet* sheet_;
    const QMLProvider* provider_;

};

class QMLCursor;

class QMLView : public QScrollView
{
    Q_OBJECT
public:
    QMLView(QWidget *parent=0, const char *name=0);
    QMLView( const QString& doc, QWidget *parent=0, const char *name=0);
    ~QMLView();

    virtual void setContents( const QString& contents);
    virtual QString contents() const;

    QMLStyleSheet& styleSheet() const;
    void setStyleSheet( QMLStyleSheet* styleSheet );

    void setPaperColorGroup( const QColorGroup& colgrp);
    void setPaperPixmap( const QPixmap& pm);

    const QColorGroup &paperColorGroup() const;

    void setProvider( QMLProvider* newProvider );
    QMLProvider& provider() const;

    QString title() const;

    int heightForWidth( int w ) const;

protected:
    void drawContentsOffset(QPainter*, int ox, int oy,
			    int cx, int cy, int cw, int ch);
    void resizeEvent(QResizeEvent*);
    void viewportMousePressEvent( QMouseEvent* );
    void viewportMouseReleaseEvent( QMouseEvent* );
    void viewportMouseMoveEvent( QMouseEvent* );
    void keyPressEvent( QKeyEvent * );

protected:

    QMLDocument& currentDocument() const;
    void paletteChange( const QPalette & );

private:
    QMLStyleSheet* sheet_;
    QMLDocument* doc_;
    QMLProvider* provider_;
    QString txt;
    QColorGroup mypapcolgrp;
    QColorGroup papcolgrp;
    void init();
    void createDocument();
    void* v1;
    void* d;
};

inline QMLDocument& QMLView::currentDocument() const
{
    if (!doc_){
	QMLView* that = (QMLView*) this;
	that->createDocument();
    }
    return *doc_;
}


class QMLEdit : public QMLView
{
    Q_OBJECT
public:
    QMLEdit(QWidget *parent=0, const char *name=0);
    ~QMLEdit();

    void setContents( const QString& contents );
    QString contents();

protected:
    void drawContentsOffset(QPainter*, int ox, int oy,
			    int cx, int cy, int cw, int ch);
    void viewportMousePressEvent( QMouseEvent* );
    void viewportMouseReleaseEvent( QMouseEvent* );
    void viewportMouseMoveEvent( QMouseEvent* );
    void keyPressEvent( QKeyEvent * );
    void resizeEvent(QResizeEvent*);

    void showCursor();
    void hideCursor();

private slots:
void cursorTimerDone();

private:
    bool cursor_hidden;
    QTimer* cursorTimer;
    QMLCursor* cursor;

    void updateSelection(int oldY=-1, int newY=-1);

    void updateScreen();
    void* d;
};


class QMLBrowser : public QMLView
{
    Q_OBJECT
public:
    QMLBrowser( QWidget *parent=0, const char *name=0 );
    ~QMLBrowser();

    virtual void setDocument(const QString& name);

    void setContents( const QString& contents );


public slots:
void backward();
    void forward();

signals:
    void backwardAvailable( bool );
    void forwardAvailable( bool );
    void highlighted( const QString& );
    void contentsChanged();

protected:
    void viewportMousePressEvent( QMouseEvent* );
    void viewportMouseReleaseEvent( QMouseEvent* );
    void viewportMouseMoveEvent( QMouseEvent* );

private:
    void popupDefinition( const QString& contents, const QPoint& pos );

    QString searchPath;
    uint goBackwards : 1;
    void* d;
    const QMLContainer* activeContainer(const QPoint& pos);
    const QMLContainer* buttonDown;
    const QMLContainer* highlight;
    QPoint lastClick;
};

/*
class QHelp : public QDialog
{

};
*/

/*!
  depthFirstSearch traversal for the tag tree
 */
inline QMLNode* QMLNode::depthFirstSearch(QMLNode* tag, QMLContainer* &parent, bool down)
{
    if (down) {
	if (tag->isContainer && ((QMLContainer*)tag)->child){
	    parent = (QMLContainer*)tag;
	    return ((QMLContainer*)tag)->child;
	}
	//  	return depthFirstSearch(tag, parent, FALSE);
    }
    //      else
    {
	if (tag == this){
	    return 0;
	}
	if (!tag->isLastSibling && tag->next){
	    return tag->next;
	}
	QMLContainer* p = (QMLContainer*)tag->next;
	if (p){
	    parent = p->parent;
	    return depthFirstSearch(p, parent, FALSE);
	}
    }
    return 0;
}


/*!
  extends the depthFirstSearch traversal so that only tags that include a layout are
  returned
*/

inline QMLNode* QMLNode::nextLayout(QMLNode* tag, QMLContainer* &parent){
    QMLNode* t;

    if (tag != this && tag->isBox)
	t = depthFirstSearch(tag, parent, FALSE);
    else
	t = depthFirstSearch(tag, parent);
    if (t) {
	if (t->isContainer && !t->isBox)
	    return nextLayout(t, parent);
    }
    return t;
}

inline QMLNode* QMLNode::nextLeaf(QMLNode* tag, QMLContainer* &parent){
    do {
	tag = depthFirstSearch(tag, parent);

    } while (tag && tag->isContainer);

    return tag;
}



inline QMLNode* QMLNode::lastSibling() const
{
    QMLNode* n = (QMLNode*) this;

    while (n && !n->isLastSibling)
	n = n->next;
    return n;
}

inline QMLContainer* QMLNode::parent() const
{
    if (isContainer)
	return ((QMLContainer*)this)->parent;
    else {
	QMLNode* n = lastSibling();
	if (n) return (QMLContainer*)n->next;
    }
    return 0;
}

inline QMLBox* QMLNode::box() const
{
    QMLContainer* par = parent();
    if (!par)
	return 0;
    else
	return par->box();
}

inline QMLNode* QMLNode::previousSibling() const
{
    QMLContainer* par = parent();
    QMLNode* result = par->child;
    if (result == this)
	return 0;
    while (result->next && result->next != this)
	result = result->next;
    return result;
}


inline QMLNode* QMLNode::nextSibling() const
{
    if (isLastSibling)
	return 0;
    return next;
}


#endif
