//
// Created by Branislav on 19/10/2015.
//

#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <fstream>
#include <vector>

using namespace std;
using namespace glm;

#define SIZE 512
#define MOON_SIZE 400

struct Pixel {
    unsigned char r,g,b;
};

struct aPixel {
    unsigned char r,g,b,a;
};

GLFWwindow *window;
Pixel framebuffer[SIZE][SIZE]; // For Displaying.
Pixel background[SIZE][SIZE]; // For background image.
aPixel moon[MOON_SIZE][MOON_SIZE]; // For the Moon-Moon.
int points[5][5];
int size = SIZE;

// Loading the shaders. For the most part copied from the "gl_texture.cpp" and an online tutorial.
GLuint LoadShaders(const string vertex_shader_file, const string fragment_shader_file){

  // Creating shaders.
  GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
  GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

  // Reading the Vertex Shader Code.
  string VertexShaderCode;
  ifstream VertexShaderStream(vertex_shader_file, ios::in);
  string line = "";
  while(getline(VertexShaderStream, line)){
    VertexShaderCode += "\n" + line;
  }
  VertexShaderStream.close();

  // Reading the Fragment SHader Code.
  string FragmentShaderCode;
  ifstream FragmentShaderStream(fragment_shader_file, ios::in);
  line = "";
  while(getline(FragmentShaderStream, line)){
    FragmentShaderCode += "\n" + line;
  }
  FragmentShaderStream.close();

  GLint result = GL_FALSE;
  int InfoLogLength;

  // Compile Vertex Shader
  cout << "Compiling Vertex Shader ..." << endl;
  char const *VertexSourcePointer = VertexShaderCode.c_str();
  glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
  glCompileShader(VertexShaderID);

  // Check vertex shader log
  glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &result);
  if (result == GL_FALSE) {
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    string VertexShaderLog((unsigned int)InfoLogLength, ' ');
    glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderLog[0]);
    cout << VertexShaderLog << endl;
  }

  // Compile Fragment Shader
  cout << "Compiling Fragment Shader ..." << endl;
  char const *FragmentSourcePointer = FragmentShaderCode.c_str();
  glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
  glCompileShader(FragmentShaderID);

  // Check fragment shader log
  glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &result);
  if (result == GL_FALSE) {
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    string FragmentShaderLog((unsigned int)InfoLogLength, ' ');
    glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderLog[0]);
    cout << FragmentShaderLog << endl;
  }

  // Create and link the program
  cout << "Linking Shader Program ..." << endl;
  auto ProgramID = glCreateProgram();
  glAttachShader(ProgramID, VertexShaderID);
  glAttachShader(ProgramID, FragmentShaderID);
  glBindFragDataLocation(ProgramID, 0, "FragmentColor");
  glLinkProgram(ProgramID);

  // Check program log
  glGetProgramiv(ProgramID, GL_LINK_STATUS, &result);
  if (result == GL_FALSE) {
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    string ProgramLog((unsigned long)InfoLogLength, ' ');
    glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramLog[0]);
    cout << ProgramLog << endl;
  }
  glDeleteShader(VertexShaderID);
  glDeleteShader(FragmentShaderID);

  return ProgramID;
}

void initializeGeometry(GLuint ProgramID){

  // Vertex Array Object.
  GLuint VertexArrayID;
  glGenVertexArrays(1, &VertexArrayID);
  glBindVertexArray(VertexArrayID);

  // Vertices data - what and where do I actually want to draw.
  static const GLfloat vertexBufferData[] = {
           1.0f,  1.0f,
          -1.0f,  1.0f,
           1.0f, -1.0f,
          -1.0f, -1.0f
  };

  GLuint vertexBuffer;
  glGenBuffers(1, &vertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertexBufferData), vertexBufferData, GL_STATIC_DRAW); // Giving the vertices to OpenGL.

  auto Position = glGetAttribLocation(ProgramID, "Position");
  glVertexAttribPointer(Position, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(Position);

  static const GLfloat texCoordData[] =  {
          // u, v
          1.0f, 0.0f,
          0.0f, 0.0f,
          1.0f, 1.0f,
          0.0f, 1.0f
  };

  GLuint textureBuffer;
  glGenBuffers(1, &textureBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, textureBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(texCoordData), texCoordData, GL_STATIC_DRAW);

  auto TexCoord = glGetAttribLocation(ProgramID, "TexCoord");
  glVertexAttribPointer(TexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(TexCoord);

}

/*
 * Loading RGB image into the background.
 */
GLuint LoadImage(const string &image_file, unsigned int width, unsigned int height) {
  // Create new texture object
  GLuint TextureID;
  glGenTextures(1, &TextureID);
  glBindTexture(GL_TEXTURE_2D, TextureID);

  // Set mipmaps
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  // Read raw data
  ifstream image_stream(image_file, ios::binary);

//   Setup buffer for pixels (r,g,b bytes), since we will not manipulate the image just keep it as char
//  vector<char> buffer(width*height*3);
//  image_stream.read(buffer.data(), buffer.size());
  image_stream.read((char *) background, sizeof(background));
  image_stream.close();

//  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, background);

  return TextureID;
}

void convolution(double **kernel, unsigned int size){
  unsigned int center = size / 2;
  Pixel **image = (Pixel **)malloc(SIZE * sizeof(Pixel *));

  for(unsigned int i = 0; i < SIZE; i++){
    image[i] = (Pixel *)malloc(SIZE * sizeof(Pixel));
    for(unsigned int j = 0; j < SIZE; j++){
      double red = 0, green = 0, blue = 0;

      for(unsigned int ki = 0; ki < size; ki++){
        for(unsigned kj = 0; kj < size; kj++){
          int pi, pj;
          pi = i-(center-ki);
          pj = j-(center-kj);

          if(pi < 0){
            pi = SIZE-(center-ki);
          }
          else if(pi >= SIZE){
            pi = -1-(center-ki);
          }

          if(pj < 0){
            pj = SIZE-(center-kj);
          }
          else if(pj >= SIZE){
            pj = -1-(center-kj);
          }
          // i - (center - ki):
          // The Pixel in the middle - The distance of kernel element from the centre.
          // I get the indexes of pixel corresponding to kernel element.
          red += (double)background[pi][pj].r * kernel[ki][kj];
          green += (double)background[pi][pj].g * kernel[ki][kj];
          blue += (double)background[pi][pj].b * kernel[ki][kj];
        }
      }

      if(red > 255){
        red = 255;
      }
      else if(red < 0){
        red = 0;
      }

      if(green > 255){
        green = 255;
      }
      else if(green < 0){
        green = 0;
      }

      if(blue > 255){
        blue = 255;
      }
      else if(blue < 0){
        blue = 0;
      }

      image[i][j].r = (unsigned char)red;
      image[i][j].g = (unsigned char)green;
      image[i][j].b = (unsigned char)blue;

    }
  }
  for(unsigned int i = 0; i < SIZE; i++) {
    for (unsigned int j = 0; j < SIZE; j++) {
      background[i][j] = image[i][j];
    }
  }
}

int factorial(int n) {
  return (n <= 1) ? 1 : factorial(n - 1) * n;
}

int choose(int a, int b){
  return factorial(a)/(factorial(b)*factorial(a-b));
}

void Bezier(int number){
//  for(int i = 0; i < SIZE; i++){
//    for(int j = 0; j < SIZE; j++)
//    {
//      background[i][j].r = 255;
//      background[i][j].g = 255;
//      background[i][j].b = 255;
//    }
//  }

//  int points[5][2] = {
//          100, 100,
//          200, 400,
//          300, 150,
//          200, 400,
//          500, 500
//  };

  int ** BezierPoints = (int **)malloc(1000000 * sizeof(int *));
  //int BezierPoints[1000000][2];

  int x = 0;
  double add = 0.001;
  for(double t = 0; t <= 1; t += add){
    BezierPoints[x] = (int *)malloc(2 * sizeof(int));
    double sumX = 0;
    double sumY = 0;
    for(int i = 0; i < number; i++){
      sumX += (double)choose(number - 1, i) * pow(1.0 - t, (double)(number - 1 - i)) * pow(t, (double)i) * (double)points[i][0];
      sumY += (double)choose(number - 1, i) * pow(1.0 - t, (double)(number - 1 - i)) * pow(t, (double)i) * (double)points[i][1];
    }
    BezierPoints[x][0] = (int)sumX;
    BezierPoints[x][1] = (int)sumY;
    x++;
  }

//  cout << "Status: x = " << x << endl;

  for(int i = 0; i < x; i++) {
    if (BezierPoints[i][0] >= 0 && BezierPoints[i][0] < SIZE && BezierPoints[i][1] >= 0 &&
        BezierPoints[i][1] < SIZE) {
      background[BezierPoints[i][0]][BezierPoints[i][1]].r = 0;
      background[BezierPoints[i][0]][BezierPoints[i][1]].g = 0;
      background[BezierPoints[i][0]][BezierPoints[i][1]].b = 0;
    }
  }
}

void runBazier(){
  for(int i = 0; i < SIZE; i++){
    for(int j = 0; j < SIZE; j++)
    {
      background[i][j].r = 255;
      background[i][j].g = 255;
      background[i][j].b = 255;
    }
  }

  // Top of A.
  points[0][0] = size/3;
  points[0][1] = 0;

  points[1][0] = 0;
  points[1][1] = 0;

  points[2][0] = 0;
  points[2][1] = size/2;

  Bezier(3);

  points[0][0] = size/3;
  points[0][1] = size - 1;

  points[1][0] = 0;
  points[1][1] = size - 1;

  points[2][0] = 0;
  points[2][1] = size/2;

  Bezier(3);

  // Straight lines on both sides.
  points[0][0] = size/3;
  points[0][1] = 0;

  points[1][0] = size - size/6;
  points[1][1] = 0;

  Bezier(2);

  points[0][0] = size/3;
  points[0][1] = size - 1;

  points[1][0] = size - size/6;
  points[1][1] = size - 1;

  Bezier(2);

  // The bottom part of A.
  points[0][0] = size - size/6;
  points[0][1] = 0;

  points[1][0] = size - 1;
  points[1][1] = 0;

  points[2][0] = size - 1;
  points[2][1] = size/6;

  Bezier(3);

  points[0][0] = size - 1;
  points[0][1] = size/6;

  points[1][0] = size - 1;
  points[1][1] = 2 * size/6;

  points[2][0] = size - size/6;
  points[2][1] = 2 * size/6;

  Bezier(3);

  points[0][0] = size - size/6;
  points[0][1] = 2 * size/6;

  points[1][0] = size - 2 * size/6;
  points[1][1] = 2 * size/6;

  points[2][0] = size - 2 * size/6;
  points[2][1] = 3 * size/6;

  Bezier(3);

  points[0][0] = size - 2 * size/6;
  points[0][1] = 3 * size/6;

  points[1][0] = size - 2 * size/6;
  points[1][1] = 4 * size/6;

  points[2][0] = size - size/6;
  points[2][1] = 4 * size/6;

  Bezier(3);

  points[0][0] = size - size/6;
  points[0][1] = 4 * size/6;

  points[1][0] = size - 1;
  points[1][1] = 4 * size/6;

  points[2][0] = size - 1;
  points[2][1] = 5 * size/6;

  Bezier(3);

  points[0][0] = size - 1;
  points[0][1] = 5 * size/6;

  points[1][0] = size - 1;
  points[1][1] = size - 1;

  points[2][0] = size - size/6;
  points[2][1] = size - 1;

  Bezier(3);

  int centerY = size/3;
  int centerX = size/2;
  int dist = size/6;

  // The hole inside A.
  points[0][0] = centerY - dist;
  points[0][1] = centerX;

  points[1][0] = centerY - dist;
  points[1][1] = centerX + dist;

  points[2][0] = centerY;
  points[2][1] = centerX + dist;

  Bezier(3);

  points[0][0] = centerY;
  points[0][1] = centerX + dist;

  points[1][0] = centerY + dist;
  points[1][1] = centerX + dist;

  points[2][0] = centerY + dist;
  points[2][1] = centerX;

  Bezier(3);

  points[0][0] = centerY + dist;
  points[0][1] = centerX;

  points[1][0] = centerY + dist;
  points[1][1] = centerX - dist;

  points[2][0] = centerY;
  points[2][1] = centerX - dist;

  Bezier(3);

  points[0][0] = centerY;
  points[0][1] = centerX - dist;

  points[1][0] = centerY - dist;
  points[1][1] = centerX - dist;

  points[2][0] = centerY - dist;
  points[2][1] = centerX;

  Bezier(3);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods){
  if(action == GLFW_PRESS) {
    switch (key) {
      case GLFW_KEY_1: {
        cout << "Activating convolution filter: >> Edge Detection 3x3 <<" << endl;
        double data[3][3] = {
          -1.0, -1.0, -1.0,
          -1.0,  8.0, -1.0,
          -1.0, -1.0, -1.0
          };

        double **kernel = (double **)malloc(3 * sizeof(double*));
        for (int i = 0; i < 3; i++){
          kernel[i] = (double *)malloc(3 * sizeof(double));
          for(int j = 0; j < 3; j++){
            kernel[i][j] = data[i][j];
          }
        }
        convolution(kernel, 3);
        break;
      }
      case GLFW_KEY_2: {
        cout << "Activating convolution filter: >> Blur 3x3 <<" << endl;
        double data[3][3] = {
          1.0/16.0, 2.0/16.0, 1.0/16.0,
          2.0/16.0, 4.0/16.0, 2.0/16.0,
          1.0/16.0, 2.0/16.0, 1.0/16.0
        };
        double **kernel = (double **)malloc(3 * sizeof(double*));
        for (int i = 0; i < 3; i++){
          kernel[i] = (double *)malloc(3 * sizeof(double));
          for(int j = 0; j < 3; j++){
            kernel[i][j] = data[i][j];
          }
        }
        convolution(kernel, 3);
        break;
      }
      case GLFW_KEY_3: {
        cout << "Activating convolution filter: >> Sharpen 3x3 <<" << endl;
        double data[3][3] = {
          -1.0, -1.0, -1.0,
          -1.0,  9.0, -1.0,
          -1.0, -1.0, -1.0
        };
        double **kernel = (double **)malloc(3 * sizeof(double*));
        for (int i = 0; i < 3; i++){
          kernel[i] = (double *)malloc(3 * sizeof(double));
          for(int j = 0; j < 3; j++){
            kernel[i][j] = data[i][j];
          }
        }
        convolution(kernel, 3);
        break;
      }
      case GLFW_KEY_4: {
        cout << "Activating convolution filter: >> Emboss 5x5 <<" << endl;
        double data[5][5] = {
         -1, -1, -1, -1,  0,
         -1, -1, -1,  0,  1,
         -1, -1,  0,  1,  1,
         -1,  0,  1,  1,  1,
          0,  1,  1,  1,  1
        };
        double **kernel = (double **)malloc(5 * sizeof(double*));
        for (int i = 0; i < 5; i++){
          kernel[i] = (double *)malloc(5 * sizeof(double));
          for(int j = 0; j < 5; j++){
            kernel[i][j] = data[i][j];
          }
        }
        convolution(kernel, 5);
        break;
      }
      case GLFW_KEY_5: {
        cout << "Activating convolution filter: >> MotionBlur 9x9 <<" << endl;
        double data[9][9] =
        {
                1.0/9.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                0.0, 1.0/9.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                0.0, 0.0, 1.0/9.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                0.0, 0.0, 0.0, 1.0/9.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                0.0, 0.0, 0.0, 0.0, 1.0/9.0, 0.0, 0.0, 0.0, 0.0,
                0.0, 0.0, 0.0, 0.0, 0.0, 1.0/9.0, 0.0, 0.0, 0.0,
                0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0/9.0, 0.0, 0.0,
                0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0/9.0, 0.0,
                0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0/9.0,
        };
        double **kernel = (double **)malloc(9 * sizeof(double*));
        for (int i = 0; i < 9; i++){
          kernel[i] = (double *)malloc(9 * sizeof(double));
          for(int j = 0; j < 9; j++){
            kernel[i][j] = data[i][j];
          }
        }
        convolution(kernel, 9);
        break;
      }
      case GLFW_KEY_X:{
        ifstream image_stream("lena.rgb", ios::binary);
        image_stream.read((char *) background, sizeof(background));
        image_stream.close();
        break;
      }
      case GLFW_KEY_W: {
        size = SIZE;
        runBazier();
        break;
      }
      case GLFW_KEY_E: {
        size = SIZE/2;
        runBazier();
        break;
      }
      case GLFW_KEY_R: {
        size = SIZE/3;
        runBazier();
        break;
      }
      case GLFW_KEY_T: {
        size = SIZE/4;
        runBazier();
        break;
      }
      case GLFW_KEY_Y: {
        size = SIZE/5;
        runBazier();
        break;
      }
      case GLFW_KEY_D: {
        for(int i = 0; i < SIZE; i++){
          for(int j = 0; j < SIZE; j++)
          {
            background[i][j].r = 255;
            background[i][j].g = 255;
            background[i][j].b = 255;
          }
        }

        //front leg
        points[0][1] = 180;
        points[0][0] = 280;

        points[1][1] = 183;
        points[1][0] = 268;

        points[2][1] = 186;
        points[2][0] = 256;

        points[3][1] = 189;
        points[3][0] = 244;

        Bezier(4);

        //tummy
        points[0][1] = 191;
        points[0][0] = 244;

        points[1][1] = 290;
        points[1][0] = 244;

        points[2][1] = 300;
        points[2][0] = 230;

        points[3][1] = 339;
        points[3][0] = 245;

        Bezier(4);

        //back leg
        points[0][1] = 340;
        points[0][0] = 246;

        points[1][1] = 350;
        points[1][0] = 290;

        points[2][1] = 360;
        points[2][0] = 300;

        points[3][1] = 355;
        points[3][0] = 210;

        Bezier(4);

        //tail
        points[0][1] = 353;
        points[0][0] = 210;

        points[1][1] = 370;
        points[1][0] = 207;

        points[2][1] = 380;
        points[2][0] = 196;

        points[3][1] = 375;
        points[3][0] = 193;

        Bezier(4);

        //back
        points[0][1] = 375;
        points[0][0] = 193;

        points[1][1] = 310;
        points[1][0] = 220;

        points[2][1] = 190;
        points[2][0] = 220;

        points[3][1] = 164;
        points[3][0] = 205;

        Bezier(4);

        //ear start
        points[0][1] = 164;
        points[0][0] = 205;

        points[1][1] = 135;
        points[1][0] = 194;

        points[2][1] = 135;
        points[2][0] = 265;

        points[3][1] = 153;
        points[3][0] = 275;

        Bezier(4);

        //ear end + head
        points[0][1] = 153;
        points[0][0] = 275;

        points[1][1] = 168;
        points[1][0] = 275;

        points[2][1] = 170;
        points[2][0] = 180;

        points[3][1] = 150;
        points[3][0] = 190;

        Bezier(4);

        //nose bridge
        points[0][1] = 149;
        points[0][0] = 190;

        points[1][1] = 122;
        points[1][0] = 214;

        points[2][1] = 142;
        points[2][0] = 204;

        points[3][1] = 85;
        points[3][0] = 240;

        Bezier(4);

        //mouth
        points[0][1] = 86;
        points[0][0] = 240;

        points[1][1] = 100;
        points[1][0] = 247;

        points[2][1] = 125;
        points[2][0] = 233;

        points[3][1] = 140;
        points[3][0] = 238;

        Bezier(4);

        break;
      }
      default:{
        break;
      }
    }
  }
}

void DoMagic(){
  double MouseX, MouseY;
  int offsetX, offsetY;
  int centerX = MOON_SIZE/2, centerY = MOON_SIZE/2;

  glfwGetCursorPos(window, &MouseX, &MouseY);
  if(MouseX >= 0 && MouseX < SIZE && MouseY >= 0 && MouseY < SIZE){
    for(int i = 0; i < MOON_SIZE; i++){
      offsetY = i - centerY;
      for(int j = 0; j < MOON_SIZE; j++) {
        offsetX = j - centerX;

        int x = (int) MouseX + offsetX, y = (int) MouseY + offsetY;
        if (x >= 0 && x < SIZE && y >= 0 && y < SIZE) {
          double a = (double)moon[i][j].a/255.0; // The Alpha channel value.

          framebuffer[y][x].r = (unsigned char)(((double)moon[i][j].r/255.0 * a + (1 - a) * (double)background[y][x].r/255.0) * 255);
          framebuffer[y][x].g = (unsigned char)(((double)moon[i][j].g/255.0 * a + (1 - a) * (double)background[y][x].g/255.0) * 255);
          framebuffer[y][x].b = (unsigned char)(((double)moon[i][j].b/255.0 * a + (1 - a) * (double)background[y][x].b/255.0) * 255);
        }
      }
    }
  }
}

int main(){
  //GLFW initialization - controlling window and keyboard.
  if(!glfwInit()){
    fprintf(stderr, "Failed to initialize GLFW\n");
    return EXIT_FAILURE;
  }

  glfwWindowHint(GLFW_SAMPLES, 4); // 4x Antialiasing.
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // Making sure I use OpenGL 3.3
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // I don't want to use the old OpenGL.

  // Making a window.
  window = glfwCreateWindow(SIZE, SIZE, "Project One", NULL, NULL);
  if(window == NULL){
    fprintf(stderr, "Failed to open GLFW window. The GPU is not compatible with OpenGL 3.3. Basicly, you're in trouble :)\n");
    glfwTerminate();
    return EXIT_FAILURE;
  }
  glfwMakeContextCurrent(window);

  glewExperimental = GL_TRUE;
  if(glewInit() != GLEW_OK){
    fprintf(stderr, "Failed to initialize GLEW.\n");
    return EXIT_FAILURE;
  }

  // GLFW will listen to my keyboard commands.
  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

  GLuint ProgramID = LoadShaders("VertexShader.glsl", "FragmentShader.glsl");
  glUseProgram(ProgramID);

  initializeGeometry(ProgramID);

  // Create texture
  auto TextureID = LoadImage("lena.rgb", SIZE, SIZE);
  auto Texture = glGetUniformLocation(ProgramID, "Texture");
  glUniform1i(Texture, 0);
  glActiveTexture(GL_TEXTURE0 + 0);
  glBindTexture(GL_TEXTURE_2D, TextureID);

  ifstream loadImg("Moon.rgba", ios::in | ios::binary);
  loadImg.read((char *)moon, sizeof(moon));
  loadImg.close();

  for(int i = 0; i < MOON_SIZE; i++){
    for(int j = 0; j < MOON_SIZE; j++){
      if(moon[i][j].a != 0){
        double r = (rand() % 56) + 200;
        moon[i][j].a = (unsigned char)(r);
      }
    }
  }

  glfwSetKeyCallback(window, keyCallback);

  // Main loop where the magic happens.
  do{
    glClearColor(.5f,.5f,.5f,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for(int i = 0; i < SIZE; i++){
      for(int j = 0; j < SIZE; j++){
        framebuffer[i][j] = background[i][j];
      }
    }

    DoMagic();

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SIZE, SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, framebuffer);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glfwSwapBuffers(window);
    glfwPollEvents();

  }while(glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);

  glfwTerminate();

  return EXIT_SUCCESS;
}