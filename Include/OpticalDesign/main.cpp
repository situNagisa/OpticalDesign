//#include "OpticalDesign/Brain.h"
//#include "OpticalDesign/CVUtil.h"

//void RunBrain() {
//	optical_design::Brain brain;
//	while (!brain.TryToStart());
//	size_t i = 1000000;
//	auto t = std::chrono::system_clock::now();
//	while (i || true) {
//		auto now = std::chrono::system_clock::now();
//		//if (std::chrono::duration_cast<std::chrono::milliseconds>(now - t).count() < 15)continue;
//		t = now;
//		brain.Update();
//		i--;
//	}
//	ngs::Allocator::I().Show();
//}
//void TEST_RecognizeTreasure() {
//	cv::Mat source;
//	cv::VideoCapture capture;
//	capture.open(0);
//	capture >> source;
//
//	//cv::Mat source = cv::imread("D:\\work\\photo\\微信图片_20230529225721.jpg");
//
//	while (optical_design::CVUtil::RecognizeTreasure(
//		source,
//		optical_design::color_defined::blue,
//		optical_design::color_defined::yellow
//	).form == optical_design::Treasure::Form::unknown) {
//		capture >> source;
//		cv::imshow("", source);
//		cv::waitKey(30);
//	}
//
//}


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

int main() {
	ngs::Allocator::I().Show();
	nsl::V4L2 v4l2;
	if (!v4l2.Open("/dev/video1") || !v4l2.Initialize(640, 480, ngs::PixelFormat::MJPEG)) {
		ngs::nos.Error("open fail!\n");
		return -1;
	}
	std::ofstream save("video.mjpeg", std::ios::binary | std::ios::out);

	{
		ngs::MeasureExecutionTime<std::chrono::nanoseconds> timer;
		ngs::byte_ptr buffer = ngs::New(new ngs::byte[97626 * 2](), 97626 * 2);
		for (size_t loops = 0; loops < 1; loops++)
		{
			v4l2.Update();
			v4l2.Read(buffer);
			save.write((const char*)buffer, v4l2.GetFrameBufferSize());
		}
		ngs::Delete(buffer);
	}

	v4l2.Close();
	ngs::Allocator::I().Show();

	return 0;
}
