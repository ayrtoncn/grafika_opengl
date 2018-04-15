// Include GLFW
#include <GLFW/glfw3.h>
extern GLFWwindow* window; // The "extern" keyword here is to access the variable "window" declared in tutorialXXX.cpp. This is a hack to keep the tutorials simple. Please avoid this.

// Include GLM
#include <glm/glm.hpp>
#include <stdio.h>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "control4.hpp"

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;
glm::vec3 ViewPosition;

glm::mat4 getViewMatrix(){
	return ViewMatrix;
}
glm::mat4 getProjectionMatrix(){
	return ProjectionMatrix;
}

glm::vec3 getViewPosition() {
	return ViewPosition;
}


// Initial position : on +Z
glm::vec3 position = glm::vec3( 0, 0, 3 ); 
// Initial horizontal angle : toward -Z
float horizontalAngle = 3.14f;
// Initial vertical angle : none
float verticalAngle = 0.0f;
// Initial Field of View
float initialFoV = 45.0f;

float speed = 3.0f; // 3 units / second
float mouseSpeed = 0.005f;

mat4 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

void computeMatricesFromInputs(GLfloat *angle){

	// glfwGetTime is called only once, the first time this function is called
	static double lastTime = glfwGetTime();

	// Compute time difference between current and last frame
	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);

	// Get mouse position
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	// Reset mouse position for next frame
	glfwSetCursorPos(window, 1024/2, 768/2);

	// Compute new orientation
	horizontalAngle += mouseSpeed * float(1024/2 - xpos );
	verticalAngle   += mouseSpeed * float( 768/2 - ypos );

	// Direction : Spherical coordinates to Cartesian coordinates conversion
	glm::vec3 direction(
		cos(verticalAngle) * sin(horizontalAngle), 
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
	);
	
	// Right vector
	glm::vec3 right = glm::vec3(
		sin(horizontalAngle - 3.14f/2.0f), 
		0,
		cos(horizontalAngle - 3.14f/2.0f)
	);
	
	// Up vector
	glm::vec3 up = glm::cross( right, direction );
	up = vec3(0, 1, 0);

	mat4 mat;
	float rotationSpeed = 0.1;

	if (xpos > 512) {
		mat = rotationMatrix(vec3(0, 1, 0), rotationSpeed);
	} else if (xpos < 512) {
		mat = rotationMatrix(vec3(0, 1, 0), -rotationSpeed);
	}

	if (ypos > 384) {
		position.y += 0.1;
	} else if (ypos < 384) {
		position.y -= 0.1;
	}

	// Move forward
	if (glfwGetKey( window, GLFW_KEY_UP ) == GLFW_PRESS){
		position.y += 0.1;
		// *angle += 0.1;
		// mat = rotationMatrix(vec3(1, 0, 0), rotationSpeed);
		// position += direction * deltaTime * speed;
	}
	// Move backward
	if (glfwGetKey( window, GLFW_KEY_DOWN ) == GLFW_PRESS){
		position.y -= 0.1;
		// *angle -= 0.1;
		// mat = rotationMatrix(vec3(1, 0, 0), -rotationSpeed);
		// position -= direction * deltaTime * speed;
	}
	// Strafe right
	if (glfwGetKey( window, GLFW_KEY_RIGHT ) == GLFW_PRESS){
		mat = rotationMatrix(vec3(0, 1, 0), rotationSpeed);
		// position += right * deltaTime * speed;
	}
	// Strafe left
	if (glfwGetKey( window, GLFW_KEY_LEFT ) == GLFW_PRESS){
		mat = rotationMatrix(vec3(0, 1, 0), -rotationSpeed);
		// position -= right * deltaTime * speed;
	}
	vec4 something = mat * vec4(position, 0);
	position = vec3(something);
	ViewPosition = position;
	// up = vec3(position.x * sin(*angle), 1, position.z * sin(*angle));
	// up = vec3(0, cos(*angle), sin(*angle));
	// up = glm::normalize(up);
	// up = glm::cross(position, vec3(0, 0, 0));
	// printf("angle: %f, %f, %f\n", up.x, up.y, up.z);

	float FoV = initialFoV;// - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	ProjectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, 0.1f, 100.0f);
	// Camera matrix
	ViewMatrix       = glm::lookAt(
								position,           // Camera is here
								vec3(0, 0, 0), // and looks here : at the same position, plus "direction"
								vec3(0, 1, 0)                 // Head is up (set to 0,-1,0 to look upside-down)
						   );

	// For the next frame, the "last time" will be "now"
	lastTime = currentTime;
}