SciTech SNAP Graphics for Qt Embedded Getting Started Guide


What is this document for?
--------------------------

This text document is intended to be a brief overview to explain how
to install and use the SciTech SNAP Graphics device drivers with the
Qt Embedded GUI toolkit under Linux.


System Requirements
-------------------

The SciTech SNAP Graphics device drivers support most modern versions
of Linux with glibc 2.1 or later installed. In order to use the
SciTech SNAP drivers with Qt Embedded, you will need the following:

 o A copy of SciTech SNAP Graphics for Linux version 2.0 or later.

 o A copy of the SciTech SNAP Graphics SDK R12 or later installed and
   configured on your machine. The Qt Embedded support for SciTech
   SNAP Graphics will *not* work with earlier versions of the SNAP
   Graphics SDK.

For more complete documentation on installing and using the SciTech
SNAP Graphics SDK, please refer to the online HTML documentation
pages found here:

 http://www.scitechsoft.com/docs/snap_ga

The latest versions of the SciTech SNAP Graphics SDK and device
drivers can be downloaded here:

 http://www.scitechsoft.com/products/product_download.html


Building SNAP SDK
-----------------

You should refer to the online HTML documentation referenced above
for complete information, but here is a very quick guide of what is
needed for using the SNAP SDK with Qt Embedded.

Extract the SNAP SDK archives in ~/scitech, then type these commands:

  # cd ~/scitech
  # . start-sdk.linux
  # cd ~/scitech/src/pm
  # dmake install BUILD_DLL=1

Assuming everything above worked correctly, the SNAP SDK PM library
is now ready for use with Qt Embedded.


Configuring Qt Embedded
-----------------------

There are two options you can use to enable support for SciTech SNAP
Graphics in Qt Embedded. The first option is to enable the support as
a static driver linked into the Qt Embedded library. To enable this,
use the following to configure Qt Embedded:

 ./configure -qt-gfx-snap

The second option is to enable support as a plugin driver module that
can be placed into the 'gfxdrivers' directory in your Qt Embedded
plugin directory. To enable the plugin version of the driver, use
the following:

 ./configure -plugin-gfx-snap

Of course you can add whatever other configuration options you need
such as enabling debugging support, linking shared or static etc.
When the configure process has completed, simply recompile the
libraries from scratch and you are set! You may want to first do a
'make confclean' before you reconfigure Qt Embedded and recompile.


Running Qt Embedded with SciTech SNAP
-------------------------------------

Once you have successfully rebuilt your copy of Qt Embedded, you can
try running the sample programs or your own programs using the new
libraries that contain the SciTech SNAP support. The biggest
difference between the SciTech SNAP drivers and the regular Qt
Embedded drivers is that you do not need your system to be in a
framebuffer console mode. The SciTech SNAP drivers will work either
in a regular hardware text mode console or in a graphical framebuffer
console mode.

One thing that you can control with the SNAP version is exactly what
resolution and color depth you wish to run your applications in. By
default the SciTech SNAP drivers for Qt Embedded will try to use
1024x768x16, then 800x600x16, then 640x480x16 and finally 640x480x8,
depending on what the underlying graphics hardware is capable of. If
you wish to manually specify the resolution and color depth to use,
you can control that with the QWS_SIZE environment variable. For
instance:

 export QWS_SIZE=1280x1024

will attempt to set the resolution to 1280x1024, but continue to use
16 bits per pixel for the color depth. Using:

 export QWS_SIZE=960x720x32

will attempt to set the resolution to 960x720 with 32-bits per pixel
as the color depth (make sure you have first configured 32 bit color
depth support in Qt Embedded when you compiled it!). That is about
it! Everything else is completely automatic, including the detection
of the underlying graphics hardware and attached monitor.
