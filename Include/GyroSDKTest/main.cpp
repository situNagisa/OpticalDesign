#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <deque>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/astar_search.hpp>//A*寻路算法
using namespace std;
using namespace boost;

enum { A, B, C, D, E, N };
string Names = "ABCDE";
//定义别名,两个顶点直接连接的边
using Edge = pair<int, int>;
//创建一个图                 边     顶点    有方向   无特性            边的权重是int
using Graph = adjacency_list<listS, vecS, directedS, no_property, property<edge_weight_t, int>>;

//创建一个图
Graph make_graph()
{
	//连接的边
	vector<Edge> edges = { {A,B}, {A,C},{A,D},{B,E},{C,E},{D,E} };
	//边对应的权重
	vector<int> weight = { 3,1,4,5,2,6 };
	//创建一个图对象
	return Graph(edges.begin(), edges.end(), weight.begin(), N);
}
//创建一个结构体,用于抛出找到信息
struct found_goal
{

};
//A*要到达的目标顶点
template<class vertex>
class astar_my_visitor :public boost::default_astar_visitor
{
public:
	//初始化内置地图
	astar_my_visitor(vertex goal) :m_goal(goal)
	{

	}
	//重载examine_vertex方法
	template<class Graph>
	void examine_vertex(vertex v, Graph& g)
	{
		//如果与目标顶点一样,则说明找到
		if (v == m_goal)
		{
			//抛出抛出找到信息
			throw found_goal();
		}
	}
private:
	//目标顶点
	vertex m_goal;
};

//计算权重寻找最短路径
template<class Graph, class costtype>
class distance_heuristic :public boost::astar_heuristic<Graph, costtype>
{
public:
	//类型替换
	using Vertex = typename boost::graph_traits<Graph>::vertex_descriptor;
	//初始化
	distance_heuristic(Vertex Goal, Graph& graph) :Goal_(Goal), graph_(graph)
	{
	}
	//重载()运算符 获得目标点到指定点的距离
	costtype operator()(Vertex v)
	{
		return get(vertex_index, graph_, Goal_) - get(vertex_index, graph_, v);
	}
private:
	Vertex Goal_;
	Graph& graph_;
};


int main()
{
	//创建图
	Graph myg = make_graph();

	//创建简写
	using Vertex = boost::graph_traits<Graph>::vertex_descriptor;
	using Cost = int;

	Vertex start = vertex(A, myg);//开始位置
	Vertex goal = vertex(E, myg);//结束位置

	//保存走过路径(由后向前)
	vector<Vertex>parents(boost::num_vertices(myg));
	//保存长度
	vector<Cost>distance(boost::num_vertices(myg));

	try
	{
		//求从指定点到终点的路线
		boost::astar_search_tree(myg, start, distance_heuristic<Graph, Cost>(goal, myg),//传递距离
			//求出路径，以及路径对应的权重，访问器访问                                 因为重载了()运算符
			boost::predecessor_map(&parents[0]).distance_map(&distance[0]).visitor(astar_my_visitor<Vertex>(goal))
		);
	}
	//catch信息
	catch (found_goal fg)
	{
		//要到的位置的前一个到达的位置如果是goal(下标是当前点,值是到这个点之前的点)
		if (parents[goal] == goal)
		{
			cout << "无路可走" << endl;
		}
		deque<Vertex> route;
		//顺藤摸瓜
		for (Vertex v = goal; v != start; v = parents[v])
		{
			route.push_front(v);
		}
		cout << "从" << Names[start] << "到" << Names[goal] << "的最短路径为：" << endl;
		cout << Names[start];
		for (auto i : route)
		{
			cout << "->" << Names[i];
		}
		cout << endl;
	}
	return 0;
}