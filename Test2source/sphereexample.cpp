// =================== EXAMPLE CAMERA CODE
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


//---- some variables for our camera
//----------------------------------------------------------------------------

// array of rotation angles (in degrees) for each axis
enum { Xaxis = 0, Yaxis = 1, Zaxis = 2, NumAxes = 3 };
int      Axis = Zaxis;
GLfloat  Theta1[NumAxes] = { 0.0, 0.0, 0.0 };

float r = 2.0;

mat4 view_matrix, default_view_matrix;
mat4 proj_matrix;

bool spherical_cam_on = true;


//---- cube model
//----------------------------------------------------------------------------
const int NumVerticesCube = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)
point4 points_cube[NumVerticesCube];
color4 colors[NumVerticesCube];

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
    colors[Index] = vertex_colors[a]; points_cube[Index] = vertices[a]; Index++;
    colors[Index] = vertex_colors[a]; points_cube[Index] = vertices[b]; Index++;
    colors[Index] = vertex_colors[a]; points_cube[Index] = vertices[c]; Index++;
    colors[Index] = vertex_colors[a]; points_cube[Index] = vertices[a]; Index++;
    colors[Index] = vertex_colors[a]; points_cube[Index] = vertices[c]; Index++;
    colors[Index] = vertex_colors[a]; points_cube[Index] = vertices[d]; Index++;
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


//---- cylinder model
//----------------------------------------------------------------------------
const int segments = 16;
const int NumVerticesCylinder = segments*6;
point4 points_cylinder[NumVerticesCylinder];
point4 vertices_cylinder[4];

// quad generates two triangles for each face and assigns colors to the vertices
void
quad_cylinder( int a, int b, int c, int d )
{
    points_cylinder[Index] = vertices_cylinder[a]; Index++;
    points_cylinder[Index] = vertices_cylinder[b]; Index++;
    points_cylinder[Index] = vertices_cylinder[c]; Index++;
    points_cylinder[Index] = vertices_cylinder[a]; Index++;
    points_cylinder[Index] = vertices_cylinder[c]; Index++;
    points_cylinder[Index] = vertices_cylinder[d]; Index++;
}


float const bottom = -0.5;
float const top    = 0.5;

void
colortube(void)
{
    Index = 0;
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
    
}

//---- sphere model
//----------------------------------------------------------------------------
const int ssegments = 16;
const int NumVerticesSphere = ssegments*ssegments*6;
point4 points_sphere[NumVerticesSphere];
point4 vertices_sphere[4];

// quad generates two triangles for each face and assigns colors to the vertices
void
quad_sphere( int a, int b, int c, int d )
{
    points_sphere[Index] = vertices_sphere[a]; Index++;
    points_sphere[Index] = vertices_sphere[b]; Index++;
    points_sphere[Index] = vertices_sphere[c]; Index++;
    points_sphere[Index] = vertices_sphere[a]; Index++;
    points_sphere[Index] = vertices_sphere[c]; Index++;
    points_sphere[Index] = vertices_sphere[d]; Index++;
}

void
colorsphere(void)
{
    Index = 0;
    int num;
    for ( int n = 0; n < ssegments; n++ )
    {
        GLfloat const t0 = 2 * M_PI * (float)n / (float)ssegments;
        GLfloat const t1 = 2 * M_PI * (float)(n+1) / (float)ssegments;
        for (int j = 0; j < ssegments; j++)
        {
            num = 0;
            float x = 0.0, y = 0.0, r = 0.5;
            
            GLfloat const p0 = 2 * M_PI * (float)j / (float)ssegments;
            GLfloat const p1 = 2 * M_PI * (float)(j+1) / (float)ssegments;
            
            //quad vertex 0
            vertices_sphere[num].x = sin(t0) * cos(p0) * r;
            vertices_sphere[num].y = sin(t0) * sin(p0) * r;
            vertices_sphere[num].z = cos(t0) * r;
            vertices_sphere[num++].w = 1.0;
            //quad vertex 1
            vertices_sphere[num].x = sin(t1) * cos(p0) * r;
            vertices_sphere[num].y = sin(t1) * sin(p0) * r;
            vertices_sphere[num].z = cos(t1) * r;
            vertices_sphere[num++].w = 1.0;
            //quad vertex 2
            vertices_sphere[num].x = sin(t1) * cos(p1) * r;
            vertices_sphere[num].y = sin(t1) * sin(p1) * r;
            vertices_sphere[num].z = cos(t1) * r;
            vertices_sphere[num++].w = 1.0;
            //quad vertex 3
            vertices_sphere[num].x = sin(t0) * cos(p1) * r;
            vertices_sphere[num].y = sin(t0) * sin(p1) * r;
            vertices_sphere[num].z = cos(t0) * r;
            vertices_sphere[num++].w = 1.0;
            
            quad_sphere( 0, 1, 2, 3 );
        }
    }
    
}

//---- OpenGL initialization

GLuint program;
GLuint vPosition;

void
init()
{
    // Load shaders and use the resulting shader program
    program = InitShader( "vshader31.glsl", "fshader31.glsl" );
    glUseProgram( program );
    
    //---------------------------------------------------------------------
    colorcube();
    colortube();
    colorsphere();
    //---------------------------------------------------------------------
    
    // Create a vertex array object
    GLuint vao;
    //glGenVertexArraysAPPLE( 1, &vao );
    //glBindVertexArrayAPPLE( vao );
    glGenVertexArrays( 1, &vao );  // removed 'APPLE' suffix for 3.2
    glBindVertexArray( vao );
    
    // Create and initialize a buffer object
    GLuint buffer;
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof(points_cube) + sizeof(colors) + sizeof(points_cylinder) + sizeof(points_sphere), NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points_cube), points_cube );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(points_cube), sizeof(colors), colors );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(points_cube) + sizeof(colors), sizeof(points_cylinder), points_cylinder );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(points_cube) + sizeof(colors) + sizeof(points_cylinder), sizeof(points_sphere), points_sphere);
    
    //---------------------------------------------------------------------
    
    // set up vertex arrays
    vPosition = glGetAttribLocation( program, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );
    
    GLuint vColor = glGetAttribLocation( program, "vColor" );
    glEnableVertexAttribArray( vColor );
    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points_cube)) );
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
    glClearColor( 1.0, 1.0, 1.0, 1.0 );
}

//----------------------------------------------------------------------------
float rotx = 0.0;


void
display( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    float left = -1.0;
    float right = 1.0;
    float bottom = -1.0;
    float top = 1.0;
    float zNear = 1.0;
    float zFar = 5.0;
    
    proj_matrix = Frustum( left, right, bottom, top, zNear, zFar );
    GLuint proj = glGetUniformLocation( program, "projection" );
    glUniformMatrix4fv( proj, 1, GL_TRUE, proj_matrix );
    
    if (spherical_cam_on)
    {
        float tr_y = Theta1[Yaxis]* M_PI/180.0;
        float tr_z = Theta1[Zaxis]* M_PI/180.0;
        //float eye_x = r * cos(tr_y);
        //float eye_z = r * sin(tr_y);
        float eye_x = r * (cos(tr_z)) * cos(tr_y);
        float eye_z = r * (cos(tr_z)) * sin(tr_y);
        float eye_y = r * sin(tr_z);
        
        vec4 up = vec4(0.0, cos(tr_z), 0.0, 0.0);
        cout << up << ' ' << normalize(up) << endl;
        
        view_matrix = LookAt( vec4(eye_x, eye_y, eye_z, 1.0), vec4(0.0, 0.0, 0.0, 1.0), vec4(0.0, cos(tr_z), 0.0, 0.0));
    }
    
    cout << view_matrix << endl;
    
    GLuint view = glGetUniformLocation( program, "view" );
    glUniformMatrix4fv( view, 1, GL_TRUE, view_matrix );
    
    /*
     //---- cube
     
     mat4 model_matrix = RotateX(rotx);// = Translate( 0.25, 0.0, -2.5 ) * rot_cube * Scale(0.2, 0.2, 0.5);
     
     GLuint model = glGetUniformLocation( program, "model" );
     glUniformMatrix4fv( model, 1, GL_TRUE, model_matrix );
     
     glUniform1i( glGetUniformLocation(program, "obj_color_on"), false );
     
     glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );
     glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );
     
     //---- cylinder
     
     for ( int i = 0; i < 100; i++) {
     mat4 transform_tube = Translate( rand()%10-5, rand()%10-5, rand()%10-5) * Scale(0.3, 0.3, 0.3);
     
     model = glGetUniformLocation( program, "model" );
     glUniformMatrix4fv( model, 1, GL_TRUE, transform_tube );
     
     glUniform1i( glGetUniformLocation(program, "obj_color_on"), true );
     glUniform4fv( glGetUniformLocation(program, "obj_color"), 1, vec4(0.0, 0.0, 0.8, 1.0) );
     
     glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points_cube) + sizeof(colors)) );
     glDrawArrays( GL_TRIANGLES, 0, NumVerticesCylinder );
     }
     */
    //---- sphere
    mat4 transform_sphere = Translate( 1.0, 0.0, 0.0 ) * Scale(0.3, 0.3, 0.3);
    
    GLuint model = glGetUniformLocation( program, "model" );
    glUniformMatrix4fv( model, 1, GL_TRUE, transform_sphere );
    
    glUniform1i( glGetUniformLocation(program, "obj_color_on"), true );
    glUniform4fv( glGetUniformLocation(program, "obj_color"), 1, vec4(0.0, 0.0, 0.8, 1.0) );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points_cube) + sizeof(colors) + sizeof(points_cylinder)) );
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    
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
    //glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
    glutInitWindowSize( 800, 800 );
    glutCreateWindow( "Color Cube" );
    
    init();
    
    glutDisplayFunc( display );
    glutReshapeFunc( reshape );
    glutKeyboardFunc( keyboard );
    glutMouseFunc( mouse );
    //glutIdleFunc( idle );
    
    glutMainLoop();
    return 0;
}