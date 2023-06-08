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
#include "OpticalDesign/GrayscaleLogic.h"

OPT_BEGIN

class Brain {
public:
	NGS_TYPE_DEFINE(RouteInfo, route);
	NGS_TYPE_DEFINE(std::vector<__route_ptr_cst>, routes);
public:
	Brain()
	{
		//_world = ngs::New(new b2World({ 0,0 }));
		_cerebellum = ngs::New(new Cerebellum());
		_eyes = ngs::New(new Eyes());
		_foots = ngs::New(new Foot());
		_moveLogic = ngs::New(new MoveLogic());
		_grayscale = ngs::New(new GrayscaleLogic());

		_timeRecord = std::chrono::high_resolution_clock::now();
	}
	~Brain() {
		//ngs::Delete(_world);
		ngs::Delete(_eyes);
		ngs::Delete(_cerebellum);
		ngs::Delete(_foots);
		ngs::Delete(_moveLogic);
		ngs::Delete(_grayscale);
	}
	enum class State {
		Forward,
		Rotate,
	};
	State state = State::Forward;
	int rotate_direct = 1;
	void Update() {
		//ngs::MeasureExecutionTime<std::chrono::microseconds> time;
		if (_moveLogic->NextPosIsArrivedDst()) {
			_eyes->Update();
		}


		//std::system("clear");
		_UpdateCerebellum();

		//_UpdateBox2D();

		_grayscale->Update();

		//ngs::nos.Trace("%d \n", state);

		_UpdateLogic();
		/*ngs::nos.Log("Brain::Update", "angle:%d %d\n", ngs::AsDegree(_body->GetAngle()), ngs::AsDegree(_body->GetAngularVelocity()));
		ngs::nos.Log("Brain::Update", "velocity:%f,%f\n", _body->GetLinearVelocity().x, _body->GetLinearVelocity().y);
		ngs::nos.Log("Brain::Update", "position %f,%f \n", _moveLogic->GetLogicPos().x, _moveLogic->GetLogicPos().y);*/
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
		//_BuildBox2dWorld();
		_position = _eyes->GetMaze()->GetStart();
		_moveLogic->Update(_position);
		_nextPos = _moveLogic->GetNextPos();
		_angle = (_nextPos - _position).ArcTan();
		ngs::nos.Log("Brain::_Initialize", "此平台支持的最大线程数为：%d\n", std::jthread::hardware_concurrency());

		ngs::nos.Log("Brain::_Initialize", "🧠初始化完成\n");
	}

	void _UpdateCerebellum() {
		_cerebellum->Update();
		//_body->ApplyForceToCenter(ngs_b2::TransformToBox2D(_cerebellum->GetAcceleration() * _body->GetMass()), true);
		///_body->SetAngularVelocity(_cerebellum->GetAngularVelocity());
		//auto linearVelocity = _foots->GetLinearVelocity();
		//_body->SetLinearVelocity({ linearVelocity * std::cos(_body->GetAngle()),linearVelocity * std::sin(_body->GetAngle()) });
	}
	/*void _UpdateBox2D() {
		auto now = std::chrono::high_resolution_clock::now();
		ngs::float64 timeStep = std::chrono::duration_cast<std::chrono::nanoseconds>(now - _timeRecord).count();

		_timeRecord = now;
		timeStep /= 1000'000'000;
		_world->Step(timeStep, 8, 3);
	}*/
	bool IsInMaze()const {
		return _eyes->GetMaze()->IsInMaze(_position);
	}
	void SetTeam(const Team& team) {
		_eyes->team = team;
	}
public:
	constexpr static size_t MAX_TIME = 2000;
	std::chrono::high_resolution_clock::time_point _rotate_start = std::chrono::high_resolution_clock::now();
	void _UpdateLogic() {
		if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - _rotate_start).count() < MAX_TIME)
			return;
		if (state == State::Forward) {
			if (!_grayscale->IsOnInflectionPoint()) {
				/*if (true || _grayscale->Left() == _grayscale->Right()) {
					_foots->MoveTo(_nextPos);
					_foots->Update(_position, _angle);
				}
				else if (_grayscale->Left()) {
					_foots->LittleRotate(1);
				}
				else if (_grayscale->Right()) {
					_foots->LittleRotate(-1);
				}*/
				_foots->MoveTo(_nextPos);
				_foots->Update(_position, _angle);
				return;
			}
		}
		_foots->StopTime();
		_rotate_start = std::chrono::high_resolution_clock::now();
		_position = _nextPos;
		if (IsInMaze()) {
			_moveLogic->Update(_position);
		}
		if (_moveLogic->IsArrivedDst()) {
			return;
		}
		_nextPos = _moveLogic->GetNextPos();
		if (_foots->GetMode() != Foot::Mode::Rotating) {
			_UpdateSeenTreasureLogic();
		}
		switch (state) {
		case State::Forward:
		{
			float aim = (_nextPos - _position).ArcTan();
			float cur = _angle;
			float delta = aim - cur;
			delta = fmod(delta, 2 * std::numbers::pi);
			if (delta > std::numbers::pi)
				delta -= 2 * std::numbers::pi;
			else if (delta < -std::numbers::pi)
				delta += 2 * std::numbers::pi;

			if (!delta)break;
			rotate_direct = ngs::Sign(delta);
			_foots->Rotate90(rotate_direct);
			_foots->StopTime();
			_angle += delta;
			_foots->MoveTo(_nextPos);
			_foots->Update(_position, _angle);
			ngs::nos.Trace("%f %f (%d,%d) (%d,%d)\n", _angle, delta, (int)_position.x, (int)_position.y, (int)_nextPos.x, (int)_nextPos.y);
			break;
		}
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
	/*void SetCarRotateTo(const b2Vec2& delta) {
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
	}*/
	/*void SetCarMoveTo(b2Vec2 delta) {
		SetCarRotateTo(delta);
		if (_body->GetAngularVelocity())
			return;
		auto distance = delta.Length();
		if (distance < 0.05) return;
		_body->SetLinearVelocity({ 2 * ngs::Cos(_body->GetAngle()),2 * ngs::Sin(_body->GetAngle()) });
	}*/

	//void _BuildBox2dWorld() {
	//	ngs::nos.Log("Brain::_BuildBox2dWorld", "初始化物理引擎 Box2D...\n");
	//	{
	//		auto walls = _eyes->GetMaze()->GetWalls();
	//		b2BodyDef bd;
	//		b2Body* ground = _world->CreateBody(&bd);
	//		b2EdgeShape shape;
	//		for (auto& start : walls) {
	//			for (auto& end : (*_eyes->GetMaze())(start.x, start.y).Next()) {
	//				shape.SetTwoSided(ngs_b2::TransformToBox2D(start), ngs_b2::TransformToBox2D(end));
	//				ground->CreateFixture(&shape, 0.0f);
	//			}
	//		}
	//	}
	//	{
	//		b2BodyDef bd;
	//		bd.type = b2_dynamicBody;
	//		bd.position = ngs_b2::TransformToBox2D(_eyes->start);
	//		bd.angle = -(_eyes->GetMaze()->GetStart() - _eyes->start).ArcTan();
	//		ngs::nos.Log("Brain::_BuildBox2dWorld", "angle:%f\n", bd.angle);
	//		bd.allowSleep = false;
	//		_body = _world->CreateBody(&bd);
	//		b2PolygonShape shape;
	//		shape.SetAsBox(CAR_SIZE_HALF.x, CAR_SIZE_HALF.y);
	//		_body->CreateFixture(&shape, CAR_DENSITY);

	//		//_body->SetAngularDamping(10);
	//		//_body->SetLinearDamping(10);
	//	}
	//	ngs::nos.Log("Brain::_BuildBox2dWorld", "物理引擎 Box2D 初始化成功\n");
	//}
public:
	Point _position = {};
	ngs::float32 _angle = 0.0f;
	std::chrono::high_resolution_clock::time_point _timeRecord;
	/*b2World* _world = nullptr;
	b2Body* _body = nullptr;*/

	Cerebellum* _cerebellum = nullptr;
	Eyes* _eyes = nullptr;
	Foot* _foots = nullptr;

	MoveLogic* _moveLogic = nullptr;
	GrayscaleLogic* _grayscale = nullptr;

	Point _nextPos = {};


};

OPT_END
