// Windows includes (For Time, IO, etc.)
#include <windows.h>
#include <mmsystem.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <math.h>
#include <vector> // STL dynamic memory.

// OpenGL includes
#include <GL/glew.h>
#include <GL/freeglut.h>

// Assimp includes
#include <assimp/cimport.h> // scene importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations

// Project includes
#include "maths_funcs.h"

// GLM includes
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>

// Anttweak Bar
#include <AntTweakBar.h>

// GLFW includes
#include <GLFW\glfw3.h>

// Eigen
#include <Eigen/Dense>
#include <Eigen/Sparse>

// Loader Class
#include "MeshLoader.h"

using namespace std;
/*----------------------------------------------------------------------------
LOAD MESHES
----------------------------------------------------------------------------*/
MeshLoader neutralObject("./models/ball.obj");
//MeshLoader jawOpenObject("./models/Mery_jaw_open.obj");
//MeshLoader kissObject("./models/Mery_kiss.obj");
//MeshLoader rightBrowLowerObject("./models/Mery_r_brow_lower.obj");
//MeshLoader leftBrowLowerObject("./models/Mery_l_brow_lower.obj");
//MeshLoader rightBrowNarrowObject("./models/Mery_r_brow_narrow.obj");
//MeshLoader leftBrowNarrowObject("./models/Mery_l_brow_narrow.obj");
//MeshLoader rightBrowRaiseObject("./models/Mery_r_brow_raise.obj");
//MeshLoader leftBrowRaiseObject("./models/Mery_l_brow_raise.obj");
//MeshLoader rightEyeClosedObject("./models/Mery_r_eye_closed.obj");
//MeshLoader leftEyeClosedObject("./models/Mery_l_eye_closed.obj");
//MeshLoader rightEyeLowerOpenObject("./models/Mery_r_eye_lower_open.obj");
//MeshLoader leftEyeLowerOpenObject("./models/Mery_l_eye_lower_open.obj");
//MeshLoader rightEyeUpperOpenObject("./models/Mery_r_eye_upper_open.obj");
//MeshLoader leftEyeUpperOpenObject("./models/Mery_l_eye_upper_open.obj");
//MeshLoader rightNoseWrinkleObject("./models/Mery_r_nose_wrinkle.obj");
//MeshLoader leftNoseWrinkleObject("./models/Mery_l_nose_wrinkle.obj");
//MeshLoader rightSuckObject("./models/Mery_r_suck.obj");
//MeshLoader leftSuckObject("./models/Mery_l_suck.obj");
//MeshLoader rightPuffObject("./models/Mery_r_puff.obj");
//MeshLoader leftPuffObject("./models/Mery_l_puff.obj");
//MeshLoader rightSadObject("./models/Mery_r_sad.obj");
//MeshLoader leftSadObject("./models/Mery_l_sad.obj");
//MeshLoader rightSmileObject("./models/Mery_r_smile.obj");
//MeshLoader leftSmileObject("./models/Mery_l_smile.obj");
//MeshLoader ballObject("./models/ball.obj");

/*----------------------------------------------------------------------------
VIEW SETUP
----------------------------------------------------------------------------*/
GLFWwindow* window;
glm::vec3 cameraPosition = glm::vec3(0.0f, 17.0f, 50.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 startingCameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cameraRight = glm::vec3(0.0f);
glm::vec3 lastCameraPos;
glm::vec3 lastCameraFront;
glm::vec3 lastCameraUp;
glm::vec3 selectedVertex;

/*----------------------------------------------------------------------------
VAO/VBO SETUP
----------------------------------------------------------------------------*/
unsigned int neutralObjectVBO1, neutralObjectVBO2, neutralObjectVAO, ballVBO1, ballVBO2, ballVAO;

/*----------------------------------------------------------------------------
VIEW AND PROJ SETUP
----------------------------------------------------------------------------*/
glm::mat4 modelView;
glm::mat4 projection;

/*----------------------------------------------------------------------------
EQUATION PARAMETERS
----------------------------------------------------------------------------*/
int k = 24;
Eigen::VectorXf f0 = Eigen::VectorXf::Zero(neutralObject.numVertices * 3, 1); // F0
Eigen::MatrixXf B = Eigen::MatrixXf::Zero(neutralObject.numVertices * 3, k); // B
Eigen::VectorXf w = Eigen::VectorXf::Zero(k, 1); // w
Eigen::VectorXf f = Eigen::VectorXf::Zero(neutralObject.numVertices * 3, 1); // F

/*----------------------------------------------------------------------------
OTHER DATA STRUCTURES
----------------------------------------------------------------------------*/
std::vector<MeshLoader> dataArray;
std::vector<GLuint> constraints;
Eigen::VectorXf m0 = Eigen::VectorXf::Zero(constraints.size() * 3, 1); // m0
Eigen::VectorXf m = Eigen::VectorXf::Zero(constraints.size() * 3, 1); // m

/*----------------------------------------------------------------------------
ANIMATION PLAYBACK
----------------------------------------------------------------------------*/
Eigen::VectorXf weights = Eigen::VectorXf::Zero(k, 1); // w

/*----------------------------------------------------------------------------
SETUP
----------------------------------------------------------------------------*/
GLuint shaderProgramID;

//ModelData mesh_data;
unsigned int mesh_vao = 0;
int width = 1400;
int height = 800;

GLuint loc1, loc2, loc3;
GLfloat rotate_y = 0.0f;

// Shader Functions- click on + to expand
#pragma region SHADER_FUNCTIONS
char* readShaderSource(const char* shaderFile) {
	FILE* fp;
	fopen_s(&fp, shaderFile, "rb");

	if (fp == NULL) { return NULL; }

	fseek(fp, 0L, SEEK_END);
	long size = ftell(fp);

	fseek(fp, 0L, SEEK_SET);
	char* buf = new char[size + 1];
	fread(buf, 1, size, fp);
	buf[size] = '\0';

	fclose(fp);

	return buf;
}

static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	// create a shader object
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		std::cerr << "Error creating shader..." << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	const char* pShaderSource = readShaderSource(pShaderText);

	// Bind the source code to the shader, this happens before compilation
	glShaderSource(ShaderObj, 1, (const GLchar**)&pShaderSource, NULL);
	// compile the shader and check for errors
	glCompileShader(ShaderObj);
	GLint success;
	// check for shader related errors using glGetShaderiv
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024] = { '\0' };
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		std::cerr << "Error compiling "
			<< (ShaderType == GL_VERTEX_SHADER ? "vertex" : "fragment")
			<< " shader program: " << InfoLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	// Attach the compiled shader object to the program object
	glAttachShader(ShaderProgram, ShaderObj);
}

GLuint CompileShaders(const char* vertexShader, const char* fragmentShader)
{
	//Start the process of setting up our shaders by creating a program ID
	//Note: we will link all the shaders together into this ID
	GLuint shaderProgramID = glCreateProgram();
	if (shaderProgramID == 0) {
		std::cerr << "Error creating shader program..." << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}

	// Create two shader objects, one for the vertex, and one for the fragment shader
	AddShader(shaderProgramID, vertexShader, GL_VERTEX_SHADER);
	AddShader(shaderProgramID, fragmentShader, GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { '\0' };
	// After compiling all shader objects and attaching them to the program, we can finally link it
	glLinkProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		std::cerr << "Error linking shader program: " << ErrorLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}

	// program has been successfully linked but needs to be validated to check whether the program can execute given the current pipeline state
	glValidateProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		std::cerr << "Invalid shader program: " << ErrorLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	// Finally, use the linked shader program
	// Note: this program will stay in effect for all draw calls until you replace it with another or explicitly disable its use
	glUseProgram(shaderProgramID);
	return shaderProgramID;
}
#pragma endregion SHADER_FUNCTIONS

#pragma region VBO_FUNCTIONS
void generateObjectBufferMesh(Eigen::VectorXf f) {
	for (int i = 0; i < neutralObject.meshVertices.size(); i++) {
		neutralObject.meshVertices[i] = f(i);
	}

	// Model
	loc1 = glGetAttribLocation(shaderProgramID, "vertex_position");
	loc2 = glGetAttribLocation(shaderProgramID, "vertex_normal");
	loc3 = glGetAttribLocation(shaderProgramID, "vertex_texture");

	glGenBuffers(1, &neutralObjectVBO1);
	glBindBuffer(GL_ARRAY_BUFFER, neutralObjectVBO1);
	glBufferData(GL_ARRAY_BUFFER, 3 * neutralObject.numVertices * sizeof(float), &neutralObject.meshVertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &neutralObjectVBO2);
	glBindBuffer(GL_ARRAY_BUFFER, neutralObjectVBO2);
	glBufferData(GL_ARRAY_BUFFER, 3 * neutralObject.numVertices * sizeof(float), &neutralObject.meshNormals[0], GL_STATIC_DRAW);

	glGenVertexArrays(1, &neutralObjectVAO);
	glBindVertexArray(neutralObjectVAO);

	glEnableVertexAttribArray(loc1);
	glBindBuffer(GL_ARRAY_BUFFER, neutralObjectVBO1);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(loc2);
	glBindBuffer(GL_ARRAY_BUFFER, neutralObjectVBO2);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	// Ball
	/*glGenBuffers(1, &ballVBO1);
	glBindBuffer(GL_ARRAY_BUFFER, ballVBO1);
	glBufferData(GL_ARRAY_BUFFER, ballObject.numVertices * sizeof(vec3), &ballObject.meshVertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &ballVBO2);
	glBindBuffer(GL_ARRAY_BUFFER, ballVBO2);
	glBufferData(GL_ARRAY_BUFFER, ballObject.numVertices * sizeof(vec3), &ballObject.meshNormals[0], GL_STATIC_DRAW);

	glGenVertexArrays(1, &ballVAO);
	glBindVertexArray(ballVAO);

	glEnableVertexAttribArray(loc1);
	glBindBuffer(GL_ARRAY_BUFFER, ballVBO1);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(loc2);
	glBindBuffer(GL_ARRAY_BUFFER, ballVBO2);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);*/
}
#pragma endregion VBO_FUNCTIONS

#pragma region BLENDSHAPE_SETUP
void makeDataArray() {
	/*dataArray.push_back(jawOpenObject);
	dataArray.push_back(kissObject);
	dataArray.push_back(rightBrowLowerObject);
	dataArray.push_back(leftBrowLowerObject);
	dataArray.push_back(rightBrowNarrowObject);
	dataArray.push_back(leftBrowNarrowObject);
	dataArray.push_back(rightBrowRaiseObject);
	dataArray.push_back(leftBrowRaiseObject);
	dataArray.push_back(rightEyeClosedObject);
	dataArray.push_back(leftEyeClosedObject);
	dataArray.push_back(rightEyeLowerOpenObject);
	dataArray.push_back(leftEyeLowerOpenObject);
	dataArray.push_back(rightEyeUpperOpenObject);
	dataArray.push_back(leftEyeUpperOpenObject);
	dataArray.push_back(rightNoseWrinkleObject);
	dataArray.push_back(leftNoseWrinkleObject);
	dataArray.push_back(rightSuckObject);
	dataArray.push_back(leftSuckObject);
	dataArray.push_back(rightPuffObject);
	dataArray.push_back(leftPuffObject);
	dataArray.push_back(rightSadObject);
	dataArray.push_back(leftSadObject);
	dataArray.push_back(rightSmileObject);
	dataArray.push_back(leftSmileObject);*/
}

void createF0Matrix() {
	for (int i = 0; i < neutralObject.meshVertices.size(); i++) {
		f0(i) = neutralObject.meshVertices[i];
	}
}

void createBMatrix() {
	for (int n = 0; n < dataArray.size(); n++) {
		for (int i = 0; i < dataArray[n].meshVertices.size(); i++) {
			B(i, n) = dataArray[n].meshVertices[i] - f0[i];
		}
	}
}
#pragma endregion BLENDSHAPE_SETUP

void display() {

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shaderProgramID);

	lastCameraPos = cameraPosition;
	lastCameraFront = cameraFront;
	lastCameraUp = cameraUp;
	glm::mat4 view = glm::mat4(1.0f);

	view = glm::lookAt(lastCameraPos, lastCameraPos + lastCameraFront, lastCameraUp);
	projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 1000.0f);

	// Model
	glm::vec3 modelColor = glm::vec3(0.0f, 0.8f, 0.0f);
	glBindVertexArray(neutralObjectVAO);
	glm::mat4 model = glm::mat4(1.0);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));

	modelView = model * view;

	glUniformMatrix4fv(glGetUniformLocation(shaderProgramID, "proj"), 1, GL_FALSE, glm::value_ptr(projection));
	glUniformMatrix4fv(glGetUniformLocation(shaderProgramID, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(shaderProgramID, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniform3fv(glGetUniformLocation(shaderProgramID, "Kd"), 1, &modelColor[0]);
	glDrawArrays(GL_TRIANGLES, 0, neutralObject.numVertices);

	//// Ball
	//for (int i = 0; i < constraints.size(); i++) {
	//	glm::vec3 ballPos(neutralObject.meshVertices[constraints[i]], neutralObject.meshVertices[constraints[i] + 1], neutralObject.meshVertices[constraints[i] + 2]);
	//	glm::vec3 ballColor = glm::vec3(1.0f, 0.0f, 0.0f);
	//	glBindVertexArray(ballVAO);
	//	glm::mat4 ball = glm::mat4(1.0);
	//	ball = glm::translate(ball, ballPos);

	//	glUniformMatrix4fv(glGetUniformLocation(shaderProgramID, "model"), 1, GL_FALSE, glm::value_ptr(ball));
	//	glUniform3fv(glGetUniformLocation(shaderProgramID, "Kd"), 1, &ballColor[0]);
	//	glDrawArrays(GL_TRIANGLES, 0, ballObject.numVertices);
	//}
}

void updateScene() {

	static DWORD last_time = 0;
	DWORD curr_time = timeGetTime();
	if (last_time == 0)
		last_time = curr_time;
	float delta = (curr_time - last_time) * 0.001f;
	last_time = curr_time;

	// Rotate the model slowly around the y axis at 20 degrees per second
	rotate_y += 20.0f * delta;
	rotate_y = fmodf(rotate_y, 360.0f);

	// Draw the next frame
	glutPostRedisplay();
}

void animation() {
	std::ifstream animationPlayback("blendshape_animation.txt", std::ifstream::in);
	std::string line;
	float value;
	while (std::getline(animationPlayback, line))
	{
		Eigen::VectorXf weight;
		for (int i = 0; i < 24; i++)
		{
			std::stringstream newLine(line);
			newLine >> value >> std::ws;
			weight(i) = value;
		}
		f = f0 + (B * weight);
		generateObjectBufferMesh(f);
	}
}

void vertexPickerM0() {

	double xWindow, yWindow;
	glfwGetCursorPos(window, &xWindow, &yWindow);
	std::cout << "Cursor Position: " << xWindow << " , " << yWindow << std::endl;

	GLfloat x, y, z;
	x = xWindow;
	y = height - yWindow - 1;
	glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &z);
	std::cout << "Cursor Position: " << x << " , " << y << std::endl;

	selectedVertex = glm::unProject(glm::vec3(x, y, z), modelView, projection, glm::vec4(0.0f, 0.0f, width, height));
	std::cout << "Values: " << selectedVertex.x << " " << selectedVertex.y << " " << selectedVertex.z << std::endl;

	GLfloat dist = 10;
	GLfloat temp = 0.0f;
	GLuint m_index = 0;
	GLuint v_index = 0;

	
	for (int i = 0; i < neutralObject.meshVertices.size(); i += 3) {
		glm::vec3 pos(neutralObject.meshVertices[i], neutralObject.meshVertices[i + 1], neutralObject.meshVertices[i + 2]);
		temp = glm::distance(selectedVertex, pos);

		if (temp <= dist) {
			dist = temp;
			v_index = i;
		}
	}

	glm::vec3 v(neutralObject.meshVertices[v_index], neutralObject.meshVertices[v_index + 1], neutralObject.meshVertices[v_index] + 2);
	constraints.push_back(v_index);
	m0.conservativeResize(constraints.size() * 3, 1);
	m0(3 * constraints.size() - 3) = v.x;
	m0(3 * constraints.size() - 2) = v.y;
	m0(3 * constraints.size() - 1) = v.z;
}

void directManipulationMethod() {
	unsigned int  num_rows = neutralObject.meshVertices.size();
	unsigned int  num_cols = dataArray.size();

	Eigen::MatrixXf A;

	// Set up LHS
	Eigen::MatrixXf B(num_rows, num_cols); //the delta-blendshape matrix + additional weight normalization

	// Add Neutral
	for (int i = 0; i < neutralObject.meshVertices.size(); i++) {
		B(i, 0) = neutralObject.meshVertices[i];
	}

	// Add other meshes 
	for (int n = 1; n <= dataArray.size(); n++) {
		for (int j = 0; j < dataArray[n].meshVertices.size(); j++) {
			B(j, n) = dataArray[n].meshVertices[j];
		}
	}
	Eigen::VectorXf wt;
	wt = w;

	// Set up RHS
	Eigen::VectorXf b(num_rows); // rows of B corresponding to constraints
	for (int k = 0, l = 0; k < num_rows, l < B.rows(); k++, l++) {
		b(k) = B(k) - m0(l);
	}

	// Set up LHS of equation (A) and RHS of equation (B)
	A = (B.transpose() * B) + (0.01 + 0.001) * Eigen::MatrixXf::Identity(num_rows, num_cols);
	b = B.transpose() * b + (0.01 * wt);

	// Taken from example
	Eigen::LDLT<Eigen::MatrixXf> solver(A);
	wt = solver.solve(b);

	// Transfer weights back to w vector.
	w = wt;
}

void init()
{
	makeDataArray();
	createF0Matrix(); // Create F0 Vector
	createBMatrix(); // Create B Matrix
	shaderProgramID = CompileShaders("./shaders/simpleVertexShader.txt", "./shaders/simpleFragmentShader.txt");
	generateObjectBufferMesh(f0);
}

void window_size_callback(GLFWwindow* window, int width, int height)
{
	// Set OpenGL viewport and camera
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(40, (double)width / height, 1, 10);
	gluLookAt(-1, 0, 3, 0, 0, 0, 0, 1, 0);

	// Send the new window size to AntTweakBar
	TwWindowSize(width, height);
}

void TW_CALL setCallback(const void *value, void *clientData)
{
	*(float*)clientData = *(const float*)value;
	f = f0 + (B * w);
	generateObjectBufferMesh(f);
}

void TW_CALL getCallback(void *value, void *clientData)
{
	*(float*)value = *(float*)clientData;
}

inline void TwEventMouseButtonGLFW3(GLFWwindow* window, int button, int action, int mods) { 
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		vertexPickerM0();
	} 
	TwEventMouseButtonGLFW(button, action); 
}

inline void TwEventMousePosGLFW3(GLFWwindow* window, double xpos, double ypos)
{
	TwMouseMotion(int(xpos), int(ypos));
}

inline void TwEventKeyGLFW3(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		switch (key) {
		case(GLFW_KEY_A):
			animation();
			break;
		}
	}
	TwEventKeyGLFW(key, action);
}

int main(int argc, char** argv) {
	if (!glfwInit())
	{
		fprintf(stderr, "GLFW initialization failed\n");
		return 1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);	
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	/*TwBar *bar;*/
	window = glfwCreateWindow(width, height, "Real-Time Animation Assignment 3", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return 1;
	}
	glfwMakeContextCurrent(window);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	// Initialize AntTweakBar
	//TwInit(TW_OPENGL_CORE, NULL);

	//// Create a tweak bar
	//bar = TwNewBar("TweakBar");
	//TwWindowSize(width, height);

	//TwDefine(" GLOBAL help='This example shows how to integrate AntTweakBar with GLFW and OpenGL.' "); // Message added to the help bar.
	//// Modifiers
	//TwAddVarRO(bar, "WorldCoords1", TW_TYPE_FLOAT, &selectedVertex[0], " label='X' ");
	//TwAddVarRO(bar, "WorldCoords2", TW_TYPE_FLOAT, &selectedVertex[1], " label='Y' ");
	//TwAddVarRO(bar, "WorldCoords3", TW_TYPE_FLOAT, &selectedVertex[2], " label='Z' ");

	//// Jaw Open (Index: 0)
	//TwAddVarCB(bar, "Jaw Open", TW_TYPE_FLOAT, setCallback, getCallback, &w[0], "min=0 max=1 step = 0.1 label='Jaw Open'");

	//// Kiss (Index: 1)
	//TwAddVarCB(bar, "Kiss", TW_TYPE_FLOAT, setCallback, getCallback, &w[1], "min=0 max=1 step = 0.1 label='Kiss'");

	//// Brow Lower (Index: 2-3)
	//TwAddVarCB(bar, "Brow Lower (Right)", TW_TYPE_FLOAT, setCallback, getCallback, &w[2], "min=0 max=1 step = 0.1 label='Brow Lower (Right)'");
	//TwAddVarCB(bar, "Brow Lower (Left)", TW_TYPE_FLOAT, setCallback, getCallback, &w[3], "min=0 max=1 step = 0.1 label='Brow Lower (Left)'");

	//// Brow Narrow (Index: 4-5)
	//TwAddVarCB(bar, "Brow Narrow (Right)", TW_TYPE_FLOAT, setCallback, getCallback, &w[4], "min=0 max=1 step = 0.1 label='Brow Narrow (Right)'");
	//TwAddVarCB(bar, "Brow Narrow (Left)", TW_TYPE_FLOAT, setCallback, getCallback, &w[5], "min=0 max=1 step = 0.1 label='Brow Narrow (Left)'");

	//// Brow Raise (Index: 6-7)
	//TwAddVarCB(bar, "Brow Raise (Right)", TW_TYPE_FLOAT, setCallback, getCallback, &w[6], "min=0 max=1 step = 0.1 label='Brow Raise (Right)'");
	//TwAddVarCB(bar, "Brow Raise (Left)", TW_TYPE_FLOAT, setCallback, getCallback, &w[7], "min=0 max=1 step = 0.1 label='Brow Raise (Left)'");

	//// Eye Closed (Index: 8-9)
	//TwAddVarCB(bar, "Eye Closed (Right)", TW_TYPE_FLOAT, setCallback, getCallback, &w[8], "min=0 max=1 step = 0.1 label='Eye Closed (Right)'");
	//TwAddVarCB(bar, "Eye Closed (Left)", TW_TYPE_FLOAT, setCallback, getCallback, &w[9], "min=0 max=1 step = 0.1 label='Eye Closed (Left)'");

	//// Eye Lower Open (Index 10-11)
	//TwAddVarCB(bar, "Eye Lower Open (Right)", TW_TYPE_FLOAT, setCallback, getCallback, &w[10], "min=0 max=1 step = 0.1 label='Eye Lower Open (Right)'");
	//TwAddVarCB(bar, "Eye Lower Open (Left)", TW_TYPE_FLOAT, setCallback, getCallback, &w[11], "min=0 max=1 step = 0.1 label='Eye Lower Open (Left)'");

	//// Eye Upper Open (Index 12-13)
	//TwAddVarCB(bar, "Eye Upper Open (Right)", TW_TYPE_FLOAT, setCallback, getCallback, &w[12], "min=0 max=1 step = 0.1 label='Eye Upper Open (Right)'");
	//TwAddVarCB(bar, "Eye Upper Open (Left)", TW_TYPE_FLOAT, setCallback, getCallback, &w[13], "min=0 max=1 step = 0.1 label='Eye Upper Open (Left)'");

	//// Nose Wrinkle (Index 14-15)
	//TwAddVarCB(bar, "Nose Wrinkle (Right)", TW_TYPE_FLOAT, setCallback, getCallback, &w[14], "min=0 max=1 step = 0.1 label='Nose Wrinkle (Right)'");
	//TwAddVarCB(bar, "Nose Wrinkle (Left)", TW_TYPE_FLOAT, setCallback, getCallback, &w[15], "min=0 max=1 step = 0.1 label='Nose Wrinkle (Left)'");

	//// Suck (Index 16-17)
	//TwAddVarCB(bar, "Suck (Right)", TW_TYPE_FLOAT, setCallback, getCallback, &w[16], "min=0 max=1 step = 0.1 label='Suck (Right)'");
	//TwAddVarCB(bar, "Suck (Left)", TW_TYPE_FLOAT, setCallback, getCallback, &w[17], "min=0 max=1 step = 0.1 label='Suck (Left)'");

	//// Puff (Index 18-19)
	//TwAddVarCB(bar, "Puff (Right)", TW_TYPE_FLOAT, setCallback, getCallback, &w[18], "min=0 max=1 step = 0.1 label='Puff (Right)'");
	//TwAddVarCB(bar, "Puff (Left)", TW_TYPE_FLOAT, setCallback, getCallback, &w[19], "min=0 max=1 step = 0.1 label='Puff (Left)'");

	//// Sad (Index 20-21)
	//TwAddVarCB(bar, "Sad (Right)", TW_TYPE_FLOAT, setCallback, getCallback, &w[20], "min=0 max=1 step = 0.1 label='Sad (Right)'");
	//TwAddVarCB(bar, "Sad (Left)", TW_TYPE_FLOAT, setCallback, getCallback, &w[21], "min=0 max=1 step = 0.1 label='Sad (Left)'");

	//// Smile (Index 22-23)
	//TwAddVarCB(bar, "Smile (Right)", TW_TYPE_FLOAT, setCallback, getCallback, &w[22], "min=0 max=1 step = 0.1 label='Smile (Right)'");
	//TwAddVarCB(bar, "Smile (Left)", TW_TYPE_FLOAT, setCallback, getCallback, &w[23], "min=0 max=1 step = 0.1 label='Smile (Left)'");

	glfwSetWindowSizeCallback(window, window_size_callback);
	glfwSetMouseButtonCallback(window, (GLFWmousebuttonfun)TwEventMouseButtonGLFW3);
	glfwSetCursorPosCallback(window, (GLFWcursorposfun)TwEventMousePosGLFW3);
	glfwSetKeyCallback(window, (GLFWkeyfun)TwEventKeyGLFW3);
	glfwSetScrollCallback(window, (GLFWscrollfun)TwEventMouseWheelGLFW);
	glfwSetCharCallback(window, (GLFWcharfun)TwEventCharGLFW);

	glewExperimental = GL_TRUE;
	GLenum glewErr = glewInit();

	if (glewErr != GLEW_OK) {
		return 1;
	}

	init();

	while (!glfwWindowShouldClose(window))
	{
		glfwWaitEvents();
		display();
		//TwDraw();
		glfwSwapBuffers(window);
	}

	// Terminate AntTweakBar and GLFW
	//TwTerminate();
	glfwTerminate();

	return 0;
}
