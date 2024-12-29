#include <stdio.h>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <util/array2d.h>
#include <util/util.h>

#include <render/shader.h>

#include<assimp/Importer.hpp>
#include<assimp/scene.h>
#include<assimp/postprocess.h>

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

static GLuint LoadTerrainTexture(const char *texture_file_path);
static GLuint LoadTextureTileBox(const char *texture_file_path);

struct Skybox {
	glm::vec3 position;		// Position of the box 
	glm::vec3 scale;		// Size of the box in each axis
	
	GLfloat vertex_buffer_data[72] = {	// Vertex definition for a canonical box
		// Front face
		-1.0f, -1.0f, 1.0f, 
		1.0f, -1.0f, 1.0f, 
		1.0f, 1.0f, 1.0f, 
		-1.0f, 1.0f, 1.0f, 
		
		// Back face 
		1.0f, -1.0f, -1.0f, 
		-1.0f, -1.0f, -1.0f, 
		-1.0f, 1.0f, -1.0f, 
		1.0f, 1.0f, -1.0f,
		
		// Left face
		-1.0f, -1.0f, -1.0f, 
		-1.0f, -1.0f, 1.0f, 
		-1.0f, 1.0f, 1.0f, 
		-1.0f, 1.0f, -1.0f, 

		// Right face 
		1.0f, -1.0f, 1.0f, 
		1.0f, -1.0f, -1.0f, 
		1.0f, 1.0f, -1.0f, 
		1.0f, 1.0f, 1.0f,

		// Top face
		-1.0f, 1.0f, 1.0f, 
		1.0f, 1.0f, 1.0f, 
		1.0f, 1.0f, -1.0f, 
		-1.0f, 1.0f, -1.0f, 

		// Bottom face
		-1.0f, -1.0f, -1.0f, 
		1.0f, -1.0f, -1.0f, 
		1.0f, -1.0f, 1.0f, 
		-1.0f, -1.0f, 1.0f, 
	};

	GLfloat color_buffer_data[72] = {
		// Front, red
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,

		// Back, yellow
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,

		// Left, green
		0.0f, 1.0f, 0.0f, 
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,

		// Right, cyan
		0.0f, 1.0f, 1.0f, 
		0.0f, 1.0f, 1.0f, 
		0.0f, 1.0f, 1.0f, 
		0.0f, 1.0f, 1.0f, 

		// Top, blue
		0.0f, 0.0f, 1.0f, 
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,

		// Bottom, magenta
		1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 
		1.0f, 0.0f, 1.0f, 
		1.0f, 0.0f, 1.0f,  
	};

	GLuint index_buffer_data[36] = {		// 12 triangle faces of a box
		0, 1, 2, 	
		0, 2, 3, 
		
		4, 5, 6, 
		4, 6, 7, 

		8, 9, 10, 
		8, 10, 11, 

		12, 13, 14, 
		12, 14, 15, 

		16, 17, 18, 
		16, 18, 19, 

		20, 21, 22, 
		20, 22, 23, 
	};

    // TODO: Define UV buffer data
    // ---------------------------
	GLfloat uv_buffer_data[48]={

		// Front
		1.0f, 0.665f,
		0.75f, 0.665f,
		0.75f, 0.334f,
		1.0f, 0.334f,

		// Back
 		0.5f, 0.665f,
		0.25f, 0.665f,
		0.25f, 0.334f,
		0.5f, 0.334f,

		// Left
        0.25f, 0.665f,
		0.0f, 0.665f,
		0.0f, 0.334f,
		0.25f, 0.334f,		

		// Right
        0.75f, 0.665f,
		0.5f, 0.665f,
		0.5f, 0.334f,
		0.75f, 0.334f,

		// Top - we do not want texture the top
		0.251f, 0.0f,
		0.499f, 0.0f,
		0.499f, 0.334f,
		0.251f, 0.334f,		

		// Bottom - we do not want texture the bottom
		0.251f, 0.665f,
		0.499f, 0.665f,
		0.499f, 1.0f,
		0.251f, 1.0f,
	};
    // ---------------------------
    
	// OpenGL buffers
	GLuint vertexArrayID; 
	GLuint vertexBufferID; 
	GLuint indexBufferID; 
	GLuint colorBufferID;
	GLuint uvBufferID;
	GLuint textureID;

	// Shader variable IDs
	GLuint mvpMatrixID;
	GLuint textureSamplerID;
	GLuint programID;

	void initialize(glm::vec3 position, glm::vec3 scale) {
		// Define scale of the building geometry
		this->position = position;
		this->scale = scale;

		// Create a vertex array object
		glGenVertexArrays(1, &vertexArrayID);
		glBindVertexArray(vertexArrayID);

		// Create a vertex buffer object to store the vertex data		
		glGenBuffers(1, &vertexBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_STATIC_DRAW);

		// Create a vertex buffer object to store the color data
        // TODO: 
		glGenBuffers(1, &colorBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(color_buffer_data), color_buffer_data, GL_STATIC_DRAW);

		// TODO: Create a vertex buffer object to store the UV data
		// --------------------------------------------------------
		glGenBuffers(1, &uvBufferID);
 		glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(uv_buffer_data), uv_buffer_data, GL_STATIC_DRAW);
        // --------------------------------------------------------

		// Create an index buffer object to store the index data that defines triangle faces
		glGenBuffers(1, &indexBufferID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data), index_buffer_data, GL_STATIC_DRAW);

		// Create and compile our GLSL program from the shaders
		this->programID = LoadShadersFromFile("../proj/box.vert", "../proj/box.frag");
		if (this->programID == 0)
		{
			std::cerr << "Failed to load shaders." << std::endl;
		}

		// Get a handle for our "MVP" uniform
		mvpMatrixID = glGetUniformLocation(this->programID, "MVP");

        // TODO: Load a texture 
        // --------------------

		this->textureID = LoadTextureTileBox("../proj/textures/sky.png");

        // --------------------

        // TODO: Get a handle to texture sampler 
        // -------------------------------------
		textureSamplerID = glGetUniformLocation(this->programID,"textureSampler");
        // -------------------------------------
	}

	void render(glm::mat4& cameraMatrix) {
		glUseProgram(this->programID);

        glBindVertexArray(this->vertexArrayID);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);

		// TODO: Model transform 
		// -----------------------
        glm::mat4 modelMatrix = glm::mat4();    
		modelMatrix = glm::translate(modelMatrix, eye_center);
        // Scale the box along each axis to make it look like a building
        modelMatrix = glm::scale(modelMatrix, scale);
        // -----------------------

		// Set model-view-projection matrix
		glm::mat4 mvp = cameraMatrix * modelMatrix;
		glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);

		// TODO: Enable UV buffer and texture sampler
		// ------------------------------------------
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

		// Set textureSampler to use texture unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, this->textureID);
		glUniform1i(textureSamplerID, 0); 

        // ------------------------------------------

		// Draw the box
		glDrawElements(
			GL_TRIANGLES,      // mode
			36,    			   // number of indices
			GL_UNSIGNED_INT,   // type
			(void*)0           // element array buffer offset
		);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);

        glBindVertexArray(0);
	}

	void cleanup() {
		glDeleteBuffers(1, &vertexBufferID);
		glDeleteBuffers(1, &colorBufferID);
		glDeleteBuffers(1, &indexBufferID);
		glDeleteVertexArrays(1, &vertexArrayID);
		//glDeleteBuffers(1, &uvBufferID);
		//glDeleteTextures(1, &textureID);
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
    glm::vec3 lightDir = glm::vec3(1.0f, -1.0f, 1.0f);
    glm::vec3 offset = glm::vec3();

     
    
    GLuint VAO;
    GLuint vertexBuffer;
    GLuint indexBuffer;

    GLuint programID;
    GLuint mvpMatrixID;
    GLuint minID;
    GLuint maxID;
    GLuint textureID;
    GLuint texSamplerID;
    GLuint lightDirID;

    struct Vertex{
        glm::vec3 pos;
        glm::vec2 tex;
        glm::vec3 norm = glm::vec3(0.0f, 0.0f, 0.0f);

        void initVertex(float y, int x, int z, float scale, float terrainSize, int orientation){
            this->pos = glm::vec3(x * scale, y, z * scale);
            float terrainScale = terrainSize / 1024;

            if(orientation == 0){
                tex = glm::vec2(fmod((float)x / (terrainSize / terrainScale), 1), fmod((float)z / (terrainSize / terrainScale), 1));
            }
            // Flip uv around x axis
            else if(orientation == 1){
                tex = glm::vec2(1.0f - fmod((float)x / (terrainSize / terrainScale), 1), fmod((float)z / (terrainSize / terrainScale), 1));
            }
            // Flip uv around z axis
            else if(orientation == 2){
                tex = glm::vec2(fmod((float)x / (terrainSize / terrainScale), 1), 1.0f - fmod((float)z / (terrainSize / terrainScale), 1));
            }
            // Flip uv around XZ
            else if(orientation == 3){
                tex = glm::vec2(1.0f - fmod((float)x / (terrainSize / terrainScale), 1), 1.0f - fmod((float)z / (terrainSize / terrainScale), 1));
            }
        }
    };

    std::vector<Vertex> vertices;

    Array2D<float> getHeightMap(){
        return this->m_heightMap;
    }

    void init(glm::vec3 offset){
        this->offset = offset;
        this->m_worldScale = 1.0f;
        // Create and compile our GLSL program from the shaders
        this->programID = LoadShadersFromFile("../proj/terrain.vert", "../proj/terrain.frag");

        if (this->programID == 0){
            std::cerr << "Failed to load shaders." << std::endl;
        }

        printf("1. Shaders loaded\n");

        this->textureID = LoadTerrainTexture("../proj/textures/texture5.jpg");

        this->mvpMatrixID = glGetUniformLocation(this->programID, "MVP");
        this->minID = glGetUniformLocation(this->programID, "minHeight");
        this->maxID = glGetUniformLocation(this->programID, "maxHeight");
        this->texSamplerID = glGetUniformLocation(this->programID, "texSampler");
        this->lightDirID = glGetUniformLocation(this->programID, "reversedLightDir");

    }

    void initTile(Array2D<float>& heightMap, int terrainSize, float minHeight, float maxHeight, glm::vec3 offset, int orientation){


        this->init(offset);

        this->m_terrainSize = terrainSize;
        this->m_minHeight = minHeight;
        this->m_maxHeight = maxHeight;
        this->m_heightMap = heightMap;

        this->createTriangleList(this->m_terrainSize, this->m_terrainSize, this);
        printf("5.Triangles width: %d, depth: %d\n", this->m_width, this->m_depth);

        this->populateBuffers(orientation);
    }

    void initMidPoint(glm::vec3 offset){

        this->init(offset);

        CreateMidpointDisplacement(this, 1024, 1.2, 1, 300);

        this->populateBuffers(0);
    }

    void initFromFile(){

        this->init(offset);

        this->LoadFromFile("../data/heightmap.save");

        this->populateBuffers(0);

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
        glBindVertexArray(this->VAO);

        glGenBuffers(1, &vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, this->vertexBuffer);

        glGenBuffers(1, &indexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->indexBuffer);

        int pos_loc = 0;
        int tex_loc = 1;
        int norm_loc = 2;

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

    void populateBuffers(int orientation){

        this->vertices.resize(this->m_width * this->m_depth);
        printf("6.Vertices size: %d\n", this->vertices.size());
        initVertices(orientation);

        std::vector<unsigned int> indices;
        int  numQuads = (this->m_width - 1) * (this->m_depth - 1);
        indices.resize(numQuads * 6);
        initIndices(indices);

        calcNormals(this->vertices, indices);

        glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(this->vertices[0]), &this->vertices[0], GL_STATIC_DRAW);

        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), &indices[0], GL_STATIC_DRAW);
    }

    void initIndices(std::vector<unsigned int>& indices){
        int index = 0;

        for(int z = 0; z < this->m_depth - 1; z++){
            for(int x = 0; x < this->m_width - 1; x++){
                unsigned int bottomLeft = z * this->m_width + x;
                unsigned int topLeft = (z + 1) * this->m_width + x;
                unsigned int topRight = (z + 1) * this->m_width + (x + 1);
                unsigned int bottomRight = z * this->m_width + (x + 1);

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

    void initVertices(int orientation){
        int index = 0;

        for(int z = 0; z < this->m_depth; z++) {
            for(int x = 0; x < this->m_width; x++){
                assert(index < this->vertices.size());
                this->vertices[index].initVertex(this->getHeight(x, z), x, z, this->m_worldScale, this->m_terrainSize, orientation);
                //printf("x: %f, y: %f, z: %f, u: %f, v: %f\n", this->vertices[index].pos.x, this->vertices[index].pos.y, this->vertices[index].pos.z, this->vertices[index].tex.x, this->vertices[index].tex.y);
                index++;
            }
        }
        printf("7. Vertices initialised!\n");
    }

    float getHeight(int x, int z) const{
        return this->m_heightMap.Get(x, z);
    }

    void render(glm::mat4 VP, glm::vec3 lightDir){
        
        glUseProgram(this->programID);

        glBindVertexArray(this->VAO);

        glBindBuffer(GL_ARRAY_BUFFER, this->vertexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->indexBuffer);

        int numFloats = 0;

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(numFloats * sizeof(float)));
        numFloats += 3;

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(numFloats * sizeof(float)));
        numFloats += 2;

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(numFloats * sizeof(float)));
        numFloats += 3;

        glm::mat4 modelMatrix = glm::mat4();
        modelMatrix = glm::translate(modelMatrix, this->offset);

        glm::mat4 MVP = VP * modelMatrix;

        glUniformMatrix4fv(this->mvpMatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniform1f(this->minID, this->m_minHeight);
        glUniform1f(this->maxID, this->m_maxHeight);
        glm::vec3 reversedLightDir = lightDir * -1.0f;
        reversedLightDir = glm::normalize(reversedLightDir);
        glUniform3f(lightDirID, reversedLightDir.x, reversedLightDir.y, reversedLightDir.z);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, this->textureID);
        glUniform1i(this->texSamplerID, 0);

        glDrawElements(GL_TRIANGLES, (this->m_depth - 1) * (this->m_width - 1) * 6, GL_UNSIGNED_INT, NULL);

        glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);

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

                float randValue = randomFloatRange(currHeight, -currHeight);
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
    eye_center.x = 500.0f;
    eye_center.y = 300.0f;
    eye_center.z = 500.0f;
    viewAzimuth = 0.8f;
    viewPolar = 0.0f;
	lookat.x = eye_center.x + sin(viewAzimuth);
    lookat.z = eye_center.z + cos(viewAzimuth);
    lookat.y = eye_center.y;

    // Creating projection matrix
    glm::float32 FoV = 60;
	glm::float32 zNear = 0.1f; 
	glm::float32 zFar = 1000.0f;
	projectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, zNear, zFar);

}


void resetEye(Terrain* terrain);

Array2D<float> flipHeightMapX(Array2D<float>& heightMap, float terrainSize);
Array2D<float> flipHeightMapZ(Array2D<float>& heightMap, float terrainSize);
Array2D<float> copyHeightMap(Array2D<float>& heightMap, float terrainSize);


int main(void){
    
    // Initialising window, glfw and opengl
    if(initWindow() == -1){
        return -1;
    }

    // Setting background colour
    glClearColor(0.2f, 0.2f, 0.25f, 0.0f);

    glFrontFace(GL_CW);
    glCullFace(GL_FRONT);
    glEnable(GL_DEPTH_TEST);


    Skybox b;
    b.initialize(glm::vec3(0, 0, 0), glm::vec3(256, 256, 256));

    Terrain terrain;
    srand(time(0));
    terrain.initMidPoint(glm::vec3(0, 0, 0));

    Terrain tileMinusX1;
    Terrain tileMinusX2;

    Terrain tilePlusX1;
    Terrain tilePlusX2;

    Terrain tileMinusZ1;
    Terrain tileMinusZ2;

    Terrain tilePlusZ1;
    Terrain tilePlusZ2;

    Terrain tilePlusX1PlusZ1;
    Terrain tilePlusX2PlusZ1;
    Terrain tilePlusX1PlusZ2;
    Terrain tilePlusX2PlusZ2;

    Terrain tilePlusX1MinusZ1;
    Terrain tilePlusX2MinusZ1;
    Terrain tilePlusX1MinusZ2;
    Terrain tilePlusX2MinusZ2;

    Terrain tileMinusX1PlusZ1;
    Terrain tileMinusX2PlusZ1;
    Terrain tileMinusX1PlusZ2;
    Terrain tileMinusX2PlusZ2;

    Terrain tileMinusX1MinusZ1;
    Terrain tileMinusX2MinusZ1;
    Terrain tileMinusX1MinusZ2;
    Terrain tileMinusX2MinusZ2;

    Array2D<float> originalHeightMap = copyHeightMap(terrain.m_heightMap, terrain.m_terrainSize);
    Array2D<float> flippedHeightMapZ = flipHeightMapZ(originalHeightMap, terrain.m_terrainSize);
    Array2D<float> flippedHeightMapX = flipHeightMapX(originalHeightMap, terrain.m_terrainSize);
    Array2D<float> flippedHeightMapXZ = flipHeightMapX(flippedHeightMapZ, terrain.m_terrainSize);

    
    tilePlusX1.initTile(flippedHeightMapZ, terrain.m_terrainSize, terrain.m_minHeight, terrain.m_maxHeight, glm::vec3(terrain.m_terrainSize - 1.0f, 0, 0), 1);
    tilePlusX2.initTile(originalHeightMap, terrain.m_terrainSize, terrain.m_minHeight, terrain.m_maxHeight, glm::vec3((terrain.m_terrainSize - 1.0f) * 2.0f, 0, 0), 1);

    tileMinusX1.initTile(flippedHeightMapZ, terrain.m_terrainSize, terrain.m_minHeight, terrain.m_maxHeight, glm::vec3(1.0f - terrain.m_terrainSize, 0, 0), 2);
    tileMinusX2.initTile(originalHeightMap, terrain.m_terrainSize, terrain.m_minHeight, terrain.m_maxHeight, glm::vec3((1.0f - terrain.m_terrainSize) * 2.0f, 0, 0), 2);
 
    tilePlusZ1.initTile(flippedHeightMapX, terrain.m_terrainSize, terrain.m_minHeight, terrain.m_maxHeight, glm::vec3(0, 0, terrain.m_terrainSize - 1.0f), 1);
    tilePlusZ2.initTile(originalHeightMap, terrain.m_terrainSize, terrain.m_minHeight, terrain.m_maxHeight, glm::vec3(0, 0, (terrain.m_terrainSize - 1.0f) * 2.0f), 1);

    tileMinusZ1.initTile(flippedHeightMapX, terrain.m_terrainSize, terrain.m_minHeight, terrain.m_maxHeight, glm::vec3(0, 0, 1.0f - terrain.m_terrainSize), 1);
    tileMinusZ2.initTile(originalHeightMap, terrain.m_terrainSize, terrain.m_minHeight, terrain.m_maxHeight, glm::vec3(0, 0, (1.0f - terrain.m_terrainSize) * 2.0f), 1);

    tilePlusX1PlusZ1.initTile(flippedHeightMapXZ, terrain.m_terrainSize, terrain.m_minHeight, terrain.m_maxHeight, glm::vec3(terrain.m_terrainSize - 1.0f, 0, terrain.m_terrainSize - 1.0f), 3);
    tilePlusX2PlusZ1.initTile(flippedHeightMapX, terrain.m_terrainSize, terrain.m_minHeight, terrain.m_maxHeight, glm::vec3((terrain.m_terrainSize - 1.0f) * 2.0f, 0, terrain.m_terrainSize - 1.0f), 1);
    tilePlusX1PlusZ2.initTile(flippedHeightMapZ, terrain.m_terrainSize, terrain.m_minHeight, terrain.m_maxHeight, glm::vec3(terrain.m_terrainSize - 1.0f, 0, (terrain.m_terrainSize - 1.0f) * 2.0f), 2);
    tilePlusX2PlusZ2.initTile(originalHeightMap, terrain.m_terrainSize, terrain.m_minHeight, terrain.m_maxHeight, glm::vec3((terrain.m_terrainSize - 1.0f) * 2.0f, 0, (terrain.m_terrainSize - 1.0f) * 2.0f), 0);

    tilePlusX1MinusZ1.initTile(flippedHeightMapXZ, terrain.m_terrainSize, terrain.m_minHeight, terrain.m_maxHeight, glm::vec3(terrain.m_terrainSize - 1.0f, 0, 1.0f - terrain.m_terrainSize), 3);
    tilePlusX2MinusZ1.initTile(flippedHeightMapX, terrain.m_terrainSize, terrain.m_minHeight, terrain.m_maxHeight, glm::vec3((terrain.m_terrainSize - 1.0f) * 2.0f, 0, 1.0f - terrain.m_terrainSize), 1);
    tilePlusX1MinusZ2.initTile(flippedHeightMapZ, terrain.m_terrainSize, terrain.m_minHeight, terrain.m_maxHeight, glm::vec3(terrain.m_terrainSize - 1.0f, 0, (1.0f - terrain.m_terrainSize) * 2.0f), 2);
    tilePlusX2MinusZ2.initTile(originalHeightMap, terrain.m_terrainSize, terrain.m_minHeight, terrain.m_maxHeight, glm::vec3((terrain.m_terrainSize - 1.0f) * 2.0f, 0, (1.0f - terrain.m_terrainSize) * 2.0f), 0);

    tileMinusX1PlusZ1.initTile(flippedHeightMapXZ, terrain.m_terrainSize, terrain.m_minHeight, terrain.m_maxHeight, glm::vec3(1.0f - terrain.m_terrainSize, 0, terrain.m_terrainSize - 1.0f), 3);
    tileMinusX2PlusZ1.initTile(flippedHeightMapX, terrain.m_terrainSize, terrain.m_minHeight, terrain.m_maxHeight, glm::vec3((1.0f - terrain.m_terrainSize) * 2.0f, 0, terrain.m_terrainSize - 1.0f), 1);
    tileMinusX1PlusZ2.initTile(flippedHeightMapZ, terrain.m_terrainSize, terrain.m_minHeight, terrain.m_maxHeight, glm::vec3(1.0f - terrain.m_terrainSize, 0, (terrain.m_terrainSize - 1.0f) * 2.0f), 2);
    tileMinusX2PlusZ2.initTile(originalHeightMap, terrain.m_terrainSize, terrain.m_minHeight, terrain.m_maxHeight, glm::vec3((1.0f - terrain.m_terrainSize) * 2.0f, 0, (terrain.m_terrainSize - 1.0f) * 2.0f), 0);

    tileMinusX1MinusZ1.initTile(flippedHeightMapXZ, terrain.m_terrainSize, terrain.m_minHeight, terrain.m_maxHeight, glm::vec3(1.0f - terrain.m_terrainSize, 0, 1.0f - terrain.m_terrainSize), 3);
    tileMinusX2MinusZ1.initTile(flippedHeightMapX, terrain.m_terrainSize, terrain.m_minHeight, terrain.m_maxHeight, glm::vec3((1.0f - terrain.m_terrainSize) * 2.0f, 0, 1.0f - terrain.m_terrainSize), 1);
    tileMinusX1MinusZ2.initTile(flippedHeightMapZ, terrain.m_terrainSize, terrain.m_minHeight, terrain.m_maxHeight, glm::vec3(1.0f - terrain.m_terrainSize, 0, (1.0f - terrain.m_terrainSize) * 2.0f), 2);
    tileMinusX2MinusZ2.initTile(originalHeightMap, terrain.m_terrainSize, terrain.m_minHeight, terrain.m_maxHeight, glm::vec3((1.0f - terrain.m_terrainSize) * 2.0f, 0, (1.0f - terrain.m_terrainSize) * 2.0f), 0);


    glm::mat4 projectionMatrix;
    initCamera(projectionMatrix);

    float light = 0.0f;
    
    do{

        resetEye(&terrain);

        light += 0.001f;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 viewMatrix = glm::lookAt(eye_center, lookat, up);
		glm::mat4 vp = projectionMatrix * viewMatrix;

        float y = min(-0.4f, cosf(light));
        glm::vec3 lightDir(sinf(light * 5.0f), y, cosf(light * 5.0f));

        //s.render(vp, eye_center, grid_size);
        glDepthFunc(GL_LEQUAL);
        glDepthMask(GL_FALSE);
        b.render(vp);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS); 

        terrain.render(vp, lightDir);

        tilePlusX1.render(vp, lightDir);
        tilePlusX2.render(vp, lightDir);

        tileMinusX1.render(vp, lightDir);
        tileMinusX2.render(vp, lightDir);

        tilePlusZ1.render(vp, lightDir);
        tilePlusZ2.render(vp, lightDir);

        tileMinusZ1.render(vp, lightDir);
        tileMinusZ2.render(vp, lightDir);

        tilePlusX1PlusZ1.render(vp, lightDir);
        tilePlusX2PlusZ1.render(vp, lightDir);
        tilePlusX1PlusZ2.render(vp, lightDir);
        tilePlusX2PlusZ2.render(vp, lightDir);

        tilePlusX1MinusZ1.render(vp, lightDir);
        tilePlusX2MinusZ1.render(vp, lightDir);
        tilePlusX1MinusZ2.render(vp, lightDir);
        tilePlusX2MinusZ2.render(vp, lightDir);

        tileMinusX1PlusZ1.render(vp, lightDir);
        tileMinusX2PlusZ1.render(vp, lightDir);
        tileMinusX1PlusZ2.render(vp, lightDir);
        tileMinusX2PlusZ2.render(vp, lightDir);

        tileMinusX1MinusZ1.render(vp, lightDir);
        tileMinusX2MinusZ1.render(vp, lightDir);
        tileMinusX1MinusZ2.render(vp, lightDir);
        tileMinusX2MinusZ2.render(vp, lightDir);

        // Swap buffers
        glfwSwapBuffers(window);

        // Polling for events
        glfwPollEvents();

    }

    // Check if esc was pressed or window was closed
    while(!glfwWindowShouldClose(window));

    printf("Goodbye Scene!");

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

static GLuint LoadTerrainTexture(const char *texture_file_path) {
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

void resetEye(Terrain* terrain){
    if(eye_center.x >= terrain->m_terrainSize * 2){
        eye_center.x -= terrain->m_terrainSize * 2;
    }

    if(eye_center.z >= terrain->m_terrainSize * 2){
        eye_center.z -= terrain->m_terrainSize * 2;
    }

    if(eye_center.x <= 0 - terrain->m_terrainSize){
        eye_center.x += terrain->m_terrainSize * 2;
    } 

    if(eye_center.z <= 0 - terrain->m_terrainSize){
        eye_center.z += terrain->m_terrainSize * 2;
    } 
}

Array2D<float> flipHeightMapX(Array2D<float>& heightMap, float terrainSize){
    
    Array2D<float> out;

    out.InitArray2D(terrainSize, terrainSize, 0.0f);

    for(int z = 0; z < (int)terrainSize; z ++){
        for(int x = 0; x < terrainSize; x++){
            float y = heightMap.Get(x, terrainSize - 1 - z);
            out.Set(x, z, y);
            
        }
    }

    return out;

}

Array2D<float> flipHeightMapZ(Array2D<float>& heightMap, float terrainSize){
    
    Array2D<float> out;

    out.InitArray2D(terrainSize, terrainSize, 0.0f);

    for(int z = 0; z < (int)terrainSize; z ++){
        for(int x = 0; x < terrainSize; x++){
            float y = heightMap.Get(terrainSize - 1 - x, z);
            out.Set(x, z, y);
            
        }
    }

    return out;

}

Array2D<float> copyHeightMap(Array2D<float>& heightMap, float terrainSize){
    Array2D<float> out;

    out.InitArray2D(terrainSize, terrainSize, 0.0f);

    for(int z = 0; z < (int)terrainSize; z ++){
        for(int x = 0; x < terrainSize; x++){
            float y = heightMap.Get(x, z);
            out.Set(x, z, y);
            
        }
    }

    return out;

}

static GLuint LoadTextureTileBox(const char *texture_file_path) {
    int w, h, channels;
    uint8_t* img = stbi_load(texture_file_path, &w, &h, &channels, 3);
    GLuint texture;
    glGenTextures(1, &texture);  
    glBindTexture(GL_TEXTURE_2D, texture);  

    // To tile textures on a box, we set wrapping to repeat
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
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



