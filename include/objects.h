#ifndef OBJECTS_H
#define OBJECTS_H 

#include "vector.h"
#include <stdbool.h> // true/false
#include <math.h> // round
#include <stddef.h> // size_t

enum connection_t {
    CONNECTION_RECT=0,
    CONNECTION_TRIANGLE,
    NUM_CONNECTIONS
};

typedef char color_t;

typedef struct mesh {
    vec3i_t** vertices;
    vec3i_t** vertices_backup;
    vec3i_t* center;
    // number of vertices
    size_t n_vertices;
    // number of surfaces
    size_t n_faces;
    struct bounding_box {
        // top left
        int x0, y0, z0;
        // bottop right
        int x1, y1, z1;
    } bounding_box;
    /*
     * 2D array that defines the surfaces of the solid.
     * Its rows consist of the following data:a
     *   -- 4 indexes
     *   -- connection type (connection_t enum)
     *   -- a character that indicates the color of the current surface
     * To define a rectangular surface, use:
     * {3, 4, 6, 7, CONNECTION_RECT, 'o'},
     * For a triangular surface:
     * {3, 4, 6, -1, CONNECTION_TRIANGLE, 'o'},
     * Last index in triangular surface is always ignored. The generated surface
     * will be spanned by vertices[3], [4], [6], [7] or [3], [4], [6] respectively
     * and painted with the 'o' character.
     */
    int** connections;
} mesh_t;

typedef struct ray {
    // origin is the centre of perspective in pinhole camera model
    vec3i_t* orig;
    vec3i_t* end;
} ray_t;

// pinhole camera where objects are shot from
typedef struct camera {
    // origin
    int x0;
    int y0;
    // focal length
    float focal_length;
} camera_t;

/* 
 * plane in 3D assuming its equation is:
 * n_x*x + n_y*y + n_z*z + d = 0 (1)
 * or n.X + d = 0                (2)
 * , where n = (n_x, n_y, n_z) is the normal
 * , X = (x, y, y) a point on the plane
 * and d is the offset from the origin
 */
typedef struct plane {
    // d from eq. (2)
    int offset;
    // n from eq. (2)
    vec3i_t* normal;
} plane_t;

//-------------------------------------------------------------------------------------------------------------
// Renderable objects
//-------------------------------------------------------------------------------------------------------------
/**
* @brief Allocates and sets a 2D triangle
*
* @param p0 A triangle vertex
* @param p1 A triangle vertex
* @param p2 A triangle vertex
* @param color Triangle's fill color
*
* @returns A pointer to the newly constructed mesh
*/
mesh_t*     obj_triangle_new           (vec3i_t* p0, vec3i_t* p1, vec3i_t* p2, color_t color);
/**
* @brief
*
* @param fpath File path to read vertex and connection info from 
* @param cx x-coordinate of the center of the mesh to be created
* @param cy y-coordinate of the center of the mesh to be created
* @param cz z-coordinate of the center of the mesh to be created
* @param width Width of the mesh
* @param height Height of the mesh
* @param depth Depth of the mesh
*
* @returns A pointer to the mesh that has been constructed 
*/
mesh_t*     obj_mesh_from_file         (const char* fpath, int cx, int cy, int cz,
                                        unsigned width, unsigned height, unsigned depth);
void        obj_mesh_rotate            (mesh_t* mesh, float angle_x_rad, float angle_y_rad, float angle_z_rad);
void        obj_mesh_translate         (mesh_t* mesh, float dx, float dy, float dz);
void        obj_mesh_free              (mesh_t* mesh);

//-------------------------------------------------------------------------------------------------------------
// Ray
//-------------------------------------------------------------------------------------------------------------
// the pixel in the screen where the ray points to
/**
 * @brief Allocates a ray structure containing an origin and end vector 
 *
 * @return A pointer to the newly constructed ray
 */
ray_t*      obj_ray_new                 ();
/**
 * @brief Sets the destination (`end` member) of a ray
 *
 * @param[in/out] ray Pointer to the ray to modify
 * @param x0 x-coordinate of ray's origin 
 * @param y0 y-coordinate of ray's origin 
 * @param z0 z-coordinate of ray's origin 
 * @param x1 x-coordinate of ray's destination
 * @param y1 y-coordinate of ray's destination
 * @param z1 z-coordinate of ray's destination
 */
void        obj_ray_set                 (ray_t* ray, int x0, int y0, int z0, int x1, int y1, int z1); 
/**
 * @brief Sets the destination (`end` member) of a ray
 *
 * @param[in/out] ray Pointer to the ray to modify
 * @param x       x-coordinate of ray's new destination
 * @param y       y-coordinate of ray's new sestination
 * @param z       z-coordinate of ray's new destination
 */
void        obj_ray_send                (ray_t* ray, int x, int y, int z); 
void        obj_ray_free                (ray_t* ray);

//-------------------------------------------------------------------------------------------------------------
// Camera 
//-------------------------------------------------------------------------------------------------------------
camera_t*   obj_camera_new              ();
void        obj_camera_set              (camera_t* camera, int cam_x0, int cam_y0, float focal_length);

//-------------------------------------------------------------------------------------------------------------
// Plane
//-------------------------------------------------------------------------------------------------------------
/**
 * @brief Allocates a plane and sets its normal vector and offset given three points
 *
 * @param p0 First point on the plane
 * @param p1 Second point on the plane
 * @param p2 Third point on the plane
 *
 * @return A pointer to the newly constructed plane
 */
plane_t*    obj_plane_new                  ();
/* recompute plane's normal and offset given 3 points */
void        obj_plane_set                  (plane_t* plane, vec3i_t* p0, vec3i_t* p1, vec3i_t* p2);
bool        obj_is_point_in_triangle       (vec3i_t* m, vec3i_t* a, vec3i_t* b, vec3i_t* c);
bool        obj_is_point_in_rect           (vec3i_t* m, vec3i_t* a, vec3i_t* b, vec3i_t* c, vec3i_t* d);
vec3i_t     render__ray_plane_intersection (plane_t* plane, ray_t* ray);
bool        obj_ray_hits_rectangle         (ray_t* ray, vec3i_t** points);
bool        obj_ray_hits_triangle          (ray_t* ray, vec3i_t** points);
void        obj_plane_free                 (plane_t* plane);

/*
 * Note for programmers:
 *
 * The table below uses a technique called X macro to expand its column.
 * The table itself is defined as a function is X. X does not need to be
 * defined yet, but it's defined later according to the expansion we want.
 * It maps the following information:
 * <connection letter in .scl file> -> <connection index> -> <intersection function>
 *
 * Connection types are defined as an enum in objects.h.
 * Intersection functions define whether the ray intersects a rectangle or
 * triangle. As a final note, all functions must take the same parameter
 * types since we later expand them as a function pointer table.
 */
#define CONN_TABLE                                          \
        X('R', CONNECTION_RECT,     obj_ray_hits_rectangle) \
        X('T', CONNECTION_TRIANGLE, obj_ray_hits_triangle)


#endif /* OBJECTS_H */
