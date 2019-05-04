#include "common.h"
#include "ShaderProgram.h"
#include "LiteMath.h"
#include "obj_loader.h"

#define GLFW_DLL
#include <GLFW/glfw3.h>

#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static GLsizei WIDTH = 1280, HEIGHT = 720;

using namespace LiteMath;

GLfloat deltaTime = 0.0f, lastFrame = 0.0f;
double lastX, lastY;
float3 g_camStartPos(0, 0, 3), g_camFront(0, 0, -1), g_camUp(0, 1, 0);
float3 g_camPos = g_camStartPos;
float  cam_rot[2] = {0, 0};

GLint mode = 0;
bool stop = false;

bool keys[1024];
bool g_captureMouse = true;
GLfloat fieldOfView = 45.0;

void windowResize(GLFWwindow* window, int width, int height) {
	
	WIDTH  = width;
	HEIGHT = height;
	
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	
	//std::cout << key << std::endl;
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (action == GLFW_PRESS)
		keys[key] = true;
	else if (action == GLFW_RELEASE)
		keys[key] = false;
	
}

void checkKey() {
	
	float d = 5.0f * deltaTime;
	if (keys[GLFW_KEY_W])
		g_camPos += g_camFront * d;
	if (keys[GLFW_KEY_S])
		g_camPos -= g_camFront * d;
	if (keys[GLFW_KEY_A])
		g_camPos -= normalize(cross(g_camFront, g_camUp)) * d;
	if (keys[GLFW_KEY_D])
		g_camPos += normalize(cross(g_camFront, g_camUp)) * d;
	if (keys[GLFW_KEY_0]) {
		g_camPos = g_camStartPos;
		mode = 0;
		stop = false;
		cam_rot[0] = cam_rot[1] = 0;
	}
	if (keys[GLFW_KEY_1])
		switch (mode) {
			case 0: mode = 1; break;
			case 1: mode = 0; break;
			case 2: mode = 3; break;
			case 3: mode = 2; break;
		}
	if (keys[GLFW_KEY_2])
		switch (mode) {
			case 0: mode = 2; break;
			case 1: mode = 3; break;
			case 2: mode = 0; break;
			case 3: mode = 1; break;
		}
	if (keys[GLFW_KEY_3])
		stop = !stop;
	
}

void mouseKey(GLFWwindow* window, int key, int action, int mods) {
	
	if (key == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
		g_captureMouse = !g_captureMouse;
	
	if (g_captureMouse)
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	else
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	
}

void mouseScroll(GLFWwindow* window, double xoffset, double yoffset) {
	
	fieldOfView -= yoffset;
	if (fieldOfView < 30) fieldOfView = 30.0;
	if (fieldOfView > 150) fieldOfView = 150.0;
	
}

void mouseMove(GLFWwindow* window, double xpos, double ypos)
{
//	std::cout << xpos << ' ' << ypos << std::endl;
    GLfloat sensitivity = 0.01;
    cam_rot[0] += (xpos - lastX) * sensitivity;
    cam_rot[1] += (lastY - ypos) * sensitivity;
	lastX = xpos;
    lastY = ypos;

    if(cam_rot[1] > PI/3)
        cam_rot[1] = PI/3;
    if(cam_rot[1] < -PI/3)
        cam_rot[1] = -PI/3;

    float3 front;
    front.x = sin(cam_rot[0]) * cos(cam_rot[1]);
    front.y = sin(cam_rot[1]);
    front.z = -cos(cam_rot[0]) * cos(cam_rot[1]);
    g_camFront = normalize(front);
}

GLuint loadTexture(const std::string &imagepath) {
    // stbi_set_flip_vertically_on_load(1);
    int width, height, nrChannels;
    unsigned char* rgb = stbi_load(imagepath.c_str(), &width, &height, &nrChannels, 3);

    // Create one OpenGL texture
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL,  0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, rgb);
    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(rgb);
    return textureID;
}

GLuint loadCubemap(std::vector<std::string> faces) {
	
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
	
}

int initGL() {
	
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize OpenGL context" << std::endl;
		return -1;
	}

	std::cout << "Vendor: "   << glGetString(GL_VENDOR) << std::endl;
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "Version: "  << glGetString(GL_VERSION) << std::endl;
	std::cout << "GLSL: "     << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

	return 0;
	
}

int main() {
	
	if(!glfwInit())
		return -1;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
	
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Space shooter", nullptr, nullptr);    
	if (window == nullptr) {
		
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
		
	}
	
	glfwMakeContextCurrent(window);
	glfwSetWindowSizeCallback(window, windowResize);
	glfwSetKeyCallback(window, key_callback);
	glfwGetCursorPos(window, &lastX, &lastY);
	glfwSetCursorPosCallback(window, mouseMove);
	glfwSetMouseButtonCallback(window, mouseKey);
	glfwSetScrollCallback(window, mouseScroll);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	
	if(initGL())
		return 1;
	GLenum gl_error = glGetError();
	while (gl_error != GL_NO_ERROR)
		gl_error = glGetError();
	
	std::vector<float> vertices, uvs, normals;
	if (!loadOBJ("obj/ufo.obj", vertices, uvs, normals)) {
		std::cout << "obj load error" << std::endl;
		glfwTerminate();
		return 1;
	}
	
	GLuint tex_diffuse = loadTexture("obj/ufo_diffuse.jpg");
	glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, tex_diffuse);
	
	std::vector<std::string> faces {
		"sky/right.png",
		"sky/left.png",
		"sky/top.png",
		"sky/bottom.png",
		"sky/front.png",
		"sky/back.png"
	};
	GLuint cubemapTexture = loadCubemap(faces);
	
	std::unordered_map<GLenum, std::string> shaders;
	shaders[GL_VERTEX_SHADER]   = "vertex.glsl";
	shaders[GL_FRAGMENT_SHADER] = "fragment.glsl";
	ShaderProgram program(shaders); GL_CHECK_ERRORS;
	
	std::unordered_map<GLenum, std::string> skyboxShader;
	skyboxShader[GL_VERTEX_SHADER]   = "skyboxVertex.glsl";
	skyboxShader[GL_FRAGMENT_SHADER] = "skyboxFragment.glsl";
	ShaderProgram skyboxProgram(skyboxShader); GL_CHECK_ERRORS;
	
	GLfloat skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

	// skybox VAO
    GLuint skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
	
		glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLfloat*) 0);
	
	glBindVertexArray(0);
	
    GLuint VBO, VAO, UVS, NORM;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
	glGenBuffers(1, &UVS);
	glGenBuffers(1, &NORM);
    glBindVertexArray(VAO);
		
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*) 0);
		
		glBindBuffer(GL_ARRAY_BUFFER, UVS);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * uvs.size(), uvs.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*) 0);
		
		glBindBuffer(GL_ARRAY_BUFFER, NORM);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * normals.size(), normals.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*) 0);
		
    glBindVertexArray(0);

	glfwSwapInterval(1);
//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_DEPTH_TEST);

	float4x4 model, view, projection;
	GLfloat timeValue;

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		checkKey();
		
		glViewport(0, 0, WIDTH, HEIGHT);
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		timeValue = glfwGetTime();
		deltaTime = timeValue - lastFrame;
		lastFrame = timeValue;
		
		model = mul(translate4x4(float3(0, sin(timeValue), 0)), rotate_X_4x4(PI/2));
		view = transpose(lookAtTransposed(g_camPos, g_camPos + g_camFront, g_camUp));
		projection = transpose(projectionMatrixTransposed(fieldOfView, (float) WIDTH / (float) HEIGHT, 0.1f, 100.0f));
		
		program.StartUseShader();
	//		std::cout << g_camPos.x << ' ' << g_camPos.y << ' ' << g_camPos.z << std::endl;
			
			program.SetUniform("model" , model);
			program.SetUniform("view", view);
			program.SetUniform("projection", projection);
//			program.SetUniform("diffuse", 3);
			program.SetUniform("lightPosition" , float3(5*cos(timeValue), 5, 5*sin(timeValue)));
			
			glBindVertexArray(VAO);
				glBindTexture(GL_TEXTURE_2D, tex_diffuse);
				glDrawArrays(GL_TRIANGLES, 0, vertices.size());
			glBindVertexArray(0);
			
		program.StopUseShader();
		
		view = transpose(lookAtTransposed(float3(0, 0, 0), g_camFront, g_camUp));
		
		glDepthFunc(GL_LEQUAL);
		skyboxProgram.StartUseShader();
			skyboxProgram.SetUniform("view", view);
			skyboxProgram.SetUniform("projection", projection);
			
			glBindVertexArray(skyboxVAO);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
				glDrawArrays(GL_TRIANGLES, 0, 36);
			glBindVertexArray(0);
		skyboxProgram.StopUseShader();
		glDepthFunc(GL_LESS);
		
		glfwSwapBuffers(window);
	}
	
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
	
	glfwTerminate();
	return 0;
}
