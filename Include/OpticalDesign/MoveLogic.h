#pragma once

#include "NGS/NGS.h"
#include "OpticalDesign/Config.h"
#include "OpticalDesign/Setting.h"
#include "OpticalDesign/Converter.h"
#include "OpticalDesign/Path.h"
#include "OpticalDesign/Maze.h"

OPT_BEGIN

//实际点
//拐点
//节点

class MoveLogic {
public:
	NGS_TYPE_DEFINE(RouteInfo, route);
	NGS_TYPE_DEFINE(std::vector<__route_ptr_cst>, routes);

public:
	~MoveLogic() {
		if (_curRoute) {
			ngs::Delete(_curRoute);
			_curRoute = nullptr;
		}
	}
	void Initialize(Maze* maze) {
		_maze = maze;
		ngs::nos.Log("MoveLogic::Initialize", "初始化运动逻辑...\n");
		_BuildPathTree();

		_PathPlanning(_maze->GetStart());
		ngs::nos.Log("MoveLogic::Initialize", "运动逻辑初始化成功\n");
	}
	void Update(const Point& actualPos) {
		//if (!_maze->At(GetDst()).IsTreasure() && !_maze->At(GetDst()).IsEnd()) {
		//	//维护路线图，减去不必要的枝
		//	ngs::nos.Log("MoveLogic::Update", "目标点:(%d,%d)[%d] 不是宝藏，重新进行路径规划\n", GetDst().x, GetDst().y, _maze->At(GetDst()).data);
		//	auto needRemoved = _pathMap.GetNodeByPos(GetDst());
		//	if (needRemoved)_pathMap.RemoveNode(needRemoved);
		//	needRemoved = _pathMap.GetNodeByPos(*_curInflectionPos);
		//	if (needRemoved)_pathMap.RemoveNode(needRemoved);
		//	_pathMap.Update();
		//	//路径规划
		//	//if (_pathMap.GetNodeByPos(_curLogicPos)->GetGrid()->IsTreasure())
		//		//_maze->Set(_curLogicPos, MazeGrid::road);
		//	_PathPlanning(actualPos);
		//	ngs::Assert(_IsOnPlanedRoute());
		//	return;
		//}
		//若未在积极拐点
		if (!IsArrivedDst()) {
			//若没有移动
			if (actualPos == _curLogicPos)return;
			//如果还在航线上
			if (_curRoute->Has(actualPos)) {
				//设置消极拐点
				auto it = _GetInflectionPoint(actualPos);
				//如果实际位置处于拐点上
				_curLogicPos = actualPos;
				if (it != _curRoute->points.end()) {
					_curInflectionPos = it;
					ngs::nos.Log("MoveLogic::Update", "当前逻辑点：(%d,%d),当前拐点:(%d,%d)", (int)_curLogicPos.x, (int)_curLogicPos.y, (int)_curInflectionPos->x, (int)_curInflectionPos->y);
					if (_curInflectionPos != _curRoute->points.end() - 1)ngs::nos.Trace("当前积极拐点:(%d,%d)", (int)GetNextPos().x, (int)GetNextPos().y);
					ngs::nos.Trace("\n");
				}
				return;
			}
			ngs::nos.Log("MoveLogic::Update", "已偏离航线，正在重新进行路径规划...\n");
			//重新进行路径规划
			_PathPlanning(actualPos);
			ngs::Assert(_IsOnPlanedRoute());
			return;
		}
		//如果已经到达地图终点
		else if (IsArrivedMazeEnd()) {
			return;
		}
		//如果到达目的地
		else {
			//维护路线图，减去不必要的枝
			ngs::nos.Log("MoveLogic::Update", "已到达目的地\n");
			_pathMap.RemoveNode(_pathMap.GetNodeByPos(_curLogicPos));
			_pathMap.RemoveNode(_pathMap.GetNodeByPos(*_curInflectionPos));
			_pathMap.Update();
			//路径规划
			if (_pathMap.GetNodeByPos(_curLogicPos)->GetGrid()->IsTreasure()) {
				_maze->SetSelfTreasure(_curLogicPos);
				_maze->FoundTreasure(_curLogicPos);
			}
			_PathPlanning(actualPos);
			ngs::Assert(_IsOnPlanedRoute());
			return;
		}
	}
	/**
	 * \brief 是否到达迷宫的终点
	 * \return
	 */
	bool IsArrivedMazeEnd()const {
		return _curLogicPos == _maze->GetEnd();
	}
	/**
	 * \brief 是否到达指定目的地
	 * \return
	 */
	bool IsArrivedDst()const {
		return _curLogicPos == _curRoute->GetB();
	}
	/**
	 * \brief 积极拐点是否是目的地
	 * \return
	 */
	bool NextPosIsArrivedDst()const {
		return (_curInflectionPos + 1) == _curRoute->points.end() - 1;
	}
	/**
	 * \brief 获取目标点
	 * \return
	 */
	Point GetDst()const { return _curRoute->points.back(); }
	/**
	 * \brief 获取当前逻辑位置
	 * \return
	 */
	Point GetLogicPos()const { return _curLogicPos; }
	/**
	 * \brief 获取当前的积极拐点
	 * \return
	 */
	Point GetNextPos()const {
		ngs::Assert(_curInflectionPos != _curRoute->points.end() - 1);
		return *(_curInflectionPos + 1);
	}
	/**
	 * \brief 获取当前的消极拐点
	 *
	 * \return
	 */
	Point GetNegativePos()const {
		return *_curInflectionPos;
	}
private:
	void _BuildPathTree() {
		ngs::nos.Log("MoveLogic::_BuildPathTree", "正在构建图...\n");
		std::vector<Point>
			deadEnds,
			roads = _maze->GetRoads();
		std::vector<const PathNode*>
			nodes;

		for (auto& i : roads) {
			MazeGrid& grid = _maze->At(i);
			if (!grid.IsPathNode())continue;
			auto node = _pathMap.CreateNode(i, &grid);
			nodes.push_back(node);

			if (grid.IsDeadEnd() && !grid.IsTreasure() && !grid.IsStart() && !grid.IsEnd())deadEnds.push_back(i);
		}

		for (auto node : nodes) {
			std::vector<RouteInfo> routes = _maze->StraightTo(node->GetPosition());
			for (auto& route : routes) {
				const PathNode* node2 = _pathMap.GetNodeByPos(route.GetB());
				if (_pathMap.GetNodeRoute(node, node2))continue;
				_pathMap.LinkNode(node, node2, route);
			}
		}
		for (auto& deadEnd : deadEnds) {
			auto node = _pathMap.GetNodeByPos(deadEnd);
			_pathMap.RemoveNode(node);
		}
		_pathMap.Update();
		ngs::nos.Log("MoveLogic::_BuildPathTree", "图构建成功！包含节点%ld个\n", _pathMap.GetNodes().size());
	}

	void _PathPlanning(const Point& actualPos) {
		if (_curRoute)ngs::Delete(const_cast<__route_ptr>(_curRoute));
		__route temp;
		ngs::nos.Log("MoveLogic::_PathPlanning", "当前宝藏数量为%ld个,正在进行路径规划...\n", _maze->GetTreasures().size());
		if (_maze->GetTreasures().size())
			temp = _BFS_SearchNeritesTreasure(actualPos);
		else
			temp = _BFS_SearchMazeEnd(actualPos);

		_curRoute = ngs::New(new __route(temp));
		_curLogicPos = actualPos;
		for (_curInflectionPos = _curRoute->points.begin(); _curInflectionPos != _curRoute->points.end() - 1; _curInflectionPos++) {
			if (ngs::In(_curLogicPos, *(_curInflectionPos), *(_curInflectionPos + 1)) && _curLogicPos != *(_curInflectionPos + 1))break;
		}
		ngs::Assert(_curInflectionPos != _curRoute->points.end() - 1, "路径规划出错！");
		ngs::nos.Log("MoveLogic::_PathPlanning", "路径规划完成!\n");
	}
	__route _BFS_SearchNeritesTreasure(const Point& logicPos) {
		return _pathMap.BFS_ShortestPath(logicPos, [](const PathNode* node) {
			return node->GetGrid()->IsTreasure();
			});
	}
	__route _BFS_SearchMazeEnd(const Point& logicPos) {
		return _pathMap.BFS_ShortestPath(logicPos, [](const PathNode* node) {
			return node->GetGrid()->IsEnd();
			});
	}

	/**
	 * @brief 判断当前位置是否在预定路径上
	 *
	 * @return true 在预定路径上
	 * @return false 不在预定路径上
	 */
	bool _IsOnPlanedRoute()const {
		return _IsOnPlanedRoute(_curLogicPos);
	}
	/**
	 * @brief 判断一个点是否在当前航线上
	 *
	 * @param pos 点的位置
	 * @return true 在航线上
	 * @return false 不在航线上
	 */
	bool _IsOnPlanedRoute(const Point& pos)const {
		if (_curRoute->Has(pos))return true;
		return false;
	}

	/**
	 * \brief 判断某个点是否是拐点，并返回值
	 *
	 * \param pos
	 * \return 拐点的迭代器
	 */
	std::vector<Point>::const_iterator _GetInflectionPoint(const Point& pos)const {
		return std::find(_curRoute->points.begin(), _curRoute->points.end(), pos);
	}

public:
	Maze* _maze;

	PathMap _pathMap = {};
	//逻辑位置
	Point _curLogicPos;
	//当前所处消极拐点位置
	std::vector<Point>::const_iterator _curInflectionPos;
	__route_ptr _curRoute = nullptr;
};

OPT_END