#include "OpticalDesign/Brain.h"
#include "OpticalDesign/CVUtil.h"


void RunBrain() {
	optical_design::Brain brain;
	while (!brain.TryToStart());
	size_t i = 1000000;
	auto t = std::chrono::system_clock::now();
	while (i || true) {
		auto now = std::chrono::system_clock::now();
		//if (std::chrono::duration_cast<std::chrono::milliseconds>(now - t).count() < 15)continue;
		t = now;
		brain.Update();
		i--;
	}
	ngs::Allocator::I().Show();
}
#if NGS_PLATFORM == NGS_LINUX
#include "NGS/Embedded/Embedded.h"

#endif
int main() {
	ngs::nos.Log("main", "start\n");
#if NGS_PLATFORM == NGS_LINUX
	devices::Key key;
	while (!key.Open());

	while (true) {
		key.Update();
		if (key.GetState() != devices::Key::Type::down)continue;
		ngs::nos.Log("main", "press\n");
		break;
}
#endif

	RunBrain();
}
