#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <iostream>
#include <math.h>

const unsigned int WIDTH = 600;
const unsigned int HEIGHT = 600;
const int MESH_NUM = 21;
const float SCALE = 0.9;

const char* glsl_version = "#version 330 core";

const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aColor;\n"
"out vec3 ourColor;\n"
"void main() {\n"
"	gl_Position = vec4(aPos, 1.0);\n"
"	ourColor = aColor;\n"
"}\n";

const char* fragmentShaderSource = "#version 330 core\n"
"in vec3 ourColor;\n"
"out vec4 FragColor;\n"
"void main() {\n"
"	FragColor = vec4(ourColor, 1.0f);\n"
"}\n";

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

int Bresenham_line(int x0, int y0, int x1, int y1, float* &points, float scale);
void plotLineLow(int x0, int y0, int x1, int y1, float* &points, float scale);
void plotLineHigh(int x0, int y0, int x1, int y1, float* &points, float scale);

int Bresenham_circle(int radius, float* &points, float scale);

void setMesh(float mesh_vertices_row[], float mesh_vertices_col[], float scale);

void setZs(float points[], int length, float value, int step);
void setColors(float points[], int length, float r, float g, float b);

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
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Bresenham", NULL, NULL);
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

	// Setup mesh
	float
		mesh_vertices_row[MESH_NUM * 6 * 2] = { 0 },
		mesh_vertices_col[MESH_NUM * 6 * 2] = { 0 };

	setMesh(mesh_vertices_row, mesh_vertices_col, SCALE);

	unsigned int VAOs[2], VBOs[2];
	glGenVertexArrays(2, VAOs);
	glGenBuffers(2, VBOs);

	// First VAO for row mesh
	glBindVertexArray(VAOs[0]);

	glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(mesh_vertices_row), mesh_vertices_row, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Second VAO for col mesh
	glBindVertexArray(VAOs[1]);

	glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(mesh_vertices_col), mesh_vertices_col, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// build and compile shader program
	// -------------------- vertex shader ---------------------------
	int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR: VERTEX SHADER COMPILATION FAILED!\n" << infoLog << std::endl;
	}
	// -------------------- fragment shader --------------------------
	int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR: FRAGMENT SHADER COMPILATION FAILED!\n" << infoLog << std::endl;
	}
	// --------------------- shader program ---------------------------
	int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR: SHADER PROGRAM LINKING FAILED!" << infoLog << std::endl;
	}
	// delete shaders after linking
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glUseProgram(shaderProgram);

	int primitive_type = 0;
	// line vertices
	int x1 = -1, y1 = -1, x2 = 1, y2 = 1, x3 = 0, y3 = 0;
	int radius = 1;

	while (!glfwWindowShouldClose(window)) {
		processInput(window);
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// ----------------------------------------------------
		// render
		glClear(GL_COLOR_BUFFER_BIT);
		glPointSize(16);
		// Draw row mesh
		glBindVertexArray(VAOs[0]);
		glDrawArrays(GL_LINES, 0, MESH_NUM * 2);
		// Draw col mesh
		glBindVertexArray(VAOs[1]);
		glDrawArrays(GL_LINES, 0, MESH_NUM * 2);

		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("Primitives")) {
				if (ImGui::MenuItem("Line")) { primitive_type = 1; }
				if (ImGui::MenuItem("Triangle")) { primitive_type = 2; }
				if (ImGui::MenuItem("Circle")) { primitive_type = 3; }
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		if (primitive_type == 1) {
			ImGui::Begin("Line Input");

			ImGui::BeginChild("X1", ImVec2(150, 20), FALSE);
			ImGui::SliderInt("X1", &x1, -MESH_NUM / 2, MESH_NUM / 2);
			ImGui::EndChild();
			ImGui::SameLine();

			ImGui::BeginChild("Y1", ImVec2(150, 20), FALSE);
			ImGui::SliderInt("Y1", &y1, -MESH_NUM / 2, MESH_NUM / 2);
			ImGui::EndChild();

			ImGui::BeginChild("X2", ImVec2(150, 20), FALSE);
			ImGui::SliderInt("X2", &x2, -MESH_NUM / 2, MESH_NUM / 2);
			ImGui::EndChild();
			ImGui::SameLine();

			ImGui::BeginChild("Y2", ImVec2(150, 20), FALSE);
			ImGui::SliderInt("Y2", &y2, -MESH_NUM / 2, MESH_NUM / 2);
			ImGui::EndChild();

			ImGui::End();
			
			float* points = NULL;

			int length = Bresenham_line(x1, y1, x2, y2, points, SCALE);
			setZs(points, length, 0.0f, 6);
			setColors(points, length, 1.0f, 0.0f, 0.0f);

			unsigned int LINE_VAO, LINE_VBO;
			glGenVertexArrays(1, &LINE_VAO);
			glGenBuffers(1, &LINE_VBO);

			glBindVertexArray(LINE_VAO);

			glBindBuffer(GL_ARRAY_BUFFER, LINE_VBO);
			glBufferData(GL_ARRAY_BUFFER, length * sizeof(float), points, GL_STATIC_DRAW);

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(1);

			glPointSize(16);
			glDrawArrays(GL_POINTS, 0, length / 6);

			delete[]points;
		}
		else if (primitive_type == 2) {
			ImGui::Begin("Triangle Input");
			ImGui::BeginChild("X1", ImVec2(150, 20), FALSE);
			ImGui::SliderInt("X1", &x1, -MESH_NUM / 2, MESH_NUM / 2);
			ImGui::EndChild();
			ImGui::SameLine();

			ImGui::BeginChild("Y1", ImVec2(150, 20), FALSE);
			ImGui::SliderInt("Y1", &y1, -MESH_NUM / 2, MESH_NUM / 2);
			ImGui::EndChild();

			ImGui::BeginChild("X2", ImVec2(150, 20), FALSE);
			ImGui::SliderInt("X2", &x2, -MESH_NUM / 2, MESH_NUM / 2);
			ImGui::EndChild();
			ImGui::SameLine();

			ImGui::BeginChild("Y2", ImVec2(150, 20), FALSE);
			ImGui::SliderInt("Y2", &y2, -MESH_NUM / 2, MESH_NUM / 2);
			ImGui::EndChild();
			
			ImGui::BeginChild("X3", ImVec2(150, 20), FALSE);
			ImGui::SliderInt("X3", &x3, -MESH_NUM / 2, MESH_NUM / 2);
			ImGui::EndChild();
			ImGui::SameLine();

			ImGui::BeginChild("Y3", ImVec2(150, 20), FALSE);
			ImGui::SliderInt("Y3", &y3, -MESH_NUM / 2, MESH_NUM / 2);
			ImGui::EndChild();

			float 
				*points_1 = NULL,
				*points_2 = NULL,
				*points_3 = NULL;

			int length = Bresenham_line(x1, y1, x2, y2, points_1, SCALE);
			setZs(points_1, length, 0.0f, 6);
			setColors(points_1, length, 1.0f, 0.0f, 0.0f);

			unsigned int LINE_VAO, LINE_VBO;
			glGenVertexArrays(1, &LINE_VAO);
			glGenBuffers(1, &LINE_VBO);

			glBindVertexArray(LINE_VAO);

			glBindBuffer(GL_ARRAY_BUFFER, LINE_VBO);
			glBufferData(GL_ARRAY_BUFFER, length * sizeof(float), points_1, GL_STATIC_DRAW);

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(1);
			glDrawArrays(GL_POINTS, 0, length / 6);

			length = Bresenham_line(x1, y1, x3, y3, points_2, SCALE);
			setZs(points_2, length, 0.0f, 6);
			setColors(points_2, length, 1.0f, 0.0f, 0.0f);
			glBufferData(GL_ARRAY_BUFFER, length * sizeof(float), points_2, GL_STATIC_DRAW);
			glDrawArrays(GL_POINTS, 0, length / 6);

			length = Bresenham_line(x2, y2, x3, y3, points_3, SCALE);
			setZs(points_3, length, 0.0f, 6);
			setColors(points_3, length, 1.0f, 0.0f, 0.0f);
			glBufferData(GL_ARRAY_BUFFER, length * sizeof(float), points_3, GL_STATIC_DRAW);
			glDrawArrays(GL_POINTS, 0, length / 6);

			delete[]points_1;
			delete[]points_2;
			delete[]points_3;

			ImGui::End();
		}
		else if (primitive_type == 3) {
			ImGui::Begin("Circle Input");
			ImGui::SliderInt("Radius", &radius, 1, MESH_NUM / 2);

			float* points = NULL;
			int length = Bresenham_circle(radius, points, SCALE);
			setZs(points, length, 0.0f, 6);
			setColors(points, length, 1.0f, 0.0f, 0.0f);

			unsigned int CIRCLE_VAO, CIRCLE_VBO;
			glGenVertexArrays(1, &CIRCLE_VAO);
			glGenBuffers(1, &CIRCLE_VBO);

			glBindVertexArray(CIRCLE_VAO);

			glBindBuffer(GL_ARRAY_BUFFER, CIRCLE_VBO);
			glBufferData(GL_ARRAY_BUFFER, length * sizeof(float), points, GL_STATIC_DRAW);

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(1);

			glPointSize(16);
			glDrawArrays(GL_POINTS, 0, length / 6);

			delete[]points;

			ImGui::End();
			
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// swap buffers and poll IO events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// cleanup
	glDeleteVertexArrays(2, VAOs);
	glDeleteBuffers(2, VBOs);
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(1, 1, width, height);
}

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

int Bresenham_line(int x0, int y0, int x1, int y1, float* &points, float scale) {
	scale = scale * 2 / (MESH_NUM - 1);
	// |dy/dx| < 1
	if (abs(y1 - y0) < abs(x1 - x0)) {
		points = new float[(abs(x1 - x0) + 1) * 6];
		if (x0 > x1)
			plotLineLow(x1, y1, x0, y0, points, scale);
		else
			plotLineLow(x0, y0, x1, y1, points, scale);
		return (abs(x1 - x0) + 1) * 6;
	}
	else {
		points = new float[(abs(y1 - y0) + 1) * 6];
		if (y0 > y1)
			plotLineHigh(x1, y1, x0, y0, points, scale);
		else
			plotLineHigh(x0, y0, x1, y1, points, scale);
		return (abs(y1 - y0) + 1) * 6;
	}
}

void plotLineLow(int x0, int y0, int x1, int y1, float* &points, float scale) {
	int dx = x1 - x0,
		dy = y1 - y0,
		yi = 1;
	if (dy < 0) {
		yi = -1;
		dy = -dy;
	}
	int D = 2 * dy - dx,
		y = y0;
	for (int i = 0; i <= dx; ++i) {
		points[i * 6] = (x0 + i) * scale;
		points[i * 6 + 1] = y * scale;
		if (D > 0) {
			y = y + yi;
			D = D - 2 * dx;
		}
		D = D + 2 * dy;
	}
}

void plotLineHigh(int x0, int y0, int x1, int y1, float* &points, float scale) {
	int dx = x1 - x0,
		dy = y1 - y0,
		xi = 1;
	if (dx < 0) {
		xi = -1;
		dx = -dx;
	}
	int D = 2 * dx - dy,
		x = x0;
	for (int i = 0; i <= dy; ++i) {
		points[i * 6] = x * scale;
 		points[i * 6 + 1] = (y0 + i) * scale;
		if (D > 0) {
			x = x + xi;
			D = D - 2 * dy;
		}
		D = D + 2 * dx;
	}
}

void plot8CirclePoints(float x, float y, float* points) {
	points[0] = x;    points[1] = y;
	points[6] = -x;   points[7] = y;
	points[12] = -x;  points[13] = -y;
	points[18] = x;   points[19] = -y;
	points[24] = y;   points[25] = x;
	points[30] = -y;  points[31] = x;
	points[36] = -y;  points[37] = -x;
	points[42] = y;   points[43] = -x;
}

int Bresenham_circle(int radius, float* &points, float scale) {
	scale = scale * 2 / (MESH_NUM - 1);

	points = new float[radius * 48];

	int x = radius,
		y = 0,
		xchange = 1 - 2 * radius,
		ychange = 1,
		radius_error = 0,
		count = 0;
	
	while (x >= y) {
		plot8CirclePoints(x * scale, y * scale, points + count * 8 * 6);
		++y;
		radius_error += ychange;
		ychange += 2;
		if (2 * radius_error + xchange > 0) {
			--x;
			radius_error += xchange;
			xchange += 2;
		}
		++count;
	}

	return count * 8 * 6;
}

void setMesh(float mesh_vertices_row[], float mesh_vertices_col[], float scale) {
	for (int i = 0; i < MESH_NUM * 6 * 2; i += 12) {
		mesh_vertices_row[i] = -scale;
		mesh_vertices_row[i + 1] = (i / 12) * (2 * scale / ((float)MESH_NUM - 1)) - scale;
		mesh_vertices_row[i + 6] = scale;
		mesh_vertices_row[i + 7] = mesh_vertices_row[i + 1];

		mesh_vertices_col[i] = (i / 12) * (2 * scale / ((float)MESH_NUM - 1)) - scale;
		mesh_vertices_col[i + 1] = -scale;
		mesh_vertices_col[i + 6] = mesh_vertices_col[i];
		mesh_vertices_col[i + 7] = scale;

		for (int j = 3; j < 6; ++j) {
			mesh_vertices_row[i + j] = 1.0f;
			mesh_vertices_col[i + j] = 1.0f;
			mesh_vertices_row[i + 6 + j] = 1.0f;
			mesh_vertices_col[i + 6 + j] = 1.0f;
		}
	}
	setColors(mesh_vertices_row, MESH_NUM * 6 * 2, 1.0f, 1.0f, 1.0f);
	setColors(mesh_vertices_col, MESH_NUM * 6 * 2, 1.0f, 1.0f, 1.0f);
}

void setZs(float points[], int length, float value, int step) {
	for (int i = 0; i < length; i += step)
		points[i + 2] = value;
}

void setColors(float points[], int length, float r, float g, float b) {
	for (int i = 0; i < length; i += 6) {
		points[i + 3] = r;
		points[i + 4] = g;
		points[i + 5] = b;
	}
}