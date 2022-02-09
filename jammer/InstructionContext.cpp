#include "InstructionContext.h"
#include <assert.h>

InstructionContext::InstructionContext()
{
}

void InstructionContext::PushContext()
{
	variables.push_back(new ContextLayer);
}

void InstructionContext::PopContext()
{
	delete variables.back();
	variables.pop_back();
}

void InstructionContext::PushContext(InstructionContext& context)
{
	variables.insert(variables.end(), context.variables.begin(), context.variables.end());
}

InstructionContext InstructionContext::CutContext(size_t fromdepth)
{
	InstructionContext ret;
	ret.variables.insert(ret.variables.end(), variables.begin() + fromdepth, variables.end());
	variables.erase(variables.begin() + fromdepth, variables.end());
	return ret;
}

// Todo : use a friendlier way than asserts to handle non-existing variables
Parameter& InstructionContext::DeclareVariable(const std::string& name)
{
	ContextLayer* backmap = variables.back();
	assert(backmap->find(name) == backmap->end() && "This variable name is already taken in this context!");
	return (*backmap)[name];
}

Parameter& InstructionContext::GetVariable(const std::string& name)
{
	for (auto it = variables.rbegin(); it != variables.rend(); ++it) {
		auto var = (*it)->find(name);
		if (var != (*it)->end()) {
			return var->second;
		}
	}
	assert(false && "Error, no variable with this name was found.");
	static Parameter imforcedtodothisdonthurtme;
	return imforcedtodothisdonthurtme;
}

void InstructionContext::Clear()
{
	for (auto layer : variables) {
		delete layer;
	}
	variables.clear();
}
