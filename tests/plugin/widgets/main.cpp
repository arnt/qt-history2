#include "../tools/designer/plugins/designerinterface.h"

#include <qapplication.h>
#include <qcleanuphandler.h>

#include <qcanvas.h>

/* XPM */
static const char * const canvas_xpm[] ={
"32 32 50 1",
". c None",
"# c #000040",
"B c #002040",
"N c #004000",
"M c #004040",
"J c #00c0c0",
"E c #00ffff",
"s c #202040",
"q c #204000",
"G c #204040",
"I c #206040",
"K c #20a080",
"D c #20c0c0",
"a c #402000",
"t c #402040",
"i c #404000",
"k c #404040",
"H c #406000",
"z c #406040",
"C c #40a080",
"F c #40c0c0",
"S c #602040",
"r c #604000",
"c c #606000",
"m c #606040",
"u c #606080",
"h c #608040",
"O c #802040",
"y c #806040",
"f c #806080",
"n c #808000",
"e c #808040",
"d c #808080",
"L c #80a040",
"U c #a08000",
"v c #a0a000",
"w c #a0a040",
"V c #a0a0a4",
"x c #a0c040",
"P c #c02080",
"A c #c0c000",
"j c #c0c040",
"T c #c0c0c0",
"R c #c0dcc0",
"p c #c0e040",
"Q c #e00080",
"o c #e0c040",
"l c #e0e000",
"b c #e0e040",
"g c #ffff00",
"..#..a..........................",
".bcd.......................efg..",
"..gfdh....................cdgb..",
"..badbeic...............jgjkb...",
"...bdlb.amc...........e.ngkk..nn",
"...bd.b.jopn........qrg.b.ke..nn",
"...gmbl.bj.be.....s...g.bldb.ann",
"...jtibab..licq...a...b.bbdb.mnn",
"....pellg..lni.....b..lllldb.ecc",
"....bulbb.clbvc...k.b..blbdwmvaa",
"....bdlbllbmbe....g.blgllmd#mn..",
"....jd..wbb.leaw.kbwjlb..mm.ei..",
".....dlllge.lx..m.lb.llggymmni..",
".....dbblg..bbj.j.eb.jlgbdgzna..",
".....kllbc.bbjb.j.Al..llgdcmn...",
"......ca..j.lia.i.Abie.aiAAmc...",
".......bgbe.ljm.j.Alibbb..n.....",
"......Bgbbb.l.i.jnplpblg.Ca.....",
".....bDbbig.mji.jasegbelqala....",
".....bEbb.g..ii.ja.bbmblFGjnH...",
"......Dbb.e..ni.iambe.elI.avi...",
".....gJ.bbw.kna..aa.blb.Kcb.....",
"......Dsbblwnci....bblw.Da.va...",
".....gI.qLegncc....abb..Fbxc....",
".....gMDDB..vvn.......CFFe.vi...",
".......ssNs.nnc......a.....v....",
".....gil.OPws.......QQ.l.pei....",
".....R.y.lbeS.......bbmb.mrc....",
"....jakT.minU.........aVnimeia..",
".....ci..dsa...........casm.ic..",
".....i...k..................ci..",
".....i...k..................ci.."};


class ExtraWidgetsInterface : public WidgetInterface
{
public:
    ExtraWidgetsInterface( QUnknownInterface *parent, const char *name = 0 );
    ~ExtraWidgetsInterface();

    bool cleanup();

    QStringList featureList() const;
    QWidget* create( const QString &classname, QWidget* parent = 0, const char* name = 0 );
    QString group( const QString& );
    QString iconSet( const QString& );
    QIconSet iconset( const QString& );
    QString includeFile( const QString& );
    QString toolTip( const QString& );
    QString whatsThis( const QString& );
    bool isContainer( const QString& );

    QGuardedCleanupHandler<QObject> objects;
};

ExtraWidgetsInterface::ExtraWidgetsInterface( QUnknownInterface *parent, const char *name )
: WidgetInterface( parent, name )
{
}

ExtraWidgetsInterface::~ExtraWidgetsInterface()
{
}

bool ExtraWidgetsInterface::cleanup()
{
    qDebug( "ExtraWidgetsInterface::cleanup()" );
    if ( !objects.isEmpty() )
	return FALSE;
    return TRUE;
}

QStringList ExtraWidgetsInterface::featureList() const
{
    QStringList list;

    list << "QCanvasView";

    return list;
}

QWidget* ExtraWidgetsInterface::create( const QString &description, QWidget* parent, const char* name )
{
    QWidget* w = 0;
    if ( description == "QCanvasView" ) {
	QCanvas* canvas = new QCanvas;
	objects.add( canvas );
	w = new QCanvasView( canvas, parent, name );
    }

    objects.add( w );
    return w;
}

QString ExtraWidgetsInterface::group( const QString& description )
{
    if ( description == "QCanvasView" )
	return "Views";

    return QString::null;
}

QString ExtraWidgetsInterface::iconSet( const QString& )
{
    return QString::null;
}

QIconSet ExtraWidgetsInterface::iconset( const QString& description )
{
    if ( description == "QCanvasView" )
	return QIconSet((const char**)canvas_xpm);

    return QIconSet();
}

QString ExtraWidgetsInterface::includeFile( const QString& )
{
    return "qcanvas.h";
}

QString ExtraWidgetsInterface::toolTip( const QString& )
{
    return QString::null;
}

QString ExtraWidgetsInterface::whatsThis( const QString& )
{
    return QString::null;
}

bool ExtraWidgetsInterface::isContainer( const QString& )
{ 
    return FALSE;
}

class ExtraWidgetsPlugIn : public QComponentInterface
{
public:
    ExtraWidgetsPlugIn();
    ~ExtraWidgetsPlugIn();
    QString name() const { return "Extra-Widgets plugin"; }
    QString description() const { return "QCanvas support for the Qt Designer"; }
    QString author() const { return "Trolltech"; }
};

ExtraWidgetsPlugIn::ExtraWidgetsPlugIn()
: QComponentInterface( "ExtraWidgetsPlugIn" )
{
    new ExtraWidgetsInterface( this, "ExtraWidgetsInterface" );
}

ExtraWidgetsPlugIn::~ExtraWidgetsPlugIn()
{
}

Q_EXPORT_INTERFACE(ExtraWidgetsPlugIn)

