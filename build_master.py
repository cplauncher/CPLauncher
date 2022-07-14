import argparse
import os
import requests
import shutil
import subprocess
import sys


def boolean_string(s):
    if s not in {'False', 'True', 'false', 'true'}:
        raise ValueError('Not a valid boolean string')
    if s == 'True' or s == 'true':
        return True
    if s == 'False' or s == 'false':
        return False
    sys.exit(f"Cannot parse {s} as boolean. Only true or false is supported")


parser = argparse.ArgumentParser(description='Process some integers.')
parser.add_argument("--output_dir", dest="output_dir", help="Output directory that will store zip releases", type=str, required=False)
parser.add_argument("--check_and_copy_to_output", type=boolean_string,
                    help="If true - check if the output directory already has the release, and skip build if it is already has it. The result zip will be copied to output folder", default=False, required=False)
parser.add_argument("--build_dir", dest="build_dir", help="Build directory", type=str, required=True)
parser.add_argument("--qt_dir", dest="qt_dir", help="QT installation directory", type=str, required=True)
parser.add_argument("--qt_compiler", dest="qt_compiler", help="Compiler used to compile QT(look in qt_dir directory)", type=str, required=True)
args = parser.parse_args()


def main():
    version = get_version()
    print('Current version in git master: ' + version)
    if args.check_and_copy_to_output:
        print(f"Output directory: {args.output_dir}")
        if not (os.path.exists(args.output_dir)):
            sys.exit(f"Output directory {args.output_dir} does not exist")

        release_name = get_output_file_name(version)
        if os.path.exists(args.output_dir + "/" + release_name):
            print("Release with the same version is already present in output directory. Exit")
            sys.exit(0)
        print(f'Release {release_name} is not found. Building')

    if not os.path.exists(args.qt_dir):
        sys.exit(f"QT installation directory is not found {args.qt_dir}")
    full_qt_dir = args.qt_dir + args.qt_compiler
    if not os.path.exists(full_qt_dir):
        sys.exit(f"QT compiler directory is not found {full_qt_dir}")

    print(f'Build directory: {args.build_dir}')
    if os.path.exists(args.build_dir):
        remove_file(args.build_dir)

    os.mkdir(args.build_dir)
    os.chdir(args.build_dir)
    run(["git", "clone", "https://github.com/cplauncher/CPLauncher.git"])
    run(["git", "clone", "https://github.com/itay-grudev/SingleApplication.git", "CPLauncher/libs/projects/SingleApplication"])
    run(["git", "clone", "https://github.com/Skycoder42/QHotkey.git", "CPLauncher/libs/projects/QHotkey"])
    run(["git", "clone", "https://github.com/edubart/minilua.git", "CPLauncher/libs/projects/lua/minilua"])
    os.chdir("CPLauncher")
    os.environ["QT_DIR"] = args.qt_dir
    os.environ["QT_COMPILER"] = args.qt_compiler
    run([sys.executable, "build.py"])
    release_name = get_output_file_name(version)
    built_release_path = os.path.abspath("../CpLauncher_build/" + release_name)
    print(f"Release {built_release_path} has been built.")
    if not os.path.exists(built_release_path):
        sys.exit("Error. Release has been built, but script cannot find it under the path: " + built_release_path)
    if args.check_and_copy_to_output:
        shutil.copy2(built_release_path, args.output_dir)
    print("Finish")


def get_version():
    version_url = "https://raw.githubusercontent.com/cplauncher/CPLauncher/main/res/version.txt"
    response = requests.get(version_url)
    if not response.status_code == 200:
        sys.exit("Cannot read version.txt file from url: " + version_url)
    data = response.text
    lines = data.splitlines()
    for line in lines:
        if line.startswith('@'):
            return line[1:].strip()
    sys.exit("Cannot find version in version.txt under the url: " + version_url)


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


def get_output_file_name(version):
    if sys.platform == "darwin":
        return "CpLauncher_" + version + "_OSX.app.zip"
    elif sys.platform == "win32":
        return "CpLauncher_" + version + "_Windows.zip"


main()
