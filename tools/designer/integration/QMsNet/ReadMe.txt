======================================================================
  QMsNet AddIn - Qt integration for Microsoft Visual Studio .NET
======================================================================

This AddIn can only be used with Visual Studio .NET 2002 and later.


0. Prerequisites
=================

This setup does not contain the .NET Framework which must be installed
on the target machine by running dotnetfx.exe before this AddIn will
install. You can find dotnetfx.exe on the Visual Studio .NET 'Windows
Components Update' media.


1. Building the DLL
====================

Open the QMsNet.sln solution file in your Visual Studio .NET IDE.
Select the Release configuration and run Build Solution (F7). This
will create the installation package for QMsNet.
(Debug configuration will _not_ build the installation package)


2. Installing
==============

After building the package, as described above, right click on the
'QMsNetSetup' project, and select 'Install'. The Microsoft installer
will now take you through the necessary steps for a propper
installation.


3. Uninstalling
================

Either:
1. Right click on the 'QMsNetSetup' project, and select 'Uninstall'.
   The Microsoft installer will now take you through the necessary
   steps for a propper uninstallation.
or
2. Open 'Add or Remove Programs' in the Controll Panel, select
   'QMsNet', and choose 'Remove'. The Microsoft installer will now
   take you through the necessary steps for a propper uninstallation.


4. Usage
=========

Please see the final step in the installation process for more on
this, or open the Usage.rtf in the AddIn directory.
