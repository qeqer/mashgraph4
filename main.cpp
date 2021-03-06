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
static float fog = 180;
static bool fog_act = 0;
static float gorit = 0.0;
static float shift = 0.5;
static bool day_change = true;
static bool change_map = false;
static bool show_arr = false;



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
	
	case GLFW_KEY_N:
		if (action == GLFW_PRESS) {
			show_arr ^=1;
		}
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
	case GLFW_KEY_G:
		if (action == GLFW_PRESS) {
			if (day_change) {
				day_change = false;
			} else {
				day_change = true;
			}
		}
		break;
	case GLFW_KEY_Z:
		if (action == GLFW_PRESS) {
			if (mode1 == 2) {
				mode1 = 1;
			} else {
				mode1 = 2;
			}
		}
		break;
	case GLFW_KEY_C:
		if (action == GLFW_PRESS) {
			change_map = true;
		}
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


//PARAMS
/**
\brief создать triangle strip плоскость и загрузить её в шейдерную программу
\param rows - число строк
\param cols - число столбцов
\param size - размер плоскости
\param vao - vertex array object, связанный с созданной плоскостью
*/

float fl_mod(float d, float a) { //d % a but for float ATTENTION: -10 % 3 = -1!!!!
	d = (d - int(d / a) * a);
	return d;
}

float terrain = 0.4; //hilly
float he = 1; //old param, don't use
int res = 257; //resolution of map
float sz = 200; //size of map
float max_h = 25; //max height of result map
float min_h = -10; //min height of result map
float powing = 2; //pow of landscape norming
float waves = 0.5; //amplitude
float waves_num = 32; //must be a pow of 2, num of waves for one chank
int copies_num = 5; //how far u load chanks

void doCameraMovement(Camera &camera, GLfloat deltaTime)
{
	camera.pos.x = fl_mod(camera.pos.x, sz);
	camera.pos.y = fl_mod(camera.pos.y, sz);
	camera.pos.z = fl_mod(camera.pos.z, sz);
	if (keys[GLFW_KEY_W])
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (keys[GLFW_KEY_A])
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (keys[GLFW_KEY_S])
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (keys[GLFW_KEY_D])
		camera.ProcessKeyboard(RIGHT, deltaTime);
}





void filldiam(std::vector<std::vector<GLfloat> > &strip, int rowl, int rowr, int coll, int colr, int rows, int cols, float size) {
	int up = rowl >= 0 ? rowl : rows - 1 + rowl;
	int down = rowr < rows ? rowr : rowr - rows + 1;
	int left = coll >= 0 ? coll : cols - 1 + coll;
	int right = colr < cols ? colr : colr - cols + 1;
	float diff = terrain * (float(rand()) / RAND_MAX - 0.5) * (size / rows * (rowl - rowr));
	if ((rowl + rowr == 0 || rowl + rowr == rows - 1) || (coll + colr == 0 || coll + colr == cols - 1)) {
		diff = 0;
	}
	strip[(rowr + rowl) / 2][(colr + coll) / 2] = (
	            strip[up][(coll + colr) / 2] +
	            strip[down][(coll + colr) / 2] +
	            strip[(rowl + rowr) / 2][left] +
	            strip[(rowl + rowr) / 2][right]
	        ) / 4;// + terrain * (float(rand()) / RAND_MAX - 0.5) * (size / rows * (rowl - rowr));
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

	float tre = strip.size();
	for (uint i = 1; i < tre - 1; i++) {
		strip[i][0] = (strip[i - 1][0] + strip[i][0] + strip[i + 1][0]) / 3;
		strip[i][tre - 1] = (strip[i - 1][tre - 1] + strip[i][tre - 1] + strip[i + 1][tre - 1]) / 3;
		strip[0][i] = (strip[0][i - 1] + strip[0][i] + strip[0][i + 1]) / 3;
		strip[tre - 1][i] = (strip[tre - 1][i - 1] + strip[tre - 1][i] + strip[tre - 1][i + 1]) / 3;
	}

	return;
}

void makewater(std::vector<std::vector<GLfloat> > &strip, float size) { //makes sinusoidal water waves
	for (uint i = 0; i < strip.size(); i++) {
		for (uint j = 0; j < strip[0].size(); j++) {
			strip[i][j] = 0.1 + waves * sin(M_PI *  i * waves_num / (strip.size() - 1));
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
			float xx = -size / 2 + x * size / (cols - 1);
			float zz = -size / 2 + z * size / (rows - 1);
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
	
	std::unordered_map<GLenum, std::string> shaders3;
	shaders3[GL_VERTEX_SHADER]   = "vertex_sky.glsl";
	shaders3[GL_FRAGMENT_SHADER] = "fragment_sky.glsl";
	ShaderProgram program3(shaders3); GL_CHECK_ERRORS;
	
	std::unordered_map<GLenum, std::string> shaders4;
	shaders4[GL_VERTEX_SHADER]   = "vertex_normals.glsl";
	shaders4[GL_FRAGMENT_SHADER] = "fragment_normals.glsl";
	shaders4[GL_GEOMETRY_SHADER] = "geometry_normals.glsl";
	ShaderProgram program4(shaders4); GL_CHECK_ERRORS;

	//Создаем и загружаем геометрию поверхности
	GLuint vaoTriStrip;
	int triStripIndices = createTriStrip(res, res, sz, vaoTriStrip, SOIL);
	GLuint vaoTriStrip2;
	int triStripIndices2 = createTriStrip(res, res, sz, vaoTriStrip2, WATER);



	//skybox
	float cub = 400.0;
	GLfloat vertices[] = {
		cub, cub, cub,
		-cub, cub, cub,
		-cub, -cub, cub,
		cub, -cub, cub,
		cub, cub, -cub,
		-cub, cub, -cub,
		-cub, -cub, -cub,
		cub, -cub, -cub,


	};
	GLuint indices[] = {  // Note that we start from 0!
		0, 1, 3,  // First Triangle
		1, 2, 3,   // Second Triangle
		0, 1, 5,
		0, 4, 5,
		5, 4, 7,
		6, 7, 5,
		6, 7, 3,
		6, 2, 3,
		1, 6, 2,
		6, 5, 1,
		7, 4, 0,
		7, 3, 0
	};
	GLuint VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	// Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0); // Note that this is allowed, the call to glVertexAttribPointer registered VBO as the currently bound vertex buffer object so afterwards we can safely unbind

	glBindVertexArray(0); // Unbind

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
	image = SOIL_load_image("textures/img1.jpg", &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);

	//for sky
	GLuint texture3, texture4;

	glActiveTexture(GL_TEXTURE2);
	glGenTextures(1, &texture3);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture3);GL_CHECK_ERRORS;
	
	image = SOIL_load_image("textures/skyday.png", &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image); GL_CHECK_ERRORS;
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture3);GL_CHECK_ERRORS;
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image); GL_CHECK_ERRORS;
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture3);GL_CHECK_ERRORS;
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image); GL_CHECK_ERRORS;

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);GL_CHECK_ERRORS;
	SOIL_free_image_data(image);

	glActiveTexture(GL_TEXTURE3);
	glGenTextures(1, &texture4);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture4);GL_CHECK_ERRORS;
	
	image = SOIL_load_image("textures/skynight.png", &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image); GL_CHECK_ERRORS;
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image); GL_CHECK_ERRORS;
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image); GL_CHECK_ERRORS;

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);GL_CHECK_ERRORS;
	SOIL_free_image_data(image);

	//прозрачность
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	// //цикл обработки сообщений и отрисовки сцены каждый кадр
	while (!glfwWindowShouldClose(window))
	{
		if (change_map) {
			triStripIndices = createTriStrip(res, res, sz, vaoTriStrip, SOIL);
			change_map = false;
		}
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

		//простой свет
		if (day_change) {
			gorit = glfwGetTime();
		} else {
			gorit = 8;
		}
		//для волн сдвиг
		//shift = sin(glfwGetTime()) * (float(sz) / 100);
		shift = fl_mod(glfwGetTime(), 16.0 * 200 / 256);

		program.StartUseShader();


		//printf("%f\n", gorit);
		//загружаем uniform-переменные в шейдерную программу (одинаковые для всех параллельно запускаемых копий шейдера)
		program.SetUniform("view",       view);       GL_CHECK_ERRORS;
		program.SetUniform("projection", projection); GL_CHECK_ERRORS;
		program.SetUniform("mode1",      mode1);
		program.SetUniform("fog",        fog);
		program.SetUniform("gorit",      gorit);
		program.SetUniform("fog_act",    fog_act);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture1);
		program.SetUniform("Texture1", 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture2);
		program.SetUniform("Texture2", 1);
		//рисуем плоскость
		float4x4 temp;
		glBindVertexArray(vaoTriStrip);
		for (int i = -copies_num / 2; i <= copies_num / 2; i++) {
			for (int j = -copies_num / 2; j <= copies_num / 2; j++) {
				temp = transpose(translate4x4(float3(float(i) * 200, 0.0, float(j) * 200)));
				program.SetUniform("model",      temp);
				glDrawElements(GL_TRIANGLE_STRIP, triStripIndices, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;
			}
		}
		float4x4 model; //начинаем с единичной матрицы
		if (show_arr) {
			program4.StartUseShader();
			for (int i = -copies_num / 2; i <= copies_num / 2; i++) {
				for (int j = -copies_num / 2; j <= copies_num / 2; j++) {
					temp = transpose(translate4x4(float3(float(i) * 200, 0.0, float(j) * 200)));
					program4.SetUniform("view",       view);       GL_CHECK_ERRORS;
					program4.SetUniform("projection", projection); GL_CHECK_ERRORS;
					program4.SetUniform("model",      temp); GL_CHECK_ERRORS;
					glDrawElements(GL_TRIANGLES, triStripIndices, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;
				}
			}
			program4.StopUseShader();
		}

		program2.StartUseShader(); GL_CHECK_ERRORS;

		program2.SetUniform("view",       view);       GL_CHECK_ERRORS;
		program2.SetUniform("projection", projection); GL_CHECK_ERRORS;
		program2.SetUniform("mode1",      mode1);
		program2.SetUniform("fog",        fog);
		program2.SetUniform("gorit",      gorit);
		program2.SetUniform("fog_act",    fog_act);
		program2.SetUniform("shift",      shift);



		glBindVertexArray(vaoTriStrip2);
		for (int i = -copies_num / 2; i <= copies_num / 2; i++) {
			for (int j = -copies_num / 2; j <= copies_num / 2; j++) {
				float4x4 temp = transpose(translate4x4(float3(float(i) * 200, 0.0, float(j) * 200)));
				program2.SetUniform("model",      temp);
				glDrawElements(GL_TRIANGLE_STRIP, triStripIndices2, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;
			}
		}

		glDepthMask(GL_FALSE);
		

		program3.StartUseShader();
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_CUBE_MAP, texture3);
		program3.SetUniform("Texture3", 2);
		
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_CUBE_MAP, texture4);
		program3.SetUniform("Texture4", 3);

		program3.SetUniform("view",       view);       GL_CHECK_ERRORS;
		program3.SetUniform("projection", projection); GL_CHECK_ERRORS;
		program3.SetUniform("model",      model);GL_CHECK_ERRORS;
		program3.SetUniform("cam",      camera.pos);GL_CHECK_ERRORS;
		program3.SetUniform("fog",        fog);GL_CHECK_ERRORS;
		program3.SetUniform("gorit",      gorit);GL_CHECK_ERRORS;
		program3.SetUniform("fog_act",    fog_act);GL_CHECK_ERRORS;


		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		glDepthMask(GL_TRUE);
		program3.StopUseShader();
		
		


		glfwSwapBuffers(window);
	}

	//очищаем vao перед закрытием программы
	glDeleteVertexArrays(1, &vaoTriStrip);
	glDeleteVertexArrays(1, &vaoTriStrip2);

	glfwTerminate();
	return 0;
}
