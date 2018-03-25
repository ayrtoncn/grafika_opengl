// Include standard headers
#include <stdio.h>
#include <stdlib.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
using namespace glm;

#include <common/shader.hpp>

// GLOBAL VARIABLES
float PI = 3.1415926f;
GLfloat CAR_LENGTH = 2.0;
GLfloat wheelRotation = 0.0;

int main( void )
{
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	int verticalSize = 300;
	int horizontalSize = 300;
	window = glfwCreateWindow( horizontalSize, verticalSize, "Car #2", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

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

	// White background
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint carProgram = LoadShaders( "CarVertexShader.vertexshader", "CarFragmentShader.fragmentshader" );
	GLuint windowProgram = LoadShaders( "WindowVertexShader.vertexshader", "WindowFragmentShader.fragmentshader" );
	GLuint wheelProgram = LoadShaders( "WheelVertexShader.vertexshader", "WheelFragmentShader.fragmentshader" );

	float x = -1.0; // car start x

	static const GLfloat car_body_vertex_buffer_data[] = { 
		x + 0.0, 0.0, 0.0,
		x + CAR_LENGTH, 0.0, 0.0,
		x + (CAR_LENGTH - 0.1), 0.5, 0.0,
		x + 0.0, 0.0, 0.0,
		x + (CAR_LENGTH - 0.1), 0.5, 0.0,
		x + (CAR_LENGTH - 0.4), 0.55, 0.0,
		x + 0.0, 0.0, 0.0,
		x + (CAR_LENGTH - 0.4), 0.55, 0.0,
		x + (CAR_LENGTH - 0.7), 1.0, 0.0,
		x + 0.0, 0.0, 0.0,
		x + (CAR_LENGTH - 0.7), 1.0, 0.0,
		x + 0.115, 1.0, 0.0
	};

	static const GLfloat front_window_vertex_buffer_data[] = { 
		x + 1.55, 0.55, 0.1,
		x + 0.89, 0.55, 0.1,
		x + 0.89, 0.95, 0.1,
		x + 1.28, 0.95, 0.1,
		x + 1.55, 0.55, 0.1,
		x + 0.89, 0.95, 0.1,
	};

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);

	do{

		// Clear the screen
		glClear( GL_COLOR_BUFFER_BIT );

		// Use our shader
		glUseProgram(carProgram);

		// Set car vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(car_body_vertex_buffer_data), car_body_vertex_buffer_data, GL_STATIC_DRAW);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// Draw the car body !
		glDrawArrays(GL_TRIANGLES, 0, 12); // 3 indices starting at 0 -> 1 triangle

		// Set windows vertices
		glUseProgram(windowProgram);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(front_window_vertex_buffer_data), front_window_vertex_buffer_data, GL_STATIC_DRAW);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// Draw the windows !
		glDrawArrays(GL_TRIANGLES, 0, 6); // 3 indices starting at 0 -> 1 triangle

		glUseProgram(wheelProgram);
		glUniform1f(glGetUniformLocation(wheelProgram, "outerRadius"), 0.2f);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glDisableVertexAttribArray(0);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	// Cleanup VBO
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteVertexArrays(1, &VertexArrayID);
	glDeleteProgram(carProgram);
	glDeleteProgram(windowProgram);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

