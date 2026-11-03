!include "MUI2.nsh"
!ifndef DPX_DIST
  !define DPX_DIST "..\\dist-windows"
!endif
Name "DPxADJU AI Mastering & Vocal Mixer"
OutFile "DPxADJU AI Installer.exe"
InstallDir "$LOCALAPPDATA\\Programs\\DPxADJU AI"
RequestExecutionLevel admin
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_LANGUAGE "English"
Section "DPxADJU AI" SEC_MAIN
  SetOutPath "$INSTDIR"
  File "${DPX_DIST}\\DPxADJU AI Studio.exe"
  CreateDirectory "$COMMONFILES64\\VST3"
  SetOutPath "$COMMONFILES64\\VST3\\DPxADJU AI.vst3"
  File /r "${DPX_DIST}\\DPxADJU AI.vst3\\*.*"
  CreateDirectory "$SMPROGRAMS\\DPxADJU AI"
  WriteUninstaller "$INSTDIR\\Uninstall.exe"
  CreateShortcut "$SMPROGRAMS\\DPxADJU AI\\DPxADJU AI Studio.lnk" "$INSTDIR\\DPxADJU AI Studio.exe"
  CreateShortcut "$SMPROGRAMS\\DPxADJU AI\\Uninstall.lnk" "$INSTDIR\\Uninstall.exe"
SectionEnd
Section "Uninstall"
  Delete "$SMPROGRAMS\\DPxADJU AI\\DPxADJU AI Studio.lnk"
  RMDir "$SMPROGRAMS\\DPxADJU AI"
  Delete "$SMPROGRAMS\\DPxADJU AI\\Uninstall.lnk"
  Delete "$INSTDIR\\DPxADJU AI Studio.exe"
  Delete "$INSTDIR\\Uninstall.exe"
  RMDir "$INSTDIR"
  RMDir /r "$COMMONFILES64\\VST3\\DPxADJU AI.vst3"
SectionEnd
