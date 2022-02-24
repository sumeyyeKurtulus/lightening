/*********
CTIS164 - Template Source Program
----------
STUDENT : Sumeyye Kurtulus
SECTION : 02
HOMEWORK: 4
----------
PROBLEMS:
----------
ADDITIONAL FEATURES: The simulation starts with a loading page which has another loading simulation inside.
				     After the loading page, the game starts if the user presses TAB.
					 If the user cannot shoot the UFO at the first shooting, pacman restarts the simulation and shoots the UFO in any case.
*********/

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>
#include "vec.h"

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 800

#define TIMER_PERIOD  30 // Period for the timer.
#define TIMER_ON         1 // 0:disable timer, 1:enable timer

#define D2R 0.0174532

/* Global Variables for Template File */
bool up = false, down = false, right = false, left = false;
int  winWidth, winHeight; // current Window width and height

#define LOAD 0
#define START 1
#define MOTION 2
#define END 3

int stat = LOAD;
int loadCnt = 0;
int timer = 30;

typedef struct{
	float r, g, b;
}color_t;

typedef struct{
	vec_t pos;
	color_t color;
	vec_t vel;
	float angle;
	bool active;
}light_t;

typedef struct{
	vec_t pos;
	vec_t N;
}vertex_t;

typedef struct{
	vec_t pos;
	float angle;
	vec_t velocity;
	bool active;
}fire_t;

light_t light = { { -400, -400 }, { 36.0 / 255.0, 72.0 / 255.0, 155.0 / 255.0 },
{ 3, 3 }, 45, true };
fire_t fire;
int f_cnt = 0;

color_t mulColor(float k, color_t c) {
	color_t tmp = { k * c.r, k * c.g, k * c.b };
	return tmp;
}


double distanceImpact(double d){
	return (-1.0 / 400.0) * d * sin(60.0)*D2R + 1.0;
}

color_t calculateColor(light_t source, vertex_t v) {
	vec_t L = subV(source.pos, v.pos);
	vec_t uL = unitV(L);
	float factor = dotP(uL, v.N) * distanceImpact(magV(uL));
	return mulColor(factor, source.color);
}

//
// to draw circle, center at (x,y)
// radius r
//
void circle(int x, int y, int r)
{
#define PI 3.1415
	float angle;
	glBegin(GL_POLYGON);
	for (int i = 0; i < 100; i++)
	{
		angle = 2 * PI*i / 100;
		glVertex2f(x + r*cos(angle), y + r*sin(angle));
	}
	glEnd();
}

void circle_wire(int x, int y, int r)
{
#define PI 3.1415
	float angle;

	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < 100; i++)
	{
		angle = 2 * PI*i / 100;
		glVertex2f(x + r*cos(angle), y + r*sin(angle));
	}
	glEnd();
}

void print(int x, int y, const char *string, void *font)
{
	int len, i;

	glRasterPos2f(x, y);
	len = (int)strlen(string);
	for (i = 0; i<len; i++)
	{
		glutBitmapCharacter(font, string[i]);
	}
}

// display text with variables.
// vprint(-winWidth / 2 + 10, winHeight / 2 - 20, GLUT_BITMAP_8_BY_13, "ERROR: %d", numClicks);
void vprint(int x, int y, void *font, const char *string, ...)
{
	va_list ap;
	va_start(ap, string);
	char str[1024];
	vsprintf_s(str, string, ap);
	va_end(ap);

	int len, i;
	glRasterPos2f(x, y);
	len = (int)strlen(str);
	for (i = 0; i<len; i++)
	{
		glutBitmapCharacter(font, str[i]);
	}
}

// vprint2(-50, 0, 0.35, "00:%02d", timeCounter);
void vprint2(int x, int y, float size, const char *string, ...) {
	va_list ap;
	va_start(ap, string);
	char str[1024];
	vsprintf_s(str, string, ap);
	va_end(ap);
	glPushMatrix();
	glTranslatef(x, y, 0);
	glScalef(size, size, 1);

	int len, i;
	len = (int)strlen(str);
	for (i = 0; i<len; i++)
	{
		glutStrokeCharacter(GLUT_STROKE_ROMAN, str[i]);
	}
	glPopMatrix();
}

void designLoadingPage(){
	glColor3ub(124, 36, 71);
	vprint2(-110, -50, 0.5, "LOADING");
	glColor3ub(22, 79, 155);
	for (int i = 0; i < loadCnt; i++){
		glRectf(-30 + i * 40, -120, -50 + i * 40, -80);
	}

}

void designStart(){
	glColor3ub(23, 52, 91);
	vprint2(-150, 100, 0.5, "WELCOME");
	glColor3ub(91, 23, 50);
	vprint(-100, 0, GLUT_BITMAP_TIMES_ROMAN_24, "START MOTION");
	vprint(-60, -50, GLUT_BITMAP_TIMES_ROMAN_24, "OPTION");
	vprint(-40, -100, GLUT_BITMAP_TIMES_ROMAN_24, "EXIT");
	glColor3ub(23, 52, 91);
	vprint(-115, -200, GLUT_BITMAP_TIMES_ROMAN_24, "Press Spacebar to Start");

}

void designLight(light_t l){

	glColor3f(l.color.r, l.color.g, l.color.b);
	circle(l.pos.x, l.pos.y, 20);

	glColor3ub(111, 140, 196);
	glRectf(l.pos.x - 40, l.pos.y, l.pos.x + 40, l.pos.y - 20);

	glColor3f(0, 0, 0);
	glBegin(GL_LINE_LOOP);
	glVertex2f(l.pos.x - 40, l.pos.y);
	glVertex2f(l.pos.x + 40, l.pos.y);
	glVertex2f(l.pos.x + 40, l.pos.y - 20);
	glVertex2f(l.pos.x - 40, l.pos.y - 20);
	glEnd();

	glBegin(GL_LINES);
	glVertex2f(l.pos.x - 15, l.pos.y - 30);
	glVertex2f(l.pos.x - 25, l.pos.y - 40);
	glVertex2f(l.pos.x, l.pos.y - 30);
	glVertex2f(l.pos.x, l.pos.y - 40);
	glVertex2f(l.pos.x + 15, l.pos.y - 30);
	glVertex2f(l.pos.x + 25, l.pos.y - 40);
	glEnd();

	glColor3ub(101, 54, 181);
	glBegin(GL_POLYGON);
	glVertex2f(l.pos.x - 20, l.pos.y - 20);
	glVertex2f(l.pos.x + 20, l.pos.y - 20);
	glVertex2f(l.pos.x + 15, l.pos.y - 30);
	glVertex2f(l.pos.x - 15, l.pos.y - 30);
	glEnd();

	glColor3ub(139, 162, 216);
	circle(l.pos.x - 7, l.pos.y + 8, 4);
	circle(l.pos.x + 7, l.pos.y + 8, 4);

}


void Pie(float x, float y, float r, float start, float end){

	glBegin(GL_POLYGON);
	glVertex2f(x, y);
	for (float a = start; a < end; a += 10){
		glVertex2f(r * cos(a * D2R) + x, r * sin(a * D2R) + y);
	}
	glVertex2f(r * cos(end * D2R) + x, r * sin(end * D2R) + y);
	glEnd();

}


void designFire(fire_t f){
	vec_t fEnd = addV(f.pos, pol2rec({ 30, f.angle }));

	glColor3ub(129, 29, 69);
	Pie(f.pos.x, f.pos.y, 20, 30, 330);
	glColor3ub(232, 167, 193);
	circle(f.pos.x - 5, f.pos.y + 5, 5);

}


//
// To display onto window using OpenGL commands
//
void display() {
	//
	// clear window to black
	//
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);


	glColor3ub(139, 162, 216);
	glRectf(-400, -400, 400, 400);

	if (stat == LOAD){
		designLoadingPage();
	}
	else if (stat == START){
		designStart();
	}
	else if (stat == MOTION){

		if (light.active)
			designLight(light);

		if (f_cnt > 0 && fire.active)
			designFire(fire);

		for (int x = -400; x <= 400; x++){

			float angle = 45 * D2R;

			vertex_t P = { { x, x * angle - 100 }, { 0, 1 } };

			color_t res = calculateColor(light, P);

			glBegin(GL_LINES);
			glColor3f(res.r, res.g, res.b);
			glVertex2f(x, x * angle - 100);

			glColor3ub(139, 162, 216);
			glVertex2f(x, x * angle - 400);
			glEnd();

		}

	}
	else if (stat == END){

		glColor3ub(139, 162, 216);
		glRectf(-400, -400, 400, 400);

		glColor3ub(129, 29, 69);
		vprint(-100, 0, GLUT_BITMAP_TIMES_ROMAN_24, "<< GAME OVER >>");
		vprint(-100, -50, GLUT_BITMAP_TIMES_ROMAN_24, "Press ESC to EXIT");

	}
	
	glutSwapBuffers();
}

//
// key function for ASCII charachters like ESC, a,b,c..,A,B,..Z
//
void onKeyDown(unsigned char key, int x, int y)
{
	// exit when ESC is pressed.
	if (key == 27)
		exit(0);

	if (stat == START && key == ' ')
		stat = MOTION;
	
	// to refresh the window it calls display() function
	glutPostRedisplay();
}

void onKeyUp(unsigned char key, int x, int y)
{
	// exit when ESC is pressed.
	if (key == 27)
		exit(0);

	// to refresh the window it calls display() function
	glutPostRedisplay();
}

//
// Special Key like GLUT_KEY_F1, F2, F3,...
// Arrow Keys, GLUT_KEY_UP, GLUT_KEY_DOWN, GLUT_KEY_RIGHT, GLUT_KEY_RIGHT
//
void onSpecialKeyDown(int key, int x, int y)
{
	// Write your codes here.
	

	// to refresh the window it calls display() function
	glutPostRedisplay();
}

//
// Special Key like GLUT_KEY_F1, F2, F3,...
// Arrow Keys, GLUT_KEY_UP, GLUT_KEY_DOWN, GLUT_KEY_RIGHT, GLUT_KEY_RIGHT
//
void onSpecialKeyUp(int key, int x, int y)
{
	// Write your codes here.
	switch (key) {
	case GLUT_KEY_UP: up = false; break;
	case GLUT_KEY_DOWN: down = false; break;
	case GLUT_KEY_LEFT: left = false; break;
	case GLUT_KEY_RIGHT: right = false; break;
	}

	// to refresh the window it calls display() function
	glutPostRedisplay();
}

//
// When a click occurs in the window,
// It provides which button
// buttons : GLUT_LEFT_BUTTON , GLUT_RIGHT_BUTTON
// states  : GLUT_UP , GLUT_DOWN
// x, y is the coordinate of the point that mouse clicked.
//
void onClick(int button, int stat, int x, int y)
{

	fire.pos = { x - winWidth / 2, winHeight / 2 - y };
	// Write your codes here.
	if (button == GLUT_LEFT_BUTTON && stat == GLUT_DOWN){
		f_cnt++;
		fire.active = true;
		//fire.angle = 0;
	}


	// to refresh the window it calls display() function
	glutPostRedisplay();
}

//
// This function is called when the window size changes.
// w : is the new width of the window in pixels.
// h : is the new height of the window in pixels.
//
void onResize(int w, int h)
{
	winWidth = w;
	winHeight = h;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-w / 2, w / 2, -h / 2, h / 2, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	display(); // refresh window.
}

void onMoveDown(int x, int y) {
	// Write your codes here.



	// to refresh the window it calls display() function   
	glutPostRedisplay();
}

// GLUT to OpenGL coordinate conversion:
//   x2 = x1 - winWidth / 2
//   y2 = winHeight / 2 - y1
void onMove(int x, int y) {
	// Write your codes here.



	// to refresh the window it calls display() function
	glutPostRedisplay();
}

int loadTime = 0;

#if TIMER_ON == 1
void onTimer(int v) {

	glutTimerFunc(timer, onTimer, 0);
	// Write your codes here.

	if (stat == LOAD){
		timer = 300;
		loadCnt = (loadCnt + 1) % 5;
		loadTime++;
		if (loadTime == 20)
			stat = START;
	}
	
	if (stat == MOTION){
	
		timer = 30;
		light.pos = addV(light.pos, light.vel);
		if (light.pos.x > 400 || light.pos.x < -400 || light.pos.y > 400 || light.pos.y < -400){
			light.pos = { -400, -400 };
		}

		vec_t fEnd = addV(fire.pos, pol2rec({ 30, fire.angle }));

		fire.angle = light.angle;
		fire.velocity = mulV(5, unitV(subV(light.pos, fire.pos)));
		fire.pos = addV(fire.pos, fire.velocity);

		if (f_cnt > 0 && sqrt((fire.pos.x - light.pos.x) * (fire.pos.x - light.pos.x) +
			(fire.pos.y - light.pos.y) * (fire.pos.y - light.pos.y)) < 1){
			stat = END;
			light.active = false;
			fire.active = false;
		}

	}

	




	// to refresh the window it calls display() function
	glutPostRedisplay(); // display()

}
#endif

void Init() {

	// Smoothing shapes
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void main(int argc, char *argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	//glutInitWindowPosition(100, 100);
	glutCreateWindow("Sumeyye Kurtulus | Chase and Light Simulation");

	glutDisplayFunc(display);
	glutReshapeFunc(onResize);

	//
	// keyboard registration
	//
	glutKeyboardFunc(onKeyDown);
	glutSpecialFunc(onSpecialKeyDown);

	glutKeyboardUpFunc(onKeyUp);
	glutSpecialUpFunc(onSpecialKeyUp);

	//
	// mouse registration
	//
	glutMouseFunc(onClick);
	glutMotionFunc(onMoveDown);
	glutPassiveMotionFunc(onMove);

#if  TIMER_ON == 1
	// timer event
	glutTimerFunc(TIMER_PERIOD, onTimer, 0);
#endif

	Init();

	glutMainLoop();
}