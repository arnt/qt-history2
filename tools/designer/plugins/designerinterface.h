#ifndef DESIGNERINTERFACE_H
#define DESIGNERINTERFACE_H

/*
  This include file declares all interfaces for the Qt Designer
  that can be used in the plugins.
*/

#include <qcomponentinterface.h>
#include <qvariant.h>
#include "../designer/actioninterface.h"
#include "../shared/widgetinterface.h"
#include "../designer/filterinterface.h"
#include "../shared/editorinterface.h"
#include "../shared/templatewizardiface.h"
#include "../shared/eventinterface.h"

class QObject;

/*
 * Interfaces for MainWindow access
 */
// {A0E661DA-F45C-4830-AF47-03EC53EB1633}
Q_UUID(IID_DesignerMainWindowInterface,
0xa0e661da, 0xf45c, 0x4830, 0xaf, 0x47, 0x3, 0xec, 0x53, 0xeb, 0x16, 0x33);

struct DesignerMainWindowInterface : public QUnknownInterface
{
    virtual void fileNew() = 0;
    virtual void fileNewProject() = 0;
    virtual void fileCloseProject() = 0;
    virtual void fileOpen() = 0;
    virtual bool fileSave() = 0;
    virtual bool fileSaveAs() = 0;
    virtual void fileSaveAll() = 0;
    virtual void fileCreateTemplate() = 0;

    virtual void editUndo() = 0;
    virtual void editRedo() = 0;
    virtual void editCut() = 0;
    virtual void editCopy() = 0;
    virtual void editPaste() = 0;
    virtual void editDelete() = 0;
    virtual void editSelectAll() = 0;
    virtual void editLower() = 0;
    virtual void editRaise() = 0;
    virtual void editAdjustSize() = 0;
    virtual void editLayoutHorizontal() = 0;
    virtual void editLayoutVertical() = 0;
    virtual void editLayoutHorizontalSplit() = 0;
    virtual void editLayoutVerticalSplit() = 0;
    virtual void editLayoutGrid() = 0;
    virtual void editLayoutContainerHorizontal() = 0;
    virtual void editLayoutContainerVertical() = 0;
    virtual void editLayoutContainerGrid() = 0;
    virtual void editBreakLayout() = 0;
    virtual void editAccels() = 0;
    virtual void editSlots() = 0;
    virtual void editConnections() = 0;
    virtual void editFormSettings() = 0;
    virtual void editProjectSettings() = 0;
    virtual void editDatabaseConnections() = 0;
    virtual void editPreferences() = 0;

    virtual void previewForm() = 0;
    virtual void previewForm( const QString& ) = 0;
    virtual void windowPropertyEditor( bool ) = 0;
    virtual void windowHierarchyView( bool ) = 0;
    virtual void windowFormList( bool ) = 0;
    virtual void windowActionEditor( bool ) = 0;

    virtual void toolsCustomWidget() = 0;

    virtual void helpContents() = 0;
    virtual void helpManual() = 0;
    virtual void helpAbout() = 0;
    virtual void helpAboutQt() = 0;

};

// {C797C970-36E5-463b-AEAE-54AF3F05588F}
Q_UUID(IID_DesignerStatusBarInterface,
0xc797c970, 0x36e5, 0x463b, 0xae, 0xae, 0x54, 0xaf, 0x3f, 0x5, 0x58, 0x8f);

struct DesignerStatusBarInterface : public QUnknownInterface
{
    virtual void setMessage( const QString&, int ms = 3000 ) = 0;
    virtual void clear() = 0;
};

/*
 * Interfaces for Widget access
 */
// {B7A60C5D-6476-4978-8C05-CCE5CE706D2B}
Q_UUID(IID_DesignerWidgetInterface,
0xb7a60c5d, 0x6476, 0x4978, 0x8c, 0x5, 0xcc, 0xe5, 0xce, 0x70, 0x6d, 0x2b);

struct DesignerWidgetInterface : public QUnknownInterface
{
    virtual QVariant property( const QCString& ) = 0;
    virtual bool setProperty( const QCString&, const QVariant& ) = 0;

    virtual void setSelected( bool ) = 0;
    virtual bool selected() const = 0;

    virtual void remove() = 0;
};

// {5F8283D7-311D-4bfb-AD6A-476B77E8C288}
Q_UUID(IID_DesignerWidgetListInterface,
0x5f8283d7, 0x311d, 0x4bfb, 0xad, 0x6a, 0x47, 0x6b, 0x77, 0xe8, 0xc2, 0x88);

struct DesignerWidgetListInterface : public QUnknownInterface
{
    virtual uint count() const = 0;
    virtual DesignerWidgetInterface *reset() = 0;
    virtual DesignerWidgetInterface *current() = 0;
    virtual DesignerWidgetInterface *next() = 0;
    virtual DesignerWidgetInterface *prev() = 0;

    virtual void selectAll() const = 0;
    virtual void removeAll() const = 0;
};

/*
 * Interfaces for FormWindow access
 */
// {A76C2285-3A5C-468b-B72F-509447034D7C}
Q_UUID(IID_DesignerFormInterface,
0xa76c2285, 0x3a5c, 0x468b, 0xb7, 0x2f, 0x50, 0x94, 0x47, 0x3, 0x4d, 0x7c);
/* {1615FA5A-AA58-40c0-8613-5ECD71FCC8E8}
Q_UUID(IID_DesignerActiveFormInterface,
0x1615fa5a, 0xaa58, 0x40c0, 0x86, 0x13, 0x5e, 0xcd, 0x71, 0xfc, 0xc8, 0xe8);
*/
struct DesignerFormInterface : public QUnknownInterface
{
    virtual QVariant property( const QCString& ) = 0;
    virtual bool setProperty( const QCString&, const QVariant& ) = 0;
    virtual void addWidget( QWidget *w ) = 0;

    virtual void save() const = 0;
    virtual void close() const = 0;
    virtual void undo() const = 0;
    virtual void redo() const = 0;

    virtual bool connect( const char *, QObject *, const char * ) = 0;
};

// {2002DD2F-9B9F-48c3-821C-619EE0375D27}
Q_UUID(IID_DesignerFormListInterface,
0x2002dd2f, 0x9b9f, 0x48c3, 0x82, 0x1c, 0x61, 0x9e, 0xe0, 0x37, 0x5d, 0x27);

struct DesignerFormListInterface : public QUnknownInterface
{
    virtual const QPixmap* pixmap( DesignerFormInterface*, int col ) const = 0;
    virtual void setPixmap( DesignerFormInterface*, int col, const QPixmap& ) = 0;
    virtual QString text( DesignerFormInterface*, int col ) const = 0;
    virtual void setText( DesignerFormInterface*, int col, const QString& ) = 0;

    virtual uint count() const = 0;
    virtual DesignerFormInterface *reset() = 0;
    virtual DesignerFormInterface *current() = 0;
    virtual DesignerFormInterface *next() = 0;
    virtual DesignerFormInterface *prev() = 0;

    virtual bool newForm() = 0;
    virtual bool loadForm() = 0;
    virtual bool saveAll() const = 0;
    virtual void closeAll() const = 0;

    virtual bool connect( const char *, QObject *, const char * ) = 0;
};

// {93f93c71-a37e-4785-928c-c367ea7fefeb}
Q_UUID(IID_DesignerProjectInterface,
       0x93f93c71, 0xa37e, 0x4785, 0x92, 0x8c, 0xc3, 0x67, 0xea, 0x7f, 0xef, 0xeb);

struct DesignerProjectInterface : public QUnknownInterface
{
    virtual QString fileName() const = 0;
    virtual QString projectName() const = 0;
    virtual QString databaseFile() const = 0;
    virtual QStringList uiFiles() const = 0;

    virtual QStringList databaseConnectionList() = 0;
    virtual QStringList databaseTableList( const QString &connection ) = 0;
    virtual QStringList databaseFieldList( const QString &connection, const QString &table ) = 0;

    virtual void openDatabase( const QString &connection ) = 0;
    virtual void closeDatabase( const QString &connection ) = 0;

};

/*
 * Interfaces for PropertyEditor access
 */
// {834330A1-8542-4c1e-948A-58D68CE97BFD}
Q_UUID(IID_DesignerPropertyEditorInterface,
0x834330a1, 0x8542, 0x4c1e, 0x94, 0x8a, 0x58, 0xd6, 0x8c, 0xe9, 0x7b, 0xfd);

/*
 * Interfaces for HierarchyView access
 */
// {13BB1270-2A4C-4d7f-A4D0-B49834DB2774}
Q_UUID(IID_DesignerHierarchyViewInterface,
0x13bb1270, 0x2a4c, 0x4d7f, 0xa4, 0xd0, 0xb4, 0x98, 0x34, 0xdb, 0x27, 0x74);

/*
 * Interfaces for Configuration access
 */
// {4F5BA81F-520D-4a59-95DD-BDB8B5B24190}
Q_UUID(IID_DesignerConfigurationInterface,
0x4f5ba81f, 0x520d, 0x4a59, 0x95, 0xdd, 0xbd, 0xb8, 0xb5, 0xb2, 0x41, 0x90);

// {41ed19be-efca-42ed-aaad-1f2e502ca5fa}
Q_UUID( IID_DesignerMetaDatabaseInterface,
	0x41ed19be, 0xefca, 0x42ed, 0xaa, 0xad, 0x1f, 0x2e, 0x50, 0x2c, 0xa5, 0xfa );

struct DesignerMetaDatabaseInterface : public QUnknownInterface
{
    virtual void setFakeProperty( QObject *o, const QString &property, const QVariant& value ) = 0;
    virtual QVariant fakeProperty( QObject * o, const QString &property ) = 0;
    virtual void setPropertyChanged( QObject *o, const QString &property, bool changed ) = 0;
    virtual bool isPropertyChanged( QObject *o, const QString &property ) = 0;
    virtual void addConnection( QObject *o, QObject *sender, const QCString &signal,
				QObject *receiver, const QCString &slot ) = 0;

};

// {e61b28ef-63af-4b4f-9382-dd9c5a4ac277}

Q_UUID( IID_DesignerWidgetFactoryInteface,
	0xe61b28ef, 0x63af, 0x4b4f, 0x93, 0x82, 0xdd, 0x9c, 0x5a, 0x4a, 0xc2, 0x77 );

struct DesignerWidgetFactoryInterface : public QUnknownInterface
{
    virtual QWidget *create( const char *className, QWidget *parent, const char *name = 0 ) = 0;
    // ### need more stuff


};

#endif //DESIGNERINTERFACE_H
