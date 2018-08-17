//-----------------------------------------------------------------------------
//           Name: ldpc-ogl.cpp
//         Author: Alfredo A. Ortega
//    Description: Implementacion de visualizador LDPC usando OpenGL
//-----------------------------------------------------------------------------
//
#include <stdlib.h>
#include <stdio.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include <vector>
#include <list>
#include <string>
#include <cmath>
#include "tga.h"

#define NDEBUG
#define BOOST_UBLAS_NDEBUG
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>


#define DEBUG  2

using namespace boost::numeric::ublas;




/* prng usando rand() */
void lfsr(unsigned short int *reg)	{
	*reg=rand();
	}

/* Devuelve la cantidad de errores de paridad del vector cw segun la matriz de paridad H*/
int checkParity(vector<int> cw, matrix<int> H)	{
	unsigned int errors = 0,q;
   	vector<int> vc;
    	vc = prod(H,cw);
    	for (q=0;q<vc.size();q++) vc(q)=vc(q) % 2;
    	for (q=0;q<vc.size();q++) errors+=vc(q);
	return errors;
}

// LDPC decode
// cw: received codeword 
// H: parity-check matrix
//
int rounds=0;
vector<int> zerocount(2000),onecount(2000);
matrix<int> response(2000,2000); //response matrix
vector<int> vnode(2000);
vector<int> decode(vector<int> cw, matrix<int> H) {
    int i,q,r;
    int numVnodes=H.size2(); // == dataLength
    int numFnodes=H.size1();
    int MAXROUNDS=10;
    H=trans(H);
    if (rounds==0)
	    for(i=0;i<numVnodes;i++) {
			zerocount[i]=0;onecount[i]=0;
			}
    if (rounds<MAXROUNDS)	{
	r=rounds;
	for(i=0;i<numVnodes;i++) {
		if (!r) vnode[i]=cw[i]; //first round
		else	{ // sum
			if (cw[i]==0) continue; // assymetric
			int nzeros = 0,nones=0;
			if (vnode[i]) nones=1; else nzeros=1; // count the original value
			for(q=0;q<numFnodes;q++)
				if (H(i,q)) {// Link between f-node and v-node
					if (response(i,q)) nones++;
					else nzeros++;
					}
			//decision, is one or zero?
			if (DEBUG>5) std::cerr<<" 0:"<<nzeros<<" 1:"<<nones<<"   ";
			//zerocount[i]+=nzeros;
			//onecount[i]+=nones;
			//nzeros=zerocount[i];
			//nones=onecount[i];
			if (nzeros>nones) vnode[i]=0;
			else	if (nzeros<nones) vnode[i]=1;
			else	if (r>3) vnode[i]=1-vnode[i];
			//if (!checkParity(vnode,trans(H))) break; // all parity checks OK
			}
		}
	if (DEBUG>5) std::cerr<<std::endl;
	int parityerrors=0,parity=0;
	for (int f=0;f<numFnodes;f++) {
		for(i=0;i<numVnodes;i++)	{
			if (  H(i,f)  ) {// Link between f-node and v-node
				parity=0;
				for (int j=0;j<numVnodes;j++)	{
					if (j==i) continue; //ignore current vnode
					if (H(j,f)) // Link between f-node and v-node
						parity+=vnode[j];
					}
				parity%=2; // Response is the value that we 
					  // need for the sum to be zero
				response(i,f)=parity;
				if (cw[i]==0) response(i,f)=0; //assymetric
				if (response(i,f) != vnode[i]) parityerrors++;
				}
			}
		}
        if (DEBUG>1) std::cerr<<"Round: "<<r<<" parityerrors: "<<parityerrors<<std::endl;
    	rounds++;
	}
return vnode;
};

void usage(void)	{
	std::cerr<<"Usage: ldpcenc <matrix> (takes input from stdin and outputs to stdout)"<<std::endl;
	exit(1);
}


// LDPC simulator
  matrix<int> H; //parity check
  matrix<int> G;

void readMatrixes(char *file)	{
   std::ifstream thematrix;
   thematrix.open (file);
   // Cargamos H (Parity Check Matrix)
   thematrix>>H;
   // Cargamos G (Generator Matrix)
   thematrix>>G;
   thematrix.close();
   if (DEBUG)
   	std::cerr<<"Loaded "<<file<<std::endl;
}




//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------

// Id de ventana OpenGL
int    g_window    = 0;
// Id de textura (Solamente tenemos una)
GLuint g_textureID = 0;

// Estas variables globales almacenan datos de traslacion, zoom y transparencia actual
// Perdon por usar variables globales...
//
//
double g_fSpinX           = 0.0f;
double g_fSpinY           = 0.0f;
double g_zoom             = 0.1f;
double g_alpha            = 1.0f;
int   g_nLastMousePositX = 0;
int   g_nLastMousePositY = 0;

// Flags para el sistema de picking
int isSelect=0;
int pickX,pickY;

/* picking tolerance in pixels:     */
#define PICK_TOL              5
/* how big to make the pick buffer: */
#define PICK_BUFFER_SIZE      256

/* picking buffer para el sistema de picking de OpenGL*/
unsigned int PickBuffer[PICK_BUFFER_SIZE]; 


//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int main(int argc, char **argv);
void init(void);
void loadTexture(void);
bool LoadTexture(char *TexName, GLuint TexHandle);
void keyboardFunc(unsigned char key, int x, int y);
void mouseFunc(int button, int state, int x, int y);
void motionFunc(int x, int y);
void idleFunc(void);
void reshapeFunc(int w, int h);
void displayFunc(void);

vector<int> cw;//(codewordLenght);

void genCodeWord()	{
   unsigned int dataLenght = G.size1();
   unsigned int codewordLenght = G.size2();
   vector<int> v(dataLenght);
   matrix<int> Gt = trans(G);
   unsigned int i,q;
    printf(" Generating test vector:  \n");
  	for (q=0;q<dataLenght;q++)	
		v(q)= (rand() % 2);
	// CodeWord = Gt*v
	cw = prod(Gt,v);
      	for (q=0;q<cw.size();q++) {
		cw(q)=cw(q) % 2; 
		std::cout<<(cw(q)?'1':'0');
		}
	// ruido
      	for (q=0;q<cw.size();q++) 
		if ((rand() % 10) == 1) cw(q)=1; 
	// Limpiamos variables de decodificacion
	for(i=0;i<codewordLenght;i++) 
		vnode[i]=0;
   	for(q=0;q<dataLenght;q++)	{
   		for(i=0;i<codewordLenght;i++) {	
			response(i,q)=0;			
			}
		}
    rounds=0;
    printf("\nLenght: %d\n" , cw.size());


}

//-----------------------------------------------------------------------------
// Name: main()
// Desc: Funcion principal. Muestra parametros del uso, carga el grafo desde la
// entrada estandard, setupea GLUT y llama al Main Loop de la misma.
//-----------------------------------------------------------------------------
int sMaxKCore;
int sMaxNode;
int sDrawLines=0;
int sNLines = 10;
int main( int argc, char **argv )
{
 
   srand(0);
   if (argc<2)	usage();
   readMatrixes(argv[1]);

    printf("C++ OpenGL version 1.0, programmed by Alfredo Ortega ortegaalfredo@gmail.com\n");
    printf("Usage:\n");
    printf(" A,Z: Change the number of links visualized \n");
    printf(" S,X: Change the transparency of the links \n");
    printf(" Space : Switch between Node and Link visualizations \n");
    printf(" Mouse drag: Move the display X and Y \n");
    printf(" Mouse click: Select node (in Node visualization only) \n");
    printf(" Mouse wheel: Zoom In/Out  \n");

		// Crea vector v()
    genCodeWord();

    glutInit( &argc, argv );
    init();
    glutSwapBuffers();
    glutMainLoop();
    return 0;
}


//-----------------------------------------------------------------------------
// Name: init()
// Desc: Inicializamos parametros de OpenGL, y seteamos las funciones callback de GLUT
//-----------------------------------------------------------------------------

void init( void )
{
	glutInitDisplayMode( GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE );
	glutInitWindowSize( 800, 600 );
	g_window = glutCreateWindow( "LDPC simulator" );

	glutDisplayFunc( displayFunc );
	glutKeyboardFunc( keyboardFunc );
	glutMouseFunc( mouseFunc );
	glutMotionFunc( motionFunc );
	glutReshapeFunc( reshapeFunc );
	glutIdleFunc( idleFunc );

	glGenTextures( 1, &g_textureID );
	if (!LoadTexture("particle.tga", g_textureID ))
		printf("Can't load texture 'particle.tga'.\n");
	else    printf("Texture loaded.\n");

	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

	//glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glAlphaFunc(GL_GREATER,0);

        glEnable( GL_LINE_SMOOTH );
	glHint(GL_POINT_SMOOTH_HINT,GL_NICEST);		// Really Nice Point Smoothing

	// Setupeamos el picking buffer, que almacenara todas las primitivas bajo el cursor 
	glSelectBuffer( PICK_BUFFER_SIZE, PickBuffer );

}

//-----------------------------------------------------------------------------
// Name: loadTexture()
// Carga una textura TGA
//-----------------------------------------------------------------------------

bool LoadTexture(char *TexName, GLuint TexHandle)
 {
  TGAImg Img;        // Image loader
 
  // Load our Texture
   if(Img.Load(TexName)!=IMG_OK)
    return false;
 
  glBindTexture(GL_TEXTURE_2D,TexHandle); // Set our Tex handle as current
  // Create the texture
   if(Img.GetBPP()==24)
    glTexImage2D(GL_TEXTURE_2D,0,3,Img.GetWidth(),Img.GetHeight(),0,
                 GL_RGB,GL_UNSIGNED_BYTE,Img.GetImg());
   else if(Img.GetBPP()==32)
    glTexImage2D(GL_TEXTURE_2D,0,4,Img.GetWidth(),Img.GetHeight(),0,
                 GL_RGBA,GL_UNSIGNED_BYTE,Img.GetImg());
   else
    return false;
 
  // Specify filtering and edge actions
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
 
  return true;
 }

//-----------------------------------------------------------------------------
// Name: keyboardFunc()
// Desc: Called when a keyboard key is pressed
//-----------------------------------------------------------------------------
void keyboardFunc( unsigned char key, int x, int y ) 
{
    switch( key )
    {
	// ESC: Salir.
        case 27:
		glutDestroyWindow( g_window );
        	exit(0);
			break;
	// avanza un paso en decodificacion LDPC
        case 32:  decode(cw,H);break; 
	// genera un nuevo codeword
        case 13:  genCodeWord();break; 
	// Aumenta o disminuye links mostrados
        case 'a': sNLines++;printf("Line limit: %d\n",sNLines);break; 
        case 'z': sNLines--;printf("Line limit: %d\n",sNLines);break; 
	// Aumenta o disminuye transparencia de los links
        case 's': g_alpha*=1.1;printf("Alpha: %f\n",g_alpha);break; 
        case 'x': g_alpha/=1.1;printf("Alpha: %f\n",g_alpha);break;
    }
}

//-----------------------------------------------------------------------------
// Name: mouseFunc()
// Desc: Called when a mouse button is pressed
//-----------------------------------------------------------------------------
void mouseFunc( int button, int state, int x, int y )
{
	if (button == 3)  // mouse wheel up
		g_zoom *=1.1;

	if (button == 4)  // mouse wheel up
		g_zoom /=1.1;

	if( button == GLUT_LEFT_BUTTON && state == GLUT_DOWN )
	{
		g_nLastMousePositX = x;
        	g_nLastMousePositY = y;
	}
	if( button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN )
	{

	// Codigo de seleccion de un objeto OpenGL:
	// Ponemos el sistema OpenGL en modo de seleccion
	glRenderMode(GL_SELECT);
	// Redibujamos pantalla
	isSelect=1;
	pickX=x;pickY=y;
	displayFunc();
	glFlush();
	isSelect=0;
	int nhits = glRenderMode(GL_RENDER);
	// Recorremos todas las primitivas OpenGL para ver cuales fueron seleccionadas
	for (int i=0,index=0;i<nhits;i++) {
		int nitems = PickBuffer[index++];
		index++;index++;
		for (int j=0;j<nitems;j++) {
			//int item = PickBuffer[index++];
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Name: motionFunc()
// Desc: Called when the mouse moves
//-----------------------------------------------------------------------------
void motionFunc( int x, int y )
{
	g_fSpinX -= (x - g_nLastMousePositX) / g_zoom / 1.5;
	g_fSpinY -= (y - g_nLastMousePositY) / g_zoom / 1.5;

	g_nLastMousePositX = x;
    	g_nLastMousePositY = y;
}

//-----------------------------------------------------------------------------
// Name: idleFunc()
// Desc: Called during idle time
//-----------------------------------------------------------------------------
void idleFunc( void )
{
	glutPostRedisplay();
}

//-----------------------------------------------------------------------------
// Name: reshapeFunc()
// Desc: Called when the window size has been changed by the user
//-----------------------------------------------------------------------------
void reshapeFunc( int w, int h )
{
    
    glMatrixMode( GL_MODELVIEW );
    glViewport( 0, 0, w, h );
}


// Dado el kcore, selecciona el color:
// Esta funcion mapea el numero kCore normalizado (De 0 a 1) a un color en especial.
// Este color se logra con tres funciones trigonometricas, una por cada canal RGB
//
void setColor(float kCore)
{
kCore*=M_PI;
glColor4f(cos(kCore*2),sin(kCore*2),sin(kCore/2),g_alpha);
}

void drawCheck(float y,int pos,int val)	{
	if (!val)
   		glColor4f(0.8,0.8,0.8,1.0);
	else 
   		glColor4f(0.9,0.2,0.2,1.0);
	float psize=0.03;
	float x=pos/5.0;
	glBegin(GL_POLYGON);
	glTexCoord2f(0,0);	
	glVertex2f(x-psize,y-psize);
	glTexCoord2f(1,0);
	glVertex2f(x+psize,y-psize);
	glTexCoord2f(1,1);
	glVertex2f(x+psize,y+psize);
	glTexCoord2f(0,1);
	glVertex2f(x-psize,y+psize);
	glEnd();
	
	}

void drawBit(float y,int pos,int val)	{
	if (!val)
   		glColor4f(1,1,1,0.3);
	else 
   		glColor4f(1,1,1,1.0);
   glEnable( GL_TEXTURE_2D );
	float psize=0.08;
	float x=pos/10.0;
	glBegin(GL_POLYGON);
	glTexCoord2f(0,0);	
	glVertex2f(x-psize,y-psize);
	glTexCoord2f(1,0);
	glVertex2f(x+psize,y-psize);
	glTexCoord2f(1,1);
	glVertex2f(x+psize,y+psize);
	glTexCoord2f(0,1);
	glVertex2f(x-psize,y+psize);
	glEnd();
   glDisable( GL_TEXTURE_2D );
	
	}

void drawLine(float yBit,float yCheck,int posBit,int posCheck,int val)	{
	if (!val)
   		glColor4f(0.8,0.8,0.8,0.1);
	else 
   		glColor4f(1,1,1,0.5);
	float xBit=posBit/10.0;
	float xCheck=posCheck/5.0;
	glBegin(GL_LINES);
		glVertex3f(xBit,yBit,0);
		glVertex3f(xCheck,yCheck,0);
 	glEnd();
	}

void drawMatrix(matrix<int> H)	{
   unsigned int dataLenght = H.size1();
   unsigned int codewordLenght = H.size2();
   unsigned int i,q;
   /* draw codeword */
   glColor4f(0.8,0.8,0.8,1.0);
   glDepthMask(GL_FALSE);
   for(q=0;q<dataLenght;q++)	{
   	for(i=0;i<codewordLenght;i++) {	
		if (H(q,i)) drawLine(0,-40,i,q,response(i,q));
		}
	}
   for(q=0;q<codewordLenght;q++)	{
	// Dibujamos una textura en el lugar del nodo
	drawBit(0,q,vnode(q));
	}
   for(q=0;q<dataLenght;q++)	{
	// Dibujamos los checks
	drawCheck(-40,q,1);
	}

   glDepthMask(GL_TRUE);
}

//-----------------------------------------------------------------------------
// Name: displayFunc()
// Desc: Called when GLUT is ready to render
// Esta es una funcion de pegamento entre el subsistema GLUT y nuestra funcion de
// dibujo drawGraph()
//-----------------------------------------------------------------------------
void displayFunc( void )
{
GLint viewport[4];
glGetIntegerv( GL_VIEWPORT, viewport );

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	if (isSelect) {
		gluPickMatrix(pickX,viewport[3]-pickY,PICK_TOL,PICK_TOL,viewport);
		glInitNames();
		glPushName(0);
		}

	gluPerspective( 45.0f, (float)viewport[2]/(float)viewport[3], 0.01f, 1000.0f);

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glTranslated( g_fSpinX/-100.0, g_fSpinY/100.0, -5.0 / g_zoom );
    drawMatrix(H);
    glutSwapBuffers();
}




