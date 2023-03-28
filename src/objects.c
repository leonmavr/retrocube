#include "vector.h"
#include "objects.h"
#include "utils.h"
#include "screen.h" // g_plane_test
#include <math.h> // round, abs
#include <stdlib.h>
#include <stdbool.h> // bool
#include <string.h> // strncpy
#include <stddef.h> // size_t 


static inline void obj__mesh_update_bbox(mesh_t* mesh, int cx, int cy, int width, int height, int depth) {
    const int m = UT_MAX(UT_MAX(abs(width), abs(height)), abs(depth));
    mesh->bounding_box.x0 = mesh->center->x - m/UT_SQRT_TWO;
    mesh->bounding_box.y0 = mesh->center->y - m/UT_SQRT_TWO;
    mesh->bounding_box.z0 = mesh->center->z - m/UT_SQRT_TWO;
    mesh->bounding_box.x1 = mesh->center->x + m/UT_SQRT_TWO;
    mesh->bounding_box.y1 = mesh->center->y + m/UT_SQRT_TWO;
    mesh->bounding_box.z1 = mesh->center->z + m/UT_SQRT_TWO;
}

//----------------------------------------------------------------------------------------------------------
// Renderable shapes 
//----------------------------------------------------------------------------------------------------------
mesh_t* obj_mesh_new(int cx, int cy, int cz, int width, int height, int depth, int type) {

    mesh_t* new = malloc(sizeof(mesh_t));
    //// common attributes
    new->type = type;
    switch (new->type) {
        case TYPE_CUBE:
            new->n_vertices = 8;
            new->n_faces = 6;
            break;
        case TYPE_RHOMBUS:
            new->n_vertices = 6;
            new->n_faces = 8;
            break;
        default:
            // TODO: throw exception
            new->n_vertices = 0;
            new->n_faces = 0;
            break;
    }
    new->center = vec_vec3i_new(cx, cy, cz);
    // colors for each of the maximum possible 8 faces - change the string below to modify them
    strncpy(new->colors, "~.=@%|O+?Tn", 8);
    new->vertices = (vec3i_t**) malloc(sizeof(vec3i_t*) * new->n_vertices);
    new->vertices_backup = (vec3i_t**) malloc(sizeof(vec3i_t*) * new->n_vertices);
    obj__mesh_update_bbox(new, new->center->x, new->center->y, width, height, depth);
    // allocate 2D array that indicates how vertices are connected at each surface
    new->connections = malloc(new->n_faces * sizeof(int*));
    for (int i = 0; i < new->n_faces; ++i)
        new->connections[i] = malloc(6 * sizeof(int));
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
        const int x = round(width/(2*UT_SQRT_TWO));
        const int y = round(height/(2*UT_SQRT_TWO));
        const int z = round(depth/(2*UT_SQRT_TWO));
        new->vertices[0] = vec_vec3i_new(-x, -y, -z);
        new->vertices[1] = vec_vec3i_new( x, -y, -z);
        new->vertices[2] = vec_vec3i_new( x,  y, -z);
        new->vertices[3] = vec_vec3i_new(-x,  y, -z);
        new->vertices[4] = vec_vec3i_new(-x, -y,  z);
        new->vertices[5] = vec_vec3i_new( x, -y,  z);
        new->vertices[6] = vec_vec3i_new( x,  y,  z);
        new->vertices[7] = vec_vec3i_new(-x,  y,  z);
        // define surfaces
        int connections[6][6] = 
        {
            {0, 1, 2, 3, CONNECTION_RECT, '~'},
            {0, 4, 7, 3, CONNECTION_RECT, '.'},
            {4, 5, 6, 7, CONNECTION_RECT, '='},
            {5, 1, 2, 6, CONNECTION_RECT, '@'},
            {7, 6, 2, 3, CONNECTION_RECT, '?'},
            {0, 4, 5, 1, CONNECTION_RECT, '+'}
        };
        for (int r = 0; r < UT_MATRIX_ROWS(connections); ++r) {
            for (int c = 0; c < UT_MATRIX_COLS(connections); ++c) {
                new->connections[r][c] = connections[r][c];
            }
        }
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
        new->vertices[0] = vec_vec3i_new(0       , 0                               , depth/2);
        new->vertices[1] = vec_vec3i_new(width/2 , 0                               , 0);
        new->vertices[2] = vec_vec3i_new(0       , 0                               , -depth/2);
        new->vertices[3] = vec_vec3i_new(-width/2, 0                               , 0);
        new->vertices[4] = vec_vec3i_new(0       , -round(UT_PHI/(1+UT_PHI)*height), 0);
        new->vertices[5] = vec_vec3i_new(0       , round(1.0/(1+UT_PHI)*height)    , 0);

        // define surfaces
        int connections[8][6] = {
            {3, 4, 0, 0, CONNECTION_TRIANGLE, '~'},
            {0, 4, 1, 0, CONNECTION_TRIANGLE, '.'},
            {4, 2, 1, 0, CONNECTION_TRIANGLE, '='},
            {4, 2, 3, 0, CONNECTION_TRIANGLE, '@'},
            {3, 0, 5, 0, CONNECTION_TRIANGLE, '%'},
            {0, 1, 5, 0, CONNECTION_TRIANGLE, '|'},
            {1, 5, 2, 0, CONNECTION_TRIANGLE, 'O'},
            {3, 2, 5, 0, CONNECTION_TRIANGLE, '+'}
        };
        for (int r = 0; r < UT_MATRIX_ROWS(connections); ++r) {
            for (int c = 0; c < UT_MATRIX_COLS(connections); ++c) {
                new->connections[r][c] = connections[r][c];
            }
        }
    } else if (type == TYPE_TRIANGLE) {
        new->vertices[0] = vec_vec3i_new(-width/2, 0     , 0);
        new->vertices[1] = vec_vec3i_new( width/2, 0     , 0);
        new->vertices[2] = vec_vec3i_new(       0, height, 0);
    }

    // finish creating the vertices - shift the to the mesh's origin, back them up
    for (int i = 0; i < new->n_vertices; ++i) {
        *new->vertices[i] = vec_vec3i_add(new->vertices[i], new->center);
        new->vertices_backup[i] = vec_vec3i_new(0, 0, 0);
        vec_vec3i_copy(new->vertices_backup[i], new->vertices[i]);
    }
    return new;
}

mesh_t* obj_triangle_new(vec3i_t* p0, vec3i_t* p1, vec3i_t* p2, color_t color) {
    mesh_t* new = malloc(sizeof(mesh_t));
    new->center = malloc(sizeof(vec3i_t));
    new->center->x = (p0->x + p1->x + p2->x)/3;
    new->center->y = (p0->y + p1->y + p2->y)/3;
    new->center->z = (p0->z + p1->z + p2->z)/3;
    new->n_vertices = 3;
    new->n_faces = 1;
    new->vertices = (vec3i_t**) malloc(sizeof(vec3i_t*) * new->n_vertices);
    new->vertices_backup = (vec3i_t**) malloc(sizeof(vec3i_t*) * new->n_vertices);
    unsigned width = UT_MAX( UT_MAX(abs(p0->x - p1->x), abs(p0->x - p2->x)),
                             UT_MAX(abs(p0->x - p1->x), abs(p1->x - p2->x)));
    unsigned height = UT_MAX(UT_MAX(abs(p0->y - p1->y), abs(p0->y - p2->y)),
                             UT_MAX(abs(p0->y - p1->y), abs(p1->y - p2->y)));
    new->vertices[0] = vec_vec3i_new(p0->x, p0->y, p0->z);
    new->vertices[1] = vec_vec3i_new(p1->x, p1->y, p1->z);
    new->vertices[2] = vec_vec3i_new(p2->x, p2->y, p2->z);
    obj__mesh_update_bbox(new, new->center->x, new->center->y, width, height, 0);

    // allocate 2D array that indicates how vertices are connected at each surface
    new->connections = malloc(new->n_faces * sizeof(int*));
    for (int i = 0; i < new->n_faces; ++i)
        new->connections[i] = malloc(6 * sizeof(int));
    // define surfaces
    new->connections[0][0] = 0;
    new->connections[0][1] = 1;
    new->connections[0][2] = 2;
    new->connections[0][3] = 0;
    new->connections[0][4] = CONNECTION_TRIANGLE;
    new->connections[0][5] = color;

    // finish creating the vertices - shift the to the mesh's origin, back them up
    for (int i = 0; i < new->n_vertices; ++i) {
        *new->vertices[i] = vec_vec3i_add(new->vertices[i], new->center);
        new->vertices_backup[i] = vec_vec3i_new(0, 0, 0);
        vec_vec3i_copy(new->vertices_backup[i], new->vertices[i]);
    }
    return new;
}

void obj_mesh_rotate (mesh_t* mesh, float angle_x_rad, float angle_y_rad, float angle_z_rad) {
    for (size_t i = 0; i < mesh->n_vertices; ++i) {
        // first, reset each vertex so no floating point error is accumulated
        vec_vec3i_copy(mesh->vertices[i], mesh->vertices_backup[i]);

        // point to rotate about
        int x0 = mesh->center->x, y0 = mesh->center->y, z0 = mesh->center->z;
        // rotate around x axis, then y, then z
        // We rotate as follows (* denotes matrix product, C the mesh's origin):
        // v = v - C, v = Rz*Ry*Rx*v, v = v + C
        vec_vec3i_rotate(mesh->vertices[i], angle_x_rad, angle_y_rad, angle_z_rad, x0, y0, z0);
    }
}

void obj_mesh_translate(mesh_t* mesh, float dx, float dy, float dz) {
    // to update bounding box after the translation
    const unsigned width = abs(mesh->bounding_box.x0 - mesh->bounding_box.x1);
    const unsigned height = abs(mesh->bounding_box.y0 - mesh->bounding_box.y1);
    const unsigned depth = abs(mesh->bounding_box.z0 - mesh->bounding_box.z1);
    vec3i_t translation = {round(dx), round(dy), round(dz)};
    *mesh->center = vec_vec3i_add(mesh->center, &translation);
    for (size_t i = 0; i < mesh->n_vertices; ++i)
        *mesh->vertices[i] = vec_vec3i_add(mesh->vertices[i], &translation);

    obj__mesh_update_bbox(mesh, mesh->center->x, mesh->center->y, width, height, depth);
}

void obj_mesh_free(mesh_t* mesh) {
    // free the data of the vertices first
    for (size_t i = 0; i < mesh->n_vertices; ++i) {
        free(mesh->vertices[i]);
        free(mesh->vertices_backup[i]);
    }
    free(mesh->vertices);
    free(mesh->vertices_backup);
    for (int i = 0; i < mesh->n_faces; ++i)
        free(mesh->connections[i]);
    free(mesh->connections);
    free(mesh->center);
    free(mesh);
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
