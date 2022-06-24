import os
import shutil
import subprocess
import sys

source_folder = os.getcwd()
QT_DIR = ""
QT_COMPILER = ""
full_qt_dir = ""
build_dir = ""


def main():
    prepare()
    # at this stage the work dir is build_dir
    if sys.platform == "darwin":
        build_osx()
    elif sys.platform == "win32":
        build_windows()
    else:
        sys.exit("Unknown platform: " + sys.platform)


def prepare():
    global full_qt_dir, QT_DIR, QT_COMPILER, build_dir
    print("Prepare")
    if "QT_DIR" not in os.environ:
        sys.exit('No QT_DIR environment variable provided')
    QT_DIR = os.environ["QT_DIR"]
    if "QT_COMPILER" not in os.environ:
        sys.exit('No QT_COMPILER environment variable provided. Please check folders with QT compiled by different compilers in your QT_DIR: ' + QT_DIR)
    QT_COMPILER = os.environ["QT_COMPILER"]

    full_qt_dir = QT_DIR + QT_COMPILER
    if not os.path.exists(full_qt_dir):
        sys.exit('Cannot find QT directory with compiler [' + full_qt_dir + ']')
    print("Using QT: " + full_qt_dir)
    print("Project folder: " + source_folder)
    build_dir = get_build_dir()
    print("Build dir: " + build_dir)
    os.chdir(build_dir)


def build_osx():
    print("Build platform: OSX")
    run([f'{full_qt_dir}/bin/qmake', f'{source_folder}/CpLauncher.pro', '-spec', 'macx-clang', 'CONFIG+=qtquickcompiler'])
    run(['/usr/bin/make', 'qmake_all'])
    run(['make', '-j12'])
    if not os.path.exists("./CpLauncher.app"):
        sys.exit('Build has been finished, but ./CpLauncher.app file is not present in build folder')
    run(['/usr/libexec/PlistBuddy', '-c', 'Add :LSUIElement bool true', './CpLauncher.app/Contents/Info.plist'])
    run([f'{full_qt_dir}/bin/macdeployqt', './CpLauncher.app', '-dmg'])
    remove_file('CpLauncher.app/Contents/Frameworks/QtOpenGL.framework')
    remove_file('CpLauncher.app/Contents/Frameworks/QtMultimediaWidgets.framework')
    remove_file('CpLauncher.app/Contents/Frameworks/QtQml.framework')
    remove_file('CpLauncher.app/Contents/Frameworks/QtQmlModels.framework')
    remove_file('CpLauncher.app/Contents/Frameworks/QtQuick.framework')
    remove_file('CpLauncher.app/Contents/Frameworks/QtVirtualKeyboard.framework')
    remove_file('CpLauncher.app/Contents/PlugIns/virtualkeyboard')
    remove_file('CpLauncher.app/Contents/PlugIns/sqldrivers/libqsqlodbc.dylib')
    remove_file('CpLauncher.app/Contents/PlugIns/sqldrivers/libqsqlpsql.dylib')
    remove_file('CpLauncher.app/Contents/PlugIns/mediaservice')
    remove_file('CpLauncher.app/Contents/PlugIns/imageformats/libqtga.dylib')
    remove_file('CpLauncher.app/Contents/PlugIns/imageformats/libqtiff.dylib')
    remove_file('CpLauncher.app/Contents/PlugIns/imageformats/libqwebp.dylib')
    version = get_version()
    shutil.make_archive("CpLauncher_" + version + "_OSX.app.zip", 'zip', 'CpLauncher.app')


def build_windows():
    print("Build platform: Windows")
    run([f'{full_qt_dir}/bin/qmake', f'{source_folder}/CpLauncher.pro', '-spec', 'macx-clang', 'CONFIG+=qtquickcompiler'])
    run(['/usr/bin/make', 'qmake_all'])
    run(['make', '-j12'])
    if not os.path.exists("./CpLauncher"):
        sys.exit('Build has been finished, but ./CpLauncher file is not present in build folder')
    os.mkdirs("CpLauncher/platforms")
    os.mkdirs("CpLauncher/styles")
    os.mkdirs("CpLauncher/plugins/audio")
    os.mkdirs("CpLauncher/plugins/bearer")
    os.mkdirs("CpLauncher/plugins/imageformats")
    os.mkdirs("CpLauncher/plugins/sqldrivers")
    shutil.copy2('release/CpLauncher.exe', './CpLauncher')
    shutil.copy2(f'{full_qt_dir}/bin/Qt5Core.dll', './CpLauncher')
    shutil.copy2(f'{full_qt_dir}/bin/Qt5Gui.dll', './CpLauncher')
    shutil.copy2(f'{full_qt_dir}/bin/Qt5Multimedia.dll', './CpLauncher')
    shutil.copy2(f'{full_qt_dir}/bin/Qt5Network.dll', './CpLauncher')
    shutil.copy2(f'{full_qt_dir}/bin/Qt5Sql.dll', './CpLauncher')
    shutil.copy2(f'{full_qt_dir}/bin/Qt5Svg.dll', './CpLauncher')
    shutil.copy2(f'{full_qt_dir}/bin/Qt5Widgets.dll', './CpLauncher')
    shutil.copy2(f'{full_qt_dir}/bin/Qt5Xml.dll', './CpLauncher')
    shutil.copy2(f'{full_qt_dir}/bin/platforms/qwindows.dll', './CpLauncher/platforms')
    shutil.copy2(f'{full_qt_dir}/bin/plugins/styles/qwindowsvistastyle.dll', './CpLauncher/styles')
    shutil.copy2(f'{full_qt_dir}/bin/plugins/audio/qtaudio_wasapi.dll', './CpLauncher/plugins/audio')
    shutil.copy2(f'{full_qt_dir}/bin/plugins/audio/qtaudio_windows.dll', './CpLauncher/plugins/audio')
    shutil.copy2(f'{full_qt_dir}/bin/plugins/bearer/qgenericbearer.dll', './CpLauncher/plugins/bearer')
    shutil.copy2(f'{full_qt_dir}/bin/plugins/imageformats/qgif.dll', './CpLauncher/plugins/imageformats')
    shutil.copy2(f'{full_qt_dir}/bin/plugins/imageformats/qicns.dll', './CpLauncher/plugins/imageformats')
    shutil.copy2(f'{full_qt_dir}/bin/plugins/imageformats/qico.dll', './CpLauncher/plugins/imageformats')
    shutil.copy2(f'{full_qt_dir}/bin/plugins/imageformats/qjpeg.dll', './CpLauncher/plugins/imageformats')
    shutil.copy2(f'{full_qt_dir}/bin/plugins/imageformats/qwbmp.dll', './CpLauncher/plugins/imageformats')
    shutil.copy2(f'{full_qt_dir}/bin/plugins/sqldrivers/qsqlite.dll', './CpLauncher/plugins/sqldrivers')
    version = get_version()
    shutil.make_archive("CpLauncher_" + version + "_Windows.zip", 'zip', 'CpLauncher.app')


def get_build_dir():
    dir = source_folder + "/../CpLauncher_build"
    if os.path.exists(dir):
        print("Build folder already exists. Removing it: " + dir)
        remove_file(dir)
    dir = os.path.abspath(dir)
    os.mkdir(dir)
    return dir


def get_version():
    version_file = source_folder + "/res/version.txt"
    if not os.path.exists(version_file):
        sys.exit("Cannot find version.txt file: " + version_file)
    with open(version_file) as file:
        lines = file.readlines()
        for line in lines:
            if line.startswith('@'):
                return line[1:].strip()


def run(command):
    process = subprocess.Popen(command, stdout=subprocess.PIPE)
    result = process.communicate()[0]
    code = process.returncode
    if code != 0:
        print(result)
        sys.exit("Command " + str(command) + " has been finished with error code " + str(code))


def remove_file(path):
    if not os.path.exists(path):
        print("Trying to remove file that does not exists:" + path)
        return
    if os.path.isdir(path):
        shutil.rmtree(path)
    else:
        os.remove(path)


main()
