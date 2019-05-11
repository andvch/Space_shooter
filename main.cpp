#include "common.h"
#include "ShaderProgram.h"
#include "LiteMath.h"
#include "obj_loader.h"

#define GLFW_DLL
#include <GLFW/glfw3.h>

#include <ctime>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define RND(MIN, MAX) (MIN + (MAX - MIN) * (float(rand()) / RAND_MAX))

//////////

static GLsizei WIDTH = 1280, HEIGHT = 720;

#define OBJ_OBJECTS 5
#define SKYBOXES 3
#define MAX_UNITS 10
#define MAX_AMMO 30
#define PLAYER_AMMO 15
#define MAX_PARTICLES 100

std::vector<std::string> obj_files {
	"obj/ufo1.obj",
	"obj/ufo2.obj",
	"obj/ufo3.obj",
	"obj/asteroid.obj",
	"obj/asteroid.obj"
};

std::vector<std::string> jpg_files {
	"obj/ufo1_diffuse.jpg",
	"obj/ufo2_diffuse.jpg",
	"obj/ufo3_diffuse.jpg",
	"obj/asteroid_diffuse1.jpg",
	"obj/asteroid_diffuse2.jpg"
};

std::string interface_file = "interface.png";

std::string obj_cube = "obj/cube.obj";

std::vector<std::string> skybox1_files {
	"sky1/right.png",
	"sky1/left.png",
	"sky1/top.png",
	"sky1/bottom.png",
	"sky1/front.png",
	"sky1/back.png"
};

std::vector<std::string> skybox2_files {
	"sky2/right.png",
	"sky2/left.png",
	"sky2/top.png",
	"sky2/bottom.png",
	"sky2/front.png",
	"sky2/back.png"
};

std::vector<std::string> skybox3_files {
	"sky3/right.png",
	"sky3/left.png",
	"sky3/top.png",
	"sky3/bottom.png",
	"sky3/front.png",
	"sky3/back.png"
};

using namespace LiteMath;

int playerHP = 100, playerScore = 0;
float playerDefaultSpeed = 3.0, maxDistance = 50, particlesDistance = 10;
float playerSpeed = playerDefaultSpeed;
GLfloat timeValue, deltaTime = 0.0, lastFrame = 0.0;

float3 playerPos(0, 0, 0), playerMove(0, 0, playerSpeed),
	camFront(0, 0, -1), camUp(0, 1, 0);
bool playerFire = false;

bool hyperMode = false;
int hyperModeStatus, sky_num = 0;
float hyperModeTimer, hyperModeCoef = 0, hyperModeSpeed = 20.0,
	hyperModeFOV = 3*PI/4, hyperModeTime[] = {3, 3, 3};

double lastX, lastY;
float2 aim(0, 0);
const float yaw = PI/6, pitch = PI/6;

bool keys[1024];
bool g_captureMouse = true;
float fieldOfViewDefault = PI/4;
float fieldOfView = fieldOfViewDefault;

int initGL()
{
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize OpenGL context" << std::endl;
		return 1;
	}

	std::cout << "Vendor: "   << glGetString(GL_VENDOR) << std::endl;
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "Version: "  << glGetString(GL_VERSION) << std::endl;
	std::cout << "GLSL: "     << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

	return 0;
}

void windowResize(GLFWwindow* window, int width, int height)
{
	WIDTH  = width;
	HEIGHT = height;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	// std::cout << key << std::endl;
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (action == GLFW_PRESS)
		keys[key] = true;
	else if (action == GLFW_RELEASE)
		keys[key] = false;
}

void checkKey()
{
	if (keys[GLFW_KEY_W] && keys[GLFW_KEY_S]) {
		playerMove.y = 0;
	} else if (keys[GLFW_KEY_W]) {
		playerMove.y = -playerSpeed;
	} else if (keys[GLFW_KEY_S]) {
		playerMove.y = playerSpeed;
	} else {
		playerMove.y = 0;
	}

	if (keys[GLFW_KEY_A] && keys[GLFW_KEY_D]) {
		playerMove.x = 0;
	} else if (keys[GLFW_KEY_A]) {
		playerMove.x = playerSpeed;
	} else if (keys[GLFW_KEY_D]) {
		playerMove.x = -playerSpeed;
	} else {
		playerMove.x = 0;
	}

	if (keys[GLFW_KEY_1]) {
		if (!hyperMode) {
			hyperMode = true;
			hyperModeStatus = 0;
			hyperModeTimer = hyperModeTime[0];
		}
	}
}

void mouseKey(GLFWwindow* window, int key, int action, int mods)
{
	if (key == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
		playerFire = true;
		return;
	}

	if (key == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE)
		g_captureMouse = !g_captureMouse;

	if (g_captureMouse)
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	else
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void mouseMove(GLFWwindow* window, double xpos, double ypos)
{
	GLfloat sensitivity = 0.01;
	aim.x += (xpos - lastX) * sensitivity;
	aim.y += (lastY - ypos) * sensitivity;
	lastX = xpos;
	lastY = ypos;

	if(aim.x > 1.0)
		aim.x = 1.0;
	if(aim.x < -1.0)
		aim.x = -1.0;

	if(aim.y > 1.0)
		aim.y = 1.0;
	if(aim.y < -1.0)
		aim.y = -1.0;
}

static inline float3 calcDir(float y, float p)
{
	float3 dir;
	dir.x = sin(y) * cos(p);
	dir.y = sin(p);
	dir.z = -cos(y) * cos(p);
	return normalize(dir);
}

static inline float4x4 rotMatrix(float3 rot)
{
	return
	mul(rotate_Z_4x4(rot.z),
	mul(rotate_Y_4x4(rot.y),
	rotate_X_4x4(rot.x)));
}

static inline float4x4 modelMatrix(float3 pos, float3 rot, float size = 1.0)
{
	return
	mul(translate4x4(pos),
	mul(rotMatrix(rot),
	scale4x4(float3(size, size, size))));
}

class GraphicOBJ {

	GLuint VAO, VBO[3], TID;
	int triangles;
	bool texture;
	GLenum texture_type;

	public:

	GraphicOBJ() : texture(false)
	{}

	bool setObj(const std::vector<float> & vertices,
		const std::vector<float> & uvs = std::vector<float>(0),
		const std::vector<float> & normals = std::vector<float>(0))
	{
		glGenVertexArrays(1, &VAO);
		glGenBuffers(3, VBO);
		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(),
			vertices.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*) 0);

		triangles = vertices.size() / 3;

		if (!uvs.empty()) {
			glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * uvs.size(),
				uvs.data(), GL_STATIC_DRAW);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*) 0);
		}

		if (!normals.empty()) {
			glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * normals.size(),
				normals.data(), GL_STATIC_DRAW);
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*) 0);
		}

		glBindVertexArray(0);
		return true;
	}

	bool setTexture(std::string img)
	{
		// stbi_set_flip_vertically_on_load(1);
		int width, height, nrChannels;
		unsigned char* data = stbi_load(img.c_str(), &width, &height, &nrChannels, 4);
		if (!data) {
			stbi_image_free(data);
			return texture = false;
		}
		glGenTextures(1, &TID);
		glBindTexture(GL_TEXTURE_2D, TID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL,  0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		stbi_image_free(data);
		texture_type = GL_TEXTURE_2D;
		return texture = true;
	}

	bool setCubemap(std::vector<std::string> faces)
	{
		// stbi_set_flip_vertically_on_load(1);
		int width, height, nrChannels;
		glGenTextures(1, &TID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, TID);
		for (unsigned int i = 0; i < faces.size(); i++) {
			unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
			if (!data) {
				stbi_image_free(data);
				return texture = false;
			}
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		texture_type = GL_TEXTURE_CUBE_MAP;
		return texture = true;
	}

	void draw()
	{
		glBindVertexArray(VAO);
		if (texture)
			glBindTexture(texture_type, TID);
		glDrawArrays(GL_TRIANGLES, 0, triangles);
		if (texture)
			glBindTexture(texture_type, 0);
		glBindVertexArray(0);
	}

	~GraphicOBJ()
	{
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(3, VBO);
		if (texture)
			glDeleteTextures(1, &TID);
	}

};

GraphicOBJ objects[OBJ_OBJECTS], skybox[SKYBOXES], square;

bool load_objects()
{
	std::vector<float> vertices, uvs, normals;
	for (int i = 0; i < OBJ_OBJECTS; ++i) {
		if (!loadOBJ(obj_files[i].c_str(), vertices, uvs, normals))
			return false;
		objects[i].setObj(vertices, uvs, normals);
		if (!objects[i].setTexture(jpg_files[i]))
			return false;
	}

	vertices = {
		-1.0f,  1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f
	};
	square.setObj(vertices);
	if (!square.setTexture(interface_file))
		return false;

	if (!loadOBJ(obj_cube.c_str(), vertices, uvs, normals))
		return false;
	skybox[0].setObj(vertices, uvs, normals);
	skybox[1].setObj(vertices, uvs, normals);
	skybox[2].setObj(vertices, uvs, normals);
	if (!skybox[0].setCubemap(skybox1_files))
		return false;
	if (!skybox[1].setCubemap(skybox2_files))
		return false;
	if (!skybox[2].setCubemap(skybox3_files))
		return false;
	return true;
}

class unit {

	int id, hp;
	float size, speed, rot_speed, timer, decay, destroy_time;
	float3 pos, dir, rot, delta_rot;
	bool shooting, destroy;
	float4x4 def_config;

	float3 calcRot(float3 target)
	{
		float3 r, tmp = normalize(target - pos);
		r.x = -asin(tmp.y);
		r.z = 0;
		if (tmp.z > 0) {
			r.y = atan(tmp.x / tmp.z);
			return r;
		}
		if (tmp.z < 0) {
			r.y = atan(tmp.x / tmp.z) +
				((tmp.x >= 0) ? PI : -PI);
			return r;
		}
		r.y = ((tmp.x >= 0) ? PI/2 : -PI/2);
		return r;
	}

	void des()
	{
		decay = 1 - timer / destroy_time;
		size *= pow(3, deltaTime);
		if (timer < 0) {
			destroy = false;
			timer = RND(2, 5);
		}
	}

	void recharge()
	{
		if (shooting)
			timer = RND(2, 5);
	}

	public:

	unit() : hp(0),	timer(0.1), destroy(false)
	{}

	void time(float delta)
	{
		timer -= delta;
	}

	bool exist()
	{
		if (hp > 0 || destroy)
			return true;
		if (timer > 0)
			return false;
		spawn(rand() % (OBJ_OBJECTS));
		return true;
	}

	bool setRespawn(float t)
	{
		if (!exist()) {
			timer = t;
			return true;
		}
		return false;
	}

	void spawn(int i)
	{
		id = i;
		pos = float3(RND(-maxDistance / 3, maxDistance / 3),
			RND(-maxDistance / 5, maxDistance / 5), -maxDistance);
		size = 1;
		decay = 0;
		rot_speed = 1;
		delta_rot = rot = float3(0, 0, 0);
		if (id < 3) {
			shooting = true;
			recharge();
		} else {
			shooting = false;
		}
		destroy_time = 1;
		switch(id) {
		case 0:
			hp = 3;
			speed = 3;
			rot = float3(PI/2, 0, 0);
			delta_rot = float3(0, (rand() % 2) ? 1 : -1, 0);
			break;
		case 1:
			hp = 6;
			speed = 2;
			rot = float3(0, PI/2, PI/2);
			break;
		case 2:
			hp = 2;
			speed = 5;
			rot = float3(0, PI/2, PI/2);
			break;
		case 3:
		case 4:
			hp = 1;
			size = RND(0.5, 2);
			speed = 0;
			rot_speed = RND(0, 1);
			rot = float3(RND(-1, 1), RND(-1, 1), RND(-1, 1));
			delta_rot = float3(RND(-1, 1), RND(-1, 1), RND(-1, 1));
			break;
		case 5:
			hp = 1;
			pos *= particlesDistance / maxDistance;
			pos.z *= RND(0, 1);
			size = RND(0.001, 0.02);
			speed = 0;
			rot_speed = RND(0, 1);
			rot = float3(RND(-1, 1), RND(-1, 1), RND(-1, 1));
			delta_rot = float3(RND(-1, 1), RND(-1, 1), RND(-1, 1));
		default:
			break;
		}
		def_config = rotMatrix(rot);
		rot = float3(0, 0, 0);
	}

	bool fire(float3 p)
	{
		if (exist())
			return false;
		spawn(6);
		hp = 3;
		size = 0.3;
		speed = 30;
		float fieldW = atan(tan(fieldOfView / 2) * float(WIDTH) / float(HEIGHT));
		dir = calcDir(aim.x * (yaw + fieldW),
			aim.y * (pitch + fieldOfView / 2));
		pos = p + size * dir;
		destroy = false;
		return true;
	}

	bool fire(unit & a)
	{
		if (!a.shooting)
			return true;
		if (a.timer > 0)
			return true;
		float3 target = playerPos;
		if (exist())
			return false;
		spawn(6);
		hp = 1;
		size = 0.15;
		speed = a.speed + 2;
		{
			float L = -(a.pos.z - playerPos.z),
				l = sqrt(a.pos.x * a.pos.x + a.pos.y * a.pos.y),
				v = playerSpeed, u = speed, d, t;
			if (fabs(u - v) > 0.1) {
				d = L * L * v * v +
					(u * u - v * v) * (L * L + l * l);
				if (d < 0)
					return true;
				t = (-L * v + sqrt(d)) / (u * u - v * v);
			} else {
				if (v > 0.1 && L > 0.1)
					t = (L * L + l * l) / (2 * v * L);
				else
					t = 0;
			}
			target.z -= v * t;
		}
		dir = normalize(target - a.pos);
		pos = a.pos  + (a.size + size) * dir;
		destroy = false;
		a.recharge();
		return true;
	}

	void setTarget(float3 target)
	{
		dir = normalize(target - pos);
		if (id == 1 || id == 2)
			delta_rot = calcRot(target) - rot;
	}

	void move(float3 offset)
	{
		if (destroy)
			des();
		rot += deltaTime * rot_speed * delta_rot;
		pos += deltaTime * (speed * dir + offset);
		if (pos.z > playerPos.z ||
			fabs(pos.x - playerPos.x) > maxDistance ||
			fabs(pos.y - playerPos.y) > maxDistance ||
			fabs(pos.z - playerPos.z) > maxDistance)
		{
			hp = 0;
			destroy = false;
		}
	}

	void damage(int n)
	{
		hp -= n;
		speed *= pow(0.75, n);
		if (hp > 0 || id > 4)
			return;
		timer = destroy_time;
		destroy = true;
	}

	int impact(float3 p)
	{
		if (!exist() || destroy)
			return 0;
		if (length(p - pos) > size)
			return 0;
		int n = hp;
		damage(hp);
		return n;
	}

	int impact(unit & a)
	{
		if (!exist() || destroy || !a.exist() || a.destroy)
			return 0;
		if (length(a.pos - pos) > a.size + size)
			return 0;
		int n = a.hp;
		a.damage(hp);
		damage(n);
		return (a.hp > 0) ? n - a.hp : n;
	}

	float4x4 getMatrix()
	{
		return mul(modelMatrix(pos, rot, size), def_config);
	}

	float4x4 getMatrix(float3 rotTarget, bool explosion = false)
	{
		float3 r = calcRot(rotTarget);
		return modelMatrix(pos, r, (explosion) ? size * decay : size);
	}

	float getDecay() { return decay; }
	int getID() { return id; }
};

void hyper()
{
	hyperModeTimer -= deltaTime;
	if (hyperModeTimer <=0 )
		if (++hyperModeStatus < 3) {
			hyperModeTimer = hyperModeTime[hyperModeStatus];
			if (hyperModeStatus == 1) {
				hyperModeCoef = 1.0;
				sky_num = (sky_num + 1) % SKYBOXES;
			}
		} else {
			hyperModeCoef = 0;
			playerSpeed = playerDefaultSpeed;
			fieldOfView = fieldOfViewDefault;
			hyperMode = false;
		}
	if (hyperModeStatus == 0 || hyperModeStatus == 2) {
		hyperModeCoef = hyperModeTimer / hyperModeTime[hyperModeStatus];
		if (hyperModeStatus == 0)
			hyperModeCoef = 1.0 - hyperModeCoef;
		playerSpeed = playerDefaultSpeed +
			hyperModeCoef * (hyperModeSpeed - playerDefaultSpeed);
		fieldOfView = fieldOfViewDefault +
			hyperModeCoef * (hyperModeFOV - fieldOfViewDefault);
	}
}

int main()
{
	srand(time(NULL));
	if(!glfwInit())
		return -1;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Space shooter",
		nullptr, nullptr);
	if (window == nullptr) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return 1;
	}

	glfwMakeContextCurrent(window);
	glfwSetWindowSizeCallback(window, windowResize);
	glfwSetKeyCallback(window, keyCallback);
	glfwGetCursorPos(window, &lastX, &lastY);
	glfwSetCursorPosCallback(window, mouseMove);
	glfwSetMouseButtonCallback(window, mouseKey);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if(initGL())
		return 1;
	GLenum gl_error = glGetError();
	while (gl_error != GL_NO_ERROR)
		gl_error = glGetError();

	std::unordered_map<GLenum, std::string> shaders;
	shaders[GL_VERTEX_SHADER]   = "vertex.glsl";
	shaders[GL_GEOMETRY_SHADER] = "geometry.glsl";
	shaders[GL_FRAGMENT_SHADER] = "fragment.glsl";
	ShaderProgram program(shaders); GL_CHECK_ERRORS;

	shaders.erase(GL_GEOMETRY_SHADER);
	shaders[GL_VERTEX_SHADER]   = "vertexSkybox.glsl";
	shaders[GL_FRAGMENT_SHADER] = "fragmentSkybox.glsl";
	ShaderProgram skyboxProgram(shaders); GL_CHECK_ERRORS;

	shaders[GL_VERTEX_SHADER]   = "vertexSimple.glsl";
	shaders[GL_FRAGMENT_SHADER] = "fragmentCircl.glsl";
	ShaderProgram circlProgram(shaders); GL_CHECK_ERRORS;

	shaders[GL_FRAGMENT_SHADER] = "fragmentMenu.glsl";
	ShaderProgram menuProgram(shaders); GL_CHECK_ERRORS;

	if (!load_objects()) {
		std::cout << "Failed to load objects" << std::endl;
		return 1;
	}

	glfwSwapInterval(1);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	float4x4 PV, PVM, skyboxPV;
	const float4x4 I;
	float redScreen = 0, tmp;
	int i, j, k;

	unit units[MAX_UNITS], ammo[MAX_AMMO], particles[MAX_PARTICLES];
	for (i = 0; i < MAX_UNITS; ++i)
		units[i].setRespawn(5.0 * (i + 1));

	while (!glfwWindowShouldClose(window) && playerHP > 0) {
		glfwPollEvents();
		checkKey();
		glViewport(0, 0, WIDTH, HEIGHT);
		glClearColor(0.2, 0.3, 0.3, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		timeValue = glfwGetTime();
		deltaTime = timeValue - lastFrame;
		lastFrame = timeValue;

		if (hyperMode)
			hyper();
		playerMove.z = playerSpeed;
		camFront = calcDir(aim.x * yaw * (1 - hyperModeCoef),
			aim.y * pitch * (1 - hyperModeCoef));

		PV = transpose(mul(lookAtTransposed(playerPos, playerPos + camFront, camUp),
			projectionMatrixTransposed(180 * fieldOfView / PI,
				float(WIDTH) / float(HEIGHT), 0.1f, maxDistance)));
		skyboxPV = PV;
		//skyboxPV = transpose(mul(lookAtTransposed(float3(0, 0, 0), camFront, camUp),
		//	projectionMatrixTransposed(180 * fieldOfView / PI,
		//		float(WIDTH) / float(HEIGHT), 0.1f, maxDistance)));

		program.StartUseShader();
		for (i = 0; i < MAX_UNITS; ++i) {
			units[i].time(deltaTime);
			if (!units[i].exist())
				continue;
			units[i].setTarget(playerPos);
			units[i].move(playerMove);
			for (j = PLAYER_AMMO; j < MAX_AMMO; ++j) {
				if (ammo[j].fire(units[i]))
					break;
			}
			for (j = 0; j < i; ++j)
				units[i].impact(units[j]);
			k = units[i].impact(playerPos);
			if (k > 0) {
				if (!hyperMode) {
					playerHP -= k;
					redScreen = 1;
				} else {
					playerScore += 5 * k;
				}
			}

			PVM = mul(PV, units[i].getMatrix());
			program.SetUniform("trans", PVM);
			program.SetUniform("decay", units[i].getDecay());
			objects[units[i].getID()].draw();
		}
		program.StopUseShader();

		glDepthFunc(GL_LEQUAL);
		skyboxProgram.StartUseShader();
		skyboxProgram.SetUniform("trans"   , skyboxPV);
		skyboxProgram.SetUniform("mixCoef" , hyperModeCoef);
		skyboxProgram.SetUniform("skyColor",
				float4(fabs(sin(timeValue - 1)),
					fabs(sin(timeValue)),
					fabs(sin(timeValue + 1)), 1.0));
		skybox[sky_num].draw();
		skyboxProgram.StopUseShader();
		glDepthFunc(GL_LESS);

		circlProgram.StartUseShader();

		for (i = 0; i < MAX_UNITS; ++i) {
			if (!units[i].exist())
				continue;
			tmp = units[i].getDecay();
			if (tmp < 0.1)
				continue;
			PVM = mul(PV, units[i].getMatrix(playerPos, true));
			circlProgram.SetUniform("trans", PVM);
			circlProgram.SetUniform("circlColor",
				float4(1.0, 1.0, 1.0, 1.0 - tmp));
			square.draw();
		}

		for (i = 0; i < MAX_AMMO ; ++i) {
			if (playerFire && i < PLAYER_AMMO)
				if (ammo[i].fire(playerPos))
					playerFire = false;
			if (!ammo[i].exist())
				continue;
			ammo[i].move(playerMove);
			for (j = 0; j < MAX_UNITS; ++j) {
				k = ammo[i].impact(units[j]);
				if (i < PLAYER_AMMO)
					playerScore += k;
			}
			k = ammo[i].impact(playerPos);
			if (k > 0 && !hyperMode) {
				playerHP -= k;
				redScreen = 1;
			}

			PVM = mul(PV, ammo[i].getMatrix(playerPos));
			circlProgram.SetUniform("trans", PVM);
			circlProgram.SetUniform("circlColor",
				float4(1.0, 1.0, 1.0, 1.0));
			square.draw();
		}

		for (i = 0; i < MAX_PARTICLES; ++i) {
			if (!particles[i].exist())
				particles[i].spawn(5);
			particles[i].move(playerMove);

			PVM = mul(PV, particles[i].getMatrix());
			circlProgram.SetUniform("trans", PVM);
			circlProgram.SetUniform("circlColor",
				float4(0.2, 0.2, 0.2, 1.0));
			square.draw();
		}
		circlProgram.StopUseShader();

		glDepthFunc(GL_ALWAYS);
		menuProgram.StartUseShader();
		menuProgram.SetUniform("trans" , I);
		menuProgram.SetUniform("aimPos", aim);
		redScreen -= deltaTime;
		if (redScreen < 0 || playerHP <= 0)
			redScreen = 0;
		menuProgram.SetUniform("menuColor", float3(redScreen, 1 - redScreen, 0));
		menuProgram.SetUniform("playerHP"   , playerHP);
		menuProgram.SetUniform("playerScore", playerScore);
		square.draw();
		menuProgram.StopUseShader();
		glDepthFunc(GL_LESS);

		glfwSwapBuffers(window);
	}
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
