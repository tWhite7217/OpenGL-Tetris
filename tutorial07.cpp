#include <stdio.h>
#include <stdlib.h>
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
GLuint MatrixID;
GLuint vertexbuffer;
GLuint uvbuffer;

GLuint camera_line_vertexbuffer;

GLuint is_empty_flag_id;

std::vector<glm::vec3> vertices;
std::vector<glm::vec2> uvs;
std::vector<glm::vec3> normals; // Won't be used at the moment.

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

const float tetris_cube_size = 2.0f;

// glm::mat4x3 getControlPointsMatrix(int i)
// {
// 	glm::vec3 first_control_point;
// 	if (i == 0)
// 	{
// 		first_control_point = control_points.back();
// 	}
// 	else
// 	{
// 		first_control_point = control_points[i - 1];
// 	}

// 	glm::vec3 second_control_point = control_points[i];

// 	glm::vec3 third_control_point;
// 	glm::vec3 fourth_control_point;
// 	if (i == control_points.size() - 2)
// 	{
// 		third_control_point = control_points[i + 1];
// 		fourth_control_point = control_points[0];
// 	}
// 	else if (i == control_points.size() - 1)
// 	{
// 		third_control_point = control_points[0];
// 		fourth_control_point = control_points[1];
// 	}
// 	else
// 	{
// 		third_control_point = control_points[i + 1];
// 		fourth_control_point = control_points[i + 2];
// 	}

// 	glm::mat4x3 control_points_matrix = glm::mat4x3(first_control_point,
// 													second_control_point,
// 													third_control_point,
// 													fourth_control_point);

// 	return control_points_matrix;
// }

// void computePartialCatmullRomSpline(int i)
// {
// 	glm::mat4x3 control_points_matrix = getControlPointsMatrix(i);

// 	float increment = 1.0f / NUM_SPLINE_POINTS_BETWEEN_CONRTOL_POINTS;
// 	float loop_cutoff = 1.0f - increment / 4;

// 	spline_points.emplace_back(control_points[i]);

// 	for (float t = increment; t < loop_cutoff; t += increment)
// 	{
// 		spline_points.push_back(getCatmullRomSplinePoint(control_points_matrix, t));
// 	}

// 	if (i < control_points.size() - 1)
// 	{
// 		spline_points.emplace_back(control_points[i + 1]);
// 	}
// }

// void computeCatmullRomSpline()
// {
// 	for (int i = 0; i < control_points.size(); i++)
// 	{
// 		computePartialCatmullRomSpline(i);
// 	}
// }

void draw_tetris_square(const int i, const int j)
{
	ModelMatrix = glm::translate(vec3(j * tetris_cube_size, i * tetris_cube_size, 0.0f));
	glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

	// glActiveTexture(GL_TEXTURE0);
	// glBindTexture(GL_TEXTURE_2D, Texture);
	// glUniform1i(TextureID, 0);

	glUniform1i(is_empty_flag_id, 0);

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

	glDrawArrays(GL_TRIANGLES, 0, vertices.size());

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

void draw_tetris_board()
{
	auto board = tetris_game.get_board();
	int num_rows = board.size();
	int num_columns = board[0].size();
	for (int i = 0; i < num_rows; i++)
	{
		for (int j = 0; j < num_columns; j++)
		{
			if (board[i][j] != EMPTY)
			{
				draw_tetris_square(i, j);
			}
		}
	}
}

// void draw_camera_path()
// {
// 	ModelMatrix = glm::mat4(1.0f);
// 	glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

// 	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

// 	// glActiveTexture(GL_TEXTURE0);
// 	// glBindTexture(GL_TEXTURE_2D, Texture);
// 	// glUniform1i(TextureID, 0);

// 	glUniform1i(is_empty_flag_id, 1);

// 	glEnableVertexAttribArray(0);
// 	glBindBuffer(GL_ARRAY_BUFFER, camera_line_vertexbuffer);
// 	glVertexAttribPointer(
// 		0,		  // attribute
// 		3,		  // size
// 		GL_FLOAT, // type
// 		GL_FALSE, // normalized?
// 		0,		  // stride
// 		(void *)0 // array buffer offset
// 	);

// 	glDrawArrays(GL_LINE_STRIP, 0, spline_points.size());

// 	glDisableVertexAttribArray(0);
// }

void key_handler(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		if (key == GLFW_KEY_P)
		{
			camera_paused = !camera_paused;
		}
		if (key == GLFW_KEY_L)
		{
			camera_path_is_shown = !camera_path_is_shown;
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
	MatrixID = glGetUniformLocation(programID, "MVP");

	// Load the texture
	// Texture = loadDDS("texture.DDS");

	// Get a handle for our "myTextureSampler" uniform
	// TextureID = glGetUniformLocation(programID, "myTextureSampler");
	is_empty_flag_id = glGetUniformLocation(programID, "is_empty");

	loadOBJ("tetris_cube.obj", vertices, uvs, normals);

	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	// glGenBuffers(1, &uvbuffer);
	// glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	// glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

	// computeCatmullRomSpline();
	// glGenBuffers(1, &camera_line_vertexbuffer);
	// glBindBuffer(GL_ARRAY_BUFFER, camera_line_vertexbuffer);
	// glBufferData(GL_ARRAY_BUFFER, spline_points.size() * sizeof(glm::vec3), &spline_points[0], GL_STATIC_DRAW);

	auto lastTime = std::chrono::system_clock::now();

	const float board_x_center = board_width * tetris_cube_size / 2;
	const float board_y_center = board_height * tetris_cube_size / 2;

	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 center = glm::vec3(board_x_center, board_y_center, 0.0f);

	ProjectionMatrix = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 205.0f);
	// ViewMatrix = glm::lookAt(spline_points[i % spline_points.size()], center, up);
	// ViewMatrix = glm::lookAt(glm::vec3(0.0f, 0.0f, -25.0f), center, up);
	ViewMatrix = glm::lookAt(glm::vec3(board_x_center, board_y_center, 125.0f), center, up);

	int i = 0;
	// int j = 0;

	auto iteration_time = 500ms;
	auto time_since_last_iteration = 0ms;

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

		// if (camera_path_is_shown)
		// {
		// 	draw_camera_path();
		// }
		draw_tetris_board();

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

		// if (!camera_paused)
		// {
		// i++;
		// }

		time_since_last_iteration += deltaTimeInMS;

		if (time_since_last_iteration > iteration_time)
		{
			tetris_game.iterate_time();
			time_since_last_iteration -= iteration_time;
		}

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0);

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteBuffers(1, &camera_line_vertexbuffer);
	glDeleteProgram(programID);
	// glDeleteTextures(1, &Texture);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}
