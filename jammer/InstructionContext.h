#ifndef __INSTRUCTION_CONTEXT_H__
#define __INSTRUCTION_CONTEXT_H__

#include "Parameter.h"
#include <unordered_map>
#include <string>

class InstructionContext {
public:
	typedef std::unordered_map<std::string, Parameter> ContextLayer;
private:
	std::vector<ContextLayer*> variables;
	size_t flaggedDepth;

public:
	InstructionContext();

	void PushContext();
	void PopContext();

	void PushContext(InstructionContext& context);
	InstructionContext CutContext(size_t fromdepth);

	inline void FlagCurrentDepth() { flaggedDepth = variables.size(); }
	inline InstructionContext CutContextAtFlaggedDepth() { return CutContext(flaggedDepth); }

	inline bool IsEmpty() const { return variables.empty(); }
	inline size_t CurrentDepth() const { return variables.size(); }

	Parameter& DeclareVariable(const std::string& name);
	Parameter& GetVariable(const std::string& name);

	void Clear();
};

#endif //__INSTRUCTION_CONTEXT_H__