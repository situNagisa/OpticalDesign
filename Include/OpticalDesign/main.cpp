#include "OpticalDesign/Brain.h"
#include "OpticalDesign/CVUtil.h"
#include "NSL/NSL.h"
#include "OpticalDesign/LinuxDevice.h"

void RunBrain() {
	optical_design::Team team = optical_design::Team::RED;
	//#if NGS_PLATFORM == NGS_LINUX
	{
		devices::Key start;
		while (!start.Open());
		start.Update();
		while (!start.IsDown()) {
			start.Update();
		}
		ngs::nos.Log("RunBrain", "start!\n");

		ngs::GPIO team_key;
		if (!team_key.Open(1, ngs::GPIO::Mode::input)) {
			ngs::nos.Error("open gpio fail!\n");
			return;
		}
		team = team_key.Get() ? optical_design::Team::BLUE : optical_design::Team::RED;
		ngs::nos.Log("RunBrain", "team is %d\n", team);
	}
	//#endif
	{
		optical_design::Brain brain;
		brain.SetTeam(team);
		while (!brain.TryToStart());
		while (true) {
			brain.Update();
		}
	}
}
void TEST_RecognizeTreasure() {
	cv::Mat source;
	cv::VideoCapture capture;
	capture.open(0);
	capture.set(cv::CAP_PROP_FRAME_WIDTH, 640);
	capture.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
	capture >> source;

	//cv::Mat source = cv::imread(R"(C:\Users\86198\Pictures\test12.jpg)");

	auto card_color = optical_design::color_defined::blue;
	auto pattern_color = optical_design::color_defined::yellow;
	while (optical_design::CVUtil::RecognizeTreasure(
		source,
		card_color,
		pattern_color
	).form == optical_design::Treasure::Form::unknown) {
		capture >> source;
		cv::imshow("", source);
		cv::waitKey(30);
	}

}

//#define NSL_USE_HEADER_IMPL 1
//
//#include "NGS/NGS.h"
//#include "NSL/NSL.h"
//#include "opencv2/opencv.hpp"
//#include "OpticalDesign/Linux/Camera.h"
//#include "OpticalDesign/Linux/Screen.hpp"
//
//void V4L2Test() {
//	nsl::V4L2 v4l2;
//	if (!v4l2.Open("/dev/video1") || !v4l2.Initialize(320, 180, ngs::PixelFormat::MJPEG)) {
//		ngs::nos.Error("open fail!\n");
//		return;
//	}
//	nsl::FrameBuffer fb;
//	if (!fb.Open("/dev/fb0")) {
//		ngs::nos.Error("open fb fail!\n");
//		return;
//	}
//
//	ngs::nos.Log("main", "width %d,height %d\n", fb.GetWidth(), fb.GetHeight());
//
//	ngs::BGRA16* screen = fb.GetData<ngs::BGRA16>();
//
//	std::vector<ngs::byte> data;
//	while (true) {
//		v4l2.Update();
//		auto size = v4l2.GetFrameBufferSize();
//		if (data.size() < size) {
//			data.resize(size);
//		}
//		v4l2.Read(data.data());
//		cv::Mat image = cv::imdecode(data, cv::ImreadModes::IMREAD_COLOR);
//		ngs::nos.Log("main", "start convert\n");
//		for (size_t y = 0; y < image.rows; y += 4) {
//			for (size_t x = 0; x < image.cols; x += 4) {
//				auto argb24 = image.ptr<ngs::ARGB24>(y, x);
//				screen[y * fb.GetWidth() + x] = ngs::BGRA16(argb24->StdBlue(), argb24->StdGreen(), argb24->StdRed(), argb24->StdAlpha());
//			}
//		}
//	}
//
//	v4l2.Close();
//}
//void Test_LCD() {
//	devices::Camera& camera = *devices::g_camera;
//	devices::Screen& screen = *devices::g_screen;
//
//	if (!camera.Open()) {
//		ngs::nos.Error("camera!\n");
//		return;
//	}
//	if (!screen.Open()) {
//		ngs::nos.Error("screen!\n");
//		return;
//	}
//
//	while (true) {
//		camera.Update();
//		std::vector<ngs::byte> data = camera.Get();
//		if (data.empty()) {
//			ngs::nos.Error("camera get!\n");
//			continue;
//		}
//		cv::Mat image = cv::imdecode(data, cv::ImreadModes::IMREAD_COLOR);
//		screen.Show(image);
//		screen.Update();
//	}
//}


int main() {
	ngs::Allocator::I().Show();

	//TEST_RecognizeTreasure();
	RunBrain();
	//V4L2Test();
	//Test_LCD();

	ngs::Allocator::I().Show();

	return 0;
}
