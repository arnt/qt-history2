#ifndef EDITFUNCTIONSIMPL_H
#define EDITFUNCTIONSIMPL_H

#include "editfunctions.h"
#include "hierarchyview.h"
#include <qmap.h>

class FormWindow;
class QListViewItem;

class EditFunctions : public EditFunctionsBase
{
    Q_OBJECT

public:
    EditFunctions( QWidget *parent, FormWindow *fw, bool showOnlySlots = FALSE );

    void setCurrentFunction( const QString &function );
    static void removeFunctionFromCode( const QString &function, FormWindow *fw );
    void functionAdd( const QString &access = QString::null, 
		      const QString &type = QString::null  );
    void functionAdd() { functionAdd( "public", "function" ); }

protected slots:
    void okClicked();
    void functionRemove();
    void currentItemChanged( QListViewItem * );
    void currentTextChanged( const QString &txt );
    void currentSpecifierChanged( const QString &s );
    void currentAccessChanged( const QString &a );
    void currentReturnTypeChanged( const QString &type );
    void currentTypeChanged( const QString &type );
    void displaySlots( bool justSlots );

private:
    void setupGUI();
    FormWindow *formWindow;
    QMap<QListViewItem*, QString> oldFunctionNames;
    QStringList removedFunctions;
};

#endif
