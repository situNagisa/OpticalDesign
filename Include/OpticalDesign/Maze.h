//==========================================================================================================
// @file	:	OpticalDesign/Maze
// @CLR		:	4.0.30319.42000
// @author	:	NAGISA
// @data	:	2023/4/29 23:24:33
// @brief	:	
//==========================================================================================================
#pragma once

#ifndef __HEAD_OpticalDesign_Maze
#define __HEAD_OpticalDesign_Maze

#include "NGS/NGS.h"
#include "OpticalDesign/Config.h"
#include "OpticalDesign/Setting.h"

OPT_BEGIN

class Segment {
public:
	Segment() = default;
	Segment(Point start, Point end)
		: start(start)
		, end(end)
	{}
public:
	Point
		start,
		end;
};

class MazeGrid {
public:
	enum Grid : ngs::byte {
		unknown = 0,

		wall,
		road,

		treasure,
		treasure_target,
		treasure_unknown,

		start,
		end,
	};
	MazeGrid() = default;
	MazeGrid(Grid data)
		: data(data)
		, direct()
	{}

	bool operator==(const MazeGrid& other)const { return data == other.data; }
	bool operator==(Grid grid)const { return data == grid; }
	void operator=(Grid grid) { data = grid; }

	bool IsWall()const { return data == Grid::wall; }
	bool IsRoad()const { return !IsWall(); }

	//@brief 若有可能是己方的宝藏，则返回true，否则返回false
	bool IsTreasure()const { return data == Grid::treasure || data == Grid::treasure_unknown; }
	bool IsStart()const { return data == Grid::start; }
	bool IsEnd()const { return data == Grid::end; }

	bool IsFork()const { return _next.size() >= 3; }
	bool IsDeadEnd()const { return _next.size() == 1; }
	bool IsPathNode()const { return IsFork() || (IsDeadEnd()); }

	size_t ColWallNum()const {
		size_t num = 0;
		if (pUp() && pUp()->IsWall())num++;
		if (pDown() && pDown()->IsWall())num++;
		return num;
	}
	size_t RowWallNum()const {
		size_t num = 0;
		if (pLeft() && pLeft()->IsWall())num++;
		if (pRight() && pRight()->IsWall())num++;
		return num;
	}
	size_t ColRoadNum()const {
		size_t num = 0;
		if (pUp() && pUp()->IsRoad())num++;
		if (pDown() && pDown()->IsRoad())num++;
		return num;
	}
	size_t RowRoadNum()const {
		size_t num = 0;
		if (pLeft() && pLeft()->IsRoad())num++;
		if (pRight() && pRight()->IsRoad())num++;
		return num;
	}

	MazeGrid*& pUp() { return _around[0]; }
	MazeGrid*& pDown() { return _around[1]; }
	MazeGrid*& pLeft() { return _around[2]; }
	MazeGrid*& pRight() { return _around[3]; }
	MazeGrid* pUp()const { return _around[0]; }
	MazeGrid* pDown()const { return _around[1]; }
	MazeGrid* pLeft()const { return _around[2]; }
	MazeGrid* pRight()const { return _around[3]; }

	MazeGrid& Up() { return *_around[0]; }
	MazeGrid& Down() { return *_around[1]; }
	MazeGrid& Left() { return *_around[2]; }
	MazeGrid& Right() { return *_around[3]; }
	MazeGrid& Up()const { return *_around[0]; }
	MazeGrid& Down()const { return *_around[1]; }
	MazeGrid& Left()const { return *_around[2]; }
	MazeGrid& Right()const { return *_around[3]; }

	void UpdateDirect() {
		switch (data) {
		case Grid::wall:
			if ((ColWallNum() + RowWallNum() != 1) && (!ColWallNum() || !RowWallNum()))
				break;
			if (pUp() && pUp()->IsWall())direct.up = true;
			if (pDown() && pDown()->IsWall())direct.down = true;
			if (pLeft() && pLeft()->IsWall())direct.left = true;
			if (pRight() && pRight()->IsWall())direct.right = true;
			break;
		case Grid::start:
		case Grid::end:
		case Grid::treasure:
		case Grid::treasure_target:
		case Grid::treasure_unknown:
		case Grid::road:
			if ((ColRoadNum() + RowRoadNum() != 1) && (!ColRoadNum() || !RowRoadNum()))
				break;
			if (pUp() && pUp()->IsRoad())direct.up = true;
			if (pDown() && pDown()->IsRoad())direct.down = true;
			if (pLeft() && pLeft()->IsRoad())direct.left = true;
			if (pRight() && pRight()->IsRoad())direct.right = true;
			break;
		default:

			break;
		}
	}

	bool IsActive()const { return direct.up || direct.down || direct.left || direct.right; }
	operator bool()const { return IsActive(); }

	const std::vector<Point>& Next()const { return _next; }
	void AddNext(const Point& pNext) { _next.push_back(pNext); }
private:
	std::array<MazeGrid*, 4> _around = {};
	std::vector<Point> _next = {};
public:
	struct {
		bool up : 1;
		bool down : 1;
		bool left : 1;
		bool right : 1;
	} direct;
	Grid data = Grid::unknown;
};


struct RouteInfo {
	size_t count;
	std::vector<Point> points;

	RouteInfo() = default;
	RouteInfo(const Point& start, const Point& end, bool pushStart = false)
		: count((size_t)(end - start).Length())
		, points()
	{
		if (pushStart)points.push_back(start);
		points.push_back(end);
	}
	Point& GetA() { return points.front(); }
	const Point& GetA()const { return points.front(); }

	Point& GetB() { return points.back(); }
	const Point& GetB()const { return points.back(); }

	const Point& GetOther(const Point& p)const {
		if (p == GetA())return GetB();
		if (p == GetB())return GetA();
		throw std::runtime_error("invalid point");
	}
	operator bool()const { return count; }
	RouteInfo& operator+=(const RouteInfo& info) {
		count += info.count;
		if (GetA() == info.GetA())
			points.insert(points.begin(), info.points.rbegin(), info.points.rend() - 1);
		else if (GetB() == info.GetB())
			points.insert(points.end(), info.points.rbegin() + 1, info.points.rend());
		else if (GetA() == info.GetB())
			points.insert(points.begin(), info.points.begin(), info.points.end() - 1);
		else if (GetB() == info.GetA())
			points.insert(points.end(), info.points.begin() + 1, info.points.end());
		else
			throw std::runtime_error("invalid route");

		return *this;
	}
	bool Has(const Point& pos)const {
		for (auto it = points.begin(); it != points.end() - 1; it++) {
			const Point& A = *it;
			const Point& B = *(it + 1);
			if (ngs::In(pos, A, B))return true;
		}
		return false;
	}
	void LinkWith(const RouteInfo& info) {
		count += info.count;
		points.insert(points.end(), info.points.begin(), info.points.end());
	}
	RouteInfo operator+(const RouteInfo& info)const {
		RouteInfo ret = *this;
		ret += info;
		return ret;
	}
};
class Maze {
public:
	inline static constexpr size_t WIDTH = GRID_NUM.x;
	inline static constexpr size_t HEIGHT = GRID_NUM.y;
	inline static constexpr size_t AREA = WIDTH * HEIGHT;

	NGS_TYPE_DEFINE(MazeGrid, grid);
	NGS_TYPE_DEFINE(MazeGrid::Grid, grid_e)
public:
	Maze()
		: _data()
		, _start()
		, _end()
		, _treasures()
		, _walls()
		, _roads()
	{
		for (size_t x = 0; x < WIDTH; x++) {
			for (size_t y = 0; y < HEIGHT; y++) {
				auto& grid = _data[x][y];
				grid.data = MazeGrid::Grid::unknown;
				grid.pUp() = _at(x, y - 1);
				grid.pDown() = _at(x, y + 1);
				grid.pLeft() = _at(x - 1, y);
				grid.pRight() = _at(x + 1, y);
			}
		}
	}

	__grid_ref operator()(size_t x, size_t y) { return _data[x][y]; }
	__grid_ref_cst operator()(size_t x, size_t y)const { return _data[x][y]; }

	__grid_ref At(size_t x, size_t y) { return _data[x][y]; }
	__grid_ref_cst At(size_t x, size_t y)const { return _data[x][y]; }
	__grid_ref At(const Point& pos) { return _data[(size_t)pos.x][(size_t)pos.y]; }
	__grid_ref_cst At(const Point& pos)const { return _data[(size_t)pos.x][(size_t)pos.y]; }

	const std::vector<Point>& GetWalls()const { return _walls; }
	const std::vector<Point>& GetRoads()const { return _roads; }

	const Point& GetStart()const { return _start; }
	const Point& GetEnd()const { return _end; }

	void Set(const Point& pos, __grid_e grid_e) {
		Set(pos.x, pos.y, grid_e);
	}
	void Set(size_t x, size_t y, __grid_e grid_e) {
		auto& grid = _data[x][y];
		if (grid.IsTreasure()) {
			std::erase(_treasures, Point(x, y));
		}
		switch (grid_e)
		{
		case __grid_e::treasure_unknown:
		case __grid_e::treasure:
			_treasures.emplace_back(x, y);
			break;
		case __grid_e::start:
			_start.Set(x, y);
			break;
		case __grid_e::end:
			_end.Set(x, y);
			break;
		default:
			break;
		}
		grid.data = grid_e;
	}
	void SetTargetTreasure(const Point& pos) {
		SetTargetTreasure(pos.x, pos.y);
	}
	void SetTargetTreasure(size_t x, size_t y) {
		auto& grid = _data[x][y];
		ngs::Assert(grid.IsTreasure());
		Set(x, y, MazeGrid::Grid::treasure_target);
		Set(GetSymmetry({ (float)x,(float)y }), MazeGrid::Grid::treasure);
	}
	void SetSelfTreasure(const Point& pos) {
		SetSelfTreasure(pos.x, pos.y);
	}
	void SetSelfTreasure(size_t x, size_t y) {
		auto& grid = _data[x][y];
		ngs::Assert(grid.IsTreasure());
		Set(x, y, MazeGrid::Grid::treasure);
		Set(GetSymmetry({ (float)x,(float)y }), MazeGrid::Grid::treasure_target);
	}
	void FoundTreasure(const Point& pos) {
		FoundTreasure(pos.x, pos.y);
	}
	void FoundTreasure(size_t x, size_t y) {
		auto& grid = _data[x][y];
		ngs::Assert(grid.IsTreasure());
		Set(x, y, MazeGrid::Grid::road);
	}
	const std::vector<Point>& GetTreasures()const { return _treasures; }

	void Update() {
		for (size_t x = 0; x < WIDTH; x++) {
			for (size_t y = 0; y < HEIGHT; y++) {
				_data[x][y].UpdateDirect();
			}
		}
		_UpdateWalls();
		_UpdateRoads();
	}

	Rect GetBounds()const { return Rect{ 0,0,(float)WIDTH,(float)HEIGHT }; }
	Point GetCenter()const { return Point{ (float)(WIDTH / 2), (float)(HEIGHT / 2) }; }
	/** @brief 获取对称点 */
	Point GetSymmetry(const Point& pos)const { return (2 * GetCenter()) - pos; }

	RouteInfo StraightTo(const Point& from, const Point& to)const {
		std::vector<Point> visited = { from };
		return _StraightTo(from, from, to, visited);
	}
	std::vector<RouteInfo> StraightTo(const Point& from)const {
		std::vector<RouteInfo> ret;
		for (const auto& next : At(from).Next()) {
			auto& grid = At(next);
			RouteInfo info = RouteInfo{ from,next ,true };
			if (grid.IsFork() || grid.IsDeadEnd()) {
				ret.emplace_back(info);
				continue;
			}
			info.LinkWith(_StraightTo(from, next));
			ret.emplace_back(info);
		}
		return ret;
	}

	friend ngs::ostream& operator<<(ngs::ostream& os, const Maze& maze) {
		for (size_t y = 0; y < maze.HEIGHT; y++)
		{
			for (size_t x = 0; x < maze.WIDTH; x++)
			{
				switch (maze.At(x, y).data) {
				case MazeGrid::wall:
					os << ngs::TextColor::BLACK << "墙";
					break;
				case MazeGrid::road:
					os << ngs::TextColor::WHITE << "路";
					break;
				case MazeGrid::treasure:
				case MazeGrid::treasure_target:
				case MazeGrid::treasure_unknown:
					os << ngs::TextColor::GREEN << "宝";
					break;
				case MazeGrid::start:
					os << ngs::TextColor::RED << "起";
					break;
				case MazeGrid::end:
					os << ngs::TextColor::BLUE << "终";
					break;
				}
			}
			os << "\n";
		}
		os << ngs::TextColor::RESERT;
		return os;
	}
private:
	__grid_ptr _at(size_t x, size_t y) {
		if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)return nullptr;
		return &_data[x][y];
	}
	__grid_ptr_cst _at(size_t x, size_t y)const {
		if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)return nullptr;
		return &_data[x][y];
	}
	/**
	 * @brief 计算从当前点cur到目标点dst的最短距离（沿着直线走）
	 *
	 * @note 这段代码实现了在当前节点cur到目标节点dst之间直线移动的逻辑。具体来说，它首先获取当前节点的下一个节点n，并遍历所有的下一个节点，直到找到目标节点或者遍历完所有的下一个节点为止。
	 *	在遍历下一个节点的过程中，它会判断下一个节点是否是上一个节点prev或者已经访问过visited，如果是则直接跳过。否则，它会将下一个节点加入已访问节点集合visited中，并递归地调用_StraightTo函数，计算从下一个节点到目标节点的距离。
	 *	最后，它将从下一个节点到目标节点的距离加上当前节点到下一个节点的距离，返回总距离。如果无法到达目标节点，则返回0。
	 *
	 * @param[in] prev是cur的前一个点，避免重复计算，防止死循环
	 * @param[in] cur是当前点
	 * @param[in] dst是目标点
	 * @param visited是已访问节点集合
	 *
	 * @return
	 */
	RouteInfo _StraightTo(const Point& prev, const Point& cur, const Point& dst, std::vector<Point>& visited) const {
		auto& c = At(cur);
		auto& n = c.Next();
		ngs::Assert(!n.empty(), "logic error");
		RouteInfo info = {};
		for (auto& next : n) {
			// 避免重复访问
			if (next == prev || find(visited.begin(), visited.end(), next) != visited.end())continue;
			// 找到目标点，返回距离
			if (next == dst) {
				info = RouteInfo{ cur,next };
				return info;
			}
			// 判断当前节点的下一个节点是否有多个相邻节点，如果有则跳过该节点，继续遍历下一个节点
			if (At(next).Next().size() > 2)continue;
			visited.push_back(next);
			// 继续遍历下一个点
			auto ret = _StraightTo(cur, next, dst, visited);
			// 如果找到了目标点，返回距离
			if (ret) {
				info = RouteInfo{ cur,next };
				info += ret;
				return info;
			}
		}
		return info;
	}
	RouteInfo _StraightTo(const Point& prevent, const Point& from)const {
		RouteInfo info = {};
		Point prev = prevent;
		Point cur = from;
		__grid_ptr_cst curGrid = _at(cur.x, cur.y);
		while (true) {
			curGrid = _at(cur.x, cur.y);
			auto& next = curGrid->Next();
			if (curGrid->IsFork())break;
			if (curGrid->IsDeadEnd() && next[0] == prev) break;
			auto& n = next[0] == prev ? next[1] : next[0];
			info.LinkWith(RouteInfo{ cur,n });
			prev = cur;
			cur = n;
		}
		return info;
	}
	void _UpdateWalls() {
		for (size_t x = 0; x < WIDTH; x++) {
			for (size_t y = 0; y < HEIGHT; y++)
			{
				__grid_ref node = _data[x][y];
				if (!node.IsWall())continue;
				if (!node)continue;
				_walls.emplace_back(x, y);
				if (node.direct.up) {
					for (size_t iy = -1; y + iy > 0; iy--)
					{
						auto& node2 = _data[x][y + iy];
						if (!node2)continue;
						if (!node2.direct.down)continue;
						node.AddNext({ (float)x,(float)(y + iy) });
						node2.AddNext({ (float)x,(float)(y) });
						node.direct.up = false;
						node2.direct.down = false;
						break;
					}
				}
				if (node.direct.down) {
					for (size_t iy = 1; y + iy < HEIGHT; iy++)
					{
						auto& node2 = _data[x][y + iy];
						if (!node2)continue;
						if (!node2.direct.up)continue;
						node.AddNext({ (float)x,(float)(y + iy) });
						node2.AddNext({ (float)x,(float)(y) });
						node.direct.down = false;
						node2.direct.up = false;
						break;
					}
				}
				if (node.direct.left) {
					for (size_t ix = -1; x + ix > 0; ix--)
					{
						auto& node2 = _data[x + ix][y];
						if (!node2)continue;
						if (!node2.direct.right)continue;
						node.AddNext({ (float)(x + ix),(float)y });
						node2.AddNext({ (float)x,(float)(y) });
						node.direct.left = false;
						node2.direct.right = false;
						break;
					}
				}
				if (node.direct.right) {
					for (size_t ix = 1; x + ix < WIDTH; ix++)
					{
						auto& node2 = _data[x + ix][y];
						if (!node2)continue;
						if (!node2.direct.left)continue;
						node.AddNext({ (float)(x + ix),(float)y });
						node2.AddNext({ (float)x,(float)(y) });
						node.direct.right = false;
						node2.direct.left = false;
						break;
					}
				}
			}
		}
	}
	void _UpdateRoads() {
		for (size_t x = 0; x < WIDTH; x++) {
			for (size_t y = 0; y < HEIGHT; y++)
			{
				__grid_ref node = _data[x][y];
				if (!node.IsRoad())continue;
				if (!node)continue;
				if (std::find(_roads.begin(), _roads.end(), Point{ (float)x,(float)y }) == _roads.end())
					_roads.emplace_back(x, y);
				if (node.direct.up) {
					for (size_t iy = -1; y + iy > 0; iy--)
					{
						auto& node2 = _data[x][y + iy];
						if (!node2)continue;
						if (!node2.direct.down)continue;
						if (std::find(_roads.begin(), _roads.end(), Point{ (float)x,(float)y + iy }) == _roads.end())
							_roads.emplace_back(x, y + iy);
						node.AddNext({ (float)x,(float)(y + iy) });
						node2.AddNext({ (float)x,(float)(y) });
						node.direct.up = false;
						node2.direct.down = false;
						break;
					}
				}
				if (node.direct.down) {
					for (size_t iy = 1; y + iy < HEIGHT; iy++)
					{
						auto& node2 = _data[x][y + iy];
						if (!node2)continue;
						if (!node2.direct.up)continue;
						if (std::find(_roads.begin(), _roads.end(), Point{ (float)x,(float)y + iy }) == _roads.end())
							_roads.emplace_back(x, y + iy);
						node.AddNext({ (float)x,(float)(y + iy) });
						node2.AddNext({ (float)x,(float)(y) });
						node.direct.down = false;
						node2.direct.up = false;
						break;
					}
				}
				if (node.direct.left) {
					for (size_t ix = -1; x + ix > 0; ix--)
					{
						auto& node2 = _data[x + ix][y];
						if (!node2)continue;
						if (!node2.direct.right)continue;
						if (std::find(_roads.begin(), _roads.end(), Point{ (float)x + ix,(float)y }) == _roads.end())
							_roads.emplace_back(x + ix, y);
						node.AddNext({ (float)(x + ix),(float)y });
						node2.AddNext({ (float)x,(float)(y) });
						node.direct.left = false;
						node2.direct.right = false;
						break;
					}
				}
				if (node.direct.right) {
					for (size_t ix = 1; x + ix < WIDTH; ix++)
					{
						auto& node2 = _data[x + ix][y];
						if (!node2)continue;
						if (!node2.direct.left)continue;
						if (std::find(_roads.begin(), _roads.end(), Point{ (float)x + ix,(float)y }) == _roads.end())
							_roads.emplace_back(x + ix, y);
						node.AddNext({ (float)(x + ix),(float)y });
						node2.AddNext({ (float)x,(float)(y) });
						node.direct.right = false;
						node2.direct.left = false;
						break;
					}
				}
			}
		}
	}
private:
	__grid _data[WIDTH][HEIGHT];
	Point _start, _end;
	std::vector<Point> _treasures;
	std::vector<Point> _walls, _roads;
};

OPT_END

#endif // !__HEAD_OpticalDesign_Maze
