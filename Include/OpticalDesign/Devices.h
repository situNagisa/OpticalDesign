#pragma once

#include "NGS/NGS.h"
#include "opencv2/opencv.hpp"

namespace devices {

	template<typename Device>
	concept Device_c = requires(Device device, const Device device_cst) {
		std::is_function_v<decltype(&Device::Open)>;
		device.Close();
		{device_cst.IsOpened()} -> std::convertible_to<bool>;
		{device.Update()};
	};

	class Camera {

	public:
		bool Open();
		void Close();

		void Update();

		bool IsOpened()const;

		std::vector<ngs::byte> Get();
	private:
		ngs::void_ptr _data = nullptr;

		//static_assert(Device_c<Camera>);
	};

	class Screen {
	public:
		bool Open();
		void Close();

		void Update();
		bool IsOpened()const;

		void Show(const cv::Mat&);
	private:
		ngs::void_ptr _data = nullptr;

		//static_assert(Device_c<Screen>);
	};

	class Gyroscope {
	public:
		bool Open();
		void Close();

		void Update();
		bool IsOpened()const;

		float GetAccelerationX()const;
		float GetAccelerationY()const;
		float GetAccelerationZ()const;

		float GetAngularVelocityX()const;
		float GetAngularVelocityY()const;
		float GetAngularVelocityZ()const;
	private:
		ngs::void_ptr _data = nullptr;
		//static_assert(Device_c<Gyroscope>);
	};

	class Engine {
	public:
		bool Open();
		void Close();

		void Update();
		bool IsOpened()const;

		void SetLinearVelocityPercent(float percent);
		void SetAngularVelocityPercent(float percent);

		ngs::float32 GetLinearVelocity()const;
	private:
		ngs::void_ptr _data = nullptr;
		//static_assert(Device_c<Engine>);
	};

	inline Camera* g_camera = ngs::New(new Camera());
	inline Screen* g_screen = ngs::New(new Screen());
	inline Gyroscope* g_gyroscope = ngs::New(new Gyroscope());
	inline Engine* g_engine = ngs::New(new Engine());
}
