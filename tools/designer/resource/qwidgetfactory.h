#ifndef QWIDGETFACTORY_H
#define QWIDGETFACTORY_H

#ifndef QT_H
#include "qstring.h"
#include "qcstring.h"
#include "qlist.h"
#include "qimage.h"
#include "qvaluelist.h"
#endif // QT_H

class QWidget;
class QLayout;
class QDomElement;

class QWidgetFactory
{
public:
    QWidgetFactory();
    virtual ~QWidgetFactory() {}
    
    static QWidget *create( const QString &uiFile, QWidget *parent = 0, const char *name = 0 );
    static void addWidgetFactory( QWidgetFactory *factory );

    virtual QWidget *createWidget( const QString &className, QWidget *parent, const char *name ) const;

private:
    enum LayoutType { HBox, VBox, Grid, NoLayout };
    void loadImageCollection( const QDomElement &e );
    void loadConnections( const QDomElement &e );
    void loadTabOrder( const QDomElement &e );
    QWidget *createWidgetInternal( const QDomElement &e, QWidget *parent, QLayout* layout );
    QLayout *createLayout( QWidget *widget, QLayout*  layout, LayoutType type );
    LayoutType layoutType( QLayout *l ) const;
    void setProperty( QObject* widget, const QString &prop, const QDomElement &e );
    void createSpacer( const QDomElement &e, QLayout *layout );

private:
    struct Image {
	QImage img;
	QString name;
	bool operator==(  const Image &i ) const {
	    return ( i.name == name &&
		     i.img == img );
	}
    };

    QValueList<Image> images;

};

#endif
