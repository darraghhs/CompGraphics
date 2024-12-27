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

float randomFloat();
int randomInt(int a, int b);
int randomFloatRange(float a, float b);

static GLuint LoadTextureTileBox(const char *texture_file_path);

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
    float m_minHeight = 1.0f;
    float m_maxHeight = 1.0f;
    float m_worldScale = 1.0f;
    Array2D<float> m_heightMap;
    Array2D<float> m_heightMap1;
    glm::vec3 lightDir = glm::vec3(1.0f, -1.0f, 1.0f);

     
    
    GLuint VAO;
    GLuint vertexBuffer;
    GLuint indexBuffer;
    GLuint uvBufferID;

    GLuint programID;
    GLuint vpMatrixID;
    GLuint minID;
    GLuint maxID;
    GLuint textureID;
    GLuint lightDirID;

    struct Vertex{
        glm::vec3 pos;
        glm::vec2 tex;
        glm::vec3 norm = glm::vec3(0.0f, 0.0f, 0.0f);

        void initVertex(float y, int x, int z, float scale, float terrainSize){
            this->pos = glm::vec3(x * scale, y, z * scale);
            float terrainScale = terrainSize / 512;
            tex = glm::vec2(fmod((float)x / (terrainSize / terrainScale), 1), fmod((float)z / (terrainSize / terrainScale), 1));
        }
    };

    std::vector<Vertex> vertices;

    void init(){
        this->m_worldScale = 1.0f;
        // Create and compile our GLSL program from the shaders
        programID = LoadShadersFromFile("../proj/terrain.vert", "../proj/terrain.frag");

        if (programID == 0){
            std::cerr << "Failed to load shaders." << std::endl;
        }

        printf("1. Shaders loaded\n");

        textureID = LoadTextureTileBox("../proj/textures/texture4.jpg");

        vpMatrixID = glGetUniformLocation(programID, "VP");
        minID = glGetUniformLocation(programID, "minHeight");
        maxID = glGetUniformLocation(programID, "maxHeight");
        textureID = glGetUniformLocation(programID, "texSampler");
        lightDirID = glGetUniformLocation(programID, "reversedLightDir");


    }


    void initMidPoint(){

        this->init();

        CreateMidpointDisplacement(this, 512, 1, 1, 100);

        this->populateBuffers();
    }

    void initFromFile(){

        this->init();

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

    void createTriangleList(int width, int depth, Terrain* terrain){
        
        terrain->m_width = width;
        terrain->m_depth = depth;

        createGLState();

    }

    void createGLState(){
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        glGenBuffers(1, &vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

        int pos_loc = 0;
        int tex_loc = 1;
        int norm_loc = 2;

        glGenBuffers(1, &indexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

        size_t numFloats = 0;

        glEnableVertexAttribArray(pos_loc);
        glVertexAttribPointer(pos_loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(numFloats * sizeof(float)));
        numFloats += 3;

        glEnableVertexAttribArray(tex_loc);
        glVertexAttribPointer(tex_loc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(numFloats * sizeof(float)));
        numFloats += 2;

        glEnableVertexAttribArray(norm_loc);
        glVertexAttribPointer(norm_loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(numFloats * sizeof(float)));
        numFloats += 3;
    }

    void populateBuffers(){
        this->vertices.resize(this->m_width * this->m_depth);
        printf("6.Vertices size: %d\n", this->vertices.size());
        initVertices();

        std::vector<unsigned int> indices;
        int  numQuads = (m_width - 1) * (m_depth - 1);
        indices.resize(numQuads * 6);
        initIndices(indices);

        calcNormals(this->vertices, indices);

        glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(this->vertices[0]), &this->vertices[0], GL_STATIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), &indices[0], GL_STATIC_DRAW);
    }

    void initIndices(std::vector<unsigned int>& indices){
        int index = 0;

        for(int z = 0; z < m_depth - 1; z++){
            for(int x = 0; x < m_width - 1; x++){
                unsigned int bottomLeft = z * m_width + x;
                unsigned int topLeft = (z + 1) * m_width + x;
                unsigned int topRight = (z + 1) * m_width + (x + 1);
                unsigned int bottomRight = z * m_width + (x + 1);

                // Top left triangle
                indices[index++] = bottomLeft;
                indices[index++] = topLeft;
                indices[index++] = topRight;

                // Bottom right triangle
                indices[index++] = bottomLeft;
                indices[index++] = topRight;
                indices[index++] = bottomRight;                

            }
        }
    }

    void initVertices(){
        int index = 0;

        for(int z = 0; z < m_depth; z++) {
            for(int x = 0; x < m_width; x++){
                assert(index < this->vertices.size());
                this->vertices[index].initVertex(this->getHeight(x, z), x, z, this->m_worldScale, this->m_terrainSize);
                //printf("x: %f, y: %f, z: %f, u: %f, v: %f\n", this->vertices[index].pos.x, this->vertices[index].pos.y, this->vertices[index].pos.z, this->vertices[index].tex.x, this->vertices[index].tex.y);
                index++;
            }
        }
        printf("7. Vertices initialised!\n");
    }

    float getHeight(int x, int z) const{
        return this->m_heightMap.Get(x, z);
    }

    float getHeight1(int x, int z) const{
        return this->m_heightMap1.Get(x, z);
    }

    void render(glm::mat4 VP, glm::vec3 lightDir){

        glUseProgram(programID);

        glUniformMatrix4fv(vpMatrixID, 1, GL_FALSE, &VP[0][0]);
        glUniform1f(minID, this->m_minHeight);
        glUniform1f(maxID, this->m_maxHeight);

        glm::vec3 reversedLightDir = lightDir * -1.0f;
        reversedLightDir = glm::normalize(reversedLightDir);
        glUniform3f(lightDirID, reversedLightDir.x, reversedLightDir.y, reversedLightDir.z);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glUniform1i(textureID, 0);

        glBindVertexArray(VAO);

        glDrawElements(GL_TRIANGLES, (m_depth - 1) * (m_width - 1) * 6, GL_UNSIGNED_INT, NULL);

        glBindVertexArray(0);

    }

    void diamondStep(Terrain* terrain, int rectSize, float currHeight){
        int terrainSize = terrain->m_terrainSize;

        int halfRectSize = rectSize / 2;

        for(int y = 0; y < terrainSize; y += rectSize){
            for(int x = 0; x < terrainSize; x += rectSize){
                int next_x = (x + rectSize) % terrainSize;
                int next_y = (y + rectSize) % terrainSize;

                float topLeft = terrain->m_heightMap.Get(x, y);
                float topRight = terrain->m_heightMap.Get(next_x, y);
                float bottomLeft = terrain->m_heightMap.Get(x, next_y);
                float bottomRight = terrain->m_heightMap.Get(next_x, next_y);

                int mid_x = x + halfRectSize;
                int mid_y = y + halfRectSize;

                float randValue = 0;//randomFloatRange(currHeight, -currHeight);
                float midPoint = (topLeft + topRight + bottomLeft + bottomRight) / 4.0f;

                terrain->m_heightMap.Set(mid_x, mid_y, midPoint + randValue);
            }
        }
    }

    void squareStep(Terrain* terrain, int rectSize, float currHeight){
        int terrainSize = terrain->m_terrainSize;

        int halfRectSize = rectSize / 2;

        for(int y = 0; y < terrainSize; y += rectSize){
            for(int x = 0; x < terrainSize; x += rectSize){
                int next_x = (x + rectSize) % terrainSize;
                int next_y = (y + rectSize) % terrainSize;

                int mid_x = x + halfRectSize;
                int mid_y = y + halfRectSize;

                int prev_mid_x = (x - halfRectSize + terrainSize) % terrainSize;
                int prev_mid_y = (y - halfRectSize + terrainSize) % terrainSize;

                float currTopLeft = terrain->m_heightMap.Get(x, y);
                float currTopRight = terrain->m_heightMap.Get(next_x, y);
                float currCenter = terrain->m_heightMap.Get(mid_x, mid_y);
                float prevYCenter = terrain->m_heightMap.Get(mid_x, prev_mid_y);
                float currBottomLeft = terrain->m_heightMap.Get(x, next_y);
                float prevXCenter = terrain->m_heightMap.Get(prev_mid_x, mid_y);

                float currLeftMid = (currTopLeft + currCenter + currBottomLeft + prevXCenter) / 4.0f + randomFloatRange(-currHeight, currHeight);
                float currTopMid = (currTopLeft + currCenter + currTopRight + prevYCenter) / 4.0f + randomFloatRange(-currHeight, currHeight);

                terrain->m_heightMap.Set(mid_x, y, currTopMid);
                terrain->m_heightMap.Set(x, mid_y, currLeftMid);
            }
        }
    }

    void CreateMidpointDisplacementF32(Terrain* terrain, float roughness){
        int rectSize = terrain->m_terrainSize;
        float currHeight = (float)rectSize / 2.0f;
        float heightReduce = pow(2.0f, -roughness);

        while(rectSize > 0){

            diamondStep(terrain, rectSize, currHeight);

            squareStep(terrain, rectSize, currHeight);

            rectSize /= 2;
            currHeight *= heightReduce;
        }
    }

    void CreateMidpointDisplacement(Terrain* terrain, int terrainSize, float roughness, float minHeight, float maxHeight){

        if(roughness < 0.0f){
            printf("Roughness must be positive!\n");
            exit(0);
        }

        terrain->m_terrainSize = terrainSize;
        terrain->m_minHeight = minHeight;
        terrain->m_maxHeight = maxHeight;

        terrain->m_heightMap.InitArray2D(terrainSize, terrainSize, 0.0f);

        CreateMidpointDisplacementF32(terrain, roughness);

        terrain->m_heightMap.Normalize(minHeight, maxHeight);

        //mirrorHeightMap(terrain);

        terrain->createTriangleList(terrain->m_terrainSize, terrain->m_terrainSize, terrain);
        printf("5.Triangles width: %d, depth: %d\n", this->m_width, this->m_depth);

    }

    void mirrorHeightMap(Terrain* terrain){
        terrain->m_heightMap1.InitArray2D(terrain->m_terrainSize * 2, terrain->m_terrainSize * 2, 0.0f);

        for(int z = 0; z < terrain->m_terrainSize; z++){
            for(int x = 0; x < terrain->m_terrainSize; x++){
                float y = terrain->m_heightMap.Get(x, z);
                terrain->m_heightMap1.Set(x, z, y);
            }
        }

        for(int z = terrain->m_terrainSize; z < terrain->m_terrainSize * 2; z++){
            for(int x = 0; x < terrain->m_terrainSize; x++){
                float y = terrain->m_heightMap.Get(x, (terrain->m_terrainSize * 2) - z);
                terrain->m_heightMap1.Set(x, z, y);
            }
        }
    }

    void calcNormals(std::vector<Vertex>& vertices, std::vector<uint>& indices){
        unsigned int index = 0;

        for(unsigned int i = 0; i < indices.size(); i += 3){
            unsigned int index0 = indices[i];
            unsigned int index1 = indices[i + 1];
            unsigned int index2 = indices[i + 2];
            glm::vec3 v1 = vertices[index1].pos - vertices[index0].pos;
            glm::vec3 v2 = vertices[index2].pos - vertices[index0].pos;
            glm::vec3 normal = glm::cross(v1, v2);
            normal = glm::normalize(normal);

            vertices[index0].norm += normal;
            vertices[index1].norm += normal;
            vertices[index1].norm += normal;
        }

        for(unsigned int i = 0; i < vertices.size(); i++){
            vertices[i].norm = glm::normalize(vertices[i].norm);
        }
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
    eye_center.x = -25.0f;
    eye_center.y = 225.0f;
    eye_center.z = -25.0f;
    viewAzimuth = 0.8f;
    viewPolar = 0.0f;
	lookat.x = eye_center.x + sin(viewAzimuth);
    lookat.z = eye_center.z + cos(viewAzimuth);
    lookat.y = eye_center.y;

    // Creating projection matrix
    glm::float32 FoV = 60;
	glm::float32 zNear = 0.1f; 
	glm::float32 zFar = 10000.0f;
	projectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, zNear, zFar);

}

int main(void){
    
    // Initialising window, glfw and opengl
    if(initWindow() == -1){
        return -1;
    }

    // Setting background colour
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glFrontFace(GL_CW);
    glCullFace(GL_FRONT);
    glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Initialising scene
    //Plane s;
    //s.init();
    AxisXYZ axis;
    axis.init();
    Terrain terrain;
    srand(time(0));
    terrain.initMidPoint();

    glm::mat4 projectionMatrix;

    initCamera(projectionMatrix);

    float light = 0.0f;
    

    do{

        light += 0.0002f;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 viewMatrix = glm::lookAt(eye_center, lookat, up);
		glm::mat4 vp = projectionMatrix * viewMatrix;

        float y = min(-0.4f, cosf(light));
        glm::vec3 lightDir(sinf(light * 5.0f), y, cosf(light * 5.0f));

        //s.render(vp, eye_center, grid_size);
        terrain.render(vp, lightDir);
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
float speed = 1.0f;
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
        eye_center.x += speed * sin(viewAzimuth);
        eye_center.z += speed * cos(viewAzimuth);
        lookat.x = eye_center.x + sin(viewAzimuth);
        lookat.z = eye_center.z + cos(viewAzimuth);
	}

    if (key == GLFW_KEY_S && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
        eye_center.x -= speed * sin(viewAzimuth);
        eye_center.z -= speed * cos(viewAzimuth);
        lookat.x = eye_center.x + sin(viewAzimuth);
        lookat.z = eye_center.z + cos(viewAzimuth);
	}

    if (key == GLFW_KEY_A && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
        float offset = viewAzimuth + pi;
        eye_center.x -= speed * cos(offset);
        eye_center.z += speed * sin(offset);
        lookat.x = eye_center.x + sin(viewAzimuth);
        lookat.z = eye_center.z + cos(viewAzimuth);
	}    

    if (key == GLFW_KEY_D && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
        float offset = viewAzimuth - pi;
        eye_center.x += speed * cos(offset);
        eye_center.z -= speed * sin(offset);
        lookat.x = eye_center.x + sin(viewAzimuth);
        lookat.z = eye_center.z + cos(viewAzimuth);
	}

    if (key == GLFW_KEY_SPACE && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
        eye_center.y += speed;
        lookat.y += speed;
	}

    if (key == GLFW_KEY_LEFT_SHIFT && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
        eye_center.y -= speed;
        lookat.y -= speed;
	}

    if (key == GLFW_KEY_E && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
        speed += 0.5f;
        printf("speed: %f\n", speed);
	} 

    if (key == GLFW_KEY_Q && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
        speed -= 0.5f;
        printf("speed: %f\n", speed);
	}    

    if (key == GLFW_KEY_Z && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
        grid_size -= .005f;
        //printf("grid size: %f\n", grid_size);
	}         

    if (key == GLFW_KEY_X && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
        grid_size += .005f;
        //printf("grid size: %f\n", grid_size);
	} 

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

    //printf("Az: %f, Po: %f, dis: %f\n", viewAzimuth, viewPolar, viewDistance);
    //printf("Eye center : %f, %f, %f\n", eye_center.x, eye_center.y, eye_center.z);
}

float randomFloat(){

    return(float)(rand()) / (float)(RAND_MAX);
}

int randomInt(int a, int b)
{
    if (a > b)
        return randomInt(b, a);
    if (a == b)
        return a;
    return a + (rand() % (b - a));
} 

int randomFloatRange(float a, float b){
    //srand(time(0));

    if(a > b){
        return randomFloatRange(b, a);
    }
    if(a == b){
        return a;
    }

    return(float)randomInt(a, b) + randomFloat();
}

static GLuint LoadTextureTileBox(const char *texture_file_path) {
    int w, h, channels;
    uint8_t* img = stbi_load(texture_file_path, &w, &h, &channels, 3);
    GLuint texture;
    glGenTextures(1, &texture);  
    glBindTexture(GL_TEXTURE_2D, texture);  

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (img) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to load texture " << texture_file_path << std::endl;
    }
    stbi_image_free(img);

    return texture;
}