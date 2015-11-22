//
// Created by Branislav on 19/10/2015.
//

// Compiling:
// mpicxx PaPP_p2.cpp -o PPaP -fopenmp -lglfw -lGLEW -lGL

#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <fstream>
#include <vector>
#include <omp.h>
#include <mpi.h>

using namespace std;
using namespace glm;

#define WIDTH 5120
#define HEIGHT 2880
#define MOON_SIZE 400

struct Pixel {
    unsigned char r,g,b;
};

struct aPixel {
    unsigned char r,g,b,a;
};

struct Chunk{

    int rows;
    Pixel **data;

    unsigned char k;
    float **kernel;

};

GLFWwindow *window;
Pixel framebuffer[HEIGHT][WIDTH]; // For Displaying.
Pixel background[HEIGHT][WIDTH]; // For background image.
aPixel moon[MOON_SIZE][MOON_SIZE]; // For the Moon-Moon.

// Loading the shaders. For the most part copied from the "gl_texture.cpp" and an online tutorial.
GLuint LoadShaders(const string vertex_shader_file, const string fragment_shader_file){

    // Creating shaders.
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Reading the Vertex Shader Code.
    string VertexShaderCode;
    ifstream VertexShaderStream(vertex_shader_file.c_str(), ios::in);
    string line = "";
    while(getline(VertexShaderStream, line)){
        VertexShaderCode += "\n" + line;
    }
    VertexShaderStream.close();

    // Reading the Fragment SHader Code.
    string FragmentShaderCode;
    ifstream FragmentShaderStream(fragment_shader_file.c_str(), ios::in);
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
    GLuint ProgramID = glCreateProgram();
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

    GLint Position = glGetAttribLocation(ProgramID, "Position");
    glVertexAttribPointer((GLuint) Position, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray((GLuint) Position);

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

    GLint TexCoord = glGetAttribLocation(ProgramID, "TexCoord");
    glVertexAttribPointer((GLuint) TexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray((GLuint) TexCoord);

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
    ifstream image_stream(image_file.c_str(), ios::binary);

//   Setup buffer for pixels (r,g,b bytes), since we will not manipulate the image just keep it as char
//  vector<char> buffer(width*height*3);
//  image_stream.read(buffer.data(), buffer.size());
    image_stream.read((char *) background, sizeof(background));
    image_stream.close();

//  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, background);

    return TextureID;
}

void convolution(Chunk *chunk){
    unsigned int center = chunk->k / 2;
    Pixel **image = (Pixel **)malloc(HEIGHT * sizeof(Pixel *));

    #pragma omp parallel for schedule(dynamic) num_threads(4)
    for(unsigned int i = 0; i < chunk->rows; i++){
        image[i] = (Pixel *)malloc(WIDTH * sizeof(Pixel));
        for(unsigned int j = 0; j < WIDTH; j++){
            double red = 0, green = 0, blue = 0;

            for(unsigned int ki = 0; ki < chunk->k; ki++){
                for(unsigned kj = 0; kj < chunk->k; kj++){
                    int pi, pj;
                    pi = i-(center-ki);
                    pj = j-(center-kj);

                    if(pi < 0){
                        pi = chunk->rows-(center-ki);
                    }
                    else if(pi >= chunk->rows){
                        pi = -1-(center-ki);
                    }

                    if(pj < 0){
                        pj = WIDTH-(center-kj);
                    }
                    else if(pj >= WIDTH){
                        pj = -1-(center-kj);
                    }
                    // i - (center - ki):
                    // The Pixel in the middle - The distance of kernel element from the centre.
                    // I get the indexes of pixel corresponding to kernel element.
                    red += (double)chunk->data[pi][pj].r * chunk->kernel[ki][kj];
                    green += (double)chunk->data[pi][pj].g * chunk->kernel[ki][kj];
                    blue += (double)chunk->data[pi][pj].b * chunk->kernel[ki][kj];
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
    for(unsigned int i = 0; i < chunk->rows; i++) {
        for (unsigned int j = 0; j < WIDTH; j++) {
            chunk->data[i][j] = image[i][j];
        }
    }
}

void keyCallback(GLFWwindow* /* window */, int key, int /* scancode */, int action, int mods){
    double time;

    if(action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_1: {
                cout << "Activating convolution filter: >> Edge Detection 3x3 <<" << endl;
                float data[3][3] = {
                        -1.0, -1.0, -1.0,
                        -1.0,  8.0, -1.0,
                        -1.0, -1.0, -1.0
                };

                float **kernel = (float **)malloc(3 * sizeof(float *));
                for (int i = 0; i < 3; i++){
                    kernel[i] = (float *)malloc(3 * sizeof(float));
                    for(int j = 0; j < 3; j++){
                        kernel[i][j] = data[i][j];
                    }
                }
                time = omp_get_wtime();

                int np;
                MPI_Comm_size(MPI_COMM_WORLD, &np);
                int chunkSize = HEIGHT/np;
                Chunk *chunk = (Chunk *)malloc(sizeof(Chunk));
                int rowsSent = 0;

                cout << "ANTI TEST 1 Chunk Size " << chunkSize << endl;
                chunk->data = (Pixel **)malloc(chunkSize * sizeof(Pixel *));
                for(int i = 1; i < np; i ++){
                    chunk->rows = chunkSize;
                    for(int j = 0; j < chunkSize; j++){
                        rowsSent ++;
                        chunk->data[j] = (Pixel *) malloc(WIDTH * sizeof(Pixel));
                        for(int k = 0; k < WIDTH; k++){
                            chunk->data[j][k] = background[rowsSent][k];
                        }
                    }
                    chunk->k = 3;
                    chunk->kernel = kernel;

                    MPI_Send(chunk, 1, MPI_BYTE, i, 50, MPI_COMM_WORLD);
                }

                cout << "ANTI TEST 2\n";
                int lastChunkIndex = rowsSent;
                chunk->rows = chunkSize;
                for(int j = 0; j < chunkSize; j++){
                    chunk->data[j] = (Pixel *) malloc(WIDTH * sizeof(Pixel));
                    for(int k = 0; k < WIDTH; k++){
                        chunk->data[j][k] = background[rowsSent][k];
                    }
                    rowsSent ++;
                }
                chunk->k = 3;
                chunk->kernel = kernel;
                cout << "ANTI TEST 3\n";
                convolution(chunk);

                cout << "ANTI TEST 4\n";
                cout << "last chunk: " << lastChunkIndex << " chunk size " << chunkSize << endl;
                for(int i = 0; i < chunkSize; i ++) {
                    for(int j = 0; j < WIDTH; j++){
//                        cout << i << " " << j << " " << "\n";
                        background[lastChunkIndex][j] = chunk->data[i][j];
                    }
                    lastChunkIndex ++;
                }

                cout << "ANTI TEST 5\n";
                MPI_Status status;
                rowsSent = 0;
                for(int i = 1; i < np; i++){
                    MPI_Recv(chunk, 1, MPI_BYTE, i, 50, MPI_COMM_WORLD, &status);
                    for(int j = 0; j < chunkSize; j ++) {
                        for(int k = 0; k < WIDTH; k++){
                            background[rowsSent][k] = chunk->data[j][k];
                        }
                        rowsSent ++;
                    }
                }

                time = omp_get_wtime() - time;
                cout << ">> Edge Detection 3x3 << finished. Took " << time << " seconds to finish." << endl;
                break;
            }
//            case GLFW_KEY_2: {
//                cout << "Activating convolution filter: >> Blur 3x3 <<" << endl;
//                double data[3][3] = {
//                        1.0/16.0, 2.0/16.0, 1.0/16.0,
//                        2.0/16.0, 4.0/16.0, 2.0/16.0,
//                        1.0/16.0, 2.0/16.0, 1.0/16.0
//                };
//                double **kernel = (double **)malloc(3 * sizeof(double*));
//                for (int i = 0; i < 3; i++){
//                    kernel[i] = (double *)malloc(3 * sizeof(double));
//                    for(int j = 0; j < 3; j++){
//                        kernel[i][j] = data[i][j];
//                    }
//                }
//                time = omp_get_wtime();
//                convolution(kernel, 3);
//                time = omp_get_wtime() - time;
//                cout << ">> Blur 3x3 << finished. Took " << time << " seconds to finish." << endl;
//                break;
//            }
//            case GLFW_KEY_3: {
//                cout << "Activating convolution filter: >> Sharpen 3x3 <<" << endl;
//                double data[3][3] = {
//                        -1.0, -1.0, -1.0,
//                        -1.0,  9.0, -1.0,
//                        -1.0, -1.0, -1.0
//                };
//                double **kernel = (double **)malloc(3 * sizeof(double*));
//                for (int i = 0; i < 3; i++){
//                    kernel[i] = (double *)malloc(3 * sizeof(double));
//                    for(int j = 0; j < 3; j++){
//                        kernel[i][j] = data[i][j];
//                    }
//                }
//                time = omp_get_wtime();
//                convolution(kernel, 3);
//                time = omp_get_wtime() - time;
//                cout << ">> Sharpen 3x3 << finished. Took " << time << " seconds to finish." << endl;
//                break;
//            }
//            case GLFW_KEY_4: {
//                cout << "Activating convolution filter: >> Emboss 5x5 <<" << endl;
//                double data[5][5] = {
//                        -1, -1, -1, -1,  0,
//                        -1, -1, -1,  0,  1,
//                        -1, -1,  0,  1,  1,
//                        -1,  0,  1,  1,  1,
//                        0,  1,  1,  1,  1
//                };
//                double **kernel = (double **)malloc(5 * sizeof(double*));
//                for (int i = 0; i < 5; i++){
//                    kernel[i] = (double *)malloc(5 * sizeof(double));
//                    for(int j = 0; j < 5; j++){
//                        kernel[i][j] = data[i][j];
//                    }
//                }
//                time = omp_get_wtime();
//                convolution(kernel, 5);
//                time = omp_get_wtime() - time;
//                cout << ">> Emboss 5x5 << finished. Took " << time << " seconds to finish." << endl;
//                break;
//            }
//            case GLFW_KEY_5: {
//                cout << "Activating convolution filter: >> MotionBlur 9x9 <<" << endl;
//                double data[9][9] =
//                        {
//                                1.0/9.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
//                                0.0, 1.0/9.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
//                                0.0, 0.0, 1.0/9.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
//                                0.0, 0.0, 0.0, 1.0/9.0, 0.0, 0.0, 0.0, 0.0, 0.0,
//                                0.0, 0.0, 0.0, 0.0, 1.0/9.0, 0.0, 0.0, 0.0, 0.0,
//                                0.0, 0.0, 0.0, 0.0, 0.0, 1.0/9.0, 0.0, 0.0, 0.0,
//                                0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0/9.0, 0.0, 0.0,
//                                0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0/9.0, 0.0,
//                                0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0/9.0,
//                        };
//                double **kernel = (double **)malloc(9 * sizeof(double*));
//                for (int i = 0; i < 9; i++){
//                    kernel[i] = (double *)malloc(9 * sizeof(double));
//                    for(int j = 0; j < 9; j++){
//                        kernel[i][j] = data[i][j];
//                    }
//                }
//                time = omp_get_wtime();
//                convolution(kernel, 9);
//                time = omp_get_wtime() - time;
//                cout << ">> MotionBlur 9x9 << finished. Took " << time << " seconds to finish." << endl;
//                break;
//            }
            case GLFW_KEY_X:{
                ifstream image_stream("bridge.rgb", ios::binary);
                image_stream.read((char *) background, sizeof(background));
                image_stream.close();
                break;
            }
            default:{
                break;
            }
        }
    }
}

int main(int argc, char **argv){

    int me, np;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &np);
    MPI_Comm_rank(MPI_COMM_WORLD, &me);

    MPI_Datatype custom;

    Chunk *myChunk = (Chunk *)malloc(sizeof(Chunk));

    if(me != 0){
        while(1){
            MPI_Recv(myChunk, 1, MPI_BYTE, 0, 50, MPI_COMM_WORLD, &status);

            cout << me << " TEST 1 ROWS" << myChunk->rows << endl;
            if(myChunk->rows < 0){
                MPI_Finalize();
                return EXIT_SUCCESS;
            }

            cout << me << " TEST 2 ROWS: " << myChunk->rows << endl;
            for(int i = 0; i < myChunk->rows; i++){
                for(int j = 0; j < WIDTH; j++){
                    background[i][j] = myChunk->data[i][j];
                }
            }
            cout << me << " TEST 3\n";
            double **kernel = (double **)malloc(myChunk->k * sizeof(double*));
            for (int i = 0; i < myChunk->k; i++){
                kernel[i] = (double *)malloc(myChunk->k * sizeof(double));
                for(int j = 0; j < myChunk->k; j++){
                    kernel[i][j] = myChunk->kernel[i][j];
                }
            }
            cout << me << " TEST 4 \n";
            convolution(myChunk);
            cout << me << " TEST 5 \n";
            MPI_Send(myChunk, 1, MPI_BYTE, 0, 50, MPI_COMM_WORLD);
        }
    }

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
    window = glfwCreateWindow(1920, 1080, "PPaP Project II", NULL, NULL);
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
    GLuint TextureID = LoadImage("bridge.rgb", WIDTH, HEIGHT);
    GLuint Texture = glGetUniformLocation(ProgramID, "Texture");
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

        for(int i = 0; i < WIDTH; i++){
            for(int j = 0; j < HEIGHT; j++){
                framebuffer[j][i] = background[j][i];
            }
        }

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, framebuffer);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glfwSwapBuffers(window);
        glfwPollEvents();

    }while(glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);

    glfwTerminate();
    MPI_Finalize();

    return EXIT_SUCCESS;
}