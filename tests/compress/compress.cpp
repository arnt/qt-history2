#include <qimage.h>

int main(int argc, char ** argv)
{
  ASSERT( argc == 2 );

  QImage image( argv[1] );
  image.save( "toto-000.png", "PNG", 0 );
  image.save( "toto-100.png", "PNG", 100 );
  image.save( "toto-000.jpg", "JPEG", 0 );
  image.save( "toto-100.jpg", "JPEG", 100 );
  return 0;
}
