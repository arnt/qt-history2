/**********************************************************************
**   Copyright (C) 2000 Troll Tech AS.  All rights reserved.
**
**   This file is part of Qt GUI Designer.
**
**   This file may be distributed under the terms of the GNU General
**   Public License version 2 as published by the Free Software
**   Foundation and appearing in the file COPYING included in the
**   packaging of this file. If you did not get the file, send email
**   to info@trolltech.com
**
**   The file is provided AS IS with NO WARRANTY OF ANY KIND,
**   INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR
**   A PARTICULAR PURPOSE.
**
**********************************************************************/

#ifndef PROPERTYEDITOR_H
#define PROPERTYEDITOR_H

#include <qvbox.h>
#include <qlistview.h>
#include <qvariant.h>
#include <qlist.h>

class PropertyList;
class PropertyEditor;
class QPainter;
class QColorGroup;
class QComboBox;
class QLineEdit;
class QPushButton;
class QHBox;
class QSpinBox;
class QLabel;
class FormWindow;
class QCloseEvent;
class QResizeEvent;

class PropertyItem : public QListViewItem
{
public:
    PropertyItem( PropertyList *l, PropertyItem *after, PropertyItem *prop, const QString &propName );
    ~PropertyItem();

    void paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int align );
    void paintBranches( QPainter * p, const QColorGroup & cg,
			int w, int y, int h, GUIStyle s );
    void paintFocus( QPainter *p, const QColorGroup &cg, const QRect &r );

    virtual bool hasSubItems() const;
    virtual void createChildren();
    virtual void initChildren();

    bool isOpen() const;
    void setOpen( bool b );

    virtual void showEditor();
    virtual void hideEditor();

    virtual void setValue( const QVariant &v );
    QVariant value() const;
    QString name() const;
    virtual void notifyValueChange();

    virtual void setChanged( bool b, bool updateDb = TRUE );
    bool isChanged() const;

    virtual void placeEditor( QWidget *w );

    virtual PropertyItem *propertyParent() const;
    virtual void childValueChanged( PropertyItem *child );

    void addChild( PropertyItem *i );
    int childCount() const;
    PropertyItem *child( int i ) const;

    virtual bool hasCustomContents() const;
    virtual void drawCustomContents( QPainter *p, const QRect &r );

    void updateBackColor();

    void setup() { QListViewItem::setup(); setHeight( QListViewItem::height() + 2 ); }

    virtual QString currentItem() const;
    virtual int currentIntItem() const;
    virtual void setCurrentItem( const QString &s );
    virtual void setCurrentItem( int i );
    virtual int currentIntItemFromObject() const;
    virtual QString currentItemFromObject() const;

    void setFocus( QWidget *w );
    
protected:
    PropertyList *listview;
    QVariant val;

private:
    QColor backgroundColor();
    void createResetButton();
    void updateResetButtonState();

private:
    bool open, changed;
    PropertyItem *property;
    QString propertyName;
    QList<PropertyItem> children;
    QColor backColor;
    QPushButton *resetButton;

};

class PropertyTextItem : public QObject,
			 public PropertyItem
{
    Q_OBJECT

public:
    PropertyTextItem( PropertyList *l, PropertyItem *after, PropertyItem *prop,
		      const QString &propName, bool comment, bool multiLine, bool ascii = FALSE, bool a = FALSE );
    ~PropertyTextItem();

    virtual void createChildren();
    virtual void initChildren();

    virtual void showEditor();
    virtual void hideEditor();

    virtual void setValue( const QVariant &v );
    virtual bool hasSubItems() const;
    virtual void childValueChanged( PropertyItem *child );

    virtual void setChanged( bool b, bool updateDb = TRUE );

private slots:
    void setValue();
    void getText();

private:
    QLineEdit *lined();
    QLineEdit *lin;
    QHBox *box;
    QPushButton *button;
    bool withComment, hasMultiLines, asciiOnly, accel;

};

class PropertyBoolItem : public QObject,
			 public PropertyItem
{
    Q_OBJECT

public:
    PropertyBoolItem( PropertyList *l, PropertyItem *after, PropertyItem *prop, const QString &propName );
    ~PropertyBoolItem();

    virtual void showEditor();
    virtual void hideEditor();

    virtual void setValue( const QVariant &v );

private slots:
    void setValue();

private:
    QComboBox *combo();
    QComboBox *comb;

};

class PropertyIntItem : public QObject,
			public PropertyItem
{
    Q_OBJECT

public:
    PropertyIntItem( PropertyList *l, PropertyItem *after, PropertyItem *prop,
		     const QString &propName, bool s );
    ~PropertyIntItem();

    virtual void showEditor();
    virtual void hideEditor();

    virtual void setValue( const QVariant &v );

private slots:
    void setValue();

private:
    QSpinBox *spinBox();
    QSpinBox *spinBx;
    bool signedValue;

};

class PropertyListItem : public QObject,
			 public PropertyItem
{
    Q_OBJECT

public:
    PropertyListItem( PropertyList *l, PropertyItem *after, PropertyItem *prop,
		      const QString &propName, bool editable );
    ~PropertyListItem();

    virtual void showEditor();
    virtual void hideEditor();

    virtual void setValue( const QVariant &v );

    QString currentItem() const;
    int currentIntItem() const;
    void setCurrentItem( const QString &s );
    void setCurrentItem( int i );
    int currentIntItemFromObject() const;
    QString currentItemFromObject() const;

private slots:
    void setValue();

private:
    QComboBox *combo();
    QComboBox *comb;
    int oldInt;
    bool editable;
    QString oldString;

};

class PropertyFontItem : public QObject,
			 public PropertyItem
{
    Q_OBJECT

public:
    PropertyFontItem( PropertyList *l, PropertyItem *after, PropertyItem *prop, const QString &propName );
    ~PropertyFontItem();

    virtual void createChildren();
    virtual void initChildren();

    virtual void showEditor();
    virtual void hideEditor();

    virtual void setValue( const QVariant &v );
    virtual bool hasSubItems() const;
    virtual void childValueChanged( PropertyItem *child );

private slots:
    void getFont();

private:
    QLineEdit *lined;
    QPushButton *button;
    QHBox *box;

};

class PropertyCoordItem : public QObject,
			  public PropertyItem
{
    Q_OBJECT

public:
    enum Type { Rect, Size, Point };

    PropertyCoordItem( PropertyList *l, PropertyItem *after, PropertyItem *prop,
		       const QString &propName, Type t );
    ~PropertyCoordItem();

    virtual void createChildren();
    virtual void initChildren();

    virtual void showEditor();
    virtual void hideEditor();

    virtual void setValue( const QVariant &v );
    virtual bool hasSubItems() const;
    virtual void childValueChanged( PropertyItem *child );

private:
    QLineEdit *lined();
    QLineEdit *lin;
    Type typ;

};

class PropertyColorItem : public QObject,
			  public PropertyItem
{
    Q_OBJECT

public:
    PropertyColorItem( PropertyList *l, PropertyItem *after, PropertyItem *prop,
		       const QString &propName, bool children );
    ~PropertyColorItem();

    virtual void createChildren();
    virtual void initChildren();

    virtual void showEditor();
    virtual void hideEditor();

    virtual void setValue( const QVariant &v );
    virtual bool hasSubItems() const;
    virtual void childValueChanged( PropertyItem *child );

    virtual bool hasCustomContents() const;
    virtual void drawCustomContents( QPainter *p, const QRect &r );

private slots:
    void getColor();

private:
    QHBox *box;
    QFrame *colorPrev;
    QPushButton *button;
    bool withChildren;

};

class PropertyPixmapItem : public QObject,
			   public PropertyItem
{
    Q_OBJECT

public:
    PropertyPixmapItem( PropertyList *l, PropertyItem *after, PropertyItem *prop,
			const QString &propName );
    ~PropertyPixmapItem();

    virtual void showEditor();
    virtual void hideEditor();

    virtual void setValue( const QVariant &v );

    virtual bool hasCustomContents() const;
    virtual void drawCustomContents( QPainter *p, const QRect &r );

private slots:
    void getPixmap();

private:
    QHBox *box;
    QLabel *pixPrev;
    QPushButton *button;

};


class PropertySizePolicyItem : public QObject,
			  public PropertyItem
{
    Q_OBJECT

public:
    PropertySizePolicyItem( PropertyList *l, PropertyItem *after, PropertyItem *prop,
			    const QString &propName );
    ~PropertySizePolicyItem();

    virtual void createChildren();
    virtual void initChildren();

    virtual void showEditor();
    virtual void hideEditor();

    virtual void setValue( const QVariant &v );
    virtual bool hasSubItems() const;
    virtual void childValueChanged( PropertyItem *child );

private:
    QLineEdit *lined();
    QLineEdit *lin;

};

class PropertyPaletteItem : public QObject,
			    public PropertyItem
{
    Q_OBJECT

public:
    PropertyPaletteItem( PropertyList *l, PropertyItem *after, PropertyItem *prop,
			const QString &propName );
    ~PropertyPaletteItem();

    virtual void showEditor();
    virtual void hideEditor();

    virtual void setValue( const QVariant &v );

    virtual bool hasCustomContents() const;
    virtual void drawCustomContents( QPainter *p, const QRect &r );

private slots:
    void getPalette();

private:
    QHBox *box;
    QLabel *palettePrev;
    QPushButton *button;

};

class PropertyCursorItem : public QObject,
			   public PropertyItem
{
    Q_OBJECT

public:
    PropertyCursorItem( PropertyList *l, PropertyItem *after, PropertyItem *prop,
			const QString &propName );
    ~PropertyCursorItem();

    virtual void showEditor();
    virtual void hideEditor();

    virtual void setValue( const QVariant &v );

private slots:
    void setValue();

private:
    QComboBox *combo();
    QComboBox *comb;

};


class PropertyList : public QListView
{
    Q_OBJECT

public:
    PropertyList( PropertyEditor *e );

    void setupProperties();

    void setCurrentItem( QListViewItem *i );
    void valueChanged( PropertyItem *i );
    void refetchData();
    void setPropertyValue( PropertyItem *i );
    void setCurrentProperty( const QString &n );

    PropertyEditor *propertyEditor() const;

public slots:
    void updateEditorSize();
    void resetProperty();

private slots:
    void itemPressed( QListViewItem *i, const QPoint &p, int c );
    void toggleOpen( QListViewItem *i );
    bool eventFilter( QObject *o, QEvent *e );

protected:
    void resizeEvent( QResizeEvent *e );
    void paintEmptyArea( QPainter *p, const QRect &r );
    bool addPropertyItem( PropertyItem *&item, const QCString &name, QVariant::Type t );

private:
    PropertyEditor *editor;

};



class PropertyEditor : public QVBox
{
    Q_OBJECT

public:
    PropertyEditor( QWidget *parent );

    QWidget *widget() const;

    void clear();
    void setup();

    void emitWidgetChanged();
    void refetchData();

    void closed( FormWindow *w );

    PropertyList *propertyList() const;
    FormWindow *formWindow() const;

    QString currentProperty() const;
    QString classOfCurrentProperty() const;
    QMetaObject* metaObjectOfCurrentProperty() const;

    void resetFocus();
    
signals:
    void hidden();

public slots:
    void setWidget( QWidget *w, FormWindow *fw );

protected:
    void closeEvent( QCloseEvent *e );

private:
    QWidget *wid;
    PropertyList *listview;
    FormWindow *formwindow;

};

#endif
