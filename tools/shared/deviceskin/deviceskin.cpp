/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "deviceskin.h"

#include <QtCore/qnamespace.h>
#include <QtGui/QApplication>
#include <QtGui/QBitmap>
#include <QtGui/QPixmap>
#include <QtGui/QPainter>
#include <QtCore/QTextStream>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtGui/QImage>
#include <QtCore/QTimer>
#include <QtCore/QDir>
#include <QtCore/QRegExp>
#include <QtGui/QMouseEvent>
#include <QtCore/QDebug>

#ifdef TEST_SKIN
#  include <QtGui/QMainWindow>
#  include <QtGui/QDialog>
#  include <QtGui/QDialogButtonBox>
#  include <QtGui/QHBoxLayout>
#endif

namespace {
    enum { joydistance = 10, key_repeat_period = 50, key_repeat_delay = 500 };
    enum { debugDeviceSkin = 0 };
}

static void parseRect(const QString &value, QRect *rect) {
    const QStringList l = value.split(QLatin1Char(' '));
    rect->setRect(l[0].toInt(), l[1].toInt(), l[2].toInt(), l[3].toInt());
}

static QString msgImageNotLoaded(const QString &f)        {
    return DeviceSkin::tr("The image file '%1' could not be loaded.").arg(f);
}

// ------------ DeviceSkinButtonArea
DeviceSkinButtonArea::DeviceSkinButtonArea() :
    keyCode(0),
    activeWhenClosed(0)
{
}

QDebug &operator<<(QDebug &str, const DeviceSkinButtonArea &a)
{

    str << "Area: " <<  a.name << " keyCode=" << a.keyCode << " area=" <<  a.area
        << " text=" << a.text << " activeWhenClosed=" << a.activeWhenClosed;
    return str;
}

// ------------  DeviceSkinParameters

QDebug operator<<(QDebug str, const DeviceSkinParameters &p)
{
    str << "Images " << p.skinImageUpFileName << ','
	<< p.skinImageDownFileName<< ',' << p.skinImageClosedFileName
	<<  ',' <<  p.skinCursorFileName <<"\nScreen: " << p.screenRect
	<< " back: " << p.backScreenRect << " closed: " << p.closedScreenRect
	<< " cursor: " << p.cursorHot << " Prefix: " <<  p.prefix
	<< " Joystick: " << p.joystick;
    const int numAreas = p.buttonAreas.size();
    for (int i = 0; i < numAreas; i++)
	str <<  p.buttonAreas[i];
    return str;
}

QSize DeviceSkinParameters::secondaryScreenSize() const
{
    return backScreenRect.isNull() ?  closedScreenRect .size(): backScreenRect.size();
}

bool DeviceSkinParameters::hasSecondaryScreen() const
{
    return secondaryScreenSize() != QSize(0, 0);
}

bool DeviceSkinParameters::read(const QString &skinDirectory,  ReadMode rm,  QString *errorMessage)
{
    // Figure out the name. remove ending '/' if present
    QString skinFile = skinDirectory;
    if (skinFile.endsWith(QLatin1Char('/')))
        skinFile.truncate(skinFile.length() - 1);

    QFileInfo fi(skinFile);
    QString fn;
    if ( fi.isDir() ) {
        prefix = skinFile;
        prefix += QLatin1Char('/');
        fn = prefix;
        fn += fi.baseName();
        fn += QLatin1String(".skin");
    } else if (fi.isFile()){
        fn = skinFile;
        prefix = fi.path();
        prefix += QLatin1Char('/');
    } else {
        *errorMessage =  DeviceSkin::tr("The skin directory '%1' does not contain a configuration file.").arg(skinDirectory);
        return false;
    }
    QFile f(fn);
    if (!f.open(QIODevice::ReadOnly )) {
        *errorMessage =  DeviceSkin::tr("The skin configuration file '%1' could not be opened.").arg(fn);
        return false;
    }
    QTextStream ts(&f);
    const bool rc = read(ts, rm, errorMessage);
    if (!rc)
	*errorMessage =  DeviceSkin::tr("The skin configuration file '%1' could read: %2").arg(fn).arg(*errorMessage);
    return rc;
}
bool DeviceSkinParameters::read(QTextStream &ts, ReadMode rm, QString *errorMessage)
{
    QStringList closedAreas;
    int nareas = 0;
    QString mark;
    ts >> mark;
    if ( mark == QLatin1String("[SkinFile]") ) {
        const QString UpKey = QLatin1String("Up");
        const QString DownKey = QLatin1String("Down");
        const QString ClosedKey = QLatin1String("Closed");
        const QString ClosedAreasKey = QLatin1String("ClosedAreas");
        const QString ScreenKey = QLatin1String("Screen");
        const QString BackScreenKey = QLatin1String("BackScreen");
        const QString ClosedScreenKey = QLatin1String("ClosedScreen");
        const QString CursorKey = QLatin1String("Cursor");
        const QString AreasKey = QLatin1String("Areas");
        // New
        while (!nareas) {
            QString line = ts.readLine();
            if ( line.isNull() )
                break;
            if ( line[0] != QLatin1Char('#') && !line.isEmpty() ) {
                int eq = line.indexOf(QLatin1Char('='));
                if ( eq >= 0 ) {
                    const QString key = line.left(eq);
                    eq++;
                    while (eq<line.length()-1 && line[eq].isSpace())
                        eq++;
                    const QString value = line.mid(eq);
                    if ( key == UpKey ) {
                        skinImageUpFileName = value;
                    } else if ( key == DownKey ) {
                        skinImageDownFileName = value;
                    } else if ( key ==  ClosedKey ) {
                        skinImageClosedFileName = value;
                    } else if ( key == ClosedAreasKey ) {
                        closedAreas = value.split(QLatin1Char(' '));
                    } else if ( key == ScreenKey ) {
                        parseRect( value, &screenRect);
                    } else if ( key == BackScreenKey ) {
                        parseRect(value, &backScreenRect);
                    } else if ( key == ClosedScreenKey ) {
                        parseRect( value, &closedScreenRect );
                    } else if ( key == CursorKey ) {
                        QStringList l = value.split(QLatin1Char(' '));
                        skinCursorFileName = l[0];
                        cursorHot = QPoint(l[1].toInt(),l[2].toInt());
                    } else if ( key == AreasKey ) {
                        nareas = value.toInt();
                    }
                } else {
                    *errorMessage =  DeviceSkin::tr("Syntax error: %1").arg(line);
                    return false;
                }
            }
        }
    } else {
        // Old
        skinImageUpFileName = mark;
        QString s;
        int x,y,w,h,na;
        ts >> s >> x >> y >> w >> h >> na;
        skinImageDownFileName = s;
        screenRect.setRect(x, y, w, h);
        nareas = na;
    }
    // Done for short mode
    if (rm ==  ReadSizeOnly)
        return true;
    //  verify skin files exist
    skinImageUpFileName.insert(0, prefix);
    if (!QFile(skinImageUpFileName).exists()) {
        *errorMessage =  DeviceSkin::tr("The skin up image file '%1' does not exist.").arg(skinImageUpFileName);
        return false;
    }
    if (!skinImageUp.load(skinImageUpFileName)) {
        *errorMessage = msgImageNotLoaded(skinImageUpFileName);
        return false;
    }

    skinImageDownFileName.insert(0, prefix);
    if (!QFile(skinImageDownFileName).exists()) {
        *errorMessage =  DeviceSkin::tr("The skin down image file '%1' does not exist.").arg(skinImageDownFileName);
        return false;
    }
    if (!skinImageDown.load(skinImageDownFileName)) {
        *errorMessage = msgImageNotLoaded(skinImageDownFileName);
        return false;
    }

    if (!skinImageClosedFileName.isEmpty()) {
        skinImageClosedFileName.insert(0, prefix);
        if (!QFile(skinImageClosedFileName).exists()) {
            *errorMessage =  DeviceSkin::tr("The skin closed image file '%1' does not exist.").arg(skinImageClosedFileName);
            return false;
        }
        if (!skinImageClosed.load(skinImageClosedFileName)) {
            *errorMessage = msgImageNotLoaded(skinImageClosedFileName);
            return false;
        }
    }

    if (!skinCursorFileName.isEmpty()) {
        skinCursorFileName.insert(0, prefix);
        if (!QFile(skinCursorFileName).exists()) {
            *errorMessage =  DeviceSkin::tr("The skin cursor image file '%1' does not exist.").arg(skinCursorFileName);
            return false;
        }
        if (!skinCursor.load(skinCursorFileName)) {
            *errorMessage = msgImageNotLoaded(skinCursorFileName);
            return false;
        }
    }

    // read areas
    if (!nareas)
        return true;
    buttonAreas.reserve(nareas);

    int i = 0;
    ts.readLine(); // eol
    joystick = -1;
    const QString Joystick = QLatin1String("Joystick");
    while (i < nareas && !ts.atEnd() ) {
        buttonAreas.push_back(DeviceSkinButtonArea());
        DeviceSkinButtonArea &area = buttonAreas.back();
        const QString line = ts.readLine();
        if ( line[0] != QLatin1Char('#') && !line.isEmpty() ) {
            const QStringList tok = line.split(QRegExp(QLatin1String("[ \t][ \t]*")));
            if ( tok.count()<6 ) {
                *errorMessage =  DeviceSkin::tr("Syntax error in area definition: %1").arg(line);
                return false;
            } else {
                area.name = tok[0];
                QString k = tok[1];
                if ( k.left(2).toLower() == QLatin1String("0x")) {
                    area.keyCode = k.mid(2).toInt(0,16);
                } else {
                    area.keyCode = k.toInt();
                }

                int p=0;
                for (int j=2; j < tok.count() - 1; ) {
                    const int x = tok[j++].toInt();
                    const int y = tok[j++].toInt();
                    area.area.putPoints(p++,1,x,y);
                }

                const QChar doubleQuote = QLatin1Char('"');
                if ( area.name[0] == doubleQuote && area.name.endsWith(doubleQuote)) {
                    area.name.truncate(area.name.size() - 1);
                    area.name.remove(0, 1);
                }
                if ( area.name.length() == 1 )
                    area.text = area.name;
                if ( area.name == Joystick)
                    joystick = i;
                area.activeWhenClosed = closedAreas.contains(area.name)
                    || area.keyCode == Qt::Key_Flip; // must be to work
                i++;
            }
        }
    }
    if (i != nareas) {
	const QString message = DeviceSkin::tr("Mismatch in number of areas, expected %1, got %2.").arg(nareas).arg(i);
	qWarning(message.toUtf8().constData());
    }
    if (debugDeviceSkin)
	qDebug() << *this;
    return true;
}

// --------- CursorWindow declaration

namespace qvfb_internal {

class CursorWindow : public QWidget
{
public:
    explicit CursorWindow(const QImage &cursor, QPoint hot, QWidget *sk);

    void setView(QWidget*);
    void setPos(QPoint);
    bool handleMouseEvent(QEvent *ev);

protected:
    bool event( QEvent *);
    bool eventFilter( QObject*, QEvent *);

private:
    QWidget *mouseRecipient;
    QWidget *m_view;
    QWidget *skin;
    QPoint hotspot;
};
}

// --------- Skin

DeviceSkin::DeviceSkin(const DeviceSkinParameters &parameters,  QWidget *p ) :
    QWidget(p),
    m_parameters(parameters),
    buttonRegions(parameters.buttonAreas.size(), QRegion()),
    parent(p),
    m_view(0),
    m_secondaryView(0),
    buttonPressed(false),
    buttonIndex(0),
    zoom(1.0),
    cursorw(0),
    joydown(0),
    t_skinkey(new QTimer(this)),
    t_parentmove(new QTimer(this)),
    flipped_open(true)
{
    Q_ASSERT(p);
    setMouseTracking(true);
    setAttribute(Qt::WA_NoSystemBackground);

    setZoom(1.0);
    connect( t_skinkey, SIGNAL(timeout()), this, SLOT(skinKeyRepeat()) );
    t_parentmove->setSingleShot( true );
    connect( t_parentmove, SIGNAL(timeout()), this, SLOT(moveParent()) );
}

void DeviceSkin::skinKeyRepeat()
{
    if ( m_view ) {
	const DeviceSkinButtonArea &area = m_parameters.buttonAreas[buttonIndex];
	emit skinKeyReleaseEvent( area.keyCode,area.text, true );
	emit skinKeyPressEvent( area.keyCode, area.text, true );
	t_skinkey->start(key_repeat_period);
    }
}

void DeviceSkin::calcRegions()
{
    const int numAreas = m_parameters.buttonAreas.size();
    for (int i=0; i<numAreas; i++) {
	QPolygon xa(m_parameters.buttonAreas[i].area.count());
	int n = m_parameters.buttonAreas[i].area.count();
	for (int p=0; p<n; p++) {
	    xa.setPoint(p,
		int(m_parameters.buttonAreas[i].area[p].x()*zoom),
		int(m_parameters.buttonAreas[i].area[p].y()*zoom));
	}
	if ( n == 2 ) {
	    buttonRegions[i] = QRegion(xa.boundingRect());
	} else {
	    buttonRegions[i] = QRegion(xa);
	}
    }
}

void DeviceSkin::loadImages()
{
    QImage iup = m_parameters.skinImageUp;
    QImage idown = m_parameters.skinImageDown;

    QImage iclosed;
    const bool hasClosedImage = !m_parameters.skinImageClosed.isNull();

    if (hasClosedImage)
	iclosed =  m_parameters.skinImageClosed;
    QImage icurs;
    const bool hasCursorImage = !m_parameters.skinCursor.isNull();
    if (hasCursorImage)
	icurs =  m_parameters.skinCursor;

    if ( zoom != int(zoom) ) {
	iup = iup.scaled(int(iup.width()*zoom),int(iup.height()*zoom), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	idown = idown.scaled(int(idown.width()*zoom),int(idown.height()*zoom), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	if (hasClosedImage)
	    iclosed = iclosed.scaled(int(idown.width()*zoom),int(idown.height()*zoom), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	if (hasCursorImage)
	    icurs = icurs.scaled(int(idown.width()*zoom),int(idown.height()*zoom), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
    const Qt::ImageConversionFlags conv = Qt::ThresholdAlphaDither|Qt::AvoidDither;
    skinImageUp = QPixmap::fromImage(iup);
    skinImageDown = QPixmap::fromImage(idown, conv);
    if (hasClosedImage)
	skinImageClosed = QPixmap::fromImage(iclosed, conv);
    if (hasCursorImage)
	skinCursor = QPixmap::fromImage(icurs, conv);

    if ( zoom == int(zoom) ) {
	QMatrix scale;
	scale = scale.scale(zoom,zoom);
	skinImageUp = skinImageUp.transformed(scale);
	skinImageDown = skinImageDown.transformed(scale);
	if (hasClosedImage)
	    skinImageClosed = skinImageClosed.transformed(scale);
	if (hasCursorImage)
	    skinCursor = skinCursor.transformed(scale);
    }

    setFixedSize( skinImageUp.size() );
    if (!skinImageUp.mask())
	skinImageUp.setMask(skinImageUp.createHeuristicMask());
    if (!skinImageClosed.mask())
	skinImageClosed.setMask(skinImageClosed.createHeuristicMask());

    QWidget* parent = parentWidget();
    parent->setMask( skinImageUp.mask() );
    parent->setFixedSize( skinImageUp.size() );

    delete cursorw;
    cursorw = 0;
    if (hasCursorImage) {
	cursorw = new qvfb_internal::CursorWindow(m_parameters.skinCursor, m_parameters.cursorHot, this);
	if ( m_view )
	    cursorw->setView(m_view);
    }
}

DeviceSkin::~DeviceSkin( )
{
    delete cursorw;
}

void DeviceSkin::setZoom( double z )
{
    zoom = z;
    calcRegions();
    loadImages();
    if ( m_view )
	m_view->move( int(m_parameters.screenRect.x()*zoom), int(m_parameters.screenRect.y()*zoom) );
    updateSecondaryScreen();
}

void DeviceSkin::updateSecondaryScreen()
{
    if (!m_secondaryView)
        return;
    if (flipped_open) {
        if (m_parameters.backScreenRect.isNull()) {
            m_secondaryView->hide();
        } else {
            m_secondaryView->move( int(m_parameters.backScreenRect.x()*zoom), int(m_parameters.backScreenRect.y()*zoom) );
            m_secondaryView->show();
        }
    } else {
        if (m_parameters.closedScreenRect.isNull()) {
            m_secondaryView->hide();
        } else {
            m_secondaryView->move( int(m_parameters.closedScreenRect.x()*zoom), int(m_parameters.closedScreenRect.y()*zoom) );
            m_secondaryView->show();
        }
    }
}

void DeviceSkin::setView( QWidget *v )
{
    m_view = v;
    m_view->setFocus();
    m_view->move( int(m_parameters.screenRect.x()*zoom), int(m_parameters.screenRect.y()*zoom) );
    if ( cursorw )
	cursorw->setView(v);
}

void DeviceSkin::setSecondaryView( QWidget *v )
{
    m_secondaryView = v;
    updateSecondaryScreen();
}

void DeviceSkin::paintEvent( QPaintEvent *)
{
    QPainter p( this );
    if ( flipped_open ) {
	p.drawPixmap( 0, 0, skinImageUp );
    } else {
	p.drawPixmap( 0, 0, skinImageClosed );
    }
    if (buttonPressed == true) {
	const DeviceSkinButtonArea &ba = m_parameters.buttonAreas[buttonIndex];
        const QRect r = buttonRegions[buttonIndex].boundingRect();
        if ( ba.area.count() > 2 )
            p.setClipRegion(buttonRegions[buttonIndex]);
        p.drawPixmap( r.topLeft(), skinImageDown, r);
    }
}

void DeviceSkin::mousePressEvent( QMouseEvent *e )
{
    if (e->button() == Qt::RightButton) {
	emit popupMenu();
    } else {
	buttonPressed = false;

	onjoyrelease = -1;
	const int numAreas = m_parameters.buttonAreas.size();
        for (int i = 0; i < numAreas ; i++) {
	    const DeviceSkinButtonArea &ba = m_parameters.buttonAreas[i];
            if (  buttonRegions[i].contains( e->pos() ) ) {
                if ( flipped_open || ba.activeWhenClosed ) {
                    if ( m_parameters.joystick == i ) {
                        joydown = true;
                    } else {
                        if ( joydown )
                            onjoyrelease = i;
                        else
                            startPress(i);
                        break;
			if (debugDeviceSkin)// Debug message to be sure we are clicking the right areas
			    qDebug()<< m_parameters.buttonAreas[i].name << " clicked";
                    }
                }
            }
        }
	clickPos = e->pos();
//	This is handy for finding the areas to define rectangles for new skins
	if (debugDeviceSkin)
	    qDebug()<< "Clicked in " <<  e->pos().x() << ',' <<  e->pos().y();
	clickPos = e->pos();
    }
}

void DeviceSkin::flip(bool open)
{
    if ( flipped_open == open )
	return;
    if ( open ) {
	parent->setMask( skinImageUp.mask() );
	emit skinKeyReleaseEvent( Qt::Key(Qt::Key_Flip), QString(), false);
    } else {
	parent->setMask( skinImageClosed.mask() );
	emit skinKeyPressEvent( Qt::Key(Qt::Key_Flip), QString(), false);
    }
    flipped_open = open;
    updateSecondaryScreen();
    repaint();
}

void DeviceSkin::startPress(int i)
{
    buttonPressed = true;
    buttonIndex = i;
    if (m_view) {
	const DeviceSkinButtonArea &ba = m_parameters.buttonAreas[buttonIndex];
	if ( ba.keyCode == Qt::Key_Flip ) {
	    flip(!flipped_open);
	} else {
	    emit skinKeyPressEvent( ba.keyCode, ba.text, false);
	    t_skinkey->start(key_repeat_delay);
	}
	repaint( buttonRegions[buttonIndex].boundingRect() );
    }
}

void DeviceSkin::endPress()
{
    const DeviceSkinButtonArea &ba = m_parameters.buttonAreas[buttonIndex];
    if (m_view && ba.keyCode != Qt::Key_Flip )
	emit skinKeyReleaseEvent(ba.keyCode, ba.text, false );
    t_skinkey->stop();
    buttonPressed = false;
    repaint( buttonRegions[buttonIndex].boundingRect() );
}

void DeviceSkin::mouseMoveEvent( QMouseEvent *e )
{
    if ( e->buttons() & Qt::LeftButton ) {
	const int joystick = m_parameters.joystick;
	QPoint newpos =  e->globalPos() - clickPos;
	if ( joydown ) {
	    int k1=0, k2=0;
	    if ( newpos.x() < -joydistance ) {
		k1 = joystick+1;
	    } else if ( newpos.x() > +joydistance ) {
		k1 = joystick+3;
	    }
	    if ( newpos.y() < -joydistance ) {
		k2 = joystick+2;
	    } else if ( newpos.y() > +joydistance ) {
		k2 = joystick+4;
	    }
	    if ( k1 || k2 ) {
		if ( !buttonPressed ) {
		    onjoyrelease = -1;
		    if ( k1 && k2 ) {
			startPress(k2);
			endPress();
		    }
		    startPress(k1 ? k1 : k2);
		}
	    } else if ( buttonPressed ) {
		endPress();
	    }
	} else if ( buttonPressed == false ) {
	    parentpos = newpos;
	    if ( !t_parentmove->isActive() )
		t_parentmove->start(50);
	}
    }
    if ( cursorw )
	cursorw->setPos(e->globalPos());
}

void DeviceSkin::moveParent()
{
    parent->move( parentpos );
}

void DeviceSkin::mouseReleaseEvent( QMouseEvent * )
{
    if ( buttonPressed )
	endPress();
    if ( joydown ) {
	joydown = false;
	if ( onjoyrelease >= 0 ) {
	    startPress(onjoyrelease);
	    endPress();
	}
    }
}

bool DeviceSkin::hasCursor() const
{
    return !skinCursor.isNull();
}

// ------------------ CursorWindow implementation

namespace qvfb_internal {

bool CursorWindow::eventFilter( QObject *, QEvent *ev)
{
    handleMouseEvent(ev);
    return false;
}

bool CursorWindow::event( QEvent *ev )
{
    if (handleMouseEvent(ev))
        return true;
    return QWidget::event(ev);
}

bool CursorWindow::handleMouseEvent(QEvent *ev)
{
    bool handledEvent = false;
    static int inhere=0;
    if ( !inhere ) {
	inhere++;
	if ( m_view ) {
	    if ( ev->type() >= QEvent::MouseButtonPress && ev->type() <= QEvent::MouseMove ) {
		QMouseEvent *e = (QMouseEvent*)ev;
		QPoint gp = e->globalPos();
		QPoint vp = m_view->mapFromGlobal(gp);
		QPoint sp = skin->mapFromGlobal(gp);
		if ( e->type() == QEvent::MouseButtonPress || e->type() == QEvent::MouseButtonDblClick ) {
		    if ( m_view->rect().contains(vp) )
			mouseRecipient = m_view;
		    else if ( skin->parentWidget()->geometry().contains(gp) )
			mouseRecipient = skin;
		    else
			mouseRecipient = 0;
		}
		if ( mouseRecipient ) {
		    setPos(gp);
		    QMouseEvent me(e->type(),mouseRecipient==skin ? sp : vp,gp,e->button(),e->buttons(),e->modifiers());
		    QApplication::sendEvent(mouseRecipient, &me);
		} else if ( !skin->parentWidget()->geometry().contains(gp) ) {
		    hide();
		} else {
		    setPos(gp);
		}
		if ( e->type() == QEvent::MouseButtonRelease )
		    mouseRecipient = 0;
		handledEvent = true;
	    }
	}
	inhere--;
    }
    return handledEvent;
}

void CursorWindow::setView(QWidget* v)
{
    if ( m_view ) {
	m_view->removeEventFilter(this);
	m_view->removeEventFilter(this);
    }
    m_view = v;
    m_view->installEventFilter(this);
    m_view->installEventFilter(this);
    mouseRecipient = 0;
}

CursorWindow::CursorWindow(const QImage &img, QPoint hot, QWidget* sk)
	:QWidget(0),
	m_view(0), skin(sk),
	hotspot(hot)
{
    setWindowFlags( Qt::FramelessWindowHint );
    mouseRecipient = 0;
    setMouseTracking(true);
    setCursor(Qt::BlankCursor);
    QPixmap p;
    p = QPixmap::fromImage(img);
    if ( !p.mask() )
	if ( img.hasAlphaChannel() ) {
	    QBitmap bm;
	    bm = QPixmap::fromImage(img.createAlphaMask());
	    p.setMask( bm );
	} else {
	    QBitmap bm;
	    bm = QPixmap::fromImage(img.createHeuristicMask());
	    p.setMask( bm );
	}
    QPalette palette;
    palette.setBrush(backgroundRole(), QBrush(p));
    setPalette(palette);
    setFixedSize( p.size() );
    if ( !p.mask().isNull() )
	setMask( p.mask() );
}

void CursorWindow::setPos(QPoint p)
{
    move(p-hotspot);
    show();
    raise();
}
}

#ifdef TEST_SKIN

int main(int argc,char *argv[])
{
    if (argc < 1)
        return 1;
    const QString skinFile = QString::fromUtf8(argv[1]);
    QApplication app(argc,argv);
    QMainWindow mw;

    DeviceSkinParameters params;
    QString errorMessage;
    if (!params.read(skinFile, DeviceSkinParameters::ReadAll, &errorMessage)) {
	qWarning() << errorMessage;
	return 1;
    }
    DeviceSkin ds(params, &mw);
    // View Dialog
    QDialog *dialog = new QDialog();
    QHBoxLayout *dialogLayout = new QHBoxLayout();
    dialog->setLayout(dialogLayout);
    QDialogButtonBox *dialogButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    QObject::connect(dialogButtonBox, SIGNAL(rejected()), dialog, SLOT(reject()));
    QObject::connect(dialogButtonBox, SIGNAL(accepted()), dialog, SLOT(accept()));
    dialogLayout->addWidget(dialogButtonBox);
    dialog->setFixedSize(params.screenSize());
    dialog->setParent(&ds, Qt::SubWindow);
    dialog->setAutoFillBackground(true);
    ds.setView(dialog);

    QObject::connect(&ds, SIGNAL(popupMenu()), &mw, SLOT(close()));
    QObject::connect(&ds, SIGNAL(skinKeyPressEvent(int,QString,bool)), &mw, SLOT(close()));
    mw.show();
    return app.exec();
}

#endif
