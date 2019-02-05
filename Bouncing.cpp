// Includes and defs --------------------------

#define STB_IMAGE_IMPLEMENTATION
// openGL functionality
#include <glad/glad.h>
#include <GLFW/glfw3.h>
	// shader helper
#include "shader.h"

#include <math.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb/stb_image.h>

// Functions
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

// Global variables ---------------------------

const int SCR_WIDTH = 1280;
const int SCR_HEIGHT = 720;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float yaw = 0.0f, pitch = 0.0f;
bool firstMouse = true;
float lastX = 400, lastY = 300;

float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

glm::vec3 ballPos = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 ballVel = glm::vec3(5.0f, 12.0f, -3.0f);

glm::vec3 grav = glm::vec3(0.0f, -9.8f, 0.0f);

int main()
{
	// Before loop starts ---------------------
	// glfw init
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// glfw window creation
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "5611 HW1", NULL, NULL);
	glfwMakeContextCurrent(window);

	// register callbacks
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	//Register mouse movement callback
	glfwSetCursorPosCallback(window, mouse_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Initialize glad
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	Shader ballShader("baseShader.vert", "baseShader.frag");
	Shader wallShader("textured.vert", "textured.frag");

	// Things to render -------------------------
	// Sphere vertices
	const int xSegments = 50;
	const int ySegments = 50;
	float sphereVertices[xSegments * ySegments * 3];

	float PI = 3.14159265f;
	for (unsigned int y = 0; y < ySegments; y++)
	{
		for (unsigned int x = 0; x < xSegments; x++)
		{
			float xSegment = (float)x / (float)xSegments;
			float ySegment = (float)y / (float)ySegments;

			int index = (y*xSegments + x) * 3;
			sphereVertices[index] = std::cos(xSegment * PI*2.0f) * std::sin(ySegment * PI); // x pos
			sphereVertices[index + 1] = std::cos(ySegment * PI); // y pos
			sphereVertices[index + 2] = std::sin(xSegment * PI*2.0f) * std::sin(ySegment * PI); // z pos
		}
	}
	// Sphere indices
	int sphereIndices[(xSegments) * (ySegments) * 6]; // Make sure this is sized correctly
	for (unsigned int y = 0; y < (ySegments); y++) // Dont go below last row
	{
		for (unsigned int x = 0; x < (xSegments); x++) // Dont go after last column
		{
			int index = (y*xSegments + x) * 6;
			sphereIndices[index] = ((y)*xSegments) + x;			// 0
			sphereIndices[index+1] = ((y+1)*xSegments) + x;		// |
			sphereIndices[index+2] = ((y+1)*xSegments) + x+1;	// 0--0

			sphereIndices[index+3] = ((y)*xSegments) + x;		// 0--0
			sphereIndices[index+4] = ((y)*xSegments) + x+1;		//  \ |
			sphereIndices[index+5] = ((y+1)*xSegments) + x+1;	//    0
		}
	}
	// Buffer stuff for sphere
	unsigned int sphereVAO, sphereVBO, sphereEBO;
	glGenVertexArrays(1, &sphereVAO);
	glBindVertexArray(sphereVAO);
	glGenBuffers(1, &sphereVBO);
	glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sphereVertices), sphereVertices, GL_STATIC_DRAW);
	glGenBuffers(1, &sphereEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphereIndices), sphereIndices, GL_STATIC_DRAW);
	// Tell OpenGL how to use vertex data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); //Uses whatever VBO is bound to GL_ARRAY_BUFFER
	glEnableVertexAttribArray(0);

	// Floor
	float floorVertices[] = {
		-5.0f, 0.0f, -5.0f, 0.0f, 0.0f,
		-5.0f, 0.0f, 5.0f, 0.0f, 2.0f,
		5.0f, 0.0f, -5.0f, 2.0f, 0.0f,
		5.0f, 0.0f, 5.0f, 2.0f, 2.0f
	};
	int floorIndices[] = {
		0, 1, 2,
		1, 2, 3
	};
	// Buffer stuff for floor
	unsigned int floorVAO, floorVBO, floorEBO;
	glGenVertexArrays(1, &floorVAO);
	glBindVertexArray(floorVAO);
	glGenBuffers(1, &floorVBO);
	glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);
	glGenBuffers(1, &floorEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, floorEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(floorIndices), floorIndices, GL_STATIC_DRAW);
	// Tell OpenGL how to use vertex data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0); //Uses whatever VBO is bound to GL_ARRAY_BUFFER
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	// Floor texture
	// Set up textures
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load and generate the texture
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char *data = stbi_load("grid.png", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
	wallShader.setInt("texture", texture);

	glEnable(GL_DEPTH_TEST);

	// uncomment this call to draw in wireframe polygons.
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// render loop ----------------------------
	while (!glfwWindowShouldClose(window))
	{
		// Set deltaT
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		processInput(window);

		//Update positions
		ballVel += deltaTime * grav;
		ballPos += deltaTime * ballVel;
		float radius = 0.5f;
		if (ballPos[1] <= -3.0f + radius)
		{
			ballPos[1] = -3.0f + radius;
			ballVel[1] *= -0.95f;
		}
		if (ballPos[1] >= 7.0f - radius)
		{
			ballPos[1] = 7.0f - radius;
			ballVel[1] *= -0.95f;
		}
		if (ballPos[0] <= -5.0f + radius)
		{
			ballPos[0] = -5.0f + radius;
			ballVel[0] *= -0.95f;
		}
		if (ballPos[0] >= 5.0f - radius)
		{
			ballPos[0] = 5.0f - radius;
			ballVel[0] *= -0.95f;
		}
		if (ballPos[2] <= -5.0f + radius)
		{
			ballPos[2] = -5.0f + radius;
			ballVel[2] *= -0.95f;
		}
		if (ballPos[2] >= 5.0f - radius)
		{
			ballPos[2] = 5.0f - radius;
			ballVel[2] *= -0.95f;
		}

		// rendering commands here
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glClear(GL_COLOR_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		glm::mat4 view;
		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		glm::mat4 projection = glm::mat4(1.0f);
		projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

		ballShader.use();
		ballShader.setMat4("view", view);
		ballShader.setMat4("projection", projection);
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, ballPos);
		model = glm::scale(model, glm::vec3(radius, radius, radius));
		ballShader.setMat4("model", model);
		glBindVertexArray(sphereVAO);
		glDrawElements(GL_TRIANGLES, (xSegments) * (ySegments) * 6, GL_UNSIGNED_INT, 0);
		
		wallShader.use();
		wallShader.setMat4("view", view);
		wallShader.setMat4("projection", projection);
		// Floor
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -3.0f, 0.0f));
		wallShader.setMat4("model", model);
		glBindVertexArray(floorVAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -3.0f, 0.0f));
		wallShader.setMat4("model", model);
		// Ceiling
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 7.0f, 0.0f));
		wallShader.setMat4("model", model);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		// Wall1
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(5.0f, 2.0f, 0.0f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f,0.0f,1.0f));
		wallShader.setMat4("model", model);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		// Wall2
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-5.0f, 2.0f, 0.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		wallShader.setMat4("model", model);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		// Wall3
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 2.0f, 5.0f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		wallShader.setMat4("model", model);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		// Wall4
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 2.0f, -5.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		wallShader.setMat4("model", model);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


		// check and call events and swap the buffers
		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}

// This function is called whenever window is resized
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// Process all ketboard input here
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);


	float cameraSpeed = 2.5f * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cameraPos += cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cameraPos -= cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.2;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);
}