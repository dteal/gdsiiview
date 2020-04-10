# gdsiiview

Displays GDSII files as 3D geometry.

Layers of potentially multiple GDSII files, according to specifications in a configuration file, are triangulated, extruded to a certain thickness, and positioned in 3D space. This is intended to help visualize multi-die MEMS structures with complex (but rectilinear) 3D geometry in real time while using a traditional 2D editor in parallel.

## Usage

Run

`./gdsiiview config.gdsiiview`

where `config.gdsiiview` is a plain text configuration file. See the example folder for an example. The configuration file or any referenced GDSII files will be reloaded whenever they are changed in the file system.

## Installation

On Debian Linux:

### Dependencies

- gcc with C++11 support
- libglfw3-dev (https://www.glfw.org/)
- libglew-dev (http://glew.sourceforge.net/)
- libglm-dev (https://glm.g-truc.net/0.9.9/index.html)
- libtriangle-dev (http://www.cs.cmu.edu/~quake/triangle.html)
- libboost-dev (https://www.boost.org/)

### Build and Run

```
cd gdsiiview
mkdir bin
make
./bin/gdsiiview ./example/example.gdsiiview
```

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
