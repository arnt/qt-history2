#ifndef TABLEEDITOR_H
#define TABLEEDITOR_H
#include "tableeditor.h"

class QTable;
class FormWindow;

class TableEditor : public TableEditorBase
{
    Q_OBJECT

public:
    TableEditor( QWidget* parent = 0, QWidget *editWidget = 0, FormWindow *fw = 0,
		 const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~TableEditor();

protected slots:
    void columnDownClicked();
    void columnTextChanged( const QString & );
    void columnUpClicked();
    void currentColumnChanged( QListBoxItem * );
    void currentFieldChanged( const QString & );
    void currentRowChanged( QListBoxItem * );
    void deleteColumnClicked();
    void deleteRowClicked();
    void newColumnClicked();
    void newRowClicked();
    void okClicked();
    void rowDownClicked();
    void rowTextChanged( const QString & );
    void rowUpClicked();
    void applyClicked();
    void chooseRowPixmapClicked();
    void deleteRowPixmapClicked();
    void chooseColPixmapClicked();
    void deleteColPixmapClicked();

private:
    void readFromTable();

private:
    QTable *editTable;
    FormWindow *formWindow;

};

#endif // TABLEEDITOR_H
