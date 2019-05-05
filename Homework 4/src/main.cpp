#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "shader.h"

#include <iostream>
#include <math.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

const unsigned int WIDTH = 800;
const unsigned int HEIGHT = 600;

const char* glsl_version = "#version 330 core";

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
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Transform", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window." << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

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

		-2.0f, -2.0f,  2.0f, 1.0f, 0.0f, 0.0f,
		 2.0f, -2.0f,  2.0f, 1.0f, 0.0f, 0.0f,
		 2.0f,  2.0f,  2.0f, 1.0f, 0.0f, 0.0f,
		 2.0f,  2.0f,  2.0f, 1.0f, 0.0f, 0.0f,
		-2.0f,  2.0f,  2.0f, 1.0f, 0.0f, 0.0f,
		-2.0f, -2.0f,  2.0f, 1.0f, 0.0f, 0.0f,

		-2.0f,  2.0f,  2.0f, 0.0f, 1.0f, 0.0f,
		-2.0f,  2.0f, -2.0f, 0.0f, 1.0f, 0.0f,
		-2.0f, -2.0f, -2.0f, 0.0f, 1.0f, 0.0f,
		-2.0f, -2.0f, -2.0f, 0.0f, 1.0f, 0.0f,
		-2.0f, -2.0f,  2.0f, 0.0f, 1.0f, 0.0f,
		-2.0f,  2.0f,  2.0f, 0.0f, 1.0f, 0.0f,

		 2.0f,  2.0f,  2.0f, 0.0f, 1.0f, 0.0f,
		 2.0f,  2.0f, -2.0f, 0.0f, 1.0f, 0.0f,
		 2.0f, -2.0f, -2.0f, 0.0f, 1.0f, 0.0f,
		 2.0f, -2.0f, -2.0f, 0.0f, 1.0f, 0.0f,
		 2.0f, -2.0f,  2.0f, 0.0f, 1.0f, 0.0f,
		 2.0f,  2.0f,  2.0f, 0.0f, 1.0f, 0.0f,

		-2.0f, -2.0f, -2.0f, 0.0f, 0.0f, 1.0f,
		 2.0f, -2.0f, -2.0f, 0.0f, 0.0f, 1.0f,
		 2.0f, -2.0f,  2.0f, 0.0f, 0.0f, 1.0f,
		 2.0f, -2.0f,  2.0f, 0.0f, 0.0f, 1.0f,
		-2.0f, -2.0f,  2.0f, 0.0f, 0.0f, 1.0f,
		-2.0f, -2.0f, -2.0f, 0.0f, 0.0f, 1.0f,

		-2.0f,  2.0f, -2.0f, 0.0f, 0.0f, 1.0f,
		 2.0f,  2.0f, -2.0f, 0.0f, 0.0f, 1.0f,
		 2.0f,  2.0f,  2.0f, 0.0f, 0.0f, 1.0f,
		 2.0f,  2.0f,  2.0f, 0.0f, 0.0f, 1.0f,
		-2.0f,  2.0f,  2.0f, 0.0f, 0.0f, 1.0f,
		-2.0f,  2.0f, -2.0f, 0.0f, 0.0f, 1.0f,
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

	int transform_type = 0;

	// place projection outside the render loop
	shader.use();
	glm::mat4 projection = glm::mat4(1.0f);
	projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
	shader.setMat4("projection", projection);

	// render loop
	while (!glfwWindowShouldClose(window)) {
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

		// create menu
		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("Transform")) {
				if (ImGui::MenuItem("Translation")) { transform_type = 1; }
				if (ImGui::MenuItem("Rotation")) { transform_type = 2; }
				if (ImGui::MenuItem("Scaling")) { transform_type = 3; }
				if (ImGui::MenuItem("Combination")) { transform_type = 4; }
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 view = glm::mat4(1.0f);

		// Translation
		// -----------
		if (transform_type == 1) {
			model = glm::translate(model, glm::vec3((float)sin(glfwGetTime()) * 4, 0.0f, 0.0f));
		}
		// Rotation
		// --------
		else if (transform_type == 2) {
			model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(1.0f, 0.0f, 1.0f));
		}
		// Scaling
		// -------
		else if (transform_type == 3) {
			float scale = (float)sin(glfwGetTime()) / 2 + 1;
			model = glm::scale(model, glm::vec3(scale, scale, scale));
		}
		// Combination
		// -----------
		else if (transform_type == 4) {
			// Zoom out
			float surrounding_object_scale = 0.5;
			view = glm::translate(view, glm::vec3(0.0f, 0.0f, -20.0f));
			model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::translate(model, glm::vec3(0.0f, 0.0f, 15.0f));
			model = glm::rotate(model, (float)glfwGetTime() * 5, glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::scale(model, glm::vec3(surrounding_object_scale, surrounding_object_scale, surrounding_object_scale));
			
			// centering object
			float centering_object_scale = 1.2;
			glm::mat4 model2 = glm::mat4(1.0f);
			model2 = glm::rotate(model2, (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
			model2 = glm::rotate(model2, glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 1.0f));
			model2 = glm::scale(model2, glm::vec3(centering_object_scale, centering_object_scale, centering_object_scale));
			shader.setMat4("model", model2);
			// render centering object
			glBindVertexArray(VAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		if (transform_type != 0) {
			// Rotate it by a certain Angle for the sake of observation
			model = glm::rotate(model, glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 1.0f));
			view = glm::translate(view, glm::vec3(0.0f, 0.0f, -20.0f));
			// pass these matrices to shaders
			shader.setMat4("model", model);
			shader.setMat4("view", view);
			// render box
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

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}