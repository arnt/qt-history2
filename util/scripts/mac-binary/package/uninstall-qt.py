#!/usr/bin/env python

#############################################################################
##
## Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
##
## This file is part of the $MODULE$ of the Qt Toolkit.
##
## $LICENSE$
##
## This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
## WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
##
#############################################################################

import commands, sys, os, shutil

trace = True
justTest = False
# Globals
AllPackages = ['docs', 'headers', 'examples', 'plugins', 'tools', 'libraries']

packagesToRemove = AllPackages

# Travese the list and remove values that won't work.
def realPackagesToRemove(packageList):
    for package in packageList:
        if package not in AllPackages:
            print "%s is NOT a valid package, removing it from the list" % (package)
            packageList.remove(package)
    return packageList

#Remove the files in the list
def removeFiles(fileList):
    directories = []
    for file in fileList:
        file = file[1:]
        if not os.path.islink(file) and os.path.isdir(file):
            directories.append(file)
        elif len(file) > 0:
            if os.path.exists(file) or os.path.islink(file):
                if trace:
                    print "remove file: " + file
                if not justTest:
                    os.remove(file) 
            else:
                print "file: %s does not exist, skipping" % file


    # Now remove any empty directories
    directories.reverse()
    for dir in directories:
        if (os.path.exists(dir)) and len(os.listdir(dir)) == 0:
            if trace:
                print "remove dir: " + dir
            if not justTest:
                os.rmdir(dir)
        elif trace and os.path.exists(dir):
            print "NOT removing " + dir

# Remove the package
def removePackage(package):
    print "removing package " + package
    realPackageName = "/Library/Receipts/Qt_" + package + ".pkg"
    bomLocation = os.path.join(realPackageName, "Contents/Archive.bom")
    if os.path.exists(realPackageName) and os.path.isdir(realPackageName):
        fileList = commands.getoutput("/usr/bin/lsbom -f -p f -d -l " + bomLocation).split()
        if len(fileList) > 0:
            removeFiles(fileList)
            shutil.removetree(realPackageName)
    else:
        print "%s is not installed, skipping." % package


################# Here's where the actual script starts ########################################
if os.getuid() != 0:
    print sys.argv[0] + ": This script must be run as root or with sudo, exiting now."
    sys.exit(-1)

# Take the names of packages on the command-line
if len(sys.argv) > 1:
    packagesToRemove = sys.argv[1:]

packagesToRemove = realPackagesToRemove(packagesToRemove)

if len(packagesToRemove) < 1:
    print "\nNo valid packages to uninstall.\nusage: %s [package ...]" % (sys.argv[0])
    sys.exit(1)

for package in packagesToRemove:
    removePackage(package)
