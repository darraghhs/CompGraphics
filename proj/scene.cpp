#include <stdio.h>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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
static glm::vec3 eye_center(0.0f, 0.0f, 0.0f);
static glm::vec3 lookat(0, 0, 0);
static glm::vec3 up(0, 1, 0);

// View control 
static float viewAzimuth = 0.5f;
static float viewPolar = 0.5f;
static float viewDistance = 50.0f;

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

	void initialize() {
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

struct Scene {

    // Buffers
    GLuint vertexArrayID; 
	GLuint vertexBufferID; 

    // Shader variables
    GLuint vpMatrixID;
    GLuint programID;

    void init(){

        // Creating VAO
		glGenVertexArrays(1, &vertexArrayID);
		glBindVertexArray(vertexArrayID);

        // Creating VBO
        //glGenBuffers(1, &vertexBufferID);
		//glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);

        // Create and compile our GLSL program from the shaders
        programID = LoadShadersFromFile("../proj/scene.vert", "../proj/scene.frag");

        if (programID == 0){
            std::cerr << "Failed to load shaders." << std::endl;
        }

        vpMatrixID = glGetUniformLocation(programID, "VP");
    }

    void render(glm::mat4 cameraMatrix){

        // Use shaders
        glUseProgram(programID);

        // Enable VBO
        glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

        // Passing vp to shader
        glUniformMatrix4fv(vpMatrixID, 1, GL_FALSE, &cameraMatrix[0][0]);

        	glDrawElements(
			GL_TRIANGLES,      // mode
			36,    			   // number of indices
			GL_UNSIGNED_INT,   // type
			(void*)0           // element array buffer offset
		);

        glDisableVertexAttribArray(0);
    }

    void cleanup() {
	    glDeleteBuffers(1, &vertexBufferID);
	    //glDeleteBuffers(1, &colorBufferID);
	    //glDeleteBuffers(1, &indexBufferID);
	    //glDeleteVertexArrays(1, &vertexArrayID);
	    //glDeleteBuffers(1, &uvBufferID);
	    //glDeleteTextures(1, &textureID);
	    glDeleteProgram(programID);
	}

};



int main(void){
    
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

    // Setting background colour
    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

    glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

    // Initialising scene
    //Scene s;
    //s.init();
    AxisXYZ axis;
    axis.initialize();

    // Camera setup
    /*
    eye_center.y = viewDistance * cos(viewPolar);
    eye_center.x = viewDistance * cos(viewAzimuth);
    eye_center.z = viewDistance * sin(viewAzimuth);
    */

    glm::float32 FoV = 60;
	glm::float32 zNear = 0.1f; 
	glm::float32 zFar = 10000.0f;
	glm::mat4 projectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, zNear, zFar);
    

    do{

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 viewMatrix = glm::lookAt(eye_center, lookat, up);
		glm::mat4 vp = projectionMatrix * viewMatrix;

        //s.render();
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
		viewAzimuth = 0.f;
		viewPolar = 0.f;
		eye_center.y = viewDistance * cos(viewPolar);
		eye_center.x = viewDistance * cos(viewAzimuth);
		eye_center.z = viewDistance * sin(viewAzimuth);
		std::cout << "Reset." << std::endl;
	}

	if (key == GLFW_KEY_UP && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		viewPolar -= 0.1f;
		eye_center.y = viewDistance * cos(viewPolar);
	}

	if (key == GLFW_KEY_DOWN && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		viewPolar += 0.1f;
		eye_center.y = viewDistance * cos(viewPolar);
	}

	if (key == GLFW_KEY_LEFT && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		viewAzimuth -= 0.1f;
		eye_center.x = viewDistance * cos(viewAzimuth);
		eye_center.z = viewDistance * sin(viewAzimuth);
	}

	if (key == GLFW_KEY_RIGHT && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		viewAzimuth += 0.1f;
		eye_center.x = viewDistance * cos(viewAzimuth);
		eye_center.z = viewDistance * sin(viewAzimuth);
	}

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

    printf("Az: %f, Po: %f\n", viewAzimuth, viewPolar);
    printf("Eye center : %f, %f, %f\n", eye_center.x, eye_center.y, eye_center.z);
}