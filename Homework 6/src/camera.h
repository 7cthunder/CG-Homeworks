#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <math.h>

// Default camera options
const float PITCH       =  0.0f;
const float YAW         = -90.0f;
const float SPEED       =  2.5f;
const float SENSITIVITY =  0.1f;
const float ZOOM        =  45.0f;

// Defines several possible options for camera movement. 
// Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

class Camera {
public:
	// Camera attributes
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;
	// Euler Angles
	float Pitch;
	float Yaw;
	// Camera options
	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;

	// Constructor with vectors
	Camera(
		glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
		float yaw = YAW, 
		float pitch = PITCH
	) : 
		Front(glm::vec3(0.0f, 0.0f, -1.0f)),
		MovementSpeed(SPEED),
		MouseSensitivity(SENSITIVITY),
		Zoom(ZOOM) 
	{
		Position = position;
		WorldUp = up;
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	}

	// Constructor with scalar values
	Camera(
		float posX, float posY, float posZ,
		float upX,  float upY,  float upZ,
		float yaw,  float pitch
	) : 
		Front(glm::vec3(0.0f, 0.0f, -1.0f)),
		MovementSpeed(SPEED),
		MouseSensitivity(SENSITIVITY),
		Zoom(ZOOM)
	{
		Position = glm::vec3(posX, posY, posZ);
		WorldUp = glm::vec3(upX, upY, upZ);
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	}

	// Returns the LookAt Matrix
	glm::mat4 getViewMatrix() {
		return glm::lookAt(Position, Position + Front, WorldUp);
	}

	// Process keyboard input
	void processKeyboard(Camera_Movement direction, float deltaTime) {
		float velocity = MovementSpeed * deltaTime;
		if (direction == FORWARD)  Position += Front * velocity;
		if (direction == BACKWARD) Position -= Front * velocity;
		if (direction == LEFT)     Position -= Right * velocity;
		if (direction == RIGHT)    Position += Right * velocity;
	}

	// Processes mouse input
	void processMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true) {
		xoffset *= MouseSensitivity;
		yoffset *= MouseSensitivity;

		Pitch += yoffset;
		Yaw   += xoffset;

		// Make sure that when pitch is out of bounds, screen doesn't get flipped
		if (constrainPitch) {
			if (Pitch >  89.0f)  Pitch =  89.0f;
			if (Pitch < -89.0f)  Pitch = -89.0f;
		}

		// Update Front, Right and Up Vectors
		updateCameraVectors();
	}

	// Process mouse scroll-wheel event
	void processMouseScroll(float yoffset) {
		if (Zoom >= 1.0f && Zoom <= 60.0f) Zoom -= yoffset;
		if (Zoom < 1.0f)  Zoom = 1.0f;
		if (Zoom > 60.0f) Zoom = 60.0f;
	}

private:
	// Calculates the front vector from the Camera's Euler Angles
	void updateCameraVectors() {
		glm::vec3 front;
		front.x = cos(glm::radians(Pitch)) * cos(glm::radians(Yaw));
		front.y = sin(glm::radians(Pitch));
		front.z = cos(glm::radians(Pitch)) * sin(glm::radians(Yaw));
		Front = glm::normalize(front);
		// Re-calculate the Right and Up vector
		Right = glm::normalize(glm::cross(Front, WorldUp));
		Up    = glm::normalize(glm::cross(Right, Front));
	}
};

#endif // !CAMERA_H