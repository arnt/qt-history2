#include <qpixmap.h>
#include <qapplication.h>
#include <stdio.h>
#include <qt_x11.h>


static int highest_bit( uint v )
{
    int i;
    uint b = (uint)1 << 31;
    for ( i=31; ((b & v) == 0) && i>=0;	 i-- )
	b >>= 1;
    return i;
}

static int lowest_bit( uint v )
{
    int i;
    ulong lb;
    lb = 1;
    for (i=0; ((v & lb) == 0) && i<32;  i++, lb<<=1);
    return i==32 ? -1 : i;
}

static uint n_bits( uint v )
{
    int i = 0;
    while ( v ) {
	v = v & (v - 1);
	i++;
    }
    return i;
}


int main( int argc, char **argv )
{
    QApplication a(argc,argv);
    QPixmap pm(8,8);
    Visual *visual = (Visual*)pm.x11Visual();

    uint red_mask	= (uint)visual->red_mask;
    uint green_mask	= (uint)visual->green_mask;
    uint blue_mask	= (uint)visual->blue_mask;

    uint red_bits	= n_bits( red_mask );
    uint green_bits	= n_bits( green_mask );
    uint blue_bits	= n_bits( blue_mask );

    int  red_shift	= lowest_bit( red_mask );
    int  green_shift	= lowest_bit( green_mask );
    int  blue_shift	= lowest_bit( blue_mask );

    const char *sc = "<unknown>";
    switch ( visual->c_class ) {
	case StaticGray	: sc = "StaticGray"; break;
	case GrayScale:   sc = "GrayScale"; break;
	case StaticColor: sc = "StaticColor"; break;
	case PseudoColor: sc = "PseudoColor"; break;
	case TrueColor:	  sc = "TrueColor"; break;
	case DirectColor: sc = "DirectColor"; break;
    }
    printf( "  Visual id = 0x%x\n", visual->visualid );
    printf( "  scr class = %s\n", sc );
    printf( "  Component   Bits   Shift   Mask\n" );
    printf( "    red       %2d     %2d     %.8x\n",
	    red_bits, red_shift, red_mask );
    printf( "   green      %2d     %2d     %.8x\n",
	    green_bits, green_shift, green_mask );
    printf( "    blue      %2d     %2d     %.8x\n",
	    blue_bits, blue_shift, blue_mask );
}
