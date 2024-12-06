# IDA PRO 7.0 patches for QT 5.6.0.

[Original QT README](README).

A handful of our users have already requested information regarding the Qt 5.6.0 build, that is shipped with IDA 7.0.

## Configure options
Here are the options that were used to build the libraries on:

Windows:
```bash
...\5.6.0\configure.bat "-nomake" "tests" "-qtnamespace" "QT"
"-confirm-license" "-accessibility" "-opensource" "-force-debug-info" "-platform" "win32-msvc2015" "-opengl" "desktop" "-prefix" "C:/Qt/5.6.0-x64"
```

Note that you will have to build with Visual Studio 2015, to obtain compatible libs
Linux:
```bash
.../5.6.0/configure "-nomake" "tests" "-qtnamespace" "QT" "-confirm-license" "-accessibility" "-opensource" "-force-debug-info" "-platform" "linux-g++-64" "-developer-build" "-fontconfig" "-qt-freetype" "-qt-libpng" "-glib" "-qt-xcb" "-dbus" "-qt-sql-sqlite" "-gtkstyle" "-prefix" "/usr/local/Qt/5.6.0-x64"
```

Mac OSX:
```bash
.../5.6.0/configure "-nomake" "tests" "-qtnamespace" "QT" "-confirm-license" "-accessibility" "-opensource" "-force-debug-info" "-platform" "macx-g++" "-debug-and-release" "-fontconfig" "-qt-freetype" "-qt-libpng" "-qt-sql-sqlite" "-prefix" "/Users/Shared/Qt/5.6.0-x64"
```
## patch

In addition to the specific configure options, the Qt build that ships with IDA includes the following [patch](https://www.hex-rays.com/wp-content/uploads/2017/08/qt-5_6_0_full-IDA70.zip?_gl=1*7742gm*_ga*MTI2NDQ0Nzg5Ny4xNzMzMTI0Nzg0*_ga_Y2G1VBHRDB*MTczMzUyMDU5OC43LjAuMTczMzUyMDU5OS4wLjAuMA..) retrieved [here](patches/full.patch). You should therefore apply it to your own Qt 5.6.0 sources before compiling, in order to obtain similar binaries.
Note that this patch should work without any modification, against the 5.6.0 release as found [there](https://download.qt.io/archive/qt). You may have to fiddle with it, if your Qt 5.6.0 sources come from somewhere else. 
