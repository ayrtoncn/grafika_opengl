#include <GL/glut.h>
#include <math.h>

// Global variables
float PI = 3.1415926f;
GLfloat INIT_X_BEGIN = 0.0;
GLfloat INIT_X_END = 13.0;
GLfloat INIT_Y = 0.0;
GLfloat CAR_LENGTH = 1.0;
GLfloat xRef = INIT_X_BEGIN - CAR_LENGTH;
GLfloat yRef = 0.0;
GLfloat zRefBegin = 0.25;
GLfloat zRefEnd = -0.25;
GLfloat wheelRotation = 0.0;

bool fullscreen = false;
bool mouseDown = false;
 
float xrot = 0.0f;
float yrot = 0.0f;
 
float xdiff = 0.0f;
float ydiff = 0.0f;

void RenderWheel(GLfloat x, GLfloat y, GLfloat z1, GLfloat z2, GLfloat radius) {
	int i;
	int triangleAmount = 1000; 
	GLfloat twicePi = 2.0f * PI;

	glEnable(GL_LINE_SMOOTH);
	glLineWidth(5.0);

	// glTranslatef(0.0, 0.0, 0.0001 * (z1 > 0 ? 1 : -1));
	const float tireRatio = 0.6f;
	glBegin(GL_LINES);
	for(i = 0; i <= triangleAmount; i++) {
		glColor3f(0.30f, 0.30f, 0.30f);
		glVertex3f(x + (tireRatio * radius * cos(i * twicePi / triangleAmount)), y + (tireRatio * radius * sin(i * twicePi / triangleAmount)), z1);
		glColor3f(0.20f, 0.20f, 0.20f);
		glVertex3f(x + (radius * cos(i * twicePi / triangleAmount)), y + (radius * sin(i * twicePi / triangleAmount)), z1);
	}
	glEnd();

	glBegin(GL_LINES);
	for(i = 0; i <= triangleAmount; i++) {
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f(x, y, z1);
		glVertex3f(x + (tireRatio * radius * cos(i * twicePi / triangleAmount)), y + (tireRatio * radius * sin(i * twicePi / triangleAmount)), z1);
	}
	glEnd();
	
	glBegin(GL_LINES);
	for(i = 0; i <= triangleAmount; i++) {
		glColor3f(0.30f, 0.30f, 0.30f);
		glVertex3f(x + (tireRatio * radius * cos(i * twicePi / triangleAmount)), y + (tireRatio * radius * sin(i * twicePi / triangleAmount)), z2);
		glColor3f(0.20f, 0.20f, 0.20f);
		glVertex3f(x + (radius * cos(i * twicePi / triangleAmount)), y + (radius * sin(i * twicePi / triangleAmount)), z2);
	}
	glEnd();

	glBegin(GL_LINES);
	for(i = 0; i <= triangleAmount; i++) {
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f(x, y, z2);
		glVertex3f(x + (tireRatio * radius * cos(i * twicePi / triangleAmount)), y + (tireRatio * radius * sin(i * twicePi / triangleAmount)), z2);
	}
	glEnd();
	
	glBegin(GL_LINES);
	for(i = 0; i <= triangleAmount; i++) {
		glColor3f(0.20f, 0.20f, 0.20f);
		glVertex3f(x + (radius * cos(i * twicePi / triangleAmount)), y + (radius * sin(i * twicePi / triangleAmount)), z1);
		glVertex3f(x + (radius * cos(i * twicePi / triangleAmount)), y + (radius * sin(i * twicePi / triangleAmount)), z2);
	}
	glEnd();

}

void RenderCarSide(GLfloat x, GLfloat y, GLfloat z) { 
	// car body
	glShadeModel(GL_SMOOTH);
	glBegin(GL_POLYGON);
	glColor3f(0.071f, 0.427f, 0.157f); // #126d28
	glVertex3f(x + 0.0, 0.0, z); 
	glVertex3f(x + CAR_LENGTH, 0.0, z);
	glColor3f(0.098f, 0.596f, 0.02f);
	glVertex3f(x + (CAR_LENGTH - 0.05), 0.25, z); 
	glVertex3f(x + (CAR_LENGTH - 0.2), 0.275, z); 
	glVertex3f(x + (CAR_LENGTH - 0.35), 0.5, z); 
	glVertex3f(x + 0.0575, 0.5, z); 
	glEnd(); 

	glTranslatef(0.0, 0.0, 0.0001 * (z > 0 ? 1 : -1));
	// front window
	glBegin(GL_POLYGON); 
	glColor3f(0.741f, 0.957f, 0.957f); 
	glVertex3f(x + 0.775, 0.275, z); 
	glVertex3f(x + 0.445, 0.275, z); 
	glVertex3f(x + 0.445, 0.475, z); 
	glVertex3f(x + 0.64, 0.475, z); 
	glEnd(); 

	// back window
	glBegin(GL_POLYGON); 
	glColor3f(0.741f, 0.957f, 0.957f); 
	glVertex3f(x + 0.0575, 0.275, z); 
	glVertex3f(x + 0.425, 0.275, z); 
	glVertex3f(x + 0.425, 0.475, z); 
	glVertex3f(x + 0.075, 0.475, z); 
	glEnd();
	glTranslatef(0.0, 0.0, -0.0001 * (z > 0 ? 1 : -1));
}

void RenderTheRestOfCar(GLfloat x, GLfloat y, GLfloat z_begin, GLfloat z_end) {
	// glShadeModel(GL_SMOOTH);

	// car back
	glBegin(GL_QUADS);
	glColor3f(0.071f, 0.427f, 0.157f); // #126d28
	glVertex3f(x + 0.0, 0.0, z_begin);
	glVertex3f(x + 0.0, 0.0, z_end);
	glColor3f(0.098f, 0.596f, 0.02f);
	glVertex3f(x + 0.0575, 0.5, z_end);
	glVertex3f(x + 0.0575, 0.5, z_begin);
	glEnd(); 

	// car rear window
	glBegin(GL_QUADS);
	glColor3f(0.741f, 0.957f, 0.957f);
	glVertex3f(x + 0.03, 0.27, z_begin - 0.02);
	glVertex3f(x + 0.03, 0.27, z_end + 0.02);
	glVertex3f(x + 0.0505, 0.475, z_end + 0.02);
	glVertex3f(x + 0.0505, 0.475, z_begin - 0.02);
	glEnd();

	// car top
	glBegin(GL_QUADS);
	glColor3f(0.071f, 0.427f, 0.157f); // #126d28
	glVertex3f(x + 0.0575, 0.5, z_begin);
	glVertex3f(x + 0.0575, 0.5, z_end);
	glColor3f(0.098f, 0.596f, 0.02f);
	glVertex3f(x + (CAR_LENGTH - 0.35), 0.5, z_end);
	glVertex3f(x + (CAR_LENGTH - 0.35), 0.5, z_begin);
	glEnd();

	// car front window host
	glBegin(GL_QUADS);
	glColor3f(0.071f, 0.427f, 0.157f); // #126d28
	glVertex3f(x + (CAR_LENGTH - 0.35), 0.5, z_end);
	glVertex3f(x + (CAR_LENGTH - 0.35), 0.5, z_begin);
	glColor3f(0.098f, 0.596f, 0.02f);
	glVertex3f(x + (CAR_LENGTH - 0.2), 0.275, z_begin);
	glVertex3f(x + (CAR_LENGTH - 0.2), 0.275, z_end);
	glEnd();

	// car front window
	glBegin(GL_QUADS);
	glColor3f(0.741f, 0.957f, 0.957f);
	glVertex3f(x + (CAR_LENGTH - 0.33), 0.48, z_begin - 0.02);
	glVertex3f(x + (CAR_LENGTH - 0.33), 0.48, z_end + 0.02);
	glVertex3f(x + (CAR_LENGTH - 0.2), 0.3, z_end + 0.02);
	glVertex3f(x + (CAR_LENGTH - 0.2), 0.3, z_begin - 0.02);
	glEnd();

	// car hood (kap mobil)
	glBegin(GL_QUADS);
	glColor3f(0.071f, 0.427f, 0.157f); // #126d28
	glVertex3f(x + (CAR_LENGTH - 0.2), 0.275, z_begin);
	glVertex3f(x + (CAR_LENGTH - 0.2), 0.275, z_end);
	glColor3f(0.098f, 0.596f, 0.02f);
	glVertex3f(x + (CAR_LENGTH - 0.05), 0.25, z_end);
	glVertex3f(x + (CAR_LENGTH - 0.05), 0.25, z_begin);
	glEnd();

	// car front exhaust
	glBegin(GL_QUADS);
	glColor3f(0.071f, 0.427f, 0.157f); // #126d28
	glVertex3f(x + (CAR_LENGTH - 0.05), 0.25, z_begin);
	glVertex3f(x + (CAR_LENGTH - 0.05), 0.25, z_end);
	glColor3f(0.098f, 0.596f, 0.02f);
	glVertex3f(x + CAR_LENGTH, 0.0, z_end);
	glVertex3f(x + CAR_LENGTH, 0.0, z_begin);
	glEnd();

	// car bottom
	glBegin(GL_QUADS);
	glColor3f(0.3, 0.3, 0.3); // 30% grey
	glVertex3f(x, 0.0, z_begin);
	glVertex3f(x, 0.0, z_end);
	glVertex3f(x + CAR_LENGTH, 0.0, z_end);
	glVertex3f(x + CAR_LENGTH, 0.0, z_begin);
	glEnd();
}
 
bool init() {
	glClearColor(0.93f, 0.93f, 0.93f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearDepth(1.0f);
	 
	return true;
}

void drawCar() {
	// car body
	RenderCarSide(xRef, yRef, zRefEnd);
	RenderCarSide(xRef, yRef, zRefBegin);

	RenderTheRestOfCar(xRef, yRef, zRefBegin, zRefEnd);

	// front right wheel
	RenderWheel(xRef + 0.8, yRef, zRefBegin, 0.19, 0.125);

	// front left wheel
	RenderWheel(xRef + 0.8, yRef, zRefEnd, -0.19, 0.125);

	// back right wheel
	RenderWheel(xRef + 0.2, yRef, zRefBegin, 0.19, 0.125);

	// back left wheel
	RenderWheel(xRef + 0.2, yRef, zRefEnd, -0.19, 0.125);
}
 
void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	 
	gluLookAt(
		0.0f, 0.0f, 3.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f);
	 
	glRotatef(xrot, 1.0f, 0.0f, 0.0f);
	glRotatef(yrot, 0.0f, 1.0f, 0.0f);
	 
	drawCar();
	 
	glFlush();
	glutSwapBuffers();
}
 
void resize(int w, int h) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	 
	glViewport(0, 0, w, h);
	 
	gluPerspective(45.0f, 1.0f * w / h, 1.0f, 100.0f);
	 
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}
 
void idle() {
	if (!mouseDown) {
		xrot += 0.3f;
		yrot += 0.4f;
	}
 
	glutPostRedisplay();
}

void MoveCar(float delta) {
	xRef += delta;
}
 
void keyboard(unsigned char key, int x, int y) {
	switch (key) {
		case 27 : 
			exit(1);
			break;
		case 119:
			MoveCar(0.01);
			glutPostRedisplay();
			break;
		case 115:
			MoveCar(-0.01);
			glutPostRedisplay();
			break;
	}
}
 
void specialKeyboard(int key, int x, int y) {
	if (key == GLUT_KEY_F1) {
		fullscreen = !fullscreen;
		 
		if (fullscreen)
			glutFullScreen();
		else {
			glutReshapeWindow(500, 500);
			glutPositionWindow(50, 50);
		}
	} else if (key == GLUT_KEY_UP) {
		MoveCar(0.01);
		glutPostRedisplay();
	} else if (key == GLUT_KEY_DOWN) {
		MoveCar(-0.01);
		glutPostRedisplay();
	}
}
 
void mouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		mouseDown = true;
		 
		xdiff = x - yrot;
		ydiff = -y + xrot;
	} else
		mouseDown = false;
}
 
void mouseMotion(int x, int y) {
	if (mouseDown) {
		yrot = x - xdiff;
		xrot = y + ydiff;
		 
		glutPostRedisplay();
	}
}
 
int main(int argc, char *argv[]) {
	glutInit(&argc, argv);
	 
	glutInitWindowPosition(50, 50);
	glutInitWindowSize(500, 500);
	 
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	 
	glutCreateWindow("Kijang Showcase");
	 
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(specialKeyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotion);
	glutReshapeFunc(resize);
	//glutIdleFunc(idle);
	 
	if (!init())
	return 1;
	 
	glutMainLoop();
	 
	return 0;
}