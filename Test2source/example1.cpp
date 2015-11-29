// =================== EXAMPLE CODE WITH ENVIRONMENT MAPPING
//
// Display a color cube, a color tube, and view them with a moving camera
//

#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <iostream>
using namespace std;

#include <GLUT/glut.h>

#include "Angel.h"

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

GLuint loadBMP_custom(unsigned char** data, int* w, int* h, const char * imagepath);

//---- some variables for our camera
//----------------------------------------------------------------------------

// array of rotation angles (in degrees) for each axis
enum { Xaxis = 0, Yaxis = 1, Zaxis = 2, NumAxes = 3 };
int      Axis = Zaxis;
GLfloat  Theta1[NumAxes] = { 0.0, 90.0, 90.0 };

float r = 8.0;

mat4 view_matrix, default_view_matrix;
mat4 proj_matrix;

bool spherical_cam_on = true;

//---- cube model
//----------------------------------------------------------------------------
const int NumVerticesCube = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)
point4 points_cube[NumVerticesCube];
color4 colors[NumVerticesCube];
vec3   normals[NumVerticesCube];
vec2   tex_coords[NumVerticesCube];

// Vertices of a unit cube centered at origin, sides aligned with axes
point4 vertices[8] = {
    point4( -0.5, -0.5,  0.5, 1.0 ),
    point4( -0.5,  0.5,  0.5, 1.0 ),
    point4(  0.5,  0.5,  0.5, 1.0 ),
    point4(  0.5, -0.5,  0.5, 1.0 ),
    point4( -0.5, -0.5, -0.5, 1.0 ),
    point4( -0.5,  0.5, -0.5, 1.0 ),
    point4(  0.5,  0.5, -0.5, 1.0 ),
    point4(  0.5, -0.5, -0.5, 1.0 )
};

// RGBA colors
color4 vertex_colors[8] = {
    color4( 0.1, 0.1, 0.1, 1.0 ),  // black
    color4( 1.0, 0.0, 0.0, 1.0 ),  // red
    color4( 1.0, 1.0, 0.0, 1.0 ),  // yellow
    color4( 0.0, 1.0, 0.0, 1.0 ),  // green
    color4( 0.0, 0.0, 1.0, 1.0 ),  // blue
    color4( 1.0, 0.0, 1.0, 1.0 ),  // magenta
    color4( 0.9, 0.9, 0.9, 1.0 ),  // white
    color4( 0.0, 1.0, 1.0, 1.0 )   // cyan
};

// quad generates two triangles for each face and assigns colors to the vertices
int Index;
void
quad_cube( int a, int b, int c, int d )
{
    // Initialize temporary vectors along the quad's edge to compute its face normal
    vec4 u = vertices[b] - vertices[a];
    vec4 v = vertices[c] - vertices[b];
    
    vec3 normal = normalize( cross(u, v) );
    
    normals[Index] = normal; colors[Index] = vertex_colors[a]; points_cube[Index] = vertices[a]; tex_coords[Index] = vec2( 1.0, 1.0 ); Index++;
    normals[Index] = normal; colors[Index] = vertex_colors[a]; points_cube[Index] = vertices[b]; tex_coords[Index] = vec2( 1.0, 0.0 ); Index++;
    normals[Index] = normal; colors[Index] = vertex_colors[a]; points_cube[Index] = vertices[c]; tex_coords[Index] = vec2( 0.0, 0.0 ); Index++;
    normals[Index] = normal; colors[Index] = vertex_colors[a]; points_cube[Index] = vertices[a]; tex_coords[Index] = vec2( 1.0, 1.0 ); Index++;
    normals[Index] = normal; colors[Index] = vertex_colors[a]; points_cube[Index] = vertices[c]; tex_coords[Index] = vec2( 0.0, 0.0 ); Index++;
    normals[Index] = normal; colors[Index] = vertex_colors[a]; points_cube[Index] = vertices[d]; tex_coords[Index] = vec2( 0.0, 1.0 ); Index++;
}

// generate 12 triangles: 36 vertices and 36 colors
void
colorcube()
{
    Index = 0;
    quad_cube( 1, 0, 3, 2 );
    quad_cube( 2, 3, 7, 6 );
    quad_cube( 3, 0, 4, 7 );
    quad_cube( 6, 5, 1, 2 );
    quad_cube( 4, 5, 6, 7 );
    quad_cube( 5, 4, 0, 1 );
}

//---- triangle model
//----------------------------------------------------------------------------
const int NumVerticesTri = 24; //(3 faces)(2 triangles/ face)+(2 caps)(1 triangle/ cap)*(3 vertices/triangle)
point4 points_tri[NumVerticesTri];
vec3   tnormals[NumVerticesTri];
vec2   ttex_coords[NumVerticesTri];

// Vertices of a unit cube centered at origin, sides aligned with axes
point4 tri_vertices[6] = {
    point4( -0.5, -0.5,  0.5, 1.0 ),
    point4( -0.5,  0.5,  0.5, 1.0 ),
    point4(  0.5,  0.5,  0.5, 1.0 ),
    point4(  0.5, -0.5,  0.5, 1.0 ),
    point4(  0.0, -0.5, -0.5, 1.0 ),
    point4(  0.0,  0.5, -0.5, 1.0 )
};
// quad generates two triangles for each face
void
quad_tri( int a, int b, int c, int d )
{
    // Initialize temporary vectors along the quad's edge to compute its face normal
    vec4 u = tri_vertices[b] - tri_vertices[a];
    vec4 v = tri_vertices[c] - tri_vertices[b];
    
    vec3 normal = normalize( cross(u, v) );
    
    tnormals[Index] = normal; points_tri[Index] = tri_vertices[a]; ttex_coords[Index] = vec2( 1.0, 1.0 ); Index++;
    tnormals[Index] = normal; points_tri[Index] = tri_vertices[b]; ttex_coords[Index] = vec2( 1.0, 0.0 ); Index++;
    tnormals[Index] = normal; points_tri[Index] = tri_vertices[c]; ttex_coords[Index] = vec2( 0.0, 0.0 ); Index++;
    tnormals[Index] = normal; points_tri[Index] = tri_vertices[a]; ttex_coords[Index] = vec2( 1.0, 1.0 ); Index++;
    tnormals[Index] = normal; points_tri[Index] = tri_vertices[c]; ttex_coords[Index] = vec2( 0.0, 0.0 ); Index++;
    tnormals[Index] = normal; points_tri[Index] = tri_vertices[d]; ttex_coords[Index] = vec2( 0.0, 1.0 ); Index++;
}

void
sing_tri( int a, int b, int c )
{
    // Initialize temporary vectors along the quad's edge to compute its face normal
    vec4 u = tri_vertices[b] - tri_vertices[a];
    vec4 v = tri_vertices[c] - tri_vertices[b];
    
    vec3 normal = normalize( cross(u, v) );
    
    tnormals[Index] = normal; points_tri[Index] = tri_vertices[a]; ttex_coords[Index] = vec2( 1.0, 1.0 ); Index++;
    tnormals[Index] = normal; points_tri[Index] = tri_vertices[b]; ttex_coords[Index] = vec2( 1.0, 0.0 ); Index++;
    tnormals[Index] = normal; points_tri[Index] = tri_vertices[c]; ttex_coords[Index] = vec2( 0.0, 0.0 ); Index++;
}

// generate 8 triangles: 24 vertices
void
colortri()
{
    Index = 0;
    quad_tri( 1, 0, 3, 2 );
    quad_tri( 2, 3, 4, 5 );
    quad_tri( 5, 4, 0, 1 );
    sing_tri( 5, 1, 2);
    sing_tri(3, 0, 4);
    /*
    vec4 u = tri_vertices[2] - tri_vertices[1];
    vec4 v = tri_vertices[5] - tri_vertices[2];
    vec3 normal = normalize( cross(u,v) );
    tnormals[Index] = normal; points_tri[Index] = tri_vertices[1]; Index++;
    tnormals[Index] = normal; points_tri[Index] = tri_vertices[2]; Index++;
    tnormals[Index] = normal; points_tri[Index] = tri_vertices[5]; Index++;
    
    u = tri_vertices[3] - tri_vertices[0];
    v = tri_vertices[4] - tri_vertices[3];
    normal = normalize( cross(u,v) );
    tnormals[Index] = normal; points_tri[Index] = tri_vertices[0]; Index++;
    tnormals[Index] = normal; points_tri[Index] = tri_vertices[3]; Index++;
    tnormals[Index] = normal; points_tri[Index] = tri_vertices[4]; Index++;
     */
}

//---- cylinder model
//----------------------------------------------------------------------------
const int segments = 64;
const int NumVerticesCylinder = segments*6 + segments*3*2;
point4 points_cylinder[NumVerticesCylinder];
vec3   cnormals[NumVerticesCylinder];
point4 vertices_cylinder[4];

// quad generates two triangles for each face and assigns colors to the vertices
void
quad_cylinder( int a, int b, int c, int d )
{
    points_cylinder[Index] = vertices_cylinder[a];
    cnormals[Index] = vec3(vertices_cylinder[a].x, 0.0, vertices_cylinder[a].z); Index++;
    points_cylinder[Index] = vertices_cylinder[b];
    cnormals[Index] = vec3(vertices_cylinder[b].x, 0.0, vertices_cylinder[b].z); Index++;
    points_cylinder[Index] = vertices_cylinder[c];
    cnormals[Index] = vec3(vertices_cylinder[c].x, 0.0, vertices_cylinder[c].z); Index++;
    points_cylinder[Index] = vertices_cylinder[a];
    cnormals[Index] = vec3(vertices_cylinder[a].x, 0.0, vertices_cylinder[a].z); Index++;
    points_cylinder[Index] = vertices_cylinder[c];
    cnormals[Index] = vec3(vertices_cylinder[c].x, 0.0, vertices_cylinder[c].z); Index++;
    points_cylinder[Index] = vertices_cylinder[d];
    cnormals[Index] = vec3(vertices_cylinder[d].x, 0.0, vertices_cylinder[d].z); Index++;
}

float const bottom = -0.5;
float const top    = 0.5;

void
colortube(void)
{
    float r = 0.5;
    Index = 0;
    
    for ( int n = 0; n < segments; n++ )
    {
        GLfloat const t0 = 2 * M_PI * (float)n / (float)segments;
        GLfloat const t1 = 2 * M_PI * (float)(n+1) / (float)segments;
        
        points_cylinder[Index].x = 0.0;
        points_cylinder[Index].y = top;
        points_cylinder[Index].z = 0.0;
        points_cylinder[Index].w = 1.0;
        cnormals[Index] = vec3(0.0, 1.0, 0.0);
        Index++;
        points_cylinder[Index].x = cos(t0) * r;
        points_cylinder[Index].y = top;
        points_cylinder[Index].z = sin(t0) * r;
        points_cylinder[Index].w = 1.0;
        cnormals[Index] = vec3(0.0, 1.0, 0.0);
        Index++;
        points_cylinder[Index].x = cos(t1) * r;
        points_cylinder[Index].y = top;
        points_cylinder[Index].z = sin(t1) * r;
        points_cylinder[Index].w = 1.0;
        cnormals[Index] = vec3(0.0, 1.0, 0.0);
        Index++;
    }
    
    int num;
    for ( int n = 0; n < segments; n++ )
    {
        num = 0;
        float x = 0.0, y = 0.0, r = 0.5;
        
        GLfloat const t0 = 2 * M_PI * (float)n / (float)segments;
        GLfloat const t1 = 2 * M_PI * (float)(n+1) / (float)segments;
        
        //quad vertex 0
        vertices_cylinder[num].x = cos(t0) * r;
        vertices_cylinder[num].y = bottom;
        vertices_cylinder[num].z = sin(t0) * r;
        vertices_cylinder[num++].w = 1.0;
        //quad vertex 1
        vertices_cylinder[num].x = cos(t1) * r;
        vertices_cylinder[num].y = bottom;
        vertices_cylinder[num].z = sin(t1) * r;
        vertices_cylinder[num++].w = 1.0;
        //quad vertex 2
        vertices_cylinder[num].x = cos(t1) * r;
        vertices_cylinder[num].y = top;
        vertices_cylinder[num].z = sin(t1) * r;
        vertices_cylinder[num++].w = 1.0;
        //quad vertex 3
        vertices_cylinder[num].x = cos(t0) * r;
        vertices_cylinder[num].y = top;
        vertices_cylinder[num].z = sin(t0) * r;
        vertices_cylinder[num++].w = 1.0;
        
        quad_cylinder( 0, 1, 2, 3 );
    }
    
    for ( int n = 0; n < segments; n++ )
    {
        GLfloat const t0 = 2 * M_PI * (float)n / (float)segments;
        GLfloat const t1 = 2 * M_PI * (float)(n+1) / (float)segments;
        
        points_cylinder[Index].x = 0.0;
        points_cylinder[Index].y = bottom;
        points_cylinder[Index].z = 0.0;
        points_cylinder[Index].w = 1.0;
        cnormals[Index] = vec3(0.0, -1.0, 0.0);
        Index++;
        points_cylinder[Index].x = cos(t1) * r;
        points_cylinder[Index].y = bottom;
        points_cylinder[Index].z = sin(t1) * r;
        points_cylinder[Index].w = 1.0;
        cnormals[Index] = vec3(0.0, -1.0, 0.0);
        Index++;
        points_cylinder[Index].x = cos(t0) * r;
        points_cylinder[Index].y = bottom;
        points_cylinder[Index].z = sin(t0) * r;
        points_cylinder[Index].w = 1.0;
        cnormals[Index] = vec3(0.0, -1.0, 0.0);
        Index++;
    }
    
}

//---- sphere model
//----------------------------------------------------------------------------
const int ssegments = 512;
const int NumVerticesSphere = ssegments*6*(ssegments-2) + ssegments*3*2;
point4 points_sphere[NumVerticesSphere];
vec3   bnormals[NumVerticesSphere];
point4 vertices_sphere[4];
vec2   stex_coords[NumVerticesSphere];

// quad generates two triangles for each face and assigns colors to the vertices
void
quad_sphere( int a, int b, int c, int d, float t0, float t1, float p0, float p1 )
{
    points_sphere[Index] = vertices_sphere[a];
    bnormals[Index] = vec3(vertices_sphere[a].x, vertices_sphere[a].y, vertices_sphere[a].z);
    stex_coords[Index] = vec2( t0/(2*M_PI), -(p1-M_PI/2.0)/M_PI );
    Index++;
    points_sphere[Index] = vertices_sphere[b];
    bnormals[Index] = vec3(vertices_sphere[b].x, vertices_sphere[b].y, vertices_sphere[b].z);
    stex_coords[Index] = vec2( t1/(2*M_PI), -(p1-M_PI/2.0)/M_PI );
    Index++;
    points_sphere[Index] = vertices_sphere[c];
    bnormals[Index] = vec3(vertices_sphere[c].x, vertices_sphere[c].y, vertices_sphere[c].z);
    stex_coords[Index] = vec2( t1/(2*M_PI), -(p0-M_PI/2.0)/M_PI );
    Index++;
    points_sphere[Index] = vertices_sphere[a];
    bnormals[Index] = vec3(vertices_sphere[a].x, vertices_sphere[a].y, vertices_sphere[a].z);
    stex_coords[Index] = vec2( t0/(2*M_PI), -(p1-M_PI/2.0)/M_PI );
    Index++;
    points_sphere[Index] = vertices_sphere[c];
    bnormals[Index] = vec3(vertices_sphere[c].x, vertices_sphere[c].y, vertices_sphere[c].z);
    stex_coords[Index] = vec2( t1/(2*M_PI), -(p1-M_PI/2.0)/M_PI );
    Index++;
    points_sphere[Index] = vertices_sphere[d];
    bnormals[Index] = vec3(vertices_sphere[d].x, vertices_sphere[d].y, vertices_sphere[d].z);
    stex_coords[Index] = vec2( t0/(2*M_PI), -(p0-M_PI/2.0)/M_PI );
    Index++;
}

void
colorbube(void)
{
    float r = 0.5;
    Index = 0;
    float ph_top = ((float)((ssegments/2)-1)/(float)(ssegments/2)) * M_PI/2.0;
    float ph_bottom = -ph_top;
    
    cout << ph_bottom << endl;
    
    for ( int n = 0; n < ssegments; n++ )
    {
        GLfloat const t0 = 2 * M_PI * (float)n / (float)ssegments;
        GLfloat const t1 = 2 * M_PI * (float)(n+1) / (float)ssegments;
        
        points_sphere[Index].x = 0.0;
        points_sphere[Index].y = top;
        points_sphere[Index].z = 0.0;
        points_sphere[Index].w = 1.0;
        bnormals[Index] = vec3(points_sphere[Index].x, points_sphere[Index].y, points_sphere[Index].z);
        stex_coords[Index] = vec2( 0.0, 0.0);
        Index++;
        points_sphere[Index].x = cos(ph_top) * cos(t0) * r;
        points_sphere[Index].y = sin(ph_top) * r;
        points_sphere[Index].z = cos(ph_top) * sin(t0) * r;
        points_sphere[Index].w = 1.0;
        bnormals[Index] = vec3(points_sphere[Index].x, points_sphere[Index].y, points_sphere[Index].z);
        stex_coords[Index] = vec2( t0/(2*M_PI), -(ph_top-M_PI/2.0)/M_PI);
        Index++;
        points_sphere[Index].x = cos(ph_top) * cos(t1) * r;
        points_sphere[Index].y = sin(ph_top) * r;
        points_sphere[Index].z = cos(ph_top) * sin(t1) * r;
        points_sphere[Index].w = 1.0;
        bnormals[Index] = vec3(points_sphere[Index].x, points_sphere[Index].y, points_sphere[Index].z);
        stex_coords[Index] = vec2( t1/(2*M_PI), -(ph_top-M_PI/2.0)/M_PI);
        Index++;
    }
    
    for( int m = 1; m < ssegments-1; m++ )
    {
        float p0 = M_PI/2.0 - m * M_PI/(float)ssegments;
        float p1 = M_PI/2.0 - (m+1) * M_PI/(float)ssegments;
        
        int num;
        for ( int n = 0; n < ssegments; n++ )
        {
            num = 0;
            float x = 0.0, y = 0.0, r = 0.5;
            
            GLfloat const t0 = 2 * M_PI * (float)n / (float)ssegments;
            GLfloat const t1 = 2 * M_PI * (float)(n+1) / (float)ssegments;
            
            //quad vertex 0
            vertices_sphere[num].x = cos(p1)*cos(t0) * r;
            vertices_sphere[num].y = sin(p1) * r;
            vertices_sphere[num].z = cos(p1)*sin(t0) * r;
            vertices_sphere[num++].w = 1.0;
            //quad vertex 1
            vertices_sphere[num].x = cos(p1)*cos(t1) * r;
            vertices_sphere[num].y = sin(p1) * r;
            vertices_sphere[num].z = cos(p1)*sin(t1) * r;
            vertices_sphere[num++].w = 1.0;
            //quad vertex 2
            vertices_sphere[num].x = cos(p0)*cos(t1) * r;
            vertices_sphere[num].y = sin(p0) * r;
            vertices_sphere[num].z = cos(p0)*sin(t1) * r;
            vertices_sphere[num++].w = 1.0;
            //quad vertex 3
            vertices_sphere[num].x = cos(p0)*cos(t0) * r;
            vertices_sphere[num].y = sin(p0) * r;
            vertices_sphere[num].z = cos(p0)*sin(t0) * r;
            vertices_sphere[num++].w = 1.0;
            
            quad_sphere( 0, 1, 2, 3, t0, t1, p0, p1 );
        }
    }
    
    for ( int n = 0; n < ssegments; n++ )
    {
        GLfloat const t0 = 2 * M_PI * (float)n / (float)ssegments;
        GLfloat const t1 = 2 * M_PI * (float)(n+1) / (float)ssegments;
        
        points_sphere[Index].x = 0.0;
        points_sphere[Index].y = bottom;
        points_sphere[Index].z = 0.0;
        points_sphere[Index].w = 1.0;
        bnormals[Index] = vec3(points_sphere[Index].x, points_sphere[Index].y, points_sphere[Index].z);
        stex_coords[Index] = vec2( 0.0, 1.0);
        //stex_coords[Index] = vec2( 0.5 + atan2f(-points_sphere[Index].z, points_sphere[Index].x)/(2*M_PI), acosf(points_sphere[Index].y)/M_PI);
        Index++;
        points_sphere[Index].x = cos(ph_bottom) * cos(t0) * r;
        points_sphere[Index].y = sin(ph_bottom) * r;
        points_sphere[Index].z = cos(ph_bottom) * sin(t0) * r;
        points_sphere[Index].w = 1.0;
        bnormals[Index] = vec3(points_sphere[Index].x, points_sphere[Index].y, points_sphere[Index].z);
        stex_coords[Index] = vec2( t0/(2*M_PI), -(ph_bottom-M_PI/2.0)/M_PI);
        //stex_coords[Index] = vec2( 0.5 + atan2f(-points_sphere[Index].z, points_sphere[Index].x)/(2*M_PI), acosf(points_sphere[Index].y)/M_PI);
        Index++;
        points_sphere[Index].x = cos(ph_bottom) * cos(t1) * r;
        points_sphere[Index].y = sin(ph_bottom) * r;
        points_sphere[Index].z = cos(ph_bottom) * sin(t1) * r;
        points_sphere[Index].w = 1.0;
        bnormals[Index] = vec3(points_sphere[Index].x, points_sphere[Index].y, points_sphere[Index].z);
        stex_coords[Index] = vec2( t1/(2*M_PI), -(ph_bottom-M_PI/2.0)/M_PI);
        //stex_coords[Index] = vec2( 0.5 + atan2f(-points_sphere[Index].z, points_sphere[Index].x)/(2*M_PI), acosf(points_sphere[Index].y)/M_PI);
        Index++;
    }
    
}

//---- OpenGL initialization

GLuint program;
GLuint vPosition;
GLuint vColor;
GLuint vNormal;
GLuint vTexCoord;

GLuint textures[16];


size_t CUBE_OFFSET;
size_t CUBE_N_OFFSET;

size_t CYLINDER_OFFSET;
size_t CYLINDER_N_OFFSET;

size_t SPHERE_OFFSET;
size_t SPHERE_N_OFFSET;

size_t CUBE_TEX_OFFSET;
size_t SPHERE_TEX_OFFSET;

size_t TRI_OFFSET;
size_t TRI_N_OFFSET;
size_t TRI_TEX_OFFSET;

void
init()
{
    // Load shaders and use the resulting shader program
    program = InitShader( "vshader_lt.glsl", "fshader_lt.glsl" );
    glUseProgram( program );
    
    //---------------------------------------------------------------------
    colorcube();
    colortube();
    colorbube();
    colortri();
    
    //---- Initialize texture objects
    glGenTextures(16, textures);
    
    glActiveTexture( GL_TEXTURE0 );
    
    unsigned char* pic1 = NULL;
    int w,h;
    loadBMP_custom(&pic1, &w, &h, "Grass1.bmp");
    
    glBindTexture( GL_TEXTURE_2D, textures[0] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, pic1 );
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    
    unsigned char* pic2 = NULL;
    loadBMP_custom(&pic2, &w, &h, "Brick1.bmp");
    
    glBindTexture( GL_TEXTURE_2D, textures[1] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, pic2 );
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    
    unsigned char* pic3 = NULL;
    loadBMP_custom(&pic3, &w, &h, "Asphalt.bmp");
    
    glBindTexture( GL_TEXTURE_2D, textures[2] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, pic3 );
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    
    unsigned char* pic4 = NULL;
    loadBMP_custom(&pic4, &w, &h, "Concrete.bmp");
    
    glBindTexture( GL_TEXTURE_2D, textures[3] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, pic4 );
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    
    unsigned char* pic5 = NULL;
    loadBMP_custom(&pic5, &w, &h, "Wood.bmp");
    
    glBindTexture( GL_TEXTURE_2D, textures[4] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, pic5 );
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    
    unsigned char* pic6 = NULL;
    loadBMP_custom(&pic6, &w, &h, "Brick2.bmp");
    
    glBindTexture( GL_TEXTURE_2D, textures[5] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, pic6 );
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    
    unsigned char* pic7 = NULL;
    loadBMP_custom(&pic7, &w, &h, "Boards.bmp");
    
    glBindTexture(GL_TEXTURE_2D, textures[6]);
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, pic7 );
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    
    unsigned char* pic8 = NULL;
    loadBMP_custom(&pic8, &w, &h, "Sand.bmp");
    
    glBindTexture(GL_TEXTURE_2D, textures[7]);
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, pic8 );
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    
    unsigned char* pic9 = NULL;
    loadBMP_custom(&pic9, &w, &h, "Roof1.bmp");
    
    glBindTexture(GL_TEXTURE_2D, textures[8]);
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, pic9 );
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    
    
    unsigned char* pic10 = NULL;
    loadBMP_custom(&pic10, &w, &h, "Stone.bmp");
    
    glBindTexture(GL_TEXTURE_2D, textures[9]);
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, pic10 );
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    
    unsigned char* pic11 = NULL;
    loadBMP_custom(&pic11, &w, &h, "Stone2.bmp");
    
    glBindTexture(GL_TEXTURE_2D, textures[10]);
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, pic11 );
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    
    unsigned char* pic12 = NULL;
    loadBMP_custom(&pic12, &w, &h, "CStone.bmp");
    
    glBindTexture(GL_TEXTURE_2D, textures[11]);
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, pic12 );
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    
    unsigned char* pic13 = NULL;
    loadBMP_custom(&pic13, &w, &h, "Brick3.bmp");
    
    glBindTexture(GL_TEXTURE_2D, textures[12]);
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, pic13 );
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    
    unsigned char* pic14 = NULL;
    loadBMP_custom(&pic14, &w, &h, "Hotel.bmp");
    
    glBindTexture(GL_TEXTURE_2D, textures[13]);
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, pic14 );
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    
    //---- cube maps
    
    unsigned char* negx_s = NULL;
    loadBMP_custom(&negx_s, &w, &h, "negx_s.bmp");
    unsigned char* negy_s = NULL;
    loadBMP_custom(&negy_s, &w, &h, "negy_s.bmp");
    unsigned char* negz_s = NULL;
    loadBMP_custom(&negz_s, &w, &h, "negz_s.bmp");
    unsigned char* posx_s = NULL;
    loadBMP_custom(&posx_s, &w, &h, "posx_s.bmp");
    unsigned char* posy_s = NULL;
    loadBMP_custom(&posy_s, &w, &h, "posy_s.bmp");
    unsigned char* posz_s = NULL;
    loadBMP_custom(&posz_s, &w, &h, "posz_s.bmp");
    
    glActiveTexture( GL_TEXTURE1 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, textures[14] );
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, posx_s);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, posy_s);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, posz_s);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, negx_s);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, negy_s);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, negz_s);
    
    glTexParameterf( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    
    //---------------------------------------------------------------------
    
    //----set offset variables
    
    CUBE_OFFSET = 0;
    CUBE_N_OFFSET = sizeof(points_cube) + sizeof(colors) + sizeof(points_cylinder) + sizeof(points_sphere);
    
    CYLINDER_OFFSET = sizeof(points_cube) + sizeof(colors);
    CYLINDER_N_OFFSET = CUBE_N_OFFSET + sizeof(normals);
    
    SPHERE_OFFSET = CYLINDER_OFFSET + sizeof(points_cylinder);
    SPHERE_N_OFFSET = CYLINDER_N_OFFSET + sizeof(cnormals);
    
    CUBE_TEX_OFFSET = SPHERE_N_OFFSET + sizeof(bnormals);
    SPHERE_TEX_OFFSET = CUBE_TEX_OFFSET + sizeof(tex_coords);
    
    TRI_OFFSET = SPHERE_TEX_OFFSET + sizeof(stex_coords);
    TRI_N_OFFSET = TRI_OFFSET + sizeof(points_tri);
    TRI_TEX_OFFSET = TRI_N_OFFSET + sizeof(tnormals);
    
    //---- Create a vertex array object
    
    GLuint vao;
    glGenVertexArrays( 1, &vao );  // removed 'APPLE' suffix for 3.2
    glBindVertexArray( vao );
    
    // Create and initialize a buffer object
    GLuint buffer;
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof(points_cube) + sizeof(colors) +
                 sizeof(points_cylinder) + sizeof(points_sphere)+
                 sizeof(normals) + sizeof(cnormals) + sizeof(bnormals) + sizeof(tex_coords) +
                 sizeof(stex_coords) + sizeof(points_tri) + sizeof(tnormals) + sizeof(ttex_coords),
                 NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points_cube), points_cube );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(points_cube), sizeof(colors), colors );
    glBufferSubData( GL_ARRAY_BUFFER, CYLINDER_OFFSET, sizeof(points_cylinder), points_cylinder );
    glBufferSubData( GL_ARRAY_BUFFER, SPHERE_OFFSET, sizeof(points_sphere), points_sphere );
    glBufferSubData( GL_ARRAY_BUFFER, CUBE_N_OFFSET, sizeof(normals), normals );
    glBufferSubData( GL_ARRAY_BUFFER, CYLINDER_N_OFFSET, sizeof(cnormals), cnormals );
    glBufferSubData( GL_ARRAY_BUFFER, SPHERE_N_OFFSET, sizeof(bnormals), bnormals );
    glBufferSubData( GL_ARRAY_BUFFER, CUBE_TEX_OFFSET, sizeof(tex_coords), tex_coords );
    glBufferSubData( GL_ARRAY_BUFFER, SPHERE_TEX_OFFSET, sizeof(stex_coords), stex_coords );
    glBufferSubData( GL_ARRAY_BUFFER, TRI_OFFSET, sizeof(points_tri), points_tri);
    glBufferSubData( GL_ARRAY_BUFFER, TRI_N_OFFSET, sizeof(tnormals), tnormals);
    glBufferSubData( GL_ARRAY_BUFFER, TRI_TEX_OFFSET, sizeof(ttex_coords), ttex_coords);
    
    //---------------------------------------------------------------------
    
    // set up vertex arrays
    vPosition = glGetAttribLocation( program, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );
    
    vColor = glGetAttribLocation( program, "vColor" );
    glEnableVertexAttribArray( vColor );
    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points_cube)) );
    
    vNormal = glGetAttribLocation( program, "vNormal" );
    glEnableVertexAttribArray( vNormal );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points_cube) + sizeof(colors) + sizeof(points_cylinder) + sizeof(points_sphere)) );
    
    vTexCoord = glGetAttribLocation( program, "vTexCoord" );
    glEnableVertexAttribArray( vTexCoord );
    glVertexAttribPointer( vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_TEX_OFFSET) );
    
    // Set the value of the fragment shader texture sampler variable (myTextureSampler) to GL_TEXTURE0
    glUniform1i( glGetUniformLocation(program, "myTextureSampler"), 0 );
    glUniform1i( glGetUniformLocation(program, "myCubeSampler"), 1 );
    
    glUniform1i( glGetUniformLocation(program, "cube_map_on"), false );
    
    //---------------------------------------------------------------------
    
    //---- setup initial view
    float tr_y = Theta1[Yaxis]* M_PI/180.0;
    float tr_z = Theta1[Zaxis]* M_PI/180.0;
    float eye_x = r * (cos(tr_z)) * cos(tr_y);
    float eye_z = r * (cos(tr_z)) * sin(tr_y);
    float eye_y = r * sin(tr_z);
    
    view_matrix = LookAt( vec4(eye_x, eye_y, eye_z, 1.0), vec4(0.0, 0.0, 0.0, 1.0), vec4(0.0, cos(tr_z), 0.0, 0.0));
    default_view_matrix = view_matrix;
    
    glEnable( GL_DEPTH_TEST );
    glClearColor( 0.5, 0.8, 1.0, 1.0 );
}

//----------------------------------------------------------------------------

void
SetMaterial( vec4 ka, vec4 kd, vec4 ks, float s )
{
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
}

//----------------------------------------------------------------------------

void
SetLight( vec4 lpos, vec4 la, vec4 ld, vec4 ls )
{
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, lpos);
    glUniform4fv( glGetUniformLocation(program, "AmbientLight"), 1, la);
    glUniform4fv( glGetUniformLocation(program, "DiffuseLight"), 1, ld);
    glUniform4fv( glGetUniformLocation(program, "SpecularLight"), 1, ls);
}

//----------------------------------------------------------------------------
enum Shape { CUBE, CYL, SPHERE, TRI };
enum Texture { GRASS, BRICK, ASPHAULT, CEMENT, WOOD, BRICK2, BOARDS, SAND, ROOF, STONE, STONE2, CSTONE, BRICK3, HOTEL };

void drawObject(mat4 model, size_t offset, size_t n_offset, int numVertices)
{
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, model );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(offset) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(n_offset) );
    glDrawArrays( GL_TRIANGLES, 0, numVertices );
}

void drawTextObject(mat4 model, size_t offset, size_t n_offset, size_t t_offset, int numVertices, int texture, float st_factor)
{
    glBindTexture( GL_TEXTURE_2D, textures[texture] );
    glUniform1i( glGetUniformLocation(program, "texture_on"), true );
    glUniform1f( glGetUniformLocation(program, "st_factor"), st_factor );
    
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, model );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(offset) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(n_offset) );
    glVertexAttribPointer( vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(t_offset) );
    glDrawArrays( GL_TRIANGLES, 0, numVertices );
    
    glUniform1i( glGetUniformLocation(program, "texture_on"), false );
}

// will draw cube, cyl, or sphere without textures
void draw(mat4 model, Shape shape)
{
    switch (shape) {
        case CUBE:
            drawObject(model, CUBE_OFFSET, CUBE_N_OFFSET, NumVerticesCube);
            break;
        case CYL:
            drawObject(model, CYLINDER_OFFSET, CYLINDER_N_OFFSET, NumVerticesCylinder);
            break;
        case SPHERE:
            drawObject(model, SPHERE_OFFSET, SPHERE_N_OFFSET, NumVerticesSphere);
            break;
        case TRI:
            drawObject(model, TRI_OFFSET, TRI_N_OFFSET, NumVerticesTri);
        default:
            break;
    }
}

// will draw a cube or sphere with a texture
void draw(mat4 model, Shape shape, Texture texture, float st_factor)
{
    switch (shape) {
        case CUBE:
            drawTextObject(model, CUBE_OFFSET, CUBE_N_OFFSET, CUBE_TEX_OFFSET, NumVerticesCube, texture, st_factor);
            break;
        case SPHERE:
            drawTextObject(model, SPHERE_OFFSET, SPHERE_N_OFFSET, SPHERE_TEX_OFFSET, NumVerticesSphere, texture, st_factor);
            break;
        case TRI:
            drawTextObject(model, TRI_OFFSET, TRI_N_OFFSET, TRI_TEX_OFFSET, NumVerticesTri, texture, st_factor);
        default:
            break;
    }
}

void drawBuilding(mat4 position, vec3 size, float doorOffset, Texture texture)
{
    //--- foundation
    mat4 model = position * Translate(0.0, 0.01, 0.0) * Scale(size.x + 0.1, 0.01, size.z + 0.1);
    draw(model, CUBE, CEMENT, 4.0);
    
    //--- brick2 building
    model = position * Translate(0.0, size.y/2 + 0.001, 0.0) * Scale(size.x, size.y, size.z);
    draw(model, CUBE, texture, 4.0);
    
    //--- door
    model = position * Translate(doorOffset, 0.31 , -size.z/2) * Scale(0.3, 0.6, 0.01);
    draw(model, CUBE, WOOD, 4.0);
    
    vec3 roof_size = vec3(size.x, 0.1, size.z + 0.1);
    
    model = position * Translate(0.0, roof_size.y/2 + size.y + 0.01, 0.0) * Scale(roof_size) * RotateY(90) * RotateX(90);
    draw(model, TRI, ROOF, 4.0);
    /*
    //--- Roof 1
    model = position * Translate(0.0, roof_size.y/2 + size.y + 0.01, roof_size.z/2 ) * RotateX(2.5) * Scale(roof_size) * RotateY(90);
    draw(model, CUBE, ROOF, 4.0);
    
    //--- Roof 1
    model = position * Translate(0.0, roof_size.y/2 + size.y + 0.01, -roof_size.z/2 ) * RotateX(-2.5) * Scale(roof_size) * RotateY(90);
    draw(model, CUBE, ROOF, 4.0);
     */
}

void drawRoad(mat4 position, float length)
{
    //---- road
    SetMaterial( vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), 5.0);
    mat4 model = position * Scale(length, 0.01, 2.5);
    draw(model, CUBE, ASPHAULT, 4.0);
    
    //---- middle line
    SetMaterial( vec4(1.0,1.0,0.0,1.0), vec4(1.0,1.0,0.0,1.0), vec4(1.0,1.0,0.0,1.0), 10.0);
    model = position * Translate(0.0, 0.0105, 0.0) * Scale(length, 0.001, 0.125);
    draw(model, CUBE);
    
    //--- road 1 -right sidewalk 1
    SetMaterial(vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), 5.0);
    model = position * Translate( 0.0, 0.015, 1.5) * Scale(length, 0.04, 0.5);
    draw(model, CUBE);
    
    //--- road 1 -left sidewalk 2
    SetMaterial(vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), 5.0);
    model = position * Translate( 0.0, 0.015, -1.5) * Scale(length, 0.04, 0.5);
    draw(model, CUBE);
}

void drawParkingStruct(mat4 position, vec3 size)
{
    //----parking structure
    //--- foundation
    mat4 model = position * Translate( 0.0, 0.01, 0.0 ) * Scale(size.x, 0.01, size.z);
    draw(model, CUBE, CEMENT, 4.0);
    
    int num_levels = size.y / 1 + 1;
    vec3 pillarSize = vec3(size.y/(num_levels * 2 - 2), size.y, size.y/(num_levels * 2 - 2));
    
    //--- corner pillars
    SetMaterial(vec4(0.9, 0.9, 0.9, 1.0), vec4(0.9, 0.9, 0.9, 1.0), vec4(0.9, 0.9, 0.9, 1.0), 5.0);
    model = position * Translate( size.x/2-pillarSize.x/2, pillarSize.y/2+0.01, size.z/2-pillarSize.z/2 ) * Scale(pillarSize);
    draw(model, CUBE);
    model = position * Translate( -(size.x/2-pillarSize.x/2), pillarSize.y/2+0.01, size.z/2-pillarSize.z/2 ) * Scale(pillarSize);
    draw(model, CUBE);
    model = position * Translate( size.x/2-pillarSize.x/2, pillarSize.y/2+0.01, -(size.z/2-pillarSize.z/2) ) * Scale(pillarSize);
    draw(model, CUBE);
    model = position * Translate( -(size.x/2-pillarSize.x/2), pillarSize.y/2+0.01, -(size.z/2-pillarSize.z/2) ) * Scale(pillarSize);
    draw(model, CUBE);
    
    for (int i = 0; i < num_levels; i++) {
        model = position * Translate(0.0, ((pillarSize.x*2*i)+(pillarSize.x/2+0.01)), 0.0) * Scale(size.x, pillarSize.x, size.z);
        draw(model, CUBE);
    }
}

void drawLamp(vec3 position, mat4 grid_pos)
{
    //---- lamp post 1
    //--- metal stand 1
    SetMaterial( vec4(0.1, 0.1, 0.1, 1.0), vec4(0.1, 0.1, 0.1, 1.0), vec4(0.3, 0.3, 0.3, 1.0), 0.5 );
    
    mat4 transform_tube = grid_pos * Translate(position) * Translate( 0.0, 0.57, 0.0 ) * Scale(0.05, 1.0, 0.05);
    draw(transform_tube, CYL);
    transform_tube = grid_pos * Translate(position) * Translate( 0.0, 0.045, 0.0 ) * Scale(0.25, 0.05, 0.25);
    draw(transform_tube, CYL);
    
    //--- spherical light 1
    vec4 light_pos = vec4(position.x, position.y + .17, position.z, 1.0);
    SetLight( light_pos, vec4(0.4, 0.4, 0.4, 1.0), vec4(0.7, 0.7, 0.7, 1.0), vec4(0.7, 0.7, 0.7, 1.0) );
    SetMaterial( vec4(0.9, 0.9, 0.7, 1.0), vec4(0.9, 0.9, 0.7, 1.0), vec4(0.9, 0.9, 0.7, 1.0), 0.5);
    glUniform1i( glGetUniformLocation(program, "light_out"), true );
    
    mat4 transform_bube = grid_pos * Translate(position) * Translate( 0.0, 1.09, 0.0 ) * Scale(0.3, 0.3, 0.3);
    draw(transform_bube, SPHERE);
    
}

float rotx = 0.0;

void camera()
{
    //---- camera
    float left = -1.0;
    float right = 1.0;
    float bottom = -1.0;
    float top = 1.0;
    float zNear = 1.0;
    float zFar = 30.0;
    
    proj_matrix = Frustum( left, right, bottom, top, zNear, zFar );
    GLuint proj = glGetUniformLocation( program, "projection" );
    glUniformMatrix4fv( proj, 1, GL_TRUE, proj_matrix );
    
    if (spherical_cam_on)
    {
        float tr_y = Theta1[Yaxis]* M_PI/180.0;
        float tr_z = Theta1[Zaxis]* M_PI/180.0;
        float eye_x = r * (cos(tr_z)) * cos(tr_y);
        float eye_z = r * (cos(tr_z)) * sin(tr_y);
        float eye_y = r * sin(tr_z);
        
        vec4 up = vec4(0.0, cos(tr_z), 0.0, 0.0);
        
        view_matrix = LookAt( vec4(eye_x, eye_y, eye_z, 1.0), vec4(0.0, 0.0, 0.0, 1.0), up);
        
        glUniform4fv( glGetUniformLocation( program, "cam_location" ), 1, vec4(eye_x, eye_y, eye_z, 1.0) );
    }
    
    GLuint view = glGetUniformLocation( program, "view" );
    glUniformMatrix4fv( view, 1, GL_TRUE, view_matrix );
}

void grid1(mat4 grid_pos_matrix)
{
    //---- freeway road
    SetMaterial( vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), 5.0);
    mat4 model = grid_pos_matrix * Translate(0.0, 0.0, -0.75) * Scale(10.0, 0.01, 8.5);
    draw(model, CUBE, ASPHAULT, 4.0);
    
    //---- center divider
    model = grid_pos_matrix * Translate(0.0, 0.21, -0.75) * Scale( 0.1, 0.4, 8.5);
    draw(model, CUBE, CEMENT, 4.0);
    
    // left wall
    model = grid_pos_matrix * Translate(4.95, 0.51, -0.75) * Scale( 0.05, 1.0, 8.5);
    draw(model, CUBE, CEMENT, 4.0);
    
    //---- right fence
    model = grid_pos_matrix * Translate(-4.95, 0.21, -1.0) * Scale( 0.05, 0.4, 3.0);
    draw(model, CUBE, CEMENT, 4.0);
}

void grid2(mat4 grid_pos_matrix)
{
    //---- Grass
    mat4 model = grid_pos_matrix * Translate(-3.0, 0.0, -2.5) * Scale(4.0, 0.01, 5.0);
    draw(model, CUBE, GRASS, 4.0);
    
    //---- asphalt piece
    SetMaterial( vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), 5.0);
    model = grid_pos_matrix * Translate(2.0, 0.0, -2.5) * Scale(6.0, 0.01, 5.0);
    draw(model, CUBE, ASPHAULT, 4.0);
    
    //---- Road
    mat4 position = grid_pos_matrix * Translate(-2.125, 0.0, 1.75);
    drawRoad(position, 5.75);
    
    //---- driveway/road
    SetMaterial( vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), 5.0);
    model = grid_pos_matrix * Translate(2.0, 0.0, 1.5) * Scale(2.5, 0.01, 3.0);
    draw(model, CUBE, ASPHAULT, 4.0);
    
    //--- road 2 - sidewalk finish
    SetMaterial(vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), 5.0);
    model = grid_pos_matrix * Translate( 2.0, 0.015, 3.25) * Scale(2.5, 0.04, 0.5);
    draw(model, CUBE);
    
    //---- Road finish
    position = grid_pos_matrix * Translate(4.125, 0.0, 1.75);
    drawRoad(position, 1.75);
    
    //---- Store
    position = grid_pos_matrix * Translate(2.0, 0.0, -3.75) * RotateY(180);
    drawBuilding(position, vec3(6.0, 1.25, 2.5), 0, BRICK);
    
    //---- side fence
    model = grid_pos_matrix * Translate(-0.975, 0.21, -1.25) * Scale( 0.05, 0.4, 2.5);
    draw(model, CUBE, CEMENT, 4.0);
}

void grid3(mat4 grid_pos_matrix)
{
    //---- Grass
    mat4 model = grid_pos_matrix * Translate(0.0, 0.0, -2.5) * Scale(10.0, 0.01, 5.0);
    draw(model, CUBE, GRASS, 4.0);
    
    //---- Sidewalk
    SetMaterial(vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), 5.0);
    model = grid_pos_matrix * Translate( 0.0, 0.015, -2.5) * Scale(0.5, 0.04, 5.0);
    draw(model, CUBE);
    
    //---- Road
    mat4 position = grid_pos_matrix * Translate(0.0, 0.0, 1.75);
    drawRoad(position, 10.0);
}

void grid4(mat4 grid_pos_matrix)
{
    //---- Road 1
    //--- Road 1 (verticle lower part)
    mat4 position = grid_pos_matrix * Translate(3.25, 0.0, -2.25) * RotateY(90);
    drawRoad(position, 5.5);
    
    //--- Road 1 (crossroad)
    SetMaterial( vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), 5.0);
    mat4 model = grid_pos_matrix * Translate(3.25, 0.0, 1.75) * Scale(3.5, 0.01, 2.5);
    draw(model, CUBE, ASPHAULT, 4.0);
    
    //--- Road 1 (verticle upper part)
    position = grid_pos_matrix * Translate(3.25, 0.0, 3.25) * RotateY(90);
    drawRoad(position, 0.5);
    /*
    //--- Road 1 (verticle upper part)
    position = grid_pos_matrix * Translate(3.25, 0.0, 4.0) * RotateY(90);
    drawRoad(position, 2.0);
     */
    
    //---- Road 2 (horizontal)
    position = grid_pos_matrix * Translate(-1.75, 0.0, 1.75);
    drawRoad(position, 6.5);
    
    //---- asphault piece
    SetMaterial( vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), 5.0);
    model = grid_pos_matrix * Translate(-1.75, 0.0, -2.5) * Scale(6.5, 0.01, 5.0);
    draw(model, CUBE, ASPHAULT, 4.0);
    
    //---- Building 1
    position = grid_pos_matrix * Translate(0.0, 0.0, -1.875);
    drawBuilding(position, vec3(3.0, 1.25, 3.75), 0, STONE);
    
    //---- Building 2
    position = grid_pos_matrix * Translate(-3.5, 0.0, -1.875);
    drawBuilding(position, vec3(3.0, 2.5, 3.75), 0, BRICK2);
    
}

void grid5(mat4 grid_pos_matrix)
{
    //---- freeway road
    SetMaterial( vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), 5.0);
    mat4 model = grid_pos_matrix * Scale(10.0, 0.01, 10.0);
    draw(model, CUBE, ASPHAULT, 4.0);
    
    //---- center divider
    model = grid_pos_matrix * Translate(0.0, 0.21, 0.0) * Scale( 0.1, 0.4, 10.0);
    draw(model, CUBE, CEMENT, 4.0);
    
    // left wall
    model = grid_pos_matrix * Translate(4.95, 0.51, 0.0) * Scale( 0.05, 1.0, 10.0);
    draw(model, CUBE, CEMENT, 4.0);
    
    //---- right fence
    model = grid_pos_matrix * Translate(-4.95, 0.21, -0.625) * Scale( 0.05, 0.4, 2.75);
    draw(model, CUBE, CEMENT, 4.0);
}

void grid6(mat4 grid_pos_matrix)
{
    //---- Road 1
    //--- Road 1 (right piece)
    mat4 position = grid_pos_matrix * Translate(-3.0, 0.0, -3.25);
    drawRoad(position, 4.0);
    
    //---- driveway/road
    SetMaterial( vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), 5.0);
    mat4 model = grid_pos_matrix * Translate(0.25, 0.0, -3.0) * Scale(2.5, 0.01, 3.0);
    draw(model, CUBE, ASPHAULT, 4.0);
    
    //--- road 2 - sidewalk finish
    SetMaterial(vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), 5.0);
    model = grid_pos_matrix * Translate( 0.25, 0.015, -4.75) * Scale(2.5, 0.04, 0.5);
    draw(model, CUBE);
    
    //--- Road 1 (left piece)
    position = grid_pos_matrix * Translate(3.25, 0.0, -3.25);
    drawRoad(position, 3.5);
    
    //---- Grass
    model = grid_pos_matrix * Translate(-3.0, 0.0, 1.75) * Scale(4.0, 0.01, 6.5);
    draw(model, CUBE, GRASS, 4.0);
    
    //---- parking lot piece 1
    SetMaterial( vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), 5.0);
    model = grid_pos_matrix * Translate(2.875, 0.0, 0.875) * Scale(4.25, 0.01, 4.75);
    draw(model, CUBE, ASPHAULT, 4.0);
    
    //---- parking lot piece 2
    SetMaterial( vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), 5.0);
    model = grid_pos_matrix * Translate(-0.125, 0.0, -0.875) * Scale(1.75, 0.01, 1.25);
    draw(model, CUBE, ASPHAULT, 4.0);
    
    //---- Drive thru
    //--- curb (right side - verticle)
    SetMaterial(vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), 5.0);
    model = grid_pos_matrix * Translate( -0.875, 0.015, 2.375) * Scale(0.25, 0.04, 5.25);
    draw(model, CUBE);
    //--- drivethrough (verticle)
    SetMaterial( vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), 5.0);
    model = grid_pos_matrix * Translate(-0.125, 0.0, 1.625) * Scale(1.25, 0.01, 3.75);
    draw(model, CUBE, ASPHAULT, 4.0);
    //--- curb (left side - verticle)
    SetMaterial(vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), 5.0);
    model = grid_pos_matrix * Translate( 0.625, 0.015, 1.5) * Scale(0.25, 0.04, 3.5);
    draw(model, CUBE);
    //--- curb (bottom - horizontal)
    SetMaterial(vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), 5.0);
    model = grid_pos_matrix * Translate( 2.75, 0.015, 3.375) * Scale(4.5, 0.04, 0.25);
    draw(model, CUBE);
    //--- drivethrough (horizontal)
    SetMaterial( vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), 5.0);
    model = grid_pos_matrix * Translate(2.125, 0.0, 4.125) * Scale(5.75, 0.01, 1.25);
    draw(model, CUBE, ASPHAULT, 4.0);
    //--- curb (top - horizontal)
    SetMaterial(vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), 5.0);
    model = grid_pos_matrix * Translate( 2.125, 0.015, 4.875) * Scale(5.75, 0.04, 0.25);
    draw(model, CUBE);
    
    //---- Menu
    //--- pole
    SetMaterial( vec4(0.1, 0.1, 0.1, 1.0), vec4(0.1, 0.1, 0.1, 1.0), vec4(0.3, 0.3, 0.3, 1.0), 0.5 );
    mat4 transform_tube = grid_pos_matrix * Translate( 0.625, 0.166, 1.5 ) * Scale(0.05, 0.25, 0.05);
    draw(transform_tube, CYL);
    
    //-- board
    model = grid_pos_matrix * Translate( 0.625, 0.425, 1.5) * RotateY(70) * Scale( 0.5, 0.5, 0.075);
    draw(model, CUBE);
    
    //---- Fast food building
    position = grid_pos_matrix * Translate(1.625, 0.0, 2.0);
    drawBuilding(position, vec3(1.75, 1.25, 2.5), 0, BRICK);
    
    //---- sign
    //---- pole
    SetMaterial( vec4(0.1, 0.1, 0.1, 1.0), vec4(0.1, 0.1, 0.1, 1.0), vec4(0.3, 0.3, 0.3, 1.0), 0.5 );
    transform_tube = grid_pos_matrix * Translate( 4.75, 1.55, -1.75 ) * Scale(0.1, 3.0, 0.1);
    draw(transform_tube, CYL);
    
    //--- board
    model = grid_pos_matrix * Translate( 4.75, 3.425, -1.75) * Scale( 1.25, 0.75, 0.15);
    draw(model, CUBE);
    
    //---- side fence
    model = grid_pos_matrix * Translate(-1.0, 0.21, 1.75) * Scale( 0.05, 0.4, 6.5);
    draw(model, CUBE, CEMENT, 4.0);
}

void grid7(mat4 grid_pos_matrix)
{
    //---- Road
    mat4 position = grid_pos_matrix * Translate(-0.25, 0.0, -3.25);
    drawRoad(position, 10.5);
    
    //---- Grass
    mat4 model = grid_pos_matrix * Translate(0.0, 0.0, 1.75) * Scale(10.0, 0.01, 6.5);
    draw(model, CUBE, GRASS, 4.0);
    
    //---- fountain base
    SetMaterial(vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), 0.5);
    mat4 transform_tube = grid_pos_matrix * Translate( 0.0, 0.1401, 4.25 ) * Scale(1.0, 0.25, 1.0);
    draw(transform_tube, CYL);
    
    //---- fountain mid
    SetMaterial(vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), 0.5);
    transform_tube = grid_pos_matrix * Translate( 0.0, 0.2651, 4.25 ) * Scale(0.5, 0.25, 0.5);
    draw(transform_tube, CYL);
    
    //---- fountain top
    SetMaterial(vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), 0.5);
    transform_tube = grid_pos_matrix * Translate( 0.0, 0.4052, 4.25 ) * Scale(0.25, 0.25, 0.25);
    draw(transform_tube, CYL);
    
    //--- sidewalk 1
    SetMaterial(vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), 5.0);
    model = grid_pos_matrix * Translate( 0.0, 0.015, 4.25) * Scale(10.0, 0.04, 0.5);
    draw(model, CUBE);
    
    //--- sidewalk 2
    SetMaterial(vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), 5.0);
    model = grid_pos_matrix * Translate( 0.0, 0.015, 1.75) * Scale(0.5, 0.04, 6.5);
    draw(model, CUBE);
}

void grid8(mat4 grid_pos_matrix)
{
    //---- road 1 (bottom)
    mat4 position = grid_pos_matrix * Translate(-1.5, 0.0, -3.25);
    drawRoad(position, 7.0);
    
    //---- cross road piece
    SetMaterial( vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), 5.0);
    mat4 model = grid_pos_matrix * Translate(3.25, 0.0, -3.25) * Scale(2.5, 0.01, 3.5);
    draw(model, CUBE, ASPHAULT, 4.0);
    
    //---- road 2 (side)
    position = grid_pos_matrix * Translate(3.25, 0.0, 0.5) * RotateY(90);
    drawRoad(position, 4.0);
    
    //---- asphalt piece
    SetMaterial( vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), 5.0);
    model = grid_pos_matrix * Translate(-1.75, 0.0, 1.75) * Scale(6.5, 0.01, 6.5);
    draw(model, CUBE, ASPHAULT, 4.0);
    
    //---- driveway/road
    SetMaterial( vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), 5.0);
    model = grid_pos_matrix * Translate(3.0, 0.0, 3.75) * Scale(3.0, 0.01, 2.5);
    draw(model, CUBE, ASPHAULT, 4.0);
    
    //--- road 2 - sidewalk finish
    SetMaterial(vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), 5.0);
    model = grid_pos_matrix * Translate( 4.75, 0.015, 3.75) * Scale(0.5, 0.04, 2.5);
    draw(model, CUBE);
    
    //----building ( 3 in 1) w/ flat roof
    //---foundation 1
    model = grid_pos_matrix * Translate( 0.5, 0.01, 0.5 ) * Scale(2.1, 0.01, 4.1);
    draw(model, CUBE, CEMENT, 4.0);
    
    //--- brick building 1
    model = grid_pos_matrix * Translate( 0.5, 1.26, 0.5 ) * Scale(2.0, 2.5, 4.0);
    draw(model, CUBE, HOTEL, 4.0);
    
    //--- flat roof 1
    model = grid_pos_matrix * Translate( 0.5, 2.535, 0.5 ) * Scale(2.0, 0.05, 4.0);
    draw(model, CUBE, ROOF, 4.0);
    
    //---foundation 2
    model = grid_pos_matrix * Translate( -1.75, 0.01, -0.5 ) * Scale(2.5, 0.01, 2.1);
    draw(model, CUBE, CEMENT, 4.0);
    
    //--- middle building 2
    position = grid_pos_matrix * Translate(-1.75, 1.26, -0.5) * Scale(2.5, 2.5, 2.0);
    draw(position, CUBE, HOTEL, 4.0);
    
    //--- flat roof 2
    model = grid_pos_matrix * Translate( -1.75, 2.535, -0.5 ) * Scale(2.5, 0.05, 2.0);
    draw(model, CUBE, ROOF, 4.0);
    
    //---foundation 3
    model = grid_pos_matrix * Translate( -4.0, 0.01, 0.5 ) * Scale(2.1, 0.01, 4.1);
    draw(model, CUBE, CEMENT, 4.0);
    
    //--- brick building 3
    model = grid_pos_matrix * Translate( -4.0, 1.26, 0.5 ) * Scale(2.0, 2.5, 4.0);
    draw(model, CUBE, HOTEL, 4.0);
    
    //--- flat roof 3
    model = grid_pos_matrix * Translate( -4.0, 2.535, 0.5 ) * Scale(2.0, 0.05, 4.0);
    draw(model, CUBE, ROOF, 4.0);
}

void grid9(mat4 grid_pos_matrix)
{
    //---- freeway road
    SetMaterial( vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), 5.0);
    mat4 model = grid_pos_matrix * Scale(10.0, 0.01, 10.0);
    draw(model, CUBE, ASPHAULT, 4.0);
    
    //---- center divider
    model = grid_pos_matrix * Translate(0.0, 0.21, 0.0) * Scale( 0.1, 0.4, 10.0);
    draw(model, CUBE, CEMENT, 4.0);
    
    // left wall
    model = grid_pos_matrix * Translate(4.95, 0.51, 0.0) * Scale( 0.05, 1.0, 10.0);
    draw(model, CUBE, CEMENT, 4.0);
    
    //---- right fence
    model = grid_pos_matrix * Translate(-4.95, 0.21, -0.625) * Scale( 0.05, 0.4, 2.75);
    draw(model, CUBE, CEMENT, 4.0);
}

void grid11(mat4 grid_pos_matrix)
{
    //---- grass 1
    vec4 light_pos = grid_pos_matrix * vec4( 1.0, 1.17, -0.35, 1.0 );
    SetLight( light_pos, vec4(0.4, 0.4, 0.4, 1.0), vec4(0.7, 0.7, 0.7, 1.0), vec4(0.7, 0.7, 0.7, 1.0) );
    SetMaterial( vec4(0.0, 0.6, 0.3, 1.0), vec4(0.0, 0.6, 0.3, 1.0), vec4(0.0, 0.6, 0.3, 1.0), 0.5);
    glUniform1i( glGetUniformLocation(program, "light_out"), false );
    
    mat4 model = grid_pos_matrix * Scale(10.0, 0.01, 10.0);
    draw(model, CUBE, GRASS, 4.0);
    
    //---- building 1
    mat4 position = grid_pos_matrix * Translate( 2.5, 0.0, -3.75 );
    drawBuilding(position, vec3(5.0, 1.25, 2.5), 0, STONE);
    
    //---- building 2
    position = grid_pos_matrix * Translate(-1.5, 0.0, -4.0);
    drawBuilding(position, vec3(2.0, 3.75, 2.0), 0.0, STONE2);
    draw(model, CUBE, WOOD, 4.0);
    
    //---- building 3
    position = grid_pos_matrix * Translate(-4.0, 0.0, -3.75);
    drawBuilding(position, vec3(2.0, 1.25, 2.5), -0.5, BRICK2);
    
    //---- Courthouse
    //--- foundation
    model = grid_pos_matrix * Translate( 0.0, 0.01, 1.2 ) * Scale(7.1, 0.01, 5.6);
    draw(model, CUBE, CEMENT, 4.0);
    
    //--- main building
    SetMaterial(vec4(0.9, 0.9, 0.9, 1.0), vec4(0.9, 0.9, 0.9, 1.0), vec4(0.9, 0.9, 0.9, 1.0), 5.0);
    model = grid_pos_matrix * Translate( 0.0, 1.26, 0.4 ) * Scale(7.0, 2.5, 3.8);
    draw(model, CUBE);
    
    //---- stairs
    //--- stair 1
    model = grid_pos_matrix * Translate( 0.0, 0.31, 2.5 ) * Scale(7.0, 0.6, 0.4);
    draw(model, CUBE);
    //--- stair 2
    model = grid_pos_matrix * Translate( 0.0, 0.21, 2.9 ) * Scale(7.0, 0.4, 0.4);
    draw(model, CUBE);
    //--- stair 3
    model = grid_pos_matrix * Translate( 0.0, 0.11, 3.3 ) * Scale(7.0, 0.2, 0.4);
    draw(model, CUBE);
    //--- stair 4
    model = grid_pos_matrix * Translate( 0.0, 0.06, 3.7 ) * Scale(7.0, 0.1, 0.4);
    draw(model, CUBE);
    
    //--- pillar 1
    mat4 transform_tube = grid_pos_matrix * Translate( -3.0, 1.56, 2.5 ) * Scale(0.25, 1.9, 0.25);
    draw(transform_tube, CYL);
    //--- pillar 2
    transform_tube = grid_pos_matrix * Translate( 3.0, 1.56, 2.5 ) * Scale(0.25, 1.9, 0.25);
    draw(transform_tube, CYL);
    //--- pillar 3
    transform_tube = grid_pos_matrix * Translate( -1.0, 1.56, 2.5 ) * Scale(0.25, 1.9, 0.25);
    draw(transform_tube, CYL);
    //--- pillar 4
    transform_tube = grid_pos_matrix * Translate( 1.0, 1.56, 2.5 ) * Scale(0.25, 1.9, 0.25);
    draw(transform_tube, CYL);
    
    //--- Top
    SetMaterial(vec4(0.9, 0.9, 0.9, 1.0), vec4(0.9, 0.9, 0.9, 1.0), vec4(0.9, 0.9, 0.9, 1.0), 5.0);
    model = grid_pos_matrix * Translate( 0.0, 3.01, 0.6 ) * Scale(7.0, 1.0, 4.2) * RotateX(90);
    draw(model, TRI);
    
    //--- entrance sidewalk
    SetMaterial(vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), 5.0);
    model = grid_pos_matrix * Translate( 0.0, 0.01, 4.475) * Scale(3.0, 0.01, 1.05);
    draw(model, CUBE);
}

void grid12(mat4 grid_pos_matrix)
{
    //---- grass 1
    mat4 model = grid_pos_matrix * Translate( -1.75, 0.0, 0.0 ) * Scale(6.5, 0.01, 10.0);
    draw(model, CUBE, GRASS, 4.0);
    
    //---- road 1
    mat4 position = grid_pos_matrix * Translate( 3.25, 0.0, -0.25) * RotateY(90);
    drawRoad(position, 10.5);
    
    //---- office building 1
    position = grid_pos_matrix * Translate(0.0, 0.0, -3.5);
    drawBuilding(position, vec3(3.0, 5.0, 3.0), 0, STONE);
    
    //---- shop 2
    position = grid_pos_matrix * Translate(-2.5, 0.0, -3.75);
    drawBuilding(position, vec3(1.5, 1.25, 2.5), -0.5, BRICK);
    
    //---- shop 3
    position = grid_pos_matrix * Translate(-4.25, 0.0, -3.75);
    drawBuilding(position, vec3(1.5, 2.5, 2.5), 0, BRICK3);
    
    position = grid_pos_matrix * Translate(-1.5, 0.0, 2.5);
    drawParkingStruct(position, vec3(6.0, 4.0, 5.0));
}

void grid13(mat4 grid_pos_matrix)
{
    //---- freeway road
    SetMaterial( vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), 5.0);
    mat4 model = grid_pos_matrix * Scale(10.0, 0.01, 10.0);
    draw(model, CUBE, ASPHAULT, 4.0);
    
    //---- center divider
    model = grid_pos_matrix * Translate(0.0, 0.21, 0.0) * Scale( 0.1, 0.4, 10.0);
    draw(model, CUBE, CEMENT, 4.0);
    
    // left wall
    model = grid_pos_matrix * Translate(4.95, 0.51, 0.0) * Scale( 0.05, 1.0, 10.0);
    draw(model, CUBE, CEMENT, 4.0);
}

void grid14(mat4 grid_pos_matrix)
{
    //---- lamp post 1
    vec3 light_position = vec3(1.0, 0.0, 4.65);
    drawLamp(light_position, grid_pos_matrix);
    
    //---- Fountain
    //---- asphault circle
    SetMaterial( vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), 5.0);
    mat4 transform_bube = grid_pos_matrix * Translate( -3.0, 0.0351, 3.25 ) * Scale(4.0, 0.01, 4.0);
    draw(transform_bube, SPHERE, ASPHAULT, 4.0);
    
    //---- sidewalk circle
    SetMaterial(vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), 0.5);
    mat4 transform_tube = grid_pos_matrix * Translate( -3.0, 0.0151, 3.25 ) * Scale(5.0, 0.04, 5.0);
    draw(transform_tube, CYL);
    
    //---- fountain base
    SetMaterial(vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), 0.5);
    transform_tube = grid_pos_matrix * Translate( -3.0, 0.1401, 3.25 ) * Scale(1.0, 0.25, 1.0);
    draw(transform_tube, CYL);
    
    //---- fountain mid
    SetMaterial(vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), 0.5);
    transform_tube = grid_pos_matrix * Translate( -3.0, 0.2651, 3.25 ) * Scale(0.5, 0.25, 0.5);
    draw(transform_tube, CYL);
    
    //---- fountain top
    SetMaterial(vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), 0.5);
    transform_tube = grid_pos_matrix * Translate( -3.0, 0.4052, 3.25 ) * Scale(0.25, 0.25, 0.25);
    draw(transform_tube, CYL);
    
    //---- park 1
    mat4 model = grid_pos_matrix * Translate( 0.0, 0.0, -1.75 ) * Scale(10.0, 0.01, 6.5);
    draw(model, CUBE, GRASS, 4.0);
    
    //---- road 1
    mat4 position = grid_pos_matrix * Translate( 0.0, 0.0, 3.25);
    drawRoad(position, 10.0);
    
    //---- building
    position = grid_pos_matrix * Translate( 2.75, 0, -1.75) * RotateY(180);
    drawBuilding(position, vec3(4.5, 2.5, 6.5), 0, BRICK);
}

void grid15(mat4 grid_pos_matrix)
{
    //---- lamp post 1
    vec3 light_position = vec3(0.0, 0.0, 4.65);
    drawLamp(light_position, grid_pos_matrix);
    
    //---- Road 1
    //--- Road 1 (left piece)
    mat4 position = grid_pos_matrix * Translate(3.125, 0.0, 3.25);
    drawRoad(position, 3.75);
    
    //--- T road piece
    SetMaterial( vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), 5.0);
    mat4 model = grid_pos_matrix * Translate(0.0, 0.0, 3.0) * Scale(2.5, 0.01, 3.0);
    draw(model, CUBE, ASPHAULT, 4.0);
    
    //--- Road 1 - sidewalk finish
    SetMaterial(vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), 5.0);
    model = grid_pos_matrix * Translate( 0.0, 0.015, 4.75) * Scale(2.5, 0.04, 0.5);
    draw(model, CUBE);
    
    //--- Road 1 (right piece)
    position = grid_pos_matrix * Translate(-3.125, 0.0, 3.25);
    drawRoad(position, 3.75);
    
    //---- road 2
    position = grid_pos_matrix * Translate(0.0, 0.0, -1.75) * RotateY(90);
    drawRoad(position, 6.5);
    
    //---- park 1
    model = grid_pos_matrix * Translate( 3.375, 0.0, -1.75 ) * Scale(3.25, 0.01, 6.5);
    draw(model, CUBE, GRASS, 4.0);
    
    //---- parking lot (piece 1)
    SetMaterial( vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), 5.0);
    model = grid_pos_matrix * Translate( -3.375, 0.0, -1.75 ) * Scale(3.25, 0.01, 6.5);
    draw(model, CUBE, ASPHAULT, 4.0);
}

void grid16(mat4 grid_pos_matrix)
{
    //---- parking lot driveway
    SetMaterial( vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), 5.0);
    mat4 model = grid_pos_matrix * Translate( 2.5, 0.0, 3.0 ) * Scale(5.0, 0.01, 3.0);
    draw(model, CUBE, ASPHAULT, 4.0);
    
    //---- parking lot (piece 2)
    SetMaterial( vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), 5.0);
    model = grid_pos_matrix * Translate( 0.0, 0.0, -1.75 ) * Scale(10.0, 0.01, 6.5);
    draw(model, CUBE, ASPHAULT, 4.0);
    
    //--- foundation
    model = grid_pos_matrix * Translate( 2.5, 0.01, 1.0 ) * Scale(0.5, 0.01, 1.0);
    draw(model, CUBE, CEMENT, 4.0);
    
    //---- ticket booth
    // booth base
    model = grid_pos_matrix * Translate( 2.5, 0.126, 1.25 ) * Scale(0.25, 0.25, 0.25);
    draw(model, CUBE);
    
    //---- sign
    //---- pole
    SetMaterial( vec4(0.1, 0.1, 0.1, 1.0), vec4(0.1, 0.1, 0.1, 1.0), vec4(0.3, 0.3, 0.3, 1.0), 0.5 );
    mat4 transform_tube = grid_pos_matrix * Translate( 2.5, 0.626, 0.75 ) * Scale(0.1, 1.25, 0.1);
    draw(transform_tube, CYL);
    
    //--- board
    model = grid_pos_matrix * Translate( 2.5, 1.135, 0.75) * Scale( 1.25, 0.75, 0.15);
    draw(model, CUBE);
    
    //---- road 2
    mat4 position = grid_pos_matrix * Translate(-2.5, 0.0, 3.25);
    drawRoad(position, 5.0);
    
    //--- road 2 sidewalk finish
    SetMaterial(vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), 5.0);
    model = grid_pos_matrix * Translate( 0.75, 0.015, 4.75) * Scale(1.5, 0.04, 0.5);
    draw(model, CUBE);
    
    //----building 4 ( L Shaped)
    //---foundation 1
    model = grid_pos_matrix * Translate( -3.5, 0.01, -1.75 ) * Scale(3.1, 0.01, 6.6);
    draw(model, CUBE, CEMENT, 4.0);
    
    //--- brick building 1
    model = grid_pos_matrix * Translate( -3.5, 1.26, -1.75 ) * Scale(3.0, 2.5, 6.5);
    draw(model, CUBE, BRICK2, 4.0);
    
    //---foundation 2
    model = grid_pos_matrix * Translate( 1.5, 0.01, -4.0 ) * Scale(7.1, 0.01, 2.1);
    draw(model, CUBE, CEMENT, 4.0);
    
    //--- brick building 2
    model = grid_pos_matrix * Translate( 1.5, 1.26, -4.0 ) * Scale(7.0, 2.5, 2.0);
    draw(model, CUBE, BRICK2, 4.0);
    /*
     //--- balcony
     model = Translate( -8.5, 0.626, -9.5 ) * Scale(7.0, 1.25, 3.0);
     draw(model, CUBE, BRICK2, 4.0);
     */
}

void grid17(mat4 grid_pos_matrix)
{
    //--- sidewalk top piece
    SetMaterial(vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), 5.0);
    mat4 model = grid_pos_matrix * Translate( 2.25, 0.015, 3.25) * Scale(5.5, 0.04, 0.5);
    draw(model, CUBE);
    
    //---- road crossway (upper)
    SetMaterial( vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), 5.0);
    model = grid_pos_matrix * Translate( 3.5, 0.0, 1.75 ) * Scale(3.0, 0.01, 2.5);
    draw(model, CUBE, ASPHAULT, 4.0);
    
    //--- sidewalk side piece
    SetMaterial(vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), 5.0);
    model = grid_pos_matrix * Translate( 1.75, 0.015, 1.75) * Scale(0.5, 0.04, 2.5);
    draw(model, CUBE);
    
    //---- road
    mat4 position = grid_pos_matrix * Translate(3.25, 0.0, -1.0) * RotateY(90);
    drawRoad(position, 3.0);
    
    //---- road crossway (lower)
    SetMaterial( vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), 5.0);
    model = grid_pos_matrix * Translate( 3.5, 0.0, -3.75 ) * Scale(3.0, 0.01, 2.5);
    draw(model, CUBE, ASPHAULT, 4.0);
    
    //--- sidewalk side piece
    SetMaterial(vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), 5.0);
    model = grid_pos_matrix * Translate( 1.75, 0.015, -3.75) * Scale(0.5, 0.04, 2.5);
    draw(model, CUBE);
    
    //---- asphalt
    SetMaterial( vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), 5.0);
    model = grid_pos_matrix * Translate( 0.5, 0.0, -1.0 ) * Scale(2.0, 0.01, 8.0);
    draw(model, CUBE, ASPHAULT, 4.0);
    
    //---- shop 1
    position = grid_pos_matrix * Translate(0.5, 0.0, 1.5) * RotateY(90);
    drawBuilding(position, vec3(3.0, 1.25, 2.0), 0, CSTONE);
    
    //---- shop 2
    position = grid_pos_matrix * Translate(0.5, 0.0, -1.0) * RotateY(90);
    drawBuilding(position, vec3(2.0, 1.25, 2.0), 0, STONE2);
    
    //---- shop 3
    position = grid_pos_matrix * Translate(0.5, 0.0, -3.75) * RotateY(90);
    drawBuilding(position, vec3(2.5, 1.25, 2.0), 0, BRICK3);
    
    //---- boardwalk
    model = grid_pos_matrix * Translate(-1.25, 0.0, -0.75) * Scale(1.5, 0.01, 8.5);
    draw(model, CUBE, BOARDS, 4.0);
    
    //---- beach
    model = grid_pos_matrix * Translate(-3.5, 0.0, -0.75) * Scale(3.0, 0.01, 8.5);
    draw(model, CUBE, SAND, 4.0);
}

void grid18(mat4 grid_pos_matrix)
{
    //--- sidewalk side piece (upper)
    SetMaterial(vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), 5.0);
    mat4 model = grid_pos_matrix * Translate( 1.75, 0.015, 3.75) * Scale(0.5, 0.04, 2.5);
    draw(model, CUBE);
    
    //---- road crossway (upper)
    SetMaterial( vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), 5.0);
    model = grid_pos_matrix * Translate( 3.5, 0.0, 3.75 ) * Scale(3.0, 0.01, 2.5);
    draw(model, CUBE, ASPHAULT, 4.0);
    
    //---- road
    mat4 position = grid_pos_matrix * Translate(3.25, 0.0, 0.25) * RotateY(90);
    drawRoad(position, 4.5);
    
    //--- sidewalk side piece (lower)
    SetMaterial(vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), 5.0);
    model = grid_pos_matrix * Translate( 1.75, 0.015, -3.25) * Scale(0.5, 0.04, 2.5);
    draw(model, CUBE);
    
    //---- road crossway (lower)
    SetMaterial( vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), 5.0);
    model = grid_pos_matrix * Translate( 3.5, 0.0, -3.25 ) * Scale(3.0, 0.01, 2.5);
    draw(model, CUBE, ASPHAULT, 4.0);
    
    //--- road bottom piece
    model = grid_pos_matrix * Translate( 3.25, 0.0, -4.75) * RotateY(90);
    drawRoad(model, 0.5);
    
    //---- asphalt
    SetMaterial( vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), 5.0);
    model = grid_pos_matrix * Translate( 0.5, 0.0, 0.0 ) * Scale(2.0, 0.01, 10.0);
    draw(model, CUBE, ASPHAULT, 4.0);
    
    //---- shop 1
    position = grid_pos_matrix * Translate(0.5, 0.0, 3.5) * RotateY(90);
    drawBuilding(position, vec3(3.0, 1.25, 2.0), 0, BRICK3);
    
    //---- shop 2
    position = grid_pos_matrix * Translate(0.5, 0.0, 0.75) * RotateY(90);
    drawBuilding(position, vec3(2.5, 1.25, 2.0), 0, STONE2);
    
    //---- shop 3
    position = grid_pos_matrix * Translate(0.5, 0.0, -2.0) * RotateY(90);
    drawBuilding(position, vec3(2.0, 1.25, 2.0), 0, STONE);
    
    //---- shop 4
    position = grid_pos_matrix * Translate(0.5, 0.0, -4.0) * RotateY(90);
    drawBuilding(position, vec3(2.0, 1.25, 2.0), 0, BRICK2);
    
    //---- boardwalk
    model = grid_pos_matrix * Translate(-1.25, 0.0, 0.0) * Scale(1.5, 0.01, 10.0);
    draw(model, CUBE, BOARDS, 4.0);
    
    //---- beach
    model = grid_pos_matrix * Translate(-3.5, 0.0, 0.0) * Scale(3.0, 0.01, 10.0);
    draw(model, CUBE, SAND, 4.0);
}

void grid19(mat4 grid_pos_matrix)
{
    //---- road
    mat4 position = grid_pos_matrix * Translate(3.25, 0.0, -0.25) * RotateY(90);
    drawRoad(position, 10.5);
    
    //---- asphalt
    SetMaterial( vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), 5.0);
    mat4 model = grid_pos_matrix * Translate( 0.5, 0.0, 0.0 ) * Scale(2.0, 0.01, 10.0);
    draw(model, CUBE, ASPHAULT, 4.0);
    
    //---- shop 1
    position = grid_pos_matrix * Translate(0.5, 0.0, 3.5) * RotateY(90);
    drawBuilding(position, vec3(3.0, 1.25, 2.0), 0, STONE);
    
    //---- shop 2
    position = grid_pos_matrix * Translate(0.5, 0.0, 1.0) * RotateY(90);
    drawBuilding(position, vec3(2.0, 1.25, 2.0), 0, BRICK2);
    
    //---- shop 3
    position = grid_pos_matrix * Translate(0.5, 0.0, -1.75) * RotateY(90);
    drawBuilding(position, vec3(2.5, 1.25, 2.0), 0, STONE2);
    
    //---- shop 4
    position = grid_pos_matrix * Translate(0.5, 0.0, -4.0) * RotateY(90);
    drawBuilding(position, vec3(2.0, 1.25, 2.0), 0, BRICK3);
    
    //---- boardwalk
    model = grid_pos_matrix * Translate(-1.25, 0.0, 0.0) * Scale(1.5, 0.01, 10.0);
    draw(model, CUBE, BOARDS, 4.0);
    
    //---- beach
    model = grid_pos_matrix * Translate(-3.5, 0.0, 0.0) * Scale(3.0, 0.01, 10.0);
    draw(model, CUBE, SAND, 4.0);
}

void grid20(mat4 grid_pos_matrix)
{
    //--- sidewalk side piece (upper)
    SetMaterial(vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), vec4(0.8, 0.8, 0.8, 1.0), 5.0);
    mat4 model = grid_pos_matrix * Translate( 1.75, 0.015, 3.25) * Scale(0.5, 0.04, 2.5);
    draw(model, CUBE);
    
    //---- road crossway (upper)
    SetMaterial( vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), 5.0);
    model = grid_pos_matrix * Translate( 3.5, 0.0, 3.25 ) * Scale(3.0, 0.01, 2.5);
    draw(model, CUBE, ASPHAULT, 4.0);
    
    //---- road
    mat4 position = grid_pos_matrix * Translate(3.25, 0.0, -1.5) * RotateY(90);
    drawRoad(position, 7.0);
    
    //---- asphalt
    SetMaterial( vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), 5.0);
    model = grid_pos_matrix * Translate( 0.5, 0.0, 0.0 ) * Scale(2.0, 0.01, 10.0);
    draw(model, CUBE, ASPHAULT, 4.0);
    
    //---- shop 1
    position = grid_pos_matrix * Translate(0.5, 0.0, 3.25) * RotateY(90);
    drawBuilding(position, vec3(3.5, 1.25, 2.0), 0, BRICK3);
    
    //---- shop 2
    position = grid_pos_matrix * Translate(0.5, 0.0, 0.0) * RotateY(90);
    drawBuilding(position, vec3(2.0, 1.25, 2.0), 0, STONE2);
    
    //---- shop 3
    position = grid_pos_matrix * Translate(0.5, 0.0, -2.0) * RotateY(90);
    drawBuilding(position, vec3(2.0, 1.25, 2.0), 0, STONE);
    
    //---- shop 4
    position = grid_pos_matrix * Translate(0.5, 0.0, -4.0) * RotateY(90);
    drawBuilding(position, vec3(2.0, 1.25, 2.0), 0, BRICK);
    
    //---- boardwalk
    model = grid_pos_matrix * Translate(-1.25, 0.0, 0.0) * Scale(1.5, 0.01, 10.0);
    draw(model, CUBE, BOARDS, 4.0);
    
    //---- beach
    model = grid_pos_matrix * Translate(-3.5, 0.0, 0.0) * Scale(3.0, 0.01, 10.0);
    draw(model, CUBE, SAND, 4.0);
}

//-----------CITY GRID---------
//1------2------3------4-----17
//5------6------7------8-----18
//9-----10-----11-----12-----19
//13----14-----15-----16-----20

// Grid 11 = (0,0) = CENTER
// 10 x 10 Grids
void loadGrids()
{
    grid1(Translate(20.0, 0.0, 20.0));
    grid2(Translate(10.0, 0.0, 20.0));
    grid3(Translate(0.0, 0.0, 20.0));
    grid4(Translate(-10.0, 0.0, 20.0));
    grid5(Translate(20.0, 0.0, 10.0));
    grid6(Translate(10.0, 0.0, 10.0));
    grid7(Translate(0.0, 0.0, 10.0));
    grid8(Translate(-10.0, 0.0, 10.0));
    grid9(Translate(20.0, 0.0, 0.0));
    grid11(Translate(0.0, 0.0, 0.0));
    grid12(Translate(-10.0, 0.0, 0.0));
    grid13(Translate(20.0, 0.0, -10.0));
    grid14(Translate(10.0, 0.0, -10.0));
    grid15(Translate(0.0, 0.0, -10.0));
    grid16(Translate(-10.0, 0.0, -10.0));
    grid17(Translate(-20.0, 0.0, 20.0));
    grid18(Translate(-20.0, 0.0, 10.0));
    grid19(Translate(-20.0, 0.0, 0.0));
    grid20(Translate(-20.0, 0.0, -10.0));
}

void
display( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glActiveTexture( GL_TEXTURE0 );
    camera();
    
    loadGrids();
    
    //grid11(Translate(-10.0, 0.0, 10.0));
    //grid8(Translate(0.0, 0.0, 0.0));
    
    //---- sphere w/ cube map
    
     glActiveTexture( GL_TEXTURE1 );
     glBindTexture( GL_TEXTURE_CUBE_MAP, textures[14] );
     glUniform1i( glGetUniformLocation(program, "light_out"), false );
     glUniform1i( glGetUniformLocation(program, "texture_on"), true );
     glUniform1i( glGetUniformLocation(program, "cube_map_on"), true );
     glUniform1f( glGetUniformLocation(program, "st_factor"), 1.0 );
     
     SetMaterial( vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), vec4(1.0,1.0,1.0,1.0), 2.0);
     
     mat4 transform_bube = Translate( 0.0, 1.0, -1.0 ) * RotateY(180) * Scale(80.0, 80.0, 80.0);
     glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform_bube );
     
     glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
     glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_N_OFFSET) );
     glVertexAttribPointer( vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_TEX_OFFSET) );
     glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
     glUniform1i( glGetUniformLocation(program, "texture_on"), false );
     glUniform1i( glGetUniformLocation(program, "cube_map_on"), false );
    
    glutSwapBuffers();
}
//----------------------------------------------------------------------------

void
keyboard( unsigned char key, int x, int y )
{
    switch( key ) {
        case 033:    //---- escape key
        case 'q': case 'Q':
            exit( EXIT_SUCCESS );
            break;
            
        case 'u':
            view_matrix = default_view_matrix;
            Theta1[Xaxis] = 0.0;
            Theta1[Yaxis] = 0.0;
            Theta1[Zaxis] = 0.0;
            glutPostRedisplay();
            break;
            
            //---- cases for spherical viewer
        case 's':    //---- spherical cam on
            Theta1[Xaxis] = 0.0;
            Theta1[Yaxis] = 0.0;
            Theta1[Zaxis] = 0.0;
            spherical_cam_on = true;
            glutPostRedisplay();
            break;
        case 'S':    //---- spherical cam off
            spherical_cam_on = false;
            glutPostRedisplay();
            break;
            
        case 'y':    //---- theta
            Axis = Yaxis;
            Theta1[Axis] += 5.0;
            Theta1[Axis] = fmod(Theta1[Axis], 360.0);
            glutPostRedisplay();
            break;
        case 'z':    //---- phi
            Axis = Zaxis;
            Theta1[Axis] += 5.0;
            Theta1[Axis] = fmod(Theta1[Axis], 360.0);
            glutPostRedisplay();
            break;
        case 'r':    //---- increase radius
            r += 0.5;
            glutPostRedisplay();
            break;
        case 'R':    //---- decrease radius
            r -= 0.5;
            glutPostRedisplay();
            break;
            
            //---- cases for flying viewer
            
        case 'f':    //---- forward
            view_matrix = Translate(0.0, 0.0, 0.2) * view_matrix;
            glutPostRedisplay();
            break;
        case 'b':    //---- back
            view_matrix = Translate(0.0, 0.0, -0.2) * view_matrix;
            glutPostRedisplay();
            break;
        case 'j':    //---- pan left
            view_matrix = RotateY(-1.0) * view_matrix;
            glutPostRedisplay();
            break;
        case 'k':    //---- pan right
            view_matrix = RotateY(1.0) * view_matrix;
            glutPostRedisplay();
            break;
    }
}

//----------------------------------------------------------------------------

void
mouse( int button, int state, int x, int y )
{
    if ( state == GLUT_DOWN ) {
        switch( button ) {
            case GLUT_LEFT_BUTTON:    Axis = Xaxis;  break;
            case GLUT_MIDDLE_BUTTON:  Axis = Yaxis;  break;
            case GLUT_RIGHT_BUTTON:   Axis = Zaxis;  break;
        }
    }
}

//----------------------------------------------------------------------------

void
idle( void )
{
    rotx = fmod(rotx + 1.0, 360.0);
    glutPostRedisplay();
}

//----------------------------------------------------------------------------

void
reshape( int w, int h )
{
    //glViewport(0,0,w,h);
}

//----------------------------------------------------------------------------

int
main( int argc, char **argv )
{
    glutInit( &argc, argv );
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
    glutInitWindowSize( 800, 800 );
    glutCreateWindow( "Color Cube" );
    
    init();
    
    glutDisplayFunc( display );
    glutReshapeFunc( reshape );
    glutKeyboardFunc( keyboard );
    glutMouseFunc( mouse );
    glutIdleFunc( idle );
    
    glutMainLoop();
    return 0;
}