#include "core.h"
#include "TextureLoader.h"
#include "shader_setup.h"
#include "ArcballCamera.h"
#include "GUClock.h"
#include "AIMesh.h"
#include "Cylinder.h"
#include "freeLookCamera.h"

using namespace std;
using namespace glm;
// this code is for the character matrices so i got him to spawn in the middle of the level.
glm::vec3 characterPosition(0.0f, 10.0f, 0.0f);
bool Jump  = false;

struct DirectionalLight {

	vec3 direction;
	vec3 colour;
	
	DirectionalLight() {

		direction = vec3(1.0f, 0.0f, 1.0f); // default to point upwards
		colour = vec3(1.0f, 1.0f, 1.0f);
	}

	DirectionalLight(vec3 direction, vec3 colour = vec3(1.0f, 1.0f, 1.0f)) {

		this->direction = direction;
		this->colour = colour;
	}
};

struct PointLight {

	vec3 pos;
	vec3 colour;
	vec3 attenuation; // x=constant, y=linear, z=quadratic

	PointLight() {

		pos = vec3(0.0f, 0.0f, 0.0f);
		colour = vec3(1.0f, 1.0f, 1.0f);
		attenuation = vec3(1.0f, 1.0f, 1.0f);
	}

	PointLight(vec3 pos, vec3 colour = vec3(1.0f, 1.0f, 1.0f), vec3 attenuation = vec3(1.0f, 1.0f, 1.0f)) {

		this->pos = pos;
		this->colour = colour;
		this->attenuation = attenuation;
	}
};


#pragma region Global variables

// Window size
unsigned int		windowWidth = 1024;
unsigned int		windowHeight = 768;

// Main clock for tracking time (for animation / interaction)
GUClock*			gameClock = nullptr;

// Main camera
ArcballCamera*		mainCamera = nullptr;
freeCamera main2;

// Mouse tracking
bool				mouseDown = false;
double				prevMouseX, prevMouseY;

// Keyboard tracking
bool				forwardPressed;
bool				backPressed;
bool				rotateLeftPressed;
bool				rotateRightPressed;


// Scene objects
AIMesh*				groundMesh = nullptr;
AIMesh*				creatureMesh = nullptr;
AIMesh*				columnMesh = nullptr;
Cylinder*			cylinderMesh = nullptr;


// Shaders

// Basic colour shader
GLuint				basicShader;
GLint				basicShader_mvpMatrix;

// Texture-directional light shader
GLuint				texDirLightShader;
GLint				texDirLightShader_modelMatrix;
GLint				texDirLightShader_viewMatrix;
GLint				texDirLightShader_projMatrix;
GLint				texDirLightShader_texture;
GLint				texDirLightShader_lightDirection;
GLint				texDirLightShader_lightColour;

// Texture-point light shader
GLuint				texPointLightShader;
GLint				texPointLightShader_modelMatrix;
GLint				texPointLightShader_viewMatrix;
GLint				texPointLightShader_projMatrix;
GLint				texPointLightShader_texture;
GLint				texPointLightShader_lightPosition;
GLint				texPointLightShader_lightColour;
GLint				texPointLightShader_lightAttenuation;

//  *** normal mapping *** Normal mapped texture with Directional light
// This is the same as the texture direct light shader above, but with the addtional uniform variable
// to set the normal map sampler2D variable in the fragment shader.
GLuint				nMapDirLightShader;
GLint				nMapDirLightShader_modelMatrix;
GLint				nMapDirLightShader_viewMatrix;
GLint				nMapDirLightShader_projMatrix;
GLint				nMapDirLightShader_diffuseTexture;
GLint				nMapDirLightShader_normalMapTexture;
GLint				nMapDirLightShader_lightDirection;
GLint				nMapDirLightShader_lightColour;

// cylinder model
vec3 cylinderPos = vec3(-2.0f, 2.0f, 0.0f);

// beast model
vec3 beastPos = vec3(2.0f, 0.0f, 0.0f);
float beastRotation = 0.0f;


// Directional light example (declared as a single instance)
float directLightTheta = 0.0f;
DirectionalLight directLight = DirectionalLight(vec3(cosf(directLightTheta), sinf(directLightTheta), 0.0f));

// Setup point light example light (use array to make adding other lights easier later)
PointLight lights[1] = {
	PointLight(vec3(0.0f, 1.0f, 0.0), vec3(1.0f, 0.0f, 0.0f), vec3(1.0f, 0.1f, 0.001f))
};

bool rotateDirectionalLight = true;


// House single / multi-mesh example
vector<AIMesh*> houseModel = vector<AIMesh*>();

// Hut single / multi-mesh example
vector<AIMesh*> hutModel = vector<AIMesh*>();

// castleflag single / multi-mesh example
vector<AIMesh*> CastleflagModel = vector<AIMesh*>();

// mosalium single / multi-mesh example
vector<AIMesh*> mosaliumModel = vector<AIMesh*>();
#pragma endregion

// itza2 single / multi-mesh example
vector<AIMesh*> itza2Model = vector<AIMesh*>();
#pragma endregion
// itza2 single / multi-mesh example
vector<AIMesh*> manModel = vector<AIMesh*>();
#pragma endregion

// itza2 single / multi-mesh example
vector<AIMesh*> NewcastleModel = vector<AIMesh*>();
#pragma endregion

// itza2 single / multi-mesh example
vector<AIMesh*> terrainModel = vector<AIMesh*>();
#pragma endregion

// multi-mesh example
vector<AIMesh*> Crate = vector<AIMesh*>();
#pragma endregion

// Function prototypes
void renderScene();
void renderHouse();
void renderhut();
void rendermosalium();
void renderCastleflag();
void renderitza2();
void renderman();
void renderNewcastle();
void renderterrain();

void renderWithDirectionalLight();
void renderWithPointLight();
void renderWithMultipleLights();
void updateScene();
void resizeWindow(GLFWwindow* window, int width, int height);
void keyboardHandler(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouseMoveHandler(GLFWwindow* window, double xpos, double ypos);
void mouseButtonHandler(GLFWwindow* window, int button, int action, int mods);
void mouseScrollHandler(GLFWwindow* window, double xoffset, double yoffset);
void mouseEnterHandler(GLFWwindow* window, int entered);



int main() {

	//
	// 1. Initialisation
	//
	
	gameClock = new GUClock();

#pragma region OpenGL and window setup

	// Initialise glfw and setup window
	glfwInit();

	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
	glfwWindowHint(GLFW_OPENGL_COMPAT_PROFILE, GLFW_TRUE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 1);


	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "CIS5013", NULL, NULL);

	// Check window was created successfully
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window!\n";
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


	// Set callback functions to handle different events
	glfwSetFramebufferSizeCallback(window, resizeWindow); // resize window callback
	glfwSetKeyCallback(window, keyboardHandler); // Keyboard input callback
	glfwSetCursorPosCallback(window, mouseMoveHandler);
	glfwSetMouseButtonCallback(window, mouseButtonHandler);
	glfwSetScrollCallback(window, mouseScrollHandler);
	glfwSetCursorEnterCallback(window, mouseEnterHandler);

	// Initialise glew
	glewInit();

	
	// Setup window's initial size
	resizeWindow(window, windowWidth, windowHeight);

#pragma endregion


	// Initialise scene - geometry and shaders etc
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // setup background colour to be black
	glClearDepth(1.0f);

	glPolygonMode(GL_FRONT, GL_FILL);
	glPolygonMode(GL_BACK, GL_LINE);
	
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);
	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);


	//
	// Setup Textures, VBOs and other scene objects
	//
	// Create a new ArcballCamera with a top-down view
mainCamera = new ArcballCamera(0.0f, 90.0f, 40.0f, 55.0f, (float)windowWidth/(float)windowHeight, 0.1f, 5000.0f);
//main2 = freeCamera(glm::vec3(0.0, 30.0f, 0.0f)); // this camera i made for when i was testing i d

	groundMesh = new AIMesh(string("Assets\\ground-surface\\terrain.obj"));
	if (groundMesh) {
		groundMesh->addTexture("Assets\\ground-surface\\texture-new.png", FIF_PNG);
		groundMesh->addNormalMap(string("Assets\\ground-surface\\normal-map.png"), FIF_PNG);
	}

	creatureMesh = new AIMesh(string("Assets\\beast\\beast.obj"));
	if (creatureMesh) {
		creatureMesh->addTexture(string("Assets\\beast\\beast_texture.bmp"), FIF_BMP);
	}
	
	columnMesh = new AIMesh(string("Assets\\column\\Column.obj"));
	if (columnMesh) {
		columnMesh->addTexture(string("Assets\\column\\column_d.bmp"), FIF_BMP);
		columnMesh->addNormalMap(string("Assets\\column\\column_n.bmp"), FIF_BMP);
	}

	cylinderMesh = new Cylinder(string("Assets\\cylinder\\cylinderT.obj"));
	

	// Load shaders
	basicShader = setupShaders(string("Assets\\Shaders\\basic_shader.vert"), string("Assets\\Shaders\\basic_shader.frag"));
	texPointLightShader = setupShaders(string("Assets\\Shaders\\texture-point.vert"), string("Assets\\Shaders\\texture-point.frag"));
	texDirLightShader = setupShaders(string("Assets\\Shaders\\texture-directional.vert"), string("Assets\\Shaders\\texture-directional.frag"));
	nMapDirLightShader = setupShaders(string("Assets\\Shaders\\nmap-directional.vert"), string("Assets\\Shaders\\nmap-directional.frag"));

	// Get uniform variable locations for setting values later during rendering
	basicShader_mvpMatrix = glGetUniformLocation(basicShader, "mvpMatrix");

	texDirLightShader_modelMatrix = glGetUniformLocation(texDirLightShader, "modelMatrix");
	texDirLightShader_viewMatrix = glGetUniformLocation(texDirLightShader, "viewMatrix");
	texDirLightShader_projMatrix = glGetUniformLocation(texDirLightShader, "projMatrix");
	texDirLightShader_texture = glGetUniformLocation(texDirLightShader, "texture");
	texDirLightShader_lightDirection = glGetUniformLocation(texDirLightShader, "lightDirection");
	texDirLightShader_lightColour = glGetUniformLocation(texDirLightShader, "lightColour");

	texPointLightShader_modelMatrix = glGetUniformLocation(texPointLightShader, "modelMatrix");
	texPointLightShader_viewMatrix = glGetUniformLocation(texPointLightShader, "viewMatrix");
	texPointLightShader_projMatrix = glGetUniformLocation(texPointLightShader, "projMatrix");
	texPointLightShader_texture = glGetUniformLocation(texPointLightShader, "texture");
	texPointLightShader_lightPosition = glGetUniformLocation(texPointLightShader, "lightPosition");
	texPointLightShader_lightColour = glGetUniformLocation(texPointLightShader, "lightColour");
	texPointLightShader_lightAttenuation = glGetUniformLocation(texPointLightShader, "lightAttenuation");

	nMapDirLightShader_modelMatrix = glGetUniformLocation(nMapDirLightShader, "modelMatrix");
	nMapDirLightShader_viewMatrix = glGetUniformLocation(nMapDirLightShader, "viewMatrix");
	nMapDirLightShader_projMatrix = glGetUniformLocation(nMapDirLightShader, "projMatrix");
	nMapDirLightShader_diffuseTexture = glGetUniformLocation(nMapDirLightShader, "diffuseTexture");
	nMapDirLightShader_normalMapTexture = glGetUniformLocation(nMapDirLightShader, "normalMapTexture");
	nMapDirLightShader_lightDirection = glGetUniformLocation(nMapDirLightShader, "lightDirection");
	nMapDirLightShader_lightColour = glGetUniformLocation(nMapDirLightShader, "lightColour");


	//
	// House example
	//
	//string houseFilename = string("Assets\\House\\House_Multi.obj");
	//const struct aiScene* houseScene = aiImportFile(houseFilename.c_str(),
		//aiProcess_GenSmoothNormals |
		//aiProcess_CalcTangentSpace |
		//aiProcess_Triangulate |
		//aiProcess_JoinIdenticalVertices |
		//aiProcess_SortByPType);

	//if (houseScene) {

		//cout << "House model: " << houseFilename << " has " << houseScene->mNumMeshes << " meshe(s)\n";

		//if (houseScene->mNumMeshes > 0) {

			// For each sub-mesh, setup a new AIMesh instance in the houseModel array
			//for (int i = 0; i < houseScene->mNumMeshes; i++) {

				//cout << "Loading house sub-mesh " << i << endl;
				//houseModel.push_back(new AIMesh(houseScene, i));
			//}
		//}
	//}

	//
	// hut example
	//
	string hutFilename = string("Assets\\House\\Hut_Multi.obj");
	const struct aiScene* HutScene = aiImportFile(hutFilename.c_str(),
		aiProcess_GenSmoothNormals |
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);

	if (HutScene) {

		cout << "Hut model: " << hutFilename << " has " << HutScene->mNumMeshes << " meshe(s)\n";

		if (HutScene->mNumMeshes > 0) {

			// For each sub-mesh, setup a new AIMesh instance in the houseModel array
			for (int i = 0; i < HutScene->mNumMeshes; i++) {

				cout << "Loading hut sub-mesh " << i << endl;
				hutModel.push_back(new AIMesh(HutScene, i));
				hutModel.back()->addTexture("Assets/Textrues/castle-hut-texture.tif", FIF_TIFF);
				hutModel.back()->addNormalMap("Assets/Textrues/normal-map-hut.tif", FIF_TIFF);
			}
		}
	}

	// castleflag example
	//
	string CastleflagFilename = string("Assets\\House\\castleflag_multi.obj");
	const struct aiScene* CastleflagScene = aiImportFile(CastleflagFilename.c_str(),
		aiProcess_GenSmoothNormals |
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);

	if (CastleflagScene) {

		cout << "Castleflag model: " << CastleflagFilename << " has " << CastleflagScene->mNumMeshes << " meshe(s)\n";

		if (CastleflagScene->mNumMeshes > 0) {

			// For each sub-mesh, setup a new AIMesh instance in the houseModel array
			for (int i = 0; i < CastleflagScene->mNumMeshes; i++) {

				cout << "Loading Castleflag sub-mesh " << i << endl;
				CastleflagModel.push_back(new AIMesh(CastleflagScene, i));
			}

			CastleflagModel[0]->addTexture("Assets/Textrues/catsle.tif", FIF_TIFF);
			CastleflagModel[1]->addTexture("Assets/Textrues/catsle.tif", FIF_TIFF);
			CastleflagModel[2]->addTexture("Assets/Textrues/catsle.tif", FIF_TIFF);
			CastleflagModel[3]->addTexture("Assets/Textrues/catsle.tif", FIF_TIFF);
			CastleflagModel[4]->addTexture("Assets/Textrues/itza_uvmap.tif", FIF_TIFF);
			CastleflagModel[5]->addTexture("Assets/Textrues/flags.tif", FIF_TIFF);

		}
	}

	// mosalium example
//
	string mosaliumFilename = string("Assets\\House\\mosalium_Multi.obj");
	const struct aiScene* mosaliumScene = aiImportFile(mosaliumFilename.c_str(),
		aiProcess_GenSmoothNormals |
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);

	if (mosaliumScene) {

		cout << "mosalium model: " << mosaliumFilename << " has " << mosaliumScene->mNumMeshes << " meshe(s)\n";

		if (mosaliumScene->mNumMeshes > 0) {

			// For each sub-mesh, setup a new AIMesh instance in the houseModel array
			for (int i = 0; i < mosaliumScene->mNumMeshes; i++) {

				cout << "Loading mosalium sub-mesh " << i << endl;
				mosaliumModel.push_back(new AIMesh(mosaliumScene, i));
				mosaliumModel.back()->addTexture("Assets/Textrues/woden_texture.tif", FIF_TIFF);
			}
		}
	}

	// itza2 example
	//
	string itza2Filename = string("Assets\\House\\itza2_Multi.obj");
	const struct aiScene* itza2Scene = aiImportFile(itza2Filename.c_str(),
		aiProcess_GenSmoothNormals |
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);

	if (itza2Scene) {

		cout << "itza2model: " << itza2Filename << " has " << itza2Scene->mNumMeshes << " meshe(s)\n";

		if (itza2Scene->mNumMeshes > 0) {

			// For each sub-mesh, setup a new AIMesh instance in the houseModel array
			for (int i = 0; i < itza2Scene->mNumMeshes; i++) {

				cout << "Loading itza2 sub-mesh " << i << endl;
				itza2Model.push_back(new AIMesh(itza2Scene, i));
				itza2Model.back()->addTexture("Assets/Textrues/itza_uvmap.tif", FIF_TIFF);
			}
		}
	}


	// man example
	//
	string manFilename = string("Assets\\character\\man+amimation_multi.obj");
	const struct aiScene* manScene = aiImportFile(manFilename.c_str(),
		aiProcess_GenSmoothNormals |
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);

	if (manScene) {

		cout << "manmodel: " << manFilename << " has " << manScene->mNumMeshes << " meshe(s)\n";

		if (manScene->mNumMeshes > 0) {

			// For each sub-mesh, setup a new AIMesh instance in the houseModel array
			for (int i = 0; i < manScene->mNumMeshes; i++) {

				cout << "Loading man sub-mesh " << i << endl;
				manModel.push_back(new AIMesh(manScene, i));
				manModel.back()->addTexture("Assets/Textrues/blnk.tif", FIF_TIFF);
			}
		}
	}

	//
	string crateFilename = string("Assets\\Crate\\Crate1.obj");
	const struct aiScene* crateScene = aiImportFile(crateFilename.c_str(),
		aiProcess_GenSmoothNormals |
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);

	if (crateScene) {

		cout << "manmodel: " << crateFilename << " has " << crateScene->mNumMeshes << " meshe(s)\n";

		if (crateScene->mNumMeshes > 0) {

			// For each sub-mesh, setup a new AIMesh instance in the houseModel array
			for (int i = 0; i < crateScene->mNumMeshes; i++) {

				cout << "Loading man sub-mesh " << i << endl;
				Crate.push_back(new AIMesh(crateScene, i));
				Crate.back()->addTexture("Assets/Textrues/woden_texture.tif", FIF_TIFF);
			}
		}
	}



	// New Castle example
	//
	string NewcastleFilename = string("Assets\\House\\small-castle_multi.obj");
	const struct aiScene* NewcastleScene = aiImportFile(NewcastleFilename.c_str(),
		aiProcess_GenSmoothNormals |
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);

	if (NewcastleScene) {

		cout << "Newcastle model: " << NewcastleFilename << " has " << NewcastleScene->mNumMeshes << " meshe(s)\n";

		if (NewcastleScene->mNumMeshes > 0) {

			// For each sub-mesh, setup a new AIMesh instance in the houseModel array
			for (int i = 0; i < 1; i++) {
				// ive made it to push back the texture and normal map i edited what the house code was as i couldnt get the meshes to load in so i decided to change the int to pushback and it worked it adds the textures and normal maps.
				cout << "Loading Castleflag sub-mesh " << i << endl;
				NewcastleModel.push_back(new AIMesh(NewcastleScene, i));
				NewcastleModel.back()->addTexture("Assets/Textrues/itza_uvmap.tif", FIF_TIFF);
				NewcastleModel.back()->addNormalMap("Assets/Textrues/castle-normal.tif", FIF_TIFF);
			}
		}
	}


	
	
	//
	// 2. Main loop
	// 

	while (!glfwWindowShouldClose(window)) {

		updateScene();
		renderScene();						// Render into the current buffer
		glfwSwapBuffers(window);			// Displays what was just rendered (using double buffering).

		glfwPollEvents();					// Use this version when animating as fast as possible
	
		// update window title
		char timingString[256];
		sprintf_s(timingString, 256, "CIS5013: Average fps: %.0f; Average spf: %f", gameClock->averageFPS(), gameClock->averageSPF() / 1000.0f);
		glfwSetWindowTitle(window, timingString);
	}

	glfwTerminate();

	if (gameClock) {

		gameClock->stop();
		gameClock->reportTimingData();
	}

	return 0;
}


// renderScene - function to render the current scene
void renderScene()
{
	//renderHouse();
	//renderhut();
	// renderCastleflag();
	 //rendermosalium();
	 //renderitza2();
	 //renderman();
	 //renderNewcastle();
	// renderterrain();
	 //renderWithDirectionalLight();
	 //renderWithPointLight();
     renderWithMultipleLights();
}


void renderWithMultipleLights() {

	// Clear the rendering window
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.2f, 0.6f, 1.0f, 1.0f);

	// Get camera matrices
	//mat4 cameraProjection = mainCamera->projectionTransform();
	//mat4 cameraView = mainCamera->viewTransform() * translate(identity<mat4>(), -beastPos);

	//glDisable(GL_CULL_FACE);


	mat4 cameraProjection = mainCamera->projectionTransform();
	mat4 cameraView = mainCamera->viewTransform();

#pragma region Render all opaque objects with directional light

	
	glUseProgram(nMapDirLightShader);

	glUniformMatrix4fv(nMapDirLightShader_viewMatrix, 1, GL_FALSE, (GLfloat*)&cameraView);
	glUniformMatrix4fv(nMapDirLightShader_projMatrix, 1, GL_FALSE, (GLfloat*)&cameraProjection);
	glUniform1i(nMapDirLightShader_diffuseTexture, 0); // set to point to texture unit 0 for AIMeshes
	glUniform1i(nMapDirLightShader_normalMapTexture, 1); // set to point to texture unit 0 for AIMeshes
	glUniform3fv(nMapDirLightShader_lightDirection, 1, (GLfloat*)&(directLight.direction));
	glUniform3fv(nMapDirLightShader_lightColour, 1, (GLfloat*)&(directLight.colour));

	if (groundMesh) {

		mat4 modelTransform = glm::scale(identity<mat4>(), vec3(2.0f, 1.0f, 2.0f));

		glUniformMatrix4fv(nMapDirLightShader_modelMatrix, 1, GL_FALSE, (GLfloat*)&modelTransform);

		groundMesh->setupTextures();
		groundMesh->render();
	}

	// Loop through array of meshes and render each one
	for (AIMesh* mesh : hutModel) {
		mat4 modelTransform = glm::translate(glm::mat4(1.0f), glm::vec3(-130.0f, 5.0f, 120.0f)) *
			glm::rotate(glm::mat4(1.0f), radians(45.0f), glm::vec3(0, 1, 0))
			*
			glm::scale(identity<mat4>(), vec3(0.85f));

		glUniformMatrix4fv(nMapDirLightShader_modelMatrix, 1, GL_FALSE, (GLfloat*)&modelTransform);

		glDisable(GL_CULL_FACE);
		mesh->setupTextures();
		mesh->render();
	}

	// Loop through array of meshes and render each one
	for (AIMesh* mesh : NewcastleModel) {
		// model transform matrices so i used this to change the layout 
		mat4 modelTransform = glm::translate(glm::mat4(1.0f), glm::vec3(120.0f, 5.0f, 110.0f)) *
			glm::rotate(glm::mat4(1.0f), radians(90.0f), glm::vec3(0, 1, 0))
			*
			glm::scale(identity<mat4>(), vec3(0.3f));

		glUniformMatrix4fv(nMapDirLightShader_modelMatrix, 1, GL_FALSE, (GLfloat*)&modelTransform);
		mesh->setupTextures();
		mesh->render();
	}


	// Loop through array of meshes and render each one
	for (AIMesh* mesh : CastleflagModel) {
		mat4 modelTransform = glm::translate(glm::mat4(1.0f), glm::vec3(-60.0f, 0.0f, -50.0f)) *
			glm::rotate(glm::mat4(1.0f), radians(-90.0f), glm::vec3(0, 1, 0))
			*
			glm::scale(identity<mat4>(), vec3(0.3f));

		glUniformMatrix4fv(nMapDirLightShader_modelMatrix, 1, GL_FALSE, (GLfloat*)&modelTransform);
		mesh->setupTextures();
		mesh->render();
	}


	glUseProgram(texDirLightShader);

	glUniformMatrix4fv(texDirLightShader_viewMatrix, 1, GL_FALSE, (GLfloat*)&cameraView);
	glUniformMatrix4fv(texDirLightShader_projMatrix, 1, GL_FALSE, (GLfloat*)&cameraProjection);
	glUniform1i(texDirLightShader_texture, 0); // set to point to texture unit 0 for AIMeshes
	glUniform3fv(texDirLightShader_lightDirection, 1, (GLfloat*)&(directLight.direction));
	glUniform3fv(texDirLightShader_lightColour, 1, (GLfloat*)&(directLight.colour));


	// Loop through array of meshes and render each one
	for (AIMesh* mesh : itza2Model) {
		mat4 modelTransform = glm::translate(glm::mat4(1.0f), glm::vec3(120.0f, 5.0f, 0.0f)) *
			glm::rotate(glm::mat4(1.0f), radians(0.0f), glm::vec3(0, 1, 0))
			*
			glm::scale(identity<mat4>(), vec3(0.5f));

		glUniformMatrix4fv(texDirLightShader_modelMatrix, 1, GL_FALSE, (GLfloat*)&modelTransform);

		mesh->setupTextures();
		mesh->render();
	}

	// Loop through array of meshes and render each one
	for (AIMesh* mesh : manModel) {
		mat4 modelTransform = glm::translate(glm::mat4(1.0f), characterPosition) *
			glm::rotate(glm::mat4(1.0f), radians(0.0f), glm::vec3(0, 1, 0))
			*
			glm::scale(identity<mat4>(), vec3(0.07f));

		glUniformMatrix4fv(texDirLightShader_modelMatrix, 1, GL_FALSE, (GLfloat*)&modelTransform);

		mesh->setupTextures();
		mesh->render();
	}

	// Loop through array of meshes and render each one
	for (AIMesh* mesh : mosaliumModel) {
		mat4 modelTransform = glm::translate(glm::mat4(1.0f), glm::vec3(120.0f, 0.0f, -120.0f)) *
			glm::rotate(glm::mat4(1.0f), radians(45.0f), glm::vec3(0, 1, 0))
			*
			glm::scale(identity<mat4>(), vec3(0.33f));

		glUniformMatrix4fv(texDirLightShader_modelMatrix, 1, GL_FALSE, (GLfloat*)&modelTransform);

		mesh->setupTextures();
		mesh->render();
	}

	
#pragma endregion


	// Enable additive blending for ***subsequent*** light sources!!!
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);



#pragma region Render all opaque objects with point light

	glUseProgram(texPointLightShader);

	glUniformMatrix4fv(texPointLightShader_viewMatrix, 1, GL_FALSE, (GLfloat*)&cameraView);
	glUniformMatrix4fv(texPointLightShader_projMatrix, 1, GL_FALSE, (GLfloat*)&cameraProjection);
	glUniform1i(texPointLightShader_texture, 0); // set to point to texture unit 0 for AIMeshes
	glUniform3fv(texPointLightShader_lightPosition, 1, (GLfloat*)&(lights[0].pos));
	glUniform3fv(texPointLightShader_lightColour, 1, (GLfloat*)&(lights[0].colour));
	glUniform3fv(texPointLightShader_lightAttenuation, 1, (GLfloat*)&(lights[0].attenuation));

	if (groundMesh) {

		mat4 modelTransform = glm::scale(identity<mat4>(), vec3(10.0f, 1.0f, 10.0f));

		glUniformMatrix4fv(texPointLightShader_modelMatrix, 1, GL_FALSE, (GLfloat*)&modelTransform);

		groundMesh->setupTextures();
		groundMesh->render();
	}

	

#pragma endregion


#pragma region Render transparant objects

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	glDisable(GL_BLEND);

#pragma endregion


	//
	// For demo purposes, render light sources
	//

	// Restore fixed-function
	glUseProgram(0);
	glBindVertexArray(0);
	glDisable(GL_TEXTURE_2D);

	mat4 cameraT = cameraProjection * cameraView;
	glLoadMatrixf((GLfloat*)&cameraT);
	glEnable(GL_POINT_SMOOTH);
	glPointSize(10.0f);
	
	glBegin(GL_POINTS);

	glColor3f(directLight.colour.r, directLight.colour.g, directLight.colour.b);
	glVertex3f(directLight.direction.x * 10.0f, directLight.direction.y * 10.0f, directLight.direction.z * 10.0f);

	glColor3f(lights[0].colour.r, lights[0].colour.g, lights[0].colour.b);
	glVertex3f(lights[0].pos.x, lights[0].pos.y, lights[0].pos.z);
	
	glEnd();
}

float TS; // this is just a new varible for time delta


// Function called to animate elements in the scene
void updateScene() {

	float tDelta = 0.0f;

	if (gameClock) {

		gameClock->tick();
		tDelta = (float)gameClock->gameTimeDelta();
	}

	cylinderMesh->update(tDelta);

	TS = tDelta;

	// update main light source i changed up some values so i could have the light source always on i tweaked the numbers until i got a value that worked.
	if (rotateDirectionalLight) {

		directLightTheta = glm::radians(70.0f);
		directLight.direction = vec3(cosf(directLightTheta), sinf(directLightTheta), 0.0f);
	}
	

	//
	// Handle movement based on user input i made a completley new jump mechanic when pressed the spacebar and i paused the wasd movement but the code is still there
	//
	if (Jump)
	{
		if (characterPosition.y >= 20.0f)
		{
			Jump = false;
		}

		characterPosition.y += 30.0f * tDelta;
	}
	else
	{
		if (characterPosition.y >= 11.0f)
		{
			characterPosition.y -= 10.0f * tDelta;
		}
		
	}


	float moveSpeed = 3.0f; // movement displacement per second
	float rotateSpeed = 90.0f; // degrees rotation per second

	if (forwardPressed) {

		mat4 R = eulerAngleY<float>(glm::radians<float>(beastRotation)); // local coord space / basis vectors - move along z
		float dPos = moveSpeed * tDelta; // calc movement based on time elapsed
		beastPos += vec3(R[2].x * dPos, R[2].y * dPos, R[2].z * dPos); // add displacement to position vector
	}
	else if (backPressed) {

		mat4 R = eulerAngleY<float>(glm::radians<float>(beastRotation)); // local coord space / basis vectors - move along z
		float dPos = -moveSpeed * tDelta; // calc movement based on time elapsed
		beastPos += vec3(R[2].x * dPos, R[2].y * dPos, R[2].z * dPos); // add displacement to position vector
	}

	if (rotateLeftPressed) {

		beastRotation += rotateSpeed * tDelta;
	}
	else if (rotateRightPressed) {

		beastRotation -= rotateSpeed * tDelta;
	}

}


#pragma region Event handler functions

// Function to call when window resized
void resizeWindow(GLFWwindow* window, int width, int height)
{
	if (mainCamera) {

		mainCamera->setAspect((float)width / (float)height);
	}

	// Update viewport to cover the entire window
	glViewport(0, 0, width, height);

	windowWidth = width;
	windowHeight = height;
}


// Function to call to handle keyboard input / i made a new jump function so when you press space bar you can jump up and down.
void keyboardHandler(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	float speed = 100.0f;

	if (action == GLFW_PRESS || action == GLFW_REPEAT) {

		// check which key was pressed...
		switch (key)
		{
			case GLFW_KEY_ESCAPE:
				glfwSetWindowShouldClose(window, true);
				break;
			
			case GLFW_KEY_W:
				forwardPressed = true;
				main2.ProcessKeyboard(Camera_Movement::FORWARD, TS * speed);
				break;

			case GLFW_KEY_S:
				backPressed = true;
				main2.ProcessKeyboard(Camera_Movement::BACKWARD, TS * speed);
				break;

			case GLFW_KEY_A:
				rotateLeftPressed = true;
				main2.ProcessKeyboard(Camera_Movement::LEFT, TS * speed);
				break;

			case GLFW_KEY_D:
				rotateRightPressed = true;
				main2.ProcessKeyboard(Camera_Movement::RIGHT, TS * speed);
				break;

			case GLFW_KEY_SPACE:
				//rotateDirectionalLight = !rotateDirectionalLight;
				Jump = true;
				break;

			default:
			{
			}
		}
	}
	else if (action == GLFW_RELEASE) {
		// handle key release events
		switch (key)
		{
			case GLFW_KEY_W:
				forwardPressed = false;
				break;

			case GLFW_KEY_S:
				backPressed = false;
				break;

			case GLFW_KEY_A:
				rotateLeftPressed = false;
				break;

			case GLFW_KEY_D:
				rotateRightPressed = false;
				break;

			default:
			{
			}
		}
	}
}


void mouseMoveHandler(GLFWwindow* window, double xpos, double ypos) {

	if (mouseDown) {

		float dx = float(xpos - prevMouseX);
		float dy = float(ypos - prevMouseY);

		if (mainCamera)
			mainCamera->rotateCamera(-dy, -dx);
		// this doesnt matter only if i use main2 camera but this is from testing i changed in the end.
		main2.ProcessMouseMovement(dx, -dy);

		prevMouseX = xpos;
		prevMouseY = ypos;
	}

}

void mouseButtonHandler(GLFWwindow* window, int button, int action, int mods) {

	if (button == GLFW_MOUSE_BUTTON_LEFT) {

		if (action == GLFW_PRESS) {

			mouseDown = true;
			glfwGetCursorPos(window, &prevMouseX, &prevMouseY);
		}
		else if (action == GLFW_RELEASE) {

			mouseDown = false;
		}
	}
}

void mouseScrollHandler(GLFWwindow* window, double xoffset, double yoffset) {

	if (mainCamera) {

		if (yoffset < 0.0)
			mainCamera->scaleRadius(1.1f);
		else if (yoffset > 0.0)
			mainCamera->scaleRadius(0.9f);
	}
}

void mouseEnterHandler(GLFWwindow* window, int entered) {
}

#pragma endregion