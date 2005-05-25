#ifndef QX11EMBED_H
#define QX11EMBED_H

#include <qwidget.h>

class QX11EmbedWidgetPrivate;
class QX11EmbedWidget : public QWidget
{
    Q_OBJECT
public:
    QX11EmbedWidget(QWidget *parent = 0);
    ~QX11EmbedWidget();

    void embedInto(WId id);
    WId containerWinId() const;

    enum Error {
	Unknown,
	Internal,
	InvalidWindowID
    };
    Error error() const;
   
signals:
    void embedded();
    void containerClosed();
    void error(Error error);
    
protected:
    bool x11Event(XEvent *);
    bool eventFilter(QObject *, QEvent *);
    bool event(QEvent *);
    void resizeEvent(QResizeEvent *);
    
private:
    Q_DECLARE_PRIVATE(QX11EmbedWidget)
    Q_DISABLE_COPY(QX11EmbedWidget)
};

class QX11EmbedContainerPrivate;
class QX11EmbedContainer : public QWidget
{
    Q_OBJECT
public:
    QX11EmbedContainer(QWidget *parent = 0);
    ~QX11EmbedContainer();
        
    void embedClient(WId id);
    void discardClient();

    WId clientWinId() const;

    QSize minimumSizeHint() const;
    
    enum Error {
	Unknown,
	Internal,
	InvalidWindowID
    };
    Error error() const;

signals:
    void clientIsEmbedded();
    void clientClosed();
    void error(Error);
    
protected:
    bool x11Event(XEvent *);
    bool eventFilter(QObject *, QEvent *);
    void paintEvent(QPaintEvent *e);
    void resizeEvent(QResizeEvent *);
    void showEvent(QShowEvent *);
    void hideEvent(QHideEvent *);
    bool event(QEvent *);
    
private:
    Q_DECLARE_PRIVATE(QX11EmbedContainer)
    Q_DISABLE_COPY(QX11EmbedContainer)
};

#endif
