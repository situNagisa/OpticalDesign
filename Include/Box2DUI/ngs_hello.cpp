#include "test.h"
#include "imgui/imgui.h"
#include "OpticalDesign/Brain.h"

class GameMainLogic : public Test {
public:

	static Test* Create() {
		return new GameMainLogic();
	}
public:
	optical_design::Brain brain;
	GameMainLogic()
		: brain()
	{
		m_world->SetGravity({ 0,0 });

		_t = brain._world;
		brain._world = m_world;
		while (!brain.TryToStart());

		g_camera.m_center = optical_design::ngs_b2::TransformToBox2D({ 10,10 });
		g_camera.m_zoom = 0.1f;
	}
	optical_design::Point
		cur, next;
	virtual ~GameMainLogic()override {
		brain._world = _t;
	}
	void Step(Settings& settings) override {
		brain.Update();
		if (!brain._moveLogic->IsArrivedDst()) {
			cur = brain._moveLogic->GetLogicPos();
			next = brain._moveLogic->GetNextPos();
			g_debugDraw.DrawSegment(optical_design::ngs_b2::TransformToBox2D(cur), optical_design::ngs_b2::TransformToBox2D(next), { 0.5,0.5,0.5 });
			g_debugDraw.DrawPoint(optical_design::ngs_b2::TransformToBox2D(cur), 2, { 1,1,1 });
			g_debugDraw.DrawPoint(optical_design::ngs_b2::TransformToBox2D(next), 2, { 1,1,1 });
		}
		for (auto& i : brain._eyes->GetMaze()->GetTreasures()) {
			g_debugDraw.DrawPoint(optical_design::ngs_b2::TransformToBox2D(i), 5, { 0xFF,0,1 });
		}

		Test::Step(settings);
	}
	void DrawRoute(const optical_design::RouteInfo* route) const {
		for (auto it = route->points.begin(); it != route->points.end() - 1; it++)
		{
			g_debugDraw.DrawSegment(optical_design::ngs_b2::TransformToBox2D(*it), optical_design::ngs_b2::TransformToBox2D(*(it + 1)), { 0.5,0.5,0.5 });
		}
		g_debugDraw.DrawPoint(optical_design::ngs_b2::TransformToBox2D(route->GetA()), 2, { 1,1,1 });
		g_debugDraw.DrawPoint(optical_design::ngs_b2::TransformToBox2D(route->GetB()), 2, { 1,1,1 });
	}
	/*void SetCarRotate(int direct = 1) {
		float radian_per_second = ngs::AsRadian(60) * direct;
		_self.GetBody()->SetAngularVelocity(radian_per_second);
	}
	void SetCarMove(int direct = 1) {
		float velocity = 1.0f * direct;
		_self.GetBody()->SetLinearVelocity({ velocity * ngs::Cos(_self.Angle()), velocity * ngs::Sin(_self.Angle()) });
	}

	void SetCarRotateTo(const Point& dst) {
		auto pos = _self.Position();
		auto angle = ngs::AsDegree(atan2(dst.y - pos.y, dst.x - pos.x));
		ngs::type::INT32 delta = angle - ngs::AsDegree(_self.Angle());
		delta %= 360;
		if (abs(delta) < 10) {
			SetCarRotate(0);
			return;
		}
		SetCarRotate(delta > 0 ? 1 : -1);
	}
	void SetCarMoveTo(const Point& dst) {
		SetCarRotateTo(dst);
		if (_self.GetBody()->GetAngularVelocity())
			return;
		auto pos = ngs_b2::Convert(_self.Position());
		auto distance = (dst - pos).Length();
		if (distance < 0.05) {
			SetCarMove(0);
			return;
		}
		SetCarMove(1);
	}

	void Step(Settings& settings) override {
		Test::Step(settings);

		Point dst(3, 17);
		_maze.Transform(dst);
		SetCarMoveTo(dst);

		g_debugDraw.DrawString(5, m_textLine, "car liner velocity: %f %f", _self.GetBody()->GetLinearVelocity().x, _self.GetBody()->GetLinearVelocity().y);
		m_textLine += m_textIncrement;
		g_debugDraw.DrawString(5, m_textLine, "car angular velocity: %f", _self.GetBody()->GetAngularVelocity());
		m_textLine += m_textIncrement;
	}*/
private:
	b2World* _t;
private:
};

static int testIndex = RegisterTest("ngs", "Game Main Logic", GameMainLogic::Create);