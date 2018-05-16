#ifndef ___INTERPOLATOR_H___
#define ___INTERPOLATOR_H___

template<typename T>
class Interpolator {
public:
	T start;
	T stop;
	T* target;
	float t;
	float dt;
	float tick;

	void Set(const T& start, const T& stop, float duration, int nbSteps);
	void Begin();
	bool Step();
	void Calculate();
};

template<typename T>
inline void Interpolator<T>::Set(const T & start, const T & stop, float duration, int nbSteps)
{
	this->start = start;
	this->stop = stop;
	this->dt = 1.0f / (float)nbSteps;
	this->tick = duration / (float)nbSteps;
}

template<typename T>
inline void Interpolator<T>::Begin()
{
	*target = start;
	t = 0.0f;
}

template<typename T>
inline bool Interpolator<T>::Step()
{
	t += dt;
	Calculate();
	return t >= 1.0f;
}

template<typename T>
inline void Interpolator<T>::Calculate()
{
	*target = start + (stop - start)*t;
}

#endif//___INTERPOLATOR_H___