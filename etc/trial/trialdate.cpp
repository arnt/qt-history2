#include <stdio.h>
#include <qdatetm.h>

class D : public QDate
{
public:
    static int g2j(int y, int m, int d ) { return greg2jul(y,m,d);}
};

int main()
{
    int j = D::g2j( 1998,
		    5,
		    1 );
    debug( "hex = %x, dec = %d", j, j );
}
