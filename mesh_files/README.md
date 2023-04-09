# .scl file manual

### Introduction

`.scl` is a file format I have developed for my project. It's inspired by Blender's `.obj` file format, however it only specifies vertices and connections. It's space-separated.

### Anatomy of an .scl file

`.scl` files are parsed by the `obj_mesh_from_file` function declared in `objects.h`. The parser only takes into account two kinds of expressions in an `.scl` file:
* v X X X  
* f Y Y Y Y T C
where:  
* X is a float from -1.0 to 1.0  
* Y an integer  
* T a character  
* C a character

`v` indicates that a vertex is to be defined. The next 3 numbers that follow (`X X X`) specify the location of the vertex. Each `X` can range from -1.0 to 1.0 and the first `X` specifies the location as a proportion of the width (-1.0 corresponds to -width, -0.25 -width/4, etc.). Likewise for the second and third `X`.  

When a vertex is defined, it's assigned a unique incremental index under the hood starting from zero. This is how it will be refererenced by the connections.  

`f` indicates that a connection is to be defined. In the end, it defines a surface. The first four integers (`Y`) reference the vertices is shall connect. For example, `0 2 4 1` connect the first, third, fifth and second vertices together. The next character indicates the connection type. Currecntly rectangular (`R`) and triangular (`T`) connections are supported. If `T` follows the vertices, only the first three are taken into account. In the previous example, `0 2 4 1 R` would define a rectangle with all four vertices and `0 2 4 1 T` would define a triangle with the `0, 2, 4`-th vertices. The number of vertex indexes must always be 4 no matter whether you want to draw a rectangle or triangle! The last entry can be any ASCII character. It specifies the filliing color of the surface to be rendered.  
Anything that doesn't start with `v` or `f` is considered a comment. Anything after `X X X` in `v`-prefixed lines is also a comment. Likewise for anything after `Y Y Y Y T C` in `f`-prefixed lines.
