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

GLuint Texture;
GLuint light_blue_texture;
GLuint dark_blue_texture;
GLuint yellow_texture;
GLuint orange_texture;
GLuint red_texture;
GLuint purple_texture;
GLuint green_texture;

GLuint TextureID;
GLuint MVPMatrixID;
GLuint MMatrixID;
GLuint VMatrixID;
GLuint PMatrixID;
GLuint LightID;
GLuint ambient_id;
GLuint diffuse_id;
GLuint specular_id;

GLuint vertexbuffer;
GLuint uvbuffer;
GLuint normalbuffer;

GLuint scoreboard_vertexbuffer;
GLuint scoreboard_uvbuffer;
GLuint scoreboard_normalbuffer;

float ambient_component = 0.1;
float diffuse_component = 0.85;
int specular_exponent = 10;

GLuint camera_line_vertexbuffer;

GLuint piece_type_flag_id;
GLuint use_lighting_flag_id;

std::vector<glm::vec3> vertices;
std::vector<glm::vec2> uvs;
std::vector<glm::vec3> normals;

std::vector<glm::vec3> scoreboard_vertices;
std::vector<glm::vec2> scoreboard_uvs;
std::vector<glm::vec3> scoreboard_normals;

glm::mat4 ModelMatrix;
glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

bool camera_path_is_shown = false;
bool camera_paused = false;

std::vector<glm::vec3> spline_points;

const int NUM_SPLINE_POINTS_BETWEEN_CONRTOL_POINTS = 100;

extern const int board_height;
extern const int board_width;

extern const int upcoming_board_width;
extern const int upcoming_board_lines_per_piece;
extern const int num_upcoming_pieces_shown;

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

const float board_width_gl = board_width * tetris_cube_size;
const float board_height_gl = board_height * tetris_cube_size;
const float board_x_center = board_width_gl / 2;
const float board_y_center = board_height_gl / 2;

const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
const glm::vec3 center = glm::vec3(board_x_center, board_y_center, 0.0f);

const auto time_between_camera_positions = 1500ms;
auto time_since_camera_change_started = 0ms;

const std::array<glm::vec3, 9> camera_positions = {{
	glm::vec3(0.0f, board_height_gl, 50.0f),
	glm::vec3(board_x_center, board_height_gl, 65.0f),
	glm::vec3(board_width_gl, board_height_gl, 90.0f),
	glm::vec3(0.0f, board_y_center, 100.f),
	glm::vec3(board_x_center, board_y_center, 80.0f),
	glm::vec3(board_width_gl, board_y_center, 60.f),
	glm::vec3(0.0f, 0.0f, 70.0f),
	glm::vec3(board_x_center, 0.0f, 120.0f),
	glm::vec3(board_width_gl, 0.0f, 80.0f),
}};

glm::vec3 position = camera_positions[4];
glm::vec3 original_position = camera_positions[4];
glm::vec3 destination_position = camera_positions[4];

void draw_tetris_square()
{
	glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

	glUniformMatrix4fv(MVPMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(MMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
	glUniformMatrix4fv(VMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

	glUniform3f(LightID, light_pos_x, light_pos_y, light_pos_z);

	glUniform1f(ambient_id, ambient_component);
	glUniform1f(diffuse_id, diffuse_component);
	glUniform1i(specular_id, specular_exponent);

	glUniform1i(use_lighting_flag_id, 1);

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
			auto square = tetris_game.get_square(i, j);
			if (square != EMPTY)
			{
				glUniform1i(piece_type_flag_id, (int)square);
				ModelMatrix = glm::translate(vec3(j * tetris_cube_size, i * tetris_cube_size, 0.0f));
				draw_tetris_square();
			}
		}
	}
}

// void draw_upcoming_tetris_piece_of_type(PieceType upcoming_piece_type, int piece_num) {
// 	for (int i = 0; i < 4; i++) {
// 		ModelMatrix = glm::translate(vec3(i*tetris_cube_size-board_width/4, board_height*3/4 - (piece_num * )));
// 		draw_tetris_square();
// 	}
// }

void draw_upcoming_pieces(float y_offset)
{
	for (int i = 0; i < num_upcoming_pieces_shown; i++)
	{
		for (int j = 0; j < upcoming_board_lines_per_piece; j++)
		{
			for (int k = 0; k < upcoming_board_width; k++)
			{
				auto square = tetris_game.get_upcoming_square(i, j, k);
				if (square != EMPTY)
				{
					glActiveTexture(GL_TEXTURE0);
					switch (square)
					{
					case LIGHT_BLUE:
						glBindTexture(GL_TEXTURE_2D, light_blue_texture);
						break;
					case DARK_BLUE:
						glBindTexture(GL_TEXTURE_2D, dark_blue_texture);
						break;
					case GREEN:
						glBindTexture(GL_TEXTURE_2D, green_texture);
						break;
					case YELLOW:
						glBindTexture(GL_TEXTURE_2D, yellow_texture);
						break;
					case ORANGE:
						glBindTexture(GL_TEXTURE_2D, orange_texture);
						break;
					case RED:
						glBindTexture(GL_TEXTURE_2D, red_texture);
						break;
					case MAGENTA:
						glBindTexture(GL_TEXTURE_2D, purple_texture);
						break;
					}
					glUniform1i(TextureID, 0);
					glUniform1i(piece_type_flag_id, -1);
					// ModelMatrix = glm::translate(vec3(j * tetris_cube_size, i * tetris_cube_size, 0.0f));
					float x = (k + board_width * 5 / 4) * tetris_cube_size;
					float y = board_height * tetris_cube_size - (num_upcoming_pieces_shown - i - 1) * upcoming_board_lines_per_piece * tetris_cube_size + j * tetris_cube_size - y_offset;
					ModelMatrix = glm::translate(vec3(x, y, 0.0f));
					draw_tetris_square();
				}
			}
		}
	}
}

void draw_scoreboard_digit(int digit_place, int digit_value)
{
	ModelMatrix = glm::translate(vec3(0.95f - (digit_place * 0.085f), -0.9f, -0.5f)) * glm::scale(vec3(0.06f, 0.09f, 0.06f));
	glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

	glUniformMatrix4fv(MVPMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(MMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
	glUniformMatrix4fv(VMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
	glUniformMatrix4fv(PMatrixID, 1, GL_FALSE, &ProjectionMatrix[0][0]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, Texture);
	glUniform1i(TextureID, 0);

	glUniform1i(piece_type_flag_id, -1);
	glUniform1i(use_lighting_flag_id, 0);

	std::vector<glm::vec2> current_uvs;
	int displacement_x;
	int displacement_y;
	if (digit_value == 0)
	{
		displacement_x = 4;
		displacement_y = 1;
	}
	else
	{
		displacement_x = (digit_value - 1) % 5;
		displacement_y = (digit_value - 1) / 5;
	}

	for (auto xy : scoreboard_uvs)
	{
		current_uvs.push_back(glm::vec2(xy.x + (0.2 * displacement_x), xy.y - (0.5 * displacement_y)));
	}

	glBindBuffer(GL_ARRAY_BUFFER, scoreboard_uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, current_uvs.size() * sizeof(glm::vec2), &current_uvs[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, scoreboard_vertexbuffer);
	glVertexAttribPointer(
		0,		  // attribute
		3,		  // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		0,		  // stride
		(void *)0 // array buffer offset
	);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, scoreboard_uvbuffer);
	glVertexAttribPointer(
		1,		  // attribute
		2,		  // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		0,		  // stride
		(void *)0 // array buffer offset
	);

	glDrawArrays(GL_TRIANGLES, 0, scoreboard_vertices.size());

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

void draw_scoreboard(int score)
{
	int thousands = score / 1000;
	int hundreds = (score / 100) % 10;
	int tens = (score / 10) % 10;
	int ones = score % 10;

	if (thousands != 0)
	{
		draw_scoreboard_digit(3, thousands);
		draw_scoreboard_digit(2, hundreds);
		draw_scoreboard_digit(1, tens);
	}
	else if (hundreds != 0)
	{
		draw_scoreboard_digit(2, hundreds);
		draw_scoreboard_digit(1, tens);
	}
	else if (tens != 0)
	{
		draw_scoreboard_digit(1, tens);
	}

	draw_scoreboard_digit(0, ones);
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
		else if (key == GLFW_KEY_INSERT)
		{
			ambient_component += 0.01;
			ambient_component = std::min(1.0f, ambient_component);
			std::cout << ambient_component << "\n";
		}
		else if (key == GLFW_KEY_DELETE)
		{
			ambient_component -= 0.01;
			ambient_component = std::max(0.0f, ambient_component);
			std::cout << ambient_component << "\n";
		}
		else if (key == GLFW_KEY_HOME)
		{
			diffuse_component += 0.02;
			diffuse_component = std::min(1.0f, diffuse_component);
			std::cout << diffuse_component << "\n";
		}
		else if (key == GLFW_KEY_END)
		{
			diffuse_component -= 0.02;
			diffuse_component = std::max(0.0f, diffuse_component);
			std::cout << diffuse_component << "\n";
		}
		else if (key == GLFW_KEY_PAGE_UP)
		{
			specular_exponent += 6;
			std::cout << specular_exponent << "\n";
		}
		else if (key == GLFW_KEY_PAGE_DOWN)
		{
			specular_exponent = std::max(1, specular_exponent - 5);
			std::cout << specular_exponent << "\n";
		}
		else if (key == GLFW_KEY_Y)
		{
			original_position = position;
			destination_position = camera_positions[0];
			time_since_camera_change_started = 0ms;
		}
		else if (key == GLFW_KEY_U)
		{
			original_position = position;
			destination_position = camera_positions[1];
			time_since_camera_change_started = 0ms;
		}
		else if (key == GLFW_KEY_I)
		{
			original_position = position;
			destination_position = camera_positions[2];
			time_since_camera_change_started = 0ms;
		}
		else if (key == GLFW_KEY_H)
		{
			original_position = position;
			destination_position = camera_positions[3];
			time_since_camera_change_started = 0ms;
		}
		else if (key == GLFW_KEY_J)
		{
			original_position = position;
			destination_position = camera_positions[4];
			time_since_camera_change_started = 0ms;
		}
		else if (key == GLFW_KEY_K)
		{
			original_position = position;
			destination_position = camera_positions[5];
			time_since_camera_change_started = 0ms;
		}
		else if (key == GLFW_KEY_B)
		{
			original_position = position;
			destination_position = camera_positions[6];
			time_since_camera_change_started = 0ms;
		}
		else if (key == GLFW_KEY_N)
		{
			original_position = position;
			destination_position = camera_positions[7];
			time_since_camera_change_started = 0ms;
		}
		else if (key == GLFW_KEY_M)
		{
			original_position = position;
			destination_position = camera_positions[8];
			time_since_camera_change_started = 0ms;
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
	PMatrixID = glGetUniformLocation(programID, "P");
	LightID = glGetUniformLocation(programID, "lightPosition_worldspace");
	ambient_id = glGetUniformLocation(programID, "ambient_component");
	diffuse_id = glGetUniformLocation(programID, "diffuse_component");
	specular_id = glGetUniformLocation(programID, "specular_exponent");

	// Load the texture
	Texture = loadDDS("SSD_Numbers.DDS");
	light_blue_texture = loadDDS("light_blue_texture.DDS");
	dark_blue_texture = loadDDS("dark_blue_texture.DDS");
	yellow_texture = loadDDS("yellow_texture.DDS");
	orange_texture = loadDDS("orange_texture.DDS");
	red_texture = loadDDS("red_texture.DDS");
	purple_texture = loadDDS("purple_texture.DDS");
	green_texture = loadDDS("green_texture.DDS");

	// Get a handle for our "myTextureSampler" uniform
	TextureID = glGetUniformLocation(programID, "textureSampler");
	piece_type_flag_id = glGetUniformLocation(programID, "piece_type");
	use_lighting_flag_id = glGetUniformLocation(programID, "use_lighting");

	// loadOBJ("tetris_cube.obj", vertices, uvs, normals);
	loadOBJ("tetris_cube_more_beveled.obj", vertices, uvs, normals);
	// loadOBJ("tetris_cube_even_more_beveled.obj", vertices, uvs, normals);

	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

	loadOBJ("SSD_Digit.obj", scoreboard_vertices, scoreboard_uvs, scoreboard_normals);

	glGenBuffers(1, &scoreboard_vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, scoreboard_vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, scoreboard_vertices.size() * sizeof(glm::vec3), &scoreboard_vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &scoreboard_uvbuffer);

	glGenBuffers(1, &scoreboard_normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, scoreboard_normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, scoreboard_normals.size() * sizeof(glm::vec3), &scoreboard_normals[0], GL_STATIC_DRAW);

	auto lastTime = std::chrono::system_clock::now();

	ProjectionMatrix = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 205.0f);
	// ViewMatrix = glm::lookAt(spline_points[i % spline_points.size()], center, up);
	// ViewMatrix = glm::lookAt(glm::vec3(0.0f, 0.0f, -25.0f), center, up);

	int i = 0;

	const int sub_iterations_per_soft_drop = 3;
	const int sub_iterations_per_movement = 5;
	const int sub_iterations_per_iteration = 15;

	auto iteration_time = 500ms;
	auto sub_iteration_time = iteration_time / sub_iterations_per_iteration;
	auto time_since_last_sub_iteration = 0ms;

	float upcoming_piece_y_offset = 0;
	const auto y_offset_period = 5000ms;
	auto y_offset_timer = 0ms;
	bool y_offset_is_increasing = true;
	float y_offset_fraction;
	const float y_offset_max = 5.0f;

	float position_fraction;

	do
	{
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);

		ViewMatrix = glm::lookAt(position, center, up);

		auto currentTime = std::chrono::system_clock::now();
		auto deltaTime = currentTime - lastTime;
		auto deltaTimeInMS = std::chrono::duration_cast<std::chrono::milliseconds>(deltaTime);
		lastTime = currentTime;

		if (time_since_camera_change_started < time_between_camera_positions)
		{
			time_since_camera_change_started += deltaTimeInMS;
			position_fraction = float(time_since_camera_change_started.count()) / float(time_between_camera_positions.count());
			position = (1.0f - position_fraction) * original_position + position_fraction * destination_position;
		}

		y_offset_timer += deltaTimeInMS;

		if (y_offset_timer > y_offset_period)
		{
			y_offset_timer -= y_offset_period;
			y_offset_is_increasing = !y_offset_is_increasing;
		}

		y_offset_fraction = float(y_offset_timer.count()) / float(y_offset_period.count());

		if (y_offset_is_increasing)
		{
			upcoming_piece_y_offset = y_offset_fraction * y_offset_max;
		}
		else
		{
			upcoming_piece_y_offset = y_offset_max - y_offset_fraction * y_offset_max;
		}

		draw_tetris_board();
		draw_upcoming_pieces(upcoming_piece_y_offset);
		draw_scoreboard(tetris_game.get_score());

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
	glDeleteBuffers(1, &scoreboard_vertexbuffer);
	glDeleteBuffers(1, &scoreboard_uvbuffer);
	glDeleteBuffers(1, &scoreboard_normalbuffer);
	glDeleteBuffers(1, &camera_line_vertexbuffer);
	glDeleteProgram(programID);
	glDeleteTextures(1, &light_blue_texture);
	glDeleteTextures(1, &dark_blue_texture);
	glDeleteTextures(1, &yellow_texture);
	glDeleteTextures(1, &orange_texture);
	glDeleteTextures(1, &red_texture);
	glDeleteTextures(1, &purple_texture);
	glDeleteTextures(1, &green_texture);
	glDeleteTextures(1, &Texture);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}
