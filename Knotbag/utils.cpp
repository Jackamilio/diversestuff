#include "utils.h"
#include <sys/stat.h>

// yeah thank you stack overflow, this looks fast, even if it's far frome being critical here
// https://stackoverflow.com/questions/12774207/fastest-way-to-check-if-a-file-exists-using-standard-c-c11-14-17-c
bool fileexists(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}