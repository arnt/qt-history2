#ifndef CONFIGURE_H
#define CONFIGURE_H

#include "configuredialog.h"
#include <qstring.h>
#include <qstringlist.h>
#include <qprocess.h>

class QCheckListItem;

class ConfigureQtDialogImpl : public ConfigureQtDialog
{
    Q_OBJECT
public:
    ConfigureQtDialogImpl( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~ConfigureQtDialogImpl();

protected slots:
    void accept();
    void saveSettings();
    void execute();

    void qmakeDone();
    void readQmakeOutput();
private:
    QCheckListItem* debugMode;
    QCheckListItem* buildType;
    QCheckListItem* threadModel;
    QCheckListItem* modules;
    QCheckListItem* mkspec;
    QCheckListItem* sqldrivers;

    QProcess qmake;

    void loadSettings();
    void set( QCheckListItem* parent, const QString& setting );
    void set( QCheckListItem* parent, const QStringList& settings );
    void saveSet( QListView* list );

};

#endif
