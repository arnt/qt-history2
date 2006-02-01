(* This script works by using FileStorm to create a disk image that you can then use as a model .DS_Store file in 
   the disk image. Here's what you need to do:
   
   Run FileStorm on Panther (10.3), if you don't do this, the disk image will look wrong
   on Panther later.
   
   Run this script, adjusting the path to the items and versions as necessary. If all items don't exist,
   then it won't finish correctly.
   
   Afterwords, mount the final disk image and make sure that it looks OK, sometimes FileStorm
   messes this up.
   
   If it's OK, copy /Volumes/Qt-x.y.z/.DS_Store to safe location (call it model-DS_Store.x.y.z)
   
   Edit this file and replace all occurences of backgroundXXXXXXXXXXXX (21 chars) with
   findersbackground (should be two places to correct).
   
   Save the file and you should be ready to go.
   
   I believe this is as automated as I can get it.
*)

tell application "FileStorm"
	activate
	make new document at before first document with properties {name:"diskimage"}
	
	tell first document
		set disk image name to "/Users/twschulz/Desktop/qt-mac-commercial-4.1.1.dmg"
		set {volume name} to {"Qt 4.1.1"}
		set icon path to "/Users/twschulz/troll/qt/4.1/util/scripts/mac-binary/package/backgrounds/DriveIcon.icns"
		set background image path to "/Users/twschulz/troll/qt/4.1/util/scripts/mac-binary/package/backgrounds/DiskImage-Commercial.png"
		set height to 660
		make new file at before first file with properties {file path:"/Users/twschulz/Desktop/foo/Qt.mpkg", left position:295, top position:95}
		make new file at before first file with properties {file path:"//Users/twschulz/Desktop/foo/ReadMe.txt", left position:295, top position:297}
		make new file at before first file with properties {file path:"/Users/twschulz/Desktop/foo/packages", left position:150, top position:640}
		tell application "FileStorm" to set bounds of window "diskimage" to {100, 100, 640, 660}
		finalize image with rebuilding
	end tell
	
end tell