#pragma once

#include "NGS/NGS.h"
#include "OpticalDesign/Config.h"
#include "OpticalDesign/Setting.h"
#include "OpticalDesign/Converter.h"
#include "OpticalDesign/Maze.h"

OPT_BEGIN

class PathMap;
class PathNode {
public:
	NGS_TYPE_DEFINE(PathNode, this);
	NGS_TYPE_DEFINE(MazeGrid, grid);
public:
	PathNode(__grid_ptr_cst grid, const Point& pos, bool active = true)
		: _grid(grid)
		, _pos(pos)
		, _active(active)
	{}

	Point& GetPosition() { return _pos; }
	const Point& GetPosition()const { return _pos; }

	__grid_ptr_cst GetGrid()const { return _grid; }

	bool isActive()const { return _active; }
	void SetActive(bool active) { _active = active; }
private:

private:
	__grid_ptr_cst _grid;
	Point _pos;
	bool _active;
};

class PathMap {
public:
	NGS_TYPE_DEFINE(PathNode, node);
	NGS_TYPE_DEFINE(RouteInfo, route);
	NGS_TYPE_DEFINE(std::vector<__route_ptr>, routes);
public:
	~PathMap() {
		for (auto& node : _nodes)
			ngs::Delete(const_cast<__node_ptr>(node));
		for (auto& route : _routes)
			ngs::Delete(route.second);
	}
	void Update() {
		ngs::nos.Log("PathMap::Update", "当前节点数量:%ld,开始剪枝...\n", _nodes.size());
		bool isRelink = false;
		do {
			isRelink = false;
			for (auto it = _nodes.begin(); it != _nodes.end();)
			{
				auto node = *it;
				__routes_ref_cst routesByNode = _routesByNode[node];
				if (routesByNode.size() != 2) {
					it++;
					continue;
				}
				const RouteInfo* routeA = routesByNode[0];
				const RouteInfo* routeB = routesByNode[1];

				__node_ptr_cst A = GetNodeByPos(routesByNode[0]->GetOther(node->GetPosition()));
				__node_ptr_cst B = GetNodeByPos(routesByNode[1]->GetOther(node->GetPosition()));

				ngs::nos.Log("PathMap::Update", "去除节点:(%d,%d)\n", (int)node->GetPosition().x, (int)node->GetPosition().y);
				_RelinkNodes(A, routeA, B, routeB, node);
				it = _nodes.erase(it);
				isRelink = true;
			}
		} while (isRelink);
		ngs::nos.Log("PathMap::Update", "剪枝完成！当前节点数量:%ld\n", _nodes.size());
	}

	void RemoveNode(__node_ptr_cst node) {
		(const_cast<__node_ptr>(node))->SetActive(false);
	}
	__node_ptr_cst CreateNode(const Point& pos, const MazeGrid* grid) {
		__node_ptr node = ngs::New(new __node(grid, pos, true));
		_nodes.insert(node);
		return node;
	}
	__node_ptr_cst GetNodeByPos(const Point& pos) {
		auto it = std::find_if(_nodes.begin(), _nodes.end(), [&](__node_ptr_cst node) {
			return node->GetPosition() == pos;
			});
		if (it == _nodes.end())return nullptr;
		return *it;
	}
	__node_ptr_cst GetNodeByPos(const Point& pos)const {
		auto it = std::find_if(_nodes.begin(), _nodes.end(), [&](__node_ptr_cst node) {
			return node->GetPosition() == pos;
			});
		if (it == _nodes.end())return nullptr;
		return *it;
	}
	RouteInfo* GetNodeRoute(__node_ptr_cst A, __node_ptr_cst B) {
		auto key = _GetRoutesKey(A, B);
		auto it = _routes.find(key);
		if (it == _routes.end())return nullptr;
		return it->second;
	}
	void LinkNode(__node_ptr_cst A, __node_ptr_cst B, __route_ref_cst route) {
		auto key = _GetRoutesKey(A, B);
		if (_routes.find(key) != _routes.end()) {
			ngs::nos.Warning("the route is already exist\n");
			return;
		}
		__route_ptr newRoute = ngs::New(new __route(route));
		if (B->isActive())_routesByNode[A].push_back(newRoute);
		if (A->isActive())_routesByNode[B].push_back(newRoute);

		_routes[key] = newRoute;
	}
	const std::unordered_map<std::string, __route_ptr>& GetRoutes()const { return _routes; }
	const std::unordered_set<__node_ptr_cst> GetNodes()const { return _nodes; }
	const std::unordered_map<__node_ptr_cst, __routes>& GetRoutesByNode()const { return _routesByNode; }

	__routes_ref_cst GetRoutesByNode(__node_ptr_cst node)const {
		auto it = _routesByNode.find(node);
		if (it == _routesByNode.end())return _nullRoute;
		return it->second;
	}
	__route BFS_ShortestPath(const Point& logicPos, const Point& dst) const {
		return BFS_ShortestPath(logicPos, [&dst](__node_ptr_cst node)->bool {
			return node->GetPosition() == dst;
			});
	}

	__route BFS_ShortestPath(const Point& logicPos, std::function<bool(__node_ptr_cst)> judger) const {
		ngs::MeasureExecutionTime<std::chrono::microseconds> measure;
		struct NodeInfo {
			std::vector<__route_ptr_cst> routes;
			size_t count;

			bool IsVisited(const Point& pos)const {
				for (auto& route : routes) {
					if (find(route->points.begin(), route->points.end(), pos) != route->points.end())return true;
				}
				return false;
			}
		};
		NGS_TYPE_DEFINE(NodeInfo, nodeInfo);
		ngs::nos.Log("PathMap::BFS_ShortestPath", "广度优先查找，起始点（%d,%d)\n", (int)logicPos.x, (int)logicPos.y);

		__node_ptr_cst startNode = GetNodeByPos(logicPos);
		if (!startNode) {
			ngs::nos.Log("PathMap::BFS_ShortestPath", "未找到位于(%d,%d)的节点，正在重新构建节点...\n", (int)logicPos.x, (int)logicPos.y);
			__route_ptr_cst route = _GetRouteByPos(logicPos);
			ngs::Assert(route, "不在迷宫内");
			Point newStart =
				((route->GetA() - logicPos).LengthSquared() < (route->GetB() - logicPos).LengthSquared()) ?
				route->GetA() : route->GetB();
			__route newRoute = BFS_ShortestPath(newStart, judger);
			if (!newRoute.Has(logicPos))newRoute = *route + newRoute;
			return newRoute;
		}

		std::queue<__node_ptr_cst> queue;
		queue.push(startNode);
		__nodeInfo shortestPath = {};
		shortestPath.count = std::numeric_limits<size_t>::max();

		bool isIncrease = false;

		std::unordered_map<__node_ptr_cst, __nodeInfo> nodeInfoBuffer;
		nodeInfoBuffer[startNode] = { {},0 };

		ngs::nos.Log("PathMap::BFS_ShortestPath", "开始遍历\n");

		size_t level = 0;
		while (queue.size()) {
			//当前层的其中一个节点
			__node_ptr_cst node = queue.front();
			queue.pop();

			//当前节点可走的路线
			__routes_ref_cst routesByNode = GetRoutesByNode(node);

			if (!nodeInfoBuffer.contains(node)) {

				continue;
			}
			//走到当前节点需要经过的路线
			__nodeInfo_ref nodeInfo = nodeInfoBuffer[node];

			std::unordered_set< __node_ptr_cst> needPush = {};
			//遍历当前节点可达到的下一层节点
			for (__route_ptr route : routesByNode) {
				//获取下一层节点
				Point nextPos = route->GetOther(node->GetPosition());
				__node_ptr_cst nextNode = GetNodeByPos(nextPos);

				if (node->isActive() && !nextNode->isActive())continue;
				//如果下一层节点已经被访问过，则跳过
				if (nodeInfo.IsVisited(nextPos)) continue;

				if (nodeInfoBuffer.contains(nextNode)) {
					size_t count = nodeInfo.count + route->count;
					if (count >= nodeInfoBuffer[nextNode].count)continue;
				}

				//创建新的节点路径数据
				__nodeInfo_ref nextNodeInfo = nodeInfoBuffer[nextNode];
				//新的路径数据要继承父节点的路径
				nextNodeInfo = nodeInfo;
				nextNodeInfo.routes.push_back(route);
				nextNodeInfo.count += route->count;

				//如果下一层节点的路径长度大于当前最短路径长度，则跳过
				if (nextNodeInfo.count >= shortestPath.count) {
					//清理数据
					nodeInfoBuffer.erase(nextNode);
					continue;
				}

				//到达当前位置时，节点满足以下条件
				// 1> 到达该节点的代价要比到达已有的最短路径的代价要小 

				//如果这个下一层节点是目标点，则替换掉原有的最短路径
				if (judger(nextNode)) {
					shortestPath = nextNodeInfo;
					ngs::nos.Log("PathMap::BFS_ShortestPath", "找到目标点！目标点：（%d,%d)，代价：%ld\n", (int)nextPos.x, (int)nextPos.y, shortestPath.count);
					continue;
				}
				//如果不是目标点，需要继续接着搜索该节点以下的节点，将该节点加入队列
				needPush.insert(nextNode);
			}
			for (auto& needPushNode : needPush) {
				auto info = nodeInfoBuffer[needPushNode];
				if (info.count >= shortestPath.count)continue;
				//std::cout << "\t:(" << needPushNode->GetPosition().x << " , " << needPushNode->GetPosition().y << ")," << nodeInfoBuffer[needPushNode].count << std::endl;
				queue.push(needPushNode);
				//标记当前层的节点路径数据已经增加
				isIncrease = true;
			}
			//删除父节点的路径数据
			nodeInfoBuffer.erase(node);
			//如果当前层的节点路径数据没有增加，则说明已经搜索到最短路径，可以退出
			level++;
			if (!isIncrease)break;
		}
		ngs::nos.Log("PathMap::BFS_ShortestPath", "遍历完成\n");
		ngs::Assert(!shortestPath.routes.empty(), "路线规划失败，无路可走!");

		__route result = *shortestPath.routes.front();
		for (auto i = shortestPath.routes.begin() + 1; i != shortestPath.routes.end(); ++i) {
			result += **i;
		}
		if (result.GetA() != logicPos)std::reverse(result.points.begin(), result.points.end());
		ngs::nos.Log("PathMap::BFS_ShortestPath", "广度优先遍历成功，代价:%ld\n", shortestPath.count);
		for (auto& i : result.points) {
			ngs::nos.Trace("(%d,%d)\n", (int)i.x, (int)i.y);
		}
		ngs::Assert(result.GetA() == logicPos);
		ngs::Assert(judger(GetNodeByPos(result.GetB())));

		return result;
	}


private:
	__route_ptr_cst _GetRouteByPos(const Point& logicPos) const {
		for (const auto& pair : _routes) {
			auto route = pair.second;
			if (route->Has(logicPos))return route;
		}
		return nullptr;
	}
	void _RelinkNodes(__node_ptr_cst A, __route_ptr_cst routeA, __node_ptr_cst B, __route_ptr_cst routeB, __node_ptr_cst node) {
		__route newRoute = *routeA;
		newRoute += *routeB;
		LinkNode(A, B, newRoute);
		_RemoveNode(node);
	}
	void _RemoveNode(__node_ptr_cst node) {
		ngs::Assert(_nodes.contains(node), "node is not in this map");
		//RemoveNodeRoutes
		{
			auto it = _routesByNode.find(node);
			ngs::Assert(it != _routesByNode.end());
			for (auto& route : it->second) {
				__node_ptr_cst other = GetNodeByPos(route->GetOther(node->GetPosition()));
				_routesByNode[other].erase(std::find(_routesByNode[other].begin(), _routesByNode[other].end(), route));
				_routes.erase(_GetRoutesKey(GetNodeByPos(route->GetA()), GetNodeByPos(route->GetB())));
				ngs::Delete(route);
			}
			_routesByNode.erase(it);
		}
		ngs::Delete(const_cast<__node_ptr>(node));
		_nodes.erase(node);
	}
	void _RemoveRoute(__route_ptr_cst route) {
		__node_ptr_cst A = GetNodeByPos(route->GetA());
		__node_ptr_cst B = GetNodeByPos(route->GetB());
		_RemoveRoute(A, B, route);
	}
	void _RemoveRoute(__node_ptr_cst A, __node_ptr_cst B, __route_ptr_cst route) {
		__routes_ref Ar = _routesByNode[A];
		__routes_ref Br = _routesByNode[B];
		Ar.erase(std::find(Ar.begin(), Ar.end(), route));
		Br.erase(std::find(Br.begin(), Br.end(), route));
		_routes.erase(_GetRoutesKey(A, B));
	}
	std::string _GetRoutesKey(__node_ptr_cst A, __node_ptr_cst B)const {
		size_t
			hashA = _nodes.hash_function()(A),
			hashB = _nodes.hash_function()(B);
		if (hashA < hashB)
			return std::to_string(hashA) + std::to_string(hashB);
		return std::to_string(hashB) + std::to_string(hashA);
	}
private:
	std::unordered_set<__node_ptr_cst> _nodes;
	std::unordered_map<std::string, __route_ptr> _routes;
	std::unordered_map<__node_ptr_cst, __routes> _routesByNode;
	inline static __routes_cst _nullRoute = {};
};

OPT_END
