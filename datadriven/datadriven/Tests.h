#pragma once

#include "exposing.h"

class Test
{
private:
	int a = 1;
	std::string b = "deux";
	float c = 3.3333f;

public:
	IM_AN_EXPOSER
};

class TestSon : public Test
{
private:
	int d = 48;

public:
	IM_AN_EXPOSER
};

class Another
{
private:
	std::string one = "one";
	std::string two = "two";
	Test yolooo;

public:
	IM_AN_EXPOSER
};

void Test1();