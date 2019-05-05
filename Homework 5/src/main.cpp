#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "camera.h"
#include "shader.h"

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// default setting
const unsigned int WIDTH = 800;
const unsigned int HEIGHT = 600;

const char* glsl_version = "#version 330 core";

// Global camera
Camera camera(glm::vec3(0.0f, 0.0f, 10.0f));
float
	lastX = WIDTH  / 2.0f,
	lastY = HEIGHT / 2.0f;
bool firstMouse = true;

// time tools
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main() {
	//----------------------------------------------------------------
	// Initialize and configure GLFW
	// Version: 3.3
	// Profile: CORE
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create GLFW window
	// Width:  800
	// Height: 600
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "HW5", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window." << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// Use GLAD to load OpenGL function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// Setup ImGui Context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;

	// Setup ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	// configure global opengl states
	glEnable(GL_DEPTH_TEST);

	// build and compile shader program
	Shader shader("shader.vs", "shader.fs");

	// vertex data
	float vertices[] = {
		-2.0f, -2.0f, -2.0f, 1.0f, 0.0f, 0.0f,
		 2.0f, -2.0f, -2.0f, 1.0f, 0.0f, 0.0f,
		 2.0f,  2.0f, -2.0f, 1.0f, 0.0f, 0.0f,
		 2.0f,  2.0f, -2.0f, 1.0f, 0.0f, 0.0f,
		-2.0f,  2.0f, -2.0f, 1.0f, 0.0f, 0.0f,
		-2.0f, -2.0f, -2.0f, 1.0f, 0.0f, 0.0f,

		-2.0f, -2.0f,  2.0f, 1.0f, 1.0f, 0.0f,
		 2.0f, -2.0f,  2.0f, 1.0f, 1.0f, 0.0f,
		 2.0f,  2.0f,  2.0f, 1.0f, 1.0f, 0.0f,
		 2.0f,  2.0f,  2.0f, 1.0f, 1.0f, 0.0f,
		-2.0f,  2.0f,  2.0f, 1.0f, 1.0f, 0.0f,
		-2.0f, -2.0f,  2.0f, 1.0f, 1.0f, 0.0f,

		-2.0f,  2.0f,  2.0f, 0.0f, 1.0f, 0.0f,
		-2.0f,  2.0f, -2.0f, 0.0f, 1.0f, 0.0f,
		-2.0f, -2.0f, -2.0f, 0.0f, 1.0f, 0.0f,
		-2.0f, -2.0f, -2.0f, 0.0f, 1.0f, 0.0f,
		-2.0f, -2.0f,  2.0f, 0.0f, 1.0f, 0.0f,
		-2.0f,  2.0f,  2.0f, 0.0f, 1.0f, 0.0f,

		 2.0f,  2.0f,  2.0f, 0.0f, 1.0f, 1.0f,
		 2.0f,  2.0f, -2.0f, 0.0f, 1.0f, 1.0f,
		 2.0f, -2.0f, -2.0f, 0.0f, 1.0f, 1.0f,
		 2.0f, -2.0f, -2.0f, 0.0f, 1.0f, 1.0f,
		 2.0f, -2.0f,  2.0f, 0.0f, 1.0f, 1.0f,
		 2.0f,  2.0f,  2.0f, 0.0f, 1.0f, 1.0f,

		-2.0f, -2.0f, -2.0f, 0.0f, 0.0f, 1.0f,
		 2.0f, -2.0f, -2.0f, 0.0f, 0.0f, 1.0f,
		 2.0f, -2.0f,  2.0f, 0.0f, 0.0f, 1.0f,
		 2.0f, -2.0f,  2.0f, 0.0f, 0.0f, 1.0f,
		-2.0f, -2.0f,  2.0f, 0.0f, 0.0f, 1.0f,
		-2.0f, -2.0f, -2.0f, 0.0f, 0.0f, 1.0f,

		-2.0f,  2.0f, -2.0f, 1.0f, 0.0f, 1.0f,
		 2.0f,  2.0f, -2.0f, 1.0f, 0.0f, 1.0f,
		 2.0f,  2.0f,  2.0f, 1.0f, 0.0f, 1.0f,
		 2.0f,  2.0f,  2.0f, 1.0f, 0.0f, 1.0f,
		-2.0f,  2.0f,  2.0f, 1.0f, 0.0f, 1.0f,
		-2.0f,  2.0f, -2.0f, 1.0f, 0.0f, 1.0f,
	};

	unsigned int VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	int type = 0;

	// Default Orthographic Projection options
	// ---------------------------------------
	float
		left   = -20.0f,
		right  =  20.0f,
		bottom = -20.0f,
		top    =  20.0f,
		nearP  =   0.1f,
		farP   = 100.0f;

	// Default Perspective Projection options
	// --------------------------------------
	float
		fov    =  45.0f,
		nearP2 =   0.1f,
		farP2  = 100.0f;

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window)) {
		// frame time
		float current = glfwGetTime();
		deltaTime = current - lastFrame;
		lastFrame = current;

		// input
		processInput(window);

		// ImGui
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// render
		// ------
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// activate shader
		shader.use();

		// create menu
		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("Transform")) {
				if (ImGui::MenuItem("Ortho")) { type = 1; }
				if (ImGui::MenuItem("Persp")) { type = 2; }
				if (ImGui::MenuItem("View"))  { type = 3; }
				if (ImGui::MenuItem("FPS"))   { type = 4; }
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 view  = glm::mat4(1.0f);
		glm::mat4 proj  = glm::mat4(1.0f);

		// Orthographic Projection
		// -----------------------
		if (type == 1) {
			ImGui::Begin("Orthographic Projection");
			ImGui::SliderFloat("Left",   &left,   -20.0f,   0.0f);
			ImGui::SliderFloat("Right",  &right,    0.0f,  20.0f);
			ImGui::SliderFloat("Bottom", &bottom, -20.0f,   0.0f);
			ImGui::SliderFloat("Top",    &top,      0.0f,  20.0f);
			ImGui::SliderFloat("Near",   &nearP,    0.1f,  10.0f);
			ImGui::SliderFloat("Far",    &farP,    10.0f, 100.0f);
			ImGui::End(); 

			view = glm::translate(view, glm::vec3(0.0f, 0.0f, -10.0f));
			proj = glm::ortho(left, right, bottom, top, nearP, farP);
		}
		// Perspective Projection
		// -----------------------
		if (type == 2) {
			ImGui::Begin("Orthographic Projection");
			ImGui::SliderFloat("Fov",  &fov,    1.0f,  60.0f);
			ImGui::SliderFloat("Near", &nearP2, 0.1f,  10.0f);
			ImGui::SliderFloat("Far",  &farP2, 10.0f, 100.0f);
			ImGui::End();

			view = glm::translate(view, glm::vec3(0.0f, 0.0f, -10.0f));
			proj = glm::perspective(glm::radians(fov), (float)WIDTH / (float)HEIGHT, nearP2, farP2);
		}
		// View Changing
		// -------------
		if (type == 3) {
			float radius = 15.0f;
			float camX = sin(glfwGetTime()) * radius;
			float camZ = cos(glfwGetTime()) * radius;
			view = glm::lookAt(glm::vec3(camX, 0.0f, camZ), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			proj = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
		}
		// FPS mode
		// --------
		if (type == 4) {
			view = camera.getViewMatrix();
			proj = glm::perspective(glm::radians(camera.Zoom), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
		}

		if (type != 0) {
			if (type == 1 || type == 2) {
				// move the cube from (0, 0, 0) to (-1.5, 0.5, -1.5)
				model = glm::translate(model, glm::vec3(-1.5f, 0.5f, -1.5f));
			}
			
			// Rotate it by a certain Angle for the sake of observation
			model = glm::rotate(model, glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 1.0f));
			
			// pass matrices to shader
			shader.setMat4("model", model);
			shader.setMat4("view", view);
			shader.setMat4("projection", proj);

			glBindVertexArray(VAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// cleanup
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

// keyboard input event callback
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.processKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.processKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.processKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.processKeyboard(RIGHT, deltaTime);
}

// mouse movement event callback
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.processMouseMovement(xoffset, yoffset);
}

// mouse scroll event callback
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.processMouseScroll(yoffset);
}