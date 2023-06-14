/*
* Laboratorio de Computación Gráfica e Interacción Humano-Computadora
* Versión repositorio
* 
*/

#include <iostream>
#include <stdlib.h>

// GLAD: Multi-Language GL/GLES/EGL/GLX/WGL Loader-Generator
// https://glad.dav1d.de/
#include <glad/glad.h>

// GLFW: https://www.glfw.org/
#include <GLFW/glfw3.h>

// GLM: OpenGL Math library
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Model loading classes
#include <shader_m.h>
#include <camera.h>
#include <model.h>
#include <material.h>
#include <light.h>
#include <cubemap.h>

#include <irrKlang.h>
using namespace irrklang;

// Max number of bones
#define MAX_RIGGING_BONES 100

// Functions
bool Start();
bool Update();

// Definición de callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// Gobals
GLFWwindow* window;

// Tamaño en pixeles de la ventana
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// Definición de las cámaras (posición en XYZ)
Camera camera(glm::vec3(0.0f, 5.0f, 10.0f)); // Es nuestro punto de partida
Camera camera3rd(glm::vec3(0.0f, 10.0f, 5.0f)); // Acomodar en posición del doc

// Controladores para el movimiento del mouse
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Variables para la velocidad de reproducción
// de la animación
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float elapsedTime = 0.0f;

float sineTime = 0.0f;

glm::vec3 position(0.0f, 0.0f, 0.0f);
glm::vec3 forwardView(0.0f, 0.0f, 1.0f);
float     scaleV = 0.005f;
float     rotateCharacter = 0.0f; // variables de los personajes
bool	  isWalking = false;
//float	  door_offset = 0.0f; // variables de las puertas
float	  puerta_rotation = 0.0f;
float	  organos_rotation = 0.0f; // variables de los órganos
float	  torrance_position = 0.0f;
float	  models_scale = 0.0f;

// Shaders (Referenciar para cada objeto uno diferente si es que se van a realizar animaciones)
Shader *doctorCaminandoShader;
Shader *doctorParadoShader;
Shader *cubemapShader;
Shader *mLightsShader;
Shader *proceduralShader;
Shader *wavesShader;
Shader *staticShader;
Shader *fresnelShader;
Shader *puertaShader;
Shader *metalPhongShader;
Shader *cartelesShader;

// Carga la información del modelo (poner referencias de los modelos a utilizar)
Model	*doctorCaminando;
Model	*doctorParado;
Model	*hospital;
Model   *puerta;
Model   *escritorio;
Model   *floorObject;
//Model	*Pancreas;
Model	*DigestiveSystem;
Model	*Glass;
Model	*MetalBase;
Model	*Red;
Model	*Vein;
Model	*CubeGlass;
Model	*MetalCube;
Model	*Blood;
Model	*Sugar;
Model	*InsulinKey;
Model   *Carteles;


// Variables globales (que van a variar durante la ejecución del ciclo de renderizado)
float tradius = 10.0f;
float theta = 0.0f;
float alpha = 0.0f;

// Cubemap (equivalente a su skybox (?))
CubeMap *mainCubeMap;

// Materiales 
/*(aquí podemos agregar los necesarios para darle a los objetos acabados metálicos, de madera, transparentes, etc)
Sugerencia: Material <tipoDeMaterial>
*/
Material material01;
Material metal02;

// Creación de luces con Light gLight;
std::vector<Light> gLights;

// Movimiento dinámico de luces (Experimento 3):
glm::vec3 lPosition = glm::vec3(0.0f); //automáticamente crea el vector lleno del valor

// Pose inicial del modelo de doctor y paciente
glm::mat4 gBones[MAX_RIGGING_BONES];
glm::mat4 gBonesBar[MAX_RIGGING_BONES];

// Variables de los modelos animados
float	fpsCaminando = 0.0f;
int		keysCaminando = 0;
int		animationCountCaminando = 0;

float	fpsParado = 0.0f;
int		keysParado = 0;
int		animationCountParado = 0;

float proceduralTime = 0.0f;
float wavesTime = 0.0f;

// Audio (se pueden agregar para que se ejecuten cuando se abra una puerta por ejemplo)
ISoundEngine *SoundEngine = createIrrKlangDevice();//Creación del motor de sonido---------------------------------------------------------------------------------------------
irrklang::ISoundSource *voz = SoundEngine->addSoundSourceFromFile("sound/Dialogo.mp3"); // revisar línea 245 para más info
irrklang::ISound *vozSonando;

// selección de cámara
bool    activeCamera = 1; // 1 = Cámara modo libre, 0 = Cámara vista desde el doctor

// Entrada a función principal (no moverle)
int main()
{
	if (!Start())
		return -1;

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		if (!Update())
			break;
	}

	glfwTerminate();
	return 0;

}

bool Start() {
	// Inicialización de GLFW

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Creación de la ventana con GLFW
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "FBX Animation with OpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// Ocultar el cursor mientras se rota la escena
	// glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: Cargar todos los apuntadores
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return false;
	}

	// Activación de buffer de profundidad
	glEnable(GL_DEPTH_TEST);

	// ------------------------ Compilación y enlace de shaders (agregar los shaders necesarios [fs y vs]) -----------------------------
	doctorCaminandoShader = new Shader("shaders/10_vertex_skinning-IT.vs", "shaders/10_fragment_skinning-IT.fs");
	doctorParadoShader = new Shader("shaders/10_vertex_skinning-IT.vs", "shaders/10_fragment_skinning-IT.fs");
	cubemapShader = new Shader("shaders/10_vertex_cubemap.vs", "shaders/10_fragment_cubemap.fs");
	mLightsShader = new Shader("shaders/11_PhongShaderMultLights.vs", "shaders/11_PhongShaderMultLights.fs");
	staticShader = new Shader("shaders/10_vertex_simple.vs", "shaders/10_fragment_simple.fs");
	fresnelShader = new Shader("shaders/11_Fresnel.vs", "shaders/11_Fresnel.fs");
	puertaShader = new Shader("shaders/11_PhongShaderMultLights.vs", "shaders/11_PhongShaderMultLights.fs");
	metalPhongShader = new Shader("shaders/11_PhongShaderMultLights.vs", "shaders/11_PhongShaderMultLights.fs");
	wavesShader = new Shader("shaders/13_wavesAnimation.vs", "shaders/13_wavesAnimation.fs");
	cartelesShader = new Shader("shaders/10_vertex_simple.vs", "shaders/10_fragment_simple.fs");

	// Máximo número de huesos: 100
	doctorCaminandoShader->setBonesIDs(MAX_RIGGING_BONES);
	doctorParadoShader->setBonesIDs(MAX_RIGGING_BONES);

	// Dibujar en malla de alambre
	// glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	// 
	// ----------------------------------------------------Importación de modelos -----------------------------------------------------------------
	// Para organización, crear el objeto y llamarlo por lo que es. Ejemplo:
	// 
	// consultorio = new Model("models/ProyectoFinal/<nombre_del_archivo>.fbx")
	hospital = new Model("models/FachadaConsultorioTest6.fbx"); // Cargar aquí el modelo del consultorio
	puerta = new Model("models/Puerta.fbx"); //Modelo de la puerta
	escritorio = new Model("models/Escritorio.fbx");

	doctorCaminando = new Model("models/doctorColor.fbx"); // Cargar modelo del personaje
	doctorParado = new Model("models/doctorColorParadoEscalado.fbx"); // Cargar modelo del personaje
	Carteles = new Model("models/CartelEntrada.fbx"); // Cargar modelo del personaje
	
	//Órganos:
	floorObject = new Model("models/floor.fbx");
	//Pancreas = new Model("models/Pancreas.fbx");
	DigestiveSystem = new Model("models/DigestiveSystem.fbx");
	Glass = new Model("models/Glass.fbx");
	MetalBase = new Model("models/MetalBase.fbx");
	Red = new Model("models/Red.fbx");
	Vein = new Model("models/Vein.fbx");
	CubeGlass = new Model("models/CubeGlass.fbx");
	MetalCube = new Model("models/MetalCube.fbx");

	
	Blood = new Model("models/Blood.fbx");
	Sugar = new Model("models/Sugar.fbx");
	InsulinKey = new Model("models/InsulinKey.fbx");
	

	// Cubemap
	vector<std::string> faces
	{
		"textures/cubemap/01/posx.jpg",
		"textures/cubemap/01/negx.jpg",
		"textures/cubemap/01/posy.jpg",
		"textures/cubemap/01/negy.jpg",
		"textures/cubemap/01/posz.jpg",
		"textures/cubemap/01/negz.jpg"
	};
	mainCubeMap = new CubeMap();
	mainCubeMap->loadCubemap(faces);
	
	// time, arrays
	// Variables de los personajes animados
	doctorCaminando->SetPose(0.0f, gBones);//Pose inicial, y se sobreescribe en cada frame.
	doctorParado->SetPose(0.0f, gBones);
	
	fpsCaminando = (float)doctorCaminando->getFramerate();
	keysCaminando = (int)doctorCaminando->getNumFrames();

	fpsParado = (float)doctorParado->getFramerate(); // Necesario importar la de todos los modelos que se muevan
	keysParado = (int)doctorParado->getNumFrames();

	// Inicialización de la cámara (3ra persona)
	camera3rd.Position = position;
	camera3rd.Position.y += 5.0f;
	camera3rd.Position -= forwardView;
	camera3rd.Front = forwardView;

	// --------------------------Lights configuration (se meten a un arreglo, pueden desactivarse)----------------------------
	
	Light light01;
	light01.Position = glm::vec3(7.0f, 17.5f, 1.5f);
	light01.Color = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f); // color de las luces. Al estar en la misma medida, son blancas
	light01.Direction = glm::vec3(1.0f, 1.0f, 1.0f); // Dirección (no aplica para fuentes puntuales)
	//gLights.push_back(light01);

	Light light02;
	light02.Position = glm::vec3(8.1f, 15.4f, -2.1f);
	light02.Color = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
	light02.Direction = glm::vec3(1.0f, 1.0f, 1.0f); // Dirección (no aplica para fuentes puntuales)
	gLights.push_back(light02);

	Light light03;
	light03.Position = glm::vec3(-16.0f, 32.0f, 34.3f);
	light03.Color = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
	light03.Direction = glm::vec3(1.0f, 1.0f, 1.0f); // Dirección (no aplica para fuentes puntuales)
	gLights.push_back(light03);

	Light light04;
	light04.Position = glm::vec3(-5.0f, 2.0f, -5.0f);
	light04.Color = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
	//gLights.push_back(light04);

	//------------------------------------------------- Editando instancias de propiedades del material--------------------------------
	// Acabado metálico
	metal02.ambient = glm::vec4(0.19225f, 0.19225f, 0.19225f, 1.0f);
	metal02.diffuse = glm::vec4(0.50754f, 0.50754f, 0.50754f, 1.0f);
	metal02.specular = glm::vec4(0.508273f, 0.508273f, 0.508273f, 1.0f);
	
	// --------------------------------------------------Efectos de sonido (sólo inicializaciones) -----------------------------------------
	voz->setDefaultVolume(0.65f); //Estableciendo volumen
	vozSonando = SoundEngine->play2D(voz, false, true); //preparando sonido (inicializado en mute).

	return true;
}


void SetLightUniformInt(Shader* shader, const char* propertyName, size_t lightIndex, int value) {
	std::ostringstream ss;
	ss << "allLights[" << lightIndex << "]." << propertyName;
	std::string uniformName = ss.str();

	shader->setInt(uniformName.c_str(), value);
}
void SetLightUniformFloat(Shader* shader, const char* propertyName, size_t lightIndex, float value) {
	std::ostringstream ss;
	ss << "allLights[" << lightIndex << "]." << propertyName;
	std::string uniformName = ss.str();

	shader->setFloat(uniformName.c_str(), value);
}
void SetLightUniformVec4(Shader* shader, const char* propertyName, size_t lightIndex, glm::vec4 value) {
	std::ostringstream ss;
	ss << "allLights[" << lightIndex << "]." << propertyName;
	std::string uniformName = ss.str();

	shader->setVec4(uniformName.c_str(), value);
}
void SetLightUniformVec3(Shader* shader, const char* propertyName, size_t lightIndex, glm::vec3 value) {
	std::ostringstream ss;
	ss << "allLights[" << lightIndex << "]." << propertyName;
	std::string uniformName = ss.str();

	shader->setVec3(uniformName.c_str(), value);
}

// -------------------------------------------- Ciclo de renderizado ------------------------------------------------------
bool Update() {
	// Cálculo del framerate
	float currentFrame = (float)glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	elapsedTime += deltaTime;
	if (elapsedTime > 1.0f / fpsCaminando) {
		// A partir de aquí se cuentan individualmente los frames de cada objeto
		animationCountCaminando++;
		if (animationCountCaminando > keysCaminando - 1) {
			animationCountCaminando = 0;
		}
		animationCountParado++;
		if (animationCountParado > keysParado - 1) {
			animationCountParado = 0;
		}

		// Configuración de la pose en el instante t
		if (activeCamera) {	// sólo cuando se tiene la cámara en primera persona
			if (isWalking == true) {
				doctorCaminando->SetPose((float)animationCountCaminando, gBones);
			}
			else {
				doctorParado->SetPose((float)animationCountParado, gBones);
			}
		}
		else {
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // limpia el búffer de dibujo, útil para tercera persona.
		}
		elapsedTime = 0.0f;

		// Procesa la entrada del teclado o mouse
		/*
		processInput(window);
		gLights[0].Position = lPosition; // cambiar indice para cambiar fuente de luz a mover.
		*/
	}

	// Procesa la entrada del teclado o mouse
	processInput(window);

	// Renderizado R - G - B - A
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Cubemap (fondo)
	{
		// Transformaciones de cámara (Mantienen estático el fondo sin importar la cámara)
		glm::mat4 projection;
		glm::mat4 view;

		if (activeCamera) {
			projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
			view = camera.GetViewMatrix();
		}
		else {
			projection = glm::perspective(glm::radians(camera3rd.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
			view = camera3rd.GetViewMatrix();
		}

		mainCubeMap->drawCubeMap(*cubemapShader, projection, view);
	}
	
	// Utilizando Shader de iluminación.
	 {
		mLightsShader->use();
		//metalPhongShader->use();
		puertaShader->use(); // Shader de puerta
		cartelesShader->use();

		// Activamos para objetos transparentes
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glm::mat4 projection;
		glm::mat4 view;

		if (activeCamera) {
			projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
			view = camera.GetViewMatrix();
		}
		else {
			projection = glm::perspective(glm::radians(camera3rd.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
			view = camera3rd.GetViewMatrix();
		}
		
		mLightsShader->setMat4("projection", projection);
		mLightsShader->setMat4("view", view);
		puertaShader->setMat4("projection", projection);
		puertaShader->setMat4("view", view);
		cartelesShader->setMat4("projection", projection);
		cartelesShader->setMat4("view", view);
		//metalPhongShader->setMat4("projection", projection);
		//metalPhongShader->setMat4("view", view);

		// Aplicamos transformaciones del modelo
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down

		mLightsShader->setMat4("model", model);
		puertaShader->setMat4("model", model);
		cartelesShader->setMat4("model", model);

		Carteles->Draw(*cartelesShader); // Dibujando Carteles

		//model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));	// it's a bit too big for our scene, so scale it down
		//metalPhongShader->setMat4("model", model);

		// Configuramos propiedades de fuentes de luz
		mLightsShader->setInt("numLights", (int)gLights.size());
		for (size_t i = 0; i < gLights.size(); ++i) {
			SetLightUniformVec3(mLightsShader, "Position", i, gLights[i].Position);
			SetLightUniformVec3(mLightsShader, "Direction", i, gLights[i].Direction);
			SetLightUniformVec4(mLightsShader, "Color", i, gLights[i].Color);
			SetLightUniformVec4(mLightsShader, "Power", i, gLights[i].Power);
			SetLightUniformInt(mLightsShader, "alphaIndex", i, gLights[i].alphaIndex);
			SetLightUniformFloat(mLightsShader, "distance", i, gLights[i].distance);
		}

		mLightsShader->setVec3("eye", camera.Position);

		puertaShader->setInt("numLights", (int)gLights.size());
		for (size_t i = 0; i < gLights.size(); ++i) {
			SetLightUniformVec3(puertaShader, "Position", i, gLights[i].Position);
			SetLightUniformVec3(puertaShader, "Direction", i, gLights[i].Direction);
			SetLightUniformVec4(puertaShader, "Color", i, gLights[i].Color);
			SetLightUniformVec4(puertaShader, "Power", i, gLights[i].Power);
			SetLightUniformInt(puertaShader, "alphaIndex", i, gLights[i].alphaIndex);
			SetLightUniformFloat(puertaShader, "distance", i, gLights[i].distance);
		}
		
		puertaShader->setVec3("eye", camera.Position);

		/*
		// Configuramos propiedades de fuentes de luz
		metalPhongShader->setInt("numLights", (int)gLights.size());
		for (size_t i = 0; i < gLights.size(); ++i) {
			SetLightUniformVec3(metalPhongShader, "Position", i, gLights[i].Position);
			SetLightUniformVec3(metalPhongShader, "Direction", i, gLights[i].Direction);
			SetLightUniformVec4(metalPhongShader, "Color", i, gLights[i].Color);
			SetLightUniformVec4(metalPhongShader, "Power", i, gLights[i].Power);
			SetLightUniformInt(metalPhongShader, "alphaIndex", i, gLights[i].alphaIndex);
			SetLightUniformFloat(metalPhongShader, "distance", i, gLights[i].distance);
		}

		metalPhongShader->setVec3("eye", camera.Position);*/


		// Aplicamos propiedades materiales
		mLightsShader->setVec4("MaterialAmbientColor", material01.ambient);
		mLightsShader->setVec4("MaterialDiffuseColor", material01.diffuse);
		mLightsShader->setVec4("MaterialSpecularColor", material01.specular);
		mLightsShader->setFloat("transparency", material01.transparency);

		hospital->Draw(*mLightsShader);

		// Aplicamos propiedades materiales
		metalPhongShader->setVec4("MaterialAmbientColor", metal02.ambient);
		metalPhongShader->setVec4("MaterialDiffuseColor", metal02.diffuse);
		metalPhongShader->setVec4("MaterialSpecularColor", metal02.specular);
		metalPhongShader->setFloat("transparency", metal02.transparency);

		escritorio->Draw(*metalPhongShader);

		puertaShader->setVec4("MaterialAmbientColor", material01.ambient);
		puertaShader->setVec4("MaterialDiffuseColor", material01.diffuse);
		puertaShader->setVec4("MaterialSpecularColor", material01.specular);
		puertaShader->setFloat("transparency", material01.transparency);


		model = glm::mat4(1.0f); // se reinicia la matriz de modelo.

		// ------------------------- Agregar los modelos de las animaciones básicas, cada una con un shader y centrada en el origen en blender ---------
		//Efecto de puerta corrediza
		 //model = glm::translate(model, glm::vec3(0.418f + door_offset, 0.0f, 6.75f)); //Procesar solo la transición
		
		// Efecto de puerta con bisagra
		model = glm::translate(model, glm::vec3(10.1f, 0.0f, 17.3f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // necesario por el cambio de coordenadas de blender
		model = glm::rotate(model, glm::radians(puerta_rotation), glm::vec3(0.0f, 0.0f, 1.0f));
		
		puertaShader->setMat4("model", model); // mandando la matriz de modelo

		 puerta->Draw(*puertaShader); // dibujado de la puerta 
	}

	glUseProgram(0);

	//Objetos estáticos
	{
		// Activamos el shader del plano
		staticShader->use();

		// Activamos para objetos transparentes
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Aplicamos transformaciones de proyección y cámara (si las hubiera)
		glm::mat4 projection;
		glm::mat4 view;

		if (activeCamera) {
			projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
			view = camera.GetViewMatrix();
		}
		else {
			projection = glm::perspective(glm::radians(camera3rd.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
			view = camera3rd.GetViewMatrix();
		}

		staticShader->setMat4("projection", projection);
		staticShader->setMat4("view", view);

		// Aplicamos transformaciones del modelo
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -0.1f, 0.0f)); // translate it down so it's at the center of the scene
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(5.0f, 5.0f, 5.0f));	// it's a bit too big for our scene, so scale it down
		staticShader->setMat4("model", model);

		floorObject->Draw(*staticShader);
		/*
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-12.8f, 5.2f, 1.3f)); // translate it down so it's at the center of the scene
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(organos_rotation), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));	// it's a bit too big for our scene, so scale it down
		staticShader->setMat4("model", model);

		Pancreas->Draw(*staticShader);
		*/
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-12.5f, 1.8f, 0.45f)); // translate it down so it's at the center of the scene
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(organos_rotation), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));	// it's a bit too big for our scene, so scale it down
		staticShader->setMat4("model", model);

		DigestiveSystem->Draw(*staticShader);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-12.5f, 1.25f, 0.5f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.25f));
		staticShader->setMat4("model", model);

		MetalBase->Draw(*staticShader);
		
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-12.5f, 5.0f+torrance_position, -5.5f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.45f, 0.4f, 0.45f));
		staticShader->setMat4("model", model);

		Red->Draw(*staticShader);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-12.5f, 0.7f, -7.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f, 3.5f, 2.5f));
		staticShader->setMat4("model", model);

		MetalCube->Draw(*staticShader);

		
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-12.5f, 3.8f, -4.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.2f+ models_scale, 0.2f+ models_scale, 0.2f+ models_scale));
		staticShader->setMat4("model", model);

		Blood->Draw(*staticShader);
		
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-12.5f, 3.8f, -7.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.3f+ models_scale, 0.3f+ models_scale, 0.3f+ models_scale));
		staticShader->setMat4("model", model);

		Sugar->Draw(*staticShader);
		
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-12.5f, 3.8f, -9.5f));
		model = glm::rotate(model, glm::radians(-180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.1f+ models_scale, 0.1f+ models_scale, 0.05f+ models_scale));
		staticShader->setMat4("model", model);

		InsulinKey->Draw(*staticShader);

	}

	glUseProgram(0);

	
	// Aplicamos Phong con acabado metálico.
	/*
	{
		puertaShader->use(); // Shader de puerta

		// Activamos para objetos transparentes
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glm::mat4 projection;
		glm::mat4 view;

		if (activeCamera) {
			projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
			view = camera.GetViewMatrix();
		}
		else {
			projection = glm::perspective(glm::radians(camera3rd.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
			view = camera3rd.GetViewMatrix();
		}

		metalPhongShader->setMat4("projection", projection);
		metalPhongShader->setMat4("view", view);

		// Aplicamos transformaciones del modelo
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));	// it's a bit too big for our scene, so scale it down

		//model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));	// it's a bit too big for our scene, so scale it down
		metalPhongShader->setMat4("model", model);


		// Configuramos propiedades de fuentes de luz
		metalPhongShader->setInt("numLights", (int)gLights.size());
		for (size_t i = 0; i < gLights.size(); ++i) {
			SetLightUniformVec3(metalPhongShader, "Position", i, gLights[i].Position);
			SetLightUniformVec3(metalPhongShader, "Direction", i, gLights[i].Direction);
			SetLightUniformVec4(metalPhongShader, "Color", i, gLights[i].Color);
			SetLightUniformVec4(metalPhongShader, "Power", i, gLights[i].Power);
			SetLightUniformInt(metalPhongShader, "alphaIndex", i, gLights[i].alphaIndex);
			SetLightUniformFloat(metalPhongShader, "distance", i, gLights[i].distance);
		}

		metalPhongShader->setVec3("eye", camera.Position);

		// Aplicamos propiedades materiales
		metalPhongShader->setVec4("MaterialAmbientColor", metal02.ambient);
		metalPhongShader->setVec4("MaterialDiffuseColor", metal02.diffuse);
		metalPhongShader->setVec4("MaterialSpecularColor", metal02.specular);
		metalPhongShader->setFloat("transparency", metal02.transparency);

		escritorio->Draw(*metalPhongShader);
	}

	glUseProgram(0);
	*/
	// Dibujamos vidrio
	{
		// Activamos el shader de fresnel
		fresnelShader->use();

		// Activamos para objetos transparentes
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Aplicamos transformaciones de proyección y cámara (si las hubiera)
		glm::mat4 projection;
		glm::mat4 view;

		if (activeCamera) {
			projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
			view = camera.GetViewMatrix();
		}
		else {
			projection = glm::perspective(glm::radians(camera3rd.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
			view = camera3rd.GetViewMatrix();
		}
		
		fresnelShader->setMat4("projection", projection);
		fresnelShader->setMat4("view", view);

		// Aplicamos transformaciones del modelo
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-12.5f, 5.0f, 0.5f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.5f, 1.5f, 2.5f));
		fresnelShader->setMat4("model", model);
		fresnelShader->setFloat("time", sineTime);
		sineTime += 0.001;
		fresnelShader->setFloat("transparency", 0.2f);

		glDepthMask(GL_FALSE);
		glBindTexture(GL_TEXTURE_CUBE_MAP, mainCubeMap->textureID);
		glDepthMask(GL_TRUE);
		Glass->Draw(*fresnelShader);

		// Aplicamos transformaciones del modelo
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-12.5f, 5.0f, -7.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f, 3.5f, 2.0f));
		fresnelShader->setMat4("model", model);
		fresnelShader->setFloat("time", sineTime);
		sineTime += 0.001;
		fresnelShader->setFloat("transparency", 0.2f);

		glDepthMask(GL_FALSE);
		glBindTexture(GL_TEXTURE_CUBE_MAP, mainCubeMap->textureID);
		glDepthMask(GL_TRUE);
		CubeGlass->Draw(*fresnelShader);
	}

	glUseProgram(0); 

	// Animacion procedural
	{
		// Activamos el shader de WavesTime
		wavesShader->use();

		// Activamos para objetos transparentes
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Aplicamos transformaciones de proyección y cámara (si las hubiera)
		glm::mat4 projection;
		glm::mat4 view;

		if (activeCamera) {
			projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
			view = camera.GetViewMatrix();
		}
		else {
			projection = glm::perspective(glm::radians(camera3rd.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
			view = camera3rd.GetViewMatrix();
		}

		wavesShader->setMat4("projection", projection);
		wavesShader->setMat4("view", view);

		// Aplicamos transformaciones del modelo
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-12.5f, 5.0f+torrance_position, -6.5f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
		wavesShader->setMat4("model", model);

		wavesShader->setFloat("time", wavesTime);
		wavesShader->setFloat("radius", 0.1f);
		wavesShader->setFloat("height", 0.1f);

		Vein->Draw(*wavesShader);
		wavesTime += 0.01;

	}

	glUseProgram(0);
	

	
	// Objetos animados por keyframes
	{
		// Activación del shader del personaje
		doctorCaminandoShader->use();
		doctorParadoShader->use();

		// Aplicamos transformaciones de proyección y cámara (si las hubiera)

		glm::mat4 projection;
		glm::mat4 view;

		if (activeCamera) {
			projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
			view = camera.GetViewMatrix();
		}
		else {
			projection = glm::perspective(glm::radians(camera3rd.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
			view = camera3rd.GetViewMatrix();
		}

		doctorCaminandoShader->setMat4("projection", projection);
		doctorCaminandoShader->setMat4("view", view);

		doctorParadoShader->setMat4("projection", projection);
		doctorParadoShader->setMat4("view", view);

		// Aplicamos transformaciones del modelo
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, position); // translate it down so it's at the center of the scene
		model = glm::rotate(model, glm::radians(rotateCharacter), glm::vec3(0.0, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.008f, 0.008f, 0.008f));	// it's a bit too big for our scene, so scale it down

		doctorCaminandoShader->setMat4("model", model);
		doctorParadoShader->setMat4("model", model);

		doctorCaminandoShader->setMat4("gBones", MAX_RIGGING_BONES, gBones);
		doctorParadoShader->setMat4("gBones", MAX_RIGGING_BONES, gBones);

		// Dibujamos el modelo en ambos para mantenerlo sincronizado
		if (activeCamera) {
			if (isWalking == true) {
				doctorCaminando->Draw(*doctorCaminandoShader);
			}
			else {
				doctorParado->Draw(*doctorParadoShader);
			}
		}
	}

	glUseProgram(0); 

	// glfw: swap buffers 
	glfwSwapBuffers(window);
	glfwPollEvents();

	return true;
}

// Procesamos entradas del teclado
// Podemos usar comandos de teclas tipo, ALT + Letra, poniendo condicionales if ALT AND Letra
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// ------------------------ Movimiento de la cámara ----------------------
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(UP, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(DOWN, deltaTime);

	// ------------------------- Modo de visualizar poligonos (vértices, mayas y texturas) -----------------
	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);

	// -------------------------- Transformaciones básicas (puertas) ----------------------------
	/*Desplazamiento sobre un eje
	if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) {
		if(door_offset < 0.836f)
			door_offset += 0.01f;			//offset de puerta
	} 
		
	if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) {
		if (door_offset > -0.836f)
			door_offset -= 0.01f;
	}
	*/
	// Rotación de la puerta de madera
	if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {
		if (puerta_rotation < 130.0f) {
			puerta_rotation += 1.f;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
		if (puerta_rotation > 0.0f) {
			puerta_rotation -= 1.f;
		}
	}

	// Rotación de los organos
	if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) {
		if (organos_rotation < 180.0f) {
			organos_rotation += 2.0f;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) {
		if (organos_rotation > -180.0f) {
			organos_rotation -= 2.0f;
		}
	}

	// Traslacion del torrente sanguineo
	if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
		if (torrance_position < 1.0f) {
			torrance_position += 0.09f;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
		if (torrance_position > -0.2f) {
			torrance_position -= 0.09f;
		}
	}

	// Escala de modelos de sangre, glucosa e insulina
	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
		if (models_scale < 0.03f) {
			models_scale += 0.001f;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
		if (models_scale > -0.03f) {
			models_scale -= 0.001f;
		}
	}
	
	// Movimiento del personaje (prácticamente sólo activa una variable y transforma la cámara)
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		isWalking = true;
		position = position + scaleV * forwardView * 5.5f;
		camera3rd.Front = forwardView;
		camera3rd.ProcessKeyboard(FORWARD, deltaTime);
		camera3rd.Position = position;
		camera3rd.Position.y += 5.0f;
		camera3rd.Position -= forwardView;

	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		isWalking = true;
		position = position - scaleV * forwardView * 5.5f;
		camera3rd.Front = forwardView;
		camera3rd.ProcessKeyboard(BACKWARD, deltaTime);
		camera3rd.Position = position;
		camera3rd.Position.y += 5.0f;
		camera3rd.Position -= forwardView;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		isWalking = true;
		rotateCharacter += 1.0f;
		//Se aplica la rotación
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(rotateCharacter), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::vec4 viewVector = model * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
		forwardView = glm::vec3(viewVector);
		forwardView = glm::normalize(forwardView);

		camera3rd.Front = forwardView;
		camera3rd.Position = position;
		camera3rd.Position.y += 5.0f;
		camera3rd.Position -= forwardView;
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		isWalking = true;
		rotateCharacter -= 1.0f;

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(rotateCharacter), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::vec4 viewVector = model * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
		forwardView = glm::vec3(viewVector);
		forwardView = glm::normalize(forwardView);

		camera3rd.Front = forwardView;
		camera3rd.Position = position;
		camera3rd.Position.y += 5.0f;
		camera3rd.Position -= forwardView;
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) != GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT) != GLFW_PRESS && glfwGetKey(window, GLFW_KEY_UP) != GLFW_PRESS && glfwGetKey(window, GLFW_KEY_DOWN) != GLFW_PRESS) {
		isWalking = false;
	}

	// ---------------------------------- Vistas de las cámaras ------------------------------------
	if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS)
		activeCamera = 0;
	if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS)
		activeCamera = 1;
	
	// ----------------------------------- Manejo del sonido ---------------------------------------
	//Ejemplo de manejo de teclas para sonido
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
		if (vozSonando)
			vozSonando->setIsPaused(true); // Pausando sonido
	}
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
		if (vozSonando)
			vozSonando->setIsPaused(false); // Reproduciendo sonido
	}

	// ----------------------------------- Auxiliar para ubicación en OpenGL ---------------------------------------
	//Cada que se presione, muestra la posición de la cámara libre/cámara del doctor
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
		if (activeCamera) {
			printf("\nPosicion de la camara:\nX: %f\nY: %f\nZ: %f\n\n", camera.Position.x, camera.Position.y, camera.Position.z);
			printf("\nPosicion de la luz:\nX: %f\nY: %f\nZ: %f\n\n", gLights[0].Position.x, gLights[0].Position.y, gLights[0].Position.z);
		}
		else {
			printf("\nPosicion de la camara:\nX: %f\nY: %f\nZ: %f\n\n", camera3rd.Position.x, camera3rd.Position.y, camera3rd.Position.z);
		}
	}

	// Variables debug para la luz
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
		lPosition.y += 0.1f;
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
		lPosition.y -= 0.1f;
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
		lPosition.x -= 0.1f;
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
		lPosition.x += 0.1f;
	if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
		lPosition.z -= 0.1f;
	if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS)
		lPosition.z += 0.1f;
}

// glfw: Actualizamos el puerto de vista si hay cambios del tamaño
// de la ventana
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// glfw: Callback del movimiento y eventos del mouse
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = (float)xpos;
		lastY = (float)ypos;
		firstMouse = false;
	}

	float xoffset = (float)xpos - lastX;
	float yoffset = lastY - (float)ypos; 

	lastX = (float)xpos;
	lastY = (float)ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: Complemento para el movimiento y eventos del mouse
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll((float)yoffset);
}
