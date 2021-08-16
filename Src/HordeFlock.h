
#pragma once

#include "Root/Scenes/SceneEntity.h"
#include "Support/MetaData.h"
#include "root/Assets/AssetTexture.h"
#include "root/Assets/AssetAnimGraph2D.h"
#include "root/Physics/PhysController.h"
#include "root/Physics/PhysScene.h"

namespace Oak
{
	class HordeFlock : public SceneEntity
	{
	public:

		META_DATA_DECL_BASE(HordeFlock)

		enum class State 
		{
			Idle,
			Patroling,
			Pursuit,
			MelleAttack,
			GoingToGhost,
			WaitingForTarget
		};

		AssetAnimGraph2DRef anim;

		struct Orc
		{
			PhysScene::BodyUserData bodyData;
			PhysController* controller = nullptr;

			State state = State::Patroling;

			AssetAnimGraph2DRef anim;

			int targetID = -1;

			int hp = 1;
			Math::Vector3 moveDir;

			Math::Vector3 lastPoint;
			float stucked = 0.0f;
			Math::Vector3 patrolTarget = 0.0f;

			Math::Vector3 lastTargetPos = 0.0f;

			float flyTime = 0.65f;
			float actionTime = 0.0f;
			float actionTime2 = 0.0f;
			float actionData = 0.0f;

			Transform transform;
			int lightID = -1;

			void Hit(Math::Vector3 dir, int damage);
			void MakeDead(Math::Vector3 dir);
			void CommandMe(HordeFlock* flock, float dt);

			void OnFrameChangeCallback(int frame, eastl::string& name, eastl::string& param);
		};

		int orcsCount = 1;
		int maxHP = 1;
		float patrolRadius = 100.0f;
		
		eastl::vector<Orc> orcs;

	#ifndef DOXYGEN_SKIP

		HordeFlock();
		virtual ~HordeFlock() = default;

		void Init() override;
		void Play() override;
		void Draw(float dt);
		Math::Vector3 ChoosePatrolTraget();
		void Restart();
	#endif
	};
}