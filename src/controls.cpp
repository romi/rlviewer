// Include GLFW
#include <GLFW/glfw3.h>
extern GLFWwindow* window; // The "extern" keyword here is to access the variable "window" declared in tutorialXXX.cpp. This is a hack to keep the tutorials simple. Please avoid this.

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <stdio.h>

#include "controls.hpp"

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

glm::mat4 getViewMatrix()
{
	return ViewMatrix;
}

glm::mat4 getProjectionMatrix()
{
	return ProjectionMatrix;
}

// Initial position : on +Z
glm::vec3 position = glm::vec3(0, 0, 5); 
// Initial horizontal angle : toward -Z
float horizontalAngle = 3.14f;
// Initial vertical angle : none
float verticalAngle = 0.0f;
// Initial Field of View
float initialFoV = 45.0f;

void computeMatrices(float r, float lat, float lon)
{
        glm::vec3 center = glm::vec3(0, 0, 0); 

        float z = r * sin(lat);
        float xy = r * cos(lat);
        float x = xy * cos(lon);
        float y = xy * sin(lon);
        
	glm::vec3 position(x, z, -y);
        
        float z_up = r * sin(M_PI/2.0 + lat);
        float xy_up = r * cos(M_PI/2.0 + lat);
        float x_up = xy_up * cos(lon);
        float y_up = xy_up * sin(lon);
        glm::vec3 up = glm::vec3(x_up, z_up, -y_up);

        // printf("r=%f, lat=%f, lon=%f, pos=(%f, %f, %f), up=(%f,%f,%f)\n",
        //        r, lat, lon, x, z, -y, x_up, z_up, -y_up);
	
	float FoV = initialFoV;
        // FoV = - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires
        // setting up a callback for this. It's a bit too complicated
        // for this beginner's tutorial, so it's disabled instead.

	// Projection matrix : 45° Field of View, 4:3 ratio, display
	// range : 0.1 unit <-> 100 units
	ProjectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, 0.1f, 100.0f);
        
	// Camera matrix
	ViewMatrix = glm::lookAt(
                position,           // Camera is here
                center, // and looks here : at the same position, plus "direction"
                up                  // Head is up (set to 0,-1,0 to look upside-down)
                );        
}
