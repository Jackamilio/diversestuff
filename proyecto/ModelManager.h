#ifndef ___MODEL_MANAGER_H___
#define ___MODEL_MANAGER_H___

#include "Model.h"

class GraphicContext;

class ModelHandler {
private:
	Model* model;
public:
	ModelHandler() : model(0) {}
	~ModelHandler() { if (model) { delete model; model = 0; } }

	inline const Model& GetValue() const { return *model; }
	inline void Load(const std::string& file, GraphicContext& graphics) { if (!model) { model = new Model(file, graphics); } }
	inline void Load(const MapData* map, GraphicContext& graphics) { if (!model) { model = new Model(*map, graphics); } }
	inline bool NeedsToLoad() const { return model == 0; }
};

class ModelManager : public ResourceManager<ModelHandler, Model>, public ResourceManager<ModelHandler, Model, const MapData*> {
private:
	GraphicContext& graphics;
public:
	ModelManager(GraphicContext& g) : graphics(g) {}
	inline const Model& Get(const std::string& file) { return ResourceManager<ModelHandler, Model, std::string>::GetRefValue(file, graphics); }
	inline const Model& Get(const MapData* level) { return ResourceManager<ModelHandler, Model, const MapData*>::GetRefValue(level, graphics); }
	inline void RemoveValue(const std::string& file) { ResourceManager<ModelHandler, Model, std::string>::RemoveValue(file); }
	inline void RemoveValue(const MapData* level) { ResourceManager<ModelHandler, Model, const MapData*>::RemoveValue(level); }

	void Clear() {
		return ResourceManager<ModelHandler, Model, std::string>::Clear();
		return ResourceManager<ModelHandler, Model, const MapData*>::Clear();
	}
};

#endif//___MODEL_MANAGER_H___