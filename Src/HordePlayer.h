
#pragma once

#include "Root/Scenes/SceneEntity.h"
#include "Support/MetaData.h"
#include "root/Assets/AssetTexture.h"
#include "root/Assets/AssetAnimGraph2D.h"
#include "root/Physics/PhysController.h"
#include "root/Physics/PhysScene.h"
#include "SceneEntities/UI/LabelWidget.h"

namespace Oak
{
	class HordePlayer : public SceneEntity
	{
	public:

		
		META_DATA_DECL_BASE(HordePlayer)

		PhysScene::BodyUserData bodyData;

		AssetAnimGraph2DRef anim;
		SceneEntityRef cursorRef;
		SceneEntityRef healthBarRef;
		SceneEntityRef selIconRef;
		SceneEntityRef useIconRef;
		SceneEntityRef amountHealthRef;
		SceneEntityRef amountLandMineRef;
		SceneEntityRef amountDecoyRef;

		PhysController* controller = nullptr;

		int heroCURSOR_X = -1;
		int heroCURSOR_Y = -1;
		int heroMoveHorz = -1;
		int heroMoveVert = -1;
		int heroFire = -1;
		int heroPrevItem = -1;
		int heroNextItem = -1;
		int heroUseItem = -1;

		Math::Vector3 moveDir;

		float timeToUseItem = 1.0f;
		float curTimeToUseItem = 0.0f;
		bool itemWasUse = false;

		int hp = 1;
		int maxHP = 1;
		float speed = 1.0f;
		float inHit = -1.0f;

		float flyTime = 0.65f;

		struct ItemDesc
		{
			eastl::string name;
			eastl::function<void()> action;
			float iconPosX = 0.0f;
			LabelWidget* amountLabel = nullptr;
			int amount = 0;
			int maxAmount = 5;
		};

		int curItem = 0;
		eastl::vector<ItemDesc> items;

	#ifndef DOXYGEN_SKIP

		HordePlayer();
		virtual ~HordePlayer() = default;

		void Init() override;
		void Restart(Math::Vector3 pos);
		void Play() override;
		void Draw(float dt);
		void OnFrameChangeCallback(int frame, eastl::string& name, eastl::string& param);
		void Hit(Math::Vector3 dir, int damage);
		void MakeDead(Math::Vector3 dir);
		void ControlPlayer(float dt);
		void UpdateIconItem();
	#endif
	};
}