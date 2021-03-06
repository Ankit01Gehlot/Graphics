#define STB_IMAGE_IMPLEMENTATION

#include <GL/glut.h>
#include "../include/variables.h"
#include "../include/vec3f.h"
#include "../include/renderer.h"
#include "../include/terrain.h"
#include "../include/imageloader.h"
#include "../include/helper_functions.h"
#include "../include/objects.h"

extern Terrain* _terrain;
float limit=89.0*M_PI/180.0f;
float yview = zNear*FSCALE*tan(fov/2*M_PI/180.0);

float gtx=0.0,gty=0.0,gtz=0.0;

void lighting() {
	GLfloat ambientColor[] = {0.4f, 0.4f, 0.4f, 1.0f};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientColor);
	
	GLfloat lightColor0[] = {0.6f, 0.6f, 0.6f, 1.0f};
	GLfloat lightPos0[] = {-0.5f, 0.8f, 0.1f, 0.0f};
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor0);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);
}

float precam_x=cam_x,precam_y=cam_y,precam_z=cam_z;

void collisionDetection(){

	float fac = 3.0f/5;
	float tcam_x = cam_x * fac;
	float tcam_z = cam_z * fac;


	for(auto val: building_coord){
		if( cam_y<100 && tcam_x>=val[0] && tcam_x<=val[0]+val[2] && tcam_z>=val[1] && tcam_z<=val[1]+val[3]) {
			cam_x = precam_x;
			cam_y = precam_y;
			cam_z = precam_z;
		}
	}
	if(cam_y<-30)
		cam_y=-30;
	if(cam_y>500)
		cam_y=500;
	if(cam_x<10)
		cam_x=10;
	if(cam_x>490)
		cam_x=490;
	if(cam_z<10)
		cam_z=10;
	if(cam_z>490)
		cam_z=490;
}

void update_local_vars() {
	if(time_hr > 4 && time_hr <= 11) {
		lighting();
		tod = TimeOfDay::Morning;
	} 
	else if (time_hr > 11 && time_hr <= 17) {
		lighting();
		tod = TimeOfDay::Afternoon;
	} 
	else if (time_hr > 17 && time_hr <= 19) {
		lighting();
		tod = TimeOfDay::Evening;
	} 
	else if ((time_hr > 19 && time_hr<=23) || ((time_hr >=0  && time_hr<=4))){
		lighting();
		tod = TimeOfDay::Night;
	} 
}

string formatTime(){
	string ts;
	if(time_hr<10){
		ts="TIME : 0"+to_string(time_hr)+":00 HRS";
	}
	else{
		ts="TIME : "+to_string(time_hr)+":00 HRS";
	}
	return ts;
}

string ControlStr[]={
	"W: MOVE FORWARD",
	"S: MOVE BACKWARD",
	"A: MOVE LEFT",
	"D: MOVE RIGHT",
	"I: MOVE UP",
	"J: MOVE DOWN",
	"T: INC TIME",
	"SHIFT+T: DEC TIME",
	"SPACE: TOGGLE MOUSE CONTROLS",
	"C: TOGGLE INSTRUCTIONS",
	"F: TOGGLE FULL SCREEN",
	"ESC: EXIT"
};

void renderText(void *font,string str,float txt_xoff,float txt_yoff,float rc,float gc,float bc){
	txt_xoff=txt_xoff/sheight*yview;
	txt_yoff=txt_yoff/sheight*yview;

	float xoff=cam_x+(zNear+0.0001)*FSCALE*lx+txt_xoff*cos(yaw)-txt_yoff*sin(pitch)*sin(yaw);
	float yoff=cam_y+(zNear+0.0001)*FSCALE*ly+txt_yoff*cos(pitch);
	float zoff=cam_z+(zNear+0.0001)*FSCALE*lz+txt_xoff*sin(yaw)+txt_yoff*sin(pitch)*cos(yaw);
	
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
	glDisable(GL_TEXTURE_2D);
	
	glColor3f(rc,gc,bc);
	glRasterPos3f(xoff,yoff,zoff);
	for (int i=0; i<str.length(); i++) {
		glutBitmapCharacter(font,str[i]);
	}
	
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_TEXTURE_2D);
}

void keyboard(unsigned char key,int x,int y){
	switch (key) {
		case 27 :
			exit(0);
		case ' ':
			mode=!mode;
		case 'w' :
			cam_x += lx * speed ;
			cam_y += ly * speed ;
			cam_z += lz * speed ;
			break;
		case 's' :
			cam_x -= lx * speed ;
			cam_y -= ly * speed ;
			cam_z -= lz * speed ;
			break;
		case 'a' :
			cam_x += cos(pitch) * sin(yaw - M_PI_2) * speed ;
			cam_z += -cos(pitch) * cos(yaw - M_PI_2) * speed ;
			break;
		case 'd' :
			cam_x += cos(pitch) * sin(yaw + M_PI_2) * speed ;
			cam_z += -cos(pitch) * cos(yaw + M_PI_2) * speed ;
			break;
		case 'i' :
			cam_y += speed ;
			break;
		case 'j': 
			cam_y -= speed ;
			break;
		case 'f':
			if(!full_screen){
				glutFullScreen();
				full_screen=1;
			}
			else{
				glutReshapeWindow(800,600);
				full_screen=0;
			}
			break;
		case 't':
			time_hr=(time_hr+1)%24;
			break;
		case 'T':
			time_hr=(time_hr+23)%24;
			break;
		case 'c':
			controls=!controls;
	}
	glutPostRedisplay();
}

void mouse(int mx,int my){
	if(mode){
		if(warped){
			warped=false;
			return;
		}
		pitch += (200-my)*rot_speed;
		if(pitch>=limit) pitch=limit;
		else if(pitch<=-limit) pitch= -limit;
		yaw += (mx-200)*rot_speed;
		glutWarpPointer(200,200);
		lx = cos(pitch)*sin(yaw);
		ly = sin(pitch);
		lz = -cos(pitch)*cos(yaw);
		warped=true;
		glutPostRedisplay();
	}
}

void render_points(Vec3f normal,int x,int z) {
	glNormal3f(FSCALE * (X_OFF +  normal[0]), FSCALE * (Y_OFF +  normal[1]), FSCALE * (Z_OFF +  normal[2]));
	glTexCoord2f((float)(x)/300,(float)(z)/300);
	glVertex3f(FSCALE * (X_OFF +  x), FSCALE * (Y_OFF +  _terrain->get_height(x, z)), FSCALE * (Z_OFF +  z));

	normal = _terrain->get_normal(x, z + 1);
	glNormal3f((X_OFF +  normal[0]), Y_OFF +  normal[1], Z_OFF +  normal[2]);
	glTexCoord2f((float)(x)/300,(float)(z+1)/300);
	glVertex3f(FSCALE * (X_OFF +  x), FSCALE * (Y_OFF +  _terrain->get_height(x, z + 1)), FSCALE * (Z_OFF +  (z + 1)));
}

void render_terrain(GLuint ground_texture) {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, ground_texture);
	
	//Bottom
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	
	glColor3f(1.0f, 1.0f, 1.0f);
	for(int z = 0; z < _terrain->length() - 1; z++) {
		glBegin(GL_TRIANGLE_STRIP);
		for(int x = 0; x < _terrain->width(); x++) {
			Vec3f normal = _terrain->get_normal(x, z);
			render_points(normal,x,z);
		}
		glEnd();
	}
}

Vec3f ver[8] = 
{
    {-100.0,-100.0,100.0},
    {-100.0,100.0,100.0},
    {100.0,100.0,100.0},
    {100.0,-100.0,100.0},
    {-100.0,-100.0,-100.0},
    {-100.0,100.0,-100.0},
    {100.0,100.0,-100.0},
    {100.0,-100.0,-100.0},
};

void quad(int a,int b,int c,int d, GLuint building_texture, Vec3f offset,vector<Vec3f> vertices, float scale){
	Vec3f goff = Vec3f(0,-25,0);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, building_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);

	glTexCoord2f(0.0f, scale);
	glVertex3fv(((vertices[a] + offset + goff) * FSCALE).to_float());

	glTexCoord2f(scale, scale);
    glVertex3fv(((vertices[b] + offset + goff) * FSCALE).to_float());

	glTexCoord2f(scale, 0.0f);
    glVertex3fv(((vertices[c] + offset + goff) * FSCALE).to_float());

	glTexCoord2f(0.0f, 0.0f);
    glVertex3fv(((vertices[d] + offset + goff) * FSCALE).to_float());

    glEnd();
}

void render_building(GLuint building_texture, Vec3f offset, vector<Vec3f> vertices) {
	// the coordinates ordering must be clockwise.
    quad(2,7,4,3,build_top,offset,vertices, 1.0f); //top
    quad(1,6,5,0,building_texture,offset,vertices, 1.0f); //bottom
    quad(4,7,6,5,building_texture,offset,vertices, 1.0f); //front
    quad(0,1,2,3,building_texture,offset,vertices, 1.0f); //back
    quad(3,4,5,0,building_texture,offset,vertices, 1.0f); //left
    quad(7,2,1,6,building_texture,offset,vertices, 1.0f); //right
}


void render_sky_helper(GLuint t1,GLuint t2,GLuint t3,GLuint t4,GLuint t5,GLuint t6, Vec3f offset, vector<Vec3f> vertices, float scale) {
	// the coordinates ordering must be clockwise.
    quad(2,7,4,3,t5,offset,vertices, scale); //top
    quad(1,6,5,0,t6,offset,vertices, scale); //bottom
    
	quad(4,7,6,5,t1,offset,vertices, scale); //front
    quad(3,4,5,0,t2,offset,vertices, scale); //right
    quad(0,1,2,3,t3,offset,vertices, scale); //back
    quad(7,2,1,6,t4,offset,vertices, scale); //left
}

void render_all_buildings() {

	// build6_texture goes good with sm buildings

	// first row
	render_building(build6_texture,Vec3f(12.5,0,12.5), buliding_struct_1_sm);
	render_building(build2_texture,Vec3f(37.5,0,12.5), buliding_struct_3_md);
	render_building(build3_texture,Vec3f(67.625,0,12.5), buliding_struct_1_md);
	render_building(build5_texture,Vec3f(95,0,12.5), buliding_struct_2_md);
	render_building(build1_texture,Vec3f(130,0,12.5), buliding_struct_2_l);
	render_building(build6_texture,Vec3f(162.5,0,12.5), buliding_struct_3_sm);
	render_building(build5_texture,Vec3f(193.75,0,12.5), buliding_struct_3_md);
	render_building(build6_texture,Vec3f(218.75,0,12.5), buliding_struct_1_sm);
	render_building(build4_texture,Vec3f(255,0,12.5), buliding_struct_3_l);

	// last row
	render_building(build6_texture,Vec3f(12.5,0,255), buliding_struct_3_sm);
	render_building(build2_texture,Vec3f(37.5,0,252.4), buliding_struct_1_md);
	render_building(build3_texture,Vec3f(67.625,0,252.4), buliding_struct_2_md);
	render_building(build5_texture,Vec3f(95,0,255), buliding_struct_1_md);
	render_building(build1_texture,Vec3f(130,0,257.4), buliding_struct_3_l);
	render_building(build6_texture,Vec3f(155,0,255), buliding_struct_2_sm);
	render_building(build5_texture,Vec3f(193.75,0,252.4), buliding_struct_1_md);
	render_building(build6_texture,Vec3f(218.75,0,255), buliding_struct_3_sm);
	render_building(build4_texture,Vec3f(255,0,252.4), buliding_struct_3_l);

	render_building(build6_texture,Vec3f(197,0,124), buliding_struct_4_xsm);
	render_building(build6_texture,Vec3f(74,0,190), buliding_struct_4_xsm);

	render_building(build5_texture,Vec3f(154,0,120), buliding_struct_5_sm);
	render_building(build6_texture,Vec3f(197,0,197), buliding_struct_5_sm);
	render_building(build3_texture,Vec3f(84,0,145), buliding_struct_6_sm);


	render_building(build4_texture,Vec3f(198,0,80), buliding_struct_6_md);
	render_building(build6_texture,Vec3f(198,0,55), buliding_struct_6_sm);

	render_building(build6_texture,Vec3f(82,0,55), buliding_struct_6_xsm);

	render_building(build1_texture,Vec3f(69,0,82), buliding_struct_1_sm);
	render_building(build2_texture,Vec3f(69,0,119), buliding_struct_1_md);
	render_building(build3_texture,Vec3f(129,0,142), buliding_struct_1_sm);

	render_building(build6_texture,Vec3f(220,0,177), buliding_struct_1_sm);
	render_building(build4_texture,Vec3f(133,0,190), buliding_struct_4_md);

	render_building(build6_texture,Vec3f(129,0,74), buliding_struct_5_xsm);

	render_building(build3_texture,Vec3f(148,0,51), buliding_struct_6_sm);

}

void render_sky() {
	switch (tod) {
		case TimeOfDay::Night : {
			render_sky_helper(sky_night1,sky_night2,sky_night3,sky_night4,sky_night1,sky_night1,Vec3f(-100,-100,-100),sky_coord,2.0f);
			break;
		}
		case TimeOfDay::Afternoon : {
			render_sky_helper(sky_afternoon1,sky_afternoon2,sky_afternoon3,sky_afternoon4,sky_afternoon1,sky_afternoon1,Vec3f(-100,-100,-100),sky_coord,1.0f);
			break;
		}
		case TimeOfDay::Morning : {
			render_sky_helper(sky_morning1,sky_morning2,sky_morning3,sky_morning4,sky_morning1,sky_morning1,Vec3f(-100,-100,-100),sky_coord,1.0f);
			break;
		}
		case TimeOfDay::Evening : {
			render_sky_helper(sky_evening1,sky_evening2,sky_evening3,sky_evening4,sky_evening1,sky_evening1,Vec3f(-100,-100,-100),sky_coord,1.0f);
			break;
		}
	}
}

void render_all_lamps() {

	draw_lamp(10*FSCALE,242,2);
	draw_lamp(10*FSCALE,260,2);
	draw_lamp(10*FSCALE,190,2 );
	draw_lamp(10*FSCALE, 180,2);
	draw_lamp(10*FSCALE, 180 , 40);
	draw_lamp(10*FSCALE, 180 , 52);
	draw_lamp(10*FSCALE, 190 , 52);
	draw_lamp(10*FSCALE, 128 , 52);
	draw_lamp(10*FSCALE, 116 , 52);
	draw_lamp(10*FSCALE, 116 , 38);
	draw_lamp(10*FSCALE, 128 , 38);
	draw_lamp(10*FSCALE, 68 , 38);
	draw_lamp(10*FSCALE, 68 , 52);
	draw_lamp(10*FSCALE, 54 , 52);
	draw_lamp(10*FSCALE, 54 , 102);
	draw_lamp(10*FSCALE, 66 , 102);
	draw_lamp(10*FSCALE, 66 , 116);
	draw_lamp(10*FSCALE, 56 , 116);

	draw_lamp(10*FSCALE, 44 , 4);
	draw_lamp(10*FSCALE, 74 , 4 );
	draw_lamp(10*FSCALE, 112 , 4);
	draw_lamp(10*FSCALE, 132 , 4);
	draw_lamp(10*FSCALE, 192 , 100);
	draw_lamp(10*FSCALE, 180 , 100);
	draw_lamp(10*FSCALE, 180 , 120);
	draw_lamp(10*FSCALE, 190 , 120);
	draw_lamp(10*FSCALE, 114 , 116);
	draw_lamp(10*FSCALE, 128 , 116);
	draw_lamp(10*FSCALE, 128 , 104);
	draw_lamp(10*FSCALE, 118 , 104);
	draw_lamp(10*FSCALE, 240 , 104);
	draw_lamp(10*FSCALE, 254 , 104);
	draw_lamp(10*FSCALE, 254 , 116);
	draw_lamp(10*FSCALE, 242 , 116);
	draw_lamp(10*FSCALE, 242 , 164);
	draw_lamp(10*FSCALE, 254 , 164);
	draw_lamp(10*FSCALE, 254 , 176);
	draw_lamp(10*FSCALE, 242 , 176);
	draw_lamp(10*FSCALE, 242 , 164 );
	draw_lamp(10*FSCALE, 52 , 166);
	draw_lamp(10*FSCALE, 52 , 176);
	draw_lamp(10*FSCALE, 66 , 176);
	draw_lamp(10*FSCALE, 66 , 166);
	draw_lamp(10*FSCALE, 118 , 166);
	draw_lamp(10*FSCALE, 118 , 176);
	draw_lamp(10*FSCALE, 128 , 176);
	draw_lamp(10*FSCALE, 128 , 166);
	draw_lamp(10*FSCALE, 128 , 176);
	draw_lamp(10*FSCALE, 178 , 176);
	draw_lamp(10*FSCALE, 178 , 166);
	draw_lamp(10*FSCALE, 190 , 166);
	draw_lamp(10*FSCALE, 190 , 176);
	draw_lamp(10*FSCALE, 242 , 236 );
	draw_lamp(10*FSCALE, 254 , 236);
	draw_lamp(10*FSCALE, 254 , 246);
	draw_lamp(10*FSCALE, 242 , 246);
	draw_lamp(10*FSCALE, 190 , 246);
	draw_lamp(10*FSCALE, 180 , 246);
	draw_lamp(10*FSCALE, 180 , 234);
	draw_lamp(10*FSCALE, 190 , 234);
	draw_lamp(10*FSCALE, 116 , 236);
	draw_lamp(10*FSCALE, 130 , 236 );
	draw_lamp(10*FSCALE, 130 , 246);
	draw_lamp(10*FSCALE, 118 , 246);
	draw_lamp(10*FSCALE, 66 , 246 );
	draw_lamp(10*FSCALE, 56 , 246);
	draw_lamp(10*FSCALE, 54 , 236);
	draw_lamp(10*FSCALE, 66 , 236 );


	draw_lamp(10*FSCALE,gtx,gty);

}

void drawScene(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	update_local_vars();
	collisionDetection();
	gluLookAt(cam_x,cam_y,cam_z,cam_x+lx,cam_y+ly,cam_z+lz,0.0f,1.0f,0.0f);

	// cout<<cam_x<<' '<<cam_z<<endl;
	// cout<<precam_x<<" "<<precam_z<<"\n\n";
	precam_x=cam_x;
	precam_y=cam_y;
	precam_z=cam_z;
	
	renderText(GLUT_BITMAP_TIMES_ROMAN_24,formatTime(),-swidth+50.0,sheight-80.0,1.0,0.0,0.0);
	
	if(controls){
		for(int i=0;i<12;i++){
			renderText(GLUT_BITMAP_HELVETICA_18,ControlStr[i],-swidth+50.0,sheight-140.0-60.0*i,1.0,1.0,1.0);
		}
	}

	float scale = 5.0f / max(_terrain->width() - 1, _terrain->length() - 1);
	glScalef(scale, scale, scale);
	
	render_terrain(ground_texture);
	
	render_all_buildings();
	render_sky();
	render_all_lamps();


	glutSwapBuffers();

}

void handleResize(int w, int h){
    glViewport(0, 0, w, h);
	swidth=w;
	sheight=h;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fov, (double)w / (double)h, zNear*FSCALE, zFar * FSCALE);
}

void load_image_resources() {

	ground_texture = loadTx("./resources/terrain_texture.bmp");

	build1_texture = loadTx("./resources/buildings/build1.bmp");
	build2_texture = loadTx("./resources/buildings/build2.bmp");
	build3_texture = loadTx("./resources/buildings/build3.bmp");
	build4_texture = loadTx("./resources/buildings/build4.bmp");
	build5_texture = loadTx("./resources/buildings/build5.bmp");
	build6_texture = loadTx("./resources/buildings/build6.bmp");
	build_top = loadTx("./resources/buildings/build_top.bmp");

	sky_afternoon1 = loadTx("./resources/sky/afternoon/1.bmp");
	sky_afternoon2 = loadTx("./resources/sky/afternoon/2.bmp");
	sky_afternoon3 = loadTx("./resources/sky/afternoon/3.bmp");
	sky_afternoon4 = loadTx("./resources/sky/afternoon/4.bmp");

	sky_evening1 = loadTx("./resources/sky/evening/1.bmp");
	sky_evening2 = loadTx("./resources/sky/evening/2.bmp");
	sky_evening3 = loadTx("./resources/sky/evening/3.bmp");
	sky_evening4 = loadTx("./resources/sky/evening/4.bmp");

	sky_morning1 = loadTx("./resources/sky/morning/1.bmp");
	sky_morning2 = loadTx("./resources/sky/morning/2.bmp");
	sky_morning3 = loadTx("./resources/sky/morning/3.bmp");
	sky_morning4 = loadTx("./resources/sky/morning/4.bmp");

	sky_night1 = loadTx("./resources/sky/night.bmp");
	sky_night2 = sky_night3 = sky_night4 = sky_night1;
}

void initRendering(){
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);
	load_image_resources();
}