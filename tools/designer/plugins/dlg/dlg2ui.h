/*
  dlg2ui.h
*/

#ifndef DLG2UI_H
#define DLG2UI_H

#include <qcombobox.h>
#include <qdom.h>
#include <qfile.h>
#include <qiconview.h>
#include <qlayout.h>
#include <qlcdnumber.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qmessagebox.h>
#include <qregexp.h>
#include <qscrollview.h>
#include <qslider.h>
#include <qspinbox.h>
#include <qstring.h>
#include <qtabbar.h>
#include <qtextstream.h>
#include <qvariant.h>

typedef QMap<QString, QString> AttributeMap;

struct DlgConnection
{
    QString sender;
    QString signal;
    QString slot;
};

class Dlg2Ui
{
public:
    QStringList convertQtArchitectDlgFile( const QString& fileName );

private:
    QString opening( const QString& tag,
		     const AttributeMap& attr = AttributeMap() );
    QString closing( const QString& tag );
    void error( const QString& message );
    void syntaxError();
    QString getTextValue( const QDomNode& node );
    QVariant getValue( const QDomNodeList& children, const QString& tagName,
		       const QString& type = "qstring" );
    void emitHeader();
    void emitFooter();
    void emitSimpleValue( const QString& tag, const QString& value,
			  const AttributeMap& attr = AttributeMap() );
    void emitOpening( const QString& tag,
		      const AttributeMap& attr = AttributeMap() );
    void emitClosing( const QString& tag );
    void emitOpeningWidget( const QString& className );
    QString widgetClassName( const QDomElement& e );
    void emitColor( const QColor& color );
    void emitColorGroup( const QString& name, const QColorGroup& group );
    void emitVariant( const QVariant& val,
		      const QString& stringType = "string" );
    void emitProperty( const QString& prop, const QVariant& val,
		       const QString& stringType = "string" );
    void emitAttribute( const QString& attr, const QVariant& val,
			   const QString& stringType = "string" );
    void emitOpeningLayout( bool needsWidget, const QString& layoutKind,
			    const QString& name, int border, int autoBorder );
    void flushWidgets();
    void emitClosingLayout( bool needsWidget, const QString& layoutKind );
    bool isWidgetType( const QDomElement& e );
    void emitSpacer( int spacing, int stretch );
    QString filteredFlags( const QString& flags, const QRegExp& filter );
    void emitWidgetBody( const QDomElement& e, bool layouted );
    bool checkTagName( const QDomElement& e, const QString& tagName );
    QString normalizeType( const QString& type );
    QVariant getValue( const QDomElement& e, const QString& tagName,
		       const QString& type = "qstring" );
    void matchDialogCommon( const QDomElement& dialogCommon );
    bool needsQLayoutWidget( const QDomElement& e );
    void matchBoxLayout( const QDomElement& boxLayout );
    void matchBoxSpacing( const QDomElement& boxSpacing );
    void matchBoxStretch( const QDomElement& boxStretch );
    void matchColumnInfo( const QDomElement& columnInfo );
    void matchColumnList( const QDomElement& columnList );
    void matchGridLayout( const QDomElement& gridLayout );
    void matchGridRow( const QDomElement& gridRow );
    void matchGridSpacer( const QDomElement& gridSpacer );
    void matchLayoutWidget( const QDomElement& layoutWidget );
    void matchBox( const QDomElement& box );
    void matchLayout( const QDomElement& layout );
    void matchWidgetLayoutCommon( const QDomElement& widgetLayoutCommon );
    void matchWidget( const QDomElement& widget );
    void matchWidgets( const QDomElement& widgets );
    void matchTabOrder( const QDomElement& tabOrder );
    void matchWidgetLayout( const QDomElement& widgetLayout );
    void matchDialog( const QDomElement& dialog );

    QString yyOut;
    QString yyIndentStr;
    QString yyFileName;
    QString yyClassName;
    QMap<QString, int> yyWidgetTypeSet;
    QMap<QString, QMap<QString, int> > yyPropertyMap;
    QMap<QString, QDomElement> yyWidgetMap;
    QMap<QString, QString> yyCustomWidgets;
    QValueList<DlgConnection> yyConnections;
    QMap<QString, QString> yySlots;
    QStringList yyTabStops;
    QString yyBoxKind;
    int yyLayoutDepth;
    int yyGridRow;
    int yyGridColumn;

    int numErrors;
    int uniqueLayout;
    int uniqueSpacer;
    int uniqueWidget;
};

#endif
