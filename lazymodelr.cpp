/*==================================================================================================
PROGRAMMER:				Byron Himes
COURSE:					CSC 525/625
MODIFIED BY:			Byron Himes
LAST MODIFIED DATE:		11/09/2015
DESCRIPTION:			Edit 2.5d mesh using keyboad, export to text file
CONTROLS:				**GENERAL**:
							-Right click and drag ROTATES scene
							-Left click and drag to MOVE POLYGON up/down/left/right
							-CTRL+left click and drag to MOVE forward/back/left/right
							-'h' to toggle HIGHLIGHT MODE (recommend on)
							-'n' to place NEW POLYGON in current viewing plane
							-F1 to EXPORT openGL code for model to c:\temp\object#.txt
							-'d' to DELETE current polygon
							-'s' to SWITCH the currently selected polygon

						**LINKING**:	(use common sense)
							---to LINK current vertex to a subsequent vertex on ANOTHER polygon
							-Press 'l' to begin link
							-Press 's' to cycle target polygon
							-Press space to cycle vertices on that polygon
							-Press 'l' again to complete the link
							-Press 'x' to cancel link mode!

						**COLOR EDIT MODE**:
							-'c' to toggle vertex coloring mode
							- entire poly coloring not supported currently
							- use arrow keys to adjust values and switch between RGBA


NOTES:					Going to add light toggle (maybe?)

FILES:					lazymodelr.cpp, structs.h, (*.sln, ...)
IDE/COMPILER:			MicroSoft Visual Studio 2013
INSTRUCTION FOR COMPILATION AND EXECUTION:
1.		Double click on aProject.sln	to OPEN the project
2.		Press Ctrl+F7					to COMPILE
3.		Press Ctrl+Shift+B				to BUILD (COMPILE+LINK)
4.		Press Ctrl+F5					to EXECUTE
==================================================================================================*/

#include <iostream>
#include <Windows.h>
#include <string>
#include <fstream>
#include <GL/glut.h>				// include GLUT library 
#include "structs.h"					// include important stuff for reasons

using namespace std;


// Camera information
double eye[3] = { (cos(4.9f) * 11.0), 3, (-sin(4.9f) * 11.0) };		// actual location of camera
double tilt[3] = { 0, 1, 0 };								// camera tilt (leave as is)
double lx = 0, lz = 0.0, ly = 1.0;							// line of sight variables

// rotation factor of scene (camera is static
float rF = 0.0; 

// Vector containing all of the polygons
vector<Poly> poly;

// current editing polygon and vertex
int curp = 0;
int curv = 0;
int linkp = 0;	// for linking
int linkv = 0;	// for linking

// keeps track of which color is currently being edited
int rgba = 0;

// current file save number & path info
int f_num = 0;
string loc = "C:\\TEMP\\object";
string ext = ".txt";

// toggles
bool ctrl_on = false;	// tracks ctrl key status
int view_mode = 0;	// 0: normal, 1: top down, 2: bottom up, NOT WORKING YET
bool highlighting = false;	// toggles color display and faded highlighting
bool c_edit_mode = false;	// toggles color editing mode (for current point(s))
bool link_mode = false;		// toggles link mode


// tracks mouse movement
int mouse_y = 0;
int mouse_x = 0;
int mouse_use = 0;

// Lighting information
GLfloat ambi[] = { 0.1f, 0.1f, 0.1f, 0.0f };
GLfloat diff[] = { 0.9f, 0.9f, 0.9, 1.0f };
GLfloat spec[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat posi[] = { 1.0f, 0.0f, 0.0f, 1.0f };

void init(){
	glClearColor(0.6f, 0.6f, 0.8f, 1.0f); // Set background color to bluey and opaque
	glClearDepth(1.0f);                   // Set background depth to farthest
	glEnable(GL_DEPTH_TEST);	// Enable depth testing for z-culling
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glEnable(GL_LIGHTING);		// Enable general lighting - need more info though!
	//glEnable(GL_LIGHT0);		// Activates the first set of lighting params, set up below!
	//glLightfv(GL_LIGHT0, GL_AMBIENT, ambi);
	//glLightfv(GL_LIGHT0, GL_DIFFUSE, diff);
	//glLightfv(GL_LIGHT0, GL_SPECULAR, spec);
	//glLightfv(GL_LIGHT0, GL_POSITION, posi);
	glDepthFunc(GL_LEQUAL);		// Set the type of depth-test
	glShadeModel(GL_SMOOTH);	// Enable smooth shading
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);  // Nice perspective corrections
	glPolygonMode(GL_FRONT, GL_FILL);	// set all polygons to fill mode
	glPolygonMode(GL_BACK, GL_LINE);
}

void draw3dAxis(){
	// not currently in use
	glLineWidth(2);

	glColor3f(1, 0, 0);
	glRasterPos3f(10.15, 0, 0);
	glutBitmapCharacter(GLUT_BITMAP_8_BY_13, 'X');
	glBegin(GL_LINES);
	glVertex3i(-10, 0, 0);
	glVertex3i(10, 0, 0);
	glEnd();


	glColor3f(0, 0, 1);
	glRasterPos3f(0, 0, 10.15);
	glutBitmapCharacter(GLUT_BITMAP_8_BY_13, 'Z');
	glBegin(GL_LINES);
	glVertex3i(0, 0, -10);
	glVertex3i(0, 0, 10);
	glEnd();

	glColor3f(0, 1, 0);
	glRasterPos3f(0, 10.15, 0);
	glutBitmapCharacter(GLUT_BITMAP_8_BY_13, 'Y');
	glBegin(GL_LINES);
	glVertex3i(0, -10, 0);
	glVertex3i(0, 10, 0);
	glEnd();
}

void drawText(string s){
	glColor3f(1.0, 1.0, 1.0);
		for (int i = 0; i < s.size(); i++){
			glutBitmapCharacter(GLUT_BITMAP_8_BY_13, s[i]);
		}
	glEnd();
}

void drawHUD(){
	glDisable(GL_LIGHTING);
	glClear(GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	double width = GLUT_WINDOW_WIDTH;
	double height = GLUT_WINDOW_HEIGHT;
	gluOrtho2D(-(width / 2), width / 2, -(height / 2), height / 2);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Rotation Factor information
	glColor3f(1, 1, 1);
	glRasterPos2f(-(width / 2) + 1, -(height / 2) + 2.5);
	drawText("Rotation Factor: ");
	drawText(to_string(rF));

	// Link mode info 
	if (link_mode){
		glRasterPos2f(-(width / 2) + 1, -(height / 2) + 1);
		drawText("Target Polygon and Vertex: ");
		drawText(to_string(linkp) + ", " + to_string(linkv));
	}

	// selected polygon info
	glRasterPos2f(-(width / 2) + 1, (height / 2) - 1);
	drawText("Selected Polygon: ");
	drawText(to_string(curp));

	// selected vertex info
	glRasterPos2f(-(width / 2) + 1, (height / 2) - 2.5);
	drawText("Selected Vertex: ");
	if (curv == 4)
		drawText("All");
	else
		drawText(to_string(curv));

	// Color info to follow:
	if (poly.size() > 0){
		glRasterPos2f(-(width / 2) + 1, (height / 2) - 4);
		drawText("Color Information:");

		// Make rbga text black if in color edit mode
		if (c_edit_mode)
			glColor3f(0, 0, 0);
		else
			glColor3f(1, 1, 1);

		// Red
		if (c_edit_mode && rgba == 0)
			glColor3f(1, 0, 0);
		else if (c_edit_mode)
			glColor3f(0, 0, 0);
		glRasterPos2f(-(width / 2) + 1.5, (height / 2) - 5.5);
		drawText("R: ");
		if (curv != 4)
			drawText(to_string(poly[curp].v[curv].c[0]));

		// Green
		if (c_edit_mode && rgba == 1)
			glColor3f(0, 1, 0);
		else if (c_edit_mode)
			glColor3f(0, 0, 0);
		glRasterPos2f(-(width / 2) + 1.5, (height / 2) - 7);
		drawText("G: ");
		if (curv != 4)
			drawText(to_string(poly[curp].v[curv].c[1]));

		// Blue
		if (c_edit_mode && rgba == 2)
			glColor3f(0, 0, 1);
		else if (c_edit_mode)
			glColor3f(0, 0, 0);
		glRasterPos2f(-(width / 2) + 1.5, (height / 2) - 8.5);
		drawText("B: ");
		if (curv != 4)
			drawText(to_string(poly[curp].v[curv].c[2]));

		// Alpha
		if (c_edit_mode && rgba == 3)
			glColor3f(.5, .5, .5);
		else if (c_edit_mode)
			glColor3f(0, 0, 0);
		glRasterPos2f(-(width / 2) + 1.5, (height / 2) - 10);
		drawText("A: ");
		if (curv != 4)
			drawText(to_string(poly[curp].v[curv].c[3]));
	}

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_LIGHTING);
}

void movePointNX(){ // moves the current point left by 0.1;
	if (poly.size() > 0){
		if (curv != 4){			// if only one point is selected
			poly[curp].v[curv].x -= 0.05;
		}
		else if (curv == 4){	// if all points are selected, move polygon
			for (int i = 0; i < 4; i++){
				poly[curp].v[i].x -= 0.05;
			}
		}
	}
}

void movePointPX(){
	if (poly.size() > 0){
		if (curv != 4){			// if only one point is selected
			poly[curp].v[curv].x += 0.05;
		}
		else if (curv == 4){	// if all points are selected, move polygon
			for (int i = 0; i < 4; i++){
				poly[curp].v[i].x += 0.05;
			}
		}
	}
}

void movePointNZ(){
	if (poly.size() > 0){
		if (curv != 4){			// if only one point is selected
			poly[curp].v[curv].z -= 0.05;
		}
		else if (curv == 4){	// if all points are selected, move polygon
			for (int i = 0; i < 4; i++){
				poly[curp].v[i].z -= 0.05;
			}
		}
	}
}

void movePointPZ(){
	if (poly.size() > 0){
		if (curv != 4){			// if only one point is selected
			poly[curp].v[curv].z += 0.05;
		}
		else if (curv == 4){	// if all points are selected, move polygon
			for (int i = 0; i < 4; i++){
				poly[curp].v[i].z += 0.05;
			}
		}
	}
}

void movePointNY(){
	if (poly.size() > 0){
		if (curv != 4){			// if only one point is selected
			poly[curp].v[curv].y -= 0.05;
		}
		else if (curv == 4){	// if all points are selected, move polygon
			for (int i = 0; i < 4; i++){
				poly[curp].v[i].y -= 0.05;
			}
		}
	}
}

void movePointPY(){
	if (poly.size() > 0){
		if (curv != 4){			// if only one point is selected
			poly[curp].v[curv].y += 0.05;
		}
		else if (curv == 4){	// if all points are selected, move polygon
			for (int i = 0; i < 4; i++){
				poly[curp].v[i].y += 0.05;
			}
		}
	}
}

void addPoly0(){	// add polygon from normal view
	Poly temp;
	temp.v[0].x = -0.5;
	temp.v[0].y = 0.5;
	temp.v[0].z = 0.0;

	temp.v[1].x = -0.5;
	temp.v[1].y = -0.5;
	temp.v[1].z = 0;

	temp.v[2].x = 0.5;
	temp.v[2].y = -0.5;
	temp.v[2].z = 0;

	temp.v[3].x = 0.5;
	temp.v[3].y = 0.5;
	temp.v[3].z = 0;

	for (int i = 0; i < 4; i++){	// for each vertex
		temp.v[i].c[0] = 1.0;
		temp.v[i].c[1] = 1.0;
		temp.v[i].c[2] = 1.0;
		temp.v[i].c[3] = 1.0;
	}

	poly.push_back(temp);
	curp = poly.size() - 1;
	curv = 4;
}

void addPoly1(){	// add polygon from 'left' view
	Poly temp;
	temp.v[0].x = -0.5;
	temp.v[0].y = 0.5;
	temp.v[0].z = -1.0;

	temp.v[1].x = -0.5;
	temp.v[1].y = -0.5;
	temp.v[1].z = -1.0;

	temp.v[2].x = -0.5;
	temp.v[2].y = -0.5;
	temp.v[2].z = 0;

	temp.v[3].x = -0.5;
	temp.v[3].y = 0.5;
	temp.v[3].z = 0;

	for (int i = 0; i < 4; i++){	// for each vertex
		temp.v[i].c[0] = 1.0;
		temp.v[i].c[1] = 1.0;
		temp.v[i].c[2] = 1.0;
		temp.v[i].c[3] = 1.0;
	}

	poly.push_back(temp);
	curp = poly.size() - 1;
	curv = 4;
}

void addPoly2(){	// add polygon from '180' view
	Poly temp;
	temp.v[0].x = 0.5;
	temp.v[0].y = 0.5;
	temp.v[0].z = -1.0;

	temp.v[1].x = 0.5;
	temp.v[1].y = -0.5;
	temp.v[1].z = -1.0;

	temp.v[2].x = -0.5;
	temp.v[2].y = -0.5;
	temp.v[2].z = -1.0;

	temp.v[3].x = -0.5;
	temp.v[3].y = 0.5;
	temp.v[3].z = -1.0;

	for (int i = 0; i < 4; i++){	// for each vertex
		temp.v[i].c[0] = 1.0;
		temp.v[i].c[1] = 1.0;
		temp.v[i].c[2] = 1.0;
		temp.v[i].c[3] = 1.0;
	}

	poly.push_back(temp);
	curp = poly.size() - 1;
	curv = 4;
}

void addPoly3(){	// add polygon from 'right' view
	Poly temp;
	temp.v[0].x = 0.5;
	temp.v[0].y = 0.5;
	temp.v[0].z = 0.0;

	temp.v[1].x = 0.5;
	temp.v[1].y = -0.5;
	temp.v[1].z = 0.0;

	temp.v[2].x = 0.5;
	temp.v[2].y = -0.5;
	temp.v[2].z = -1.0;

	temp.v[3].x = 0.5;
	temp.v[3].y = 0.5;
	temp.v[3].z = -1.0;

	for (int i = 0; i < 4; i++){	// for each vertex
		temp.v[i].c[0] = 1.0;
		temp.v[i].c[1] = 1.0;
		temp.v[i].c[2] = 1.0;
		temp.v[i].c[3] = 1.0;
	}

	poly.push_back(temp);
	curp = poly.size() - 1;
	curv = 4;
}

void drawScene(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	// Set up camera
	if (view_mode == 1){
		gluLookAt(
			2, 4, 0,
			0, 0, 0,
			tilt[0], tilt[1], tilt[2]
			);
	}
	else if (view_mode == 2){
		gluLookAt(
			2, -4, 0,
			0, 0, 0,
			tilt[0], tilt[1], tilt[2]
			);
	}
	else{
		gluLookAt(
			eye[0], eye[1], eye[2],
			0, 0, 0,
			tilt[0], tilt[1], tilt[2]
			);
	}


	// Rotate coordinate system and draw 3D axis
	glRotatef(rF, 0, 1, 0);
	draw3dAxis();

	// Draw all polygons in 'poly'
	for (int i = 0; i < poly.size(); i++){	// FOR EACH POLYGON IN POLY:
		if (curp == i && curv == 4){	// if all points selected on this poly:
			glColor4f(0, 1, 0, 0.6);		// highlight whole in green
		} else {
			glColor4f(1, 1, 1, 0.3);		// else leave white
		}
		glBegin(GL_POLYGON);
		for (int j = 0; j < 4; j++){	// FOR EACH EDGE OF CUR. POLYGON:
			if (highlighting){	// use highlighting if it's turned on
				if (curv == j && curp == i){	// does this if only a single vtx is selected
					glColor4f(1, 0, 0, 0.6);	// highlight selected vertex red
				}
				else if (curv != j && curv != 4) {
					glColor4f(1, 1, 1, 0.3);	// highlight nonselected polygons white
				}
			}
			else{
				glColor4f(	// use actual vertex color if highlighting is off
					poly[i].v[j].c[0],	// r
					poly[i].v[j].c[1],	// g
					poly[i].v[j].c[2],	// b
					poly[i].v[j].c[3]	// a
				);
			}
			if (link_mode && i == linkp && j == linkv){
				glColor4f(0, 0, 1, 0.6);	// highlight link-target vertex blue
			}
			glVertex3f(poly[i].v[j].x, poly[i].v[j].y, poly[i].v[j].z);
		}
		glEnd();
	}

	// Following section draws a red dot at the point(s) selected
	// Highlight current vertex of current polygon
	if (poly.size() > 0){
		glColor4f(1, 0, 0, 1);
		glPointSize(4);

		// If only a single vertex is selected...
		if (curv != 4){
			glBegin(GL_POINTS);
			glVertex3f(poly[curp].v[curv].x, poly[curp].v[curv].y, poly[curp].v[curv].z);
			glEnd();
		}
			// If all vertices of the polygon are selected at once...
		else if (curv == 4){
			glBegin(GL_POINTS);
			glVertex3f(poly[curp].v[0].x, poly[curp].v[0].y, poly[curp].v[0].z);
			glVertex3f(poly[curp].v[1].x, poly[curp].v[1].y, poly[curp].v[1].z);
			glVertex3f(poly[curp].v[2].x, poly[curp].v[2].y, poly[curp].v[2].z);
			glVertex3f(poly[curp].v[3].x, poly[curp].v[3].y, poly[curp].v[3].z);
			glEnd();
		}

		// draws a blue dot for link-target vertex
		if (link_mode){
			glColor4f(0, 0, 1, 1);
			glBegin(GL_POINTS);
			glVertex3f(poly[linkp].v[linkv].x, poly[linkp].v[linkv].y, poly[linkp].v[linkv].z);
			glEnd();
		}
	}

	// Draw the hud information
	drawHUD();

	// swap buffers
	glutSwapBuffers();
	glFlush(); // flush out the buffer contents
}

void display(){
	glClear(GL_COLOR_BUFFER_BIT);	// draw the background
	drawScene();
}

void reset(){
	
}

void specKeys(int key, int x, int y){
	if (key == GLUT_KEY_F1){
		// export the data when F1 is pressed
		ofstream file;

		// Concatenate save file name into usable version
		string path = loc + to_string(f_num) + ext;

		// Open file and write to it
		file.open(path);
		if (file.is_open()){	// verify that file is open before writing
				for (int i = 0; i < poly.size(); i++){
					file << "glBegin(GL_POLYGON);\n";
					for (int j = 0; j < 4; j++){	// print each vertex in code

						// write vertex color information (RGBA)
						file << "glColor4f(" << poly[i].v[j].c[0]
							<< ", " << poly[i].v[j].c[1]
							<< ", " << poly[i].v[j].c[2]
							<< ", " << poly[i].v[j].c[3]
							<< ");\n";

						// write vertex information (x, y, z)
						file << "glVertex3f(" << poly[i].v[j].x
							<< ", " << poly[i].v[j].y
							<< ", " << poly[i].v[j].z
							<< ");\n";
					}
					file << "glEnd();\n\n";
				}
		}
		file.close();
		f_num++;	// increment f_num so files aren't overwritten by accident
	}
	if (key == GLUT_KEY_LEFT && c_edit_mode && curv != 4){
		// left arrow during color edit mode
		poly[curp].v[curv].c[rgba] -= 0.05;
		if (poly[curp].v[curv].c[rgba] < 0)
			poly[curp].v[curv].c[rgba] = 0.0;
	}
	if (key == GLUT_KEY_RIGHT && c_edit_mode && curv != 4){
		// right arrow during color edit mode
		poly[curp].v[curv].c[rgba] += 0.05;
		if (poly[curp].v[curv].c[rgba] > 1.0)
			poly[curp].v[curv].c[rgba] = 1.0;
	}
	if (key == GLUT_KEY_UP && c_edit_mode && curv != 4){
		// up arrow during color edit mode
		rgba = (rgba - 1) % 4;
	}
	if (key == GLUT_KEY_DOWN && c_edit_mode && curv != 4){
		// down arrow during color edit mode
		rgba = (rgba + 1) % 4;
	}
	drawScene();
}

void normKeys(unsigned char key, int x, int y){ 
	if (key == 'n'){
		// code to generate new polygon
		if (rF < 45 || rF >= 315){
			addPoly0();
		}
		else if (rF < 135 && rF >= 45){
			addPoly1();
		}
		else if (rF < 225 && rF >= 135){
			addPoly2();
		}
		else if (rF < 315 && rF >= 225){
			addPoly3();
		}
	}
	if (key == ' '){
		// code to change vertex of current polygon
		if (link_mode){	// if in link mode, change TARGET vertex
			linkv = (linkv + 1) % 4; // unable to select whole poly in link mode
		} else {
			curv = (curv + 1) % 5;	 // normal vtx switching
		}
	}
	if (key == 's'){
		// code to Switch current polygon
		if (link_mode){	// if linking vertices...
			linkp = (linkp + 1) % poly.size();
			if (linkp == curp)	// target polygon can't equal cur. poly.
				linkp = (linkp + 1) % poly.size();
		} 
		else{
			curp = (curp + 1) % poly.size();
		}
	}
	if (key == 'v'){ // still gotta fix this shit
		view_mode = (view_mode + 1) % 3;
	}
	if (key == 'h' && !link_mode){ // can't tog highlight in link mode
		if (highlighting)
			highlighting = false;
		else{
			highlighting = true;
			c_edit_mode = false; // if highlight tog'd on, exit color edit mode
		}
	}
	if (key == 'c' && !link_mode){	// can't enter c_edit while linking
		if (c_edit_mode)
			c_edit_mode = false;
		else{
			c_edit_mode = true;
			highlighting = false; // highlight mode yields to color edit mode
		}
	}
	if (key == 'd' && poly.size() > 0 && !link_mode){	// don't delete in link mode
		// press 'd' to DELETE selected polygon
		poly.erase(poly.begin() + curp);
		curp -= 1;
		if (curp < 0)
			curp = 0;
	}
	if (key == 'l' && curv != 4 && poly.size() > 2){ // need at least 2 polygons for link
		if (!link_mode){	// starting a link
			link_mode = true;					// initiate link mode
			highlighting = true;				// turn on highlighting
			c_edit_mode = false;				// turn off color edit
			linkp = (curp + 1) % poly.size();	// linkp is the target polygon
			linkv = 0;							// linkv is the target vertex
		}
		else if (link_mode){	// completing a link
			link_mode = false;	// exit link mode
			if (linkp != curp){	// only link if different polygon is selected as target
				poly[curp].v[curv].x = poly[linkp].v[linkv].x;	// link x
				poly[curp].v[curv].y = poly[linkp].v[linkv].y;	// link y 
				poly[curp].v[curv].z = poly[linkp].v[linkv].z;	// link z
			}
		}
	}
	if (key == 'x' && link_mode){
		link_mode = false;	// used to cancel link mode
	}


	drawScene();	// redraw the scene
}

void mouseClick(int button, int state, int x, int y){
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN){
		mouse_use = 2;
		mouse_x = x;
	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
		mouse_use = 1;
		mouse_x = x;
	}
	if (glutGetModifiers() == GLUT_ACTIVE_CTRL){
		ctrl_on = true;
	}
	else {
		ctrl_on = false;
	}
}

void mouseDrag(int x, int y){
	// for rotating scene
	if (mouse_use == 2 && !ctrl_on){	
		if (x > mouse_x && mouse_use == 2){
			rF += 0.9f;
			if (rF >= 360){
				rF = 0;
			}
		}
		else if (x < mouse_x && mouse_use == 2 && !ctrl_on){
			rF -= 0.9f;
			if (rF < 0){
				rF = 359;
			}
		}
	}
	else if (mouse_use == 2 && ctrl_on && curp == 4 && !link_mode){
		// can only rotate entire polygons and only outside link mode
		if (rF < 45 || rF > 315){ // 'normal' view rotation
			if (x > mouse_x){
				// rotate 
			}
			else if (x < mouse_x){
				
			}
			if (y < mouse_y){
				
			}
			else if (y > mouse_y){
				
			}
		}

		if (rF < 135 && rF > 45){	// 'left' view obj rotation
			if (x > mouse_x){
				
			}
			else if (x < mouse_x){
				
			}
			if (y < mouse_y){
				
			}
			else if (y > mouse_y){
				
			}
		}

		if (rF < 225 && rF > 135){ // '180 degree view' obj rotation
			if (x > mouse_x){

			}
			else if (x < mouse_x){

			}
			if (y < mouse_y){
				
			}
			else if (y > mouse_y){

			}
		}

		if (rF < 315 && rF > 225){ // 'right' view object rotation
			if (x > mouse_x){
				
			}
			else if (x < mouse_x){

			}
			if (y < mouse_y){

			}
			else if (y > mouse_y){

			}
		}
	}

	// for moving objects
	if (mouse_use == 1){	
		if (rF < 45 || rF > 315){ // 'normal' view
			if (x > mouse_x){
				movePointPX();
			}
			else if (x < mouse_x){
				movePointNX();
			}
			if (y < mouse_y){
				if (ctrl_on){
					movePointNZ();
				}
				else{
					movePointPY();
				}
			}
			else if (y > mouse_y){
				if (ctrl_on){
					movePointPZ();
				}
				else{
					movePointNY();
				}
			}
		}

		if (rF < 135 && rF > 45){	// 'left' view
			if (x > mouse_x){
				movePointPZ();
			}
			else if (x < mouse_x){
				movePointNZ();
			}
			if (y < mouse_y){
				if (ctrl_on){
					movePointPX();
				}
				else{
					movePointPY();
				}
			}
			else if (y > mouse_y){
				if (ctrl_on){
					movePointNX();
				}
				else{
					movePointNY();
				}
			}
		}
		
		if (rF < 225 && rF > 135){ // '180 degree view'
			if (x > mouse_x){
				movePointNX();
			}
			else if (x < mouse_x){
				movePointPX();
			}
			if (y < mouse_y){
				if (ctrl_on){
					movePointPZ();
				}
				else{
					movePointPY();
				}
			}
			else if (y > mouse_y){
				if (ctrl_on){
					movePointNZ();
				}
				else{
					movePointNY();
				}
			}
		}

		if (rF < 315 && rF > 225){ // 'right' view
			if (x > mouse_x){
				movePointNZ();
			}
			else if (x < mouse_x){
				movePointPZ();
			}
			if (y < mouse_y){
				if (ctrl_on){
					movePointNX();
				}
				else{
					movePointPY();
				}
			}
			else if (y > mouse_y){
				if (ctrl_on){
					movePointPX();
				}
				else{
					movePointNY();
				}
			}
		}
	}





	mouse_x = x;
	mouse_y = y;
	drawScene();
}

void reshape(GLsizei width, GLsizei height) {  // GLsizei for non-negative integer
	// Compute aspect ratio of the new window
	if (height == 0) height = 1;                // To prevent divide by 0
	GLfloat aspect = (GLfloat)width / (GLfloat)height;

	// Set the viewport to cover the new window
	glViewport(0, 0, width, height);

	// Set the aspect ratio of the clipping volume to match the viewport
	glMatrixMode(GL_PROJECTION);  // To operate on the Projection matrix

	glLoadIdentity();             // Reset
	// Enable perspective projection with fovy, aspect, zNear and zFar
	gluPerspective(45.0f, aspect, 0.1f, 1000.0f);
}

void main(int argc, char ** argv){
	glutInit(& argc, argv);
	glutInitWindowSize(1200, 900);
	glutInitWindowPosition(100, 0);
	glutCreateWindow("-- lazy modelr --");
	glutInitDisplayMode(GLUT_DOUBLE);
	init();
	reset();

	glutDisplayFunc(display);
	glutSpecialFunc(specKeys);
	glutKeyboardFunc(normKeys);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouseClick);
	glutMotionFunc(mouseDrag);


	glutWarpPointer(600, 450);
	glutMainLoop();
}