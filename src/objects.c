#include "vector.h"
#include "objects.h"
#include "utils.h"
#include "draw.h" // g_plane_test
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h> // strncpy
#include <stddef.h> // size_t 


static inline void obj__shape_update_bbox(shape_t* shape, int cx, int cy, int width, int height) {
    const int m = UT_MAX(width, height);
    shape->bounding_box.x0 = shape->center->x - m/UT_SQRT_TWO;
    shape->bounding_box.y0 = shape->center->y - m/UT_SQRT_TWO;
    shape->bounding_box.x1 = shape->center->x + m/UT_SQRT_TWO;
    shape->bounding_box.y1 = shape->center->y + m/UT_SQRT_TWO;
}

//----------------------------------------------------------------------------------------------------------
// Renderable shapes 
//----------------------------------------------------------------------------------------------------------
shape_t* obj_shape_new(int cx, int cy, int cz, int width, int height, int type) {

    shape_t* new = malloc(sizeof(shape_t));
    //// common attributes
    new->type = type;
    switch (new->type) {
        case TYPE_CUBE:
            new->n_vertices = 8;
            break;
        case TYPE_RHOMBUS:
            new->n_vertices = 6;
            break;
        case TYPE_TRIANGLE:
            new->n_vertices = 3;
            break;
        default:
            new->n_vertices = 0;
            break;
    }
    new->center = vec_vec3i_new(cx, cy, cz);
    // colors for each of the maximum possible 8 faces - change the string below to modify them
    strncpy(new->colors, "~.=@%|O+?Tn", 8);
    new->vertices = (vec3i_t**) malloc(sizeof(vec3i_t*) * new->n_vertices);
    new->vertices_backup = (vec3i_t**) malloc(sizeof(vec3i_t*) * new->n_vertices);
    obj__shape_update_bbox(new, new->center->x, new->center->y, width, height);
    //// attributes that depend on number of vertices
    if (type == TYPE_CUBE) {
        /*
        *          p3                  p2 
        *           +-------------------+
        *           | \                 | \
        *           |    \              |    \            ^y
        *           |      \  p7        |       \         |
        *           |         +-------------------+ p6    |
        *           |         |         .         |       |
        *           |         |*(cx,xy,cz)        |       o-------> x
        *           |         |         .         |        \
        *           |         |         .         |         \
        *           |         |         .         |          v z
        *        p0 +---------|.........+ p1      |
        *            \        |           .       |
        *              \      |             .     |
        *                 \   |                .  |
        *                    \+-------------------+
        *                     p4                   p5
        */
        int diag = round(UT_MIN(width/2, height/2)/UT_SQRT_TWO); 
        new->vertices[0] = vec_vec3i_new(-diag, -diag, -diag);
        new->vertices[1] = vec_vec3i_new( diag, -diag, -diag);
        new->vertices[2] = vec_vec3i_new( diag,  diag, -diag);
        new->vertices[3] = vec_vec3i_new(-diag,  diag, -diag);
        new->vertices[4] = vec_vec3i_new(-diag, -diag,  diag);
        new->vertices[5] = vec_vec3i_new( diag, -diag,  diag);
        new->vertices[6] = vec_vec3i_new( diag,  diag,  diag);
        new->vertices[7] = vec_vec3i_new(-diag,  diag,  diag);
        // back them up so no floating point error is accumulated affter the rotations
    } else if (type == TYPE_RHOMBUS) {
        /*
        *               p5              * center
        *               X               X vertex
        *              / \
        *             /   \             y 
        *            /     \            ^
        *           /       \           |
        *          /    p2   \          |
        *         /     X     \         |
        *        /  ..     ..  \        o--------> x 
        *       /..          .. \        \
        *   p3 X.       *       .X p1     \
        *       \__           __/          \
        *        \  \__   __/  /            v
        *         \     X     /              z
        *          \    p0   / 
        *           \       / 
        *            \     / 
        *             \   / 
        *              \ / 
        *               X 
        *               p4
        */
        new->vertices[0] = vec_vec3i_new(0       , 0                               , width/2);
        new->vertices[1] = vec_vec3i_new(width/2 , 0                               , 0);
        new->vertices[2] = vec_vec3i_new(0       , 0                               , -width/2);
        new->vertices[3] = vec_vec3i_new(-width/2, 0                               , 0);
        new->vertices[4] = vec_vec3i_new(0       , -round(UT_PHI/(1+UT_PHI)*height), 0);
        new->vertices[5] = vec_vec3i_new(0       , round(1.0/(1+UT_PHI)*height)    , 0);
    } else if (type == TYPE_TRIANGLE) {
        new->vertices[0] = vec_vec3i_new(-width/2, 0     , 0);
        new->vertices[1] = vec_vec3i_new( width/2, 0     , 0);
        new->vertices[2] = vec_vec3i_new(       0, height, 0);
    }
    // finish creating the vertices
    for (int i = 0; i < new->n_vertices; ++i) {
        // shift them to the shape's center 
        *new->vertices[i] = vec_vec3i_add(new->vertices[i], new->center);
        // back them up so no floating point error is accumulated after any rotations
        new->vertices_backup[i] = vec_vec3i_new(0, 0, 0);
        vec_vec3i_copy(new->vertices_backup[i], new->vertices[i]);
    }
    return new;
}

void obj_shape_rotate (shape_t* shape, float angle_x_rad, float angle_y_rad, float angle_z_rad) {
    for (size_t i = 0; i < shape->n_vertices; ++i) {
        // first, reset each vertex so no floating point error is accumulated
        vec_vec3i_copy(shape->vertices[i], shape->vertices_backup[i]);

        // point to rotate about
        int x0 = shape->center->x, y0 = shape->center->y, z0 = shape->center->z;
        // rotate around x axis, then y, then z
        // We rotate as follows (* denotes matrix product, C the shape's origin):
        // v = v - C, v = Rz*Ry*Rx*v, v = v + C
        vec_vec3i_rotate(shape->vertices[i], angle_x_rad, angle_y_rad, angle_z_rad, x0, y0, z0);
    }
}

void obj_shape_translate(shape_t* shape, float dx, float dy, float dz) {
    // to update bounding box after the translation
    const unsigned width = abs(shape->bounding_box.x0 - shape->bounding_box.x0);
    const unsigned height = abs(shape->bounding_box.y0 - shape->bounding_box.y0);
    vec3i_t translation = {round(dx), round(dy), round(dz)};
    *shape->center = vec_vec3i_add(shape->center, &translation);
    for (size_t i = 0; i < shape->n_vertices; ++i)
        *shape->vertices[i] = vec_vec3i_add(shape->vertices[i], &translation);

    obj__shape_update_bbox(shape, shape->center->x, shape->center->y, width, height);
}

void obj_shape_free(shape_t* shape) {
    // free the data of the vertices first
    for (size_t i = 0; i < shape->n_vertices; ++i) {
        free(shape->vertices[i]);
        free(shape->vertices_backup[i]);
    }
    free(shape->vertices);
    free(shape->vertices_backup);
    free(shape->center);
    free(shape);
}

//----------------------------------------------------------------------------------------------------------
// Ray
//----------------------------------------------------------------------------------------------------------
ray_t* obj_ray_new(int x0, int y0, int z0, int x1, int y1, int z1) {
    ray_t* new = malloc(sizeof(ray_t));
    new->orig = vec_vec3i_new(x0, y0, z0);
    new->end = vec_vec3i_new(x1, y1, z1);
    return new;
}

void obj_ray_set(ray_t* ray, int x0, int y0, int z0, int x1, int y1, int z1) {
    ray->orig = vec_vec3i_new(x0, y0, z0);
    ray->end = vec_vec3i_new(x1, y1, z1);

}

void obj_ray_send(ray_t* ray, int x, int y, int z) {
    ray->end->x = x;
    ray->end->y = y;
    ray->end->z = z;
}

void obj_ray_free(ray_t* ray) {
    free(ray->orig);
    free(ray->end); 
    free(ray);
}

//-------------------------------------------------------------------------------------------------------------
// Camera 
//-------------------------------------------------------------------------------------------------------------
camera_t* obj_camera_new(int cam_x0, int cam_y0, float focal_length) {
    camera_t* new = malloc(sizeof(camera_t));
    new->x0 = cam_x0;
    new->y0 = cam_y0;
    new->focal_length = focal_length;
    return new;
}

void obj_camera_set(camera_t* camera, int cam_x0, int cam_y0, float focal_length) {
    camera->x0 = cam_x0;
    camera->y0 = cam_y0;
    camera->focal_length = focal_length;
}


//----------------------------------------------------------------------------------------------------------
// Plane
//----------------------------------------------------------------------------------------------------------

plane_t* obj_plane_new (vec3i_t* p0, vec3i_t* p1, vec3i_t* p2) {
    /*
    * Determine the plane through 3 3D points p0, p1, p2 by determining:
    *     1. the normal vector
    *     2. the offset
    *
    *                                     normal
    *                                       ^
    *                                      /
    *                    +----------------/-----------+
    *                   /     *p0        /           /
    *                  /       <_       /           /
    *                 /          \__   /           /
    *                /              \ /           /
    *               /               *p1          /
    *              /             _/             / 
    *             /           _/               /
    *            /           <                /
    *           /         p2*                /
    *          /                            /
    *         +----------------------------+
    * If p0, p1, p2 are co-planar, then the normal through p1 is
    * perpendicular to both * p1p2 = p2 - p1 and p1p0 = p0 - p1.
    * Thererefore it's determined as the cross product of the two:
    * normal = p1p2 x p1p0 = (p2 - p1) x (p0 - p1)
    *
    *                                    normal
    *                                    ^
    *                                   /     
    *                  +---------------/-----------+
    *                 /               /           /
    *                /               /           /
    *               /              *p1          /
    *              /              /            / 
    *             /           ___/            /
    *            /           /               /
    *           /       * <_/               /
    *          /        x                  /
    *         +---------------------------+
    * If x = (x,y,z) is any point on the plane, then the normal
    * through p1 is perpendicular to p1x = x - p1 therefore their
    * dot product is zero:
    * n.(x - p1) = 0 => 
    * n.x - n.p1 = 0
    * -n.p1 is the offset from the origin 
    */
    plane_t* new = malloc(sizeof(plane_t));
    new->normal = malloc(sizeof(vec3_t));
    vec3i_t p1p2 = vec_vec3i_sub(p2, p1);
    vec3i_t p1p0 = vec_vec3i_sub(p0, p1);
    *new->normal = vec_vec3i_crossprod(&p1p2, &p1p0);
    new->offset = -vec_vec3i_dotprod(new->normal, p1);
    return new;
}



void obj_plane_set(plane_t* plane, vec3i_t* p0, vec3i_t* p1, vec3i_t* p2) {
    // reset plane's normal and offset like obj_plane_new function computes them
    vec3i_t p1p2 = vec_vec3i_sub(p2, p1);
    vec3i_t p1p0 = vec_vec3i_sub(p0, p1);
    *plane->normal = vec_vec3i_crossprod(&p1p2, &p1p0);
    plane->offset = -vec_vec3i_dotprod(plane->normal, p1);
}

void obj_plane_free (plane_t* plane) {
    free(plane->normal);
    free(plane); 
}
