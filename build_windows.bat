mkdir "CpLauncher"
mkdir "CpLauncher\platforms"
mkdir "CpLauncher\styles"
mkdir "CpLauncher\plugins"
mkdir "CpLauncher\plugins\audio"
mkdir "CpLauncher\plugins\bearer"
mkdir "CpLauncher\plugins\imageformats"
mkdir "CpLauncher\plugins\sqldrivers"

SET QT_DIR=d:\Tools\QT\5.15.2\msvc2019
cp release\CpLauncher.exe CpLauncher\
cp %QT_DIR%\bin\Qt5Core.dll CpLauncher\
cp %QT_DIR%\bin\Qt5Gui.dll CpLauncher\
cp %QT_DIR%\bin\Qt5Multimedia.dll CpLauncher\
cp %QT_DIR%\bin\Qt5Network.dll CpLauncher\
cp %QT_DIR%\bin\Qt5Sql.dll CpLauncher\
cp %QT_DIR%\bin\Qt5Svg.dll CpLauncher\
cp %QT_DIR%\bin\Qt5Widgets.dll CpLauncher\
cp %QT_DIR%\bin\Qt5Xml.dll CpLauncher\
cp %QT_DIR%\plugins\platforms\qwindows.dll CpLauncher\platforms\
cp %QT_DIR%\plugins\styles\qwindowsvistastyle.dll CpLauncher\styles\
cp %QT_DIR%\plugins\audio\qtaudio_wasapi.dll CpLauncher\plugins\audio\
cp %QT_DIR%\plugins\audio\qtaudio_windows.dll CpLauncher\plugins\audio\
cp %QT_DIR%\plugins\bearer\qgenericbearer.dll CpLauncher\plugins\bearer\
cp %QT_DIR%\plugins\imageformats\qgif.dll CpLauncher\plugins\imageformats\
cp %QT_DIR%\plugins\imageformats\qicns.dll CpLauncher\plugins\imageformats\
cp %QT_DIR%\plugins\imageformats\qico.dll CpLauncher\plugins\imageformats\
cp %QT_DIR%\plugins\imageformats\qjpeg.dll CpLauncher\plugins\imageformats\
cp %QT_DIR%\plugins\imageformats\qwbmp.dll CpLauncher\plugins\imageformats\
cp %QT_DIR%\plugins\sqldrivers\qsqlite.dll CpLauncher\plugins\sqldrivers\
