#!/usr/bin/python

import sys
import subprocess
import os
import shutil
import datetime
import platform
import socket
import re

# Determine default path to common storage
if "linux" in sys.platform:
    def_common = "/media/syn-common"
    line = subprocess.check_output(["g++", "--version"]).split("\n")[0]
    idx = line.rindex(" ")
    compiler = "g++-%s" % line[idx+1:]
elif "win32" in sys.platform:
    def_common = "S:\\"
    line = subprocess.check_output(["cl.exe"], stderr=subprocess.STDOUT).split("\n")[0]
    m = re.match(".*Version\s([0-9.]*).*", line)
    clver = m.group(1)
    compiler = {
        "19.00.23918" : "msvc2015",
        "19.00.23506" : "msvc2015",
        "19.00.23026" : "msvc2015",
        "19.00.22816" : "msvc2015-rc",
        "16.00.40219.01" : "msvc2010",
        }[clver]
elif "darwin" in sys.platform:
    def_common = "/Volumes/common"
    line = subprocess.check_output(["g++", "--version"]).split("\n")[0]
    idx = line.index(" ")
    compiler = line[0:idx]
elif "cygwin" in sys.platform:
    raise Exception("This POS must be run from a lame 'cmd' thing")

from argparse import ArgumentParser
parser = ArgumentParser()
parser.add_argument("-V", "--version", dest="version", help="Directory/version to package")
parser.add_argument("-C", "--common", dest="common", default=def_common, help="Path to mounted \\\\syn\\common")
parser.add_argument("-R", "--dry-run", dest="dry_run", default=False, action="store_true", help="Don't actually perform the packaging")
parser.add_argument("-S", "--source", dest="package_source", default=False, action="store_true", help="Package source, not a built distro")
options = parser.parse_args()

now = datetime.datetime.now()
now = now.replace(microsecond=0)
date_str = now.isoformat().replace(":", "_")

fname = None

version = options.version
while version.endswith("/") or version.endswith("\\"):
    version = version[:-1]

def path_on_common(fname):
    return os.path.join(options.common, "Qt", version.replace("-x64", "").replace("-vs2010", ""), fname)

def run(argv, cwd=None):
    print "Executing: \"%s\"" % " ".join(argv)
    if not options.dry_run:
        subprocess.check_call(argv, cwd=cwd)


if options.package_source:
    fname = "qt-src-%s-%s.tar" % (version, date_str)
    dest_path = path_on_common(fname)
    run(["git", "archive", "-o", dest_path, "master"])
    run(["bzip2", dest_path])
else:
    def get_unix_host():
        host = subprocess.check_output(["uname", "-n"]).rstrip()
        host = host.replace(".hex-rays.com", "")
        return host

    if "linux" in sys.platform:
        def get_after_colon(argv):
            line = subprocess.check_output(argv)
            parts = map(lambda l : l.strip(), line.rstrip().split(":"))
            return parts[1]
        dist_id = get_after_colon(["lsb_release", "-i"])
        release = get_after_colon(["lsb_release", "-r"])
        host = get_unix_host()
        fname = "qt-%s-%s-%s-%s-%s-%s.tar.bz2" % (version, date_str, dist_id, release, host, compiler)
    elif "darwin" in sys.platform:
        # dsymutil-ize all the things
        lib_reldir = os.path.join(version, "lib")
        dirs = filter(lambda d: d.endswith(".framework"), os.listdir(lib_reldir))
        cwd = os.getcwd()
        for d in dirs:
            fwk = d.replace(".framework", "")
            # it seems Qt 5.5.0 introduced a 'QtUiPlugin' framework that only contains header files.
            # so, don't bother running dsymutil on it
            if fwk == "QtUiPlugin":
                continue
            run(["dsymutil", fwk + "_debug"], cwd=os.path.join(cwd, lib_reldir, d, "Versions", "Current"))
        run(["dsymutil", "libqcocoa_debug.dylib"], cwd=os.path.join(version, "plugins", "platforms"))
        # package
        release = subprocess.check_output(["sw_vers", "-productVersion"]).rstrip()
        host = get_unix_host()
        fname = "qt-%s-%s-%s-%s-%s-%s.tar.bz2" % (version, date_str, "osx", release, host, compiler)
    elif "win32" in sys.platform:
        release = platform.platform()
        host = socket.gethostname()
        fname = "qt-%s-%s-%s-%s-%s.zip" % (version, date_str, release, host, compiler)

    dest_path = path_on_common(fname)
    if fname.endswith(".tar") or fname.endswith(".tar.bz2"):
        argv = ["tar", "cjf", dest_path, version]
    else:
        argv = ["zip", "-r", dest_path, version]

    run(argv)
