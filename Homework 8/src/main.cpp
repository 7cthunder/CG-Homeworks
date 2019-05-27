#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <iostream>
#include <math.h>
#include <vector>

struct Point {
	float x;
	float y;
	Point(float x_, float y_) { x = x_; y = y_; }
};

const unsigned int WIDTH = 800;
const unsigned int HEIGHT = 600;
const unsigned int NUM_POINT_TO_PAINT = 100;
const unsigned int UPDATE_EVERY = 50;

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

// Points Vertices
std::vector<Point> points;
// Current Cursor Position
float xpos, ypos;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double x, double y);

float Bernstein(float t, int i, int n);

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
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Bezier Curve", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window." << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);

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
	
	unsigned int VAOs[2], VBOs[2];
	glGenVertexArrays(2, VAOs);
	glGenBuffers(2, VBOs);

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

	// Curve/Point vertices for painting
	float curveVertices[(NUM_POINT_TO_PAINT + 1) * 6];
	float pointVertices[(NUM_POINT_TO_PAINT + 1) * 6];

	// drawType
	// false: for immediately show
	// true: for showing progress
	bool showProgress = false;

	int counter = 0;
	float T = 0.0f;

	while (!glfwWindowShouldClose(window)) {
		processInput(window);
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Bezier Curve");
		ImGui::Text("+-------------------------------+");
		ImGui::Text("| Tips:                         |");
		ImGui::Text("| Left  Mouse Btn Add Point     |");
		ImGui::Text("| Right Mouse Btn Remove Point  |");
		ImGui::Text("+-------------------------------+");

		ImGui::Checkbox("Show Progress", &showProgress);
		ImGui::End();

		// ----------------------------------------------------
		// render
		glClear(GL_COLOR_BUFFER_BIT);

		// Set up vertices
		for (int i = 0; i < points.size(); ++i) {
			pointVertices[6 * i + 0] = points[i].x;
			pointVertices[6 * i + 1] = points[i].y;
			pointVertices[6 * i + 2] = 0.0f;
			// color: (255, 255, 255) -> white
			pointVertices[6 * i + 3] = 1.0f;
			pointVertices[6 * i + 4] = 1.0f;
			pointVertices[6 * i + 5] = 1.0f;
		}

		// for point
		glBindVertexArray(VAOs[0]);
		glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
		glBufferData(GL_ARRAY_BUFFER, points.size() * 6 * sizeof(float), pointVertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		glPointSize(16);
		glDrawArrays(GL_POINTS, 0, points.size());
		glDrawArrays(GL_LINE_STRIP, 0, points.size());

		if (!showProgress) {
			
			for (int j = 0; j <= NUM_POINT_TO_PAINT; ++j) {
				float t = (float)j / NUM_POINT_TO_PAINT;
				float currentX = 0.0f, currentY = 0.0f;
				for (int i = 0; i < points.size(); ++i) {
					float b = Bernstein(t, i, points.size() - 1);
					currentX += points[i].x * b;
					currentY += points[i].y * b;
				}

				curveVertices[6 * j + 0] = currentX;
				curveVertices[6 * j + 1] = currentY;
				curveVertices[6 * j + 2] = 0.0f;
				// color: (255, 255, 255) -> white
				curveVertices[6 * j + 3] = 1.0f;
				curveVertices[6 * j + 4] = 1.0f;
				curveVertices[6 * j + 5] = 1.0f;
			}

			// for curve
			glBindVertexArray(VAOs[1]);
			glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
			glBufferData(GL_ARRAY_BUFFER, (NUM_POINT_TO_PAINT + 1) * 6 * sizeof(float), curveVertices, GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(1);
			glDrawArrays(GL_LINE_STRIP, 0, NUM_POINT_TO_PAINT + 1);
		}
		else {
			if (counter % UPDATE_EVERY == 0) {
				T = (float)counter / (NUM_POINT_TO_PAINT * UPDATE_EVERY);
			}
			counter += 1;
			if (counter > NUM_POINT_TO_PAINT * UPDATE_EVERY) counter = 1;
			
			std::vector<Point> tmpPoints1(points);

			while (tmpPoints1.size() > 1) {
				int pointsCounter = 0;
				std::vector<Point> tmpPoints2;
				for (int i = 0; i < tmpPoints1.size() - 1; ++i) {
					float
						b0 = Bernstein(T, 0, 1),
						b1 = Bernstein(T, 1, 1);
					float
						tmpX = b0 * tmpPoints1[i].x + b1 * tmpPoints1[i + 1].x,
						tmpY = b0 * tmpPoints1[i].y + b1 * tmpPoints1[i + 1].y;
					tmpPoints2.push_back(Point(tmpX, tmpY));
					pointVertices[6 * pointsCounter + 0] = tmpX;
					pointVertices[6 * pointsCounter + 1] = tmpY;
					pointVertices[6 * pointsCounter + 2] = 0.0f;
					// color: (0, 255, 0) -> green
					pointVertices[6 * pointsCounter + 3] = 0.0f;
					pointVertices[6 * pointsCounter + 4] = 1.0f;
					pointVertices[6 * pointsCounter + 5] = 0.0f;
					pointsCounter += 1;
				}

				// for point
				glBindVertexArray(VAOs[0]);
				glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
				glBufferData(GL_ARRAY_BUFFER, pointsCounter * 6 * sizeof(float), pointVertices, GL_STATIC_DRAW);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
				glEnableVertexAttribArray(1);
				glPointSize(4);
				glDrawArrays(GL_POINTS, 0, pointsCounter);
				glDrawArrays(GL_LINE_STRIP, 0, pointsCounter);

				tmpPoints1.assign(tmpPoints2.begin(), tmpPoints2.end());
			}

			
			for (int j = 0; j <= counter/UPDATE_EVERY; ++j) {
				float t = (float)j / NUM_POINT_TO_PAINT;
				float currentX = 0.0f, currentY = 0.0f;
				for (int i = 0; i < points.size(); ++i) {
					float b = Bernstein(t, i, points.size() - 1);
					currentX += points[i].x * b;
					currentY += points[i].y * b;
				}

				curveVertices[6 * j + 0] = currentX;
				curveVertices[6 * j + 1] = currentY;
				curveVertices[6 * j + 2] = 0.0f;
				// color: (255, 0, 0) -> red
				curveVertices[6 * j + 3] = 1.0f;
				curveVertices[6 * j + 4] = 0.0f;
				curveVertices[6 * j + 5] = 0.0f;
			}

			// for curve
			glBindVertexArray(VAOs[1]);
			glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
			glBufferData(GL_ARRAY_BUFFER, (counter / UPDATE_EVERY + 1) * 6 * sizeof(float), curveVertices, GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(1);
			glDrawArrays(GL_LINE_STRIP, 0, counter / UPDATE_EVERY + 1);
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

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (action == GLFW_PRESS) {
		switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT:
			points.push_back(Point(xpos, ypos));
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			points.pop_back();
			break;
		default:
			break;
		}
	}
}

void cursor_position_callback(GLFWwindow* window, double x, double y) {
	xpos = float( (x - WIDTH  / 2) / WIDTH)  * 2;
	ypos = float(-(y - HEIGHT / 2) / HEIGHT) * 2;
}

float Bernstein(float t, int i, int n) {
	float nFactorial = 1.0,
		  iFactorial = 1.0,
		  niFactorial = 1.0;
	for (int k = n; k > 1; k--) {
		if (k <= n - i) niFactorial *= k;
		if (k <= i) iFactorial *= k;
		nFactorial *= k;
	}
	return nFactorial * pow(t, i) * pow(1 - t, n - i) / (iFactorial * niFactorial);
}