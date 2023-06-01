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
void TEST_RecognizeTreasure() {
	cv::Mat source;
	cv::VideoCapture capture;
	capture.open(0);
	capture >> source;

	//cv::Mat source = cv::imread("D:\\work\\photo\\微信图片_20230529225721.jpg");

	while (optical_design::CVUtil::RecognizeTreasure(
		source,
		optical_design::color_defined::blue,
		optical_design::color_defined::yellow
	).form == optical_design::Treasure::Form::unknown) {
		capture >> source;
		cv::imshow("", source);
		cv::waitKey(30);
	}

}

//#if NGS_PLATFORM == NGS_LINUX
//#include "NGS/Embedded/Embedded.h"
//
//#endif
//int main() {
//	ngs::nos.Log("main", "start\n");
//#if NGS_PLATFORM == NGS_LINUX
//	devices::Key key;
//	while (!key.Open());
//
//	while (true) {
//		key.Update();
//		if (key.GetState() != devices::Key::Type::down)continue;
//		ngs::nos.Log("main", "press\n");
//		break;
//	}
//
//
//#endif
//
//	//RunBrain();
//}

#define NSL_USE_HEADER_IMPL 1

#include "NGS/NGS.h"
#include "NSL/NSL.h"
#include "opencv2/opencv.hpp"

void V4L2Test() {
	nsl::V4L2 v4l2;
	if (!v4l2.Open("/dev/video1") || !v4l2.Initialize(640, 480, ngs::PixelFormat::MJPEG)) {
		ngs::nos.Error("open fail!\n");
		return;
	}
	nsl::DeviceFile fb("/dev/fb0");
	if (!fb.Open()) {
		ngs::nos.Error("open fb fail!\n");
		return;
	}
	fb_var_screeninfo var = {};
	fb_fix_screeninfo fix = {};

	fb.IOCtrl(FBIOGET_VSCREENINFO, &var);
	fb.IOCtrl(FBIOGET_FSCREENINFO, &fix);

	size_t size = fix.line_length * var.yres;
	size_t width = var.xres;
	size_t height = var.yres;

	ngs::nos.Log("main", "width %d,height %d\n");

	ngs::ARGB16* screen = (ngs::ARGB16*)fb.MemoryMap(size, PROT_READ | PROT_WRITE, MAP_SHARED);

	std::vector<ngs::byte> data;
	while (true) {
		v4l2.Update();
		auto size = v4l2.GetFrameBufferSize();
		if (data.size() < size) {
			data.resize(size);
		}
		v4l2.Read(data.data());
		cv::Mat image = cv::imdecode(data, cv::ImreadModes::IMREAD_COLOR);
		ngs::nos.Log("main", "start convert\n");
		for (size_t y = 0; y < image.rows; y++) {
			for (size_t x = 0; x < image.cols; x++) {
				auto argb24 = image.ptr<ngs::ARGB24>(y, x);
				screen[y * width + x] = (ngs::ARGB16)(*argb24);
			}
		}
	}

	v4l2.Close();
}
int main() {
	ngs::Allocator::I().Show();

	RunBrain();

	ngs::Allocator::I().Show();

	return 0;
}
