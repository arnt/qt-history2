#ifndef FORMWINDOW_H
#define FORMWINDOW_H

#include "formeditor_global.h"
#include "formeditor.h"

// sdk
#include <abstractformwindowmanager.h>
#include <abstractformwindow.h>

// Qt
#include <QWidget>
#include <QPixmap>
#include <QHash>
#include <QList>

#include <QMap>

class AbstractFormWindowCursor;
class FormWindowCursor;
class DomConnections;

class QLabel;
class QTimer;
class QAction;
class QStackedWidget;
class QMenu;
class QtUndoStack;
class QRubberBand;
class WidgetSelection;
class BreakLayoutCommand;
class FormWindowManager;
class OrderIndicator;
class FormEditor;
class SignalSlotEditor;

class FormWindowDnDItem : public AbstractDnDItem
{
    Q_OBJECT
public:    
    FormWindowDnDItem(QWidget *widget);
    FormWindowDnDItem(DomUI *dom_ui, QWidget *widget);
    virtual ~FormWindowDnDItem();
    
    virtual DomUI *domUi() const;
    virtual QWidget *decoration() const;
    virtual QWidget *widget() const;
    virtual QPoint hotSpot() const;
private:
    QWidget *m_decoration, *m_widget;
    DomUI *m_dom_ui;
    QPoint m_hot_spot;
};

class QT_FORMEDITOR_EXPORT FormWindow: public AbstractFormWindow
{
    Q_OBJECT
public:
    enum // ### remove
    {
        DefaultMargin = 11,
        DefaultSpacing = 6
    };
    
    enum HighlightMode
    {
        Restore,
        Highlight
    };
    
public:
    FormWindow(FormEditor *core, QWidget *parent = 0, Qt::WFlags flags = 0);
    virtual ~FormWindow();

    virtual FormEditor *core() const;

    virtual AbstractFormWindowCursor *cursor() const;

    virtual bool hasFeature(Feature f) const;
    virtual Feature features() const;
    virtual void setFeatures(Feature f);
    
    virtual EditMode editMode() const;
    virtual void setEditMode(EditMode mode);
    
    QString fileName() const;
    void setFileName(const QString &fileName);

    QString contents() const;
    void setContents(const QString &contents);
    void setContents(QIODevice *dev);

    inline QPoint grid() const { return m_grid; }
    inline void setGrid(const QPoint &grid) { m_grid = grid; }

    QWidget *mainContainer() const;
    void setMainContainer(QWidget *mainContainer);
    bool isMainContainer(const QWidget *w) const;

    QWidget *currentWidget() const;
    void setCurrentWidget(QWidget *currentWidget);

    virtual QSize sizeHint() const 
    { return QSize(400, 300); }  /// ### remove me

    bool hasInsertedChildren(QWidget *w) const;

    QList<QWidget *> selectedWidgets() const;
    void clearSelection(bool changePropertyDisplay=true);
    bool isWidgetSelected(QWidget *w) const;
    void selectWidget(QWidget *w, bool select=true);

    void selectWidgets();
    void repaintSelection();
    void repaintSelection(QWidget *w);
    void updateSelection(QWidget *w);
    void updateChildSelections(QWidget *w);
    void raiseChildSelections(QWidget *w);
    void raiseSelection(QWidget *w);
    void simplifySelection(QList<QWidget*> *sel);
    void hideSelection(QWidget *w);

    inline QList<QWidget *> widgets() const { return m_widgets; }
    inline int widgetCount() const { return m_widgets.count(); }
    inline QWidget *widgetAt(int index) const { return m_widgets.at(index); }
    
    QList<QWidget *> widgets(QWidget *widget) const;

    QWidget *createWidget(DomUI *ui, const QRect &rect, QWidget *target);
    void insertWidget(QWidget *w, const QRect &rect, QWidget *target);
    void resizeWidget(QWidget *widget, const QRect &geometry);
    void deleteWidgets(const QList<QWidget*> &widget_list);

    bool isManaged(QWidget *w) const;

    void manageWidget(QWidget *w);
    void unmanageWidget(QWidget *w);

    void widgetChanged(QObject *w);

    inline QtUndoStack *commandHistory() const
    { return m_commandHistory; }
    
    void beginCommand(const QString &description);
    void endCommand();

    void emitSelectionChanged();

    bool unify(QObject *w, QString &s, bool changeIt);

    int currentTool() const;
    bool isDirty() const;
    void setDirty(bool dirty);

    static FormWindow *findFormWindow(QWidget *w);

    virtual QWidget *containerAt(const QPoint &pos);
    virtual QWidget *widgetAt(const QPoint &pos);
    virtual void highlightWidget(QWidget *w, const QPoint &pos,
                                    HighlightMode mode = Highlight);
                      
    DomConnections *saveConnections();
    void createConnections(DomConnections *connections, QWidget *parent);
                      
    inline void emitGeometryChanged(QWidget *w) 
    { emit geometryChanged(w); }
                                             
signals:
    void showContextMenu(QWidget *w, const QPoint &pos);
    void geometryChanged(QWidget *w);
    void editModeChanged(EditMode);

public slots:
    void deleteWidgets();
    void raiseWidgets();
    void lowerWidgets();
    void copy();
    void cut();
    void paste();
    void selectAll();

    void layoutHorizontal();
    void layoutVertical();
    void layoutGrid();
    void layoutHorizontalSplit();
    void layoutVerticalSplit();
    void layoutHorizontalContainer(QWidget *w);
    void layoutVerticalContainer(QWidget *w);
    void layoutGridContainer(QWidget *w);
    void breakLayout(QWidget *w);

    void breakLayout();

protected:
    virtual QMenu *createPopupMenu(QWidget *w);
    virtual void resizeEvent(QResizeEvent *e);
    
private slots:
    void selectionChangedTimerDone();
    void invalidCheckedSelections();
    void updateDirty();

private:
    void init();
    QPoint gridPoint(const QPoint &p) const;

    enum Tool { PointerTool, BuddyTool, OrderTool };
    enum RectType { Insert, Rubber };

    void startRectDraw(const QPoint &global, QWidget *, RectType t);
    void continueRectDraw(const QPoint &global, QWidget *, RectType t);
    void endRectDraw();

    bool allowMove(QWidget *w);
    QWidget *containerAt(const QPoint &pos, QWidget *notParentOf);

    QList<QWidget*> checkSelectionsForMove(QWidget *w);

    QLabel *sizePreview() const;
    void checkPreviewGeometry(QRect &r);

    void handleContextMenu(QWidget *w, QContextMenuEvent *e);
    void handleMouseButtonDblClickEvent(QWidget *w, QMouseEvent *e);
    void handleMousePressEvent(QWidget *w, QMouseEvent *e);
    void handleMouseMoveEvent(QWidget *w, QMouseEvent *e);
    void handleMouseReleaseEvent(QWidget *w, QMouseEvent *e);
    void handleKeyPressEvent(QWidget *w, QKeyEvent *e);
    void handleKeyReleaseEvent(QWidget *w, QKeyEvent *e);
    void handlePaintEvent(QWidget *w, QPaintEvent *e);

    bool isCentralWidget(QWidget *w) const;
    QWidget *designerWidget(QWidget *w) const;

    BreakLayoutCommand *breakLayoutCommand(QWidget *w);

    void showOrderIndicators();
    void hideOrderIndicators();
    void updateOrderIndicators();
    void repositionOrderIndicators();

    void setCursorToAll(const QCursor &c, QWidget *start);
    void restoreCursors(QWidget *start, FormWindow *fw);

    QPoint mapToForm(const QWidget *w, const QPoint &pos) const;
    bool canBeBuddy(QWidget *w) const;

    QWidget *findContainer(QWidget *w, bool excludeLayout) const;
    QWidget *findTargetContainer(QWidget *widget) const;

    static int widgetDepth(QWidget *w);
    static bool isChildOf(QWidget *c, const QWidget *p);
    
private:
    Feature m_feature;
    FormEditor *m_core;
    FormWindowCursor *m_cursor;
    QWidget *m_mainContainer;
    QWidget *m_currentWidget;
    mutable QLabel *sizePreviewLabel;
    QPoint m_grid;

    bool drawRubber;
    QPoint rectAnchor;
    QRect currRect;
    bool oldRectValid;

    QList<QWidget*> m_widgets;
    QHash<QWidget *, QWidget*> m_insertedWidgets;

    bool checkedSelectionsForMove;
    QList<WidgetSelection *> selections;
    QHash<QWidget *, WidgetSelection *> usedSelections;

    bool validForBuddy;
    QPoint startPos;
    QPoint currentPos;

    QRect widgetGeom;
    QPoint oldPressPos;
    QPoint origPressPos;
    QWidget *startWidget;
    QWidget *endWidget;

    QWidget *targetContainer;
    bool hadOwnPalette;
    QPalette restorePalette;

    QtUndoStack *m_commandHistory;

    QString m_fileName;
    QString pixLoader;

    QList<OrderIndicator*> orderIndicators;
    QList<QWidget*> orderedWidgets;
    QList<QWidget*> stackedWidgets;

    QMap<QWidget*, QPalette> palettesBeforeHighlight;
    
    QRubberBand *m_rubberBand;

    QTimer *m_selectionChangedTimer;
    int m_dirty;
    int m_lastIndex;

    SignalSlotEditor *m_signalSlotEditor;
    EditMode m_editMode;
    
private:
    friend class FormWindowManager;
    friend class SizeHandle;
    friend class WidgetSelection;
    friend class QDesignerWidget;
};



#endif
