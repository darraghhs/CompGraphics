#include <stdio.h>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <util/array2d.h>
#include <util/util.h>

#include <render/shader.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <vector>
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>

// Initialising window object
static GLFWwindow *window;

// Declaring key callback function
static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);

// OpenGL camera view parameters
static glm::vec3 eye_center(1.0f, 1.0f, 1.0f);
static glm::vec3 lookat(0, 0, 0);
static glm::vec3 up(0, 1, 0);
float pi = 3.141592654;
float grid_size = 0.025;

// View control 
static float viewAzimuth = 0.0f;
static float viewPolar = 0.0f;
static float viewDistance = 1.0f;

struct AxisXYZ {
    // A structure for visualizing the global 3D coordinate system 
	
	GLfloat vertex_buffer_data[18] = {
		// X axis
		0.0, 0.0f, 0.0f, 
		100.0f, 0.0f, 0.0f,
		
		// Y axis
		0.0f, 0.0f, 0.0f, 
		0.0f, 100.0f, 0.0f, 
		
		// Z axis
		0.0f, 0.0f, 0.0f, 
		0.0f, 0.0f, 100.0f,
	};

	GLfloat color_buffer_data[18] = {
		// X, red
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,

		// Y, green
		0.0f, 1.0f, 0.0f, 
		0.0f, 1.0f, 0.0f,

		// Z, blue
		0.0f, 0.0f, 1.0f, 
		0.0f, 0.0f, 1.0f,
	};

	// OpenGL buffers
	GLuint vertexArrayID; 
	GLuint vertexBufferID; 
	GLuint colorBufferID;

	// Shader variable IDs
	GLuint mvpMatrixID;
	GLuint programID;

	void init() {
		// Create a vertex array object
		glGenVertexArrays(1, &vertexArrayID);
		glBindVertexArray(vertexArrayID);

		// Create a vertex buffer object to store the vertex data		
		glGenBuffers(1, &vertexBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_STATIC_DRAW);

		// Create a vertex buffer object to store the color data
		glGenBuffers(1, &colorBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(color_buffer_data), color_buffer_data, GL_STATIC_DRAW);

		// Create and compile our GLSL program from the shaders
		programID = LoadShadersFromFile("../proj/axis.vert", "../proj/axis.frag");
		if (programID == 0)
		{
			std::cerr << "Failed to load shaders." << std::endl;
		}

		// Get a handle for our "MVP" uniform
		mvpMatrixID = glGetUniformLocation(programID, "MVP");
	}

	void render(glm::mat4 cameraMatrix) {
		glUseProgram(programID);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glm::mat4 mvp = cameraMatrix;
		glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);

        // Draw the lines
        glDrawArrays(GL_LINES, 0, 6);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
	}

	void cleanup() {
		glDeleteBuffers(1, &vertexBufferID);
		glDeleteBuffers(1, &colorBufferID);
		glDeleteVertexArrays(1, &vertexArrayID);
		glDeleteProgram(programID);
	}
}; 

struct Plane {

    // Shader variables
    GLuint vpMatrixID;
    GLuint eyecenterID;
    GLuint gridSizeID;
    GLuint programID;

    void init(){

        // Create and compile our GLSL program from the shaders
        programID = LoadShadersFromFile("../proj/plane.vert", "../proj/plane.frag");

        if (programID == 0){
            std::cerr << "Failed to load shaders." << std::endl;
        }

        vpMatrixID = glGetUniformLocation(programID, "VP");
        eyecenterID = glGetUniformLocation(programID, "camPos");
        gridSizeID = glGetUniformLocation(programID, "gGridCCellSize");
    }

    void render(glm::mat4 cameraMatrix, glm::vec3 eyeCenter, float grid_size){

        // Use shaders
        glUseProgram(programID);

        // Passing vp and camera position to shader
        glm::mat4 vp = cameraMatrix;
        glm::vec3 camPos = eyeCenter;
        glUniformMatrix4fv(vpMatrixID, 1, GL_FALSE, &cameraMatrix[0][0]);
        glUniform3fv(eyecenterID, 1, &camPos[0]);
        glUniform1f(gridSizeID, grid_size);

        glDrawArrays(GL_TRIANGLES, 0, 6);

    }

    void cleanup() {

	    glDeleteProgram(programID);
	}

};

struct Terrain {
    int m_terrainSize = 0;
    int m_width = 0;
    int m_depth = 0;
    Array2D<float> m_heightMap;
     
    
    GLuint VAO;
    GLuint vertexBuffer;
    GLuint indexBuffer;

    GLuint programID;
    GLuint vpMatrixID;

    struct Vertex{
        glm::vec3 pos;

        void initVertex(float y, int x, int z){
            this->pos = glm::vec3(x, y, z);
        }
    };

    std::vector<Vertex> vertices;

    void init(){
          
        // Create and compile our GLSL program from the shaders
        programID = LoadShadersFromFile("../proj/terrain.vert", "../proj/terrain.frag");

        if (programID == 0){
            std::cerr << "Failed to load shaders." << std::endl;
        }

        printf("1. Shaders loaded\n");

        vpMatrixID = glGetUniformLocation(programID, "VP");

        this->LoadFromFile("../data/heightmap.save");

        this->populateBuffers();

    }

    void LoadFromFile(const char* pFilename){
        
        this->LoadHeightMap(pFilename);
        printf("4. Height map loaded!\n");
        this->createTriangleList(this->m_terrainSize, this->m_terrainSize, this);
        printf("5.Triangles width: %d, depth: %d\n", this->m_width, this->m_depth);
    }

    void LoadHeightMap(const char* pFilename){
        int fileSize = 0;
        unsigned char* p = (unsigned char*)ReadBinaryFile(pFilename, fileSize);
        printf("2. File size: %d\n", fileSize);
        assert(fileSize % sizeof(float) == 0);
        this->m_terrainSize = sqrt(fileSize / sizeof(float));
        printf("3. terrain size: %d\n", this->m_terrainSize);

        this->m_heightMap.InitArray2D(m_terrainSize, m_terrainSize, p);
    }

    void createTriangleList(int width, int depth, const Terrain* terrain){
        
        this->m_width = width;
        this->m_depth = depth;

        createGLState();

    }

    void createGLState(){
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        glGenBuffers(1, &vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

        int pos_loc = 0;

        glEnableVertexAttribArray(pos_loc);

        size_t numFloats = 0;
        glVertexAttribPointer(pos_loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(numFloats * sizeof(float)));
        numFloats += 3;
    }

    void populateBuffers(){
        this->vertices.resize(this->m_width * this->m_depth);
        printf("6.Vertices size: %d\n", this->vertices.size());
        initVertices();

        glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(this->vertices[0]), &this->vertices[0], GL_STATIC_DRAW);
    }

    void initVertices(){
        int index = 0;

        for(int z = 0; z < m_depth; z++) {
            for(int x = 0; x < m_width; x++){
                assert(index < this->vertices.size());
                this->vertices[index].initVertex(this->getHeight(x, z), x, z);
                //printf("x: %f, y: %f z: %f\n", this->vertices[index].pos.x, this->vertices[index].pos.y, this->vertices[index].pos.z);
                index++;
            }
        }
        printf("7. Vertices initialised!\n");
    }

    float getHeight(int x, int z) const{
        return this->m_heightMap.Get(x, z);
    }

    void render(glm::mat4 VP){

        glUseProgram(programID);

        glUniformMatrix4fv(vpMatrixID, 1, GL_FALSE, &VP[0][0]);

        glBindVertexArray(VAO);

        glDrawArrays(GL_POINTS, 0, m_depth * m_width);

        glBindVertexArray(0);

    }


};

int initWindow(){

    // Initialise GLFW
	if (!glfwInit())
	{
		std::cerr << "Failed to initialize GLFW." << std::endl;
		return -1;
	}
    // Window hints
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Creating window
    window = glfwCreateWindow(1024, 768, "Final Project", NULL, NULL);

    // Checking for errors creating window
    if (window == NULL){
        std::cerr << "Failed to open a GLFW window." << std::endl;
		glfwTerminate();
		return -1;
    }
    // Making the window the current context
    glfwMakeContextCurrent(window);

    // Ensuring key press capture
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    // Setting key callback function
	glfwSetKeyCallback(window, key_callback);

    // Loading OpenGL functions
	int version = gladLoadGL(glfwGetProcAddress);
	if (version == 0)
	{
		std::cerr << "Failed to initialize OpenGL context." << std::endl;
		return -1;
	}

    return 0;
}

void initCamera(glm::mat4 &projectionMatrix){

     // Camera setup
    eye_center.x = 0.0f;
    eye_center.y = 0.1f;
    eye_center.z = 0.0f;
    viewAzimuth = 0.0f;
    viewPolar = 0.0f;
	lookat.x = eye_center.x + sin(viewAzimuth);
    lookat.z = eye_center.z + cos(viewAzimuth);
    lookat.y = 0.0f;

    // Creating projection matrix
    glm::float32 FoV = 60;
	glm::float32 zNear = 0.1f; 
	glm::float32 zFar = 500.0f;
	projectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, zNear, zFar);

}

int main(void){
    
    // Initialising window, glfw and opengl
    if(initWindow() == -1){
        return -1;
    }

    // Setting background colour
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Initialising scene
    //Plane s;
    //s.init();
    AxisXYZ axis;
    axis.init();
    Terrain terrain;
    terrain.init();

    glm::mat4 projectionMatrix;

    initCamera(projectionMatrix);
    

    do{

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 viewMatrix = glm::lookAt(eye_center, lookat, up);
		glm::mat4 vp = projectionMatrix * viewMatrix;

        //s.render(vp, eye_center, grid_size);
        terrain.render(vp);
        axis.render(vp);

        // Swap buffers
        glfwSwapBuffers(window);

        // Polling for events
        glfwPollEvents();

    }
    // Check if esc was pressed or window was closed
    while(!glfwWindowShouldClose(window));

    printf("Goodbye Scene!");
    //s.cleanup();
    axis.cleanup();
    glfwTerminate();
    return 0;
}

// Funtion runs when key is pressed
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_R && action == GLFW_PRESS)
	{
		viewAzimuth = 0.0f;
		viewPolar = 0.0f;
		eye_center.y = 0.1f;
		eye_center.x = 0.0f;
		eye_center.z = 0.0f;
		lookat.x = eye_center.x + sin(viewAzimuth);
        lookat.z = eye_center.z + cos(viewAzimuth);
        lookat.y = 0.0f;

		std::cout << "Reset." << std::endl;
	}

	if (key == GLFW_KEY_UP && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
	
		lookat.y += 0.1f;
	}

	if (key == GLFW_KEY_DOWN && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		lookat.y -= 0.1f;
	}

	if (key == GLFW_KEY_LEFT && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		viewAzimuth += 0.1f;
		lookat.x = eye_center.x + sin(viewAzimuth);
        lookat.z = eye_center.z + cos(viewAzimuth);
	}

	if (key == GLFW_KEY_RIGHT && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		viewAzimuth -= 0.1f;
		lookat.x = eye_center.x + sin(viewAzimuth);
        lookat.z = eye_center.z + cos(viewAzimuth);
	}

    if (key == GLFW_KEY_W && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
        eye_center.x += 1.0 * sin(viewAzimuth);
        eye_center.z += 1.0 * cos(viewAzimuth);
        lookat.x = eye_center.x + sin(viewAzimuth);
        lookat.z = eye_center.z + cos(viewAzimuth);
	}

    if (key == GLFW_KEY_S && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
        eye_center.x -= 0.1 * sin(viewAzimuth);
        eye_center.z -= 0.1 * cos(viewAzimuth);
        lookat.x = eye_center.x + sin(viewAzimuth);
        lookat.z = eye_center.z + cos(viewAzimuth);
	}

    if (key == GLFW_KEY_A && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
        float offset = viewAzimuth + pi;
        eye_center.x -= 0.1 * cos(offset);
        eye_center.z += 0.1 * sin(offset);
        lookat.x = eye_center.x + sin(viewAzimuth);
        lookat.z = eye_center.z + cos(viewAzimuth);
	}    

    if (key == GLFW_KEY_D && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
        float offset = viewAzimuth - pi;
        eye_center.x += 0.1 * cos(offset);
        eye_center.z -= 0.1 * sin(offset);
        lookat.x = eye_center.x + sin(viewAzimuth);
        lookat.z = eye_center.z + cos(viewAzimuth);
	}

    if (key == GLFW_KEY_SPACE && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
        float speed = 1.0f;
        eye_center.y += speed;
        lookat.y += speed;
	}

    if (key == GLFW_KEY_LEFT_SHIFT && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
        float speed = 1.0f;
        eye_center.y -= speed;
        lookat.y -= speed;
	}

    if (key == GLFW_KEY_Z && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
        grid_size -= .005;
        //printf("grid size: %f\n", grid_size);
	}         

    if (key == GLFW_KEY_X && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
        grid_size += .005;
        //printf("grid size: %f\n", grid_size);
	} 

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

    //printf("Az: %f, Po: %f, dis: %f\n", viewAzimuth, viewPolar, viewDistance);
    //printf("Eye center : %f, %f, %f\n", eye_center.x, eye_center.y, eye_center.z);
}