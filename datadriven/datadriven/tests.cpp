#include "Tests.h"

#define EXPOSE_TYPE Test
EXPOSE_START
EXPOSE(a, "min=0 max=100")
EXPOSE(b)
EXPOSE(c)
EXPOSE_END
#undef EXPOSE_TYPE

#define EXPOSE_TYPE TestSon
EXPOSE_START
EXPOSE_PARENT(Test)
EXPOSE(d)
EXPOSE_END
#undef EXPOSE_TYPE

#define EXPOSE_TYPE Another
EXPOSE_START
EXPOSE(one)
EXPOSE(two)
EXPOSE(yolooo)
EXPOSE_END
#undef EXPOSE_TYPE


void Test1() {
	static Test test;
	static TestSon son;
	static Another other;

	EXPOSE_GLOBAL(test);
	EXPOSE_GLOBAL(son);
	EXPOSE_GLOBAL(other);

	//system("PAUSE");
}