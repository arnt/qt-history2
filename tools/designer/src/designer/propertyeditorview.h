#ifndef PROPERTYEDITORVIEW_H
#define PROPERTYEDITORVIEW_H

#include <QMainWindow>

class AbstractFormEditor;

class PropertyEditorView: public QMainWindow
{
    Q_OBJECT
public:
    PropertyEditorView(AbstractFormEditor *core, QWidget *parent = 0);
    virtual ~PropertyEditorView();

protected:
    void showEvent(QShowEvent *ev);
    void hideEvent(QHideEvent *ev);

signals:
    void visibilityChanged(bool isVisible);

private:
    AbstractFormEditor *m_core;
};

#endif // PROPERTYEDITORVIEW_H
