#include <widgetinterface.h>
#include <qobjectcleanuphandler.h>

class CustomWidgetInterface : public WidgetInterface
{
public:
    CustomWidgetInterface();
    virtual ~CustomWidgetInterface() {}

    // From QUnknownInterface
    QRESULT queryInterface( const QUuid&, QUnknownInterface **iface );
    Q_REFCOUNT

    // From QFeatureListInterface
    QStringList featureList() const;

    // From WidgetInterface
    QWidget* create( const QString &classname, QWidget* parent = 0, const char* name = 0 );
    QString group( const QString& ) const;
    QIconSet iconSet( const QString& ) const;
    QString includeFile( const QString& ) const;
    QString toolTip( const QString& ) const;
    QString whatsThis( const QString& ) const;
    bool isContainer( const QString& ) const;

private:
    QObjectCleanupHandler objects;

    ulong ref;
};
