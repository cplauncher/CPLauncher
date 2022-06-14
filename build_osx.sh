/usr/libexec/PlistBuddy -c 'Add :LSUIElement bool true' ./CpLauncher.app/Contents/Info.plist
${QT_DIR}/bin/macdeployqt CpLauncher.app -dmg
rm -rf CpLauncher.app/Contents/Frameworks/QtOpenGL.framework
rm -rf CpLauncher.app/Contents/Frameworks/QtMultimediaWidgets.framework
rm -rf CpLauncher.app/Contents/Frameworks/QtQml.framework
rm -rf CpLauncher.app/Contents/Frameworks/QtQmlModels.framework
rm -rf CpLauncher.app/Contents/Frameworks/QtQuick.framework
rm -rf CpLauncher.app/Contents/Frameworks/QtVirtualKeyboard.framework
rm -rf CpLauncher.app/Contents/PlugIns/virtualkeyboard
rm -rf CpLauncher.app/Contents/PlugIns/sqldrivers/libqsqlodbc.dylib
rm -rf CpLauncher.app/Contents/PlugIns/sqldrivers/libqsqlpsql.dylib
rm -rf CpLauncher.app/Contents/PlugIns/mediaservice
rm -rf CpLauncher.app/Contents/PlugIns/imageformats/libqtga.dylib
rm -rf CpLauncher.app/Contents/PlugIns/imageformats/libqtiff.dylib
rm -rf CpLauncher.app/Contents/PlugIns/imageformats/libqwebp.dylib
zip -r CpLauncher.app.zip CpLauncher.app