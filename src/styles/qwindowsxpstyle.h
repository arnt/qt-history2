#ifndef QWINDOWSXPSTYLE_H
#define QWINDOWSXPSTYLE_H

#ifndef QT_H
#include "qwindowsstyle.h"
#endif // QT_H

#ifndef QT_NO_STYLE_WINDOWSXP

#if defined(QT_PLUGIN_STYLE_WINDOWSXP)
#define Q_EXPORT_STYLE_WINDOWSXP
#else
#define Q_EXPORT_STYLE_WINDOWSXP Q_EXPORT
#endif

class Q_EXPORT_STYLE_WINDOWSXP QWindowsXPStyle : public QWindowsStyle
{
    Q_OBJECT
public:
    QWindowsXPStyle();
    ~QWindowsXPStyle();

    void polish( QWidget* );
    void unPolish( QWidget* );

    // shapes
    void drawPanel( QPainter *p, int x, int y, int w, int h,
                    const QColorGroup &, bool sunken=FALSE,
                    int lineWidth = 1, const QBrush *fill = 0 );

    void drawButton( QPainter *p, int x, int y, int w, int h,
                     const QColorGroup &g, bool sunken = FALSE,
                     const QBrush *fill = 0 );

    void drawBevelButton( QPainter *p, int x, int y, int w, int h,
		     const QColorGroup &g, bool sunken = FALSE,
		     const QBrush *fill = 0 );

    void drawPopupPanel( QPainter *p, int x, int y, int w, int h,
				 const QColorGroup &,  int lineWidth = 2,
				 const QBrush *fill = 0 );

    void drawArrow( QPainter *p, Qt::ArrowType type, bool down,
		     int x, int y, int w, int h,
		     const QColorGroup &g, bool enabled, const QBrush *fill = 0 );

    // PushButton
    void drawPushButton( QPushButton* btn, QPainter *p);
    void drawPushButtonLabel( QPushButton* btn, QPainter *p);
    void getButtonShift( int &x, int &y ) const;

    // ToolButton
    void drawToolButton( QPainter *p, int x, int y, int w, int h,
		     const QColorGroup &g, bool on, bool down, bool enabled, 
		     bool autoRaised = FALSE, const QBrush *fill = 0 );
    void drawDropDownButton( QPainter *p, int x, int y, int w, int h,
		     const QColorGroup &g, bool down, bool enabled, bool autoRaised = FALSE,
		     const QBrush *fill = 0 );

    // RadioButton
    QSize exclusiveIndicatorSize() const;

    void drawExclusiveIndicator( QPainter* p, int x, int y, int w, int h,
		    const QColorGroup &g, bool on, bool down = FALSE, bool enabled = TRUE );

    // CheckBox
    QSize indicatorSize() const;

    void drawIndicator( QPainter* p, int x, int y, int w, int h, const QColorGroup &g,
			    int state, bool down = FALSE, bool enabled = TRUE );

    // ComboBox
    void drawComboButton( QPainter *p, int x, int y, int w, int h,
		      const QColorGroup &g, bool sunken = FALSE,
		      bool editable = FALSE, bool enabled = TRUE, const QBrush *fill = 0 );

    // ToolBar
    virtual int  toolBarHandleExtent() const;
    virtual void drawToolBarHandle( QPainter *p, const QRect &r,
				    Qt::Orientation orientation,
				    bool highlight, const QColorGroup &cg,
				    bool drawBorder = FALSE );
    virtual int  toolBarFrameWidth() const;
    virtual void drawToolBarPanel( QPainter *p, int x, int y, int w, int h,
				   const QColorGroup &, const QBrush *fill = 0 );
    void drawToolBarSeparator( QPainter *p, int x, int y, int w, int h,
				       const QColorGroup & g, Orientation orientation );
    QSize toolBarSeparatorSize( Qt::Orientation orientation ) const;

    // TabBar
    void drawTab( QPainter*, const QTabBar*, QTab*, bool selected );
    void drawTabBarExtension( QPainter * p, int x, int y, int w, int h,
				      const QColorGroup & cg, const QTabWidget * tw );

    // ScrollBar
    void drawScrollBarControls( QPainter*,  const QScrollBar*,
			    int sliderStart, uint controls, uint activeControl );


    // Slider
    void drawSlider( QPainter *p,
			    int x, int y, int w, int h,
			    const QColorGroup &g,
			    Orientation, bool tickAbove, bool tickBelow);

    void drawSliderGroove( QPainter *p,
			    int x, int y, int w, int h,
			    const QColorGroup& g, QCOORD c,
			    Orientation );

    // Splitter
    void drawSplitter( QPainter *p,
			int x, int y, int w, int h,
			const QColorGroup &g,
			Orientation);

    // Popup Menu
    void drawCheckMark( QPainter *p, int x, int y, int w, int h,
			const QColorGroup &g, bool act, bool dis );

    void drawPopupMenuItem( QPainter* p, bool checkable,
			int maxpmw, int tab, QMenuItem* mi,
			const QPalette& pal,
			bool act, bool enabled,
			int x, int y, int w, int h);

    // MenuBar
    void drawMenuBarItem( QPainter* p, int x, int y, int w, int h,
			QMenuItem* mi, QColorGroup& g, bool active,
			bool down, bool hasFocus = FALSE );
    void drawMenuBarPanel( QPainter *p, int x, int y, int w, int h,
			const QColorGroup &, const QBrush *fill = 0 );

    // TitleBar
    void drawTitleBar( QPainter *p, int x, int y, int w, int h, const QColor &left, const QColor &right, 
		       bool active );
    void drawTitleBarLabel( QPainter *p, int x, int y, int w, int h, const QString &text, 
		       const QColor &tc, bool active );
    void drawTitleBarButton( QPainter *p, int x, int y, int w, int h, const QColorGroup &g, bool down );
    void drawTitleBarButtonLabel( QPainter *p, int x, int y, int w, int h, const QPixmap *, int button, bool down );

    // Header
    void drawHeaderSection( QPainter *p, int x, int y, int w, int h, const QColorGroup &g, bool down );

    // spinbox
    int spinBoxFrameWidth() const;
    void drawSpinBoxButton( QPainter *p, int x, int y, int w, int h, const QColorGroup &g, 
			const QSpinBox *sp, bool upDown, bool enabled, bool down );
    void drawSpinBoxSymbol( QPainter *p, int x, int y, int w, int h, const QColorGroup &g, 
			const QSpinBox *sp, bool upDown, bool enabled, bool down );

    // groupbox
    void drawGroupBoxTitle( QPainter *p, int x, int y, int w, int h, const QColorGroup &g, const QString &text, bool enabled );
    void drawGroupBoxFrame( QPainter *p, int x, int y, int w, int h, const QColorGroup &g, const QGroupBox *gb );

    // statusbar
    void drawStatusBarSection( QPainter *p, int x, int y, int w, int h, const QColorGroup &g, bool permanent );
    void drawSizeGrip( QPainter *p, int x, int y, int w, int h, const QColorGroup &g );

    // progressbar
    int progressChunkWidth() const;
    void drawProgressBar( QPainter *p, int x, int y, int w, int h, const QColorGroup &g );
    void drawProgressChunk( QPainter *p, int x, int y, int w, int h, const QColorGroup &g );

protected:
    bool eventFilter( QObject *o, QEvent *e );

private:	// Disabled copy constructor and operator=
    class Private;
    Private *d;

#if defined(Q_DISABLE_COPY)
    QWindowsXPStyle( const QWindowsXPStyle & );
    QWindowsXPStyle& operator=( const QWindowsXPStyle & );
#endif
};

#endif // QT_NO_STYLE_WINDOWSXP

#endif // QWINDOWSXPSTYLE_H
