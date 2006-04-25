#ifndef QTBRUSHEDITOR_H
#define QTBRUSHEDITOR_H

#include <QWidget>

class QtBrushManager;

class QtBrushEditor : public QWidget
{
    Q_OBJECT
public:
    QtBrushEditor(QWidget *parent = 0);
    ~QtBrushEditor();

    void setBrush(const QBrush &brush);
    QBrush brush() const;

    void setBrushManager(QtBrushManager *manager);

signals:
    void textureChooserActivated(QWidget *parent, const QBrush &initialBrush);
private:
    class QtBrushEditorPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QtBrushEditor)
    Q_DISABLE_COPY(QtBrushEditor)
    Q_PRIVATE_SLOT(d_func(), void slotPatternChooserClicked());
    Q_PRIVATE_SLOT(d_func(), void slotTextureChooserClicked());
    Q_PRIVATE_SLOT(d_func(), void slotGradientChooserClicked());
    Q_PRIVATE_SLOT(d_func(), void slotChooserClicked());
    Q_PRIVATE_SLOT(d_func(), void slotApplyClicked());
    Q_PRIVATE_SLOT(d_func(), void slotAddToCustomClicked());
    Q_PRIVATE_SLOT(d_func(), void slotRemoveClicked());
    Q_PRIVATE_SLOT(d_func(), void slotItemActivated(QListWidgetItem *));
    Q_PRIVATE_SLOT(d_func(), void slotCurrentItemChanged(QListWidgetItem *));
    Q_PRIVATE_SLOT(d_func(), void slotItemRenamed(QListWidgetItem *));
    Q_PRIVATE_SLOT(d_func(), void slotBrushAdded(const QString &, const QBrush &));
    Q_PRIVATE_SLOT(d_func(), void slotBrushRemoved(const QString &));
    Q_PRIVATE_SLOT(d_func(), void slotCurrentBrushChanged(const QString &, const QBrush &));
};

#endif
