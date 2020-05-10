# gdsiiview

Displays GDSII files as 3D geometry.

Layers of potentially multiple GDSII files, according to specifications in a configuration file, are triangulated, extruded to a certain thickness, and positioned in 3D space. This is intended to help visualize multi-die MEMS structures with complex (but rectilinear) 3D geometry in real time while using a traditional 2D editor in parallel.

## Usage

After compilation, a single executable (`gdsiiview.exe`, etc.) should exist. Run this program to see a window with an empty 3D view and a menubar. Select `File->Open File...` from the menu bar to open a `*.gdsiiview` file (a normal text file with specific formatting), which references GDSII files with colors and transformations.

See the sample files `example/example.gdsiiview` and `example/example.gds`.

## Development

This project is designed to compile on multiple platforms. Linux and Windows are tested; MacOS support is theoretical.

### Dependencies

This project depends on the OpenGL Mathematics (GLM) library (https://glm.g-truc.net/0.9.9/index.html) and Jonathan Shewchuk's Triangle library (https://www.cs.cmu.edu/~quake/triangle.html). For ease of compilation on different platforms, the source for both is included directly in the project source repository, although GLM is a git submodule. Thus, to obtain a local copy of the source, run:

```
git clone https://github.com/dteal/gdsiiview
cd gdsiiview
git submodule init
git submodule update
```

The GUI is written in Qt; obtain the latest version (Qt 5.x) from, e.g., (https://www.qt.io/download-qt-installer).

### Compilation

This project can be opened and compiled in Qt Creator, the Qt C++ IDE.

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
