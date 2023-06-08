#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <deque>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/astar_search.hpp>//A*Ѱ·�㷨
using namespace std;
using namespace boost;

enum { A, B, C, D, E, N };
string Names = "ABCDE";
//�������,��������ֱ�����ӵı�
using Edge = pair<int, int>;
//����һ��ͼ                 ��     ����    �з���   ������            �ߵ�Ȩ����int
using Graph = adjacency_list<listS, vecS, directedS, no_property, property<edge_weight_t, int>>;

//����һ��ͼ
Graph make_graph()
{
	//���ӵı�
	vector<Edge> edges = { {A,B}, {A,C},{A,D},{B,E},{C,E},{D,E} };
	//�߶�Ӧ��Ȩ��
	vector<int> weight = { 3,1,4,5,2,6 };
	//����һ��ͼ����
	return Graph(edges.begin(), edges.end(), weight.begin(), N);
}
//����һ���ṹ��,�����׳��ҵ���Ϣ
struct found_goal
{

};
//A*Ҫ�����Ŀ�궥��
template<class vertex>
class astar_my_visitor :public boost::default_astar_visitor
{
public:
	//��ʼ�����õ�ͼ
	astar_my_visitor(vertex goal) :m_goal(goal)
	{

	}
	//����examine_vertex����
	template<class Graph>
	void examine_vertex(vertex v, Graph& g)
	{
		//�����Ŀ�궥��һ��,��˵���ҵ�
		if (v == m_goal)
		{
			//�׳��׳��ҵ���Ϣ
			throw found_goal();
		}
	}
private:
	//Ŀ�궥��
	vertex m_goal;
};

//����Ȩ��Ѱ�����·��
template<class Graph, class costtype>
class distance_heuristic :public boost::astar_heuristic<Graph, costtype>
{
public:
	//�����滻
	using Vertex = typename boost::graph_traits<Graph>::vertex_descriptor;
	//��ʼ��
	distance_heuristic(Vertex Goal, Graph& graph) :Goal_(Goal), graph_(graph)
	{
	}
	//����()����� ���Ŀ��㵽ָ����ľ���
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
	//����ͼ
	Graph myg = make_graph();

	//������д
	using Vertex = boost::graph_traits<Graph>::vertex_descriptor;
	using Cost = int;

	Vertex start = vertex(A, myg);//��ʼλ��
	Vertex goal = vertex(E, myg);//����λ��

	//�����߹�·��(�ɺ���ǰ)
	vector<Vertex>parents(boost::num_vertices(myg));
	//���泤��
	vector<Cost>distance(boost::num_vertices(myg));

	try
	{
		//���ָ���㵽�յ��·��
		boost::astar_search_tree(myg, start, distance_heuristic<Graph, Cost>(goal, myg),//���ݾ���
			//���·�����Լ�·����Ӧ��Ȩ�أ�����������                                 ��Ϊ������()�����
			boost::predecessor_map(&parents[0]).distance_map(&distance[0]).visitor(astar_my_visitor<Vertex>(goal))
		);
	}
	//catch��Ϣ
	catch (found_goal fg)
	{
		//Ҫ����λ�õ�ǰһ�������λ�������goal(�±��ǵ�ǰ��,ֵ�ǵ������֮ǰ�ĵ�)
		if (parents[goal] == goal)
		{
			cout << "��·����" << endl;
		}
		deque<Vertex> route;
		//˳������
		for (Vertex v = goal; v != start; v = parents[v])
		{
			route.push_front(v);
		}
		cout << "��" << Names[start] << "��" << Names[goal] << "�����·��Ϊ��" << endl;
		cout << Names[start];
		for (auto i : route)
		{
			cout << "->" << Names[i];
		}
		cout << endl;
	}
	return 0;
}