// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>

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

float smokeStartPos = -1.0f;

// CPU representation of a particle
struct Particle{
	glm::vec3 pos, speed;
	unsigned char r,g,b,a; // Color
	float size, angle, weight;
	float life; // Remaining life of the particle. if <0 : dead and unused.
	float cameradistance; // *Squared* distance to the camera. if dead : -1.0f

	bool operator<(const Particle& that) const {
		// Sort in reverse order : far particles drawn first.
		return this->cameradistance > that.cameradistance;
	}
};

const int MaxParticles = 100000;
Particle ParticlesContainer[MaxParticles];
int LastUsedParticle = 0;

Particle SmokeParticlesContainer[MaxParticles];
int SmokeLastUsedParticle = 0;

struct CubeCollider {
	glm::vec3 minPos;
	glm::vec3 maxPos;
};

bool betweenLine(float position, float max, float min) {
	return (max - position) * (position - min) > 0;
}

bool checkCollision(CubeCollider o1, CubeCollider o2) {
	// checks for the x
	if (betweenLine(o1.maxPos.x, o2.maxPos.x, o2.minPos.x) ||
		betweenLine(o1.minPos.x, o2.maxPos.x, o2.minPos.x) ||
		betweenLine(o2.maxPos.x, o1.maxPos.x, o1.minPos.x) ||
		betweenLine(o2.minPos.x, o1.maxPos.x, o1.minPos.x)) {

		// checks for y
		if (betweenLine(o1.maxPos.y, o2.maxPos.y, o2.minPos.y) ||
			betweenLine(o1.minPos.y, o2.maxPos.y, o2.minPos.y) ||
			betweenLine(o2.maxPos.y, o1.maxPos.y, o1.minPos.y) ||
			betweenLine(o2.minPos.y, o1.maxPos.y, o1.minPos.y)) {


			// checks for z
			if (betweenLine(o1.maxPos.z, o2.maxPos.z, o2.minPos.z) ||
				betweenLine(o1.minPos.z, o2.maxPos.z, o2.minPos.z) ||
				betweenLine(o2.maxPos.z, o1.maxPos.z, o1.minPos.z) ||
				betweenLine(o2.minPos.z, o1.maxPos.z, o1.minPos.z)) {
				return true;
			}

		}
	}
	return false;
}

// Finds a Particle in ParticlesContainer which isn't used yet.
// (i.e. life < 0);
int FindUnusedParticle(){

	for(int i=LastUsedParticle; i<MaxParticles; i++){
		if (ParticlesContainer[i].life < 0){
			LastUsedParticle = i;
			return i;
		}
	}

	for(int i=0; i<LastUsedParticle; i++){
		if (ParticlesContainer[i].life < 0){
			LastUsedParticle = i;
			return i;
		}
	}

	return 0; // All particles are taken, override the first one
}

int SmokeFindUnusedParticle(){

	for(int i=SmokeLastUsedParticle; i<MaxParticles; i++){
		if (SmokeParticlesContainer[i].life < 0){
			SmokeLastUsedParticle = i;
			return i;
		}
	}

	for(int i=0; i<SmokeLastUsedParticle; i++){
		if (SmokeParticlesContainer[i].life < 0){
			SmokeLastUsedParticle = i;
			return i;
		}
	}

	return 0; // All particles are taken, override the first one
}

void SortParticles(){
	std::sort(&ParticlesContainer[0], &ParticlesContainer[MaxParticles]);
}

void SmokeSortParticles(){
	std::sort(&SmokeParticlesContainer[0], &SmokeParticlesContainer[MaxParticles]);
}

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
	window = glfwCreateWindow( 1024, 768, "IF3260 - Blackjack - OpenGL Car Showcase", NULL, NULL);
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
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

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

	// this one is for land
	GLuint landTexture = loadBMP_custom("land.bmp");
	
	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");

	// Our vertices. Tree consecutive floats give a 3D vertex; Three consecutive vertices give a triangle.
	// A cube has 6 faces with 2 triangles each, so this makes 6*2=12 triangles, and 12*3 vertices
	static GLfloat g_vertex_buffer_data[] = {
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
		-0.05f, 0.25f, 0.25f,
		// land
		100.0f, -0.1f, 100.0f,
		100.0f, -0.1f, -100.0f,
		-100.0f, -0.1f, -100.0f,
		-100.0f, -0.1f, 100.0f,
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
		0.0f, 1.0f,
		// land
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,
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
		34, 35, 32,
		// land
		36, 37, 38,
		38, 39, 36,
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

	GLuint particleProgramID = LoadShaders( "Particle.vs", "Particle.fs" );
	GLuint smokeParticleProgramID = LoadShaders( "SmokeParticle.vs", "SmokeParticle.fs" );

	GLuint CameraRight_worldspace_ID = glGetUniformLocation(particleProgramID, "CameraRight_worldspace");
	GLuint CameraUp_worldspace_ID = glGetUniformLocation(particleProgramID, "CameraUp_worldspace");
	GLuint ViewProjMatrixID = glGetUniformLocation(particleProgramID, "VP");
	
	GLuint particleTexture = loadDDS("particle.DDS");
	// GLuint smokeParticleTexture = loadDDS("particle.DDS");

	// rain particle
	static const GLfloat rain_vertex_buffer_data[] = {
		-0.05f, -0.4f, 0.0f,
		0.05f, -0.4f, 0.0f,
		0.25f,  0.4f, 0.0f,
		0.35f,  0.4f, 0.0f,
	};
	GLuint billboard_vertex_buffer;
	glGenBuffers(1, &billboard_vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rain_vertex_buffer_data), rain_vertex_buffer_data, GL_STATIC_DRAW);

	// The VBO containing the positions and sizes of the particles
	GLuint particles_position_buffer;
	glGenBuffers(1, &particles_position_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

	// The VBO containing the colors of the particles
	GLuint particles_color_buffer;
	glGenBuffers(1, &particles_color_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);

	static GLfloat* g_particule_position_size_data = new GLfloat[MaxParticles * 4];
	static GLubyte* g_particule_color_data         = new GLubyte[MaxParticles * 4];

	// smoke
	static const GLfloat smoke_vertex_buffer_data[] = {
		-0.4f, -0.4f, 0.0f,
		0.4f, -0.4f, 0.0f,
		-0.4f,  0.4f, 0.0f,
		0.4f,  0.4f, 0.0f,
	};
	GLuint smoke_billboard_vertex_buffer;
	glGenBuffers(1, &smoke_billboard_vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, smoke_billboard_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(smoke_vertex_buffer_data), smoke_vertex_buffer_data, GL_STATIC_DRAW);

	// The VBO containing the positions and sizes of the particles
	GLuint smoke_particles_position_buffer;
	glGenBuffers(1, &smoke_particles_position_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, smoke_particles_position_buffer);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

	// The VBO containing the colors of the particles
	GLuint smoke_particles_color_buffer;
	glGenBuffers(1, &smoke_particles_color_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, smoke_particles_color_buffer);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);

	static GLfloat* smoke_g_particule_position_size_data = new GLfloat[MaxParticles * 4];
	static GLubyte* smoke_g_particule_color_data         = new GLubyte[MaxParticles * 4];


	for(int i=0; i<MaxParticles; i++){
		ParticlesContainer[i].life = -1.0f;
		ParticlesContainer[i].cameradistance = -1.0f;
		SmokeParticlesContainer[i].life = -1.0f;
		SmokeParticlesContainer[i].cameradistance = -1.0f;
	}

	double lastTime = glfwGetTime();

	CubeCollider landCollider;
	landCollider.maxPos = vec3(1000.0f, -0.1f, 1000.0f);
	landCollider.minPos = vec3(-1000.0f, -0.1f, -1000.0f);

	do{
		// TODO: NOT YET
		float carMovement = 0.0f;
		if (glfwGetKey( window, GLFW_KEY_W ) == GLFW_PRESS){
			carMovement += 0.03f;
		}
		if (glfwGetKey( window, GLFW_KEY_S ) == GLFW_PRESS){
			carMovement -= 0.03f;
		}
		smokeStartPos += carMovement;

		for (int i = 0; i + 4 < sizeof(g_vertex_buffer_data) / sizeof(GLfloat) / 3; i++) {
			g_vertex_buffer_data[3 * i] += carMovement;
		}

		for (int i = 0; i < 36 * TRIANGLE_AMOUNT; i++) {
			wheel_data[3 * i] += carMovement;
		}

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		double currentTime = glfwGetTime();
		double delta = currentTime - lastTime;
		lastTime = currentTime;


		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs(&angle);
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1.0);

		// We will need the camera's position in order to sort the particles
		// w.r.t the camera's distance.
		// There should be a getCameraPosition() function in common/controls.cpp, 
		// but this works too.
		glm::vec3 CameraPosition(glm::inverse(ViewMatrix)[3]);

		glm::mat4 ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		// Generate 10 new particule each millisecond,
		// but limit this to 16 ms (60 fps), or if you have 1 long frame (1sec),
		// newparticles will be huge and the next frame even longer.
		int newparticles = (int)(delta*10000.0);
		if (newparticles > (int)(0.016f*10000.0))
			newparticles = (int)(0.016f*10000.0);
		
		for(int i=0; i<newparticles; i++){
			// rain particle
			int particleIndex = FindUnusedParticle();
			ParticlesContainer[particleIndex].life = 3.0f;
			ParticlesContainer[particleIndex].pos = glm::vec3((rand() % 2000 - 1000.0f) / 100.0f, 8.0f, (rand() % 2000 - 1000.0f) / 100.0f);
			ParticlesContainer[particleIndex].speed = vec3(1.0f);


			// Very bad way to generate a random color

			ParticlesContainer[particleIndex].size = 0.08f;

			// smoke particle
			int smokeParticleIndex = SmokeFindUnusedParticle();
			SmokeParticlesContainer[smokeParticleIndex].life = 4.0f;
			SmokeParticlesContainer[smokeParticleIndex].pos = vec3(smokeStartPos, 0.0f, 0.0f);
			SmokeParticlesContainer[smokeParticleIndex].speed = vec3(0.0f);

			float spread = 0.3f;
			glm::vec3 maindir = glm::vec3(-1.0f, 0.0f, 0.0f);
			// Very bad way to generate a random direction; 
			// See for instance http://stackoverflow.com/questions/5408276/python-uniform-spherical-distribution instead,
			// combined with some user-controlled parameters (main direction, spread, etc)
			glm::vec3 randomdir = glm::vec3(
				(rand()%2000 - 1000.0f)/1000.0f,
				(rand()%2000 - 1000.0f)/1000.0f,
				(rand()%2000 - 1000.0f)/1000.0f
			);
			
			SmokeParticlesContainer[smokeParticleIndex].speed = 0.2f * (maindir + randomdir * spread);

			SmokeParticlesContainer[smokeParticleIndex].size = 0.08f;
			
		}



		// Simulate all particles
		int ParticlesCount = 0;
		int SmokeParticlesCount = 0;
		for(int i=0; i<MaxParticles; i++){

			Particle& p = ParticlesContainer[i]; // shortcut

			if(p.life > 0.0f){

				// Decrease life
				p.life -= delta;
				if (p.life > 0.0f){

					// Simulate simple physics : gravity only, no collisions
					p.speed += glm::vec3(0.0f,-9.81f, 0.0f) * (float)delta * 0.5f * 0.65f;

					// check collision
					CubeCollider thisCollider;
					thisCollider.maxPos = p.pos + vec3(0.35f, 0.4f, 0.0f) + p.speed * (float)delta;
					thisCollider.minPos = p.pos + vec3(-0.05f, -0.4f, 0.0f) + p.speed * (float)delta;
					if (checkCollision(landCollider, thisCollider)) {
						p.life = -1;
						p.cameradistance = -1.0f;
						p.pos = vec3(1000.0f, 1000.0f, 1000.0f);
					} else {

						p.pos += p.speed * (float)delta;
						p.cameradistance = glm::length( p.pos - CameraPosition );
						//ParticlesContainer[i].pos += glm::vec3(0.0f,10.0f, 0.0f) * (float)delta;

						// Fill the GPU buffer
						g_particule_position_size_data[4*ParticlesCount+0] = p.pos.x;
						g_particule_position_size_data[4*ParticlesCount+1] = p.pos.y;
						g_particule_position_size_data[4*ParticlesCount+2] = p.pos.z;
													   
						g_particule_position_size_data[4*ParticlesCount+3] = p.size;
													   
						g_particule_color_data[4*ParticlesCount+0] = p.r;
						g_particule_color_data[4*ParticlesCount+1] = p.g;
						g_particule_color_data[4*ParticlesCount+2] = p.b;
						g_particule_color_data[4*ParticlesCount+3] = p.a;
					}
					// CubeCollider thisCollider;
					// thisCollider.maxPos = p.pos + vec3(0.35f, 0.4f, 0.0f);
					// thisCollider.minPos = p.pos + vec3(-0.05f, -0.4f, 0.0f);
					// if (checkCollision(landCollider, thisCollider)) {
					// 	p.life = -1;
					// 	p.cameradistance = -1.0f;
					// 	p.pos = vec3(1000.0f, 1000.0f, 1000.0f);
					// }

				}else{
					// Particles that just died will be put at the end of the buffer in SortParticles();
					p.cameradistance = -1.0f;
				}

				ParticlesCount++;

			}

			// smoke
			Particle& smokeP = SmokeParticlesContainer[i]; // shortcut
			if(smokeP.life > 0.0f){

				// Decrease life
				smokeP.life -= delta;
				if (smokeP.life > 0.0f){
					if (smokeP.life > 2.0f) {
						smokeP.speed.y = 0.1f;
					}
					smokeP.size *= 0.8;

					// Simulate simple physics : gravity only, no collisions
					// smokeP.speed += glm::vec3(0.0f,-9.81f, 0.0f) * (float)delta * 0.5f;
					smokeP.speed.x += 0.01f;
					if (smokeP.speed.x >= 0)
						smokeP.speed.x = -0.01f;
					smokeP.pos += smokeP.speed * (float)delta;
					smokeP.cameradistance = glm::length( smokeP.pos - CameraPosition );
					//ParticlesContainer[i].pos += glm::vec3(0.0f,10.0f, 0.0f) * (float)delta;

					// Fill the GPU buffer
					smoke_g_particule_position_size_data[4*SmokeParticlesCount+0] = smokeP.pos.x;
					smoke_g_particule_position_size_data[4*SmokeParticlesCount+1] = smokeP.pos.y;
					smoke_g_particule_position_size_data[4*SmokeParticlesCount+2] = smokeP.pos.z;
												   
					smoke_g_particule_position_size_data[4*SmokeParticlesCount+3] = smokeP.size;
												   
					smoke_g_particule_color_data[4*SmokeParticlesCount+0] = smokeP.r;
					smoke_g_particule_color_data[4*SmokeParticlesCount+1] = smokeP.g;
					smoke_g_particule_color_data[4*SmokeParticlesCount+2] = smokeP.b;
					smoke_g_particule_color_data[4*SmokeParticlesCount+3] = smokeP.a;

				}else{
					// Particles that just died will be put at the end of the buffer in SortParticles();
					smokeP.cameradistance = -1.0f;
				}

				SmokeParticlesCount++;

			}
		}

		SortParticles();
		SmokeSortParticles();
		// Update the buffers that OpenGL uses for rendering.
		// There are much more sophisticated means to stream data from the CPU to the GPU, 
		// but this is outside the scope of this tutorial.
		// http://www.opengl.org/wiki/Buffer_Object_Streaming


		glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
		glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
		glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLfloat) * 4, g_particule_position_size_data);

		glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
		glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
		glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLubyte) * 4, g_particule_color_data);


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);// Use our shader
		glUseProgram(particleProgramID);

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, particleTexture);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(TextureID, 0);

		// Same as the billboards tutorial
		glUniform3f(CameraRight_worldspace_ID, ViewMatrix[0][0], ViewMatrix[1][0], ViewMatrix[2][0]);
		glUniform3f(CameraUp_worldspace_ID   , ViewMatrix[0][1], ViewMatrix[1][1], ViewMatrix[2][1]);

		glUniformMatrix4fv(ViewProjMatrixID, 1, GL_FALSE, &ViewProjectionMatrix[0][0]);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
		glVertexAttribPointer(
			0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);
		
		// 2nd attribute buffer : positions of particles' centers
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
		glVertexAttribPointer(
			1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			4,                                // size : x + y + z + size => 4
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// 3rd attribute buffer : particles' colors
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
		glVertexAttribPointer(
			2,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			4,                                // size : r + g + b + a => 4
			GL_UNSIGNED_BYTE,                 // type
			GL_TRUE,                          // normalized?    *** YES, this means that the unsigned char[4] will be accessible with a vec4 (floats) in the shader ***
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// These functions are specific to glDrawArrays*Instanced*.
		// The first parameter is the attribute buffer we're talking about.
		// The second parameter is the "rate at which generic vertex attributes advance when rendering multiple instances"
		// http://www.opengl.org/sdk/docs/man/xhtml/glVertexAttribDivisor.xml
		glVertexAttribDivisor(0, 0); // particles vertices : always reuse the same 4 vertices -> 0
		glVertexAttribDivisor(1, 1); // positions : one per quad (its center)                 -> 1
		glVertexAttribDivisor(2, 1); // color : one per quad                                  -> 1

		// Draw the particules !
		// This draws many times a small triangle_strip (which looks like a quad).
		// This is equivalent to :
		// for(i in ParticlesCount) : glDrawArrays(GL_TRIANGLE_STRIP, 0, 4), 
		// but faster.
		glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, ParticlesCount);

		// smoke

		// Update the buffers that OpenGL uses for rendering.
		// There are much more sophisticated means to stream data from the CPU to the GPU, 
		// but this is outside the scope of this tutorial.
		// http://www.opengl.org/wiki/Buffer_Object_Streaming


		glBindBuffer(GL_ARRAY_BUFFER, smoke_particles_position_buffer);
		glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
		glBufferSubData(GL_ARRAY_BUFFER, 0, SmokeParticlesCount * sizeof(GLfloat) * 4, smoke_g_particule_position_size_data);

		glBindBuffer(GL_ARRAY_BUFFER, smoke_particles_color_buffer);
		glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
		glBufferSubData(GL_ARRAY_BUFFER, 0, SmokeParticlesCount * sizeof(GLubyte) * 4, smoke_g_particule_color_data);


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);// Use our shader
		glUseProgram(smokeParticleProgramID);

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, particleTexture);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(TextureID, 0);

		// Same as the billboards tutorial
		glUniform3f(CameraRight_worldspace_ID, ViewMatrix[0][0], ViewMatrix[1][0], ViewMatrix[2][0]);
		glUniform3f(CameraUp_worldspace_ID   , ViewMatrix[0][1], ViewMatrix[1][1], ViewMatrix[2][1]);

		glUniformMatrix4fv(ViewProjMatrixID, 1, GL_FALSE, &ViewProjectionMatrix[0][0]);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, smoke_billboard_vertex_buffer);
		glVertexAttribPointer(
			0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);
		
		// 2nd attribute buffer : positions of particles' centers
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, smoke_particles_position_buffer);
		glVertexAttribPointer(
			1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			4,                                // size : x + y + z + size => 4
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// 3rd attribute buffer : particles' colors
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, smoke_particles_color_buffer);
		glVertexAttribPointer(
			2,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			4,                                // size : r + g + b + a => 4
			GL_UNSIGNED_BYTE,                 // type
			GL_TRUE,                          // normalized?    *** YES, this means that the unsigned char[4] will be accessible with a vec4 (floats) in the shader ***
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		glVertexAttribDivisor(0, 0); // particles vertices : always reuse the same 4 vertices -> 0
		glVertexAttribDivisor(1, 1); // positions : one per quad (its center)                 -> 1
		glVertexAttribDivisor(2, 1); // color : one per quad                                  -> 1

		glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, SmokeParticlesCount);
		// smoke end

		// reset for car
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		glVertexAttribDivisor(0, 0);
		glVertexAttribDivisor(1, 0);
		glVertexAttribDivisor(2, 0);

		// RotateWheel(front_cross_buffer_data, 48, -0.2/*0.2*/, 0.0);
		// RotateWheel(back_cross_buffer_data, 48, -0.755/*-0.225*/, 0.0);

		// Use our shader
		glUseProgram(programID);

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniform3fv(VecID, 1, &getViewPosition()[0]);

		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

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

		// land
		glBindTexture(GL_TEXTURE_2D, landTexture); // TODO: change this to landTexture
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), indices + 60, GL_STATIC_DRAW);
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

