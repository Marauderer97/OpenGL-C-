#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <map>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;

struct COLOR {
    int r;
    int g;
    int b;
};
typedef struct COLOR COLOR;

struct Sprite {
    string name;
    COLOR color;
    float x,y;
    VAO* object;
    int status;
    float height,width;
    float x_speed,y_speed;
    int inAir;
    float radius;
    int fixed;
};
typedef struct Sprite Sprite;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

map <string, Sprite> objects;

float gravity = 0.01;
float airResistance = 0.003;

pair<float,float> moveObject(string name, float dx, float dy) {
    objects[name].x+=dx;
    objects[name].y+=dy;
    return make_pair(objects[name].x,objects[name].y);
}

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, GLfloat* vertex_buffer_data, GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, GLfloat* vertex_buffer_data, GLfloat red, GLfloat green, GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/

float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
     // Function is called first on GLFW_PRESS.

    if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_C:
                rectangle_rot_status = !rectangle_rot_status;
                break;
            case GLFW_KEY_P:
                triangle_rot_status = !triangle_rot_status;
                break;
            case GLFW_KEY_X:
                // do something ..
                break;
            default:
                break;
        }
    }
    else if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;
            default:
                break;
        }
    }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
            quit(window);
            break;
		default:
			break;
	}
}

double mouse_x,mouse_y;
double mouse_x_old,mouse_y_old;

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            if (action == GLFW_PRESS) {
                glfwGetCursorPos(window,&mouse_x_old,&mouse_y_old);
            }
            if (action == GLFW_RELEASE) {
                glfwGetCursorPos(window,&mouse_x,&mouse_y);
                if(objects["vishrectangle"].inAir == 0){
                    objects["vishrectangle"].inAir = 1;
                    //Set max jump speeds here (currently 300 and 300)
                    if(mouse_y_old-mouse_y==0)
                        objects["vishrectangle"].y_speed=0;
                    else
                        objects["vishrectangle"].y_speed = -(((mouse_y_old-mouse_y)/(abs(mouse_y_old-mouse_y))/1000)*min(abs(mouse_y_old-mouse_y),300.0));
                    if(mouse_x_old-mouse_x==0)
                        objects["vishrectangle"].x_speed=0;
                    else
                        objects["vishrectangle"].x_speed = (((mouse_x_old-mouse_x)/(abs(mouse_x_old-mouse_x))/1000)*min(abs(mouse_x_old-mouse_x),300.0));
                }
                triangle_rot_dir *= -1;
            }
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_RELEASE) {
                rectangle_rot_dir *= -1;
            }
            break;
        default:
            break;
    }
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
     is different from WindowSize */
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    // Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
    Matrices.projection = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 500.0f);
}


// Creates the triangle object used in this sample code
void createTriangle (string name, COLOR color, float x[], float y[])
{
  /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

  /* Define vertex array as used in glBegin (GL_TRIANGLES) */
  float xc=(x[0]+x[1]+x[2])/3;
  float yc=(y[0]+y[1]+y[2])/3;
  GLfloat vertex_buffer_data [] = {
    x[0]-xc,y[0]-yc,0, // vertex 0
    x[1]-xc,y[1]-yc,0, // vertex 1
    x[2]-xc,y[2]-yc,0 // vertex 2
  };

  GLfloat color_buffer_data [] = {
    color.r,color.g,color.b, // color 1
    color.r,color.g,color.b, // color 2
    color.r,color.g,color.b // color 3
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  VAO *triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_FILL);
  Sprite vishsprite = {};
  vishsprite.color = color;
  vishsprite.name = name;
  vishsprite.object = triangle;
  vishsprite.x=(x[0]+x[1]+x[2])/3; //Position of the sprite is the position of the centroid
  vishsprite.y=(y[0]+y[1]+y[2])/3;
  vishsprite.height=-1; //Height of the sprite is undefined
  vishsprite.width=-1; //Width of the sprite is undefined
  vishsprite.status=1;
  vishsprite.inAir=0;
  vishsprite.x_speed=0;
  vishsprite.y_speed=0;
  vishsprite.radius=-1; //The bounding circle radius is not defined.
  vishsprite.fixed=0;
  objects[name]=vishsprite;
}

// Creates the rectangle object used in this sample code
void createRectangle (string name, COLOR color, float x, float y, float height, float width)
{
  // GL3 accepts only Triangles. Quads are not supported
  float w=width/2,h=height/2;
  GLfloat vertex_buffer_data [] = {
    -w,-h,0, // vertex 1
    -w,h,0, // vertex 2
    w,h,0, // vertex 3

    w,h,0, // vertex 3
    w,-h,0, // vertex 4
    -w,-h,0  // vertex 1
  };

  GLfloat color_buffer_data [] = {
    color.r,color.g,color.b, // color 1
    color.r,color.g,color.b, // color 2
    color.r,color.g,color.b, // color 3
    
    color.r,color.g,color.b, // color 4
    color.r,color.g,color.b, // color 5
    color.r,color.g,color.b // color 6
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  VAO *rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
  Sprite vishsprite = {};
  vishsprite.color = color;
  vishsprite.name = name;
  vishsprite.object = rectangle;
  vishsprite.x=x;
  vishsprite.y=y;
  vishsprite.height=height;
  vishsprite.width=width;
  vishsprite.status=1;
  vishsprite.inAir=0;
  vishsprite.x_speed=0;
  vishsprite.y_speed=0;
  vishsprite.fixed=0;
  vishsprite.radius=(sqrt(height*height+width*width))/2;
  objects[name]=vishsprite;
}

void createCircle (string name, COLOR color, float x, float y, float r, int NoOfParts)
{
    int parts = NoOfParts;
    float radius = r;
    GLfloat vertex_buffer_data[parts*9];
    GLfloat color_buffer_data[parts*9];
    int i,j;
    float angle=(2*M_PI/parts);
    float current_angle = 0;
    for(i=0;i<parts;i++){
        for(j=0;j<3;j++){
            color_buffer_data[i*9+j*3]=color.r;
            color_buffer_data[i*9+j*3+1]=color.g;
            color_buffer_data[i*9+j*3+2]=color.b;
        }
        vertex_buffer_data[i*9]=0;
        vertex_buffer_data[i*9+1]=0;
        vertex_buffer_data[i*9+2]=0;
        vertex_buffer_data[i*9+3]=radius*cos(current_angle);
        vertex_buffer_data[i*9+4]=radius*sin(current_angle);
        vertex_buffer_data[i*9+5]=0;
	    vertex_buffer_data[i*9+6]=radius*cos(current_angle+angle);
        vertex_buffer_data[i*9+7]=radius*sin(current_angle+angle);
        vertex_buffer_data[i*9+8]=0;
    	current_angle+=angle;
    }
    VAO *circle = create3DObject(GL_TRIANGLES, (parts*9)/3, vertex_buffer_data, color_buffer_data, GL_FILL);
    Sprite vishsprite = {};
    vishsprite.color = color;
    vishsprite.name = name;
    vishsprite.object = circle;
    vishsprite.x=x;
    vishsprite.y=y;
    vishsprite.height=2*r; //Height of the sprite is 2*r
    vishsprite.width=2*r; //Width of the sprite is 2*r
    vishsprite.status=1;
    vishsprite.inAir=0;
    vishsprite.x_speed=0;
    vishsprite.y_speed=0;
    vishsprite.radius=r;
    vishsprite.fixed=0;
    objects[name]=vishsprite;
}

float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;

//Check collisions between rectangles only
//Bounding boxes collision
//Best Method
int checkCollision(string name, float dx, float dy){
    int colleft=0,colright=0,colbottom=0,coltop=0;
    for(map<string,Sprite>::iterator it2=objects.begin();it2!=objects.end();it2++){
        string colliding = it2->first;
        Sprite col_object = it2->second;
        Sprite my_object = objects[name];
        if(colliding!=name && col_object.height!=-1){ //Check collision only with circles and rectangles
            if(dx>0 && col_object.y+col_object.height/2>my_object.y-my_object.height/2 && col_object.y-col_object.height/2<my_object.y+my_object.height/2 && col_object.x-col_object.width/2<my_object.x+my_object.width/2 && col_object.x+col_object.width/2>my_object.x-my_object.width/2){
                if(col_object.fixed==0){
                    col_object.x_speed=my_object.x_speed/2;
                    col_object.inAir=1;
                }
                my_object.x_speed*=-1;
                my_object.x_speed/=1.6;
                my_object.x=col_object.x-col_object.width/2-my_object.width/2;
            }
            else if(dx<0 && col_object.y+col_object.height/2>my_object.y-my_object.height/2 && col_object.y-col_object.height/2<my_object.y+my_object.height/2 && col_object.x+col_object.width/2>my_object.x-my_object.width/2 && col_object.x-col_object.width/2<my_object.x+my_object.width/2){
                if(col_object.fixed==0){
                    col_object.x_speed=my_object.x_speed/2;
                    col_object.inAir=1;
                }
                my_object.x_speed*=-1;
                my_object.x_speed/=1.6;
                my_object.x=col_object.x+col_object.width/2+my_object.width/2;
            }
            if(dy>0 && col_object.x+col_object.width/2>my_object.x-my_object.width/2 && col_object.x-col_object.width/2<my_object.x+my_object.width/2 && col_object.y-col_object.height/2<my_object.y+my_object.height/2 && col_object.y+col_object.height/2>my_object.y-my_object.height/2){
                if(col_object.fixed==0){
                    col_object.y_speed=my_object.y_speed/2;
                    col_object.inAir=1;
                }
                my_object.y_speed*=-1;
                my_object.y_speed/=2;
                my_object.y=col_object.y-col_object.height/2-my_object.height/2;
            }
            else if(dy<0 && col_object.x+col_object.width/2>my_object.x-my_object.width/2 && col_object.x-col_object.width/2<my_object.x+my_object.width/2 && col_object.y+col_object.height/2>my_object.y-my_object.height/2 && col_object.y-col_object.height/2<my_object.y+my_object.height/2){
                if(col_object.fixed==0){
                    col_object.y_speed=my_object.y_speed/2;
                    col_object.inAir=1;
                }
                if(abs(objects[name].y_speed)<=0.05){
                    my_object.y_speed=0;
                    my_object.x_speed=0;
                    my_object.inAir=0;
                }
                my_object.y_speed*=-1;
                my_object.y_speed/=2;
                my_object.y=col_object.y+col_object.height/2+my_object.height/2;
            }
        }
        objects[name]=my_object;
        objects[colliding]=col_object;
    }
    return colleft+colright+colbottom+coltop;
}

//Check collision b/w rectangles or spheres
//This is not accurate or efficient but is useful when we have rotated objects since standard collision checks dont work well for rotated objects
//Call this function either with dx or dy only not both!
int checkCollisionSphere(string name,float dx, float dy){
    int collide=0;
    for(map<string,Sprite>::iterator it2=objects.begin();it2!=objects.end();it2++){
        string colliding = it2->first;
        Sprite col_object = it2->second;
        Sprite my_object = objects[name];
        if(colliding!=name && col_object.radius!=-1){ //Check collision only with circles and rectangles
            if((my_object.x-col_object.x)*(my_object.x-col_object.x)+(my_object.y-col_object.y)*(my_object.y-col_object.y)<(my_object.radius+col_object.radius)*(my_object.radius+col_object.radius))
            {
                if(dx!=0){
                    if(col_object.fixed==0){
                        col_object.x_speed=my_object.x_speed/2;
                        col_object.inAir=1;
                    }
                    my_object.x-=dx;
                    my_object.x_speed*=-1;
                    my_object.x_speed/=2;
                }
                else if(dy!=0){
                    if(col_object.fixed==0){
                        col_object.y_speed=my_object.y_speed/2;
                        col_object.inAir=1;
                    }
                    my_object.y-=dy;
                    my_object.y_speed*=-1;
                    my_object.y_speed/=2;
                    if(abs(my_object.y_speed)<=0.05){
                        my_object.y_speed=0;
                        my_object.x_speed=0;
                        my_object.inAir=0;
                    }
                }
                collide=1;
            }
        }
        objects[name]=my_object;
        objects[colliding]=col_object;
    }
    return collide;
}

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ()
{
  // clear the color and depth in the frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // use the loaded shader program
  // Don't change unless you know what you are doing
  glUseProgram (programID);

  // Eye - Location of camera. Don't change unless you are sure!!
  glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
  // Target - Where is the camera looking at.  Don't change unless you are sure!!
  glm::vec3 target (0, 0, 0);
  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
  glm::vec3 up (0, 1, 0);

  // Compute Camera matrix (view)
  // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
  Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
  glm::mat4 VP = Matrices.projection * Matrices.view;

  for(map<string,Sprite>::iterator it=objects.begin();it!=objects.end();it++){
    string current = it->first; //The name of the current object
    if(objects[current].fixed==0 && objects[current].inAir==0 && objects[current].y_speed==0 && current!="vishrectangle"){
        if(!checkCollision(current,0,0)){
            objects[current].inAir=1;
        }
    }
    if(objects[current].status==0)
        continue;
    if(objects[current].inAir && objects[current].fixed==0){
        if(objects[current].y_speed>=-0.2)
            objects[current].y_speed-=gravity;
        if(objects[current].x_speed>0)
            objects[current].x_speed-=airResistance;
        else if(objects[current].x_speed<0)
            objects[current].x_speed+=airResistance;
        pair<float,float> position = moveObject(current,objects[current].x_speed,0);
        //We can also use the checkCollisionSphere here instead but since we don't have any rotated blocks currently we will stick with this
        checkCollision(current,objects[current].x_speed,0); //Always call the checkCollision function with only 1 position change at a time!
        position = moveObject(current,0,objects[current].y_speed);
        //We can also use the checkCollisionSphere here instead but since we don't have any rotated blocks currently we will stick with this
        checkCollision(current,0,objects[current].y_speed);
    }
    // Send our transformation to the currently bound shader, in the "MVP" uniform
    // For each model you render, since the MVP will be different (at least the M part)
    //  Don't change unless you are sure!!
    glm::mat4 MVP;	// MVP = Projection * View * Model

    // Load identity to model matrix
    Matrices.model = glm::mat4(1.0f);

    /* Render your scene */

    glm::mat4 translateTriangle = glm::translate (glm::vec3(objects[current].x, objects[current].y, 0.0f)); // glTranslatef
    //glm::mat4 rotateTriangle = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
    glm::mat4 triangleTransform = translateTriangle;
    Matrices.model *= triangleTransform; 
    MVP = VP * Matrices.model; // MVP = p * V * M

    //  Don't change unless you are sure!!
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(objects[current].object);
    // Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
    //glPopMatrix ();
  }

  /*Matrices.model = glm::mat4(1.0f);

  glm::mat4 translateRectangle = glm::translate (glm::vec3(2, 0, 0));        // glTranslatef
  glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateRectangle * rotateRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(rectangle);*/

  // Increment angles
  float increments = 1;

  //camera_rotation_angle++; // Simulating camera rotation
  triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
  rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    /* --- register callbacks with GLFW --- */

    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
     is different from WindowSize */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders */
	// Create the models
    
    COLOR vishcolor = {0,1,1};

    float x[] = {0.0,0.0,1.0};
    float y[] = {0.0,1.0,1.0};
	//createTriangle("vishtriangle",vishcolor,x,y); // Generate the VAO, VBOs, vertices data & copy into the array buffer
    createRectangle("vishrectangle",vishcolor,1.5,1,0.1,0.1); //Generate sprites
    createRectangle("vishrectangle2",vishcolor,-1,0.5,0.3,0.3); 
    createRectangle("vishrectangle3",vishcolor,-1,1,0.3,0.3); 
    createRectangle("vishrectangle4",vishcolor,-1,1.5,0.3,0.3); 
    createRectangle("vishrectangle5",vishcolor,-1,2,0.3,0.3); 
    createRectangle("floor",vishcolor,0,-2,0.3,8);
    objects["floor"].fixed=1;
    createRectangle("roof",vishcolor,0,4,0.3,8);
    objects["roof"].fixed=1;
    createRectangle("wall1",vishcolor,-4,1,6,0.3);
    objects["wall1"].fixed=1;
    createRectangle("wall2",vishcolor,4,1,6,0.3);
    objects["wall2"].fixed=1;
    //createCircle("vishcircle",vishcolor,0,0,0.5,15);
	
	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");

	
	reshapeWindow (window, width, height);

    // Background color of the scene
	glClearColor (0.3f, 0.3f, 0.3f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
	int width = 700;
	int height = 700;

    GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);

    double last_update_time = glfwGetTime(), current_time;

    /* Draw in loop */
    while (!glfwWindowShouldClose(window)) {

        // OpenGL Draw commands
        draw();

        // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
        glfwPollEvents();

        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
        current_time = glfwGetTime(); // Time in seconds
        if ((current_time - last_update_time) >= 0.5) { // atleast 0.5s elapsed since last frame
            // do something every 0.5 seconds ..
            last_update_time = current_time;
        }
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
