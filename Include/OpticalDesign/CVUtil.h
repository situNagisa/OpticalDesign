#pragma once

#include "NGS/NGS.h"

#include "opencv2/opencv.hpp"

#include "OpticalDesign/Config.h"
#include "OpticalDesign/Setting.h"
#include "OpticalDesign/Converter.h"
#include "OpticalDesign/Maze.h"


OPT_BEGIN

class CVUtil {
public:
	/**
	 * @brief 将图像缩放到最大边长的大小
	 *
	 * \param image
	 */
	static void MaxSize(cv::Mat& image) {
		resize(image, image, { ngs::Max(image.rows,image.cols),ngs::Max(image.rows,image.cols) });
	}
	/**
	 * @brief 将图像缩放到最大边长的大小
	 *
	 * \param image
	 */
	static void MinSize(cv::Mat& image) {
		resize(image, image, { ngs::Min(image.rows,image.cols),ngs::Min(image.rows,image.cols) });
	}

	/**
	* @brief 对输入图像进行细化,骨骼化
	* @param src为输入图像,用cvThreshold函数处理过的8位灰度图像格式，元素中只有0与1,1代表有元素，0代表为空白
	* @param maxIterations限制迭代次数，如果不进行限制，默认为-1，代表不限制迭代次数，直到获得最终结果
	* @return 为对src细化后的输出图像,格式与src格式相同，元素中只有0与1,1代表有元素，0代表为空白
	*/
	static cv::Mat Thin(const cv::Mat& src, const int maxIterations = -1)
	{
		assert(src.type() == CV_8UC1);
		cv::Mat dst;
		int width = src.cols;
		int height = src.rows;
		src.copyTo(dst);
		int count = 0;  //记录迭代次数  
		while (true)
		{
			count++;
			if (maxIterations != -1 && count > maxIterations) //限制次数并且迭代次数到达  
				break;
			std::vector<uchar*> mFlag; //用于标记需要删除的点  
			//对点标记  
			for (int i = 0; i < height; ++i)
			{
				uchar* p = dst.ptr<uchar>(i);
				for (int j = 0; j < width; ++j)
				{
					//如果满足四个条件，进行标记  
					//  p9 p2 p3  
					//  p8 p1 p4  
					//  p7 p6 p5  
					uchar p1 = p[j];
					if (p1 != 1) continue;
					uchar p4 = (j == width - 1) ? 0 : *(p + j + 1);
					uchar p8 = (j == 0) ? 0 : *(p + j - 1);
					uchar p2 = (i == 0) ? 0 : *(p - dst.step + j);
					uchar p3 = (i == 0 || j == width - 1) ? 0 : *(p - dst.step + j + 1);
					uchar p9 = (i == 0 || j == 0) ? 0 : *(p - dst.step + j - 1);
					uchar p6 = (i == height - 1) ? 0 : *(p + dst.step + j);
					uchar p5 = (i == height - 1 || j == width - 1) ? 0 : *(p + dst.step + j + 1);
					uchar p7 = (i == height - 1 || j == 0) ? 0 : *(p + dst.step + j - 1);
					if ((p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) >= 2 && (p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) <= 6)
					{
						int ap = 0;
						if (p2 == 0 && p3 == 1) ++ap;
						if (p3 == 0 && p4 == 1) ++ap;
						if (p4 == 0 && p5 == 1) ++ap;
						if (p5 == 0 && p6 == 1) ++ap;
						if (p6 == 0 && p7 == 1) ++ap;
						if (p7 == 0 && p8 == 1) ++ap;
						if (p8 == 0 && p9 == 1) ++ap;
						if (p9 == 0 && p2 == 1) ++ap;

						if (ap == 1 && p2 * p4 * p6 == 0 && p4 * p6 * p8 == 0)
							mFlag.push_back(p + j);
					}
				}
			}

			//将标记的点删除  
			for (std::vector<uchar*>::iterator i = mFlag.begin(); i != mFlag.end(); ++i)
				**i = 0;

			//直到没有点满足，算法结束  
			if (mFlag.empty())
				break;
			else
				mFlag.clear();//将mFlag清空  

			//对点标记  
			for (int i = 0; i < height; ++i)
			{
				uchar* p = dst.ptr<uchar>(i);
				for (int j = 0; j < width; ++j)
				{
					//如果满足四个条件，进行标记  
					//  p9 p2 p3  
					//  p8 p1 p4  
					//  p7 p6 p5  
					uchar p1 = p[j];
					if (p1 != 1) continue;
					uchar p4 = (j == width - 1) ? 0 : *(p + j + 1);
					uchar p8 = (j == 0) ? 0 : *(p + j - 1);
					uchar p2 = (i == 0) ? 0 : *(p - dst.step + j);
					uchar p3 = (i == 0 || j == width - 1) ? 0 : *(p - dst.step + j + 1);
					uchar p9 = (i == 0 || j == 0) ? 0 : *(p - dst.step + j - 1);
					uchar p6 = (i == height - 1) ? 0 : *(p + dst.step + j);
					uchar p5 = (i == height - 1 || j == width - 1) ? 0 : *(p + dst.step + j + 1);
					uchar p7 = (i == height - 1 || j == 0) ? 0 : *(p + dst.step + j - 1);

					if ((p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) >= 2 && (p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) <= 6)
					{
						int ap = 0;
						if (p2 == 0 && p3 == 1) ++ap;
						if (p3 == 0 && p4 == 1) ++ap;
						if (p4 == 0 && p5 == 1) ++ap;
						if (p5 == 0 && p6 == 1) ++ap;
						if (p6 == 0 && p7 == 1) ++ap;
						if (p7 == 0 && p8 == 1) ++ap;
						if (p8 == 0 && p9 == 1) ++ap;
						if (p9 == 0 && p2 == 1) ++ap;

						if (ap == 1 && p2 * p4 * p8 == 0 && p2 * p6 * p8 == 0)
							mFlag.push_back(p + j);
					}
				}
			}

			//将标记的点删除  
			for (std::vector<uchar*>::iterator i = mFlag.begin(); i != mFlag.end(); ++i)
				**i = 0;

			//直到没有点满足，算法结束  
			if (mFlag.empty())
				break;
			else
				mFlag.clear();//将mFlag清空  
		}
		return dst;
	}

	/***********************************************************
	增强算法的原理在于先统计每个灰度值在整个图像中所占的比例
	然后以小于当前灰度值的所有灰度值在总像素中所占的比例，作为增益系数
	对每一个像素点进行调整。由于每一个值的增益系数都是小于它的所有值所占
	的比例和。所以就使得经过增强之后的图像亮的更亮，暗的更暗。
	************************************************************/
	static void ImageStretchByHistogram(const cv::Mat& src, cv::Mat& dst)
	{
		//判断传入参数是否正常
		if (!(src.size().width == dst.size().width))
		{
			std::cout << "error" << std::endl;
			return;
		}
		double p[256], p1[256], num[256];

		memset(p, 0, sizeof(p));
		memset(p1, 0, sizeof(p1));
		memset(num, 0, sizeof(num));
		int height = src.size().height;
		int width = src.size().width;
		long wMulh = height * width;

		//统计每一个灰度值在整个图像中所占个数
		for (int x = 0; x < width; x++)
		{
			for (int y = 0; y < height; y++)
			{
				uchar v = src.at<uchar>(y, x);
				num[v]++;
			}
		}

		//使用上一步的统计结果计算每一个灰度值所占总像素的比例
		for (int i = 0; i < 256; i++)
		{
			p[i] = num[i] / wMulh;
		}

		//计算每一个灰度值，小于当前灰度值的所有灰度值在总像素中所占的比例
		//p1[i]=sum(p[j]);	j<=i;
		for (int i = 0; i < 256; i++)
		{
			for (int k = 0; k <= i; k++)
				p1[i] += p[k];
		}

		//以小于当前灰度值的所有灰度值在总像素中所占的比例，作为增益系数对每一个像素点进行调整。
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++) {
				uchar v = src.at<uchar>(y, x);
				dst.at<uchar>(y, x) = p1[v] * 255 + 0.5;
			}
		}
		return;
	}

	/**
	 * @brief 二值化并细化图像
	 *
	 * \param image
	 * \return 新图像
	 */
	static cv::Mat Threshold(const cv::Mat& image)
	{
		cv::Mat gray, bin;

		cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
		adaptiveThreshold(gray, bin, 0x01, cv::THRESH_BINARY, cv::ADAPTIVE_THRESH_GAUSSIAN_C, 7, 3);
		cv::resize(bin, bin, { cv::min(bin.rows,bin.cols),cv::min(bin.rows,bin.cols) });

		//创建结构元素-水平直线
		cv::Mat hLineKernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(bin.cols / 16, 1), cv::Point(-1, -1));
		//创建结构元素-垂直直线
		cv::Mat vLineKernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1, bin.rows / 16), cv::Point(-1, -1));

		//提取水平线
		cv::Mat hLineImage;
		morphologyEx(bin, hLineImage, cv::MORPH_OPEN, hLineKernel);
		//提取垂直线0
		cv::Mat vLineImage;
		morphologyEx(bin, vLineImage, cv::MORPH_OPEN, vLineKernel);

		//提取字母
		cv::Mat alpImage;
		bitwise_or(hLineImage, vLineImage, alpImage);

		/*alpImage *= 0xFF;
		cv::imshow("", alpImage);
		cv::waitKey();*/

		//图像细化
		cv::Mat dst = Thin(alpImage);

		//dilate(dst, dst, getStructuringElement(MORPH_RECT, Size(2, 2)), Point(-1, -1), 7);

		//显示图像
		dst *= 0xFF;

		return dst;
	}

	/**
	 * @brief 获取图像中的宝藏区域
	 *
	 * \param image 一个等宽高彩色图
	 * \return 宝藏区域数组
	 */
	static std::vector<Rect> TreasureRanges(const cv::Mat& src) {
		ngs::Assert(src.cols == IMAGE_SIZE.x && src.rows == IMAGE_SIZE.y);
		cv::Mat bin;
		cv::cvtColor(src, bin, cv::ColorConversionCodes::COLOR_BGR2GRAY);
		adaptiveThreshold(bin, bin, 0xFF, cv::THRESH_BINARY, cv::ADAPTIVE_THRESH_GAUSSIAN_C, 7, 10);

		morphologyEx(bin, bin, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MorphShapes::MORPH_RECT, { 1,1 }), { -1,-1 }, 2);
		//erode(bin,bin, getStructuringElement(MorphShapes::MORPH_RECT, { 2,2 }), { -1,-1 }, 3);
		//dilate(bin,bin, getStructuringElement(MorphShapes::MORPH_RECT, { 2,2 }), { -1,-1 }, 2);

		std::vector<std::vector<cv::Point>> contours;
		std::vector<cv::Vec4i> hierarchy;
		//findContours(srcImg, contours, hierarchy,RETR_EXTERNAL, CHAIN_APPROX_SIMPLE ); //查找外轮廓，压缩存储轮廓点
		findContours(bin, contours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE); //查找所有轮廓
		//findContours(srcImg, contours, hierarchy,CV_RETR_CCOMP, CHAIN_APPROX_SIMPLE ); //查找所有轮廓
		//findContours(srcImg, contours, hierarchy,RETR_TREE, CHAIN_APPROX_NONE ); //查找所有轮廓，存储所有轮廓点

		// 定义一个矩形集合
		std::vector<Rect> rects;
		constexpr size_t imageArea = IMAGE_SIZE.x * IMAGE_SIZE.y;
		size_t areaThresholdMax = imageArea / (18 * 18);
		size_t areaThresholdMin = imageArea / (40 * 40);

		for (size_t i = 0; i < contours.size(); i++)
		{
			auto& contour = contours[i];
			// 如果轮廓点数小于5，则跳过
			if (contour.size() < 5) continue;

			// 计算当前轮廓的最小外接矩形
			Rect rect = ngs_cv::Convert(cv::boundingRect(cv::Mat(contour)));

			// 如果矩形面积大于90，则跳过
			if (rect.Area() > areaThresholdMax) continue;
			if (rect.Area() < areaThresholdMin)continue;

			// 如果长宽比小于0.6或大于1.73，则跳过
			if (!ngs::In<float>(rect.WH_Ratio(), std::numbers::inv_pi, std::numbers::ln10)) continue;

			// 将合法的矩形加入集合中
			rects.push_back(rect);
		}
		if (rects.empty())return {};
		std::sort(rects.begin(), rects.end(), [](const Rect& a, const Rect& b) {return a > b; });

		constexpr float treasure_area_threshold = 0.05;
		bool independent = false;
		while (!independent) {
			independent = true;
			for (auto i = rects.begin(); i != rects.end(); i++) {
				Rect& rect1 = *i;
				for (auto j = rects.begin(); j != rects.end(); j++) {
					if (i == j)continue;
					Rect& rect2 = *j;
					Rect intersect = rect1 & rect2;
					if (!intersect)continue;
					if (intersect < rect1 && intersect < rect2)continue;
					independent = false;
					rect1 = rect1 | rect2;
					j = rects.erase(j);
					break;
				}
				if (!independent)break;
			}
		}
		/*cv::Mat dst;
		cv::cvtColor(bin, dst, cv::ColorConversionCodes::COLOR_GRAY2BGR);
		for (auto& rect : rects) {
			cv::rectangle(dst, ngs_cv::Convert(rect), { 0x00,0x00,0xFF }, 1);
		}
		cv::imshow("dbg", bin);
		cv::imshow("", dst);
		cv::waitKey();*/
		return rects;
	}

	static std::vector<cv::Rect> ColorBounds(
		const cv::Mat& hsv,
		const ColorRange& range
	) {
		return ColorBounds(hsv, range.GetMask(hsv));
	}

	static std::vector<cv::Rect> ColorBounds(
		const cv::Mat& hsv,
		const cv::Mat& mask
	) {
		cv::Mat filter;
		cv::morphologyEx(mask, filter, cv::MorphTypes::MORPH_OPEN, cv::getStructuringElement(cv::MorphShapes::MORPH_RECT, { 5,5 }), { -1,-1 }, 1);
		//dilate(mask, mask, getStructuringElement(MorphShapes::MORPH_RECT, { 8,8 }), { -1,-1 }, 2);
		//erode(mask,mask, getStructuringElement(MorphShapes::MORPH_RECT, { 8,8 }), { -1,-1 }, 2);

		/*cv::imshow("", filter);
		cv::waitKey();*/

		return ContoursToBounds(filter);
	}
	/**
	 *
	 *
	 * \param bin
	 * \return
	 */
	static std::vector<cv::Rect> ContoursToBounds(const cv::Mat& bin) {

		std::vector<std::vector<cv::Point>> contours;
		std::vector<cv::Vec4i> hierarchy;
		findContours(bin, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_NONE);
		//cvtColor(bin, bin, COLOR_GRAY2BGR);

		std::vector<cv::Rect> bounds;
		for (auto& points : contours) {
			bounds.push_back(boundingRect(points));
		}
		auto compare = [](const cv::Rect& a, const cv::Rect& b)->bool {
			return a.area() > b.area();
		};
		sort(bounds.begin(), bounds.end(), compare);

		return bounds;
	}

	static Point ToMaze(const Rect& rect, const Rect& frame) {
		return ToMaze(rect.Center(), frame);
	}
	static Point ToMaze(const Point& pos, const Rect& frame) {
		Point grid(frame.width / MAZE_SIZE.x, frame.height / MAZE_SIZE.y);
		Point O{ frame.x + grid.x / 2,frame.y + grid.y / 2 };
		Point out = pos - O;
		out.x = 2 * round(out.x / grid.x) + 1;
		out.y = 2 * round(out.y / grid.y) + 1;
		return out;
	}

	template<size_t WIDTH, size_t HEIGHT>
	static cv::Mat RecognizeMaze(const cv::Mat& source) {
		ngs::Assert(!source.empty(), "source is empty");

		//外部定义,模板轮廓
		//	---------
		//	|		|
		//	|		|
		//	---------
		static const std::vector<cv::Point> temp_contour = {
			{1,0},
			{0,1},
			{0,26},
			{1,27},
			{26,27},
			{27,26},
			{27,1},
			{26,0},
		};
		constexpr Point image_size(500, 500);

		cv::Mat src;
		if constexpr (DEBUG_EYES) {
			source.copyTo(src);
		}
		else {
			src = source;
		}

		std::vector<cv::Point> corners;
		{
			struct Buffer {
				size_t index;
				ngs::float64 matchLevel;
				Rect rect;
			};

			cv::Mat canny;
			{
				cv::resize(src, src, {}, image_size.x / src.cols, image_size.y / src.cols);
				cv::cvtColor(src, canny, cv::ColorConversionCodes::COLOR_BGR2GRAY);
				cv::adaptiveThreshold(canny, canny, 0xFF, cv::THRESH_BINARY, cv::ADAPTIVE_THRESH_GAUSSIAN_C, 7, 8);
				cv::morphologyEx(canny, canny, cv::MORPH_OPEN, cv::getStructuringElement(cv::MORPH_RECT, { 2, 2 }), { -1,-1 }, 1);
				cv::Laplacian(canny, canny, 16);
			}
			if constexpr (DEBUG_EYES) {
				cv::imshow("", canny);
				cv::waitKey();
			}


			std::vector<std::vector<cv::Point>> contours;
			std::vector<cv::Vec4i> hierarchy;
			cv::findContours(canny, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, { 0,0 });

			constexpr auto area_threshold = 300;

			std::vector<Buffer> buffers;
			buffers.reserve(8);
			for (size_t i = 0; i < contours.size(); i++)
			{
				//过滤不符合要求的物体
				ngs::float64 matchLevel;
				Rect rect;
				{
					//去除不规则多边形
					std::vector<cv::Point> contour;
					contour.reserve(20);
					cv::approxPolyDP(contours[i], contour, 5, true);
					if (contour.size() > 6)continue;

					//去除匹配精度不够高的物体
					matchLevel = (
						cv::matchShapes(contours[i], temp_contour, cv::ShapeMatchModes::CONTOURS_MATCH_I1, 0)
						+ cv::matchShapes(contour, temp_contour, cv::ShapeMatchModes::CONTOURS_MATCH_I1, 0)
						) / 2;

					if (matchLevel > 1)continue;

					//去除小物体
					double area = cv::contourArea(contours[i]);
					if (area < area_threshold)continue;

					//去除长得极为抽象的物体
					rect = ngs_cv::Convert(cv::boundingRect(contour));
					if (ngs::Abs(area - rect.Area()) > area_threshold)continue;

					//去除长方形
					auto ratio = rect.WH_Ratio();
					if (!ngs::In<ngs::float32>(ratio, std::numbers::inv_pi, std::numbers::ln10))continue;

					//去除过大的物体
					const Point threshold = { canny.cols / 8.0f,canny.rows / 8.0f };
					if (rect.width > threshold.x || rect.height > threshold.y)continue;


				}
				buffers.emplace_back(i, matchLevel, rect);
				//debug
				if constexpr (DEBUG_EYES) cv::drawContours(src, contours, i, { 0,0,0xFF }, 1);
			}
			if (buffers.size())ngs::nos.Log("Eyes::RecognizeMaze", "找到%ld个可能的图形\n", buffers.size());

			//在符合要求的物体中找到最符合要求的物体
			if (buffers.size() < 4) {
				ngs::nos.Log("CVUtil::RecognizeMaze", "识别失败!找到的图形不足四个! %d\n", buffers.size());
				return {};
			}
			std::sort(buffers.begin(), buffers.end(), [](const Buffer& a, const Buffer& b)->bool {
				if (ngs::Abs(a.rect.Area() - b.rect.Area()) > area_threshold) return a.matchLevel < b.matchLevel;
				return a.matchLevel < b.matchLevel;
				});

			for (size_t i = 0; i < buffers.size() - 3; i++)
			{
				const Buffer&
					b1 = buffers[i],
					b2 = buffers[i + 1],
					b3 = buffers[i + 2],
					b4 = buffers[i + 3]
					;
				auto averageArea = (b1.rect.Area() + b2.rect.Area() + b3.rect.Area() + b4.rect.Area()) / 4;
				constexpr ngs::float64 rate = 0.4;
				if (
					ngs::Abs(averageArea - b1.rect.Area()) > averageArea * rate ||
					ngs::Abs(averageArea - b2.rect.Area()) > averageArea * rate ||
					ngs::Abs(averageArea - b3.rect.Area()) > averageArea * rate ||
					ngs::Abs(averageArea - b4.rect.Area()) > averageArea * rate
					)continue;


				for (size_t j = 0; j < 4; j++)
				{
					//debug
					if constexpr (DEBUG_EYES) cv::drawContours(src, contours, buffers[i + j].index, { 0,0,0xFF }, 3);
					corners.push_back(ngs_cv::Convert(buffers[i + j].rect.Center()));
				}
				std::sort(corners.begin(), corners.end(), [](const cv::Point& a, const cv::Point& b)->bool {
					return a.x + a.y < b.x + b.y;
					});
			}
			if constexpr (DEBUG_EYES) {
				cv::imshow("", canny);
				cv::waitKey();
			}
			ngs::nos.Log("Eyes::RecognizeMaze", "识别成功!\n");
		}

		if (!corners.size()) {
			ngs::nos.Log("CVUtil::RecognizeMaze", "识别失败!\n");
			return {};
		}

		cv::Mat result{ WIDTH, HEIGHT, src.type() };
		{
			cv::Mat matrix;
			//需要四个点
			std::vector<cv::Point2f> srcPointP;
			std::vector<cv::Point2f> dstPointP;
			//坐标顺序会一一对应变换
			srcPointP.push_back(corners[0]);
			srcPointP.push_back(corners[1]);
			srcPointP.push_back(corners[2]);
			srcPointP.push_back(corners[3]);

			dstPointP.push_back(cv::Point2f(0, 0));
			dstPointP.push_back(cv::Point2f(0, HEIGHT));
			dstPointP.push_back(cv::Point2f(WIDTH, 0));
			dstPointP.push_back(cv::Point2f(WIDTH, HEIGHT));

			//获得透视变换矩阵
			matrix = getPerspectiveTransform(srcPointP, dstPointP);
			//进行透视变换
			warpPerspective(src, result, matrix, result.size());
		}

		return result;
	}
	static Treasure RecognizeTreasure(
		cv::Mat& source,
		const ColorRange& CardRange,
		const ColorRange& PatternRange
	) {
		cv::resize(source, source, { IMAGE_SIZE.x,IMAGE_SIZE.y });

		/*cv::imshow("", source);
		cv::waitKey();*/

		auto bounds = _RecognizeCardColor(source, CardRange);
		if (bounds.size() == 0) {
			ngs::nos.Warning("未找到指定的颜色范围内的轮廓\n");
			return { {},{},Treasure::Form::unknown };
		}
		auto form = _RecognizePattern(source, PatternRange, bounds[0]);
		Treasure treasure = {};
		treasure.card = ngs_cv::Convert(CardRange.GetColor());
		treasure.pattern = ngs_cv::Convert(PatternRange.GetColor());
		treasure.form = form;
		return treasure;
	}

public:
	static std::vector<Rect> _RecognizeCardColor(
		cv::Mat& source,
		const ColorRange& CardRange
	) {
		constexpr Point min_rate = { 0.05,0.05 };
		constexpr Point max_rate = { 0.6,0.6 };

		cv::Mat hsv;
		cv::cvtColor(source, hsv, cv::COLOR_BGR2HSV);
		cv::Mat mask = CardRange.GetMask(hsv);
		cv::Mat filter;
		cv::morphologyEx(mask, filter, cv::MorphTypes::MORPH_CLOSE, cv::getStructuringElement(cv::MorphShapes::MORPH_RECT, { 5,5 }), { -1,-1 }, 1);

		std::vector<std::vector<cv::Point>> CardContours;
		std::vector<cv::Vec4i> hierarchy;
		cv::findContours(filter, CardContours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

		std::vector<Rect> bounds;
		for (const auto& CardContour : CardContours) {
			auto bound = ngs_cv::Convert(cv::boundingRect(CardContour));
			if (!ngs::In(bound.width, IMAGE_SIZE.x * min_rate.x, IMAGE_SIZE.x * max_rate.x))continue;
			if (!ngs::In(bound.height, IMAGE_SIZE.y * min_rate.y, IMAGE_SIZE.y * max_rate.y))continue;
			cv::rectangle(source, ngs_cv::Convert(bound), { 0x00,0xFF,0x00 }, 2);
			if (!ngs::In<float>(bound.WH_Ratio(), 0.1, 10))continue;

			double CardArea = cv::contourArea(CardContour);
			if (!ngs::In(bound.Area() / CardArea, 0.8, 1.2))continue;
			bounds.push_back(bound);
		}
		ngs::nos.Log("CVUtil::_RecognizeCardColor", "检测到卡片%d个\n", bounds.size());
		std::ranges::sort(bounds, [](const Rect& a, const Rect& b) {
			return a < b;
			});
		return bounds;
	}

	static Treasure::Form _RecognizePattern(
		const cv::Mat& source,
		const ColorRange& PatternRange,
		const Rect& bound
	) {
		constexpr Point min_rate = { 0.6,0.3 };
		constexpr Point max_rate = { 0.95,0.7 };

		cv::Mat hsv;
		source(ngs_cv::Convert(bound)).copyTo(hsv);
		cv::cvtColor(hsv, hsv, cv::COLOR_BGR2HSV);
		Point image_size = { 200,200 };
		cv::resize(hsv, hsv, {}, image_size.x / hsv.cols, image_size.y / hsv.cols);
		image_size.Set((float)hsv.cols, (float)hsv.rows);

		cv::Mat mask = PatternRange.GetMask(hsv);
		cv::Mat filter;
		cv::morphologyEx(mask, filter, cv::MorphTypes::MORPH_OPEN, cv::getStructuringElement(cv::MorphShapes::MORPH_RECT, { 5,5 }), { -1,-1 }, 1);


		cv::imshow("", filter);
		cv::waitKey();
		std::vector<std::vector<cv::Point>> PatternContours;
		std::vector<cv::Vec4i> hierarchy;
		cv::findContours(filter, PatternContours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);
		cv::cvtColor(hsv, hsv, cv::COLOR_HSV2BGR);
		std::vector<std::vector<cv::Point>> validContours;
		std::vector<Rect> bounds;


		for (size_t i = 0; i < PatternContours.size(); i++) {
			const auto& PatternContour = PatternContours[i];
			cv::drawContours(hsv, PatternContours, i, { 0x00,0xFF,0x00 }, 2);

			auto bound = ngs_cv::Convert(cv::boundingRect(PatternContour));
			if (!ngs::In(bound.width, image_size.x * min_rate.x, image_size.x * max_rate.x))continue;
			if (!ngs::In(bound.height, image_size.y * min_rate.y, image_size.y * max_rate.y))continue;
			//cv::rectangle(source, ngs_cv::Convert(bound), { 0x00,0xFF,0x00 }, 2);
			if (!ngs::In<float>(bound.WH_Ratio(), 0.5, 2))continue;
			//cv::rectangle(hsv, ngs_cv::Convert(bound), { 0x00,0xFF,0x00 }, 2);

			cv::imshow("", hsv);
			cv::waitKey();
			double PatternArea = cv::contourArea(PatternContour);
			if (!ngs::In<float>(bound.Area() / PatternArea, 0.8 * 0.8, 1.5 * 1.5))continue;

			validContours.push_back(PatternContour);
		}

		ngs::nos.Log("CVUtil::_RecognizePattern", "检测到形状%d个\n", validContours.size());

		if (validContours.empty())
		{
			ngs::nos.Log("CVUtil::_RecognizePattern", "未找到指定的颜色范围内的轮廓\n");
			return Treasure::Form::unknown;
		}

		for (size_t i = 0; i < validContours.size(); i++)
		{
			size_t count = 0;
			Treasure::Form form = Treasure::Form::unknown;

			if (_IsCircle(validContours)) {
				form = Treasure::Form::circle;
				count++;
			}
			if (_IsTriangle(validContours)) {
				form = Treasure::Form::triangle;
				count++;
			}
			if (count != 1)continue;
			return form;
		}
		return Treasure::Form::unknown;

	}

	static bool _IsCircle(const std::vector<std::vector<cv::Point>>& contours) {
		for (size_t i = 0; i < contours.size(); i++)
		{
			cv::Point2f center;
			float radius;
			cv::minEnclosingCircle(contours[i], center, radius);
			double area = cv::contourArea(contours[i]);
			double perimeter = cv::arcLength(contours[i], true);
			double circularity = 4 * CV_PI * area / (perimeter * perimeter);

			if (circularity > 0.7) {
				ngs::nos.Log("CVUtil::_IsCircle", "这是一个圆形\n");
				return true;
			}
			return false;
		}
		return false;
	}
	static bool _IsTriangle(const std::vector<std::vector<cv::Point>>& contours) {
		for (size_t i = 0; i < contours.size(); i++)
		{
			std::vector<cv::Point> polygon;
			polygon.reserve(20);
			cv::approxPolyDP(contours[i], polygon, 10, true);

			if (ngs::In<size_t>(polygon.size(), 3, 5))
			{
				ngs::nos.Log("CVUtil::_IsTriangle", "这是一个三角形\n");
				return true;
			}
			return false;
		}
		return false;
	}


};

OPT_END