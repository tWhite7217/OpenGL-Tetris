#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <chrono>

#include <GL/glew.h>

#include <GLFW/glfw3.h>
GLFWwindow *window;

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/texture.hpp>
// #include <common/controls.hpp>
#include <common/objloader.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

#include "TetrisGame.h"

using namespace std::chrono_literals;

// GLuint Texture;
// GLuint TextureID;
GLuint MVPMatrixID;
GLuint MMatrixID;
GLuint VMatrixID;
GLuint LightID;
GLuint vertexbuffer;
GLuint uvbuffer;
GLuint normalbuffer;

GLuint camera_line_vertexbuffer;

GLuint piece_type_flag_id;

std::vector<glm::vec3> vertices;
std::vector<glm::vec2> uvs;
std::vector<glm::vec3> normals;

glm::mat4 ModelMatrix;
glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

bool camera_path_is_shown = false;
bool camera_paused = false;

std::vector<glm::vec3> spline_points;

const int NUM_SPLINE_POINTS_BETWEEN_CONRTOL_POINTS = 100;

extern const int board_height;
extern const int board_width;

TetrisGame tetris_game;

bool soft_drop_is_active = false;
bool left_is_active = false;
bool right_is_active = false;

const float tetris_cube_size = 2.02f;

float light_pos_x = 7.0f;
float light_pos_y = 22.0f;
float light_pos_z = 7.0f;
const float light_pos_delta = 3.0f;

int soft_drop_counter = 0;
int movement_counter = 0;
int iteration_counter = 0;

void draw_tetris_square(const int i, const int j)
{
	ModelMatrix = glm::translate(vec3(j * tetris_cube_size, i * tetris_cube_size, 0.0f));
	glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

	glUniformMatrix4fv(MVPMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(MMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
	glUniformMatrix4fv(VMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

	// glActiveTexture(GL_TEXTURE0);
	// glBindTexture(GL_TEXTURE_2D, Texture);
	// glUniform1i(TextureID, 0);

	glUniform1i(piece_type_flag_id, (int)tetris_game.get_square(i, j));
	glUniform3f(LightID, light_pos_x, light_pos_y, light_pos_z);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(
		0,		  // attribute
		3,		  // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		0,		  // stride
		(void *)0 // array buffer offset
	);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glVertexAttribPointer(
		1,		  // attribute
		2,		  // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		0,		  // stride
		(void *)0 // array buffer offset
	);

	// 3rd attribute buffer : normals
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glVertexAttribPointer(
		2,		  // attribute
		3,		  // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		0,		  // stride
		(void *)0 // array buffer offset
	);

	glDrawArrays(GL_TRIANGLES, 0, vertices.size());

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
}

void draw_tetris_board()
{
	for (int i = 0; i < board_height; i++)
	{
		for (int j = 0; j < board_width; j++)
		{
			if (tetris_game.get_square(i, j) != EMPTY)
			{
				draw_tetris_square(i, j);
			}
		}
	}
}

void key_handler(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		if (key == GLFW_KEY_W)
		{
			tetris_game.soft_drop();
			soft_drop_is_active = true;
			soft_drop_counter = 0;
			iteration_counter = 0;
		}
		else if (key == GLFW_KEY_S)
		{
			tetris_game.hard_drop();
		}
		else if (key == GLFW_KEY_A)
		{
			tetris_game.handle_left_input();
			left_is_active = true;
			movement_counter = 0;
		}
		else if (key == GLFW_KEY_D)
		{
			tetris_game.handle_right_input();
			right_is_active = true;
			movement_counter = 0;
		}
		else if (key == GLFW_KEY_LEFT)
		{
			tetris_game.rotate_left();
		}
		else if (key == GLFW_KEY_RIGHT)
		{
			tetris_game.rotate_right();
		}
		else if (key == GLFW_KEY_KP_7)
		{
			light_pos_x -= light_pos_delta;
			std::cout << light_pos_x << "\n";
		}
		else if (key == GLFW_KEY_KP_9)
		{
			light_pos_x += light_pos_delta;
			std::cout << light_pos_x << "\n";
		}
		else if (key == GLFW_KEY_KP_4)
		{
			light_pos_y -= light_pos_delta;
			std::cout << light_pos_y << "\n";
		}
		else if (key == GLFW_KEY_KP_6)
		{
			light_pos_y += light_pos_delta;
			std::cout << light_pos_y << "\n";
		}
		else if (key == GLFW_KEY_KP_1)
		{
			light_pos_z -= light_pos_delta;
			std::cout << light_pos_z << "\n";
		}
		else if (key == GLFW_KEY_KP_3)
		{
			light_pos_z += light_pos_delta;
			std::cout << light_pos_z << "\n";
		}
	}
	else if (action == GLFW_RELEASE)
	{
		if (key == GLFW_KEY_W)
		{
			soft_drop_is_active = false;
		}
		else if (key == GLFW_KEY_A)
		{
			left_is_active = false;
		}
		else if (key == GLFW_KEY_D)
		{
			right_is_active = false;
		}
	}
}

int main(void)
{
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(1024, 768, "Tutorial 07 - Model Loading", NULL, NULL);
	if (window == NULL)
	{
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK)
	{
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	glfwSetKeyCallback(window, key_handler);

	// Background
	glClearColor(0.05f, 0.05f, 0.05f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders("TransformVertexShader.glsl", "TextureFragmentShader.glsl");

	// Get a handle for our "MVP" uniform
	MVPMatrixID = glGetUniformLocation(programID, "MVP");
	MMatrixID = glGetUniformLocation(programID, "M");
	VMatrixID = glGetUniformLocation(programID, "V");
	LightID = glGetUniformLocation(programID, "lightPosition_worldspace");

	// Load the texture
	// Texture = loadDDS("texture.DDS");

	// Get a handle for our "myTextureSampler" uniform
	// TextureID = glGetUniformLocation(programID, "myTextureSampler");
	piece_type_flag_id = glGetUniformLocation(programID, "piece_type");

	loadOBJ("tetris_cube.obj", vertices, uvs, normals);

	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	// glGenBuffers(1, &uvbuffer);
	// glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	// glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

	auto lastTime = std::chrono::system_clock::now();

	const float board_x_center = board_width * tetris_cube_size / 2;
	const float board_y_center = board_height * tetris_cube_size / 2;

	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 center = glm::vec3(board_x_center, board_y_center, 0.0f);

	ProjectionMatrix = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 205.0f);
	// ViewMatrix = glm::lookAt(spline_points[i % spline_points.size()], center, up);
	// ViewMatrix = glm::lookAt(glm::vec3(0.0f, 0.0f, -25.0f), center, up);
	ViewMatrix = glm::lookAt(glm::vec3(board_x_center, board_y_center, 80.0f), center, up);

	int i = 0;

	const int sub_iterations_per_soft_drop = 3;
	const int sub_iterations_per_movement = 5;
	const int sub_iterations_per_iteration = 15;

	auto iteration_time = 500ms;
	auto sub_iteration_time = iteration_time / sub_iterations_per_iteration;
	auto time_since_last_sub_iteration = 0ms;

	do
	{
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);

		auto currentTime = std::chrono::system_clock::now();
		auto deltaTime = currentTime - lastTime;
		auto deltaTimeInMS = std::chrono::duration_cast<std::chrono::milliseconds>(deltaTime);
		lastTime = currentTime;

		draw_tetris_board();

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

		time_since_last_sub_iteration += deltaTimeInMS;

		if (time_since_last_sub_iteration > sub_iteration_time)
		{
			iteration_counter++;

			if (soft_drop_is_active)
			{
				soft_drop_counter++;
				if (soft_drop_counter == sub_iterations_per_soft_drop)
				{
					soft_drop_counter = 0;
					iteration_counter = 0;
					tetris_game.soft_drop();
				}
			}

			if (!(left_is_active && right_is_active) && (left_is_active || right_is_active))
			{
				movement_counter++;
				if (movement_counter == sub_iterations_per_movement)
				{
					movement_counter = 0;
					if (left_is_active)
					{
						tetris_game.handle_left_input();
					}
					else
					{
						tetris_game.handle_right_input();
					}
				}
			}

			if (iteration_counter == sub_iterations_per_iteration)
			{
				iteration_counter = 0;
				tetris_game.iterate_time();
			}

			time_since_last_sub_iteration -= sub_iteration_time;
		}

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0);

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteBuffers(1, &normalbuffer);
	glDeleteBuffers(1, &camera_line_vertexbuffer);
	glDeleteProgram(programID);
	// glDeleteTextures(1, &Texture);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}
