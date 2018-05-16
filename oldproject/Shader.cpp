#include "Shader.h"
#include <iostream>
#include <fstream>
#include <sstream>

void Shader::Load(const std::string& file)
{
	needsToLoad = false;

	// identify type first
	if (file.size() < 2) {
		fprintf(stdout, "Impossible to determine shader type from file '%s'. Shader creation aborted.\n", file.c_str());
		return;
	}
	std::string ftype = file.substr(file.size() - 2, 2);
	GLenum type = 0;
	if (ftype == "vs") {
		type = GL_VERTEX_SHADER;
	}
	else if (ftype == "fs") {
		type = GL_FRAGMENT_SHADER;
	}
	if (!type) {
		fprintf(stdout, "Impossible to determine shader type from file '%s'. Shader creation aborted.\n", file.c_str());
		return;
	}

	// read the file
	std::ifstream f;
	f.open(file, std::ios::in);
	if (f.is_open()) {
		std::stringstream strstr;
		strstr << f.rdbuf();

		id = glCreateShader(type);
		std::string str = strstr.str();
		const char* cstrfile = str.c_str();
		glShaderSource(id, 1, &cstrfile, 0);

		glCompileShader(id);
		GLint status;
		glGetShaderiv(id, GL_COMPILE_STATUS, &status);

		if (status != GL_TRUE) {
			GLint logsize;
			glGetShaderiv(id, GL_INFO_LOG_LENGTH, &logsize);

			char* log = new char[logsize + 1];

			glGetShaderInfoLog(id, logsize, &logsize, log);
			log[logsize] = 0;

			fprintf(stdout, "Shader '%s' failed to compile : %s\n", file.c_str(), log);

			delete[] log;

			glDeleteShader(id);
			id = 0;
		}

		f.close();
	}
	else {
		fprintf(stdout, "Error while opening file '%s'. Shader creation aborted.\n", file.c_str());
	}
}

void Program::Load(const std::string& file, ShaderManager & shaderManager)
{
	needsToLoad = false;

	std::ifstream f;
	f.open(file, std::ios::in);

	if (f.is_open()) {
		std::string word;

		id = glCreateProgram();

		while (!f.eof()) {
			f >> word;

			const Shader& s = shaderManager.GetHandler(word);
			if (s.GetValue()) {
				glAttachShader(id, s.GetValue());
			}
		}

		glLinkProgram(id);
		GLint status;
		glGetProgramiv(id, GL_LINK_STATUS, &status);

		if (status != GL_TRUE) {
			GLint logsize;
			glGetProgramiv(id, GL_INFO_LOG_LENGTH, &logsize);

			char* log = new char[logsize + 1];

			glGetProgramInfoLog(id, logsize, &logsize, log);
			log[logsize] = 0;

			fprintf(stdout, "Program '%s' failed to link : %s\n", file.c_str(), log);

			delete[] log;

			glDeleteProgram(id);
			id = 0;
		}
	}
	else {
		fprintf(stdout, "Error while opening file '%s'. Program creation aborted.\n", file.c_str());
	}
}
