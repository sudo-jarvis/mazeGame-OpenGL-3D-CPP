#include <GL/glut.h>
#include <bits/stdc++.h>
#include <GL/glext.h>
#include <mmsystem.h>

#define WALLS_COLOR 0.0, 0.9, 0.0
#define FLOOR_COLOR 1.0, 1.0, 1.0
#define CEILING_COLOR 1.0, 1.0, 1.0
#define MAX_LINE_SIZE 67
#define MAX_X 33
#define MAX_Y 33
#define COMPONENTS 3

PFNGLACTIVETEXTUREARBPROC       glActiveTexture = nullptr;
PFNGLMULTITEXCOORD2FARBPROC     glMultiTexCoord2f = nullptr;

const double SQUARE_SIZE = 1.0;
const double WALL_HEIGHT = 1.0;
const int POLYS_PER_WALL = 4;
const double PI = 3.1415926;
const int	TURN_NUMBER_OF_STEP = 45;
const int	TURN_TIME = 250;
const float TURN_ANGLE = 90.0;
const float MOVEMENT_SIZE = (float)SQUARE_SIZE;
enum MazeTurnDirection {MZ_LEFT = -1, MZ_RIGHT = 1};
bool topView = false;
float direction[3] = {0, 0, -1};
float position[3] =	{0.f, 0.f, 0.f};
int mazePosition[2] = {static_cast<int>(0.0), static_cast<int>(0.0)};
std::vector<double> vertices(0) ;	// coordinates of all vertices

class Maze {
    static int readXWalls(const char *line, int lineSize, int *wall)
    {
        int i;
        int initialX=-1;
        int pos;
        for(i=0;i<MAX_X;i++) {
            pos= 2 * i;
            if(pos < lineSize) {
                if(line[pos] == '|') wall[i]=1;
                else wall[i]=0;
            }
            else wall[i]=0;
            if(pos + 1 < lineSize)
                if(line[pos + 1] == '*') initialX=i;
        }
        return initialX;
    }

    static void readYWalls(const char *line, int lineSize, int *wall)
    {
        int i, pos;
        for(i=0;i<MAX_X;i++) {
            pos= 2 * i + 1;
            if(pos < lineSize) {
                if(line[pos] == '-') wall[i]=1;
                else wall[i]=0;
            }
            else wall[i]=0;
        }
    }
public:
    int yWall[MAX_Y][MAX_X]{};
    int xWall[MAX_Y][MAX_X]{};
    int initX, initY;
    int maxX, maxY;
    Maze()
    {
        int i, j;
        /* zeroing xWall and yWall */
        for(i=0;i<MAX_Y;i++){
            for(j=0;j<MAX_X;j++) {
                yWall[i][j]=0;
                xWall[i][j]=0;
            }
        }
        maxX=0; maxY=0; initX=0; initY=0;
    }

    void load()
    {
        FILE *file;
        int rYWall[MAX_Y][MAX_X]; // these will store things in reverse y order
        int rXWall[MAX_Y][MAX_X]; // since reading is done starting with the max y
        int lineSize;
        int x,y;
        char line[MAX_LINE_SIZE];
        int i,j;
        file=fopen("maze.txt","r");
        if(file==nullptr) { fprintf(stderr,"error reading maze.txt\n"); exit(1);}
        y=0;
        while(true) {
            // get y walls
            if(fgets(line,MAX_LINE_SIZE,file)==nullptr) break;
            lineSize = (int)strlen(line) - 1;
            readYWalls(line, lineSize, &(rYWall[y][0]));
            // get x walls
            if(fgets(line,MAX_LINE_SIZE,file)==nullptr) break;
            lineSize = (int)strlen(line) - 1;
            x= readXWalls(line, lineSize, &(rXWall[y][0]));
            if(x>=0){ initX=x;initY=y;}
            y++;
        }
        for(i=y+1;i<MAX_Y;i++){
            for(j=0;j<MAX_X;j++) {
                rYWall[i][j]=0;
                rXWall[i][j]=0;
            }
        }
        /* find maximum x and y */
        maxX=0; maxY=0;
        for(i=0;i<MAX_Y;i++)
            for(j=0;j<MAX_X;j++) {
                /* for max y, look for bottom |.  Since this is in
                               order of increasing y, can just update when seen */
                if(rXWall[i][j] == 1) maxY=i;
                if(rYWall[i][j] == 1 && j > maxX) maxX=j;
            }
        /* reverse y order */
        for(i=0; i <= maxY; i++)
            for(j=0;j<MAX_X;j++)
                xWall[maxY - i][j]=rXWall[i][j];
        for(i=0; i <= maxY + 1; i++)
            for(j=0;j<MAX_X;j++)
                yWall[maxY - i + 1][j]=rYWall[i][j];
        initY= maxY - initY;
    }
};
// creating object of class Maze
Maze maze;

class ImageFile
{
private :
    int sizeX{};
    int sizeY{};
    unsigned char * data;
public :
    ImageFile()
    {
        data = nullptr;
    }

    ~ImageFile()
    {
        delete [] data ;
    }

    void createTextureFromPPM(char *filename)
    {
        std::ifstream file;
        char buf[BUFSIZ];
        // load the ppm file
        file.open(filename, std::ios::in);
        if (file.fail())
        {
            printf("File not found");
            return;
        }
        // read image identifier
        file >> buf;
        // read image width and height
        file >> sizeX >> sizeY;
        // read maximum color-component value
        // free space
        delete [] data ;
        data = new unsigned char[sizeX * sizeY * COMPONENTS] ;

        // read pixel colors
        int number;
        for(int i = 0; i < sizeX * sizeY * COMPONENTS; i++)
        {
            file >> number;
            data[i] = number;
        }
        file.close();

        // create the texture
        glPixelStorei(GL_UNPACK_ALIGNMENT,1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        gluBuild2DMipmaps(GL_TEXTURE_2D, COMPONENTS, sizeX, sizeY, GL_RGB, GL_UNSIGNED_BYTE, data);
    }
};

ImageFile square;

void success()
{
    PlaySound("win.wav", nullptr, SND_ASYNC | SND_FILENAME);
    MessageBox(nullptr, "Congratulations, you won!", "Exit from maze", 0);
    exit(0);
}

void init()
{
	//use this color when clearing frame buffer
	glClearColor(1.0, 1.0, 1.0, 1.0);

    // load the maze data from the txt file
    maze.load();

    // assign the initial maze coordinates
    mazePosition[0] = maze.initX;
    mazePosition[1] = maze.initY;

    // turn the initial maze coordinates into world coordinates
    position[0] = (float)(SQUARE_SIZE / 2.0 + maze.initX * SQUARE_SIZE);
    position[1] = (float)(WALL_HEIGHT / 2.0);
    position[2] = (float)(-SQUARE_SIZE / 2.0 - maze.initY * SQUARE_SIZE);

    // Create the array of vertices : 4 per wall

    // x walls
    for(int i = 0; i <= maze.maxY; i++)
    {
        // number of x walls = number of squares + 1 for the last wall
        for(int j = 0; j <= maze.maxX + 1; j++)
        {
            if(maze.xWall[i][j])
            {
                // each wall is divided into several polygons to be able
                // to compute a better lighting
                for(int xPoly = 0; xPoly < POLYS_PER_WALL; xPoly++)
                    for(int yPoly = 0; yPoly < POLYS_PER_WALL; yPoly++)
                    {
                        // 1st vertex
                        vertices.push_back(j * SQUARE_SIZE);
                        vertices.push_back(yPoly * WALL_HEIGHT / POLYS_PER_WALL);
                        vertices.push_back(-i * SQUARE_SIZE - (xPoly * SQUARE_SIZE / POLYS_PER_WALL));
                        // 2nd vertex
                        vertices.push_back(j * SQUARE_SIZE);
                        vertices.push_back((yPoly+1) * WALL_HEIGHT / POLYS_PER_WALL);
                        vertices.push_back(-i * SQUARE_SIZE - (xPoly * SQUARE_SIZE / POLYS_PER_WALL));
                        // 3rd vertex
                        vertices.push_back(j * SQUARE_SIZE);
                        vertices.push_back((yPoly+1) * WALL_HEIGHT / POLYS_PER_WALL);
                        vertices.push_back(-i * SQUARE_SIZE - ((xPoly+1) * SQUARE_SIZE / POLYS_PER_WALL));
                        // 4th vertex
                        vertices.push_back(j * SQUARE_SIZE);
                        vertices.push_back(yPoly * WALL_HEIGHT / POLYS_PER_WALL);
                        vertices.push_back(-i * SQUARE_SIZE - ((xPoly+1) * SQUARE_SIZE / POLYS_PER_WALL));
                    }
            }
        }
    }
    // y walls
    // number of y walls = number of squares + 1 for the last wall
    for(int i = 0; i <= maze.maxY + 1; i++)
    {
        for(int j = 0; j <= maze.maxX; j++)
        {
            if(maze.yWall[i][j])
            {
                // each wall is divided into several polygons to be able
                // to compute a better lighting
                for(int xPoly = 0; xPoly < POLYS_PER_WALL; xPoly++)
                    for(int yPoly = 0; yPoly < POLYS_PER_WALL; yPoly++)
                    {
                        // 1st vertex
                        vertices.push_back(j * SQUARE_SIZE + (xPoly * SQUARE_SIZE / POLYS_PER_WALL));
                        vertices.push_back(yPoly * WALL_HEIGHT / POLYS_PER_WALL);
                        vertices.push_back(-i * SQUARE_SIZE);
                        // 2nd vertex
                        vertices.push_back(j * SQUARE_SIZE + (xPoly * SQUARE_SIZE / POLYS_PER_WALL));
                        vertices.push_back((yPoly+1) * WALL_HEIGHT / POLYS_PER_WALL);
                        vertices.push_back(-i * SQUARE_SIZE);
                        // 3rd vertex
                        vertices.push_back(j * SQUARE_SIZE + ((xPoly+1) * SQUARE_SIZE / POLYS_PER_WALL));
                        vertices.push_back((yPoly+1) * WALL_HEIGHT / POLYS_PER_WALL);
                        vertices.push_back(-i * SQUARE_SIZE);
                        // 4th vertex
                        vertices.push_back(j * SQUARE_SIZE + ((xPoly+1) * SQUARE_SIZE / POLYS_PER_WALL));
                        vertices.push_back(yPoly * WALL_HEIGHT / POLYS_PER_WALL);
                        vertices.push_back(-i * SQUARE_SIZE);
                    }
            }
        }
    }

	// set the position to the start of the maze
    glMatrixMode(GL_MODELVIEW_MATRIX);
    glLoadIdentity();
    glTranslatef(-position[0], -position[1], -position[2]);

    /////////////////////////////////////////////////////////////////////////
	// initialize textures
    // load all textures from disk
    char s[] = "images/square.ppm";
    square.createTextureFromPPM(s);
    glActiveTexture = (PFNGLACTIVETEXTUREARBPROC) wglGetProcAddress("glActiveTextureARB");
    if (glActiveTexture == nullptr) {
        printf("No glActiveTexture function available\n");
        return;
    }

    glMultiTexCoord2f = (PFNGLMULTITEXCOORD2FARBPROC) wglGetProcAddress("glMultiTexCoord2fARB");
    if (glMultiTexCoord2f == nullptr)
    {
        printf("glMultiTexCoord2fv not supported");
        return;
    }

    // exit square texture
    glActiveTexture(GL_TEXTURE1_ARB);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // this texture unit use blend mode with white color
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
    float white[4] = {1,1,1,1};
    glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, white);
    // assign a texture for this texture unit

    // default texture
    glActiveTexture(GL_TEXTURE0_ARB);
    // bi-linear filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    //////////////////////////////////////////////////////////////////////////////
	glEnable(GL_DEPTH_TEST);

	// print description of commands on terminal
	printf("- MAZE HELP MENU -\n\n");
    printf(" Left arrow     -     Move left\n");
    printf(" Right arrow    -     Move right\n");
    printf(" Top arrow      -     Move forward\n");
    printf(" Bottom arrow   -     Move backward\n");
	printf(" F1             -     Toggle top view\n");
	printf(" F3             -     Quit\n");
}

void reshape(int width, int height)
{
	glViewport(0,0,width,height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(100, (float)width/(float)height, 0.1, 250);
    glMatrixMode(GL_MODELVIEW);
}

// Assign a texture coordinate for next vertex
void assignTextCoord()
{
    static int numVertex = 0;
    if (numVertex % 4 == 0) glTexCoord2f(0, 1);
    if (numVertex % 4 == 1) glTexCoord2f(0, 0);
    if (numVertex % 4 == 2) glTexCoord2f(1, 0);
    if (numVertex % 4 == 3) glTexCoord2f(1, 1);
    numVertex++;
}

// Draw a horizontal plane at the given altitude
void drawHorizontalPlane(float zCoordinate)
{
    glBegin(GL_QUADS);
    // define the normal
    glNormal3f(0.0, -1.0, 0.0);

    // divide the ceiling into several polygons to be able
    // to compute a better lighting
    for(int xPoly = 0; xPoly < POLYS_PER_WALL * (maze.maxX + 1); xPoly++)
        for(int yPoly = 0; yPoly < POLYS_PER_WALL * (maze.maxY + 1); yPoly++)
        {
            // draw vertices
            assignTextCoord();
            glVertex3d(xPoly * SQUARE_SIZE / POLYS_PER_WALL, zCoordinate, -yPoly * SQUARE_SIZE / POLYS_PER_WALL);
            assignTextCoord();
            glVertex3d(xPoly * SQUARE_SIZE / POLYS_PER_WALL, zCoordinate, -(yPoly+1) * SQUARE_SIZE / POLYS_PER_WALL);
            assignTextCoord();
            glVertex3d((xPoly+1) * SQUARE_SIZE / POLYS_PER_WALL, zCoordinate, -(yPoly+1) * SQUARE_SIZE / POLYS_PER_WALL);
            assignTextCoord();
            glVertex3d((xPoly+1) * SQUARE_SIZE / POLYS_PER_WALL, zCoordinate, -yPoly * SQUARE_SIZE / POLYS_PER_WALL);
        }
    glEnd();
}

void display()
{
    // clear both color buffer and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    /////////////////////////////////////////////////////
    // draw position
    // get a new matrix
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glColor3f(1.0f, 1.0f, 0.0f);
    // translate to the actual position and draw the sphere
    glTranslatef(position[0], position[1], position[2]);
    glutSolidSphere(0.1f, 10, 10);
    // draw a line materializing the direction
    glBegin(GL_LINES);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f((float)(direction[0] * SQUARE_SIZE/2), (float)(direction[1] * SQUARE_SIZE/2), (float)(direction[2] * SQUARE_SIZE/2));
    glEnd();
    glPopMatrix();
    /////////////////////////////////////////////////////////////
    // draw maze
    // draw all the walls stored in the array
    glColor3f(WALLS_COLOR);
    glBegin(GL_QUADS);

    // draw the vertices
    for(int i = 0; i < vertices.size(); i += 3){
        assignTextCoord();
        glVertex3dv(&vertices[i]);
    }
    glEnd();

    // draw the floor
    glColor3f(FLOOR_COLOR);
    drawHorizontalPlane(0.0);

    // draw the ceiling
    if(!topView)
    {
        glColor3f(CEILING_COLOR);
        drawHorizontalPlane((float) WALL_HEIGHT);
    }
    //////////////////////////////////////////////////////////////////////////
    // double buffering : swap buffers to send results to screen
    glutSwapBuffers();
}

// Perform a left or right rotation according to the argument
void turn(MazeTurnDirection turnDirection)
{
    int elapsedTime, waiting;

    // decompose the rotation into several steps
    for(int i = 0; i < TURN_NUMBER_OF_STEP; i++)
    {
        waiting = 0;
        elapsedTime= glutGet(GLUT_ELAPSED_TIME) ;
        while(waiting < TURN_TIME / TURN_NUMBER_OF_STEP)
            waiting = glutGet(GLUT_ELAPSED_TIME) - elapsedTime;

        // perform the rotate step
        glTranslatef(position[0], position[1], position[2]);
        glRotatef((float)turnDirection * TURN_ANGLE / (float)TURN_NUMBER_OF_STEP, 0.0, 1.0f, 0.0);
        glTranslatef(-position[0], -position[1], -position[2]);

        // update the direction
        float saveDir = direction[0];
        direction[0] = (float)(cos(-((float)turnDirection * TURN_ANGLE / (float)TURN_NUMBER_OF_STEP) / 180.0 * PI) * direction[0]
                       + sin(-((float)turnDirection * TURN_ANGLE / (float)TURN_NUMBER_OF_STEP) / 180.0 * PI) * direction[2]);
        direction[2] = (float)(-sin(-((float)turnDirection * TURN_ANGLE / (float)TURN_NUMBER_OF_STEP) / 180.0 * PI) * saveDir
                       + cos(-((float)turnDirection * TURN_ANGLE / (float)TURN_NUMBER_OF_STEP) / 180.0 * PI) * direction[2]);

        // draw this step
        display();
    }
    // round the direction to undo the error caused by float precision
    direction[0] = (float)((int)direction[0]);
    direction[2] = (float)((int)direction[2]);
}

// Move one step forward in the maze
void moveForward()
{
    // collision detection
    if(direction[0] == 1.0 && maze.xWall[mazePosition[1]][mazePosition[0] + 1] == 1) return;
    if(direction[0] == -1.0 && maze.xWall[mazePosition[1]][mazePosition[0]] == 1) return;
    if(direction[2] == -1.0 && maze.yWall[mazePosition[1] + 1][mazePosition[0]] == 1) return;
    if(direction[2] == 1.0 && maze.yWall[mazePosition[1]][mazePosition[0]] == 1) return;

    // if there is no wall compute the transformation
    glTranslatef(-direction[0] * MOVEMENT_SIZE, -direction[1] * MOVEMENT_SIZE, -direction[2] * MOVEMENT_SIZE);

    // update position in world coordinates
    position[0] += direction[0] * MOVEMENT_SIZE;
    position[1] += direction[1] * MOVEMENT_SIZE;
    position[2] += direction[2] * MOVEMENT_SIZE;

    // update position in maze coordinates
    mazePosition[0] += (int)direction[0];
    mazePosition[1] -= (int)direction[2];

    // test if the position is outside the maze
    if(mazePosition[0] < 0 || mazePosition[0] > maze.maxX || mazePosition[1] < 0 || mazePosition[1] > maze.maxY) success();
}

// Move one step backward in the maze
void moveBack()
{
    // collision detection
    if(direction[0] == 1.0 && maze.xWall[mazePosition[1]][mazePosition[0]] == 1) return;
    if(direction[0] == -1.0 && maze.xWall[mazePosition[1]][mazePosition[0] + 1] == 1) return;
    if(direction[2] == -1.0 && maze.yWall[mazePosition[1]][mazePosition[0]] == 1) return;
    if(direction[2] == 1.0 && maze.yWall[mazePosition[1] + 1][mazePosition[0]] == 1) return;

    // if there is no wall compute the transformation
    glTranslatef(direction[0] * MOVEMENT_SIZE, direction[1] * MOVEMENT_SIZE, direction[2] * MOVEMENT_SIZE);

    // update position in world coordinates
    position[0] -= direction[0] * MOVEMENT_SIZE;
    position[1] -= direction[1] * MOVEMENT_SIZE;
    position[2] -= direction[2] * MOVEMENT_SIZE;

    // update position in maze coordinates
    mazePosition[0] -= (int)direction[0];
    mazePosition[1] += (int)direction[2];

    // test if the position is outside the maze
    if(mazePosition[0] < 0 || mazePosition[0] > maze.maxX || mazePosition[1] < 0 || mazePosition[1] > maze.maxY) success();
}

void keyboardSpecial(int key, int, int)
{
	switch (key)
	{
		case GLUT_KEY_UP :		moveForward(); break;
		case GLUT_KEY_DOWN :	moveBack(); break;
		case GLUT_KEY_LEFT :	turn(MZ_LEFT); break;
		case GLUT_KEY_RIGHT :	turn(MZ_RIGHT); break;

		// switch top view
		case GLUT_KEY_F1 :
            static float orthoPosVect[3];

            // compute the vector orthogonal to the current direction
            // it will be used as the rotation axis to obtain a top view
            orthoPosVect[0] = direction[2];
            orthoPosVect[1] = 0.0;
            orthoPosVect[2] = -direction[0];

            if(!topView)
            {
                // rotate about the position in the maze
                glTranslatef(position[0], position[1], position[2]);
                glRotatef(-90.0, orthoPosVect[0], orthoPosVect[1], orthoPosVect[2]);
                glTranslatef(-position[0], -position[1], -position[2]);
                // move back to see the whole maze
                glTranslatef(0.0, -4.0, 0.0);
                topView = true;
            }
            else
            {
                // undo the transformations
                glTranslatef(0.0, 4.0, 0.0);
                glTranslatef(position[0], position[1], position[2]);
                glRotatef(90.0, orthoPosVect[0], orthoPosVect[1], orthoPosVect[2]);
                glTranslatef(-position[0], -position[1], -position[2]);
                topView = false;
            }
        break;
        case GLUT_KEY_F3 :      exit(0);
        default:;
	}
	// redraw the scene
	glutPostRedisplay() ;
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(500, 500);
	glutCreateWindow(argv[0]);
	init();
    glutFullScreen();
    glutReshapeFunc(reshape);
	glutDisplayFunc(display);
    PlaySound("game.wav", nullptr, SND_ASYNC | SND_FILENAME | SND_LOOP);
	glutSpecialFunc(keyboardSpecial) ;
	glutMainLoop();
	return 0;
}