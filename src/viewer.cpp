// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <pthread.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "shader.hpp"
#include "texture.hpp"
#include "controls.hpp"
#include "objloader.hpp"
#include "vboindexer.hpp"
#include "viewer.h"

static void *poll_events(void*);
static int load_model(const char *path);
static void unload_model(void);

// We would expect width and height to be 1024 and 768
GLFWwindow* window;
int windowWidth = 1024;
int windowHeight = 768;
GLuint programID;
GLuint VertexArrayID;
GLuint MatrixID;
GLuint ViewMatrixID;
GLuint ModelMatrixID;
GLuint Texture;
GLuint TextureID;
GLuint Light1ID;
GLuint Light2ID;
GLuint Light3ID;
GLuint Light4ID;
GLuint LightPower1ID;
GLuint LightPower2ID;
GLuint LightPower3ID;
GLuint LightPower4ID;

bool model_loaded = false;
GLuint vertexbuffer;
//GLuint uvbuffer;
GLuint normalbuffer;
GLuint elementbuffer;

GLuint FramebufferName = 0;
GLuint quad_vertexbuffer;
GLuint renderedTexture;
GLuint depthrenderbuffer;
GLuint quad_programID;
GLuint texID;
std::vector<unsigned short> indices;
pthread_t thread;
bool running = true;
glm::vec3 lightPos1 = glm::vec3(40, 40, 40);
glm::vec3 lightPos2 = glm::vec3(-40, -40, -40);
glm::vec3 lightPos3 = glm::vec3(0, 0, 80);
glm::vec3 lightPos4 = glm::vec3(0, 0, -80);
float lightPower1 = 5000.0f;
float lightPower2 = 0.0f;
float lightPower3 = 0.0f;
float lightPower4 = 0.0f;


int viewer_init(void)
{
	// Initialise GLFW
	if( !glfwInit() ) {
		fprintf( stderr, "Failed to initialize GLFW\n" );
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        // To make MacOS happy; should not be needed:
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); 
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(1024, 768, "Romi RL Viewer",
                                  NULL, NULL);
	if (window == NULL) {
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, "
                         "they are not 3.3 compatible. "
                         "Try the 2.1 version of the tutorials.\n" );
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
        
        // But on MacOS X with a retina screen it'll be 1024*2 and
        // 768*2, so we get the actual framebuffer size:
        glfwGetFramebufferSize(window, &windowWidth, &windowHeight);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
        // Hide the mouse and enable unlimited mouvement
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
        // Set the mouse at the center of the screen
        glfwPollEvents();
        glfwSetCursorPos(window, 1024/2, 768/2);

	// Black background
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS); 

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders("StandardShadingRTT.vertexshader",
                                "SimpleFragmentShader.fragmentshader");

	// Get a handle for our "MVP" uniform
        MatrixID = glGetUniformLocation(programID, "MVP");
        ViewMatrixID = glGetUniformLocation(programID, "V");
        ModelMatrixID = glGetUniformLocation(programID, "M");

	// Load the texture
        Texture = loadDDS("uvmap.DDS");
	
	// Get a handle for our "myTextureSampler" uniform
	TextureID  = glGetUniformLocation(programID, "myTextureSampler");
        
	// Get a handle for our "LightPosition" uniform
	glUseProgram(programID);
	Light1ID = glGetUniformLocation(programID, "LightPosition1_worldspace");
	LightPower1ID = glGetUniformLocation(programID, "LightPower1");
	Light2ID = glGetUniformLocation(programID, "LightPosition2_worldspace");
	LightPower2ID = glGetUniformLocation(programID, "LightPower2");
	Light3ID = glGetUniformLocation(programID, "LightPosition3_worldspace");
	LightPower3ID = glGetUniformLocation(programID, "LightPower3");
	Light4ID = glGetUniformLocation(programID, "LightPosition4_worldspace");
	LightPower4ID = glGetUniformLocation(programID, "LightPower4");
        

	// ---------------------------------------------
	// Render to Texture - specific code begins here
	// ---------------------------------------------

	// The framebuffer, which regroups 0, 1, or more textures, and
	// 0 or 1 depth buffer.
	glGenFramebuffers(1, &FramebufferName);
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);

	// The texture we're going to render to
	glGenTextures(1, &renderedTexture);
	
	// "Bind" the newly created texture: all future texture
	// functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, renderedTexture);

	// Give an empty image to OpenGL ( the last "0" means "empty" )
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, windowWidth, windowHeight,
                     0, GL_RGB, GL_UNSIGNED_BYTE, 0);

	// Poor filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// The depth buffer
	glGenRenderbuffers(1, &depthrenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT,
                              windowWidth, windowHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                  GL_RENDERBUFFER, depthrenderbuffer);

	//// Alternative : Depth texture. Slower, but you can sample it later in your shader
	//GLuint depthTexture;
	//glGenTextures(1, &depthTexture);
	//glBindTexture(GL_TEXTURE_2D, depthTexture);
	//glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT24, 1024, 768, 0,GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Set "renderedTexture" as our colour attachement #0
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexture, 0);

	//// Depth texture alternative : 
	//glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0);


	// Set the list of draw buffers.
	GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

	// Always check that our framebuffer is ok
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return false;

	
	// The fullscreen quad's FBO
	static const GLfloat g_quad_vertex_buffer_data[] = { 
		-1.0f, -1.0f, 0.0f,
                1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
                1.0f, -1.0f, 0.0f,
                1.0f,  1.0f, 0.0f,
	};

	glGenBuffers(1, &quad_vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data),
                     g_quad_vertex_buffer_data, GL_STATIC_DRAW);

	// Create and compile our GLSL program from the shaders
	quad_programID = LoadShaders( "Passthrough.vertexshader",
                                      "Passthrough.fragmentshader" );
        texID = glGetUniformLocation(quad_programID, "renderedTexture");

        viewer_grab(NULL, 5.0f, 0.0f, 0.0f);

        
        running = true;
        int ret = pthread_create(&thread, NULL, poll_events, NULL);
        
        return 0;
}

int viewer_load(const char *path)
{
        if (model_loaded)
                unload_model();
        
        return load_model(path);
}

static void *poll_events(void* arg)
{
        // glfwMakeContextCurrent(window);
        while (running)
                glfwPollEvents();

        return NULL;
}

static void do_draw_model(float r, float lat, float lon)
{
        // Use our shader
        glUseProgram(programID);

        // Compute the MVP matrix from keyboard and mouse input
        computeMatrices(r, lat, lon);
        glm::mat4 ProjectionMatrix = getProjectionMatrix();
        glm::mat4 ViewMatrix = getViewMatrix();
        glm::mat4 ModelMatrix = glm::mat4(1.0);
        glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

        // Send our transformation to the currently bound shader, 
        // in the "MVP" uniform
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
        glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

        glUniform3f(Light1ID, lightPos1.x, lightPos1.y, lightPos1.z);
        glUniform1f(LightPower1ID, lightPower1);
        glUniform3f(Light2ID, lightPos2.x, lightPos2.y, lightPos2.z);
        glUniform1f(LightPower2ID, lightPower2);
        glUniform3f(Light3ID, lightPos3.x, lightPos3.y, lightPos3.z);
        glUniform1f(LightPower3ID, lightPower3);
        glUniform3f(Light4ID, lightPos4.x, lightPos4.y, lightPos4.z);
        glUniform1f(LightPower4ID, lightPower4);

        // Bind our texture in Texture Unit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture);
        // Set our "myTextureSampler" sampler to use Texture Unit 0
        glUniform1i(TextureID, 0);

        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(
                0,                  // attribute
                3,                  // size
                GL_FLOAT,           // type
                GL_FALSE,           // normalized?
                0,                  // stride
                (void*)0            // array buffer offset
                );

        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
        glVertexAttribPointer(
                2,                                // attribute
                3,                                // size
                GL_FLOAT,                         // type
                GL_FALSE,                         // normalized?
                0,                                // stride
                (void*)0                          // array buffer offset
                );

        // Index buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

        // Draw the triangles !
        glDrawElements(
                GL_TRIANGLES,      // mode
                indices.size(),    // count
                GL_UNSIGNED_SHORT, // type
                (void*)0           // element array buffer offset
                );

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
}

static void draw_model(float r, float lat, float lon)
{
        if (model_loaded)
                do_draw_model(r, lat, lon);
}

static void render_to_screen()
{
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Render on the whole framebuffer, complete from the lower
        // left corner to the upper right
        glViewport(0, 0, windowWidth, windowHeight);

        // Clear the screen
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use our shader
        glUseProgram(quad_programID);

        // Bind our texture in Texture Unit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, renderedTexture);
        
        // Set our "renderedTexture" sampler to use Texture Unit 0
        glUniform1i(texID, 0);

        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
        glVertexAttribPointer(
                // attribute 0. No particular reason for 0, but must
                // match the layout in the shader.
                0,    
                3,                  // size
                GL_FLOAT,           // type
                GL_FALSE,           // normalized?
                0,                  // stride
                (void*)0            // array buffer offset
		);

        // Draw the triangles !
        glDrawArrays(GL_TRIANGLES, 0, 6); // 2*3 indices starting at 0 -> 2 triangles

        glDisableVertexAttribArray(0);

        // Swap buffers
        glfwSwapBuffers(window);
}

static void clear_framebuffer()
{
        // Render to our framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
        
        // Render on the whole framebuffer, complete from the lower
        // left corner to the upper right
        glViewport(0, 0, windowWidth, windowHeight); 

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

int viewer_grab(uint8_t *pixels, float r, float lat, float lon)
{
        clear_framebuffer();

        draw_model(r, lat, lon);
        
        if (pixels)
                glReadPixels(0, 0, windowWidth, windowHeight,
                             GL_BGR, GL_UNSIGNED_BYTE, (GLvoid *) pixels);

        render_to_screen();
        
        return 0;
}

int viewer_mask(float *mask, float r, float lat, float lon)
{
        clear_framebuffer();

        draw_model(r, lat, lon);
        
        if (mask)
                glReadPixels(0, 0, windowWidth, windowHeight,
                              GL_DEPTH_COMPONENT, GL_FLOAT, (GLvoid *) mask);

        render_to_screen();
        
        return 0;
}

static int load_model(const char *path)
{
        printf("loading %s\n", path);
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec3> normals;
//        bool res = loadOBJ(path, vertices, uvs, normals);
        bool res = loadOBJ_new(path, vertices, normals);
        if (!res)
                return -1;
        
        std::vector<glm::vec3> indexed_vertices;
        std::vector<glm::vec3> indexed_normals;
    indexVBO(vertices, normals, indices, indexed_vertices, indexed_normals);

        // Load it into a VBO
        glGenBuffers(1, &vertexbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3),
                     &indexed_vertices[0], GL_STATIC_DRAW);

        glGenBuffers(1, &normalbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
        glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3),
                     &indexed_normals[0], GL_STATIC_DRAW);

        // Generate a buffer for the indices as well
        glGenBuffers(1, &elementbuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short),
                     &indices[0], GL_STATIC_DRAW);

        model_loaded = true;
        return 0;
}

static void unload_model(void)
{
        if (model_loaded) {
                // Cleanup VBO
                glDeleteBuffers(1, &vertexbuffer);
                glDeleteBuffers(1, &normalbuffer);
                glDeleteBuffers(1, &elementbuffer);
                model_loaded = false;
        }
}

int viewer_cleanup(void)
{
        unload_model();
        
        // Cleanup shader
	glDeleteProgram(programID);
	glDeleteTextures(1, &Texture);

	glDeleteFramebuffers(1, &FramebufferName);
	glDeleteTextures(1, &renderedTexture);
	glDeleteRenderbuffers(1, &depthrenderbuffer);
	glDeleteBuffers(1, &quad_vertexbuffer);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

void viewer_set_light(int index, float x, float y, float z, float power)
{
        if (index == 0) {
                lightPos1 = glm::vec3(x, y, z);
                lightPower1 = power;
        } else if (index == 1) {
                lightPos2 = glm::vec3(x, y, z);
                lightPower2 = power;
        } else if (index == 2) {
                lightPos3 = glm::vec3(x, y, z);
                lightPower3 = power;
        } else if (index == 3) {
                lightPos4 = glm::vec3(x, y, z);
                lightPower4 = power;
        }
}

