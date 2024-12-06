
import sys
import subprocess
import os

from argparse import ArgumentParser
parser = ArgumentParser()
parser.add_argument("--build-x64", dest="build_x64", default=False, action="store_true", help="Build x64 libs or x86 libs?")
parser.add_argument("--vs2010", dest="vs_2010", default=False, action="store_true", help="Build with VisualStudio 2010. Windows only, duh.")
parser.add_argument("--dry-run", dest="dry_run", default=False, action="store_true", help="Don't issue configure command; just print it.")
clopts = parser.parse_args()

qtver = "5.6.0"

configure_script, build_command, install_dir = {
    "win32" : (
        "configure.bat",
        ["jom", "-j"],
        "C:/Qt/"
    ),
    "linux2" : (
        "configure",
        ["make", "-j"],
        "/usr/local/Qt/"
    ),
    "darwin" : (
        "configure",
        ["make", "-j"],
        "/Users/Shared/Qt/"
    ),
    }[sys.platform]
options = {
    "common" : [
        "-nomake", "tests",
        "-qtnamespace", "QT",
        "-confirm-license",
        "-accessibility",
        "-opensource",
        "-force-debug-info",
        ],
    "win32" : [
        "-platform", ("win32-msvc2010" if clopts.vs_2010 else "win32-msvc2015"),
        "-opengl", "desktop",
        ],
    "linux2" : [
        "-platform", ("linux-g++-64" if clopts.build_x64 else "linux-g++-32"),
        "-developer-build",
        "-fontconfig",
        "-qt-freetype",
        "-qt-libpng",
        "-glib",
        "-qt-xcb",
        "-dbus",
        "-qt-sql-sqlite",
        "-gtkstyle",
        ],
    "darwin" : [
        "-platform", ("macx-g++" if clopts.build_x64 else "macx-g++-32"),
        "-debug-and-release",
        "-fontconfig",
        "-qt-freetype",
        "-qt-libpng",
        "-qt-sql-sqlite",
        ]
    }

if sys.platform == "win32" and clopts.build_x64:
    print "WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING"
    print "Hopefully you are running this in a Visual Studio x64 prompt!"
    print "WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING"

parent_dir = os.path.join(os.getcwd(), "..")
argv = [os.path.join(parent_dir, configure_script)]
argv.extend(options["common"])
argv.extend(options[sys.platform])
argv.extend(["-prefix", (install_dir + "%s%s%s") % (qtver, "-vs2010" if clopts.vs_2010 else "", "-x64" if clopts.build_x64 else "")])

def run(argv):
    if clopts.dry_run:
        uf_argv = [argv[0]] # user-friendly argv
        for arg in argv[1:]:
            uf_argv.append("\"%s\"" % arg)
        print " ".join(uf_argv)
    else:
        subprocess.check_call(argv)

def phase(which):
    print "*" * 75
    print "{:^75}".format(which)
    print "*" * 75

phase("Configuring")
run(argv)

phase("Building")
argv = build_command[:]
run(argv)

phase("Installing")
argv = []
if sys.platform != "win32":
    argv.append("sudo")
argv.extend(build_command[:])
argv.append("install")
run(argv)

if sys.platform == "linux2":
    need_to_patch_runpaths()
