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
GLfloat wheelRotation = -0.1;
int TRIANGLE_AMOUNT = 1000;

void RotateWheel(GLfloat* data, int size, GLfloat x, GLfloat y) {
	// wheelRotation += 0.1;
	for (int i = 0; i < size; i++) {
		GLfloat old_x = data[3 * i] - x;
		GLfloat old_y = data[3 * i + 1] - y;
		data[3 * i] = x + old_x * cos(wheelRotation) - old_y * sin(wheelRotation);
		data[3 * i + 1] = y + old_x * sin(wheelRotation) + old_y * cos(wheelRotation);;
	}
}

void generateCircleArray(GLfloat* result, GLfloat x, GLfloat y, GLfloat z, GLfloat radius) {
	// data = new GLfloat[9 * TRIANGLE_AMOUNT];
	// static GLfloat* result = new GLfloat[9 * TRIANGLE_AMOUNT];
	// printf("SIZE %ld\n", sizeof(data));
	for (int i = 0; i < TRIANGLE_AMOUNT; i++) {
		result[9*i] = x;
		result[9*i + 1] = y;
		result[9*i + 2] = z;
		result[9*i + 3] = x + radius * cos(i * 2 * PI / TRIANGLE_AMOUNT);
		result[9*i + 4] = y + radius * sin(i * 2 * PI / TRIANGLE_AMOUNT);
		result[9*i + 5] = z;
		result[9*i + 6] = x + radius * cos((i + 1) * 2 * PI / TRIANGLE_AMOUNT);
		result[9*i + 7] = y + radius * sin((i + 1) * 2 * PI / TRIANGLE_AMOUNT);
		result[9*i + 8] = z;
	}
	// return result;
}

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
	GLuint crossProgram = LoadShaders( "CrossVertexShader.vertexshader", "CrossFragmentShader.fragmentshader" );

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

	static const GLfloat window_vertex_buffer_data[] = { 
		x + 1.55, 0.55, 0.1,
		x + 0.89, 0.55, 0.1,
		x + 0.89, 0.95, 0.1,
		x + 1.28, 0.95, 0.1,
		x + 1.55, 0.55, 0.1,
		x + 0.89, 0.95, 0.1,

		x + 0.115, 0.55, 0.1,
		x + 0.85, 0.55, 0.1,
		x + 0.85, 0.95, 0.1,
		x + 0.115, 0.55, 0.1,
		x + 0.85, 0.95, 0.1,
		x + 0.15, 0.95, 0.1
	};

	static GLfloat back_cross_buffer_data[] = {
		-0.50, 0.15, 0.3,
		-0.40, 0.15, 0.3,
		-0.40, -0.15, 0.3,
		-0.40, -0.15, 0.3,
		-0.50, -0.15, 0.3,
		-0.50, 0.15, 0.3,
		-0.30, 0.05, 0.3,
		-0.30, -0.05, 0.3,
		-0.60, 0.05, 0.3,
		-0.60, 0.05, 0.3,
		-0.60, -0.05, 0.3,
		-0.30, -0.05, 0.3,
	};

	static GLfloat front_cross_buffer_data[] = {
		0.45, 0.15, 0.3,
		0.35, 0.15, 0.3,
		0.35, -0.15, 0.3,
		0.35, -0.15, 0.3,
		0.45, -0.15, 0.3,
		0.45, 0.15, 0.3,
		0.25, 0.05, 0.3,
		0.25, -0.05, 0.3,
		0.55, 0.05, 0.3,
		0.55, 0.05, 0.3,
		0.55, -0.05, 0.3,
		0.25, -0.05, 0.3,
	};

	static GLfloat *front_wheel_buffer_data, *back_wheel_buffer_data;
	front_wheel_buffer_data = new GLfloat[9 * TRIANGLE_AMOUNT];
	back_wheel_buffer_data = new GLfloat[9 * TRIANGLE_AMOUNT];
	generateCircleArray(front_wheel_buffer_data, 0.4, 0.0, 0.0, 0.25);
	generateCircleArray(back_wheel_buffer_data, -0.45, 0.0, 0.0, 0.25);
	// front_wheel_buffer_data = generateCircleArray(0.4, 0.0, 0.0, 0.2);
	// back_wheel_buffer_data = generateCircleArray(0.4, 0.0, 0.0, 0.2);

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);

	do{
		RotateWheel(front_cross_buffer_data, 12, 0.4, 0.0);
		RotateWheel(back_cross_buffer_data, 12, -0.45, 0.0);
		// Clear the screen
		glClear( GL_COLOR_BUFFER_BIT );

		// Use car shader
		glUseProgram(carProgram);

		// Car body
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(car_body_vertex_buffer_data), car_body_vertex_buffer_data, GL_STATIC_DRAW);
		glDrawArrays(GL_TRIANGLES, 0, 12); // 3 indices starting at 0 -> 1 triangle

		// Use window shader
		glUseProgram(windowProgram);

		// Front window
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(window_vertex_buffer_data), window_vertex_buffer_data, GL_STATIC_DRAW);
		glDrawArrays(GL_TRIANGLES, 0, 12); // 3 indices starting at 0 -> 1 triangle

		// Use wheel shader
		glUseProgram(wheelProgram);

		// Front wheel
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, 9 * TRIANGLE_AMOUNT * sizeof(GLfloat), front_wheel_buffer_data, GL_STATIC_DRAW);
		glDrawArrays(GL_TRIANGLES, 0, 9 * TRIANGLE_AMOUNT);

		// Back wheel
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, 9 * TRIANGLE_AMOUNT * sizeof(GLfloat), back_wheel_buffer_data, GL_STATIC_DRAW);
		glDrawArrays(GL_TRIANGLES, 0, 9 * TRIANGLE_AMOUNT);

		// Use cross shader
		glUseProgram(crossProgram);

		// Back wheel cross
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(back_cross_buffer_data), back_cross_buffer_data, GL_STATIC_DRAW);
		glDrawArrays(GL_TRIANGLES, 0, 12);
		
		// Front wheel cross
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(front_cross_buffer_data), front_cross_buffer_data, GL_STATIC_DRAW);
		glDrawArrays(GL_TRIANGLES, 0, 12);

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

