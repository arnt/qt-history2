#include <widgetinterface.h>
#include <qcleanuphandler.h>

class FileChooserInterface : public WidgetInterface
{
public:
    FileChooserInterface();
    virtual ~FileChooserInterface() {}

    QUnknownInterface *queryInterface( const QUuid& );
    ulong addRef();
    ulong release();

    QStringList featureList() const;

    QWidget* create( const QString &classname, QWidget* parent = 0, const char* name = 0 );
    QString group( const QString& ) const;
    QString iconSet( const QString& ) const;
    QIconSet iconset( const QString& ) const;
    QString includeFile( const QString& ) const;
    QString toolTip( const QString& ) const;
    QString whatsThis( const QString& ) const;
    bool isContainer( const QString& ) const;

private:
    QGuardedCleanupHandler<QObject> objects;

    ulong ref;
};
