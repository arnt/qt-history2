#
# creates a qt.conf file in $QTDIR
# push "c:\qt"  #QTDIR
# call MakeQtConfFile
#
Function MakeQtConfFile
  exch $1 ; QTDIR
  push $0 ; file handle
  push $2 ; converted qtdir
  
  push $1
  push "\"
  push "\\"
  call ReplaceString
  pop $2

  ClearErrors
  FileOpen $0 "$1\qt.conf" w
  IfErrors done

  FileWrite $0 "[Paths]$\r$\n"
  FileWrite $0 "Prefix = $2$\r$\n"
  FileWrite $0 "Binaries = $2\\bin$\r$\n"
  FileWrite $0 "Documentation = $2\\doc$\r$\n"
  FileWrite $0 "Headers = $2\\include$\r$\n"
  FileWrite $0 "Libraries = $2\\lib$\r$\n"
  FileWrite $0 "Plugins = $2\\plugins$\r$\n"
  FileWrite $0 "Data = $2$\r$\n"
  FileWrite $0 "Translations = $2\\translations$\r$\n"
  FileClose $0

  done:
  pop $2
  pop $0
  pop $1
FunctionEnd

#
# patch paths in text files
# push "qtcore4.prl"  #Filename
# push "c:\compile" #OLD_QTDIR
# push "c:\qt"  #QTDIR
# call PatchPath
#
Function PatchPath
  exch $2 ;NEW
  exch 2
  exch $1 ;Filename
  exch
  exch $0 ;OLD
  push $3 ;readline
  push $4 ;file 1
  push $5 ;file 2
  push $6 ;tmpfilename

  push $7 ;forward slash NEW
  push $8 ;forward slash OLD

  push $2
  push "\"
  push "/"
  call ReplaceString
  pop $7

  push $0
  push "\"
  push "/"
  call ReplaceString
  pop $8

  ClearErrors
  GetTempFileName $6
  FileOpen $5 $6 w
  FileOpen $4 $1 r
  IfErrors done

nextline:
  FileRead $4 $3
  IfErrors renameFile
  push $3
  push $0
  push $2
  call ReplaceString ;replace backward slash path
  push $8
  push $7
  call ReplaceString ;replace forward slash path
  pop $3
  FileWrite $5 $3
  goto nextline

renameFile:
  FileClose $5
  FileClose $4
  Delete $1
  Rename $6 $1
  
done:
  pop $8
  pop $7
  pop $6
  pop $5
  pop $4
  pop $3
  pop $0
  pop $1
  pop $2
FunctionEnd

#
# replaces a string with another string
# push string
# push "c:\qt"  #replace
# push "c:\compile" #with
# call ReplaceString
# pop $0 #new string
#
Function ReplaceString
  exch $2 ;NEW
  exch 2
  exch $1 ;string
  exch
  exch $0 ;OLD

  push $3 ; tmp string
  push $4 ; counter
  push $5 ; result
  
  push $6 ; old strlen

  StrCpy $4 "-1"
  StrCpy $5 ""
  
  StrLen $6 $0

  loop:
  IntOp $4 $4 + 1 ;increase counter
  StrCpy $3 $1 $6 $4 ;get substring
  StrCmp $3 "" done ; check for end
  StrCmp $3 $0 replace ;replace if old
  StrCpy $3 $1 "1" $4
  StrCpy $5 $5$3 ;append character to result
  goto loop

  replace:
  StrCpy $5 $5$2 ;insert new qtdir
  IntOp $4 $4 + $6 ;increase offset
  IntOp $4 $4 - 1 ;decrease offset one more
  goto loop

  done:
  StrCpy $2 $5
  pop $6
  pop $5
  pop $4
  pop $3
  pop $0
  pop $1
  exch $2
FunctionEnd