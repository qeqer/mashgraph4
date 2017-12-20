//internal includes
#include "common.h"
#include "ShaderProgram.h"
#include "Camera.h"

//External dependencies
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include <random>
#include "SOIL/SOIL.h"

#define WATER 1
#define SOIL 0


static const GLsizei WIDTH = 1920, HEIGHT = 1080; //размеры окна
static int filling = 0;
static bool keys[1024]; //массив состояний кнопок - нажата/не нажата
static GLfloat lastX = 400, lastY = 300; //исходное положение мыши
static bool firstMouse = true;
static bool g_captureMouse         = true;  // Мышка захвачена нашим приложением или нет?
static bool g_capturedMouseJustNow = false;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;
static int mode1 = 1;
static float fog = 80;
static bool fog_act = 0;
static float gorit = 0.0;
static float shift = 0.5;



Camera camera(float3(50.0f, 50.0f, 50.0f));

//функция для обработки нажатий на кнопки клавиатуры
void OnKeyboardPressed(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	//std::cout << key << std::endl;
	switch (key)
	{
	case GLFW_KEY_ESCAPE: //на Esc выходим из программы
		if (action == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GL_TRUE);
		break;
	case GLFW_KEY_SPACE: //на пробел переключение в каркасный режим и обратно
		if (action == GLFW_PRESS)
		{
			if (filling == 0)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				filling = 1;
			}
			else
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				filling = 0;
			}
		}
		break;
	case GLFW_KEY_1:
		mode1 = 0;
		//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		break;
	case GLFW_KEY_2:
		mode1 = 1;
		// glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		break;
	case GLFW_KEY_F:
		if (action == GLFW_PRESS) {
			if (fog_act) {
				fog_act = false;
			} else {
				fog_act = true;
			}
		}
		break;
	default:
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;
	}
}

//функция для обработки клавиш мыши
void OnMouseButtonClicked(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
		g_captureMouse = !g_captureMouse;


	if (g_captureMouse)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		g_capturedMouseJustNow = true;
	}
	else
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

}

//функция для обработки перемещения мыши
void OnMouseMove(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = float(xpos);
		lastY = float(ypos);
		firstMouse = false;
	}

	GLfloat xoffset = float(xpos) - lastX;
	GLfloat yoffset = lastY - float(ypos);

	lastX = float(xpos);
	lastY = float(ypos);

	if (g_captureMouse)
		camera.ProcessMouseMove(xoffset, yoffset);
}


void OnMouseScroll(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(GLfloat(yoffset));
}

void doCameraMovement(Camera &camera, GLfloat deltaTime)
{
	if (keys[GLFW_KEY_W])
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (keys[GLFW_KEY_A])
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (keys[GLFW_KEY_S])
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (keys[GLFW_KEY_D])
		camera.ProcessKeyboard(RIGHT, deltaTime);
}


/**
\brief создать triangle strip плоскость и загрузить её в шейдерную программу
\param rows - число строк
\param cols - число столбцов
\param size - размер плоскости
\param vao - vertex array object, связанный с созданной плоскостью
*/
float terrain = 0.2;
float he = 1;
int res = 257;
float sz = 40;
float max_h = 10;
float min_h = -3;
float powing = 2;
float waves = 0.01;
float waves_hz = 10;



void filldiam(std::vector<std::vector<GLfloat> > &strip, int rowl, int rowr, int coll, int colr, int rows, int cols, float size) {
	int up = rowl >= 0 ? rowl : rows - 1 + rowl;
	int down = rowr < rows ? rowr : rowr - rows + 1;
	int left = coll >= 0 ? coll : cols - 1 + coll;
	int right = colr < cols ? colr : colr - cols + 1;
	strip[(rowr + rowl) / 2][(colr + coll) / 2] = (
	            strip[up][(coll + colr) / 2] +
	            strip[down][(coll + colr) / 2] +
	            strip[(rowl + rowr) / 2][left] +
	            strip[(rowl + rowr) / 2][right]
	        ) / 4 + terrain * (float(rand()) / RAND_MAX - 0.5) * (size / rows * (rowl - rowr));
	// printf("Diam: %d %d %d %d %d %d %d %d %d %d %f\n", rowl, rowr, coll, colr, rows, cols, up, down, left, right, strip[(rowr + rowl) / 2][(colr + coll) / 2]);
	return;
}

void fillsq(std::vector<std::vector<GLfloat> > &strip, int rowl, int rowr, int coll, int colr, int rows, int cols, float size) {
	// printf("Square: %d %d %d %d\n", rowl, rowr, coll, colr);
	strip[(rowl + rowr) / 2][(coll + colr) / 2] = (
	            strip[rowl][coll] +
	            strip[rowl][colr] +
	            strip[rowr][coll] +
	            strip[rowr][colr]) /
	        4 + terrain * (float(rand()) / RAND_MAX - 0.5) * (size / rows * (rowl - rowr)) * 0.7; //central point of square

	return;
}






void fillstrip(std::vector<std::vector<GLfloat> > &strip, int rows, int cols, float size) {
	printf("Started: %d %d %lu %lu\n", rows, cols, strip.size(), strip[0].size());
	strip[0][0] = 0;//terrain * (float(rand()) / RAND_MAX - 0.5) * size;
	strip[0][cols - 1] = 0;// terrain * (float(rand()) / RAND_MAX - 0.5) * size;
	strip[(rows - 1)][0] = 0;//terrain * (float(rand()) / RAND_MAX - 0.5) * size;
	strip[(rows - 1)][cols - 1] = 0;//terrain * (float(rand()) / RAND_MAX - 0.5) * size;
	printf("Edges filled\n");
	// fillsq(strip, 0, rows - 1, 0, cols - 1, rows, cols, size);
	rows--;
	cols--;
	int step_size = rows;
	while (step_size > 1) {
		for (int i = 0; i < rows; i += step_size) {
			for (int j = 0; j < cols; j += step_size) {
				fillsq(strip, i, i + step_size, j, j + step_size, rows + 1, cols + 1, size);
			}
		}
		for (int i = 0; i < rows; i += step_size) {
			for (int j = 0; j <= cols; j += step_size) {
				filldiam(strip, i, i + step_size, j - step_size / 2, j + step_size / 2, rows + 1, cols + 1, size);
			}
		}
		for (int i = 0; i <= rows; i += step_size) {
			for (int j = 0; j < cols; j += step_size) {
				filldiam(strip, i - step_size / 2, i + step_size / 2, j, j + step_size, rows + 1, cols + 1, size);
			}
		}


		step_size = step_size / 2;


	}
	printf("All filled\n");
	// for (int i = 0; i < rows + 1; i++) {
	// 	printf("!!!");
	//   for (int j = 0; j < cols + 1; j++) {
	//     printf("%-4.1f ", strip[i][j]);
	//   }
	//   printf("???\n");
	// }

	return;
}


float box_filter(std::vector<std::vector<GLfloat> > &strip, uint row, uint col) {
	float sum = 0;
	for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {
			int x = int(row) + i, y = int(col) + j;
			if (x < 0) {
				x = x + strip.size();
			}
			if (x >= strip.size()) {
				x = x - strip.size();
			}
			if (y < 0) {
				y = y + strip.size();
			}
			if (y >= strip[0].size()) {
				y = y - strip[0].size();
			}
			sum += strip[x][y];
		}
	}
	return sum / 9;
}

void makeflatwater(std::vector<std::vector<GLfloat> > &strip) {
	for (uint i = 0; i < strip.size(); i++) {
		for (uint j = 0; j < strip[i].size(); j++) {
			if (strip[i][j] < 0) {
				strip[i][j] = -0.5;
			}
		}
	}
	return;
}

void makemagic(std::vector<std::vector<GLfloat> > &strip) { //нормировка, бокс фильтер
	float min = 0, max = 0;
	for (uint i = 0; i < strip.size(); i++) {
		for (uint j = 0; j < strip[i].size(); j++) {
			if (strip[i][j] < min) {
				min = strip[i][j];
			}
			if (strip[i][j] > max) {
				max = strip[i][j];
			}
		}
	}
	std::vector<std::vector<GLfloat> > temp(strip.size(), std::vector<GLfloat>(strip[0].size(), 0.0));
	float delta = (max - min);
	float delta_pref = (max_h - min_h);
	for (uint i = 0; i < strip.size(); i++) {
		for (uint j = 0; j < strip[i].size(); j++) {
			strip[i][j] = pow((strip[i][j] - min) / delta, 2) * delta_pref + min_h;
			temp[i][j] = strip[i][j];
		}
	}
	for (uint i = 1; i < strip.size() - 1; i++) {
		for (uint j = 1; j < strip[i].size() - 1; j++) {
			strip[i][j] = box_filter(temp, i, j);
		}
	}

	return;
}

void makewater(std::vector<std::vector<GLfloat> > &strip, float size) {
	for (uint i = 0; i < strip.size(); i++) {
		for (uint j = 0; j < strip[0].size(); j++) {
			strip[i][j] = 0.1 + waves * sin(waves_hz * i * size / strip.size());
		}
	}
	return;
}

static int createTriStrip(int rows, int cols, float size, GLuint &vao, int type)
{

	int numIndices = 2 * cols * (rows - 1) + rows - 1;

	std::vector<GLfloat> vertices_vec; //вектор атрибута координат вершин
	vertices_vec.reserve(rows * cols * 3);

	std::vector<GLfloat> normals_vec; //вектор атрибута нормалей к вершинам
	normals_vec.reserve(rows * cols * 3);

	std::vector<GLfloat> texcoords_vec; //вектор атрибут текстурных координат вершин
	texcoords_vec.reserve(rows * cols * 2);

	std::vector<float3> normals_vec_tmp(rows * cols, float3(0.0f, 0.0f, 0.0f)); //временный вектор нормалей, используемый для расчетов

	std::vector<int3> faces;         //вектор граней (треугольников), каждая грань - три индекса вершин, её составляющих; используется для удобства расчета нормалей
	faces.reserve(numIndices / 3);

	std::vector<GLuint> indices_vec; //вектор индексов вершин для передачи шейдерной программе
	indices_vec.reserve(numIndices);
	srand(time(NULL));

	std::vector<std::vector<GLfloat> > ytemp(rows, std::vector<GLfloat>(cols, 0.0));
	if (type == SOIL) {
		fillstrip(ytemp, rows, cols, size);
		makemagic(ytemp);
		// makeflatwater(ytemp);
	}
	if (type == WATER) {
		makewater(ytemp, size);
	}
	for (int z = 0; z < rows; ++z)
	{
		for (int x = 0; x < cols; ++x)
		{
			//вычисляем координаты каждой из вершин
			float xx = -size / 2 + x * size / cols;
			float zz = -size / 2 + z * size / rows;
			float yy = float(ytemp[z][x]) / he;
			//float r = sqrt(xx*xx + zz*zz);
			// float yy = 5.0f * (r != 0.0f ? sin(r) / r : 1.0f);

			vertices_vec.push_back(xx);
			vertices_vec.push_back(yy);
			vertices_vec.push_back(zz);

			texcoords_vec.push_back(x / float(cols - 1)); // вычисляем первую текстурную координату u, для плоскости это просто относительное положение вершины
			texcoords_vec.push_back(z / float(rows - 1)); // аналогично вычисляем вторую текстурную координату v
		}
	}

	//primitive restart - специальный индекс, который обозначает конец строки из треугольников в triangle_strip
	//после этого индекса формирование треугольников из массива индексов начнется заново - будут взяты следующие 3 индекса для первого треугольника
	//и далее каждый последующий индекс будет добавлять один новый треугольник пока снова не встретится primitive restart index

	int primRestart = cols * rows;

	for (int x = 0; x < cols - 1; ++x)
	{
		for (int z = 0; z < rows - 1; ++z)
		{
			int offset = x * cols + z;

			//каждую итерацию добавляем по два треугольника, которые вместе формируют четырехугольник
			if (z == 0) //если мы в начале строки треугольников, нам нужны первые четыре индекса
			{
				indices_vec.push_back(offset + 0);
				indices_vec.push_back(offset + rows);
				indices_vec.push_back(offset + 1);
				indices_vec.push_back(offset + rows + 1);
			}
			else // иначе нам достаточно двух индексов, чтобы добавить два треугольника
			{
				indices_vec.push_back(offset + 1);
				indices_vec.push_back(offset + rows + 1);

				if (z == rows - 2) indices_vec.push_back(primRestart); // если мы дошли до конца строки, вставляем primRestart, чтобы обозначить переход на следующую строку
			}
		}
	}

	///////////////////////
	//формируем вектор граней(треугольников) по 3 индекса на каждый
	int currFace = 1;
	for (int i = 0; i < indices_vec.size() - 2; ++i)
	{
		int3 face;

		int index0 = indices_vec.at(i);
		int index1 = indices_vec.at(i + 1);
		int index2 = indices_vec.at(i + 2);

		if (index0 != primRestart && index1 != primRestart && index2 != primRestart)
		{
			if (currFace % 2 != 0) //если это нечетный треугольник, то индексы и так в правильном порядке обхода - против часовой стрелки
			{
				face.x = indices_vec.at(i);
				face.y = indices_vec.at(i + 1);
				face.z = indices_vec.at(i + 2);

				currFace++;
			}
			else //если треугольник четный, то нужно поменять местами 2-й и 3-й индекс;
			{	//при отрисовке opengl делает это за нас, но при расчете нормалей нам нужно это сделать самостоятельно
				face.x = indices_vec.at(i);
				face.y = indices_vec.at(i + 2);
				face.z = indices_vec.at(i + 1);

				currFace++;
			}
			faces.push_back(face);
		}
	}


	///////////////////////
	//расчет нормалей
	for (int i = 0; i < faces.size(); ++i)
	{
		//получаем из вектора вершин координаты каждой из вершин одного треугольника
		float3 A(vertices_vec.at(3 * faces.at(i).x + 0), vertices_vec.at(3 * faces.at(i).x + 1), vertices_vec.at(3 * faces.at(i).x + 2));
		float3 B(vertices_vec.at(3 * faces.at(i).y + 0), vertices_vec.at(3 * faces.at(i).y + 1), vertices_vec.at(3 * faces.at(i).y + 2));
		float3 C(vertices_vec.at(3 * faces.at(i).z + 0), vertices_vec.at(3 * faces.at(i).z + 1), vertices_vec.at(3 * faces.at(i).z + 2));

		//получаем векторы для ребер треугольника из каждой из 3-х вершин
		float3 edge1A(normalize(B - A));
		float3 edge2A(normalize(C - A));

		float3 edge1B(normalize(A - B));
		float3 edge2B(normalize(C - B));

		float3 edge1C(normalize(A - C));
		float3 edge2C(normalize(B - C));

		//нормаль к треугольнику - векторное произведение любой пары векторов из одной вершины
		float3 face_normal = cross(edge1A, edge2A);

		//простой подход: нормаль к вершине = средняя по треугольникам, к которым принадлежит вершина
		normals_vec_tmp.at(faces.at(i).x) += face_normal;
		normals_vec_tmp.at(faces.at(i).y) += face_normal;
		normals_vec_tmp.at(faces.at(i).z) += face_normal;
	}

	//нормализуем векторы нормалей и записываем их в вектор из GLFloat, который будет передан в шейдерную программу
	for (int i = 0; i < normals_vec_tmp.size(); ++i)
	{
		float3 N = normalize(normals_vec_tmp.at(i));

		normals_vec.push_back(N.x);
		normals_vec.push_back(N.y);
		normals_vec.push_back(N.z);
	}


	GLuint vboVertices, vboIndices, vboNormals, vboTexCoords;

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vboVertices);
	glGenBuffers(1, &vboIndices);
	glGenBuffers(1, &vboNormals);
	glGenBuffers(1, &vboTexCoords);


	glBindVertexArray(vao); GL_CHECK_ERRORS;
	{

		//передаем в шейдерную программу атрибут координат вершин
		glBindBuffer(GL_ARRAY_BUFFER, vboVertices); GL_CHECK_ERRORS;
		glBufferData(GL_ARRAY_BUFFER, vertices_vec.size() * sizeof(GL_FLOAT), &vertices_vec[0], GL_STATIC_DRAW); GL_CHECK_ERRORS;
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), (GLvoid*)0); GL_CHECK_ERRORS;
		glEnableVertexAttribArray(0); GL_CHECK_ERRORS;

		//передаем в шейдерную программу атрибут нормалей
		glBindBuffer(GL_ARRAY_BUFFER, vboNormals); GL_CHECK_ERRORS;
		glBufferData(GL_ARRAY_BUFFER, normals_vec.size() * sizeof(GL_FLOAT), &normals_vec[0], GL_STATIC_DRAW); GL_CHECK_ERRORS;
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), (GLvoid*)0); GL_CHECK_ERRORS;
		glEnableVertexAttribArray(1); GL_CHECK_ERRORS;

		//передаем в шейдерную программу атрибут текстурных координат
		glBindBuffer(GL_ARRAY_BUFFER, vboTexCoords); GL_CHECK_ERRORS;
		glBufferData(GL_ARRAY_BUFFER, texcoords_vec.size() * sizeof(GL_FLOAT), &texcoords_vec[0], GL_STATIC_DRAW); GL_CHECK_ERRORS;
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GL_FLOAT), (GLvoid*)0); GL_CHECK_ERRORS;
		glEnableVertexAttribArray(2); GL_CHECK_ERRORS;

		//передаем в шейдерную программу индексы
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices); GL_CHECK_ERRORS;
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_vec.size() * sizeof(GLuint), &indices_vec[0], GL_STATIC_DRAW); GL_CHECK_ERRORS;

		glEnable(GL_PRIMITIVE_RESTART); GL_CHECK_ERRORS;
		glPrimitiveRestartIndex(primRestart); GL_CHECK_ERRORS;
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);


	return numIndices;
}


int initGL()
{
	int res = 0;

	//грузим функции opengl через glad
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize OpenGL context" << std::endl;
		return -1;
	}

	//выводим в консоль некоторую информацию о драйвере и контексте opengl
	std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

	std::cout << "Controls: " << std::endl;
	std::cout << "press left mose button to capture/release mouse cursor  " << std::endl;
	std::cout << "press spacebar to alternate between shaded wireframe and fill display modes" << std::endl;
	std::cout << "press ESC to exit" << std::endl;

	return 0;
}

int main(int argc, char** argv)
{
	if (!glfwInit())
		return -1;

	//запрашиваем контекст opengl версии 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);


	GLFWwindow*  window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL basic sample", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	//регистрируем коллбеки для обработки сообщений от пользователя - клавиатура, мышь..
	glfwSetKeyCallback        (window, OnKeyboardPressed);
	glfwSetCursorPosCallback  (window, OnMouseMove);
	glfwSetMouseButtonCallback(window, OnMouseButtonClicked);
	glfwSetScrollCallback     (window, OnMouseScroll);
	glfwSetInputMode          (window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (initGL() != 0)
		return -1;

	//Reset any OpenGL errors which could be present for some reason
	GLenum gl_error = glGetError();
	while (gl_error != GL_NO_ERROR)
		gl_error = glGetError();

	//создание шейдерной программы из двух файлов с исходниками шейдеров
	//используется класс-обертка ShaderProgram
	std::unordered_map<GLenum, std::string> shaders;
	shaders[GL_VERTEX_SHADER]   = "vertex.glsl";
	shaders[GL_FRAGMENT_SHADER] = "fragment.glsl";
	ShaderProgram program(shaders); GL_CHECK_ERRORS;
	
	std::unordered_map<GLenum, std::string> shaders2;
	shaders2[GL_VERTEX_SHADER]   = "vertex_water.glsl";
	shaders2[GL_FRAGMENT_SHADER] = "water.glsl";
	ShaderProgram program2(shaders2); GL_CHECK_ERRORS;


	//Создаем и загружаем геометрию поверхности
	GLuint vaoTriStrip;
	int triStripIndices = createTriStrip(res, res, sz, vaoTriStrip, SOIL);
	GLuint vaoTriStrip2;
	int triStripIndices2 = createTriStrip(res, res, sz + 2.0 * (float(sz) / 100), vaoTriStrip2, WATER);


	glViewport(0, 0, WIDTH, HEIGHT);  GL_CHECK_ERRORS;
	glEnable(GL_DEPTH_TEST);  GL_CHECK_ERRORS;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	
	//текстурки
	GLuint texture1, texture2;
	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D, texture1);
	int width, height;
	unsigned char* image = SOIL_load_image("textures/img4.jpg", &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);
	glGenTextures(1, &texture2);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture2);
	glGenTextures(1, &texture2);
	glBindTexture(GL_TEXTURE_2D, texture2);
	image = SOIL_load_image("textures/img2.jpg", &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);

	//прозрачность
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// //цикл обработки сообщений и отрисовки сцены каждый кадр

	while (!glfwWindowShouldClose(window))
	{
		//считаем сколько времени прошло за кадр
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwPollEvents();
		doCameraMovement(camera, deltaTime);



		//очищаем экран каждый кадр
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f); GL_CHECK_ERRORS;
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); GL_CHECK_ERRORS;

		program.StartUseShader(); GL_CHECK_ERRORS;

		//обновляем матрицы камеры и проекции каждый кадр
		float4x4 view       = camera.GetViewMatrix();
		float4x4 projection = projectionMatrixTransposed(camera.zoom, float(WIDTH) / float(HEIGHT), 0.1f, 1000.0f);

		//модельная матрица, определяющая положение объекта в мировом пространстве
		float4x4 model; //начинаем с единичной матрицы

		//простой свет
		gorit = glfwGetTime();
		//для волн сдвиг
		shift = sin(gorit) * (float(sz) / 100); 

		program.StartUseShader();

		
		//printf("%f\n", gorit);
		//загружаем uniform-переменные в шейдерную программу (одинаковые для всех параллельно запускаемых копий шейдера)
		program.SetUniform("view",       view);       GL_CHECK_ERRORS;
		program.SetUniform("projection", projection); GL_CHECK_ERRORS;
		program.SetUniform("model",      model);
		program.SetUniform("mode1",      mode1);
		program.SetUniform("fog",        fog);
		program.SetUniform("gorit",      gorit);
		program.SetUniform("fog_act",    fog_act);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture1);
		program.SetUniform("Texture1", 0);

		//рисуем плоскость
		glBindVertexArray(vaoTriStrip);
		glDrawElements(GL_TRIANGLE_STRIP, triStripIndices, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;
		
		program2.StartUseShader(); GL_CHECK_ERRORS;

		program2.SetUniform("view",       view);       GL_CHECK_ERRORS;
		program2.SetUniform("projection", projection); GL_CHECK_ERRORS;
		program2.SetUniform("model",      model);
		program2.SetUniform("mode1",      mode1);
		program2.SetUniform("fog",        fog);
		program2.SetUniform("gorit",      gorit);
		program2.SetUniform("fog_act",    fog_act);
		program2.SetUniform("shift",      shift);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture2);
		program2.SetUniform("Texture2", 1);

		glBindVertexArray(vaoTriStrip2);
		glDrawElements(GL_TRIANGLE_STRIP, triStripIndices2, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;
		glBindVertexArray(0); GL_CHECK_ERRORS;
		program2.StopUseShader();
		

		glfwSwapBuffers(window);
	}

	//очищаем vao перед закрытием программы
	glDeleteVertexArrays(1, &vaoTriStrip);
	glDeleteVertexArrays(1, &vaoTriStrip2);

	glfwTerminate();
	return 0;
}
