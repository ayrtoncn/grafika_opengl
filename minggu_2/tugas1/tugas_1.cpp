#include <iostream> 
#include <bits/stdc++.h> 
#include <GL/glew.h>
#include <GL/gl.h> 
#include <GL/glu.h> 
#include <GL/glut.h>

using namespace std;

// Global variables
float PI = 3.1415926f;
GLfloat INIT_X_BEGIN = 0.0;
GLfloat INIT_X_END = 13.0;
GLfloat INIT_Y = 0.0;
GLfloat CAR_LENGTH = 2.0;
GLfloat xRef = INIT_X_BEGIN - CAR_LENGTH;
GLfloat yRef = 0.0;
GLfloat wheelRotation = 0.0;

void RenderWheel(GLfloat x, GLfloat y, GLfloat radius);
void RenderCar(GLfloat x, GLfloat y, GLfloat z);
void MoveCar();

void RenderWheel(GLfloat x, GLfloat y, GLfloat radius) {
	int i;
	int triangleAmount = 1000; 
	GLfloat twicePi = 2.0f * PI;

	glEnable(GL_LINE_SMOOTH);
	glLineWidth(5.0);

	glBegin(GL_LINES);
	for(i = 0; i <= triangleAmount; i++) {
		glColor3f(0.30f, 0.30f, 0.30f);
		glVertex2f(x, y);
		glColor3f(0.20f, 0.20f, 0.20f);
		glVertex2f(x + (radius * cos(i * twicePi / triangleAmount)), y + (radius * sin(i * twicePi / triangleAmount)));
	}
	glEnd();
	// glMatrixMode(GL_MODELVIEW);
	// double* modelviewMatrix;
	// glGetDoublev (GL_MODELVIEW_MATRIX, modelviewMatrix);

	// GLfloat now_x = modelviewMatrix[12];
	// GLfloat now_y = modelviewMatrix[13];
	// GLfloat now_z = modelviewMatrix[14];
	// glTranslatef(-x, -y, 0);
	glTranslatef(x, y, 0);
	glRotatef(wheelRotation, 0, 0, 1);
	glTranslatef(-x, -y, 0);
	glBegin(GL_LINES);
	// this one for rotation
	for(i = 0; i <= triangleAmount; i++) {
		if (i % 250 == 0) {
			// glColor3f(1.0f, 1.0f, 1.0f);
			glColor3f(1.0f, 0.0f, 0.0f);
			glVertex2f(x, y);
			glVertex2f(x + ((radius/2) * cos(i * twicePi / triangleAmount)), y + ((radius/2) * sin(i * twicePi / triangleAmount)));
		}
	}
	glEnd();
	glTranslatef(x, y, 0);
	glRotatef(-wheelRotation, 0, 0, 1);
	glTranslatef(-x, -y, 0);
	// glTranslatef(x, y, 0);
}

void RenderCar(GLfloat x, GLfloat y, GLfloat z) { 
	glClear(GL_COLOR_BUFFER_BIT); // clears the color buffer
	// car body
	glShadeModel(GL_SMOOTH); 
	glBegin(GL_POLYGON); 
	glColor3f(0.071f, 0.427f, 0.157f); // #126d28
	glVertex3f(x + 0.0, 0.0, 0.0); 
	glVertex3f(x + CAR_LENGTH, 0.0, 0.0); 
	glColor3f(0.098f, 0.596f, 0.02f);
	glVertex3f(x + (CAR_LENGTH - 0.1), 0.5, 0.0); 
	glVertex3f(x + (CAR_LENGTH - 0.4), 0.55, 0.0); 
	glVertex3f(x + (CAR_LENGTH - 0.7), 1.0, 0.0); 
	glVertex3f(x + 0.115, 1.0, 0.0); 
	glEnd(); 

	// front window
	glBegin(GL_POLYGON); 
	glColor3f(0.741f, 0.957f, 0.957f); 
	glVertex3f(x + 1.55, 0.55, 0.1); 
	glVertex3f(x + 0.89, 0.55, 0.1); 
	glVertex3f(x + 0.89, 0.95, 0.1); 
	glVertex3f(x + 1.28, 0.95, 0.1); 
	glEnd(); 

	// back window
	glBegin(GL_POLYGON); 
	glColor3f(0.741f, 0.957f, 0.957f); 
	glVertex3f(x + 0.115, 0.55, 0.1); 
	glVertex3f(x + 0.85, 0.55, 0.1); 
	glVertex3f(x + 0.85, 0.95, 0.1); 
	glVertex3f(x + 0.15, 0.95, 0.1); 
	glEnd(); 
} 

void MoveCar() {
	if (xRef > INIT_X_END + CAR_LENGTH) {
		xRef = INIT_X_BEGIN - CAR_LENGTH;
	} else {
		xRef += 0.001;
	}
}

// void RotateWheel() {
// 	wheelRotation = (wheelRotation + 249) % 250;
// }

void RotateWheel() {
	wheelRotation -= 0.1;
}

void RenderAll() { 
	// car body
	RenderCar(xRef, yRef, 0.0);

	// front wheel
	RenderWheel(xRef + 1.6, yRef, 0.25);

	// back wheel
	RenderWheel(xRef + 0.4, yRef, 0.25);

	glFlush();
	RotateWheel();
	MoveCar();
	glutPostRedisplay(); // enables animation by rendering changed position of objects
} 

void InitializeDisplay() { 
	glClearColor(1.0, 1.0, 1.0, 1.0); // sets background to white with opacity of 1.0 
	glMatrixMode(GL_PROJECTION); 
	glLoadIdentity(); 
	glOrtho(INIT_X_BEGIN, INIT_X_END, -0.5, 1.2, -1.0, 1.0); 
} 

void processKeys(unsigned char key, int x, int y) {
	switch(key){
		case 27: exit(0); break; // ESC
	}
}

int main(int argc, char** argv) { 
	glutInit(&argc, argv); 
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(1300, 200); 
	glutCreateWindow("Car #1"); 

	InitializeDisplay(); 
	glutDisplayFunc(RenderAll);
	glutKeyboardFunc(processKeys);
	glutMainLoop(); 
	return 0; 
}