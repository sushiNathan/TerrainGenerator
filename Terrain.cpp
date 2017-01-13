//Made by Nathan Mangaoil

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <vector>
#include <cmath>
using namespace std;


#ifdef __APPLE__
#  include <OpenGL/gl.h>
#  include <OpenGL/glu.h>
#  include <GLUT/glut.h>
#else
#  include <GL/gl.h>
#  include <GL/glu.h>
#  include <GL/freeglut.h>
#endif
#include <stdio.h>
#include <stdlib.h>

int window1, window2;//Integers that represent the 2 windows

//Dimensions of the terrain
int gridx;
int gridy;

//These variables determine the distance between each vertex
//This prevents terrains with large numbers of vertices from being too large
float inc_x;
float inc_y;

//This vector stores the x,y,z values of the vertices
vector< vector< vector<float> > > terrain(300);

//Variables that represent height range and color range
//Used for converting height values to color values
float hRange = 8;
float cRange = 1;

//Camera values
float camPos[] = {-16, 11, -16};
float camLook[] = {5, 0, 5};
float angle = 0.0f;

//Position of the first light
float light_pos[] = {-22, 15, -22, 0};

//Ambient, diffuse, and specular values for the first light
float amb0[4]  = {1, 1, 1, 1}; 
float diff0[4] = {1, 1, 1, 1}; 
float spec0[4] = {1, 1, 1, 1}; 

//Position of the second light
float light_pos2[] = {22, 15, 22, 0};

//Ambient, diffuse, and specular values for the second light
float amb1[4]  = {1, 1, 1, 1}; 
float diff1[4] = {1, 1, 1, 1}; 
float spec1[4] = {1, 1, 1, 1}; 

//Values for material 1: turquoise
float m_amb[] = {0.1, 0.18725, 0.1745, 1.0};
float m_diff[] = {0.396, 0.74151, 0.69102, 1.0};
float m_spec[] = {0.297354, 0.30829, 0.306678, 1.0};
float shiny = 60;

//Values for material 2: pearl
float m_amb2[] = {0.25, 0.20725, 0.20725, 1.0};
float m_diff2[] = {1, 0.829, 0.829, 1.0};
float m_spec2[] = {0.6, 0.6, 0.6, 1.0};
float shiny2 = 60;

//Values for material 3: ruby
float m_amb3[] = {0.1745, 0.01175, 0.01175, 1.0};
float m_diff3[] = {0.61424, 0.04136, 0.04136, 1.0};
float m_spec3[] = {0.727811, 0.626959, 0.626959, 1.0};
float shiny3 = 60;

//Circle size  and displacement amount used in the circle algorithm
float terrainCircleSize = 20;
float disp = 0.5;

//Displacement amount used in the falt algorithm
float faultDisp = 0.1;

int alg = 0;//Integer that determines which terrain algorithm to use(0 = circle, 1 = fault)
int smoothON = 0;//Integer that determines which shading type to use(0 = flat, 1 = smooth)
int shapeType = 0;//Integer that determines which shape to draw with(0 = triangles, 1 = quads)
int wire = 0;//Integer that determines how to display the terrain(0 = filled shapes, 1 = wireframe, 2 = both at the same time)
int lightOn = 0;//Integer that determines if lighting is on or off (0 = off, 1 = on)
int moveOption = 1;//Integer that determines which part of the scene the user is currently able to move (1 = terrain, 2 = light1, 3 = light2)
int charPos = 0;//Integer that determines the position of the first sphere
int charDir = 1;//Integer that determines the direction of the first sphere
int charPos2 = 0;//Integer that determines the position of the second sphere
int charDir2 = 0;//Integer that determines the direction of the second sphere

//Variables that keep track of how much the terrain is rotated/tilted
float rotate = 0.0;
float tiltUp = 0.0;

//Function that adds the vertices to the vector
void setupPoints(){
	cout<<"Creating points\n";

	for(int i=0; i<gridx; i++){
		for(int j=0; j<gridy; j++){
			terrain[i].push_back(vector<float>());
			terrain[i][j].push_back(inc_x*j - gridx*inc_x/2);
			terrain[i][j].push_back(0);
			terrain[i][j].push_back(inc_y*i - gridy*inc_y/2);
		}
	}
}

//Function used to calculate the distance between 2 points
float distance (float x1, float y1, float x2, float y2){
	float result = sqrt(pow(x2-x1,2) + pow(y2-y1,2));
	return result;
}

//Circle algorithm used to create the terrain
void circAlg(){
	int add = rand() % 2;
	int rx = rand() % (gridx-1);
	int ry = rand() % (gridy-1);
	for(int i=0; i < gridx; i++){
		for(int j=0; j < gridy; j++){
			float pd = distance(rx,ry,i,j)*2/terrainCircleSize;
			if (fabs(pd)<=1.0){
				if(add==0){
					terrain[i][j][1] += disp/2 + cos(pd*3.14)*disp/2;
				}
				else{
					terrain[i][j][1] -= disp/2 + cos(pd*3.14)*disp/2;
				}
			}
		}
	}
}

//Fault algorithm used to create the terrain
void faultAlg(){
	int rx1 = rand() % (gridx-1);
	int ry1 = rand() % (gridy-1);

	int rx2 = rand() % (gridx-1);
	int ry2 = rand() % (gridy-1);

	float v1[2] = {terrain[rx2][ry2][0] - terrain[rx1][ry1][0], terrain[rx2][ry2][2] - terrain[rx1][ry1][2]};
	for(int i=0; i<gridx; i++){
		for (int j=0; j<gridy; j++){
			float vt[2] = {terrain[i][j][0] - terrain[rx1][ry1][0], terrain[i][j][2] - terrain[rx1][ry1][2]};
			if(v1[0]*vt[1] - v1[1]*vt[0] > 0){
				terrain[i][j][1] += faultDisp;
			}
			else{
				terrain[i][j][1] -= faultDisp;
			}
		}
	}
}

//Function that creates the terrain
void makeTerrain(){

	setupPoints();//Create the vertices
	srand(time(NULL));//Set the random seed
	for(int i=0; i<3*gridx; i++){
		//Use either the circle algorithm or fault algorithm depending on the value of alg
		if(alg==0){
			circAlg();
		}
		else{
			faultAlg();
		}
	}
}

//Function that draws the heightmap onto the second window
void makeHeightMap(){
	glBegin(GL_POINTS);
	for(int i = 0; i<gridx; i++){
		float newx = (((i-0.0f)*2.0f)/gridx)+(-1.0f);
		for(int j = 0; j<gridy; j++){
			float color = (((terrain[i][j][1] - (-4))*cRange)/hRange)+0;//Convert the height value to a color value
			glColor3f(color,color,color);
			
			float newy = (((j-0.0f)*2.0f)/gridy)+(-1.0f);
			glVertex2f(newx,newy);
		}
	}
	glEnd();
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{	
		//Exit the program
		case 'q':
		case 27:
			exit (0);
			break;

		//Toggle between circle and fault algorithm
		case 'a':
		case 'A':
			if(alg==0){
				alg = 1;
			}
			else{
				alg = 0;
			}
			terrain.clear();
			terrain.resize(gridx);
			makeTerrain();
			makeHeightMap();
			break;

		//Turn lighting on and off
		case 'l':
		case 'L':
			if(lightOn==0){
				glEnable(GL_LIGHTING);
				glEnable(GL_LIGHT0);
				glEnable(GL_LIGHT1);
				lightOn = 1;
			}
			else{
				glDisable(GL_LIGHT0);
				glDisable(GL_LIGHT1);
				glDisable(GL_LIGHTING);
				lightOn = 0;
			}
			break;

		//Swtich to using triangles
		case 't':
		case 'T':
			shapeType = 0;
			break;

		//Switch to using quads
		case 'y':
		case 'Y':
			shapeType = 1;
			break;

		//Turn on smooth (Gouraud) shading
		case 'z':
		case 'Z':
			if(smoothON == 0){
				smoothON = 1;
			}
			else{
				smoothON = 0;
			}
			break;

		//Reset(remake) the terrain
		case 'r':
		case 'R':
			terrain.clear();
			terrain.resize(gridx);
			makeTerrain();
			makeHeightMap();
			break;

		//Switch between filled polygons, wireframes, and both at the same time
		case 'w':
		case 'W':
			if(wire==0){
				wire = 1;
				glPolygonMode( GL_FRONT, GL_LINE);
			}
			else if(wire==1){
				wire = 2;
				glPolygonMode( GL_FRONT, GL_FILL);
			}
			else{
				wire = 0;
			}
			break;	

		//Allow the user to move the terrain using the arrow keys
		case '1':
			moveOption = 1;
			cout<<"Moving terrain\n";
			break;

		//Allow the user to move the first light source using the arrow keys
		case '2':
			moveOption = 2;
			cout<<"Moving light source 1\n";
			break;

		//Allow the user to move the second light source using the arrow keys
		case '3':
			moveOption = 3;
			cout<<"Moving light source 2\n";
			break;	
	}
	glutSetWindow(1);
	glutPostRedisplay();
	glutSetWindow(2);
	glutPostRedisplay();
	glutSetWindow(1);
}

void special(int key, int x, int y)
{
	//Arrow keys move the terrain/light source depending on the current value of moveOption
	switch(key)
	{
		case GLUT_KEY_LEFT:
			if(moveOption==1){
				rotate-=1;
			}
			else if(moveOption==2){
				light_pos[0] -= 3;
			}
			else{
				light_pos2[0] -= 3;
			}
			break;

		case GLUT_KEY_RIGHT:
			if(moveOption==1){
				rotate+=1;
			}
			else if(moveOption==2){
				light_pos[0] += 3;
			}
			else{
				light_pos2[0] += 3;
			}
			break;

		case GLUT_KEY_UP:
			if(moveOption==1){
				if(tiltUp < 50){
					tiltUp += 1;
				}
			}
			else if(moveOption==2){
				light_pos[2] += 3;
			}
			else{
				light_pos2[2] += 3;
			}
			break;

		case GLUT_KEY_DOWN:
			if(moveOption==1){
				if(tiltUp > -50){
					tiltUp -= 1;
				}
			}
			else if(moveOption==2){
				light_pos[2] -= 3;
			}
			else{
				light_pos2[2] -= 3;
			}
			break;
	}
	glutPostRedisplay();
}

void init(void)
{
	//Set the background color
	glClearColor(0, 0.1, 0.1, 0);
	glColor3f(1, 1, 1);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, 1, 1, 100);
	
	//Turn on the light sources
	glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
	glLightfv(GL_LIGHT0, GL_AMBIENT, amb0); 
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diff0); 
	glLightfv(GL_LIGHT0, GL_SPECULAR, spec0);

	glLightfv(GL_LIGHT1, GL_POSITION, light_pos2);
	glLightfv(GL_LIGHT1, GL_AMBIENT, amb1); 
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diff1); 
	glLightfv(GL_LIGHT1, GL_SPECULAR, spec1);
}

void FPS(int val){
	glutPostRedisplay();
	glutTimerFunc(17, FPS, 0);
}

void callBackInit(){
	glutTimerFunc(0, FPS, 0);
}


//Display using triangles
void displayTri(bool wirefill){
	//Use these options when drawing filled faces
	if(wirefill==false){
		glLineWidth(1);
		glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
		glLightfv(GL_LIGHT1, GL_POSITION, light_pos2);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, m_amb);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, m_diff);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, m_spec);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shiny);
	}
	//Use these options when drawing a wireframe
	else{
		glLineWidth(3);
		glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
		glLightfv(GL_LIGHT1, GL_POSITION, light_pos2);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, m_amb2);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, m_diff2);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, m_spec2);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shiny2);
	}
	for(int i=0; i<gridx-1; i++){
		for(int j=1; j<gridy; j++){

			if(wirefill==true){
				//Use LINE_LOOP for wirefram
				glBegin(GL_LINE_LOOP);
			}
			else{
				glBegin(GL_TRIANGLES);
			}
			//Convert height values to color values
			float color = (((terrain[i][j-1][1] - (-4))*cRange)/hRange)+0;
			glColor3f(color,color,color);
			if(wirefill==true){
				glColor3f(0,color,color);
			}
			glVertex3f(terrain[i][j-1][0], terrain[i][j-1][1], terrain[i][j-1][2]);
			color = (((terrain[i+1][j-1][1] - (-4))*cRange)/hRange)+0;
			glColor3f(color,color,color);
			if(wirefill==true){
				glColor3f(0,color,color);
			}
			glVertex3f(terrain[i+1][j-1][0], terrain[i+1][j-1][1], terrain[i+1][j-1][2]);

			color = (((terrain[i][j][1] - (-4))*cRange)/hRange)+0;
			glColor3f(color,color,color);
			if(wirefill==true){
				glColor3f(0,color,color);
			}
			
			//Calculate face normal
			float v1[3] = {terrain[i][j-1][0] - terrain[i][j][0], terrain[i][j-1][1] - terrain[i][j][1], terrain[i][j-1][2] - terrain[i][j][2]};
			float v2[3] = {terrain[i+1][j-1][0] - terrain[i][j][0], terrain[i+1][j-1][1] - terrain[i][j][1], terrain[i+1][j-1][2] - terrain[i][j][2]};
			float vx = v1[1]*v2[2] - v1[2]*v2[1];
			float vy = v1[2]*v2[0] - v1[0]*v2[2];
			float vz = v1[0]*v2[1] - v1[1]*v2[0];
			float length = sqrt(vx*vx + vy*vy + vz*vz);
			glNormal3f(vx/length, vy/length, vz/length);//Apply normal
			
			glVertex3f(terrain[i][j][0], terrain[i][j][1], terrain[i][j][2]);
			glEnd();

			//Same process as the previous triangle
			if(wirefill==true){
				glBegin(GL_LINE_LOOP);
			}
			else{
				glBegin(GL_TRIANGLES);
			}
			color = (((terrain[i+1][j-1][1] - (-4))*cRange)/hRange)+0;
			glColor3f(color,color,color);
			if(wirefill==true){
				glColor3f(0,color,color);
			}
			glVertex3f(terrain[i+1][j-1][0], terrain[i+1][j-1][1], terrain[i+1][j-1][2]);

			color = (((terrain[i+1][j][1] - (-4))*cRange)/hRange)+0;
			glColor3f(color,color,color);
			if(wirefill==true){
				glColor3f(0,color,color);
			}
			glVertex3f(terrain[i+1][j][0], terrain[i+1][j][1], terrain[i+1][j][2]);

			color = (((terrain[i][j][1] - (-4))*cRange)/hRange)+0;
			glColor3f(color,color,color);
			if(wirefill==true){
				glColor3f(0,color,color);
			}

			float v12[3] = {terrain[i+1][j-1][0] - terrain[i][j][0], terrain[i+1][j-1][1] - terrain[i][j][1], terrain[i+1][j-1][2] - terrain[i][j][2]};
			float v22[3] = {terrain[i+1][j][0] - terrain[i][j][0], terrain[i+1][j][1] - terrain[i][j][1], terrain[i+1][j][2] - terrain[i][j][2]};
			float vx2 = v12[1]*v22[2] - v12[2]*v22[1];
			float vy2 = v12[2]*v22[0] - v12[0]*v22[2];
			float vz2 = v12[0]*v22[1] - v12[1]*v22[0];
			float length2 = sqrt(vx2*vx2 + vy2*vy2 + vz2*vz2);
			glNormal3f(vx2/length2, vy2/length2, vz2/length2);
			
			glVertex3f(terrain[i][j][0], terrain[i][j][1], terrain[i][j][2]);
			glEnd();
		}
	}
}

//Calculates a normal by using int i and j from the for loop
//Used to calculate vertex normals for triangles
//The value of i and j determines how many face normals are required
void setNormalTri(int i, int j){
		//top left corner
		if(i==0 && j==0){
			float v1[3] = {terrain[i][j+1][0] - terrain[i][j][0], terrain[i][j+1][1] - terrain[i][j][1], terrain[i][j+1][2] - terrain[i][j][2]};
			float v2[3] = {terrain[i+1][j][0] - terrain[i][j][0], terrain[i+1][j][1] - terrain[i][j][1], terrain[i+1][j][2] - terrain[i][j][2]};
			float vx = v1[1]*v2[2] - v1[2]*v2[1];
			float vy = v1[2]*v2[0] - v1[0]*v2[2];
			float vz = v1[0]*v2[1] - v1[1]*v2[0];
			float length = sqrt(vx*vx + vy*vy + vz*vz);
			glNormal3f(vx/length, vy/length, vz/length);
		}
		//top row
		else if(i==0 && j>0 && j<gridy-1){
			float v1[3] = {terrain[i][j-1][0] - terrain[i][j][0], terrain[i][j-1][1] - terrain[i][j][1], terrain[i][j-1][2] - terrain[i][j][2]};
			float v2[3] = {terrain[i+1][j-1][0] - terrain[i][j][0], terrain[i+1][j-1][1] - terrain[i][j][1], terrain[i+1][j-1][2] - terrain[i][j][2]};
			float vx = v1[1]*v2[2] - v1[2]*v2[1];
			float vy = v1[2]*v2[0] - v1[0]*v2[2];
			float vz = v1[0]*v2[1] - v1[1]*v2[0];
			float length = sqrt(vx*vx + vy*vy + vz*vz);
			float normal1[] = {vx/length, vy/length, vz/length};

			float v1_2[3] = {terrain[i+1][j-1][0] - terrain[i][j][0], terrain[i+1][j-1][1] - terrain[i][j][1], terrain[i+1][j-1][2] - terrain[i][j][2]};
			float v2_2[3] = {terrain[i+1][j][0] - terrain[i][j][0], terrain[i+1][j][1] - terrain[i][j][1], terrain[i+1][j][2] - terrain[i][j][2]};
			float vx2 = v1_2[1]*v2_2[2] - v1_2[2]*v2_2[1];
			float vy2 = v1_2[2]*v2_2[0] - v1_2[0]*v2_2[2];
			float vz2 = v1_2[0]*v2_2[1] - v1_2[1]*v2_2[0];
			float length2 = sqrt(vx2*vx2 + vy2*vy2 + vz2*vz2);
			float normal2[] = {vx2/length2, vy2/length2, vz2/length2};

			float v1_3[3] = {terrain[i+1][j][0] - terrain[i][j][0], terrain[i+1][j][1] - terrain[i][j][1], terrain[i+1][j][2] - terrain[i][j][2]};
			float v2_3[3] = {terrain[i][j+1][0] - terrain[i][j][0], terrain[i][j+1][1] - terrain[i][j][1], terrain[i][j+1][2] - terrain[i][j][2]};
			float vx3 = v1_3[1]*v2_3[2] - v1_3[2]*v2_3[1];
			float vy3 = v1_3[2]*v2_3[0] - v1_3[0]*v2_3[2];
			float vz3 = v1_3[0]*v2_3[1] - v1_3[1]*v2_3[0];
			float length3 = sqrt(vx3*vx3 + vy3*vy3 + vz3*vz3);
			float normal3[] = {vx3/length3, vy3/length3, vz3/length3};

			float newx = (normal1[0] + normal2[0] + normal3[0])/3;
			float newy = (normal1[1] + normal2[1] + normal3[1])/3;
			float newz = (normal1[2] + normal2[2] + normal3[2])/3;
			float newlength = sqrt(newx*newx + newy*newy + newz*newz);

			glNormal3f(fabs(newx),fabs(newy),fabs(newz));
		}
		//top right corner
		else if(i==0 && j==gridy-1){
			float v1[3] = {terrain[i][j-1][0] - terrain[i][j][0], terrain[i][j-1][1] - terrain[i][j][1], terrain[i][j-1][2] - terrain[i][j][2]};
			float v2[3] = {terrain[i+1][j-1][0] - terrain[i][j][0], terrain[i+1][j-1][1] - terrain[i][j][1], terrain[i+1][j-1][2] - terrain[i][j][2]};
			float vx = v1[1]*v2[2] - v1[2]*v2[1];
			float vy = v1[2]*v2[0] - v1[0]*v2[2];
			float vz = v1[0]*v2[1] - v1[1]*v2[0];
			float length = sqrt(vx*vx + vy*vy + vz*vz);
			float normal1[] = {vx/length, vy/length, vz/length};

			float v1_2[3] = {terrain[i+1][j-1][0] - terrain[i][j][0], terrain[i+1][j-1][1] - terrain[i][j][1], terrain[i+1][j-1][2] - terrain[i][j][2]};
			float v2_2[3] = {terrain[i+1][j][0] - terrain[i][j][0], terrain[i+1][j][1] - terrain[i][j][1], terrain[i+1][j][2] - terrain[i][j][2]};
			float vx2 = v1_2[1]*v2_2[2] - v1_2[2]*v2_2[1];
			float vy2 = v1_2[2]*v2_2[0] - v1_2[0]*v2_2[2];
			float vz2 = v1_2[0]*v2_2[1] - v1_2[1]*v2_2[0];
			float length2 = sqrt(vx2*vx2 + vy2*vy2 + vz2*vz2);
			float normal2[] = {vx2/length2, vy2/length2, vz2/length2};

			float newx = (normal1[0] + normal2[0])/2;
			float newy = (normal1[1] + normal2[1])/2;
			float newz = (normal1[2] + normal2[2])/2;
			float newlength = sqrt(newx*newx + newy*newy + newz*newz);

			glNormal3f(fabs(newx),fabs(newy),fabs(newz));	
		}
		//left-most column
		else if(i>0 && i<gridx-1 && j==0){
			float v1[3] = {terrain[i-1][j][0] - terrain[i][j][0], terrain[i-1][j][1] - terrain[i][j][1], terrain[i-1][j][2] - terrain[i][j][2]};
			float v2[3] = {terrain[i-1][j+1][0] - terrain[i][j][0], terrain[i-1][j+1][1] - terrain[i][j][1], terrain[i-1][j+1][2] - terrain[i][j][2]};
			float vx = v1[1]*v2[2] - v1[2]*v2[1];
			float vy = v1[2]*v2[0] - v1[0]*v2[2];
			float vz = v1[0]*v2[1] - v1[1]*v2[0];
			float length = sqrt(vx*vx + vy*vy + vz*vz);
			float normal1[] = {vx/length, vy/length, vz/length};

			float v1_2[3] = {terrain[i-1][j+1][0] - terrain[i][j][0], terrain[i-1][j+1][1] - terrain[i][j][1], terrain[i-1][j+1][2] - terrain[i][j][2]};
			float v2_2[3] = {terrain[i][j+1][0] - terrain[i][j][0], terrain[i][j+1][1] - terrain[i][j][1], terrain[i][j+1][2] - terrain[i][j][2]};
			float vx2 = v1_2[1]*v2_2[2] - v1_2[2]*v2_2[1];
			float vy2 = v1_2[2]*v2_2[0] - v1_2[0]*v2_2[2];
			float vz2 = v1_2[0]*v2_2[1] - v1_2[1]*v2_2[0];
			float length2 = sqrt(vx2*vx2 + vy2*vy2 + vz2*vz2);
			float normal2[] = {vx2/length2, vy2/length2, vz2/length2};

			float v1_3[3] = {terrain[i][j+1][0] - terrain[i][j][0], terrain[i][j+1][1] - terrain[i][j][1], terrain[i][j+1][2] - terrain[i][j][2]};
			float v2_3[3] = {terrain[i+1][j][0] - terrain[i][j][0], terrain[i+1][j][1] - terrain[i][j][1], terrain[i+1][j][2] - terrain[i][j][2]};
			float vx3 = v1_3[1]*v2_3[2] - v1_3[2]*v2_3[1];
			float vy3 = v1_3[2]*v2_3[0] - v1_3[0]*v2_3[2];
			float vz3 = v1_3[0]*v2_3[1] - v1_3[1]*v2_3[0];
			float length3 = sqrt(vx3*vx3 + vy3*vy3 + vz3*vz3);
			float normal3[] = {vx3/length3, vy3/length3, vz3/length3};

			float newx = (normal1[0] + normal2[0] + normal3[0])/3;
			float newy = (normal1[1] + normal2[1] + normal3[1])/3;
			float newz = (normal1[2] + normal2[2] + normal3[2])/3;
			float newlength = sqrt(newx*newx + newy*newy + newz*newz);

			glNormal3f(fabs(newx),fabs(newy),fabs(newz));	
		}
		//right-most column
		else if(i>0 && i<gridx-1 && j==gridy-1){
			float v1[3] = {terrain[i-1][j][0] - terrain[i][j][0], terrain[i-1][j][1] - terrain[i][j][1], terrain[i-1][j][2] - terrain[i][j][2]};
			float v2[3] = {terrain[i][j-1][0] - terrain[i][j][0], terrain[i][j-1][1] - terrain[i][j][1], terrain[i][j-1][2] - terrain[i][j][2]};
			float vx = v1[1]*v2[2] - v1[2]*v2[1];
			float vy = v1[2]*v2[0] - v1[0]*v2[2];
			float vz = v1[0]*v2[1] - v1[1]*v2[0];
			float length = sqrt(vx*vx + vy*vy + vz*vz);
			float normal1[] = {vx/length, vy/length, vz/length};

			float v1_2[3] = {terrain[i][j-1][0] - terrain[i][j][0], terrain[i][j-1][1] - terrain[i][j][1], terrain[i][j-1][2] - terrain[i][j][2]};
			float v2_2[3] = {terrain[i+1][j-1][0] - terrain[i][j][0], terrain[i+1][j-1][1] - terrain[i][j][1], terrain[i+1][j-1][2] - terrain[i][j][2]};
			float vx2 = v1_2[1]*v2_2[2] - v1_2[2]*v2_2[1];
			float vy2 = v1_2[2]*v2_2[0] - v1_2[0]*v2_2[2];
			float vz2 = v1_2[0]*v2_2[1] - v1_2[1]*v2_2[0];
			float length2 = sqrt(vx2*vx2 + vy2*vy2 + vz2*vz2);
			float normal2[] = {vx2/length2, vy2/length2, vz2/length2};

			float v1_3[3] = {terrain[i+1][j-1][0] - terrain[i][j][0], terrain[i+1][j-1][1] - terrain[i][j][1], terrain[i+1][j-1][2] - terrain[i][j][2]};
			float v2_3[3] = {terrain[i+1][j][0] - terrain[i][j][0], terrain[i+1][j][1] - terrain[i][j][1], terrain[i+1][j][2] - terrain[i][j][2]};
			float vx3 = v1_3[1]*v2_3[2] - v1_3[2]*v2_3[1];
			float vy3 = v1_3[2]*v2_3[0] - v1_3[0]*v2_3[2];
			float vz3 = v1_3[0]*v2_3[1] - v1_3[1]*v2_3[0];
			float length3 = sqrt(vx3*vx3 + vy3*vy3 + vz3*vz3);
			float normal3[] = {vx3/length3, vy3/length3, vz3/length3};

			float newx = (normal1[0] + normal2[0] + normal3[0])/3;
			float newy = (normal1[1] + normal2[1] + normal3[1])/3;
			float newz = (normal1[2] + normal2[2] + normal3[2])/3;
			float newlength = sqrt(newx*newx + newy*newy + newz*newz);

			glNormal3f(fabs(newx),fabs(newy),fabs(newz));
		}
		//bottom left corner
		else if(i==gridx-1 && j==0){
			float v1[3] = {terrain[i-1][j][0] - terrain[i][j][0], terrain[i-1][j][1] - terrain[i][j][1], terrain[i-1][j][2] - terrain[i][j][2]};
			float v2[3] = {terrain[i-1][j+1][0] - terrain[i][j][0], terrain[i-1][j+1][1] - terrain[i][j][1], terrain[i-1][j+1][2] - terrain[i][j][2]};
			float vx = v1[1]*v2[2] - v1[2]*v2[1];
			float vy = v1[2]*v2[0] - v1[0]*v2[2];
			float vz = v1[0]*v2[1] - v1[1]*v2[0];
			float length = sqrt(vx*vx + vy*vy + vz*vz);
			float normal1[] = {vx/length, vy/length, vz/length};

			float v1_2[3] = {terrain[i-1][j+1][0] - terrain[i][j][0], terrain[i-1][j+1][1] - terrain[i][j][1], terrain[i-1][j+1][2] - terrain[i][j][2]};
			float v2_2[3] = {terrain[i][j+1][0] - terrain[i][j][0], terrain[i][j+1][1] - terrain[i][j][1], terrain[i][j+1][2] - terrain[i][j][2]};
			float vx2 = v1_2[1]*v2_2[2] - v1_2[2]*v2_2[1];
			float vy2 = v1_2[2]*v2_2[0] - v1_2[0]*v2_2[2];
			float vz2 = v1_2[0]*v2_2[1] - v1_2[1]*v2_2[0];
			float length2 = sqrt(vx2*vx2 + vy2*vy2 + vz2*vz2);
			float normal2[] = {vx2/length2, vy2/length2, vz2/length2};

			float newx = (normal1[0] + normal2[0])/2;
			float newy = (normal1[1] + normal2[1])/2;
			float newz = (normal1[2] + normal2[2])/2;
			float newlength = sqrt(newx*newx + newy*newy + newz*newz);

			glNormal3f(fabs(newx),fabs(newy),fabs(newz));
		}
		//bottom row
		else if(i==gridx-1 && j>0 && j<gridy-1){
			float v1[3] = {terrain[i-1][j][0] - terrain[i][j][0], terrain[i-1][j][1] - terrain[i][j][1], terrain[i-1][j][2] - terrain[i][j][2]};
			float v2[3] = {terrain[i][j-1][0] - terrain[i][j][0], terrain[i][j-1][1] - terrain[i][j][1], terrain[i][j-1][2] - terrain[i][j][2]};
			float vx = v1[1]*v2[2] - v1[2]*v2[1];
			float vy = v1[2]*v2[0] - v1[0]*v2[2];
			float vz = v1[0]*v2[1] - v1[1]*v2[0];
			float length = sqrt(vx*vx + vy*vy + vz*vz);
			float normal1[] = {vx/length, vy/length, vz/length};

			float v1_2[3] = {terrain[i-1][j][0] - terrain[i][j][0], terrain[i-1][j][1] - terrain[i][j][1], terrain[i-1][j][2] - terrain[i][j][2]};
			float v2_2[3] = {terrain[i-1][j+1][0] - terrain[i][j][0], terrain[i-1][j+1][1] - terrain[i][j][1], terrain[i-1][j+1][2] - terrain[i][j][2]};
			float vx2 = v1_2[1]*v2_2[2] - v1_2[2]*v2_2[1];
			float vy2 = v1_2[2]*v2_2[0] - v1_2[0]*v2_2[2];
			float vz2 = v1_2[0]*v2_2[1] - v1_2[1]*v2_2[0];
			float length2 = sqrt(vx2*vx2 + vy2*vy2 + vz2*vz2);
			float normal2[] = {vx2/length2, vy2/length2, vz2/length2};

			float v1_3[3] = {terrain[i-1][j+1][0] - terrain[i][j][0], terrain[i-1][j+1][1] - terrain[i][j][1], terrain[i-1][j+1][2] - terrain[i][j][2]};
			float v2_3[3] = {terrain[i][j+1][0] - terrain[i][j][0], terrain[i][j+1][1] - terrain[i][j][1], terrain[i][j+1][2] - terrain[i][j][2]};
			float vx3 = v1_3[1]*v2_3[2] - v1_3[2]*v2_3[1];
			float vy3 = v1_3[2]*v2_3[0] - v1_3[0]*v2_3[2];
			float vz3 = v1_3[0]*v2_3[1] - v1_3[1]*v2_3[0];
			float length3 = sqrt(vx3*vx3 + vy3*vy3 + vz3*vz3);
			float normal3[] = {vx3/length3, vy3/length3, vz3/length3};

			float newx = (normal1[0] + normal2[0] + normal3[0])/3;
			float newy = (normal1[1] + normal2[1] + normal3[1])/3;
			float newz = (normal1[2] + normal2[2] + normal3[2])/3;
			float newlength = sqrt(newx*newx + newy*newy + newz*newz);

			glNormal3f(fabs(newx),fabs(newy),fabs(newz));
		}
		//bottom-right corner
		else if(i==gridx-1 && j==gridy-1){
			float v1[3] = {terrain[i-1][j][0] - terrain[i][j][0], terrain[i-1][j][1] - terrain[i][j][1], terrain[i-1][j][2] - terrain[i][j][2]};
			float v2[3] = {terrain[i][j-1][0] - terrain[i][j][0], terrain[i][j-1][1] - terrain[i][j][1], terrain[i][j-1][2] - terrain[i][j][2]};
			float vx = v1[1]*v2[2] - v1[2]*v2[1];
			float vy = v1[2]*v2[0] - v1[0]*v2[2];
			float vz = v1[0]*v2[1] - v1[1]*v2[0];
			float length = sqrt(vx*vx + vy*vy + vz*vz);
			float normal1[] = {vx/length, vy/length, vz/length};

			glNormal3f(vx/length, vy/length, vz/length);
		}
		//every other point
		else{
			float v1[3] = {terrain[i-1][j][0] - terrain[i][j][0], terrain[i-1][j][1] - terrain[i][j][1], terrain[i-1][j][2] - terrain[i][j][2]};
			float v2[3] = {terrain[i][j-1][0] - terrain[i][j][0], terrain[i][j-1][1] - terrain[i][j][1], terrain[i][j-1][2] - terrain[i][j][2]};
			float vx = v1[1]*v2[2] - v1[2]*v2[1];
			float vy = v1[2]*v2[0] - v1[0]*v2[2];
			float vz = v1[0]*v2[1] - v1[1]*v2[0];
			float length = sqrt(vx*vx + vy*vy + vz*vz);
			float normal1[] = {vx/length, vy/length, vz/length};

			float v1_2[3] = {terrain[i][j-1][0] - terrain[i][j][0], terrain[i][j-1][1] - terrain[i][j][1], terrain[i][j-1][2] - terrain[i][j][2]};
			float v2_2[3] = {terrain[i+1][j-1][0] - terrain[i][j][0], terrain[i+1][j-1][1] - terrain[i][j][1], terrain[i+1][j-1][2] - terrain[i][j][2]};
			float vx2 = v1_2[1]*v2_2[2] - v1_2[2]*v2_2[1];
			float vy2 = v1_2[2]*v2_2[0] - v1_2[0]*v2_2[2];
			float vz2 = v1_2[0]*v2_2[1] - v1_2[1]*v2_2[0];
			float length2 = sqrt(vx2*vx2 + vy2*vy2 + vz2*vz2);
			float normal2[] = {vx2/length2, vy2/length2, vz2/length2};

			float v1_3[3] = {terrain[i+1][j-1][0] - terrain[i][j][0], terrain[i+1][j-1][1] - terrain[i][j][1], terrain[i+1][j-1][2] - terrain[i][j][2]};
			float v2_3[3] = {terrain[i+1][j][0] - terrain[i][j][0], terrain[i+1][j][1] - terrain[i][j][1], terrain[i+1][j][2] - terrain[i][j][2]};
			float vx3 = v1_3[1]*v2_3[2] - v1_3[2]*v2_3[1];
			float vy3 = v1_3[2]*v2_3[0] - v1_3[0]*v2_3[2];
			float vz3 = v1_3[0]*v2_3[1] - v1_3[1]*v2_3[0];
			float length3 = sqrt(vx3*vx3 + vy3*vy3 + vz3*vz3);
			float normal3[] = {vx3/length3, vy3/length3, vz3/length3};

			float v1_4[3] = {terrain[i-1][j][0] - terrain[i][j][0], terrain[i-1][j][1] - terrain[i][j][1], terrain[i-1][j][2] - terrain[i][j][2]};
			float v2_4[3] = {terrain[i-1][j+1][0] - terrain[i][j][0], terrain[i-1][j+1][1] - terrain[i][j][1], terrain[i-1][j+1][2] - terrain[i][j][2]};
			float vx4 = v1_4[1]*v2_4[2] - v1_4[2]*v2_4[1];
			float vy4 = v1_4[2]*v2_4[0] - v1_4[0]*v2_4[2];
			float vz4 = v1_4[0]*v2_4[1] - v1_4[1]*v2_4[0];
			float length4 = sqrt(vx4*vx4 + vy4*vy4 + vz4*vz4);
			float normal4[] = {vx4/length4, vy4/length4, vz4/length4};

			float v1_5[3] = {terrain[i-1][j+1][0] - terrain[i][j][0], terrain[i-1][j+1][1] - terrain[i][j][1], terrain[i-1][j+1][2] - terrain[i][j][2]};
			float v2_5[3] = {terrain[i][j+1][0] - terrain[i][j][0], terrain[i][j+1][1] - terrain[i][j][1], terrain[i][j+1][2] - terrain[i][j][2]};
			float vx5 = v1_5[1]*v2_5[2] - v1_5[2]*v2_5[1];
			float vy5 = v1_5[2]*v2_5[0] - v1_5[0]*v2_5[2];
			float vz5 = v1_5[0]*v2_5[1] - v1_5[1]*v2_5[0];
			float length5 = sqrt(vx5*vx5 + vy5*vy5 + vz5*vz5);
			float normal5[] = {vx5/length5, vy5/length5, vz5/length5};

			float v1_6[3] = {terrain[i][j+1][0] - terrain[i][j][0], terrain[i][j+1][1] - terrain[i][j][1], terrain[i][j+1][2] - terrain[i][j][2]};
			float v2_6[3] = {terrain[i+1][j][0] - terrain[i][j][0], terrain[i+1][j][1] - terrain[i][j][1], terrain[i+1][j][2] - terrain[i][j][2]};
			float vx6 = v1_6[1]*v2_6[2] - v1_6[2]*v2_6[1];
			float vy6 = v1_6[2]*v2_6[0] - v1_6[0]*v2_6[2];
			float vz6 = v1_6[0]*v2_6[1] - v1_6[1]*v2_6[0];
			float length6 = sqrt(vx6*vx6 + vy6*vy6 + vz6*vz6);
			float normal6[] = {vx6/length6, vy6/length6, vz6/length6};

			float newx = (normal1[0] + normal2[0] + normal3[0] + normal4[0] + normal5[0] + normal6[0])/6;
			float newy = (normal1[1] + normal2[1] + normal3[1] + normal4[1] + normal5[1] + normal6[1])/6;
			float newz = (normal1[2] + normal2[2] + normal3[2] + normal4[2] + normal5[2] + normal6[2])/6;
			float newlength = sqrt(newx*newx + newy*newy + newz*newz);

			glNormal3f((newx),(newy),(newz));
		}
}

//Display using triangles and smooth shading
//Similar process as flat shading
//Main difference is the normals are calculated before placed each vertex
void displayTriSmooth(bool wirefill){
	if(wirefill==false){
		glLineWidth(1);
		glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
		glLightfv(GL_LIGHT1, GL_POSITION, light_pos2);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, m_amb);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, m_diff);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, m_spec);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shiny);
	}
	else{
		glLineWidth(3);
		glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
		glLightfv(GL_LIGHT1, GL_POSITION, light_pos2);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, m_amb2);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, m_diff2);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, m_spec2);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shiny2);
	}
	for(int i=0; i<gridx-1; i++){
		for(int j=1; j<gridy; j++){
			if(wirefill==true){
				glBegin(GL_LINE_LOOP);
			}
			else{
				glBegin(GL_TRIANGLES);
			}
			float color = (((terrain[i][j-1][1] - (-4))*cRange)/hRange)+0;
			glColor3f(color,color,color);
			if(wirefill==true){
				glColor3f(0,color,color);
			}
			
			setNormalTri(i,j-1);
			glVertex3f(terrain[i][j-1][0], terrain[i][j-1][1], terrain[i][j-1][2]);

			color = (((terrain[i+1][j-1][1] - (-4))*cRange)/hRange)+0;
			glColor3f(color,color,color);
			if(wirefill==true){
				glColor3f(0,color,color);
			}
			
			setNormalTri(i+1, j-1);
			glVertex3f(terrain[i+1][j-1][0], terrain[i+1][j-1][1], terrain[i+1][j-1][2]);

			color = (((terrain[i][j][1] - (-4))*cRange)/hRange)+0;
			glColor3f(color,color,color);
			if(wirefill==true){
				glColor3f(0,color,color);
			}	
			setNormalTri(i,j);
			glVertex3f(terrain[i][j][0], terrain[i][j][1], terrain[i][j][2]);
			glEnd();

			if(wirefill==true){
				glBegin(GL_LINE_LOOP);
			}
			else{
				glBegin(GL_TRIANGLES);
			}
			color = (((terrain[i+1][j-1][1] - (-4))*cRange)/hRange)+0;
			glColor3f(color,color,color);
			if(wirefill==true){
				glColor3f(0,color,color);
			}
			setNormalTri(i+1,j-1);
			glVertex3f(terrain[i+1][j-1][0], terrain[i+1][j-1][1], terrain[i+1][j-1][2]);

			color = (((terrain[i+1][j][1] - (-4))*cRange)/hRange)+0;
			glColor3f(color,color,color);
			if(wirefill==true){
				glColor3f(0,color,color);
			}
			setNormalTri(i+1,j);
			glVertex3f(terrain[i+1][j][0], terrain[i+1][j][1], terrain[i+1][j][2]);

			color = (((terrain[i][j][1] - (-4))*cRange)/hRange)+0;
			glColor3f(color,color,color);
			if(wirefill==true){
				glColor3f(0,color,color);
			}			
			setNormalTri(i,j);
			glVertex3f(terrain[i][j][0], terrain[i][j][1], terrain[i][j][2]);
			glEnd();
		}
	}
}

//display using quads
//Similar to drawing using triangles
//Main difference is the use of GL_QUADS
void displayQuad(bool wirefill){
	if(wirefill==false){
		glLineWidth(1);
		glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
		glLightfv(GL_LIGHT1, GL_POSITION, light_pos2);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, m_amb);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, m_diff);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, m_spec);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shiny);
	}
	else{
		glLineWidth(3);
		glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
		glLightfv(GL_LIGHT1, GL_POSITION, light_pos2);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, m_amb2);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, m_diff2);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, m_spec2);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shiny2);
	}
	for(int i=0; i<gridx-1; i++){
		for(int j=1; j<gridy; j++){
			if(wirefill==true){
				glBegin(GL_LINE_LOOP);
			}
			else{
				glBegin(GL_QUADS);
			}

			//top left
			float color = (((terrain[i][j-1][1] - (-2))*cRange)/hRange)+0;
			glColor3f(color,color,color);
			if(wirefill==true){
				glColor3f(0,color,color);
			}
			glVertex3f(terrain[i][j-1][0], terrain[i][j-1][1], terrain[i][j-1][2]);

			//bottom left
			color = (((terrain[i+1][j-1][1] - (-4))*cRange)/hRange)+0;
			glColor3f(color,color,color);
			if(wirefill==true){
				glColor3f(0,color,color);
			}
			glVertex3f(terrain[i+1][j-1][0], terrain[i+1][j-1][1], terrain[i+1][j-1][2]);

			//bottom right
			color = (((terrain[i+1][j][1] - (-4))*cRange)/hRange)+0;
			glColor3f(color,color,color);
			if(wirefill==true){
				glColor3f(0,color,color);
			}
			glVertex3f(terrain[i+1][j][0], terrain[i+1][j][1], terrain[i+1][j][2]);

			//top right
			color = (((terrain[i][j][1] - (-4))*cRange)/hRange)+0;
			glColor3f(color,color,color);
			if(wirefill==true){
				glColor3f(0,color,color);
			}
			float v1[3] = {terrain[i][j-1][0] - terrain[i][j][0], terrain[i][j-1][1] - terrain[i][j][1], terrain[i][j-1][2] - terrain[i][j][2]};
			float v2[3] = {terrain[i+1][j-1][0] - terrain[i][j][0], terrain[i+1][j-1][1] - terrain[i][j][1], terrain[i+1][j-1][2] - terrain[i][j][2]};
			float vx = v1[1]*v2[2] - v1[2]*v2[1];
			float vy = v1[2]*v2[0] - v1[0]*v2[2];
			float vz = v1[0]*v2[1] - v1[1]*v2[0];
			float length = sqrt(vx*vx + vy*vy + vz*vz);

			float v12[3] = {terrain[i+1][j-1][0] - terrain[i][j][0], terrain[i+1][j-1][1] - terrain[i][j][1], terrain[i+1][j-1][2] - terrain[i][j][2]};
			float v22[3] = {terrain[i+1][j][0] - terrain[i][j][0], terrain[i+1][j][1] - terrain[i][j][1], terrain[i+1][j][2] - terrain[i][j][2]};
			float vx2 = v12[1]*v22[2] - v12[2]*v22[1];
			float vy2 = v12[2]*v22[0] - v12[0]*v22[2];
			float vz2 = v12[0]*v22[1] - v12[1]*v22[0];
			float length2 = sqrt(vx2*vx2 + vy2*vy2 + vz2*vz2);

			float avgX = ((vx/length)+(vx2/length2))/2;
			float avgY = ((vy/length)+(vy2/length2))/2;
			float avgZ = ((vz/length)+(vz2/length2))/2;

			glNormal3f(avgX, avgY, avgZ);

			glVertex3f(terrain[i][j][0], terrain[i][j][1], terrain[i][j][2]);
			glEnd();

		}
	}
}

//Calculates a normal by using int i and j from the for loop
//Used to calculate vertex normals for quads
//The value of i and j determines how many face normals are required
void setNormalQuad(int i, int j){
		//top left corner
		if(i==0 && j==0){
			float v1[3] = {terrain[i][j+1][0] - terrain[i][j][0], terrain[i][j+1][1] - terrain[i][j][1], terrain[i][j+1][2] - terrain[i][j][2]};
			float v2[3] = {terrain[i+1][j][0] - terrain[i][j][0], terrain[i+1][j][1] - terrain[i][j][1], terrain[i+1][j][2] - terrain[i][j][2]};
			float vx = v1[1]*v2[2] - v1[2]*v2[1];
			float vy = v1[2]*v2[0] - v1[0]*v2[2];
			float vz = v1[0]*v2[1] - v1[1]*v2[0];
			float length = sqrt(vx*vx + vy*vy + vz*vz);

			float v12[3] = {terrain[i+1][j][0] - terrain[i+1][j+1][0], terrain[i+1][j][1] - terrain[i+1][j+1][1], terrain[i+1][j][2] - terrain[i+1][j+1][2]};
			float v22[3] = {terrain[i][j+1][0] - terrain[i][j][0], terrain[i][j+1][1] - terrain[i][j][1], terrain[i][j+1][2] - terrain[i][j][2]};
			float vx2 = v12[1]*v22[2] - v12[2]*v22[1];
			float vy2 = v12[2]*v22[0] - v12[0]*v22[2];
			float vz2 = v12[0]*v22[1] - v12[1]*v22[0];
			float length2 = sqrt(vx2*vx2 + vy2*vy2 + vz2*vz2);

			float avgX = ((vx/length)+(vx2/length2))/2;
			float avgY = ((vy/length)+(vy2/length2))/2;
			float avgZ = ((vz/length)+(vz2/length2))/2;

			glNormal3f(avgX, avgY, avgZ);
		}
		//top row
		else if(i==0 && j>0 && j<gridy-1){
			float v1[3] = {terrain[i][j+1][0] - terrain[i][j][0], terrain[i][j+1][1] - terrain[i][j][1], terrain[i][j+1][2] - terrain[i][j][2]};
			float v2[3] = {terrain[i+1][j][0] - terrain[i][j][0], terrain[i+1][j][1] - terrain[i][j][1], terrain[i+1][j][2] - terrain[i][j][2]};
			float vx = v1[1]*v2[2] - v1[2]*v2[1];
			float vy = v1[2]*v2[0] - v1[0]*v2[2];
			float vz = v1[0]*v2[1] - v1[1]*v2[0];
			float length = sqrt(vx*vx + vy*vy + vz*vz);

			float v12[3] = {terrain[i+1][j-1][0] - terrain[i+1][j+1][0], terrain[i+1][j-1][1] - terrain[i+1][j+1][1], terrain[i+1][j-1][2] - terrain[i+1][j+1][2]};
			float v22[3] = {terrain[i][j+1][0] - terrain[i][j][0], terrain[i][j+1][1] - terrain[i][j][1], terrain[i][j+1][2] - terrain[i][j][2]};
			float vx2 = v12[1]*v22[2] - v12[2]*v22[1];
			float vy2 = v12[2]*v22[0] - v12[0]*v22[2];
			float vz2 = v12[0]*v22[1] - v12[1]*v22[0];
			float length2 = sqrt(vx2*vx2 + vy2*vy2 + vz2*vz2);

			float avgX = ((vx/length)+(vx2/length2))/2;
			float avgY = ((vy/length)+(vy2/length2))/2;
			float avgZ = ((vz/length)+(vz2/length2))/2;
			float normal1[3] = {avgX,avgY,avgZ};

			float v1_3[3] = {terrain[i][j-1][0] - terrain[i][j][0], terrain[i][j-1][1] - terrain[i][j][1], terrain[i][j-1][2] - terrain[i][j][2]};
			float v2_3[3] = {terrain[i+1][j-1][0] - terrain[i][j][0], terrain[i+1][j-1][1] - terrain[i][j][1], terrain[i+1][j-1][2] - terrain[i][j][2]};
			float vx3 = v1_3[1]*v2_3[2] - v1_3[2]*v2_3[1];
			float vy3 = v1_3[2]*v2_3[0] - v1_3[0]*v2_3[2];
			float vz3 = v1_3[0]*v2_3[1] - v1_3[1]*v2_3[0];
			float length3 = sqrt(vx3*vx3 + vy3*vy3 + vz3*vz3);

			float v1_4[3] = {terrain[i+1][j-1][0] - terrain[i][j][0], terrain[i+1][j-1][1] - terrain[i][j][1], terrain[i+1][j-1][2] - terrain[i][j][2]};
			float v2_4[3] = {terrain[i+1][j][0] - terrain[i][j][0], terrain[i+1][j][1] - terrain[i][j][1], terrain[i+1][j][2] - terrain[i][j][2]};
			float vx4 = v1_4[1]*v2_4[2] - v1_4[2]*v2_4[1];
			float vy4 = v1_4[2]*v2_4[0] - v1_4[0]*v2_4[2];
			float vz4 = v1_4[0]*v2_4[1] - v1_4[1]*v2_4[0];
			float length4 = sqrt(vx4*vx4 + vy4*vy4 + vz4*vz4);

			float avgX2 = ((vx/length)+(vx2/length2))/2;
			float avgY2 = ((vy/length)+(vy2/length2))/2;
			float avgZ2 = ((vz/length)+(vz2/length2))/2;
			float normal2[3] = {avgX2,avgY2,avgZ2};

			float newx = (avgX + avgX2)/2;
			float newy = (avgY + avgY2)/2;
			float newz = (avgZ + avgZ2)/2;

			glNormal3f(newx,newy,newz);
		}
		//top right corner
		else if(i==0 && j==gridy-1){
			float v1[3] = {terrain[i][j-1][0] - terrain[i][j][0], terrain[i][j-1][1] - terrain[i][j][1], terrain[i][j-1][2] - terrain[i][j][2]};
			float v2[3] = {terrain[i+1][j-1][0] - terrain[i][j][0], terrain[i+1][j-1][1] - terrain[i][j][1], terrain[i+1][j-1][2] - terrain[i][j][2]};
			float vx = v1[1]*v2[2] - v1[2]*v2[1];
			float vy = v1[2]*v2[0] - v1[0]*v2[2];
			float vz = v1[0]*v2[1] - v1[1]*v2[0];
			float length = sqrt(vx*vx + vy*vy + vz*vz);
			float normal1[] = {vx/length, vy/length, vz/length};

			float v1_2[3] = {terrain[i+1][j-1][0] - terrain[i][j][0], terrain[i+1][j-1][1] - terrain[i][j][1], terrain[i+1][j-1][2] - terrain[i][j][2]};
			float v2_2[3] = {terrain[i+1][j][0] - terrain[i][j][0], terrain[i+1][j][1] - terrain[i][j][1], terrain[i+1][j][2] - terrain[i][j][2]};
			float vx2 = v1_2[1]*v2_2[2] - v1_2[2]*v2_2[1];
			float vy2 = v1_2[2]*v2_2[0] - v1_2[0]*v2_2[2];
			float vz2 = v1_2[0]*v2_2[1] - v1_2[1]*v2_2[0];
			float length2 = sqrt(vx2*vx2 + vy2*vy2 + vz2*vz2);
			float normal2[] = {vx2/length2, vy2/length2, vz2/length2};

			float newx = (normal1[0] + normal2[0])/2;
			float newy = (normal1[1] + normal2[1])/2;
			float newz = (normal1[2] + normal2[2])/2;

			glNormal3f(newx,newy,newz);	
		}
		//left-most column
		else if(i>0 && i<gridx-1 && j==0){
			float v1[3] = {terrain[i][j+1][0] - terrain[i][j][0], terrain[i][j+1][1] - terrain[i][j][1], terrain[i][j+1][2] - terrain[i][j][2]};
			float v2[3] = {terrain[i+1][j+1][0] - terrain[i][j][0], terrain[i+1][j+1][1] - terrain[i][j][1], terrain[i+1][j+1][2] - terrain[i][j][2]};
			float vx = v1[1]*v2[2] - v1[2]*v2[1];
			float vy = v1[2]*v2[0] - v1[0]*v2[2];
			float vz = v1[0]*v2[1] - v1[1]*v2[0];
			float length = sqrt(vx*vx + vy*vy + vz*vz);

			float v12[3] = {terrain[i+1][j][0] - terrain[i][j][0], terrain[i+1][j][1] - terrain[i][j][1], terrain[i+1][j][2] - terrain[i][j][2]};
			float v22[3] = {terrain[i+1][j+1][0] - terrain[i][j][0], terrain[i+1][j+1][1] - terrain[i][j][1], terrain[i+1][j+1][2] - terrain[i][j][2]};
			float vx2 = v12[1]*v22[2] - v12[2]*v22[1];
			float vy2 = v12[2]*v22[0] - v12[0]*v22[2];
			float vz2 = v12[0]*v22[1] - v12[1]*v22[0];
			float length2 = sqrt(vx2*vx2 + vy2*vy2 + vz2*vz2);

			float avgX = ((vx/length)+(vx2/length2))/2;
			float avgY = ((vy/length)+(vy2/length2))/2;
			float avgZ = ((vz/length)+(vz2/length2))/2;
			float normal1[3] = {avgX,avgY,avgZ};

			float v1_3[3] = {terrain[i][j+1][0] - terrain[i][j][0], terrain[i][j+1][1] - terrain[i][j][1], terrain[i][j+1][2] - terrain[i][j][2]};
			float v2_3[3] = {terrain[i+1][j][0] - terrain[i][j][0], terrain[i+1][j][1] - terrain[i][j][1], terrain[i+1][j][2] - terrain[i][j][2]};
			float vx3 = v1_3[1]*v2_3[2] - v1_3[2]*v2_3[1];
			float vy3 = v1_3[2]*v2_3[0] - v1_3[0]*v2_3[2];
			float vz3 = v1_3[0]*v2_3[1] - v1_3[1]*v2_3[0];
			float length3 = sqrt(vx3*vx3 + vy3*vy3 + vz3*vz3);

			float v1_4[3] = {terrain[i+1][j][0] - terrain[i+1][j+1][0], terrain[i+1][j][1] - terrain[i+1][j+1][1], terrain[i+1][j][2] - terrain[i+1][j+1][2]};
			float v2_4[3] = {terrain[i][j+1][0] - terrain[i+1][j+1][0], terrain[i][j+1][1] - terrain[i+1][j+1][1], terrain[i][j+1][2] - terrain[i+1][j+1][2]};
			float vx4 = v1_4[1]*v2_4[2] - v1_4[2]*v2_4[1];
			float vy4 = v1_4[2]*v2_4[0] - v1_4[0]*v2_4[2];
			float vz4 = v1_4[0]*v2_4[1] - v1_4[1]*v2_4[0];
			float length4 = sqrt(vx4*vx4 + vy4*vy4 + vz4*vz4);

			float avgX2 = ((vx/length)+(vx2/length2))/2;
			float avgY2 = ((vy/length)+(vy2/length2))/2;
			float avgZ2 = ((vz/length)+(vz2/length2))/2;
			float normal2[3] = {avgX2,avgY2,avgZ2};

			float newx = (avgX + avgX2)/2;
			float newy = (avgY + avgY2)/2;
			float newz = (avgZ + avgZ2)/2;

			glNormal3f(newx,newy,newz);
		}
		//right-most column
		else if(i>0 && i<gridx-1 && j==gridy-1){
			float v1[3] = {terrain[i][j-1][0] - terrain[i][j][0], terrain[i][j-1][1] - terrain[i][j][1], terrain[i][j-1][2] - terrain[i][j][2]};
			float v2[3] = {terrain[i+1][j][0] - terrain[i][j][0], terrain[i+1][j][1] - terrain[i][j][1], terrain[i+1][j][2] - terrain[i][j][2]};
			float vx = v1[1]*v2[2] - v1[2]*v2[1];
			float vy = v1[2]*v2[0] - v1[0]*v2[2];
			float vz = v1[0]*v2[1] - v1[1]*v2[0];
			float length = sqrt(vx*vx + vy*vy + vz*vz);

			float v12[3] = {terrain[i][j-1][0] - terrain[i+1][j-1][0], terrain[i][j-1][1] - terrain[i+1][j-1][1], terrain[i][j-1][2] - terrain[i+1][j-1][2]};
			float v22[3] = {terrain[i+1][j][0] - terrain[i+1][j-1][0], terrain[i+1][j][1] - terrain[i+1][j-1][1], terrain[i+1][j][2] - terrain[i+1][j-1][2]};
			float vx2 = v12[1]*v22[2] - v12[2]*v22[1];
			float vy2 = v12[2]*v22[0] - v12[0]*v22[2];
			float vz2 = v12[0]*v22[1] - v12[1]*v22[0];
			float length2 = sqrt(vx2*vx2 + vy2*vy2 + vz2*vz2);

			float avgX = ((vx/length)+(vx2/length2))/2;
			float avgY = ((vy/length)+(vy2/length2))/2;
			float avgZ = ((vz/length)+(vz2/length2))/2;
			float normal1[3] = {avgX,avgY,avgZ};

			float v1_3[3] = {terrain[i][j-1][0] - terrain[i][j][0], terrain[i][j-1][1] - terrain[i][j][1], terrain[i][j-1][2] - terrain[i][j][2]};
			float v2_3[3] = {terrain[i+1][j-1][0] - terrain[i][j][0], terrain[i+1][j-1][1] - terrain[i][j][1], terrain[i+1][j-1][2] - terrain[i][j][2]};
			float vx3 = v1_3[1]*v2_3[2] - v1_3[2]*v2_3[1];
			float vy3 = v1_3[2]*v2_3[0] - v1_3[0]*v2_3[2];
			float vz3 = v1_3[0]*v2_3[1] - v1_3[1]*v2_3[0];
			float length3 = sqrt(vx3*vx3 + vy3*vy3 + vz3*vz3);

			float v1_4[3] = {terrain[i+1][j][0] - terrain[i][j][0], terrain[i+1][j][1] - terrain[i][j][1], terrain[i+1][j][2] - terrain[i][j][2]};
			float v2_4[3] = {terrain[i+1][j-1][0] - terrain[i][j][0], terrain[i+1][j-1][1] - terrain[i][j][1], terrain[i+1][j-1][2] - terrain[i][j][2]};
			float vx4 = v1_4[1]*v2_4[2] - v1_4[2]*v2_4[1];
			float vy4 = v1_4[2]*v2_4[0] - v1_4[0]*v2_4[2];
			float vz4 = v1_4[0]*v2_4[1] - v1_4[1]*v2_4[0];
			float length4 = sqrt(vx4*vx4 + vy4*vy4 + vz4*vz4);

			float avgX2 = ((vx/length)+(vx2/length2))/2;
			float avgY2 = ((vy/length)+(vy2/length2))/2;
			float avgZ2 = ((vz/length)+(vz2/length2))/2;
			float normal2[3] = {avgX2,avgY2,avgZ2};

			float newx = (avgX + avgX2)/2;
			float newy = (avgY + avgY2)/2;
			float newz = (avgZ + avgZ2)/2;

			glNormal3f(newx,newy,newz);
		}
		//bottom left corner
		else if(i==gridx-1 && j==0){
			float v1[3] = {terrain[i-1][j][0] - terrain[i][j][0], terrain[i-1][j][1] - terrain[i][j][1], terrain[i-1][j][2] - terrain[i][j][2]};
			float v2[3] = {terrain[i-1][j+1][0] - terrain[i][j][0], terrain[i-1][j+1][1] - terrain[i][j][1], terrain[i-1][j+1][2] - terrain[i][j][2]};
			float vx = v1[1]*v2[2] - v1[2]*v2[1];
			float vy = v1[2]*v2[0] - v1[0]*v2[2];
			float vz = v1[0]*v2[1] - v1[1]*v2[0];
			float length = sqrt(vx*vx + vy*vy + vz*vz);
			float normal1[] = {vx/length, vy/length, vz/length};

			float v1_2[3] = {terrain[i-1][j+1][0] - terrain[i][j][0], terrain[i-1][j+1][1] - terrain[i][j][1], terrain[i-1][j+1][2] - terrain[i][j][2]};
			float v2_2[3] = {terrain[i][j+1][0] - terrain[i][j][0], terrain[i][j+1][1] - terrain[i][j][1], terrain[i][j+1][2] - terrain[i][j][2]};
			float vx2 = v1_2[1]*v2_2[2] - v1_2[2]*v2_2[1];
			float vy2 = v1_2[2]*v2_2[0] - v1_2[0]*v2_2[2];
			float vz2 = v1_2[0]*v2_2[1] - v1_2[1]*v2_2[0];
			float length2 = sqrt(vx2*vx2 + vy2*vy2 + vz2*vz2);
			float normal2[] = {vx2/length2, vy2/length2, vz2/length2};

			float newx = (normal1[0] + normal2[0])/2;
			float newy = (normal1[1] + normal2[1])/2;
			float newz = (normal1[2] + normal2[2])/2;
			float newlength = sqrt(newx*newx + newy*newy + newz*newz);

			glNormal3f(newx,newy,newz);
		}
		//bottom row
		else if(i==gridx-1 && j>0 && j<gridy-1){
			float v1[3] = {terrain[i][j-1][0] - terrain[i][j][0], terrain[i][j-1][1] - terrain[i][j][1], terrain[i][j-1][2] - terrain[i][j][2]};
			float v2[3] = {terrain[i-1][j][0] - terrain[i][j][0], terrain[i-1][j][1] - terrain[i][j][1], terrain[i-1][j][2] - terrain[i][j][2]};
			float vx = v1[1]*v2[2] - v1[2]*v2[1];
			float vy = v1[2]*v2[0] - v1[0]*v2[2];
			float vz = v1[0]*v2[1] - v1[1]*v2[0];
			float length = sqrt(vx*vx + vy*vy + vz*vz);

			float v12[3] = {terrain[i][j-1][0] - terrain[i-1][j-1][0], terrain[i][j-1][1] - terrain[i-1][j-1][1], terrain[i][j-1][2] - terrain[i-1][j-1][2]};
			float v22[3] = {terrain[i-1][j][0] - terrain[i-1][j-1][0], terrain[i-1][j][1] - terrain[i-1][j-1][1], terrain[i-1][j][2] - terrain[i-1][j-1][2]};
			float vx2 = v12[1]*v22[2] - v12[2]*v22[1];
			float vy2 = v12[2]*v22[0] - v12[0]*v22[2];
			float vz2 = v12[0]*v22[1] - v12[1]*v22[0];
			float length2 = sqrt(vx2*vx2 + vy2*vy2 + vz2*vz2);

			float avgX = ((vx/length)+(vx2/length2))/2;
			float avgY = ((vy/length)+(vy2/length2))/2;
			float avgZ = ((vz/length)+(vz2/length2))/2;
			float normal1[3] = {avgX,avgY,avgZ};

			float v1_3[3] = {terrain[i][j+1][0] - terrain[i][j][0], terrain[i][j+1][1] - terrain[i][j][1], terrain[i][j+1][2] - terrain[i][j][2]};
			float v2_3[3] = {terrain[i-1][j+1][0] - terrain[i][j][0], terrain[i-1][j+1][1] - terrain[i][j][1], terrain[i-1][j+1][2] - terrain[i][j][2]};
			float vx3 = v1_3[1]*v2_3[2] - v1_3[2]*v2_3[1];
			float vy3 = v1_3[2]*v2_3[0] - v1_3[0]*v2_3[2];
			float vz3 = v1_3[0]*v2_3[1] - v1_3[1]*v2_3[0];
			float length3 = sqrt(vx3*vx3 + vy3*vy3 + vz3*vz3);

			float v1_4[3] = {terrain[i-1][j][0] - terrain[i][j][0], terrain[i-1][j][1] - terrain[i][j][1], terrain[i-1][j][2] - terrain[i][j][2]};
			float v2_4[3] = {terrain[i-1][j+1][0] - terrain[i][j][0], terrain[i-1][j+1][1] - terrain[i][j][1], terrain[i-1][j+1][2] - terrain[i][j][2]};
			float vx4 = v1_4[1]*v2_4[2] - v1_4[2]*v2_4[1];
			float vy4 = v1_4[2]*v2_4[0] - v1_4[0]*v2_4[2];
			float vz4 = v1_4[0]*v2_4[1] - v1_4[1]*v2_4[0];
			float length4 = sqrt(vx4*vx4 + vy4*vy4 + vz4*vz4);

			float avgX2 = ((vx/length)+(vx2/length2))/2;
			float avgY2 = ((vy/length)+(vy2/length2))/2;
			float avgZ2 = ((vz/length)+(vz2/length2))/2;
			float normal2[3] = {avgX2,avgY2,avgZ2};

			float newx = (avgX + avgX2)/2;
			float newy = (avgY + avgY2)/2;
			float newz = (avgZ + avgZ2)/2;

			glNormal3f(newx,newy,newz);
		}
		//bottom-right corner
		else if(i==gridx-1 && j==gridy-1){
			float v1[3] = {terrain[i-1][j][0] - terrain[i][j][0], terrain[i-1][j][1] - terrain[i][j][1], terrain[i-1][j][2] - terrain[i][j][2]};
			float v2[3] = {terrain[i][j-1][0] - terrain[i][j][0], terrain[i][j-1][1] - terrain[i][j][1], terrain[i][j-1][2] - terrain[i][j][2]};
			float vx = v1[1]*v2[2] - v1[2]*v2[1];
			float vy = v1[2]*v2[0] - v1[0]*v2[2];
			float vz = v1[0]*v2[1] - v1[1]*v2[0];
			float length = sqrt(vx*vx + vy*vy + vz*vz);

			float v12[3] = {terrain[i][j-1][0] - terrain[i-1][j-1][0], terrain[i][j-1][1] - terrain[i-1][j-1][1], terrain[i][j-1][2] - terrain[i-1][j-1][2]};
			float v22[3] = {terrain[i-1][j][0] - terrain[i-1][j-1][0], terrain[i-1][j][1] - terrain[i-1][j-1][1], terrain[i-1][j][2] - terrain[i-1][j-1][2]};
			float vx2 = v12[1]*v22[2] - v12[2]*v22[1];
			float vy2 = v12[2]*v22[0] - v12[0]*v22[2];
			float vz2 = v12[0]*v22[1] - v12[1]*v22[0];
			float length2 = sqrt(vx2*vx2 + vy2*vy2 + vz2*vz2);

			float avgX = ((vx/length)+(vx2/length2))/2;
			float avgY = ((vy/length)+(vy2/length2))/2;
			float avgZ = ((vz/length)+(vz2/length2))/2;
			float normal1[3] = {avgX,avgY,avgZ};

			glNormal3f(vx/length, vy/length, vz/length);
		}
		//every other point
		else{
			float v1[3] = {terrain[i][j-1][0] - terrain[i][j][0], terrain[i][j-1][1] - terrain[i][j][1], terrain[i][j-1][2] - terrain[i][j][2]};
			float v2[3] = {terrain[i+1][j][0] - terrain[i][j][0], terrain[i+1][j][1] - terrain[i][j][1], terrain[i+1][j][2] - terrain[i][j][2]};
			float vx = v1[1]*v2[2] - v1[2]*v2[1];
			float vy = v1[2]*v2[0] - v1[0]*v2[2];
			float vz = v1[0]*v2[1] - v1[1]*v2[0];
			float length = sqrt(vx*vx + vy*vy + vz*vz);

			float v12[3] = {terrain[i][j-1][0] - terrain[i+1][j-1][0], terrain[i][j-1][1] - terrain[i+1][j-1][1], terrain[i][j-1][2] - terrain[i+1][j-1][2]};
			float v22[3] = {terrain[i+1][j][0] - terrain[i+1][j-1][0], terrain[i+1][j][1] - terrain[i+1][j-1][1], terrain[i+1][j][2] - terrain[i+1][j-1][2]};
			float vx2 = v12[1]*v22[2] - v12[2]*v22[1];
			float vy2 = v12[2]*v22[0] - v12[0]*v22[2];
			float vz2 = v12[0]*v22[1] - v12[1]*v22[0];
			float length2 = sqrt(vx2*vx2 + vy2*vy2 + vz2*vz2);

			float avgX = ((vx/length)+(vx2/length2))/2;
			float avgY = ((vy/length)+(vy2/length2))/2;
			float avgZ = ((vz/length)+(vz2/length2))/2;
			float normal1[3] = {avgX,avgY,avgZ};

			float v1_3[3] = {terrain[i][j-1][0] - terrain[i][j][0], terrain[i][j-1][1] - terrain[i][j][1], terrain[i][j-1][2] - terrain[i][j][2]};
			float v2_3[3] = {terrain[i+1][j-1][0] - terrain[i][j][0], terrain[i+1][j-1][1] - terrain[i][j][1], terrain[i+1][j-1][2] - terrain[i][j][2]};
			float vx3 = v1_3[1]*v2_3[2] - v1_3[2]*v2_3[1];
			float vy3 = v1_3[2]*v2_3[0] - v1_3[0]*v2_3[2];
			float vz3 = v1_3[0]*v2_3[1] - v1_3[1]*v2_3[0];
			float length3 = sqrt(vx3*vx3 + vy3*vy3 + vz3*vz3);

			float v1_4[3] = {terrain[i+1][j][0] - terrain[i][j][0], terrain[i+1][j][1] - terrain[i][j][1], terrain[i+1][j][2] - terrain[i][j][2]};
			float v2_4[3] = {terrain[i+1][j-1][0] - terrain[i][j][0], terrain[i+1][j-1][1] - terrain[i][j][1], terrain[i+1][j-1][2] - terrain[i][j][2]};
			float vx4 = v1_4[1]*v2_4[2] - v1_4[2]*v2_4[1];
			float vy4 = v1_4[2]*v2_4[0] - v1_4[0]*v2_4[2];
			float vz4 = v1_4[0]*v2_4[1] - v1_4[1]*v2_4[0];
			float length4 = sqrt(vx4*vx4 + vy4*vy4 + vz4*vz4);

			float avgX2 = ((vx/length)+(vx2/length2))/2;
			float avgY2 = ((vy/length)+(vy2/length2))/2;
			float avgZ2 = ((vz/length)+(vz2/length2))/2;
			float normal2[3] = {avgX2,avgY2,avgZ2};


			float v1_5[3] = {terrain[i][j+1][0] - terrain[i][j][0], terrain[i][j+1][1] - terrain[i][j][1], terrain[i][j+1][2] - terrain[i][j][2]};
			float v2_5[3] = {terrain[i+1][j+1][0] - terrain[i][j][0], terrain[i+1][j+1][1] - terrain[i][j][1], terrain[i+1][j+1][2] - terrain[i][j][2]};
			float vx5 = v1_5[1]*v2_5[2] - v1_5[2]*v2_5[1];
			float vy5 = v1_5[2]*v2_5[0] - v1_5[0]*v2_5[2];
			float vz5 = v1_5[0]*v2_5[1] - v1_5[1]*v2_5[0];
			float length5 = sqrt(vx5*vx5 + vy5*vy5 + vz5*vz5);

			float v1_6[3] = {terrain[i+1][j][0] - terrain[i][j][0], terrain[i+1][j][1] - terrain[i][j][1], terrain[i+1][j][2] - terrain[i][j][2]};
			float v2_6[3] = {terrain[i+1][j+1][0] - terrain[i][j][0], terrain[i+1][j+1][1] - terrain[i][j][1], terrain[i+1][j+1][2] - terrain[i][j][2]};
			float vx6 = v1_6[1]*v2_6[2] - v1_6[2]*v2_6[1];
			float vy6 = v1_6[2]*v2_6[0] - v1_6[0]*v2_6[2];
			float vz6 = v1_6[0]*v2_6[1] - v1_6[1]*v2_6[0];
			float length6 = sqrt(vx6*vx6 + vy6*vy6 + vz6*vz6);

			float avgX3 = ((vx5/length5)+(vx6/length6))/2;
			float avgY3 = ((vy5/length5)+(vy6/length6))/2;
			float avgZ3 = ((vz5/length5)+(vz6/length6))/2;
			float normal3[3] = {avgX3,avgY3,avgZ3};

			float v1_7[3] = {terrain[i][j+1][0] - terrain[i][j][0], terrain[i][j+1][1] - terrain[i][j][1], terrain[i][j+1][2] - terrain[i][j][2]};
			float v2_7[3] = {terrain[i+1][j][0] - terrain[i][j][0], terrain[i+1][j][1] - terrain[i][j][1], terrain[i+1][j][2] - terrain[i][j][2]};
			float vx7 = v1_7[1]*v2_7[2] - v1_7[2]*v2_7[1];
			float vy7 = v1_7[2]*v2_7[0] - v1_7[0]*v2_7[2];
			float vz7 = v1_7[0]*v2_7[1] - v1_7[1]*v2_7[0];
			float length7 = sqrt(vx7*vx7 + vy7*vy7 + vz7*vz7);

			float v1_8[3] = {terrain[i+1][j][0] - terrain[i+1][j+1][0], terrain[i+1][j][1] - terrain[i+1][j+1][1], terrain[i+1][j][2] - terrain[i+1][j+1][2]};
			float v2_8[3] = {terrain[i][j+1][0] - terrain[i+1][j+1][0], terrain[i][j+1][1] - terrain[i+1][j+1][1], terrain[i][j+1][2] - terrain[i+1][j+1][2]};
			float vx8 = v1_8[1]*v2_8[2] - v1_8[2]*v2_8[1];
			float vy8 = v1_8[2]*v2_8[0] - v1_8[0]*v2_8[2];
			float vz8 = v1_8[0]*v2_8[1] - v1_8[1]*v2_8[0];
			float length8 = sqrt(vx8*vx8 + vy8*vy8 + vz8*vz8);

			float avgX4 = ((vx7/length7)+(vx8/length8))/2;
			float avgY4 = ((vy7/length7)+(vy8/length8))/2;
			float avgZ4 = ((vz7/length7)+(vz8/length8))/2;
			float normal4[3] = {avgX4,avgY4,avgZ4};

			float newx = (avgX + avgX2 + avgX3 + avgX4)/4;
			float newy = (avgY + avgY2 + avgY3 + avgY4)/4;
			float newz = (avgZ + avgZ2 + avgZ3 + avgZ4)/4;

			glNormal3f(newx,newy,newz);
		}
}

//displapy using quads and smooth shading
void displayQuadSmooth(bool wirefill){
	if(wirefill==false){
		glLineWidth(1);
		glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
		glLightfv(GL_LIGHT1, GL_POSITION, light_pos2);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, m_amb);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, m_diff);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, m_spec);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shiny);
	}
	else{
		glLineWidth(3);
		glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
		glLightfv(GL_LIGHT1, GL_POSITION, light_pos2);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, m_amb2);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, m_diff2);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, m_spec2);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shiny2);
	}
	for(int i=0; i<gridx-1; i++){
		for(int j=1; j<gridy; j++){
			if(wirefill==true){
				glBegin(GL_LINE_LOOP);
			}
			else{
				glBegin(GL_QUADS);
			}

			//top left
			float color = (((terrain[i][j-1][1] - (-4))*cRange)/hRange)+0;
			glColor3f(color,color,color);
			if(wirefill==true){
				glColor3f(0,color,color);
			}
			setNormalQuad(i,j-1);
			glVertex3f(terrain[i][j-1][0], terrain[i][j-1][1], terrain[i][j-1][2]);

			//bottom left
			color = (((terrain[i+1][j-1][1] - (-4))*cRange)/hRange)+0;
			glColor3f(color,color,color);
			if(wirefill==true){
				glColor3f(0,color,color);
			}
			setNormalQuad(i+1,j-1);
			glVertex3f(terrain[i+1][j-1][0], terrain[i+1][j-1][1], terrain[i+1][j-1][2]);

			//bottom right
			color = (((terrain[i+1][j][1] - (-4))*cRange)/hRange)+0;
			glColor3f(color,color,color);
			if(wirefill==true){
				glColor3f(0,color,color);
			}
			setNormalQuad(i+1,j);
			glVertex3f(terrain[i+1][j][0], terrain[i+1][j][1], terrain[i+1][j][2]);

			//top right
			color = (((terrain[i][j][1] - (-4))*cRange)/hRange)+0;
			glColor3f(color,color,color);
			if(wirefill==true){
				glColor3f(0,color,color);
			}
			setNormalQuad(i,j);
			glVertex3f(terrain[i][j][0], terrain[i][j][1], terrain[i][j][2]);
			glEnd();

		}
	}
}

/* display function - GLUT display callback function
 *		clears the screen, sets the camera position, draws the ground plane and movable box
 */
void display(void)
{
	float origin[3] = {0,0,0};
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//Set the shade model
	if(smoothON==0){
		glShadeModel(GL_FLAT);
	}
	else{
		glShadeModel(GL_SMOOTH);
	}
	//Set the camera
	gluLookAt(camPos[0], camPos[1], camPos[2], camLook[0], camLook[1], camLook[2], 0,1,0);
	
	//Apply the necessary roations and centre the terrain around the origin
	glTranslatef(-gridx/200, 0, -gridy/200);
	glRotatef(rotate,0,1,0);
	glRotatef(tiltUp,1,0,0);
	//Call the diplay functions depending on the values of smoothON, shapeType, and wire
	if(shapeType == 0){
		if(smoothON==0){
			displayTri(false);
			if(wire==2){
				glPushMatrix();
				displayTri(true);
				glTranslatef(0,1,0);
				glPopMatrix();
			}
		}
		else{
			displayTriSmooth(false);
			if(wire==2){
				glPushMatrix();
				displayTriSmooth(true);
				glTranslatef(0,1,0);
				glPopMatrix();
			}
		}
		
	}
	else{
		if(smoothON==0){
			displayQuad(false);
			if(wire==2){
				glPushMatrix();
				displayQuad(true);
				glTranslatef(0,1,0);
				glPopMatrix();
			}
		}
		else{
			displayQuadSmooth(false);
			if(wire==2){
				glPushMatrix();
				displayQuadSmooth(true);
				glTranslatef(0,1,0);
				glPopMatrix();
			}
		}		
	}
	//Draw 2 spheres that move back and forth across the terrain
	glLineWidth(1);
	glColor3f(0,0,1);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, m_amb3);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, m_diff3);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, m_spec3);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shiny3);

	glPushMatrix();
		glTranslatef(inc_x*charPos,terrain[10][charPos][1] + 0.1,0);
		glTranslatef(-gridx*inc_x/2,0,inc_y*10-gridy*inc_y/2);
		glutSolidSphere(0.25,10,10);
		if(wire==2){
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, m_amb2);
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, m_diff2);
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, m_spec2);
			glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shiny2);
			glColor3f(1,0,0);
			glutWireSphere(0.26,10,10);
		}
	glPopMatrix();
	charPos += charDir;
	if(charPos==gridx-5){
		charDir = -1;
	}
	if(charPos==0){
		charDir = 1;
	}

	glColor3f(0,0,1);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, m_amb3);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, m_diff3);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, m_spec3);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shiny3);
	glPushMatrix();
		glTranslatef(inc_x*charPos2,terrain[40][charPos2][1] + 0.1,0);
		glTranslatef(-gridx*inc_x/2,0,inc_y*40-gridy*inc_y/2);
		glutSolidSphere(0.25,10,10);
		if(wire==2){
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, m_amb2);
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, m_diff2);
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, m_spec2);
			glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shiny2);
			glColor3f(1,0,0);
			glutWireSphere(0.26,10,10);
		}
	glPopMatrix();
	charPos2 += charDir2;
	if(charPos2==gridx-5){
		charDir2 = -1;
	}
	if(charPos2==0){
		charDir2 = 1;
	}
	
	glutSwapBuffers();
}

//Display function for the second window
void display_2(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(0, 0, 3.42f, 0, 0, 0, 0,1,0);
	glPointSize(2);
	
	//Display the height map
	makeHeightMap();

	glutSwapBuffers();
}

//init functio for the second window
void init2(void)
{
	glClearColor(0, 0.1, 0.1, 1);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, 1, 1, 100);
}

/* main function - program entry point */
int main(int argc, char** argv)
{
	cout<<"Enter terrain width (50-300): ";
	cin>>gridx;
	while(gridx < 50 || gridx > 300){
		cout<<"Enter terrain width (50-300): ";
		cin>>gridx;
	}
	cout<<"Enter terrain length (50-300): ";
	cin>>gridy;
	while(gridy < 50 || gridy > 300){
		cout<<"Enter terrain length (50-300): ";
		cin>>gridy;
	}
	inc_x = (50.0/gridx)*0.3;
	inc_y = (50.0/gridy)*0.3;
	makeTerrain();
	cout<<"Controls\n"<<"Q or Esc: exit program\n";
	cout<<"A: toggle between circle and fault algorithm\n";
	cout<<"L: toggle lighting on and off\n";
	cout<<"T: draw terrain using triangles\n";
	cout<<"Y: draw terrain using quads\n";
	cout<<"W: toggle wireframe representations\n";
	cout<<"Z: toggle shading type\n";
	cout<<"R: reset terrain\n";
	cout<<"1: allows movement of the terrain using the arrow keys\n";
	cout<<"2: allows movement of light source 1 using the arrow keys\n";
	cout<<"3: allows movement of light source 2 using the arrow keys\n";
	glutInit(&argc, argv);		//starts up GLUT
	
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	
	//Create the first window
	glutInitWindowSize(800, 800);
	glutInitWindowPosition(200, 100);

	window1 = glutCreateWindow(argv[0]);
	glutSetWindowTitle("Terrain");

	glutDisplayFunc(display);	//registers "display" as the display callback function
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	callBackInit();

	glEnable(GL_DEPTH_TEST);
	init();

	//Create the second window
	glutInitWindowSize(gridx*2, gridy*2);
	glutInitWindowPosition(0, 0);
	window2 = glutCreateWindow(argv[0]);
	glutSetWindowTitle("Heightmap");
	glutDisplayFunc(display_2);
	init2();
	
	glutSetWindow(window1);

	glutMainLoop();				//starts the event loop

	return(0);					//return may not be necessary on all compilers
}