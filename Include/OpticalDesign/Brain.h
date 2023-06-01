#pragma once

#include "NGS/NGS.h"
#include "OpticalDesign/Config.h"
#include "OpticalDesign/Setting.h"
#include "OpticalDesign/Converter.h"
#include "OpticalDesign/Eyes.h"
#include "OpticalDesign/Path.h"
#include "OpticalDesign/Cerebellum.h"
#include "OpticalDesign/MoveLogic.h"
#include "OpticalDesign/Foot.h"

OPT_BEGIN

class Brain {
public:
	NGS_TYPE_DEFINE(RouteInfo, route);
	NGS_TYPE_DEFINE(std::vector<__route_ptr_cst>, routes);
public:
	Brain()
	{
		_world = ngs::New(new b2World({ 0,0 }));
		_cerebellum = ngs::New(new Cerebellum());
		_eyes = ngs::New(new Eyes());
		_foots = ngs::New(new Foot());
		_moveLogic = ngs::New(new MoveLogic());

		_timeRecord = std::chrono::high_resolution_clock::now();
	}
	~Brain() {
		ngs::Delete(_world);
		ngs::Delete(_eyes);
		ngs::Delete(_cerebellum);
		ngs::Delete(_foots);
		ngs::Delete(_moveLogic);
	}

	void Update() {
		//ngs::MeasureExecutionTime<std::chrono::microseconds> time;
		//_eyes->Update();

		_UpdateCerebellum();

		_UpdateBox2D();

		if (IsInMaze())
			_UpdateLogic();

		_UpdateFoots();

		//ngs::nos.Log("Brain::Update", "angle velocity:%f\n", _cerebellum->GetAngularVelocity());
		//ngs::nos.Log("Brain::Update", "acceleration:%f,%f\n", _cerebellum->GetAcceleration().x, _cerebellum->GetAcceleration().y);
	}


	bool TryToStart() {
		if (!_eyes->Initialize())return false;
		_eyes->SetMode(Eyes::Mode::maze);
		_eyes->Update();
		if (!_eyes->GetMaze())return false;
		_eyes->SetMode(Eyes::Mode::runtime);

		ngs::nos.Log("Brain::TryToStart", "识别迷宫成功\n");
		_Initialize();

		return true;
	}

public:
	void _Initialize() {
		ngs::nos.Log("Brain::TryToStart", "🧠开始初始化...\n");
		_moveLogic->Initialize(_eyes->GetMaze());
		_cerebellum->Initialize();
		_BuildBox2dWorld();
		ngs::nos.Log("Brain::_Initialize", "此平台支持的最大线程数为：%d\n", std::jthread::hardware_concurrency());

		ngs::nos.Log("Brain::_Initialize", "🧠初始化完成\n");
	}

	void _UpdateCerebellum() {
		_cerebellum->Update();
		//_body->ApplyForceToCenter(ngs_b2::TransformToBox2D(_cerebellum->GetAcceleration() * _body->GetMass()), true);
		_body->SetAngularVelocity(_cerebellum->GetAngularVelocity());
		auto linearVelocity = _foots->GetLinearVelocity();
		_body->SetLinearVelocity({ linearVelocity * std::cos(_body->GetAngle()),linearVelocity * std::sin(_body->GetAngle()) });
	}
	void _UpdateBox2D() {
		auto now = std::chrono::high_resolution_clock::now();
		ngs::float64 timeStep = std::chrono::duration_cast<std::chrono::nanoseconds>(now - _timeRecord).count();

		_timeRecord = now;
		timeStep /= 1000'000'000;
		_world->Step(timeStep, 8, 3);
	}
	bool IsInMaze()const {
		auto pos = ngs_b2::TransformToNGS(_body->GetPosition());
		auto maze = _eyes->GetMaze()->GetBounds();
		if (!ngs::Between(pos.x, maze.Left(), maze.Right()))return false;
		if (!ngs::Between(pos.y, maze.Top(), maze.Bottom()))return false;
		return true;
	}
private:
	void _UpdateLogic() {
		_moveLogic->Update(ngs_b2::TransformToNGS(_body->GetPosition()));
		if (_moveLogic->IsArrivedDst()) {
			return;
		}
		_nextPos = _moveLogic->GetNextPos();
		if (_foots->GetMode() != Foot::Mode::Rotating) {
			_UpdateSeenTreasureLogic();
		}
	}
	void _UpdateSeenTreasureLogic() {
		if (!_eyes->HasSeenTreasure())return;
		if (!_moveLogic->NextPosIsArrivedDst()) {
			ngs::nos.Error("看到宝藏，但却不是当前目标点\n");
			return;
		}
		auto& maze = *_eyes->GetMaze();
		auto treasurePos = _moveLogic->GetDst();

		if (_eyes->IsSelfTreasure()) {
			maze.SetSelfTreasure(treasurePos);
			return;
		}
		maze.SetTargetTreasure(treasurePos);
	}
	void SetCarRotateTo(const b2Vec2& delta) {
		auto angle = ngs::AsDegree(atan2(delta.y, delta.x));
		if (angle < 0)angle += 360;
		auto a = ngs::AsDegree(_body->GetAngle());
		if (a < 0)a += 360;
		ngs::int32 deltaAngle = angle - a;
		if (ngs::Abs(deltaAngle % 360) < 6) {
			_body->SetAngularVelocity(0);
			return;
		}
		_body->SetAngularVelocity((deltaAngle > 0 ? 3 : -3) * ((deltaAngle % 360) > 180 ? -1 : 1));
	}
	void SetCarMoveTo(b2Vec2 delta) {
		SetCarRotateTo(delta);
		if (_body->GetAngularVelocity())
			return;
		auto distance = delta.Length();
		if (distance < 0.05) return;
		_body->SetLinearVelocity({ 2 * ngs::Cos(_body->GetAngle()),2 * ngs::Sin(_body->GetAngle()) });
	}
	void _UpdateFoots() {
		_foots->MoveTo(_nextPos);
		_foots->Update(ngs_b2::Convert(_body->GetPosition()), _body->GetAngle());
	}

	void _BuildBox2dWorld() {
		ngs::nos.Log("Brain::_BuildBox2dWorld", "初始化物理引擎 Box2D...\n");
		{
			auto walls = _eyes->GetMaze()->GetWalls();
			b2BodyDef bd;
			b2Body* ground = _world->CreateBody(&bd);

			b2EdgeShape shape;

			for (auto& start : walls) {
				for (auto& end : (*_eyes->GetMaze())(start.x, start.y).Next()) {
					shape.SetTwoSided(ngs_b2::TransformToBox2D(start), ngs_b2::TransformToBox2D(end));
					ground->CreateFixture(&shape, 0.0f);
				}
			}
		}
		{
			b2BodyDef bd;
			bd.type = b2_dynamicBody;
			bd.position = ngs_b2::TransformToBox2D(_eyes->GetMaze()->GetStart());
			bd.allowSleep = false;
			_body = _world->CreateBody(&bd);
			b2PolygonShape shape;
			shape.SetAsBox(CAR_SIZE_HALF.x, CAR_SIZE_HALF.y);
			_body->CreateFixture(&shape, CAR_DENSITY);

			//_body->SetAngularDamping(10);
			//_body->SetLinearDamping(10);
		}
		ngs::nos.Log("Brain::_BuildBox2dWorld", "物理引擎 Box2D 初始化成功\n");
	}
private:
	std::chrono::high_resolution_clock::time_point _timeRecord;
	b2World* _world = nullptr;
	b2Body* _body = nullptr;

	Cerebellum* _cerebellum = nullptr;

	Eyes* _eyes = nullptr;

	Foot* _foots = nullptr;

	MoveLogic* _moveLogic = nullptr;

	Point _nextPos = {};
};

OPT_END
