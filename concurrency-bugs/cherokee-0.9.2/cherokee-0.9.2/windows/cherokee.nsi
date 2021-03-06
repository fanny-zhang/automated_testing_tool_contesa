!include "MUI.nsh"

Name "Cherokee Web Server"
OutFile "Cherokee-setup-0.9.2.exe"
InstallDir "$PROGRAMFILES\Cherokee"
DirText "Choose a folder in which to install Cherokee"
ShowInstDetails show

!define MUI_WELCOMEPAGE_TITLE "Welcome to the Cherokee Web Server Setup Wizard"
!define MUI_WELCOMEPAGE_TEXT "This wizard will guide you through the installation of Cherokee Web Server..\r\n\r\nWhile we do our best to be easy, fast and flexible, you need to let us know if we're succeeding.\r\n\r\nPlease, don't hesitate to email us on\r\n\r\n\tcherokee@cherokee-project.com\r\n\r\nand visit the web site\r\n\r\n\thttp://www.cherokee-project.com\r\n\r\n\r\n\r\n$_CLICK"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "../COPYING"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY

!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!undef MUI_ICON
!undef MUI_UNICON
!define MUI_ICON "cherokee_pkg.ico"
!define MUI_UNICON "cherokee_pkg.ico"

!define MUI_FINISHPAGE_LINK "Visit the Cherokee site for the latest news!!"
!define MUI_FINISHPAGE_LINK_LOCATION "http://www.cherokee-project.com/"

!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_LANGUAGE "English"

UninstPage uninstConfirm
UninstPage instfiles

Section "HTTP Server" SecCore
  SetOutPath $INSTDIR\www
  File ../www\*.html
  SetOutPath $INSTDIR\www\images
  File ../www\*.png

  SetOutPath $INSTDIR
  File ../cherokee\cherokee.exe
  File cherokee.ico
  File pthreadGC2.dll

  File install.bat
  ExecWait '"$INSTDIR\install.bat"'  
  delete $INSTDIR\install.bat

  WriteUninstaller "uninstall.exe"
SectionEnd

Section "Documentation"
  SetOutPath $INSTDIR\doc
  File ../doc\*.html  
  File ../doc\*.png

  SetOutPath $INSTDIR\doc\images
  File ../doc\images\*.*
SectionEnd

Section "Start Menu Shortcuts"
  CreateShortCut  "$SMPROGRAMS\Cherokee HTTP Server.lnk" "$INSTDIR\cherokee.bat" "" "$INSTDIR\cherokee.ico"
  CreateDirectory "$SMPROGRAMS\Cherokee HTTP Server utils"
  CreateShortCut  "$SMPROGRAMS\Cherokee HTTP Server utils\Documentation.lnk" "$INSTDIR\doc\index.html" "" "$INSTDIR\doc\index.html" 0
  CreateShortCut  "$SMPROGRAMS\Cherokee HTTP Server utils\Edit Configuration.lnk" "notepad.exe" "$INSTDIR\cherokee.conf"
  WriteINIStr     "$SMPROGRAMS\Cherokee HTTP Server utils\Cherokee Site.url" "InternetShortcut" "URL" "http://www.cherokee-project.com/"
SectionEnd

Section "Uninstall"
  Delete $INSTDIR\cherokee.exe
  Delete $INSTDIR\cherokee.bat
  Delete $INSTDIR\uninstall.exe

  Delete "$SMPROGRAMS\Cherokee\*.*"

  RMDir "$SMPROGRAMS\Cherokee"
  RMDir "$INSTDIR"
SectionEnd
