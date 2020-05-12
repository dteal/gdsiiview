# gdsiiview

Displays GDSII files as 3D geometry.

Layers of potentially multiple GDSII files, according to specifications in a configuration file, are triangulated, extruded to a certain thickness, and positioned in 3D space. This is intended to help visualize multi-die MEMS structures with complex (but rectilinear) 3D geometry in real time while using a traditional 2D editor in parallel.

NOTE: This program is still under development, and currently only supports GDSII polygons (notably, paths and cells are NOT displayed (properly)). Certain other features (e.g., units other than 1000 database units = 1um) are also not yet implemented.

![screenshot](example/example_screenshot.png?raw=true "Example Screenshot")

## Usage

Until I figure out how to properly package this in a single executable or something, it's easiest to compile and run this program through Qt Creator, a cross-platform C++ IDE for the Qt graphical user interface library. Follow the "Compilation" instructions in the next section to set this up.

When you compile and run this program, you will see a window with an empty 3D view and a menubar. Select `File->Open...` from the menu bar to open a `*.gdsiiview` file (a normal text file with specific formatting), which in turn opens GDSII files with colors and transformations. See "`example.gdsiiview` in the `example` folder for an example. Each GDSII file described in the `*.gdsiiview` file can have transformations (rotation, translation), applied in the order they appear, display only selected layers with different thicknesses and colors, and comments (lines beginning with `#` are ignored). Images can also be referenced; these are extruded into a rectangular prism. This is good for visualizing the position of, e.g., a bare CMOS die assembled on top of a larger MEMS chip.

See the sample file `example/example.gdsiiview` for more detail.

Three mutually perpendicular circles in the middle of the window are a "3D cursor". Its circles and lines pointing out from their centers show the location and position of the X (red), Y (green), and Z (blue) axes. Once the a `*.gdsiiview` file is opened, the 3D model it represents is displayed in the window with the 3D cursor at the origin. Rotate the model about this 3D cursor by dragging with the left mouse button, and zoom in and out with the scroll wheel. Dragging with the right mouse button moves the model relative to the 3D cursor; this allows one to progressively zoom into and view small features anywhere on the model.

The window menubar contains several helpful commands. "View->Fit" zooms and repositions the model to fill the window. "View->Center Model Origin" moves the model origin back to the 3D cursor, and "View->Show/Hide 3D Cursor" does exactly what it says. The other view commands, e.g., "View->Top" and "View->Isometric", rotate the view to a specific orientation.

"File->Export Image..." exports the current window to an image file. This is useful for, e.g., making figures for later use. The image file resolution is the current size of the window. The background color can be defined in the `*.gdsiiview` file.

Finally, "File->Open..." opens a `*.gdsiiview` file, and both the `*.gdsiiview` file and referenced files (i.e., GDSII and image files) are watched. If any of the above are changed (e.g., edited in a 2D layout editor), the files are reloaded and the 3D view updated.

## Compilation

This project is designed to compile on multiple platforms. It has been tested on Linux and Windows; it probably works on MacOS, but the compilation process may or may not need some troubleshooting.

To compile and run this project, follow these steps:

### 1. Install Qt and Qt Creator

Qt is a very good cross-platform C++ graphical user interface (GUI) library. Download an installer for the latest version (Qt 5.x) from, e.g., (https://www.qt.io/download-qt-installer). If asked, select the open source option; Qt is dual licensed under an open source GPL license and a commercial license. Install both the Qt libraries and the Qt Creator IDE with C++ support; default settings are probably fine.

### 2. Dowload Source

his project depends on the OpenGL Mathematics (GLM) library (https://glm.g-truc.net/0.9.9/index.html) and Jonathan Shewchuk's Triangle library (https://www.cs.cmu.edu/~quake/triangle.html). For ease of compilation on different platforms, the source for both is included directly in the project source repository. However, the GLM source is included as a git submodule, so a single `git clone` is not enough to download everything. Instead, to download a local copy of the source code, run:

```
git clone https://github.com/dteal/gdsiiview
cd gdsiiview
git submodule init
git submodule update
```

### 3. Compile and Run in Qt Creator

Run Qt Creator and open the `gdsiiview/gdsiiview.pro` file. Qt should ask you to configure the project, i.e., choose a folder for it to put compiled files (e.g., `*.exe`) into for several different use cases (debug, release, etc). It is easiest to select only "release", then choose any folder (the default, or "gdsiiview/bin", work fine).

You should now be able to see a list of source code files on the left of the Qt Creator window, which can be double-clicked to open in the right.

To compile and run the program, click the green triangle (without the bug; that one's for debugging) in the bottom of the column on the left of the Qt Creator window. If all goes well, the program should compile and run, showing the main program window, and be ready to use.

If compilation fails, or the gdsiiview window doesn't open, click the "Compile Output" or "Issues" buttons on the bottom bar of the Qt Creator window for more information.

Finally, it is easiest to run this program in this same way every time (i.e., open Qt Creator and press the green triangle). It may be possible to run the program directly (instead of going through Qt Creator) by running the compiled executable in the compilation folder chosen when first opening the project. This works on Linux and possibly MacOS, but on Windows, the files "Qt5Core.dll", "Qt5GUI.dll", and "Qt5Widgets.dll" from the Qt installation directory (e.g., `C:\Qt\5.14.2\mingw73_64\bin`) and the folder "plugins" from the same (e.g., `C:\Qt\5.14.2\mingw73_64\plugins`) should be copied to the same folder as the executable first.

Finally, there are still many bugs and yet-to-be-implemented features in the program; let me (Daniel Teal) know if you run into problems so I can try to help fix them.

## License

gdsiiview - Displays GDSII files as 3D geometry.
Copyright (C) 2020 Daniel Teal

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
