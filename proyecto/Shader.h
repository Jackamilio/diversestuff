#ifndef ___SHADER_H___
#define ___SHADER_H___

#include <map>
#include <string>
#include <vector>
#include <allegro5/allegro.h>
#include <allegro5/allegro_opengl.h>
#include <glm/glm.hpp>
#include "ResourceManager.h"

class Shader {
private:
	GLuint id;
	bool needsToLoad;
public:
	Shader() : id(0), needsToLoad(true) {}
	~Shader() { if (id) { glDeleteShader(id); id = 0; } }
	void Load(const std::string& file);
	inline GLuint GetValue() const { return id; }
	inline bool NeedsToLoad() const { return needsToLoad; }
};

typedef ResourceManager<Shader, GLuint> ShaderManager;

class Uniform {
private:
	GLint loc;
	bool needsToLoad;
public:
	Uniform() : loc(-1), needsToLoad(true) {}
	~Uniform() {}
	void Load(const std::string& name, GLuint programID) { loc = glGetUniformLocation(programID, name.c_str()); needsToLoad = false; }
	inline GLint GetValue() const { return loc; }
	inline bool NeedsToLoad() const { return needsToLoad; }
};

class Program : ResourceManager<Uniform, GLint> {
	friend class ProgramManager;
	friend class ResourceManager<Program, GLuint>;
private:
	GLuint id;
	bool needsToLoad;

	void Load(const std::string& file, ShaderManager& shaderManager);
	inline bool NeedsToLoad() const { return needsToLoad; }
	inline GLuint GetValue() const { return id; }
	inline void Use() const { glUseProgram(id); }
	inline bool operator == (const Program& compare) const { return id == compare.id; }
public:
	Program() : id(0), needsToLoad(true) {};
	Program(const Program& rhs) : id(rhs.id), needsToLoad(rhs.needsToLoad) {}
	~Program() { if (id) { glDeleteProgram(id); id = 0; } }

	inline GLint GetUniform(const std::string& name) { return GetHandler(name, id).GetValue(); }

	inline void SetUniform(const std::string& name, int val) { glUniform1i(GetUniform(name), val); }
	inline void SetUniform(const std::string& name, const int* vals, int size) { glUniform1iv(GetUniform(name), size, vals); }
	inline void SetUniform(const std::string& name, const std::vector<int>& vals) { glUniform1iv(GetUniform(name), (GLsizei)vals.size(), (int*)&vals[0]); }

	inline void SetUniform(const std::string& name, float val) { glUniform1f(GetUniform(name), val); }
	inline void SetUniform(const std::string& name, const float* vals, int size) { glUniform1fv(GetUniform(name), size, vals); }
	inline void SetUniform(const std::string& name, const std::vector<float>& vals) { glUniform1fv(GetUniform(name), (GLsizei)vals.size(), (float*)&vals[0]); }

	inline void SetUniform(const std::string& name, const glm::vec3& val) { glUniform3f(GetUniform(name), val.x, val.y, val.z); }
	inline void SetUniform(const std::string& name, const glm::vec3* vals, int size) { glUniform3fv(GetUniform(name), size, (float*)vals); }
	inline void SetUniform(const std::string& name, const std::vector<glm::vec3>& vals) { glUniform3fv(GetUniform(name), (GLsizei)vals.size(), (float*)&vals[0]); }

	inline void SetUniform(const std::string& name, const glm::vec4& val) { glUniform4f(GetUniform(name), val.x, val.y, val.z, val.w); }
	inline void SetUniform(const std::string& name, const glm::vec4* vals, int size) { glUniform4fv(GetUniform(name), size, (float*)vals); }
	inline void SetUniform(const std::string& name, const std::vector<glm::vec4>& vals) { glUniform4fv(GetUniform(name), (GLsizei)vals.size(), (float*)&vals[0]); }

	inline void SetUniform(const std::string& name, const glm::mat4x2& val) { glUniformMatrix4x2fv(GetUniform(name), 1, false, (float*)&val); }
	inline void SetUniform(const std::string& name, const glm::mat4x2* vals, int size) { glUniformMatrix4x2fv(GetUniform(name), size, false, (float*)vals); }
	inline void SetUniform(const std::string& name, const std::vector<glm::mat4x2>& vals) { glUniformMatrix4x2fv(GetUniform(name), (GLsizei)vals.size(), false, (float*)&vals[0][0]); }

	inline void SetUniform(const std::string& name, const glm::mat2x4& val) { glUniformMatrix2x4fv(GetUniform(name), 1, false, (float*)&val); }
	inline void SetUniform(const std::string& name, const glm::mat2x4* vals, int size) { glUniformMatrix2x4fv(GetUniform(name), size, false, (float*)vals); }
	inline void SetUniform(const std::string& name, const std::vector<glm::mat2x4>& vals) { glUniformMatrix2x4fv(GetUniform(name), (GLsizei)vals.size(), false, (float*)&vals[0][0]); }

	inline void SetUniform(const std::string& name, const glm::mat3& val) { glUniformMatrix3fv(GetUniform(name), 1, false, (float*)&val); }
	inline void SetUniform(const std::string& name, const glm::mat3* vals, int size) { glUniformMatrix3fv(GetUniform(name), size, false, (float*)vals); }
	inline void SetUniform(const std::string& name, const std::vector<glm::mat3>& vals) { glUniformMatrix3fv(GetUniform(name), (GLsizei)vals.size(), false, (float*)&vals[0][0]); }

	inline void SetUniform(const std::string& name, const glm::mat4& val) { glUniformMatrix4fv(GetUniform(name), 1, false, (float*)&val); }
	inline void SetUniform(const std::string& name, const glm::mat4* vals, int size) { glUniformMatrix4fv(GetUniform(name), size, false, (float*)vals); }
	inline void SetUniform(const std::string& name, const std::vector<glm::mat4>& vals) { glUniformMatrix4fv(GetUniform(name), (GLsizei)vals.size(), false, (float*)&vals[0][0]); }
};

class ProgramManager : public ResourceManager<Program, GLuint> {
private:
	ShaderManager shaders;
	Program* currentProgram;
public:
	ProgramManager() : currentProgram(0) {}
	inline Program* GetCurrent() { return currentProgram; }
	inline Program& Get(const std::string& file) { return GetHandler(file, shaders); }

	inline void Use(Program* program) {
		program->Use();
		currentProgram = program;
	}

	void Use(const std::string& file) {
		Use(&Get(file));
	}

	void StopToUse() {
		if (currentProgram) {
			glUseProgram(0);
			currentProgram = 0;
		}
	}

	inline void Clear() { StopToUse(); ResourceManager<Program, GLuint>::Clear(); }
};


#endif//___SHADER_H___
