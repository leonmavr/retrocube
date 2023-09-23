## :black_large_square: retrocube :white_large_square:

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
---
Render 3D meshes in ASCII on the command line.
It runs on the standard C library.
```
                    +==
                ++======
             +============
          ==================
      ========================
   +===========================..
   ||+=====================.......
    ||+=================..........
    +||||===========...............
     +||||||==+.....................
      +||||||........................
       +||||||.......................
       +||||||~.......................
        +||||||....................~.%
           |||||...............~.
            ||||~..........~..
               ||.........
                ||....~

```

### 1. This implementation

In human language, the graphics are rendered more or less by the following algorithm:
```
rows <- terminal's height
columns <- terminal's width
// a face (surface) is a plane segment (x, y, z) restricted within 4 cube vertices
initialise a cube (6 faces)
for (r in rows):
    for (c in columns):
        z_rendered <- +inf
        have_intersection <- false
        pixel_to_draw <- (c, r)
        color_to_draw <- background
        for (surface in cube's faces):
            // from equation ax + by + cz + d = 0
            z <- surface.z(c, r)
            if (z < z_rendered) and ((c, r, z) in surface):
                z_rendered <- z
                color_to_draw <- surface.color
        draw(pixel_to_draw, color_to_draw)
```

### 2. Requirements

Currenctly there is no Windows support. You only need gcc and make:
1. **gcc**
2. **make**

### 3. Development and installation

#### 3.1 Development

##### 3.1.1 Compiling the project

The naming convention follows the one of [stb](https://github.com/nothings/stb).
Source files are found in `src` and headers in `include`.

When compiling the from project from a clean state, you need to specify where the mesh files
(those that specify how shapes are rendereed) shall be stored. You can do this by setting the
`PREFIX` variable to your directory of choice, e.g.:
```
make PREFIX=~/.config/retrocube
```
You can run the binary with (a list of command line arguments is provided in the next section):
```
./cube
```
You can delete the binary and object files with:
```
make clean
```
##### 3.1.2 Compiling the demos

Several demos that showcase various usages of the libraries are found in the `demos` directory.
These are compiled independently from their own file. To compile them you need to set the `PREFIX`
once again:
```
cd demos
make PREFIX=~/.config/retrocube
# then you will see some binaries and run the binary of your choice
```


#### 3.2 General installation

The `Makefile` includes an installation command. The binary will be installed at `/usr/bin/cube` as:
```
sudo make install
```
Similarly, you can uninstall it from `/usr/bin` as:
```
sudo make uninstall
```

#### 3.3 Installation as Nix package

On Nix (with flakes enabled) you don't need to install it and you can directly run it with:
```
nix run github:leonmavr/retrocube
```
Credits for the Nix packaging @pmarreck and @Quantenzitrone.

### 4. Usage

#### 4.1 Arguments

By default the program runs forever so you can stop it with `Ctr+C`. Below are the command line arguments it accepts.


| Short specifier | Long specifier            | Argument type | Default | Description                                                                                 |
|:--------------- |:--------------------------|:--------------|:--------|:--------------------------------------------------------------------------------------------|
| `-sx`           | `--speedx`                | float         | 0.7     |Rotational speed around the x axis (-1 to 1). If set, disables random rotations.             |
| `-sy`           | `--speedy`                | float         | 0.4     |Rotational speed around the y axis (-1 to 1). If set, disables random rotations.             |
| `-sz`           | `--speedz`                | float         | 0.6     |Rotational speed around the z axis (-1 to 1). If set, disables random rotations.             |
| `-f`            | `--fps`                   | int           | 40      |Throttle the fps at which the graphics can be rendered (lower it if high CPU usage or if flicker) |
| `-r`            | `--random`                | no argument   | On      |Rotate the shape randomly and sinusoidally.                                                  |
| `-cx`           | `--cx`                    | int           | 0       |x-coordinate of the shapes's center in pixels                                                |
| `-cy`           | `--cy`                    | int           | 0       |y-coordinate of the shapes's center in pixels                                                |
| `-cz`           | `--cz`                    | int           | 0       |z-coordinate of the shapes's center in pixels                                                |
| `-wi`           | `--width`                 | int           | 60      |Width of shape in pixels                                                                     |
| `-he`           | `--height`                | int           | 60      |Height of shape in pixels                                                                    |
| `-de`           | `--depth`                 | int           | 60      |Depth of shape in pixels                                                                     |
| `-ff`           | `--from-file`             | string        | `./mesh_files/cube.scl` |The filepath to the mesh file to render. See `mesh_files` directory.         |
| `-mi`           | `--maximum-iterations`    | int           | Inf/ty  |How many frames to run the program for                                                       |
| `-up`           | `--use-perspective`       | no argument   | Off     |Whether or not to use pinhole camera's perspective transform on rendered pixels              |
| `-be`           | `--bounce-every`          | int           | 0       |If non-zero (`-be N` or `--bounce-every N`), changes moving direction every N frames         |
| `-mx`           | `--movex`                 | int           | 2       |Move the object by this many pixels along x axis per frame if bounce (`-b`/`--bounce`) is enabled. |
| `-my`           | `--movey`                 | int           | 1       |Move the object by this many pixels along y axis per frame if bounce (`-b`/`--bounce`) is enabled. |
| `-mz`           | `--movez`                 | int           | 1       |Move the object by this many pixels along z axis per frame if bounce (`-b`/`--bounce`) is enabled. |

Below are two examples of running the demo binary `./cube`:

`./cube` | `./cube -b 100 -cz 300 -my -1 -mz 1`
:-------------------------:|:-------------------------:
![](https://github.com/leonmavr/retrocube/blob/master/assets/demo_still.gif?raw=true)  |  ![](https://raw.githubusercontent.com/leonmavr/retrocube/master/assets/demo_moving.gif)


#### 4.2 Tips

1. If the CPU usage is too high (it was low on my ancient laptop), you can reduce the fps e.g. to 15 by: `./cube -f 15` or `./cube --fps 15`.


### 5. Contributing

If you'd like to contribute, please follow the codiing guidelines (section 3.1) and make sure that it builds and runs.
I'll be happy to merge new changes.

Kudos to:
* [@pmarreck](https://github.com/pmarreck) - Nix packaging
* [@Quantenzitrone](https://github.com/Quantenzitrone) - Fixing and competing nix packaging, including the mesh text files properly, tidying up the make build
* [@IchikaZou](https://github.com/IchikaZou) - porting to Gentoo
* Anyone else who opened an issue for helping me make this project more robust.
