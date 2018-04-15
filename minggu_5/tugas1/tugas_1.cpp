// Include standard headers
#include <stdio.h>
#include <stdlib.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;
GLfloat angle = 0.0f;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/control4.hpp>

float PI = 3.1415926f;
int TRIANGLE_AMOUNT = 1000;
GLfloat wheelRotation = -0.1;

vec3 getNormal(vec3 vertex1, vec3 vertex2, vec3 vertex3) {
	vec3 edge1 = vertex2 - vertex1;
	vec3 edge2 = vertex3 - vertex1;
	vec3 normalVector = normalize(cross(edge1, edge2));
	return normalVector;
}

void RotateWheel(GLfloat* data, int size, GLfloat x, GLfloat y) {
	for (int i = 0; i < size; i++) {
		GLfloat old_x = data[3 * i] - x;
		GLfloat old_y = data[3 * i + 1] - y;
		data[3 * i] = x + old_x * cos(wheelRotation) - old_y * sin(wheelRotation);
		data[3 * i + 1] = y + old_x * sin(wheelRotation) + old_y * cos(wheelRotation);;
	}
}

void generateCircleArray(GLfloat* result, GLfloat x1, GLfloat x2, GLfloat y,
	GLfloat z1, GLfloat z2, GLfloat radius) {
	GLfloat x, z;
	GLfloat width;
	for (int time = 0; time < 4; time++) {
		if (time == 0) {
			x = x1;
			z = z1;
		} else if (time == 1) {
			x = x1;
			z = z2;
		} else if (time == 2) {
			x = x2;
			z = z1;
		} else {
			x = x2;
			z = z2;
		}
		if (z > 0) {
			width = -0.075;
		} else
			width = 0.075;
		for (int i = time * TRIANGLE_AMOUNT; i < (time + 1) * TRIANGLE_AMOUNT; i++) {
			result[27*i] = x;
			result[27*i + 1] = y;
			result[27*i + 2] = z;
			result[27*i + 3] = x + radius * cos(i * 2 * PI / TRIANGLE_AMOUNT);
			result[27*i + 4] = y + radius * sin(i * 2 * PI / TRIANGLE_AMOUNT);
			result[27*i + 5] = z;
			result[27*i + 6] = x + radius * cos((i + 1) * 2 * PI / TRIANGLE_AMOUNT);
			result[27*i + 7] = y + radius * sin((i + 1) * 2 * PI / TRIANGLE_AMOUNT);
			result[27*i + 8] = z;

			result[27*i + 9] = x;
			result[27*i + 10] = y;
			result[27*i + 11] = z + width;
			result[27*i + 12] = x + radius * cos(i * 2 * PI / TRIANGLE_AMOUNT);
			result[27*i + 13] = y + radius * sin(i * 2 * PI / TRIANGLE_AMOUNT);
			result[27*i + 14] = z + width;
			result[27*i + 15] = x + radius * cos((i + 1) * 2 * PI / TRIANGLE_AMOUNT);
			result[27*i + 16] = y + radius * sin((i + 1) * 2 * PI / TRIANGLE_AMOUNT);
			result[27*i + 17] = z + width;

			result[27*i + 18] = x;
			result[27*i + 19] = y;
			result[27*i + 20] = z;
			result[27*i + 21] = x + radius * cos(i * 2 * PI / TRIANGLE_AMOUNT);
			result[27*i + 22] = y + radius * sin(i * 2 * PI / TRIANGLE_AMOUNT);
			result[27*i + 23] = z;
			result[27*i + 24] = x + radius * cos(i * 2 * PI / TRIANGLE_AMOUNT);
			result[27*i + 25] = y + radius * sin(i * 2 * PI / TRIANGLE_AMOUNT);
			result[27*i + 26] = z + width;
		}
	}
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
	window = glfwCreateWindow( 1024, 768, "Tutorial 0 - Keyboard and Mouse", NULL, NULL);
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
    // Hide the mouse and enable unlimited mouvement
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, 1024/2, 768/2);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS); 

	// Cull triangles which normal is not towards the camera
	// glEnable(GL_CULL_FACE);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders( "TransformVertexShader.vs", "TextureFragmentShader.fs" );

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint VecID = glGetUniformLocation(programID, "ViewPosition");

	// Load the texture
	// GLuint Texture = loadDDS("uvtemplate.DDS");
	GLuint sideTexture = loadBMP_custom("side.bmp");
	GLuint backTexture = loadBMP_custom("backside.bmp");
	GLuint topTexture = loadBMP_custom("topside.bmp");
	GLuint bottomTexture = loadBMP_custom("bottomside.bmp");
	GLuint frontWindowTexture = loadBMP_custom("frontwindow.bmp");
	GLuint frontCarTexture = loadBMP_custom("frontside.bmp");
	GLuint tireTexture = loadBMP_custom("tire.bmp");
	GLuint rimTexture = loadBMP_custom("rim.bmp");
	
	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");

	// Our vertices. Tree consecutive floats give a 3D vertex; Three consecutive vertices give a triangle.
	// A cube has 6 faces with 2 triangles each, so this makes 6*2=12 triangles, and 12*3 vertices
	static const GLfloat g_vertex_buffer_data[] = {
		// right side
		-1.0f, 0.0f, 0.25f,
		0.0f, 0.0f, 0.25f,
		-0.05f, 0.25f, 0.25f,
		-0.2f, 0.275f, 0.25f,
		-0.35f, 0.5f, 0.25f,
		-0.9425f, 0.5f, 0.25f,
		// left side
		-1.0f, 0.0f, -0.25f,
		0.0f, 0.0f, -0.25f,
		-0.05f, 0.25f, -0.25f,
		-0.2f, 0.275f, -0.25f,
		-0.35f, 0.5f, -0.25f,
		-0.9425f, 0.5f, -0.25f,
		// back
		-1.0f, 0.0f, 0.25f,
		-0.9425f, 0.5f, 0.25f,
		-0.9425f, 0.5f, -0.25f,
		-1.0f, 0.0f, -0.25f,
		// top
		-0.9425f, 0.5f, 0.25f,
		-0.9425f, 0.5f, -0.25f,
		-0.35f, 0.5f, -0.25f,
		-0.35f, 0.5f, 0.25f,
		// hood
		-0.05f, 0.25f, 0.25f,
		-0.05f, 0.25f, -0.25f,
		-0.2f, 0.275f, -0.25f,
		-0.2f, 0.275f, 0.25f,
		// bottom
		-1.0f, 0.0f, 0.25f,
		0.0f, 0.0f, 0.25f,
		0.0f, 0.0f, -0.25f,
		-1.0f, 0.0f, -0.25f,
		// front window
		-0.2f, 0.275f, 0.25f,
		-0.35f, 0.5f, 0.25f,
		-0.35f, 0.5f, -0.25f,
		-0.2f, 0.275f, -0.25f,
		// front car
		0.0f, 0.0f, 0.25f,
		0.0f, 0.0f, -0.25f,
		-0.05f, 0.25f, -0.25f,
		-0.05f, 0.25f, 0.25f
	};

	// Two UV coordinatesfor each vertex. They were created with Blender.
	static const GLfloat g_uv_buffer_data[] = {
		// right side
		0.0f, 0.0f,
		1.0f, 0.0f,
		0.95f, 0.5f,
		0.8f, 0.55f,
		0.65f, 1.0f,
		0.0575f, 1.0f,
		// left side
		0.0f, 0.0f,
		1.0f, 0.0f,
		0.95f, 0.5f,
		0.8f, 0.55f,
		0.65f, 1.0f,
		0.0575f, 1.0f,
		// back
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,
		// top
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,
		// hood
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,
		// bottom
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,
		// front window
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,
		// front car
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f
	};

	static const GLuint indices[] = {
		// right side
		0, 1, 2,
		0, 2, 3,
		0, 3, 4,
		0, 4, 5,
		// left side
		11, 10, 6,
		10, 9, 6,
		9, 8, 6,
		8, 7, 6,
		// back
		12, 13, 14,
		14, 15, 12,
		// top
		16, 19, 18,
		18, 17, 16,
		// hood
		20, 21, 22,
		22, 23, 20,
		// bottom
		24, 27, 26,
		26, 25, 24,
		// front window
		28, 31, 30,
		30, 29, 28,
		// car
		32, 33, 34,
		34, 35, 32
	};
	static GLfloat back_cross_buffer_data[] = {
		// left outside
		-0.25, 0.075, 0.2503,
		-0.20, 0.075, 0.2503,
		-0.20, -0.075, 0.2503,
		-0.20, -0.075, 0.2503,
		-0.25, -0.075, 0.2503,
		-0.25, 0.075, 0.2503,
		-0.15, 0.025, 0.2503,
		-0.15, -0.025, 0.2503,
		-0.30, 0.025, 0.2503,
		-0.30, 0.025, 0.2503,
		-0.30, -0.025, 0.2503,
		-0.15, -0.025, 0.2503,
		// right outside
		-0.25, 0.075, -0.2503,
		-0.20, 0.075, -0.2503,
		-0.20, -0.075, -0.2503,
		-0.20, -0.075, -0.2503,
		-0.25, -0.075, -0.2503,
		-0.25, 0.075, -0.2503,
		-0.15, 0.025, -0.2503,
		-0.15, -0.025, -0.2503,
		-0.30, 0.025, -0.2503,
		-0.30, 0.025, -0.2503,
		-0.30, -0.025, -0.2503,
		-0.15, -0.025, -0.2503,
		// left inside
		-0.25, 0.075, 0.2503,
		-0.20, 0.075, 0.2503,
		-0.20, -0.075, 0.2503,
		-0.20, -0.075, 0.2503,
		-0.25, -0.075, 0.2503,
		-0.25, 0.075, 0.2503,
		-0.15, 0.025, 0.2503,
		-0.15, -0.025, 0.2503,
		-0.30, 0.025, 0.2503,
		-0.30, 0.025, 0.2503,
		-0.30, -0.025, 0.2503,
		-0.15, -0.025, 0.2503,
		// right inside
		-0.25, 0.075, -0.2503,
		-0.20, 0.075, -0.2503,
		-0.20, -0.075, -0.2503,
		-0.20, -0.075, -0.2503,
		-0.25, -0.075, -0.2503,
		-0.25, 0.075, -0.2503,
		-0.15, 0.025, -0.2503,
		-0.15, -0.025, -0.2503,
		-0.30, 0.025, -0.2503,
		-0.30, 0.025, -0.2503,
		-0.30, -0.025, -0.2503,
		-0.15, -0.025, -0.2503,
	};

	static GLfloat front_cross_buffer_data[] = {
		// right outside
		0.225, 0.075, 0.2503,
		0.175, 0.075, 0.2503,
		0.175, -0.075, 0.2503,
		0.175, -0.075, 0.2503,
		0.225, -0.075, 0.2503,
		0.225, 0.075, 0.2503,
		0.125, 0.025, 0.2503,
		0.125, -0.025, 0.2503,
		0.275, 0.025, 0.2503,
		0.275, 0.025, 0.2503,
		0.275, -0.025, 0.2503,
		0.125, -0.025, 0.2503,
		// left outside
		0.225, 0.075, -0.2503,
		0.175, 0.075, -0.2503,
		0.175, -0.075, -0.2503,
		0.175, -0.075, -0.2503,
		0.225, -0.075, -0.2503,
		0.225, 0.075, -0.2503,
		0.125, 0.025, -0.2503,
		0.125, -0.025, -0.2503,
		0.275, 0.025, -0.2503,
		0.275, 0.025, -0.2503,
		0.275, -0.025, -0.2503,
		0.125, -0.025, -0.2503,
		// right inside
		0.225, 0.075, 0.2503,
		0.175, 0.075, 0.2503,
		0.175, -0.075, 0.2503,
		0.175, -0.075, 0.2503,
		0.225, -0.075, 0.2503,
		0.225, 0.075, 0.2503,
		0.125, 0.025, 0.2503,
		0.125, -0.025, 0.2503,
		0.275, 0.025, 0.2503,
		0.275, 0.025, 0.2503,
		0.275, -0.025, 0.2503,
		0.125, -0.025, 0.2503,
		// left inside
		0.225, 0.075, -0.2503,
		0.175, 0.075, -0.2503,
		0.175, -0.075, -0.2503,
		0.175, -0.075, -0.2503,
		0.225, -0.075, -0.2503,
		0.225, 0.075, -0.2503,
		0.125, 0.025, -0.2503,
		0.125, -0.025, -0.2503,
		0.275, 0.025, -0.2503,
		0.275, 0.025, -0.2503,
		0.275, -0.025, -0.2503,
		0.125, -0.025, -0.2503,
	};

	for (int i = 0; i < 48; i++) {
		front_cross_buffer_data[i * 3] -= 0.4;
		back_cross_buffer_data[i * 3] -= 0.53;
		if (i >= 24 && i < 36) {
			front_cross_buffer_data[i * 3 + 2] = 0.1749;
			back_cross_buffer_data[i * 3 + 2] = 0.1749;
		} else if (i >= 36) {
			front_cross_buffer_data[i * 3 + 2] = -0.1749;
			back_cross_buffer_data[i * 3 + 2] = -0.1749;
		}
	}

	static GLfloat *wheel_data;
	wheel_data = new GLfloat[12 * 9 * TRIANGLE_AMOUNT];
	generateCircleArray(wheel_data, -0.2f, -0.75f, 0.0f, 0.2501f, -0.2501f, 0.1f);

	GLfloat g_normal_buffer_data[sizeof(g_vertex_buffer_data) / sizeof(GLfloat)];

	for (int i = 0; i < sizeof(indices) / sizeof(float) / 3; i++) {
		GLuint indice1 = indices[3 * i];
		GLuint indice2 = indices[3 * i + 1];
		GLuint indice3 = indices[3 * i + 2];
		// printf("indices %d")
		vec3 vertex1 = vec3(g_vertex_buffer_data[3 * indice1], g_vertex_buffer_data[3 * indice1 + 1],
			g_vertex_buffer_data[3 * indice1 + 2]);
		vec3 vertex2 = vec3(g_vertex_buffer_data[3 * indice2], g_vertex_buffer_data[3 * indice2 + 1],
			g_vertex_buffer_data[3 * indice2 + 2]);
		vec3 vertex3 = vec3(g_vertex_buffer_data[3 * indice3], g_vertex_buffer_data[3 * indice3 + 1],
			g_vertex_buffer_data[3 * indice3 + 2]);
		vec3 normalVector = getNormal(vertex1, vertex2, vertex3);
		g_normal_buffer_data[3 * indice1] = normalVector.x;
		g_normal_buffer_data[3 * indice2] = normalVector.x;
		g_normal_buffer_data[3 * indice3] = normalVector.x;
		g_normal_buffer_data[3 * indice1 + 1] = normalVector.y;
		g_normal_buffer_data[3 * indice2 + 1] = normalVector.y;
		g_normal_buffer_data[3 * indice3 + 1] = normalVector.y;
		g_normal_buffer_data[3 * indice1 + 2] = normalVector.z;
		g_normal_buffer_data[3 * indice2 + 2] = normalVector.z;
		g_normal_buffer_data[3 * indice3 + 2] = normalVector.z;
	}

	for (int i = 0; i < 36; i++) {
		printf("normal %d - %f, %f, %f\n", i, g_normal_buffer_data[3 * i],
			g_normal_buffer_data[3 * i + 1], g_normal_buffer_data[3 * i + 2]);
	}

	GLfloat g_wheel_normal_buffer_data[sizeof(wheel_data) / sizeof(GLfloat)];
	for (int i = 0; i < sizeof(wheel_data) / sizeof(GLfloat) / 3; i++) {
		g_wheel_normal_buffer_data[3 * i] = 0.0f;
		g_wheel_normal_buffer_data[3 * i + 1] = 0.0f;
		if (wheel_data[3 * i + 2 > 0]) {
			g_wheel_normal_buffer_data[3 * i + 2] = 1.0f;
		} else
			g_wheel_normal_buffer_data[3 * i + 2] = -1.0f;
	}

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);

	GLuint normalbuffer;
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_normal_buffer_data), g_normal_buffer_data, GL_STATIC_DRAW);

	GLuint ebobuffer;
	glGenBuffers(1, &ebobuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebobuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	do{
		RotateWheel(front_cross_buffer_data, 48, -0.2/*0.2*/, 0.0);
		RotateWheel(back_cross_buffer_data, 48, -0.755/*-0.225*/, 0.0);

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);

		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs(&angle);
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniform3fv(VecID, 1, &getViewPosition()[0]);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(
			1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			2,                                // size : U+V => 2
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// 3nd attribute buffer : Normal
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glVertexAttribPointer(
			2,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			3,                                // size : U+V => 2
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// Bind our texture normal in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		// glUniform1i(TextureID, 0);

		// Draw the triangle !

		// side
		glBindTexture(GL_TEXTURE_2D, sideTexture);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 24 * sizeof(GLuint), indices, GL_STATIC_DRAW);
		glDrawElements(GL_TRIANGLES, 24, GL_UNSIGNED_INT, 0);

		// back
		glBindTexture(GL_TEXTURE_2D, backTexture);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), indices + 24, GL_STATIC_DRAW);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		// top
		glBindTexture(GL_TEXTURE_2D, topTexture);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), indices + 30, GL_STATIC_DRAW);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		// hood
		glBindTexture(GL_TEXTURE_2D, topTexture);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), indices + 36, GL_STATIC_DRAW);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		// bottom
		glBindTexture(GL_TEXTURE_2D, bottomTexture);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), indices + 42, GL_STATIC_DRAW);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		// front window
		glBindTexture(GL_TEXTURE_2D, frontWindowTexture);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), indices + 48, GL_STATIC_DRAW);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		// front
		glBindTexture(GL_TEXTURE_2D, frontCarTexture);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), indices + 54, GL_STATIC_DRAW);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		GLuint vertexbuffer;
		glGenBuffers(1, &vertexbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		// glBufferData(GL_ARRAY_BUFFER, sizeof(front_wheel_buffer_data), front_wheel_buffer_data, GL_STATIC_DRAW);
		
		glDisableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// wheel

		glBindTexture(GL_TEXTURE_2D, tireTexture);
		glBufferData(GL_ARRAY_BUFFER, 12 * 9 * TRIANGLE_AMOUNT * sizeof(GLfloat), wheel_data , GL_STATIC_DRAW);
		glDrawArrays(GL_TRIANGLES, 0, 12 * 9 * TRIANGLE_AMOUNT);

/*
		// Back wheel cross
		glBindTexture(GL_TEXTURE_2D, rimTexture);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(back_cross_buffer_data), back_cross_buffer_data, GL_STATIC_DRAW);
		glDrawArrays(GL_TRIANGLES, 0, 48);
		
		// Front wheel cross
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(front_cross_buffer_data), front_cross_buffer_data, GL_STATIC_DRAW);
		glDrawArrays(GL_TRIANGLES, 0, 48);
*/
		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(0);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &normalbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteProgram(programID);
	glDeleteTextures(1, &TextureID);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

