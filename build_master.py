import os
import shutil
import subprocess
import sys


def main():
    print('main')


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
