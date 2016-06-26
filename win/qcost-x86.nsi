!define VERSION "0.8.2"
Name "QCost ${VERSION}"
OutFile "QCost Installer - ${VERSION} - x86.exe"

; The default installation directory
InstallDir $PROGRAMFILES\QCost

; The text to prompt the user to enter a directory
DirText "This will install QCost on your computer. Choose a directory"

Section "QCost"
  SetOutPath $INSTDIR
  File "QCost.exe"
  File "QCost.ico"
  File "LICENSE"
  File "LICENSE.GPLv3"
  File /r doc
  File /r epa
  File /r iconengines
  File /r imageformats
  File /r platforms
  File /r sqldrivers
  File /r translations
  File D3Dcompiler_47.dll 
  File libEGL.dll
  File libGLESV2.dll
  File opengl32sw.dll
  File Qt5Widgets.dll
  File Qt5Core.dll
  File Qt5Gui.dll
  File Qt5Sql.dll
  File Qt5Svg.dll
  ReadRegStr $1 HKLM "SOFTWARE\Microsoft\DevDiv\vc\Servicing\12.0\RuntimeMinimum" "Install"
  StrCmp $1 1 vcrinstalled
  ExecWait 'vcredist_x86.exe'
  vcrinstalled:
  WriteUninstaller $INSTDIR\Uninstall.exe
  createDirectory "$SMPROGRAMS\QCost"
  CreateShortCut "$SMPROGRAMS\QCost\QCost.lnk" "$INSTDIR\QCost.exe" "" "$INSTDIR\QCost.ico"
  CreateShortCut "$SMPROGRAMS\QCost\Uninstall QCost.lnk" "$INSTDIR\Uninstall.exe" ""
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\QCost" \ 
               "DisplayName" "QCost"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\QCost" \ 
               "Publisher" "IngegneriaLibera"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\QCost" \
               "UninstallString" "$\"$INSTDIR\Uninstall.exe$\""
SectionEnd

Section "Uninstall"
  Delete $INSTDIR\Uninstall.exe
  RMDir /r $INSTDIR
  RMDir /r "$SMPROGRAMS\QCost"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\QCost"
SectionEnd 