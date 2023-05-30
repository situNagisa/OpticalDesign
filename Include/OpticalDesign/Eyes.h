#pragma once

#include "NGS/NGS.h"

#include "opencv2/opencv.hpp"

#include "OpticalDesign/Config.h"
#include "OpticalDesign/Setting.h"
#include "OpticalDesign/Converter.h"
#include "OpticalDesign/Maze.h"
#include "OpticalDesign/DeviceAPI.h"
#include "OpticalDesign/CVUtil.h"

OPT_BEGIN

class Eyes {
public:
	enum class Mode : ngs::byte {
		ready,
		maze,
		runtime,
		finish,
	};
public:
	~Eyes() {
		if (devices::g_camera->IsOpened()) {
			devices::g_camera->Close();
		}
		if (_curMaze) {
			ngs::Delete(_curMaze);
			_curMaze = nullptr;
		}
	}
	void Update() {
		devices::g_camera->Update();
		switch (_mode)
		{
		case Mode::ready:
			_UpdateReady();
			break;
		case Mode::maze:
			_UpdateMaze();
			break;
		case Mode::runtime:
			_UpdateRuntime();
			break;
		case Mode::finish:

			break;
		default:
			break;
		}
	}

	Maze* GetMaze() { return _curMaze; }
	const Maze* GetMaze() const { return _curMaze; }

	bool HasSeenTreasure()const { return _hasSeenTreasure; }
	bool IsSelfTreasure()const { return false; }

	void SetMode(Mode mode) { _mode = mode; }
private:
	void _UpdateReady() {

	}
	void _UpdateMaze() {
		if (!_RecognizeMaze())return;
		_ConvertImageToMaze();
	}
	void _UpdateRuntime() {
		_hasSeenTreasure = _RecognizeTreasure();
	}
	bool _RecognizeTreasure() {
		auto view = _GetView();
		//return CVUtil::RecognizeTreasure(view);
		return false;
	}
	bool _RecognizeMaze() {
		cv::Mat temp = CVUtil::RecognizeMaze<IMAGE_SIZE.x, IMAGE_SIZE.y>(_GetView());
		if (temp.empty()) {
			if constexpr (DEBUG_EYES) {
				cv::imshow("", temp);
				cv::waitKey();
				return false;
			}
			else
				return false;
		}
		_curImage = temp;
		return true;
	}
	bool _ConvertImageToMaze() {
		ngs::nos.Log("Eyes::_ConvertImageToMaze", "开始转换图像为迷宫...\n");

		ngs::Assert(!_curMaze, "迷宫内存管理错误!");
		_curMaze = ngs::New(new Maze());
		auto& maze = *_curMaze;
		// 二值化并细化图像
		auto bin = CVUtil::Threshold(_curImage);

		if constexpr (DEBUG_EYES) {
			cv::imshow("", _curImage);
			cv::waitKey();
		}

		std::vector<Point> treasures;
		Point start, end;
		{
			Rect frame;
			double stepX, stepY;
			//计算地图边界及每个格子的大小
			{
				ngs::nos.Log("Eyes::_ConvertImageToMaze", "开始寻找起点终点...\n");
				cv::Mat hsv;
				cv::cvtColor(_curImage, hsv, cv::COLOR_BGR2HSV);
				auto red_bounds = CVUtil::ColorBounds(
					hsv
					, color_defined::red
				);
				auto blue_bounds = CVUtil::ColorBounds(
					hsv
					, color_defined::blue
				);
				if (red_bounds.empty() || blue_bounds.empty()) {
					ngs::nos.Warning("未找到起点终点\n");
					ngs::Delete(_curMaze);
					_curMaze = nullptr;
					return false;
				}
				Rect red_start = ngs_cv::Convert(red_bounds[0]);
				Rect blue_start = ngs_cv::Convert(blue_bounds[0]);

				ngs::nos.Log("Eyes::_ConvertImageToMaze", "红色区域有%ld个，蓝色区域有%ld个，将(%d,%d),(%d,%d)作为起点和终点\n"
					, red_bounds.size(), blue_bounds.size()
					, (int)red_start.CenterX(), (int)red_start.CenterY()
					, (int)blue_start.CenterX(), (int)blue_start.CenterY()
				);
				ngs::nos.Log("Eyes::_ConvertImageToMaze", "开始寻找地图边界...\n");

				Rect range = red_start | blue_start;
				Rect un;
				auto bin_bounds = CVUtil::ContoursToBounds(bin);
				for (auto& bound_cv : bin_bounds) {
					auto bound = ngs_cv::Convert(bound_cv);
					if (!(bound & range))continue;
					if (un.Empty()) {
						un = bound;
						continue;
					}
					un |= bound;
				}
				auto map_center = un.Center();
				auto len = ngs::Min(un.width, un.height);
				Rect map{ map_center.x - len / 2,map_center.y - len / 2 ,len,len };

				stepX = round((double)len / MAZE_SIZE.x);
				stepY = round((double)len / MAZE_SIZE.y);

				frame.Set(
					(int)(map.x - (stepX * MAZE_SIZE.x - map.width) / 2)
					, (int)(map.y - (stepY * MAZE_SIZE.y - map.height) / 2)
					, (int)(stepX * MAZE_SIZE.x)
					, (int)(stepY * MAZE_SIZE.y)
				);
				ngs::nos.Log("Eyes::_ConvertImageToMaze", "地图边界为(%d,%d,%d,%d)，每个格子大小为(%d,%d)\n"
					, (int)frame.x, (int)frame.y, (int)frame.width, (int)frame.height, (int)stepX, (int)stepY);


				if (TEAM == Team::RED) {
					start = CVUtil::ToMaze(red_start.Center(), frame);
					end = CVUtil::ToMaze(blue_start.Center(), frame);
				}
				else if (TEAM == Team::BLUE) {
					start = CVUtil::ToMaze(blue_start.Center(), frame);
					end = CVUtil::ToMaze(red_start.Center(), frame);
				}

				start.x = ngs::Clamp<int>(start.x, 0, 2 * MAZE_SIZE.x);
				start.y = ngs::Clamp<int>(start.y, 0, 2 * MAZE_SIZE.y);
				end.x = ngs::Clamp<int>(end.x, 0, 2 * MAZE_SIZE.x);
				end.y = ngs::Clamp<int>(end.y, 0, 2 * MAZE_SIZE.y);

				ngs::nos.Log("Eyes::_ConvertImageToMaze", "起点：(%d,%d) 终点：(%d,%d)\n", (int)start.x, (int)start.y, (int)end.x, (int)end.y);
				ngs::nos.Log("Eyes::_ConvertImageToMaze", "开始寻找宝藏区域...\n");
				// 获取宝藏区域
				auto treasures_t = CVUtil::TreasureRanges(_curImage);
				if (treasures_t.size() < 8) {
					ngs::nos.Warning("宝藏数量小于8个");
					ngs::Delete(_curMaze);
					_curMaze = nullptr;
					return false;
				}

				ngs::nos.Log("Eyes::_ConvertImageToMaze", "宝藏区域数量%ld\n", treasures_t.size());
				for (auto& i : treasures_t) {
					if (!(i & frame))continue;
					Point p = CVUtil::ToMaze(i, frame);
					treasures.push_back(p);
					ngs::nos.Log("Eyes::_ConvertImageToMaze", "宝藏区域为(%d,%d)\n"
						, (int)p.x, (int)p.y);
				}
				if (treasures.size() != 8) {
					ngs::nos.Warning("宝藏区域数量不为8\n");
					ngs::Delete(_curMaze);
					_curMaze = nullptr;
					return false;
				}
			}
			//将数据转换成图
			{
				ngs::nos.Log("Eyes::_ConvertImageToMaze", "开始将墙与路写入迷宫...\n");

				std::vector<std::vector<int>> original_data_row;
				std::vector<std::vector<int>> original_data_col;
				{
					for (int y = 0; y < MAZE_SIZE.y; y++) {
						size_t black = 0;
						size_t n = 0;
						original_data_row.emplace_back();
						for (size_t x = frame.Left(); x < frame.Right(); x++) {
							auto bw = bin.at<uchar>(
								frame.Top() + y * stepY + stepY * 0.5
								, x
							);
							if (bw) {
								size_t t = round(((double)black / frame.width) * MAZE_SIZE.x);
								if (t) {
									original_data_row.back().push_back(t);
									n += t;
								}
								black = 0;
								continue;
							}
							black++;
						}
						if (black) {
							size_t t = round(((double)black / frame.width) * MAZE_SIZE.x);
							if (t) {
								original_data_row.back().push_back(t);
								n += t;
							}
							black = 0;
						}
						if (n != 10) {
							ngs::nos.Warning("转换失败");
							ngs::Delete(_curMaze);
							_curMaze = nullptr;
							return false;
						}
					}


					for (int x = 0; x < MAZE_SIZE.x; x++) {
						size_t black = 0;
						size_t n = 0;
						original_data_col.emplace_back();
						for (size_t y = frame.Top(); y < frame.Bottom(); y++) {
							auto bw = bin.at<uchar>(
								y
								, frame.Left() + x * stepX + stepX * 0.5
							);
							if (bw) {
								size_t t = round(((double)black / frame.width) * MAZE_SIZE.x);
								if (t) {
									original_data_col.back().push_back(t);
									n += t;
								}
								black = 0;
								continue;
							}
							black++;
						}
						if (black) {
							size_t t = round(((double)black / frame.width) * MAZE_SIZE.x);
							if (t) {
								original_data_col.back().push_back(t);
								n += t;
							}
							black = 0;
						}
						if (n != 10) {
							ngs::nos.Warning("转换失败\n");
							ngs::Delete(_curMaze);
							_curMaze = nullptr;
							return false;
						}
					}
				}

				{
					for (size_t j = 0; j < GRID_NUM.y; j++) {
						for (size_t i = 0; i < GRID_NUM.x; i++)
							maze.Set(i, j, MazeGrid::wall);
					}

					for (size_t j = 0; j < original_data_row.size(); j++) {
						size_t y = 2 * j + 1;
						size_t x = 0;
						maze.Set(x, y, MazeGrid::wall);
						for (auto num : original_data_row[j]) {
							for (size_t i = 0; i < num; i++) {
								x++;
								maze.Set(x, y, MazeGrid::road);
								x++;
								maze.Set(x, y, MazeGrid::road);
							}
							maze.Set(x, y, MazeGrid::wall);
						}
					}

					for (size_t j = 0; j < original_data_col.size(); j++) {
						size_t x = 2 * j + 1;
						size_t y = 0;
						maze.Set(x, y, MazeGrid::wall);
						for (auto num : original_data_col[j]) {
							for (size_t i = 0; i < num; i++) {
								y++;
								maze.Set(x, y, MazeGrid::road);
								y++;
								maze.Set(x, y, MazeGrid::road);
							}
							maze.Set(x, y, MazeGrid::wall);
						}
					}
				}
				ngs::nos.Log("Eyes::_ConvertImageToMaze", "写入成功\n");
			}
		}
		ngs::nos.Log("Eyes::_ConvertImageToMaze", "开始将宝藏写入迷宫...\n");
		maze.Set(start.x, start.y, MazeGrid::start);
		maze.Set(end.x, end.y, MazeGrid::end);
		for (auto& i : treasures) {
			maze.Set(i, MazeGrid::treasure_unknown);
		}
		maze.Update();
		ngs::nos << maze << std::endl;
		ngs::nos.Log("Eyes::_ConvertImageToMaze", "写入成功\n");
		return true;
	}

	cv::Mat _GetView() {
		//如果有摄像头设备可以注释掉这个return
		return cv::imread(TEST_PICTURE);

		if (!devices::g_camera->IsOpened() && !devices::g_camera->Open())
			return {};

		std::vector<ngs::byte> data = devices::g_camera->Get();
		if (data.empty())return {};
		cv::Mat image = cv::imdecode(data, cv::ImreadModes::IMREAD_COLOR);
		cv::imshow("", image);
		cv::waitKey();
		return image;
	}
private:

	/**
	 * @brief 当前图像，保证长宽相等
	 */
	cv::Mat _curImage;
	Maze* _curMaze = nullptr;

	Mode _mode = Mode::ready;

	bool _hasSeenTreasure = false;
};


OPT_END
