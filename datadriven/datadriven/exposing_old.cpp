#include "exposing.h"

using namespace std;

Exposer globalexposer;
TwBar* exposureBar;

void Exposer::print(int level)
{
	for (const auto& pair : exposedvars) {
		for (int i = 0; i < level; ++i) {
			cout << "---";
		}
		cout << ' ' << pair.first;

		ExposedT<Exposer>* e = dynamic_cast<ExposedT<Exposer>*>(pair.second);

		if (e) {
			cout << " :" << endl;
			e->target->print(level + 1);
		}
		else {
			cout << " = " << pair.second->value() << endl;
		}
	}
}
