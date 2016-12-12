#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include <iostream>

static void glfwError(int, const char* msg) {
	std::cerr << msg << std::endl;
}

#ifndef NDEBUG
void APIENTRY glDebug(GLenum, GLenum, GLuint, GLenum, GLsizei, const GLchar* msg, const void*) {
	std::cerr << msg << std::endl;
}
#endif

void mainloop(GLFWwindow*);

int main() {
	using std::cerr;
	using std::endl;

	glfwSetErrorCallback(glfwError);

	if(!glfwInit()) {
		cerr << "Could not init GLFW. Shutting down..." << endl;
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
#ifndef NDEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

	GLFWwindow* window = glfwCreateWindow(640, 480, "particles", nullptr, nullptr);
	if(!window) {
		cerr << "Could not create window. Shutting down..." << endl;
		return -1;
	}
	
	glfwMakeContextCurrent(window);
	
	if(gl3wInit()) {
		cerr << "Could not initialize gl3w. Shutting down..." << endl;
		return -1;
	}

#ifndef NDEBUG
	glDebugMessageCallback(glDebug, nullptr);
	glEnable(GL_DEBUG_OUTPUT);
#endif
	
	glfwSwapInterval(-1);

	glClear(GL_COLOR_BUFFER_BIT);
	glfwSwapBuffers(window);

	mainloop(window);
	
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
