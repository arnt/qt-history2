#include <projectsettingsiface.h>

class CppProjectSettings;

class ProjectSettingsInterfaceImpl : public ProjectSettingsInterface
{
public:
    ProjectSettingsInterfaceImpl( QUnknownInterface *outer );

    ulong addRef();
    ulong release();

    ProjectSettings *projectSetting();
    QStringList projectSettings() const;
    void connectTo( QUnknownInterface *appInterface );
    void deleteProjectSettingsObject( ProjectSettings * );
    QRESULT queryInterface( const QUuid &uuid, QUnknownInterface **iface );

private:
    QUnknownInterface *parent;
    int ref;
    CppProjectSettings *settingsTab;
};

