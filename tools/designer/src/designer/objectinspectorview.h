#ifndef OBJECTINSPECTORVIEW_H
#define OBJECTINSPECTORVIEW_H

#include <QMainWindow>

class AbstractFormEditor;

class ObjectInspectorView: public QMainWindow
{
    Q_OBJECT
public:
    ObjectInspectorView(AbstractFormEditor *core, QWidget *parent = 0);
    virtual ~ObjectInspectorView();

signals:
    void visibilityChanged(bool isVisible);

protected:
    void showEvent(QShowEvent *ev);
    void hideEvent(QHideEvent *ev);

private:
    AbstractFormEditor *m_core;
};

#endif // OBJECTINSPECTORVIEW_H
