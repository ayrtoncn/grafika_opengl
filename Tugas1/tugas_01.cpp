#include <iostream> 
#include <bits/stdc++.h> 
#include <GL/glew.h>
#include <GL/gl.h> 
#include <GL/glu.h> 
#include <GL/glut.h>

using namespace std; 

#ifndef PI
#define PI 3.1415926f
#endif

void drawCircle(GLfloat x, GLfloat y, GLfloat radius);
void RenderCar();

void drawCircle(GLfloat x, GLfloat y, GLfloat radius) {
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
}

void RenderCar() { 
	glClear(GL_COLOR_BUFFER_BIT); // clears the color buffer
	// car body
	glShadeModel(GL_SMOOTH); 
	glBegin(GL_POLYGON); 
	glColor3f(0.071f, 0.427f, 0.157f); // #126d28
	glVertex3f(-1.0, 0.0, 0.0); 
	glVertex3f(1.0, 0.0, 0.0); 
	glColor3f(0.098f, 0.596f, 0.02f);
	glVertex3f(0.9, 0.5, 0.0); 
	glVertex3f(0.6, 0.55, 0.0); 
	glVertex3f(0.3, 1.0, 0.0); 
	glVertex3f(-0.885, 1.0, 0.0); 
	glEnd(); 

	// front window
	glBegin(GL_POLYGON); 
	glColor3f(0.741f, 0.957f, 0.957f); 
	glVertex3f(0.55, 0.55, 0.1); 
	glVertex3f(-0.11, 0.55, 0.1); 
	glVertex3f(-0.11, 0.95, 0.1); 
	glVertex3f(0.28, 0.95, 0.1); 
	glEnd(); 

	// back window
	glBegin(GL_POLYGON); 
	glColor3f(0.741f, 0.957f, 0.957f); 
	glVertex3f(-0.885, 0.55, 0.1); 
	glVertex3f(-0.15, 0.55, 0.1); 
	glVertex3f(-0.15, 0.95, 0.1); 
	glVertex3f(-0.85, 0.95, 0.1); 
	glEnd(); 

	// front wheel
	drawCircle(0.6, 0.0, 0.25);

	// front wheel
	drawCircle(-0.6, 0.0, 0.25);

	glFlush(); 
} 

void RenderTriangleSmooth() { 
	glClear(GL_COLOR_BUFFER_BIT); // clears the color buffer 
	glShadeModel(GL_SMOOTH); 
	glBegin(GL_TRIANGLES); 
	glColor3f(1.0, 0.0, 0.0); 
	glVertex3f(0.0, 0.0, 0.0); 
	glColor3f(0.0, 1.0, 0.0); 
	glVertex3f(1.0, 0.0, 0.0); 
	glColor3f(0.0, 0.0, 1.0); 
	glVertex3f(0.5, 1.0, 0.0); 
	glEnd(); 
	glFlush(); 
} 

void InitializeDisplay() { 
	glClearColor(0.0, 0.0, 0.0, 1.0); // sets background to white with opacity of 1.0 
	glMatrixMode(GL_PROJECTION); 
	glLoadIdentity(); 
	glOrtho(-1.0, 1.0, -0.5, 1.2, -1.0, 1.0); 
} 

void processKeys(unsigned char key, int x, int y) {
	switch(key){
		case 27: exit(0); break; // ESC
	}
}

int main(int argc, char** argv) { 
	glutInit(&argc, argv); 
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutCreateWindow("Car #1"); 
	glutInitWindowSize(1000,1000); 
	glutInitWindowPosition(200,200); 

	InitializeDisplay(); 
	glutDisplayFunc(RenderCar);
	glutKeyboardFunc(processKeys);
	glutMainLoop(); 
	return 0; 
}