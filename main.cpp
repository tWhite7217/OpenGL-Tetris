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

GLuint ssd_digit_texture;
GLuint light_blue_texture;
GLuint dark_blue_texture;
GLuint yellow_texture;
GLuint orange_texture;
GLuint red_texture;
GLuint purple_texture;
GLuint green_texture;
GLuint I_billboard_texture;
GLuint J_billboard_texture;
GLuint L_billboard_texture;
GLuint O_billboard_texture;
GLuint S_billboard_texture;
GLuint Z_billboard_texture;
GLuint T_billboard_texture;

GLuint texture_sampler_id;
GLuint MVPMatrixID;
GLuint MMatrixID;
GLuint VMatrixID;
GLuint PMatrixID;
GLuint LightID;
GLuint ambient_id;
GLuint diffuse_id;
GLuint specular_id;

GLuint billboard_center_id;
GLuint billboard_size_id;
GLuint camera_up_id;
GLuint camera_right_id;

float ambient_component = 0.1f;
float diffuse_component = 0.85f;
int specular_exponent = 10;

GLuint piece_type_flag_id;
GLuint use_lighting_flag_id;
GLuint use_mvp_flag_id;

std::vector<GLuint> active_buffers;
std::vector<GLuint> active_textures;

typedef struct OBJData
{
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	GLuint vertex_buffer;
	GLuint uv_buffer;
	GLuint normal_buffer;
};

OBJData tetris_square_obj;
OBJData scoreboard_obj;
OBJData hold_obj;

glm::mat4 ModelMatrix;
glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

bool camera_path_is_shown = false;
bool camera_paused = false;

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
int sub_iteration_counter = 0;

const float board_width_gl = TetrisGame::board_width * tetris_cube_size;
const float board_height_gl = TetrisGame::board_height * tetris_cube_size;
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

void load_texture(GLuint &texture_pointer, std::string filename)
{
	texture_pointer = loadDDS(filename.c_str());
	active_textures.push_back(texture_pointer);
}

void generate_gl_buffer(GLuint &buffer)
{
	glGenBuffers(1, &buffer);
	active_buffers.push_back(buffer);
}

template <typename T>
void fill_gl_buffer(GLuint buffer, std::vector<T> buffer_data)
{
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, buffer_data.size() * sizeof(T), &buffer_data[0], GL_STATIC_DRAW);
}

template <typename T>
void generate_and_fill_gl_buffer(GLuint &buffer, std::vector<T> buffer_data)
{
	generate_gl_buffer(buffer);
	fill_gl_buffer(buffer, buffer_data);
}

void loadOBJ_into_vectors_and_buffers(std::string filename, OBJData &obj_data)
{
	loadOBJ(filename.c_str(), obj_data.vertices, obj_data.uvs, obj_data.normals);
	generate_and_fill_gl_buffer(obj_data.vertex_buffer, obj_data.vertices);
	generate_and_fill_gl_buffer(obj_data.uv_buffer, obj_data.uvs);
	generate_and_fill_gl_buffer(obj_data.normal_buffer, obj_data.normals);
}

void draw_object(GLuint vertexbuffer, GLuint uvbuffer, size_t num_vertices)
{
	glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

	glUniformMatrix4fv(MVPMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(MMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
	glUniformMatrix4fv(VMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
	glUniformMatrix4fv(PMatrixID, 1, GL_FALSE, &ProjectionMatrix[0][0]);

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

	glDrawArrays(GL_TRIANGLES, 0, num_vertices);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

void draw_object_with_normals(GLuint vertexbuffer, GLuint uvbuffer, GLuint normalbuffer, size_t num_vertices)
{
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

	draw_object(vertexbuffer, uvbuffer, num_vertices);

	glDisableVertexAttribArray(2);
}

void draw_tetris_square()
{
	glUniform3f(LightID, light_pos_x, light_pos_y, light_pos_z);

	glUniform1f(ambient_id, ambient_component);
	glUniform1f(diffuse_id, diffuse_component);
	glUniform1i(specular_id, specular_exponent);

	glUniform1i(use_lighting_flag_id, 1);

	draw_object_with_normals(tetris_square_obj.vertex_buffer, tetris_square_obj.uv_buffer, tetris_square_obj.normal_buffer, tetris_square_obj.vertices.size());
}

void draw_tetris_board()
{
	for (int i = 0; i < TetrisGame::board_height; i++)
	{
		for (int j = 0; j < TetrisGame::board_width; j++)
		{
			auto square = tetris_game.get_square(i, j);
			if (square != TetrisGame::BoardSquareColor::EMPTY)
			{
				glUniform1i(piece_type_flag_id, (int)square);
				ModelMatrix = glm::translate(vec3(j * tetris_cube_size, i * tetris_cube_size, 0.0f));
				draw_tetris_square();
			}
		}
	}
}

void draw_held_tetris_piece()
{
	glm::vec3 billboard_center = glm::vec3(-8.0f, board_height_gl - 5.0f, 0.0f);
	ModelMatrix = glm::rotate(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	glm::vec3 camera_right_worldspace = glm::vec3(ViewMatrix[0][0], ViewMatrix[1][0], ViewMatrix[2][0]);
	glm::vec3 camera_up_worldspace = glm::vec3(ViewMatrix[0][1], ViewMatrix[1][1], ViewMatrix[2][1]);

	GLuint current_texture;

	glm::vec2 billboard_size;

	auto held_piece_type = tetris_game.get_held_piece();

	using PT = TetrisGame::PieceType;
	switch (held_piece_type)
	{
	case PT::I:
		billboard_size = glm::vec2(2, 1.5);
		current_texture = I_billboard_texture;
		break;
	case PT::O:
		billboard_size = glm::vec2(1.3, 3);
		current_texture = O_billboard_texture;
		break;
	default:
		billboard_size = glm::vec2(2, 3);
		switch (held_piece_type)
		{
		case PT::J:
			current_texture = J_billboard_texture;
			break;
		case PT::L:
			current_texture = L_billboard_texture;
			break;
		case PT::S:
			current_texture = S_billboard_texture;
			break;
		case PT::Z:
			current_texture = Z_billboard_texture;
			break;
		case PT::T:
			current_texture = T_billboard_texture;
			break;
		default:
			break;
		}
		break;
	}

	glUniform3f(billboard_center_id, billboard_center.x, billboard_center.y, billboard_center.z);
	glUniform2f(billboard_size_id, billboard_size.x, billboard_size.y);
	glUniform3f(camera_up_id, camera_up_worldspace.x, camera_up_worldspace.y, camera_up_worldspace.z);
	glUniform3f(camera_right_id, camera_right_worldspace.x, camera_right_worldspace.y, camera_right_worldspace.z);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, current_texture);
	glUniform1i(texture_sampler_id, 0);

	glUniform1i(piece_type_flag_id, -1);
	glUniform1i(use_lighting_flag_id, 0);
	glUniform1i(use_mvp_flag_id, 1);

	draw_object(hold_obj.vertex_buffer, hold_obj.uv_buffer, hold_obj.vertices.size());
}

void draw_upcoming_pieces(float y_offset)
{
	for (int i = 0; i < TetrisGame::num_upcoming_pieces_shown; i++)
	{
		for (int j = 0; j < TetrisGame::upcoming_board_lines_per_piece; j++)
		{
			for (int k = 0; k < TetrisGame::upcoming_board_width; k++)
			{
				using BSC = TetrisGame::BoardSquareColor;
				auto square = tetris_game.get_upcoming_square(i, j, k);
				if (square != BSC::EMPTY)
				{
					glActiveTexture(GL_TEXTURE0);
					switch (square)
					{
					case BSC::LIGHT_BLUE:
						glBindTexture(GL_TEXTURE_2D, light_blue_texture);
						break;
					case BSC::DARK_BLUE:
						glBindTexture(GL_TEXTURE_2D, dark_blue_texture);
						break;
					case BSC::GREEN:
						glBindTexture(GL_TEXTURE_2D, green_texture);
						break;
					case BSC::YELLOW:
						glBindTexture(GL_TEXTURE_2D, yellow_texture);
						break;
					case BSC::ORANGE:
						glBindTexture(GL_TEXTURE_2D, orange_texture);
						break;
					case BSC::RED:
						glBindTexture(GL_TEXTURE_2D, red_texture);
						break;
					case BSC::MAGENTA:
						glBindTexture(GL_TEXTURE_2D, purple_texture);
						break;
					}
					glUniform1i(texture_sampler_id, 0);
					glUniform1i(piece_type_flag_id, -1);
					float x = (k + TetrisGame::board_width * 5 / 4) * tetris_cube_size;
					float y = board_height_gl - (TetrisGame::num_upcoming_pieces_shown - i - 1) * TetrisGame::upcoming_board_lines_per_piece * tetris_cube_size + j * tetris_cube_size - y_offset;
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

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ssd_digit_texture);
	glUniform1i(texture_sampler_id, 0);

	glUniform1i(piece_type_flag_id, -1);
	glUniform1i(use_lighting_flag_id, 0);
	glUniform1i(use_mvp_flag_id, 0);

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

	for (auto xy : scoreboard_obj.uvs)
	{
		current_uvs.push_back(glm::vec2(xy.x + (0.2 * displacement_x), xy.y - (0.5 * displacement_y)));
	}

	fill_gl_buffer(scoreboard_obj.uv_buffer, current_uvs);

	draw_object(scoreboard_obj.vertex_buffer, scoreboard_obj.uv_buffer, scoreboard_obj.vertices.size());
}

void draw_scoreboard(int score)
{
	int thousands = score / 1000;
	int hundreds = (score / 100) % 10;
	int tens = (score / 10) % 10;
	int ones = score % 10;

	int num_digits_to_draw =
		thousands ? 4 : (hundreds ? 3 : (tens ? 2 : 1));

	switch (num_digits_to_draw)
	{
	case 4:
		draw_scoreboard_digit(3, thousands);
	case 3:
		draw_scoreboard_digit(2, hundreds);
	case 2:
		draw_scoreboard_digit(1, tens);
	default:
		draw_scoreboard_digit(0, ones);
	}
}

void begin_moving_camera_to(glm::vec3 new_position)
{
	original_position = position;
	destination_position = new_position;
	time_since_camera_change_started = 0ms;
}

template <typename T>
void increment_and_print_value_with_max(T &value, T delta, T max_value)
{
	value = std::min(value + delta, max_value);
	std::cout << value << "\n";
}

template <typename T>
void decrement_and_print_value_with_min(T &value, T delta, T min_value)
{
	value = std::max(value - delta, min_value);
	std::cout << value << "\n";
}

void key_handler(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		switch (key)
		{
		case GLFW_KEY_W:
			tetris_game.soft_drop();
			soft_drop_is_active = true;
			soft_drop_counter = 0;
			sub_iteration_counter = 0;
			break;
		case GLFW_KEY_S:
			tetris_game.hard_drop();
			break;
		case GLFW_KEY_A:
			tetris_game.handle_left_input();
			left_is_active = true;
			movement_counter = 0;
			break;
		case GLFW_KEY_D:
			tetris_game.handle_right_input();
			right_is_active = true;
			movement_counter = 0;
			break;
		case GLFW_KEY_LEFT:
			tetris_game.rotate_left();
			break;
		case GLFW_KEY_RIGHT:
			tetris_game.rotate_right();
			break;
		case GLFW_KEY_KP_7:
			decrement_and_print_value_with_min(light_pos_x, light_pos_delta, -1000.0f);
			break;
		case GLFW_KEY_KP_9:
			increment_and_print_value_with_max(light_pos_x, light_pos_delta, 1000.0f);
			break;
		case GLFW_KEY_KP_4:
			decrement_and_print_value_with_min(light_pos_y, light_pos_delta, -1000.0f);
			break;
		case GLFW_KEY_KP_6:
			increment_and_print_value_with_max(light_pos_y, light_pos_delta, 1000.0f);
			break;
		case GLFW_KEY_KP_1:
			decrement_and_print_value_with_min(light_pos_z, light_pos_delta, -1000.0f);
			break;
		case GLFW_KEY_KP_3:
			increment_and_print_value_with_max(light_pos_z, light_pos_delta, 1000.0f);
			break;
		case GLFW_KEY_INSERT:
			increment_and_print_value_with_max(ambient_component, 0.01f, 1.0f);
			break;
		case GLFW_KEY_DELETE:
			decrement_and_print_value_with_min(ambient_component, 0.01f, 0.0f);
			break;
		case GLFW_KEY_HOME:
			increment_and_print_value_with_max(diffuse_component, 0.02f, 1.0f);
			break;
		case GLFW_KEY_END:
			decrement_and_print_value_with_min(diffuse_component, 0.02f, 0.0f);
			break;
		case GLFW_KEY_PAGE_UP:
			increment_and_print_value_with_max(specular_exponent, 6, 10000);
			break;
		case GLFW_KEY_PAGE_DOWN:
			decrement_and_print_value_with_min(specular_exponent, 5, 1);
			break;
		case GLFW_KEY_G:
			begin_moving_camera_to(vec3(-70.0f, 20.0f, 0.0f));
			break;
		case GLFW_KEY_Y:
			begin_moving_camera_to(camera_positions[0]);
			break;
		case GLFW_KEY_U:
			begin_moving_camera_to(camera_positions[1]);
			break;
		case GLFW_KEY_I:
			begin_moving_camera_to(camera_positions[2]);
			break;
		case GLFW_KEY_H:
			begin_moving_camera_to(camera_positions[3]);
			break;
		case GLFW_KEY_J:
			begin_moving_camera_to(camera_positions[4]);
			break;
		case GLFW_KEY_K:
			begin_moving_camera_to(camera_positions[5]);
			break;
		case GLFW_KEY_B:
			begin_moving_camera_to(camera_positions[6]);
			break;
		case GLFW_KEY_N:
			begin_moving_camera_to(camera_positions[7]);
			break;
		case GLFW_KEY_M:
			begin_moving_camera_to(camera_positions[8]);
			break;
		case GLFW_KEY_LEFT_SHIFT:
			tetris_game.hold_piece();
			break;
		}
	}
	else if (action == GLFW_RELEASE)
	{
		switch (key)
		{
		case GLFW_KEY_W:
			soft_drop_is_active = false;
			break;
		case GLFW_KEY_A:
			left_is_active = false;
			break;
		case GLFW_KEY_D:
			right_is_active = false;
			break;
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
	window = glfwCreateWindow(1024, 768, "OpenGL Tetris", NULL, NULL);
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

	MVPMatrixID = glGetUniformLocation(programID, "MVP");
	MMatrixID = glGetUniformLocation(programID, "M");
	VMatrixID = glGetUniformLocation(programID, "V");
	PMatrixID = glGetUniformLocation(programID, "P");
	LightID = glGetUniformLocation(programID, "lightPosition_worldspace");
	ambient_id = glGetUniformLocation(programID, "ambient_component");
	diffuse_id = glGetUniformLocation(programID, "diffuse_component");
	specular_id = glGetUniformLocation(programID, "specular_exponent");
	billboard_center_id = glGetUniformLocation(programID, "billboard_center");
	billboard_size_id = glGetUniformLocation(programID, "billboard_size");
	camera_up_id = glGetUniformLocation(programID, "camera_up_worldspace");
	camera_right_id = glGetUniformLocation(programID, "camera_right_worldspace");

	load_texture(ssd_digit_texture, "textures/SSD_Numbers.DDS");
	load_texture(light_blue_texture, "textures/colors/light_blue.DDS");
	load_texture(dark_blue_texture, "textures/colors/dark_blue.DDS");
	load_texture(yellow_texture, "textures/colors/yellow.DDS");
	load_texture(orange_texture, "textures/colors/orange.DDS");
	load_texture(red_texture, "textures/colors/red.DDS");
	load_texture(purple_texture, "textures/colors/purple.DDS");
	load_texture(green_texture, "textures/colors/green.DDS");
	load_texture(I_billboard_texture, "textures/billboards/I.DDS");
	load_texture(J_billboard_texture, "textures/billboards/J.DDS");
	load_texture(L_billboard_texture, "textures/billboards/L.DDS");
	load_texture(O_billboard_texture, "textures/billboards/O.DDS");
	load_texture(S_billboard_texture, "textures/billboards/S.DDS");
	load_texture(Z_billboard_texture, "textures/billboards/Z.DDS");
	load_texture(T_billboard_texture, "textures/billboards/T.DDS");

	texture_sampler_id = glGetUniformLocation(programID, "textureSampler");
	piece_type_flag_id = glGetUniformLocation(programID, "piece_type");
	use_lighting_flag_id = glGetUniformLocation(programID, "use_lighting");
	use_mvp_flag_id = glGetUniformLocation(programID, "use_mvp");

	loadOBJ_into_vectors_and_buffers("objs/tetris_cube_more_beveled.obj", tetris_square_obj);
	loadOBJ_into_vectors_and_buffers("objs/SSD_Digit.obj", scoreboard_obj);
	loadOBJ_into_vectors_and_buffers("objs/hold.obj", hold_obj);

	auto lastTime = std::chrono::system_clock::now();

	ProjectionMatrix = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 205.0f);

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

	ViewMatrix = glm::lookAt(position, center, up);

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

		if (time_since_camera_change_started < time_between_camera_positions)
		{
			time_since_camera_change_started += deltaTimeInMS;
			position_fraction = float(time_since_camera_change_started.count()) / float(time_between_camera_positions.count());
			position = (1.0f - position_fraction) * original_position + position_fraction * destination_position;
			ViewMatrix = glm::lookAt(position, center, up);
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
		if (tetris_game.get_whether_a_piece_is_held())
		{
			draw_held_tetris_piece();
		}

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

		time_since_last_sub_iteration += deltaTimeInMS;

		if (time_since_last_sub_iteration > sub_iteration_time)
		{
			sub_iteration_counter++;

			if (soft_drop_is_active)
			{
				soft_drop_counter++;
				if (soft_drop_counter == sub_iterations_per_soft_drop)
				{
					soft_drop_counter = 0;
					sub_iteration_counter = 0;
					tetris_game.soft_drop();
				}
			}

			if (left_is_active ^ right_is_active)
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

			if (sub_iteration_counter == sub_iterations_per_iteration)
			{
				sub_iteration_counter = 0;
				tetris_game.iterate_time();
			}

			time_since_last_sub_iteration -= sub_iteration_time;
		}

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0);

	// Cleanup VBO and shader
	glDeleteBuffers(active_buffers.size(), active_buffers.data());
	glDeleteTextures(active_textures.size(), active_textures.data());
	glDeleteProgram(programID);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}
