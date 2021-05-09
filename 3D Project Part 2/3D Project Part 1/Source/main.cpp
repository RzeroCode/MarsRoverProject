#include <iostream>
#include <vector>

#define GLM_FORCE_LEFT_HANDED
#include "GLM/glm.hpp"
#include "GLM/common.hpp"
#include "GLM/gtc/type_ptr.hpp"
#include "GLM/gtx/rotate_vector.hpp"
#include "GLAD/glad.h"
#include "GLFW/glfw3.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "opengl_utilities.h"
#include "mesh_generation.h"

/* Keep the global state inside this struct */
static struct {
	glm::dvec2 mouse_position;
	glm::ivec2 screen_dimensions = glm::ivec2(960, 960); //960x960
} Globals;

/* GLFW Callback functions */
static void ErrorCallback(int error, const char* description)
{
	std::cerr << "Error: " << description << std::endl;
}
//vars for mouse pos
bool firstMouse = true;
auto yaw = 90.0f;
auto pitch = 0.0f;
double lastX = 480, lastY = 480;
auto camera_front = glm::vec3(0, 0, 1);
static void CursorPositionCallback(GLFWwindow* window, double x, double y)
{
	Globals.mouse_position.x = x;
	Globals.mouse_position.y = y;
	//deneme
	if (firstMouse)
	{
		lastX = x;
		lastY = y;
		firstMouse = false;
	}

	//float xoffset = x - lastX;
	float xoffset = lastX - x;
	float yoffset = lastY - y;
	lastX = x;
	lastY = y;

	float sensitivity = 0.2f;  //0.1f
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 direction;
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	camera_front = glm::normalize(direction);
}

static void WindowSizeCallback(GLFWwindow* window, int width, int height)
{
	Globals.screen_dimensions.x = width;
	Globals.screen_dimensions.y = height;

	glViewport(0, 0, width, height);
}


bool CheckCollision(glm::vec3& first, glm::vec3& second) { // AABB - AABB collision

	auto r = 0.0763892f; //0.0763892f     //radius of sphere
	// collision x-axis?
	bool collisionX = first.x + r >= second.x - r &&
		second.x + r >= first.x - r;
	// collision y-axis?
	bool collisionY = first.y + r >= second.y - r &&
		second.y + r >= first.y - r;
	// collision z-axis?
	bool collisionZ = first.z + r >= second.z - r &&
		second.z + r >= first.z - r;
	// collision only if on all axes
	return collisionX && collisionY && collisionZ;
}


int main(int argc, char* argv[])
{
	/* Set GLFW error callback */
	glfwSetErrorCallback(ErrorCallback);

	/* Initialize the library */
	if (!glfwInit())
	{
		std::cout << "Failed to initialize GLFW" << std::endl;
		return -1;
	}

	/* Create a windowed mode window and its OpenGL context */
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_FALSE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
	GLFWwindow* window = glfwCreateWindow(
		Globals.screen_dimensions.x, Globals.screen_dimensions.y,
		"Efehan Guner", NULL, NULL
	);
	if (!window)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	/* Move window to a certain position [do not change] */
	glfwSetWindowPos(window, 10, 50);
	/* Make the window's context current */
	glfwMakeContextCurrent(window);
	/* Enable VSync */
	glfwSwapInterval(1);

	/* Load OpenGL extensions with GLAD */
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		glfwTerminate();
		return -1;
	}

	/* Set GLFW Callbacks */
	glfwSetCursorPosCallback(window, CursorPositionCallback);
	glfwSetWindowSizeCallback(window, WindowSizeCallback);

	/* Configure OpenGL */
	glClearColor(0, 0, 0, 1);
	glEnable(GL_DEPTH_TEST);
	//blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/* Creating Meshes */
	std::vector<glm::vec3> positions1;
	std::vector<glm::vec3> normals1;
	std::vector<glm::vec2> uvs1;
	std::vector<GLuint> indices1;

	GenerateParametricShapeFrom2D(positions1, normals1, uvs1, indices1, ParametricHalfCircle, 1024, 1024);
	VAO sphereVAO(positions1, normals1, uvs1, indices1);

	std::vector<glm::vec3> positions2;
	std::vector<glm::vec3> normals2;
	std::vector<glm::vec2> uvs2;
	std::vector<GLuint> indices2;
	GenerateParametricShapeFrom2D(positions2, normals2, uvs2, indices2, ParametricCircle, 512, 512);
	VAO torusVAO(positions2, normals2, uvs2, indices2);
	/* Creating Textures */

	stbi_set_flip_vertically_on_load(true);

	auto filename = "Assets/mars_1k_color.jpg";
	int x, y, n;
	unsigned char* texture_data = stbi_load(filename, &x, &y, &n, 0);
	if (texture_data == NULL)
	{
		std::cout << "Texture " << filename << " failed to load." << std::endl;
		std::cout << "Error: " << stbi_failure_reason() << std::endl;
	}
	else
	{
		std::cout << "Texture " << filename << " is loaded, X:" << x << " Y:" << y << " N:" << n << std::endl;
	}

	GLuint texture;
	glGenTextures(1, &texture);

	if (x * n % 4 != 0)
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA,
		x, y, 0, n == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, texture_data
	);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glGenerateMipmap(GL_TEXTURE_2D);

	if (x * n % 4 != 0)
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	stbi_image_free(texture_data);

	/* Creating Programs */
	GLuint program = CreateProgramFromSources(
		R"VERTEX(
#version 330 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_uv;

uniform mat4 u_model;
uniform mat4 u_projection_view;

out vec4 world_space_position;
out vec3 world_space_normal;
out vec2 vertex_uv;

void main()
{
	world_space_position = u_model * vec4(a_position, 1);
	world_space_normal = vec3(u_model * vec4(a_normal, 0));
	vertex_uv = a_uv;

	gl_Position = u_projection_view * world_space_position;
}
		)VERTEX",

		R"FRAGMENT(
#version 330 core

uniform vec2 u_mouse_position;
uniform sampler2D u_texture;
uniform vec3 u_surface_color;
uniform vec3 textured;

in vec4 world_space_position;
in vec3 world_space_normal;
in vec2 vertex_uv;

out vec4 out_color;

void main()
{
	vec3 color = vec3(0);

	vec3 surface_position = world_space_position.xyz;
	vec3 surface_normal = normalize(world_space_normal);
	vec2 surface_uv = vertex_uv;
	vec3 texture_color = texture(u_texture, surface_uv).rgb;
	texture_color=texture_color * textured;
	vec3 surface_color = u_surface_color;

	vec3 ambient_color = vec3(0.7);
	color += ambient_color * surface_color * texture_color;

	vec3 light_direction = normalize(vec3(-1, -1, 1));
	vec3 to_light = -light_direction;

	vec3 light_color = vec3(0.3);

	float diffuse_intensity = max(0, dot(to_light, surface_normal));
	color += diffuse_intensity * light_color * surface_color;

	vec3 view_dir = vec3(0, 0, -1);	
	vec3 halfway_dir = normalize(view_dir + to_light);
	float shininess = 4;
	float specular_intensity = max(0, dot(halfway_dir, surface_normal));
	color += pow(specular_intensity, shininess) * light_color;

	out_color = vec4(color,1);
}
		)FRAGMENT");

	if (program == NULL)
	{
		glfwTerminate();
		return -1;
	}

	glUseProgram(program);

	auto texture_location = glGetUniformLocation(program, "u_texture");
	glUniform1i(texture_location, 0);

	glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture
	glBindTexture(GL_TEXTURE_2D, texture);



	auto mouse_location = glGetUniformLocation(program, "u_mouse_position");
	auto textured_location = glGetUniformLocation(program, "textured");
	auto model_location = glGetUniformLocation(program, "u_model");
	auto projection_view_location = glGetUniformLocation(program, "u_projection_view");
	auto surface_color_location = glGetUniformLocation(program, "u_surface_color");

	auto camera_position = glm::vec3(0, 0, -5);
	auto camera_up = glm::vec3(0, 1, 0);

	bool camera_mode = false;
	bool mode_set = false;
	bool rover_mode = false;

	auto movement = glm::vec3(0);
	auto chasing_pos1 = glm::vec3(0, 0, 0);
	auto chasing_pos2 = glm::vec3(0, 0, 0);
	bool first_run = true;

	bool rotate_tires1 = false;
	bool collision = false;
	bool collision1 = false;
	bool collision2 = false;

	float previous_time = glfwGetTime();
	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Calculate mouse position
		auto mouse_position = Globals.mouse_position;
		mouse_position /= glm::dvec2(Globals.screen_dimensions);
		mouse_position.y = 1. - mouse_position.y;
		mouse_position = mouse_position * 2. - 1.;

		glUniform2fv(mouse_location, 1, glm::value_ptr(glm::vec2(mouse_position)));


		//camera_front.x *= -1;

		float current_time = glfwGetTime();
		float delta_time = current_time - previous_time;
		previous_time = current_time;

		if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
		{
			camera_mode = true;
			rover_mode = false;
		}
		if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
		{
			camera_mode = false;
			rover_mode = true;
		}

		const float cameraSpeed = 0.01f; // adjust accordingly
		auto rover_speed = 0.8f; //0.5f

		if (collision1 || collision2) {
			rover_speed = 0.f;
			std::cout << ("User controlled rover has collided and stopped please restart the program if you want to move it") << std::endl; //debug

		}
		/*if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
		{
			camera_mode = !camera_mode;
		}*/

		//camera and rover movement

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			if (camera_mode)
				camera_position += cameraSpeed * camera_front;
			if (rover_mode)
				//movement += rover_speed * glm::vec3(0, 0, 0.1f);
			{
				movement += delta_time * glm::vec3(0, 0, rover_speed);
				rotate_tires1 = true;
			}

		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			if (camera_mode)
				camera_position -= cameraSpeed * camera_front;
			if (rover_mode)
				//movement += rover_speed * glm::vec3(0, 0, -0.1f);
			{
				movement += delta_time * glm::vec3(0, 0, -rover_speed);
				rotate_tires1 = true;
			}
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			if (camera_mode)
				camera_position += glm::normalize(glm::cross(camera_front, camera_up)) * cameraSpeed;
			if (rover_mode)
				//movement += rover_speed * glm::vec3(-0.1f, 0, 0);
			{
				movement += delta_time * glm::vec3(-rover_speed, 0, 0);
				rotate_tires1 = true;
			}
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			if (camera_mode)
				camera_position -= glm::normalize(glm::cross(camera_front, camera_up)) * cameraSpeed;
			if (rover_mode)
				//movement += rover_speed * glm::vec3(0.1f, 0, 0);
			{
				movement += delta_time * glm::vec3(rover_speed, 0, 0);
				rotate_tires1 = true;
			}
		}



		/*if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			camera_position.z += 0.01;
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			camera_position.z -= 0.01;
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			camera_position.x -= 0.05;
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			camera_position.x += 0.05;*/


			/*auto r = 3.;
			auto view = glm::lookAt(
				glm::vec3(cos(glfwGetTime()) * r, 1, sin(glfwGetTime()) * r),
				glm::vec3(0, 0, 0),
				glm::vec3(0, 1, 0)
			);*/

		auto view = glm::lookAt(
			camera_position,  //glm::vec3(0, 0, -1.15)
			camera_position + camera_front,
			camera_up
		);
		auto projection = glm::perspective(glm::radians(45.f), 1.f, 0.1f, 10.f); //far was 10.f

		glUniformMatrix4fv(projection_view_location, 1, GL_FALSE, glm::value_ptr(projection * view));


		//generate mars
		auto scale = glm::scale(glm::vec3(2.f));
		auto transform = scale;
		glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(transform));
		glUniform3fv(surface_color_location, 1, glm::value_ptr(glm::vec3(1, 1, 1)));
		glUniform3fv(textured_location, 1, glm::value_ptr(glm::vec3(1)));
		glBindVertexArray(sphereVAO.id);
		glDrawElements(GL_TRIANGLES, sphereVAO.element_array_count, GL_UNSIGNED_INT, 0);


		////rover movement
		//auto rover_speed = 0.1f;
		////auto movement = glm::vec3(0);
		//
		//if (!camera_mode ) {
		//	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		//		movement += rover_speed * glm::vec3(0, 0, 0.1f);
		//	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		//		movement += rover_speed * glm::vec3(0, 0, -0.1f);
		//	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		//		movement += rover_speed * glm::vec3(-0.1f, 0, 0);
		//	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		//		movement += rover_speed * glm::vec3(0.1f, 0, 0);
		//}

		//generate rovers
		//auto rover_move = glm::translate(movement);
		//auto moving_angle = glm::translate(movement);

		auto rover_scale = glm::scale(glm::vec3(0.08f));
		auto init_rover_pos = glm::vec3(0, 0, -2.2);
		auto rover_pos = init_rover_pos + movement;

		//std::cout << rover_pos.x << " " << rover_pos.y << " " << rover_pos.z << std::endl; //debug

		auto rover_move = glm::translate(rover_pos);
		//auto general_rover_transform = rover_translate * rover_scale;
		auto transform_rover = rover_move * rover_scale;
		glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(transform_rover));
		glUniform3fv(surface_color_location, 1, glm::value_ptr(glm::vec3(1, 0, 0)));
		glUniform3fv(textured_location, 1, glm::value_ptr(glm::vec3(0)));
		glBindVertexArray(sphereVAO.id);
		glDrawElements(GL_TRIANGLES, sphereVAO.element_array_count, GL_UNSIGNED_INT, 0);

		//tires

		scale = glm::scale(glm::vec3(0.015f));
		auto tire_translate = glm::translate(glm::vec3(0.05, -0.07, 0.02));
		auto rotate = glm::rotate(glm::radians(float(90)), glm::vec3(0, 0, 1));
		if(rotate_tires1)
			rotate *= glm::rotate(glm::radians(float(cos(glfwGetTime() * 40) + sin(glfwGetTime() * 40)) * 10), glm::vec3(0.1, 0, 0.1));
		auto transform_rover_tire = rover_move * tire_translate * scale * rotate;
		glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(transform_rover_tire));
		glUniform3fv(surface_color_location, 1, glm::value_ptr(glm::vec3(0, 0, 0)));
		glUniform3fv(textured_location, 1, glm::value_ptr(glm::vec3(0)));
		glBindVertexArray(torusVAO.id);
		glDrawElements(GL_TRIANGLES, torusVAO.element_array_count, GL_UNSIGNED_INT, 0);

		scale = glm::scale(glm::vec3(0.015f));
		tire_translate = glm::translate(glm::vec3(-0.05, -0.07, 0.02));
		rotate = glm::rotate(glm::radians(float(90)), glm::vec3(0, 0, 1));
		if (rotate_tires1)
			rotate *= glm::rotate(glm::radians(float(cos(glfwGetTime() * 40) + sin(glfwGetTime() * 40)) * 10), glm::vec3(0.1, 0, 0.1));
		transform_rover_tire = rover_move * tire_translate * scale * rotate;
		glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(transform_rover_tire));
		glUniform3fv(surface_color_location, 1, glm::value_ptr(glm::vec3(0, 0, 0)));
		glUniform3fv(textured_location, 1, glm::value_ptr(glm::vec3(0)));
		glBindVertexArray(torusVAO.id);
		glDrawElements(GL_TRIANGLES, torusVAO.element_array_count, GL_UNSIGNED_INT, 0);

		scale = glm::scale(glm::vec3(0.015f));
		tire_translate = glm::translate(glm::vec3(0.05, -0.07, -0.04));
		rotate = glm::rotate(glm::radians(float(90)), glm::vec3(0, 0, 1));
		if (rotate_tires1)
			rotate *= glm::rotate(glm::radians(float(cos(glfwGetTime() * 40) + sin(glfwGetTime() * 40)) * 10), glm::vec3(0.1, 0, 0.1));
		transform_rover_tire = rover_move * tire_translate * scale * rotate;
		glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(transform_rover_tire));
		glUniform3fv(surface_color_location, 1, glm::value_ptr(glm::vec3(0, 0, 0)));
		glUniform3fv(textured_location, 1, glm::value_ptr(glm::vec3(0)));
		glBindVertexArray(torusVAO.id);
		glDrawElements(GL_TRIANGLES, torusVAO.element_array_count, GL_UNSIGNED_INT, 0);

		scale = glm::scale(glm::vec3(0.015f));
		tire_translate = glm::translate(glm::vec3(-0.05, -0.07, -0.04));
		rotate = glm::rotate(glm::radians(float(90)), glm::vec3(0, 0, 1));
		if (rotate_tires1)
			rotate *= glm::rotate(glm::radians(float(cos(glfwGetTime() * 40) + sin(glfwGetTime() * 40)) * 10), glm::vec3(0.1, 0, 0.1));
		transform_rover_tire = rover_move * tire_translate * scale * rotate;
		glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(transform_rover_tire));
		glUniform3fv(surface_color_location, 1, glm::value_ptr(glm::vec3(0, 0, 0)));
		glUniform3fv(textured_location, 1, glm::value_ptr(glm::vec3(0)));
		glBindVertexArray(torusVAO.id);
		glDrawElements(GL_TRIANGLES, torusVAO.element_array_count, GL_UNSIGNED_INT, 0);

		//generate two catching rovers

		//first one
		auto rover2_offset = glm::vec3(0, 1.f, 0.2f) + init_rover_pos; //(0, 1.f, -0.3)

		if (first_run)
			chasing_pos1 = rover2_offset;
		auto chasing_move1 = glm::translate(chasing_pos1);
		if (rover_mode && !collision1) {
			chasing_pos1 = glm::mix(rover_pos, chasing_pos1, 0.99);
		}

		//auto translate_offset = glm::translate(glm::vec3(0, 0.5f, 0) + init_rover_pos);
		auto rover2_move = glm::translate(chasing_pos1);
		auto transform_rover2 = rover2_move * rover_scale;
		glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(transform_rover2));
		glUniform3fv(surface_color_location, 1, glm::value_ptr(glm::vec3(0, 0, 1)));
		glUniform3fv(textured_location, 1, glm::value_ptr(glm::vec3(0)));
		glBindVertexArray(sphereVAO.id);
		glDrawElements(GL_TRIANGLES, sphereVAO.element_array_count, GL_UNSIGNED_INT, 0);


		//tires

		scale = glm::scale(glm::vec3(0.015f));
		tire_translate = glm::translate(glm::vec3(0.05, -0.07, 0.02));
		rotate = glm::rotate(glm::radians(float(90)), glm::vec3(0, 0, 1));
		if (rover_mode)
			rotate *= glm::rotate(glm::radians(float(cos(glfwGetTime() * 40) + sin(glfwGetTime() * 40)) * 10), glm::vec3(0.1, 0, 0.1));
		transform_rover_tire = tire_translate * rover2_move * scale * rotate;
		glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(transform_rover_tire));
		glUniform3fv(surface_color_location, 1, glm::value_ptr(glm::vec3(0, 0, 0)));
		glUniform3fv(textured_location, 1, glm::value_ptr(glm::vec3(0)));
		glBindVertexArray(torusVAO.id);
		glDrawElements(GL_TRIANGLES, torusVAO.element_array_count, GL_UNSIGNED_INT, 0);

		scale = glm::scale(glm::vec3(0.015f));
		tire_translate = glm::translate(glm::vec3(-0.05, -0.07, 0.02));
		rotate = glm::rotate(glm::radians(float(90)), glm::vec3(0, 0, 1));
		if (rover_mode)
			rotate *= glm::rotate(glm::radians(float(cos(glfwGetTime() * 40) + sin(glfwGetTime() * 40)) * 10), glm::vec3(0.1, 0, 0.1));
		transform_rover_tire = tire_translate * rover2_move * scale * rotate;
		glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(transform_rover_tire));
		glUniform3fv(surface_color_location, 1, glm::value_ptr(glm::vec3(0, 0, 0)));
		glUniform3fv(textured_location, 1, glm::value_ptr(glm::vec3(0)));
		glBindVertexArray(torusVAO.id);
		glDrawElements(GL_TRIANGLES, torusVAO.element_array_count, GL_UNSIGNED_INT, 0);

		scale = glm::scale(glm::vec3(0.015f));
		tire_translate = glm::translate(glm::vec3(0.05, -0.07, -0.04));
		rotate = glm::rotate(glm::radians(float(90)), glm::vec3(0, 0, 1));
		if (rover_mode)
			rotate *= glm::rotate(glm::radians(float(cos(glfwGetTime() * 40) + sin(glfwGetTime() * 40)) * 10), glm::vec3(0.1, 0, 0.1));
		transform_rover_tire = tire_translate * rover2_move * scale * rotate;
		glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(transform_rover_tire));
		glUniform3fv(surface_color_location, 1, glm::value_ptr(glm::vec3(0, 0, 0)));
		glUniform3fv(textured_location, 1, glm::value_ptr(glm::vec3(0)));
		glBindVertexArray(torusVAO.id);
		glDrawElements(GL_TRIANGLES, torusVAO.element_array_count, GL_UNSIGNED_INT, 0);

		scale = glm::scale(glm::vec3(0.015f));
		tire_translate = glm::translate(glm::vec3(-0.05, -0.07, -0.04));
		rotate = glm::rotate(glm::radians(float(90)), glm::vec3(0, 0, 1));
		if (rover_mode)
			rotate *= glm::rotate(glm::radians(float(cos(glfwGetTime() * 40) + sin(glfwGetTime() * 40)) * 10), glm::vec3(0.1, 0, 0.1));
		transform_rover_tire = tire_translate * rover2_move * scale * rotate;
		glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(transform_rover_tire));
		glUniform3fv(surface_color_location, 1, glm::value_ptr(glm::vec3(0, 0, 0)));
		glUniform3fv(textured_location, 1, glm::value_ptr(glm::vec3(0)));
		glBindVertexArray(torusVAO.id);
		glDrawElements(GL_TRIANGLES, torusVAO.element_array_count, GL_UNSIGNED_INT, 0);

		if (CheckCollision(rover_pos, chasing_pos1)) {
			std::cout << ("Collision detected from first chasing rover!!!") << std::endl; //debug
			collision1 = true;

		}

		//second catching rover
		auto rover3_offset = glm::vec3(-1.f, 0, 0.2) + init_rover_pos;

		if (first_run)
			chasing_pos2 = rover3_offset;
		auto chasing_move2 = glm::translate(chasing_pos2);
		if (rover_mode && !collision2) {
			chasing_pos2 = glm::mix(rover_pos, chasing_pos2, 0.98);
		}

		auto rover3_move = glm::translate(chasing_pos2);
		auto transform_rover3 = rover3_move * rover_scale;
		glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(transform_rover3));
		glUniform3fv(surface_color_location, 1, glm::value_ptr(glm::vec3(1, 0, 1)));
		glUniform3fv(textured_location, 1, glm::value_ptr(glm::vec3(0)));
		glBindVertexArray(sphereVAO.id);
		glDrawElements(GL_TRIANGLES, sphereVAO.element_array_count, GL_UNSIGNED_INT, 0);

		//tires

		scale = glm::scale(glm::vec3(0.015f));
		tire_translate = glm::translate(glm::vec3(0.05, -0.07, 0.02));
		rotate = glm::rotate(glm::radians(float(90)), glm::vec3(0, 0, 1));
		if(rover_mode)
			rotate *= glm::rotate(glm::radians(float(cos(glfwGetTime() * 40) + sin(glfwGetTime() * 40)) * 10), glm::vec3(0.1, 0, 0.1));
		transform_rover_tire = tire_translate * rover3_move * scale * rotate;
		glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(transform_rover_tire));
		glUniform3fv(surface_color_location, 1, glm::value_ptr(glm::vec3(0, 0, 0)));
		glUniform3fv(textured_location, 1, glm::value_ptr(glm::vec3(0)));
		glBindVertexArray(torusVAO.id);
		glDrawElements(GL_TRIANGLES, torusVAO.element_array_count, GL_UNSIGNED_INT, 0);

		scale = glm::scale(glm::vec3(0.015f));
		tire_translate = glm::translate(glm::vec3(-0.05, -0.07, 0.02));
		rotate = glm::rotate(glm::radians(float(90)), glm::vec3(0, 0, 1));
		if (rover_mode)
			rotate *= glm::rotate(glm::radians(float(cos(glfwGetTime() * 40) + sin(glfwGetTime() * 40)) * 10), glm::vec3(0.1, 0, 0.1));
		transform_rover_tire = tire_translate * rover3_move * scale * rotate;
		glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(transform_rover_tire));
		glUniform3fv(surface_color_location, 1, glm::value_ptr(glm::vec3(0, 0, 0)));
		glUniform3fv(textured_location, 1, glm::value_ptr(glm::vec3(0)));
		glBindVertexArray(torusVAO.id);
		glDrawElements(GL_TRIANGLES, torusVAO.element_array_count, GL_UNSIGNED_INT, 0);

		scale = glm::scale(glm::vec3(0.015f));
		tire_translate = glm::translate(glm::vec3(0.05, -0.07, -0.04));
		rotate = glm::rotate(glm::radians(float(90)), glm::vec3(0, 0, 1));
		if (rover_mode)
			rotate *= glm::rotate(glm::radians(float(cos(glfwGetTime() * 40) + sin(glfwGetTime() * 40)) * 10), glm::vec3(0.1, 0, 0.1));
		transform_rover_tire = tire_translate * rover3_move * scale * rotate;
		glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(transform_rover_tire));
		glUniform3fv(surface_color_location, 1, glm::value_ptr(glm::vec3(0, 0, 0)));
		glUniform3fv(textured_location, 1, glm::value_ptr(glm::vec3(0)));
		glBindVertexArray(torusVAO.id);
		glDrawElements(GL_TRIANGLES, torusVAO.element_array_count, GL_UNSIGNED_INT, 0);

		scale = glm::scale(glm::vec3(0.015f));
		tire_translate = glm::translate(glm::vec3(-0.05, -0.07, -0.04));
		rotate = glm::rotate(glm::radians(float(90)), glm::vec3(0, 0, 1));
		if (rover_mode)
			rotate *= glm::rotate(glm::radians(float(cos(glfwGetTime() * 40) + sin(glfwGetTime() * 40)) * 10), glm::vec3(0.1, 0, 0.1));
		transform_rover_tire = tire_translate * rover3_move * scale * rotate;
		glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(transform_rover_tire));
		glUniform3fv(surface_color_location, 1, glm::value_ptr(glm::vec3(0, 0, 0)));
		glUniform3fv(textured_location, 1, glm::value_ptr(glm::vec3(0)));
		glBindVertexArray(torusVAO.id);
		glDrawElements(GL_TRIANGLES, torusVAO.element_array_count, GL_UNSIGNED_INT, 0);

		if (CheckCollision(rover_pos, chasing_pos2)) {
			std::cout << ("Collision detected from second chasing rover!!!") << std::endl; //debug
			collision2 = true;

		}




		first_run = false;
		rotate_tires1 = false;
		/* Swap front and back buffers */
		glfwSwapBuffers(window);


		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}