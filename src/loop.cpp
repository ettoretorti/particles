#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include <mathfu/glsl_mappings.h>

#include <iostream>
#include <array>
#include <cassert>
#include <vector>
#include <utility>
#include <memory>

#include "PSys.hpp"

constexpr GLuint POS_ATTR_IDX = 0;
constexpr GLuint COL_ATTR_IDX = 1;
constexpr GLuint NRM_ATTR_IDX = 2;
constexpr GLuint SPX_ATTR_IDX = 3;

constexpr GLuint MVP_UNIF_IDX = 0;

constexpr float CUBE_SIZE = 3.0;

static constexpr double __t = 1.61803398875;

const std::array<mathfu::vec3_packed, 12> ISO_VERTS = {
	mathfu::vec3_packed(mathfu::vec3(-1.,  __t, 0.)),
	mathfu::vec3_packed(mathfu::vec3( 1.,  __t, 0.)),
	mathfu::vec3_packed(mathfu::vec3(-1., -__t, 0.)),
	mathfu::vec3_packed(mathfu::vec3( 1., -__t, 0.)),
	
	mathfu::vec3_packed(mathfu::vec3(0., -1.,  __t)),
	mathfu::vec3_packed(mathfu::vec3(0.,  1.,  __t)),
	mathfu::vec3_packed(mathfu::vec3(0., -1., -__t)),
	mathfu::vec3_packed(mathfu::vec3(0.,  1., -__t)),

	mathfu::vec3_packed(mathfu::vec3( __t, 0., -1.)),
	mathfu::vec3_packed(mathfu::vec3( __t, 0.,  1.)),
	mathfu::vec3_packed(mathfu::vec3(-__t, 0., -1.)),
	mathfu::vec3_packed(mathfu::vec3(-__t, 0.,  1.)),
	
};

const std::array<mathfu::vec3_packed, 12> ISO_NORMS = {
	mathfu::vec3_packed(mathfu::vec3(ISO_VERTS[0]).Normalized()),
	mathfu::vec3_packed(mathfu::vec3(ISO_VERTS[1]).Normalized()),
	mathfu::vec3_packed(mathfu::vec3(ISO_VERTS[2]).Normalized()),
	mathfu::vec3_packed(mathfu::vec3(ISO_VERTS[3]).Normalized()),

	mathfu::vec3_packed(mathfu::vec3(ISO_VERTS[4]).Normalized()),
	mathfu::vec3_packed(mathfu::vec3(ISO_VERTS[5]).Normalized()),
	mathfu::vec3_packed(mathfu::vec3(ISO_VERTS[6]).Normalized()),
	mathfu::vec3_packed(mathfu::vec3(ISO_VERTS[7]).Normalized()),

	mathfu::vec3_packed(mathfu::vec3(ISO_VERTS[8]).Normalized()),
	mathfu::vec3_packed(mathfu::vec3(ISO_VERTS[9]).Normalized()),
	mathfu::vec3_packed(mathfu::vec3(ISO_VERTS[10]).Normalized()),
	mathfu::vec3_packed(mathfu::vec3(ISO_VERTS[11]).Normalized()),
};

const std::array<GLushort, 20*3> ISO_IDXS = {
	 0, 11,  5,
	 0,  5,  1,
	 0,  1,  7,
	 0,  7, 10,
	 0, 10, 11,

	 1,  5,  9,
	 5, 11,  4,
	11, 10,  2,
        10,  7,  6,
	 7,  1,  8,

	 3,  9,  4,
	 3,  4,  2,
	 3,  2,  6,
	 3,  6,  8,
	 3,  8,  9,

	 4,  9,  5,
	 2,  4, 11,
	 6,  2, 10,
	 8,  6,  7,
	 9,  8,  1,
};

const std::array<mathfu::vec3_packed, 8> CUBE_VERTS = {
	mathfu::vec3_packed(mathfu::vec3(-1.0, -1.0, -1.0)),
	mathfu::vec3_packed(mathfu::vec3(-1.0, -1.0,  1.0)),
	mathfu::vec3_packed(mathfu::vec3(-1.0,  1.0, -1.0)),
	mathfu::vec3_packed(mathfu::vec3(-1.0,  1.0,  1.0)),
	mathfu::vec3_packed(mathfu::vec3( 1.0, -1.0, -1.0)),
	mathfu::vec3_packed(mathfu::vec3( 1.0, -1.0,  1.0)),
	mathfu::vec3_packed(mathfu::vec3( 1.0,  1.0, -1.0)),
	mathfu::vec3_packed(mathfu::vec3( 1.0,  1.0,  1.0)),
};

const std::array<GLushort, 6*3*2> CUBE_IDXS = {
	0, 4, 6,
	6, 2, 0,

	4, 5, 6,
	7, 6, 5,

	5, 1, 3,
	3, 7, 5,
	
	0, 2, 1,
	2, 3, 1,

	2, 6, 3,
	6, 7, 3,

	1, 5, 4,
	4, 0, 1,
};

const std::array<GLushort, 12*2> CUBE_LINE_IDXS = {
	0, 1,
	0, 2,
	0, 4,
	1, 3,
	1, 5,
	2, 3,
	2, 6,
	3, 7,
	4, 5,
	4, 6,
	5, 7,
	6, 7,
};

const GLchar* regVshSrc = 
	"#version 450\n"
	"\n"
	"layout(location = 0) uniform mat4 uMVP;\n"
	"\n"
	"layout(location = 0) in vec3 aPos;\n"
	"layout(location = 1) in vec4 aColor;\n"
	"layout(location = 2) in vec3 aNorm;\n"
	"\n"
	"out vec3 fPos;\n"
	"out vec4 fColor;\n"
	"out vec3 fNorm;\n"
	"\n"
	"void main() {\n"
	"    gl_Position = uMVP * vec4(aPos, 1.0);\n"
	"    fPos = aPos;\n"
	"    fColor = aColor;\n"
	"    fNorm = aNorm;\n"
	"}\n";

const GLchar* isoVshSrc =
	"#version 450\n"
	"\n"
	"layout(location = 0) uniform mat4 uMVP;\n"
	"\n"
	"layout(location = 0) in vec3 aPos;\n"
	"layout(location = 1) in vec4 aColor;\n"
	"layout(location = 2) in vec3 aNorm;\n"
	"layout(location = 3) in vec4 aOffsetScale;\n"
	"\n"
	"out vec3 fPos;\n"
	"out vec4 fColor;\n"
	"out vec3 fNorm;\n"
	"\n"
	"void main() {\n"
	"    gl_Position = uMVP * vec4(aOffsetScale.w * aPos + aOffsetScale.xyz, 1.0);\n"
	"    fPos = aPos;\n"
	"    fColor = aColor;\n"
	"    fNorm = aNorm;\n"
	"}\n";

const GLchar* regFshSrc =
	"#version 450\n"
	"\n"
	"in vec4 fColor;\n"
	"in vec3 fNorm;\n"
	"\n"
	"layout(location = 0) out vec4 oColor;\n"
	"\n"
	"void main() {\n"
	"    oColor = fColor;\n"
	"}\n";


std::pair<std::vector<mathfu::vec3_packed>, std::vector<mathfu::vec3_packed>> uvSphere(size_t meridians, size_t parallels) {
	//TODO
	std::vector<mathfu::vec3_packed> verts;
	std::vector<mathfu::vec3_packed> norms;

	return std::make_pair(verts, norms);
}

void mainloop(GLFWwindow* window) {
	using namespace mathfu;
	using namespace std;

	// Create buffer for isocahedron
	GLuint isoBuf;
	glCreateBuffers(1, &isoBuf);
	glNamedBufferData(isoBuf, sizeof(ISO_VERTS) + sizeof(ISO_NORMS) + sizeof(ISO_IDXS), nullptr, GL_STATIC_DRAW);
	glNamedBufferSubData(isoBuf, 0, sizeof(ISO_IDXS), &ISO_IDXS);
	glNamedBufferSubData(isoBuf, sizeof(ISO_IDXS), sizeof(ISO_VERTS), &ISO_VERTS);
	glNamedBufferSubData(isoBuf, sizeof(ISO_IDXS) + sizeof(ISO_VERTS), sizeof(ISO_NORMS), &ISO_NORMS);

	// Create buffer for arena cube
	GLuint cubeBuf;
	glCreateBuffers(1, &cubeBuf);
	glNamedBufferData(cubeBuf, sizeof(CUBE_VERTS) + sizeof(CUBE_IDXS) + sizeof(CUBE_LINE_IDXS), nullptr, GL_STATIC_DRAW);
	glNamedBufferSubData(cubeBuf, 0, sizeof(CUBE_IDXS), &CUBE_IDXS);
	glNamedBufferSubData(cubeBuf, sizeof(CUBE_IDXS), sizeof(CUBE_VERTS), &CUBE_VERTS);
	glNamedBufferSubData(cubeBuf, sizeof(CUBE_IDXS) + sizeof(CUBE_VERTS), sizeof(CUBE_LINE_IDXS), &CUBE_LINE_IDXS);


	// Create buffer for tier 1 particles
	GLuint t1Buf;
	glCreateBuffers(1, &t1Buf);

	// Compile shaders and link programs
	GLuint regProg = glCreateProgram();
	GLuint isoProg = glCreateProgram();
	{
		GLint ok;

		GLuint vsh = glCreateShader(GL_VERTEX_SHADER);
		GLuint fsh = glCreateShader(GL_FRAGMENT_SHADER);

		glShaderSource(vsh, 1, &regVshSrc, nullptr);
		glCompileShader(vsh);
		glGetShaderiv(vsh, GL_COMPILE_STATUS, &ok);
		assert(ok);

		glShaderSource(fsh, 1, &regFshSrc, nullptr);
		glCompileShader(fsh);
		glGetShaderiv(fsh, GL_COMPILE_STATUS, &ok);
		assert(ok);

		glAttachShader(regProg, vsh);
		glAttachShader(regProg, fsh);
		glLinkProgram(regProg);
		glGetProgramiv(regProg, GL_LINK_STATUS, &ok);
		assert(ok);

		glDeleteShader(vsh);
		vsh = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vsh, 1, &isoVshSrc, nullptr);
		glCompileShader(vsh);
		glGetShaderiv(vsh, GL_COMPILE_STATUS, &ok);
		assert(ok);

		glAttachShader(isoProg, vsh);
		glAttachShader(isoProg, fsh);
		glLinkProgram(isoProg);
		glGetProgramiv(isoProg, GL_LINK_STATUS, &ok);
		assert(ok);

		glDeleteShader(vsh);
		glDeleteShader(fsh);
	}

	GLuint vao;
	glCreateVertexArrays(1, &vao);
	glBindVertexArray(vao);

	mat4 view = mat4::LookAt(vec3(0.), vec3(5., 5., 5.), vec3(0., 1., 0.), 1.0);
	mat4 proj = mat4::Perspective(1.309, 640.0/480, 0.5, 50.0);

	glEnable(GL_DEPTH_TEST);

	vec2i fbSize(0, 0);

	PSys pSys(PSysParams(15.0, 0.05, CUBE_SIZE));
	PSysParams& params = pSys.parameters();
	
	params.restituition = 0.9;
	params.gravity = vec3(0.0, -9.8, 0.0);
	params.initColor = vec4(1.0, 0.0, 0.0, 0.0);
	params.deadColor = vec4(0.0, 0.0, 1.0, 0.0);
	
	params.position = vec3(-1.0, 0.0, 0.0);
	params.velocity = vec3(4.0, 0.0, 0.1);
	pSys.spawn();
	params.position = vec3(1.0, 0.0, 0.0);
	params.velocity = vec3(-4.0, 0.0, 0.0);
	pSys.spawn();

	params.velocity = vec3(0.0);
	params.velocity = vec3(-8.0, 5.0, 1.5);

	double curTime = glfwGetTime();
	double spawnRate = 20.0;
	double nextSpawn = curTime += 1.0/spawnRate;

	double ang1 = 0.0;
	double ang2 = 0.0;
	float cDist = 10.0;

	while(!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		
		// Time updates
		double elapsedTime;
		{
			double now = glfwGetTime();
			elapsedTime = now - curTime;
			curTime = now;
		}

		if(glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
			cout << "spawn rate is " << spawnRate << endl;
			cout << "gravity is " << params.gravity.y() << endl;
			cout << "restituition is " << params.restituition << endl;
		}
		if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			ang1 -= 1.5 * elapsedTime;
		}
		if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			ang1 += 1.5 * elapsedTime;
		}
		
		if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			ang2 -= 1.5 * elapsedTime;
		}
		if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			ang2 += 1.5 * elapsedTime;
		}
		if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
			cDist -= 4.0 * elapsedTime;
		}
		if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
			cDist += 4.0 * elapsedTime;
		}
		if(glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
			params.gravity += float(elapsedTime) * vec3(0.0, 8.0, 0.0);
		}
		if(glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
			params.gravity -= float(elapsedTime) * vec3(0.0, 8.0, 0.0);
		}
		if(glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
			spawnRate += elapsedTime * 2.0;
		}
		if(glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
			spawnRate -= elapsedTime * 2.0;
		}
		if(glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
			params.restituition += elapsedTime * 0.25;
			params.restituition = std::min(1.0f, params.restituition);
		}
		if(glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
			params.restituition -= elapsedTime * 0.25;
			params.restituition = std::max(0.01f, params.restituition);
		}
		
		if(glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) {
			params.velocity = quat::FromAngleAxis(2.01 * elapsedTime, vec3(0.0, 1.0, 0.0)) * params.velocity;
		}
		if(glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {
			params.velocity = quat::FromAngleAxis(-2.01 * elapsedTime, vec3(0.0, 1.0, 0.0)) * params.velocity;
		}

		if(glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) {
			params.velocity = quat::FromAngleAxis(2.01 * elapsedTime, vec3(0.0, 0.0, 1.0)) * params.velocity;
		}
		if(glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
			params.velocity = quat::FromAngleAxis(-2.01 * elapsedTime, vec3(0.0, 0.0, 1.0)) * params.velocity;
		}


		vec3 cPos = cDist * vec3(8./5., 1., 0.);
		cPos = quat::FromAngleAxis(ang1, vec3(0.0, 1.0, 0.0)) * quat::FromAngleAxis(-ang2, vec3(0.0, 0.0, 1.0)) *  cPos;
		view = mat4::LookAt(vec3(0.), cPos, vec3(0., 1., 0.), 1.0);


		if(curTime > nextSpawn) {
			pSys.spawn();
			nextSpawn += 1.0/spawnRate;
		}

		pSys.step(elapsedTime);

		// Framebuffer resizing
		{
			vec2i newSize;
			glfwGetFramebufferSize(window, &newSize.x(), &newSize.y());

			if(newSize != fbSize) {
				fbSize = newSize;
				proj = mat4::Perspective(1.309, float(fbSize.x())/fbSize.y(), 0.5, 50.0);
				glViewport(0, 0, fbSize.x(), fbSize.y());
			}
		}
		
		// Clear
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		// Render tier 1 particles
		if(pSys.size() > 0) {
			struct Particle {
				vec4_packed posAndScale;
				vec4_packed color;
			};

			glNamedBufferData(t1Buf, pSys.size() * sizeof(Particle), nullptr, GL_STREAM_DRAW);
			Particle* buf = (Particle*) glMapNamedBuffer(t1Buf, GL_WRITE_ONLY);

			for(size_t i = 0; i < pSys.size(); i++) {
				buf[i].posAndScale = vec4(pSys.positions()[i].x(), pSys.positions()[i].y(), pSys.positions()[i].z(), params.radius);
				buf[i].color = pSys.colors()[i];
			}

			glUnmapNamedBuffer(t1Buf);

			glUseProgram(isoProg);
			glUniformMatrix4fv(MVP_UNIF_IDX, 1, false, &(proj * view)[0]);

			glVertexArrayElementBuffer(vao, isoBuf);

			glBindBuffer(GL_ARRAY_BUFFER, isoBuf);
			glVertexAttribPointer(POS_ATTR_IDX, 3, GL_FLOAT, false, 0, (void*)(sizeof(ISO_IDXS)));
			glEnableVertexArrayAttrib(vao, POS_ATTR_IDX);

			glVertexAttribPointer(NRM_ATTR_IDX, 3, GL_FLOAT, false, 0, (void*)(sizeof(ISO_IDXS) * sizeof(ISO_VERTS)));
			glEnableVertexArrayAttrib(vao, NRM_ATTR_IDX);

			glBindBuffer(GL_ARRAY_BUFFER, t1Buf);
			glVertexAttribPointer(COL_ATTR_IDX, 4, GL_FLOAT, false, sizeof(Particle), (void*)offsetof(Particle, color));
			glEnableVertexArrayAttrib(vao, COL_ATTR_IDX);
			glVertexAttribDivisor(COL_ATTR_IDX, 1);

			glVertexAttribPointer(SPX_ATTR_IDX, 4, GL_FLOAT, false, sizeof(Particle), (void*)offsetof(Particle, posAndScale));
			glEnableVertexArrayAttrib(vao, SPX_ATTR_IDX);
			glVertexAttribDivisor(SPX_ATTR_IDX, 1);


			glEnable(GL_CULL_FACE);

			glDrawElementsInstanced(GL_TRIANGLES, ISO_IDXS.size(), GL_UNSIGNED_SHORT, (void*)0, pSys.size());

			glDisable(GL_CULL_FACE);

			glVertexAttribDivisor(COL_ATTR_IDX, 0);
		}

		// Render cube
		{
			glUseProgram(regProg);

			glVertexArrayElementBuffer(vao, cubeBuf);

			mat4 modl = mat4::FromScaleVector(vec3(CUBE_SIZE, CUBE_SIZE, CUBE_SIZE));

			glUniformMatrix4fv(MVP_UNIF_IDX, 1, false, &(proj * view * modl)[0]);
			glVertexAttrib4f(COL_ATTR_IDX, 0.0, 1.0, 0.0, 0.2);

			glBindBuffer(GL_ARRAY_BUFFER, cubeBuf);
			
			glVertexAttribPointer(POS_ATTR_IDX, 3, GL_FLOAT, false, 0, (void*)sizeof(CUBE_IDXS));
			glEnableVertexArrayAttrib(vao, POS_ATTR_IDX);
			
			glDisableVertexArrayAttrib(vao, COL_ATTR_IDX);
			glDisableVertexArrayAttrib(vao, NRM_ATTR_IDX);
			
			// For the transparency
			glDepthMask(GL_FALSE);
			glEnable(GL_BLEND);
			glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);

			glDrawElements(GL_TRIANGLES, CUBE_IDXS.size(), GL_UNSIGNED_SHORT, (void*)0);
			
			glDisable(GL_BLEND);
			glDepthMask(GL_TRUE);
			
			// Lines n stuff
			glVertexAttrib4f(COL_ATTR_IDX, 0.0, 0.0, 0.0, 1.0);
			glDrawElements(GL_LINES, CUBE_LINE_IDXS.size(), GL_UNSIGNED_SHORT, (void*)(sizeof(CUBE_IDXS) + sizeof(CUBE_VERTS)));
		}

		glfwSwapBuffers(window);
	}
}
