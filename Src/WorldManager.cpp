
#include "WorldManager.h"
#include "HordeFlock.h"
#include "Root/Root.h"

namespace Oak
{
	CLASSREG(SceneEntity, WorldManager, "WorldManager")

	META_DATA_DESC(WorldManager)
		BASE_SCENE_ENTITY_PROP(WorldManager)

		SCENEOBJECT_PROP(WorldManager, titleScreenRef, "Refs", "title screen")
		SCENEOBJECT_PROP(WorldManager, helpScreenRef, "Refs", "help screen")
		SCENEOBJECT_PROP(WorldManager, themeMusicRef, "Refs", "Theme music")
		SCENEOBJECT_PROP(WorldManager, levelRef, "Refs", "level")
		SCENEOBJECT_PROP(WorldManager, playerRef, "Refs", "player")
		SCENEOBJECT_PROP(WorldManager, treasureRef, "Refs", "treasure")
		SCENEOBJECT_PROP(WorldManager, startRef, "Refs", "start")
		SCENEOBJECT_PROP(WorldManager, lightRendererRef, "Refs", "lights renderer")
		SCENEOBJECT_PROP(WorldManager, hordesRootRef, "Refs", "hords root")
		SCENEOBJECT_PROP(WorldManager, wastedRef, "Refs", "wasted label")
		SCENEOBJECT_PROP(WorldManager, victoryRef, "Refs", "victory label")
		SCENEOBJECT_PROP(WorldManager, escapeRef, "Refs", "escape label")

		ASSET_TEXTURE_PROP(WorldManager, escapeArrowTex, "Escape", "Arrow Texture")

		INT_PROP(WorldManager, hpAmountForPlayer, 1, "HP", "Amount for player", "Amount for player")
		FLOAT_PROP(WorldManager, hpIconPosX, 0.0f, "HP", "Icon pos x", "Icon pos x")

		ASSET_TEXTURE_PROP(WorldManager, landMineTexture, "Land Mine", "Texture")
		ASSET_TEXTURE_PROP(WorldManager, landMineBoomTex, "Land Mine", "Boom Texture")
		COLOR_PROP(WorldManager, landMineBoomFlareColor, COLOR_YELLOW, "Land Mine", "Boom color")
		FLOAT_PROP(WorldManager, landMineActivateRadius, 50.0f, "Land Mine", "Activation radius", "Activation radius")
		FLOAT_PROP(WorldManager, landMineBoomRadius, 300.0f, "Land Mine", "Boom radius", "Boom radius")
		INT_PROP(WorldManager, landMineAmountForPlayer, 1, "Land Mine", "Amount for player", "Amount for player")
		FLOAT_PROP(WorldManager, landMineIconPosX, 0.0f, "Land Mine", "Icon pos x", "Icon pos x")

		ASSET_TEXTURE_PROP(WorldManager, decoyTexture, "Decoy", "Texture")
		INT_PROP(WorldManager, decoyAmountForPlayer, 1, "Decoy", "Amount for player", "Amount for player")
		COLOR_PROP(WorldManager, decoyFlareColor, COLOR_YELLOW, "Decoy", "flare color")
		FLOAT_PROP(WorldManager, decoyFlareRadius, 50.0f, "Decoy", "flare radius", "flare radius")

		FLOAT_PROP(WorldManager, decoyIconPosX, 0.0f, "Decoy", "Icon pos x", "Icon pos x")

		FLOAT_PROP(WorldManager, orcViewDist, 400.0f, "Orcs", "view distance", "view dist")
		FLOAT_PROP(WorldManager, orcViewAngle, 65.0f, "Orcs", "view angle", "view angle")
		FLOAT_PROP(WorldManager, orcSpeedPatrol, 120.0f, "Orcs", "speed patrol", "speed oatrol")
		FLOAT_PROP(WorldManager, orcSpeedPursuit, 200.0f, "Orcs", "speed pursuit", "speed pursuit")
		FLOAT_PROP(WorldManager, orcAgroRadius, 700.0f, "Orcs", "agro radius", "agro radius")
		COLOR_PROP(WorldManager, orcFlareColor, COLOR_YELLOW, "Orcs", "flare color")
		FLOAT_PROP(WorldManager, orcFlareRadius, 50.0f, "Orcs", "flare radius", "flare radius")

	META_DATA_DESC_END()

	WorldManager* WorldManager::instance = nullptr;

	WorldManager::WorldManager() : SceneEntity()
	{
		//inst_class_name = "SpriteInst";
	}

	void WorldManager::Init()
	{
		transform.unitsScale = &Sprite::pixelsPerUnit;
		transform.unitsInvScale = &Sprite::pixelsPerUnitInvert;
		transform.transformFlag = TransformFlag::MoveXYZ | TransformFlag::RotateZ | TransformFlag::ScaleX | TransformFlag::ScaleY;

		Tasks(false)->AddTask(0, this, (Object::Delegate) & WorldManager::Work);
		Tasks(true)->AddTask(0, this, (Object::Delegate) & WorldManager::Draw);
		Tasks(true)->AddTask(150, this, (Object::Delegate) & WorldManager::DrawOverlay);

		instance = this;
	}

	void WorldManager::Play()
	{
		orcViewAngle *= Math::Radian;

		if (playerRef.entity)
		{
			player = dynamic_cast<HordePlayer*>(playerRef.entity);
		}

		if (hordesRootRef.entity)
		{
			for (auto& child : hordesRootRef.entity->GetChilds())
			{
				HordeFlock* flock = dynamic_cast<HordeFlock*>(child);

				if (flock)
				{
					flocks.push_back(flock);
				}
			}
		}
	}

	void WorldManager::Reset()
	{
		treasureGrabbed = false;

		for (auto& decoy : decoys)
		{
			ReleaseLightInstance(decoy.lightID);
		}

		decoys.clear();

		for (auto& mine : landMines)
		{
			ReleaseLightInstance(mine.lightID);
		}

		landMines.clear();

		decoyID = 0;

		if (treasureRef.entity)
		{
			treasureRef.entity->SetVisible(true);
		}

		if (startRef.entity)
		{
			player->Restart(startRef.entity->GetTransform().position);
		}

		for (auto flock : flocks)
		{
			flock->Restart();
		}
	}

	void WorldManager::PlaceLandMine(Math::Vector3 pos)
	{
		LandMine landMine;
		landMine.pos = pos;

		landMines.push_back(landMine);
	}

	void WorldManager::PlaceDecoy(Math::Vector3 pos)
	{
		Decoy decoy;
		decoy.id = decoyID;
		decoy.pos = pos;
		decoy.hp = 5;
		decoy.lightID = AddLightInstance(pos, decoyFlareRadius, decoyFlareColor);

		decoyID++;

		decoys.push_back(decoy);
	}

	void WorldManager::SetState(GameState setState)
	{
		switch (state)
		{
			case GameState::TittleScreen:
			{
				if (titleScreenRef.entity)
				{
					titleScreenRef.entity->SetVisible(false);
				}

				break;
			}

			case GameState::HelpScreen:
			{
				if (helpScreenRef.entity)
				{
					helpScreenRef.entity->SetVisible(false);
				}

				if (themeMusicRef.entity)
				{
					themeMusicRef.entity->SetVisible(false);
				}

				break;
			}

			case GameState::Victory:
			{
				if (levelRef.entity)
				{
					levelRef.entity->SetVisible(false);
				}

				if (victoryRef.entity)
				{
					victoryRef.entity->SetVisible(false);
				}

				break;
			}

			case GameState::Wasted:
			{
				if (levelRef.entity)
				{
					levelRef.entity->SetVisible(false);
				}

				if (wastedRef.entity)
				{
					wastedRef.entity->SetVisible(false);
				}

				break;
			}
		}

		state = setState;

		switch (state)
		{
			case GameState::TittleScreen:
			{
				if (titleScreenRef.entity)
				{
					titleScreenRef.entity->SetVisible(true);
				}

				if (themeMusicRef.entity)
				{
					themeMusicRef.entity->SetVisible(true);
				}

				break;
			}

			case GameState::HelpScreen:
			{
				if (helpScreenRef.entity)
				{
					helpScreenRef.entity->SetVisible(true);
				}

				break;
			}

			case GameState::Gameplay:
			{
				if (levelRef.entity)
				{
					levelRef.entity->SetVisible(true);
				}

				Reset();

				break;
			}

			case GameState::Victory:
			{
				if (victoryRef.entity)
				{
					victoryRef.entity->SetVisible(true);
				}

				if (escapeRef.entity)
				{
					escapeRef.entity->SetVisible(false);
				}

				break;
			}

			case GameState::Wasted:
			{
				if (wastedRef.entity)
				{
					wastedRef.entity->SetVisible(true);
				}

				if (escapeRef.entity)
				{
					escapeRef.entity->SetVisible(false);
				}

				break;
			}
		}
	}

	void WorldManager::UpdateGameplay(float dt)
	{
		for (auto& landMine : landMines)
		{
			if (landMine.detonated)
			{
				continue;
			}

			float dist2 = landMineActivateRadius * landMineActivateRadius;

			bool activated = false;

			for (auto flock : flocks)
			{
				for (auto& orc : flock->orcs)
				{
					if (orc.hp == 0)
					{
						continue;
					}

					if ((orc.transform.position - landMine.pos).Length2() < dist2)
					{
						activated = true;
						break;
					}
				}

				if (activated)
				{
					break;
				}
			}

			if (activated)
			{
				BoomEnemies(landMine.pos, landMineBoomRadius, 10);
				landMine.detonated = true;
				landMine.lightID = AddLightInstance(landMine.pos, 0.0f, landMineBoomFlareColor);
			}
		}

		if (treasureGrabbed)
		{
			if (startRef.entity)
			{
				auto dir = startRef.entity->GetTransform().position - player->GetTransform().position;

				if (dir.Normalize() < 400.0f)
				{
					WorldManager::instance->SetState(GameState::Victory);

					return;
				}
			}
		}
		else
		{
			if (treasureRef.entity)
			{
				auto dir = player->GetTransform().position - treasureRef.entity->GetTransform().position;

				if (dir.Length2() < 50 * 50)
				{
					treasureRef.entity->SetVisible(false);
					treasureGrabbed = true;

					if (escapeRef.entity)
					{
						escapeRef.entity->SetVisible(true);
					}

					AgroEnemies(player->GetTransform().position, -1.0f);
				}
			}
		}
	}

	void WorldManager::Work(float dt)
	{
		if (scene->IsPlaying())
		{
			int device_index;

			const char* keyPressed = root.controls.GetActivatedKey(device_index);

			switch (state)
			{
				case GameState::TittleScreen:
				{
					if (keyPressed)
					{
						SetState(GameState::HelpScreen);
					}

					break;
				}

				case GameState::HelpScreen:
				{
					if (keyPressed)
					{
						SetState(GameState::Gameplay);
					}

					break;
				}

				case GameState::Gameplay:
				{
					UpdateGameplay(dt);

					break;
				}

				case GameState::Victory:
				{
					if (keyPressed)
					{
						SetState(GameState::TittleScreen);
					}

					break;
				}

				case GameState::Wasted:
				{
					if (keyPressed)
					{
						SetState(GameState::TittleScreen);
					}

					break;
				}
			}
		}
	}

	void WorldManager::Draw(float dt)
	{
		Transform transform;
		Math::Vector2 size = landMineTexture.GetSize();
		transform.size = Math::Vector3(size.x, size.y, 0.0f);

		for (int i = 0; i < landMines.size(); i++)
		{
			auto& mine = landMines[i];

			if (!mine.detonated)
			{
				transform.position = mine.pos;
				transform.BuildMatrices();

				landMineTexture.Draw(&transform, COLOR_WHITE, dt);
			}
			else
			{
				mine.fxTimer += dt * 4.0f;

				if (mine.fxTimer > 1.0f)
				{
					ReleaseLightInstance(mine.lightID);

					landMines.erase(landMines.begin() + i);
					i--;
				}
				else
				if (mine.lightID != -1)
				{
					UpdateLightInstance(mine.lightID, mine.pos, mine.fxTimer * 250.0f, landMineBoomFlareColor);
				}
			}
		}

		size = decoyTexture.GetSize();
		transform.size = Math::Vector3(size.x, size.y, 0.0f);

		for (auto& decoy : decoys)
		{
			if (decoy.hp == 0)
			{
				continue;
			}

			transform.position = decoy.pos;
			transform.BuildMatrices();

			decoyTexture.Draw(&transform, COLOR_WHITE, dt);
		}
	}

	void WorldManager::DrawOverlay(float dt)
	{
		Transform transform;
		Math::Vector2 size = landMineBoomTex.GetSize();

		for (auto& mine : landMines)
		{
			if (mine.detonated)
			{
				transform.size = Math::Vector3(size.x, size.y, 0.0f) * mine.fxTimer * 2.0f;

				transform.position = mine.pos;
				transform.BuildMatrices();

				landMineBoomTex.prg = Sprite::quadPrgNoZ;
				landMineBoomTex.Draw(&transform, COLOR_WHITE_A(1.0f - powf(mine.fxTimer, 4)), dt);
			}
		}

		if (treasureGrabbed)
		{
			if (startRef.entity)
			{
				auto dir = startRef.entity->GetTransform().position - player->GetTransform().position;
				dir.Normalize();

				size = escapeArrowTex.GetSize();
				transform.size = Math::Vector3(size.x, size.y, 0.0f);
				transform.rotation.z = -atan2f(dir.y, dir.x) / Math::Radian;
				transform.position = player->GetTransform().position + dir * 150.0f;
				transform.BuildMatrices();

				escapeArrowTex.prg = Sprite::quadPrgNoZ;
				escapeArrowTex.Draw(&transform, COLOR_WHITE, dt);
			}
		}
	}

	void WorldManager::KickTarget(Math::Vector3 pos, int targetID, float angle, int damage)
	{
		if (targetID == playerID)
		{
			if (player->hp == 0)
			{
				return;
			}

			auto playerPos = player->GetTransform().global.Pos();

			if (Math::IsPointInRectangle(Math::Vector2(playerPos.x, playerPos.y), Math::Vector2(pos.x, pos.y), 0.0f, Math::Vector2(120.0f, 90.0f), -angle * Math::Radian, showDebug))
			{
				auto dir = playerPos - pos;
				dir.z = 0.0f;
				dir.Normalize();

				player->Hit(dir, damage);
			}

			return;
		}

		for (int i = 0; i < decoys.size(); i++)
		{
			auto& decoy = decoys[i];

			if (decoy.id == targetID)
			{
				if (decoy.hp > 0)
				{
					if (Math::IsPointInRectangle(Math::Vector2(decoy.pos.x, decoy.pos.y), Math::Vector2(pos.x, pos.y), 0.0f, Math::Vector2(120.0f, 90.0f), -angle * Math::Radian, showDebug))
					{
						decoy.hp -= damage;

						if (decoy.hp == 0)
						{
							ReleaseLightInstance(decoy.lightID);

							decoys.erase(decoys.begin() + i);
						}
					}
				}

				return;
			}
		}
	}

	void WorldManager::AgroEnemies(Math::Vector3 pos, float radius)
	{
		float dist2 = radius * radius;

		for (auto flock : flocks)
		{
			for (auto& orc : flock->orcs)
			{
				if (orc.hp == 0)
				{
					continue;
				}

				if (radius < -0.5f || (orc.state != HordeFlock::State::Pursuit && orc.state != HordeFlock::State::MelleAttack && (orc.transform.position - pos).Length2() < dist2))
				{
					orc.lastTargetPos = player->GetTransform().position;
					orc.state = HordeFlock::State::GoingToGhost;		
				}
			}
		}
	}

	void WorldManager::KickEnemies(Math::Vector3 pos, float angle, int damage)
	{
		for (auto flock : flocks)
		{
			for (auto& orc : flock->orcs)
			{
				if (orc.hp == 0)
				{
					continue;
				}

				auto enemyPos = orc.transform.global.Pos();

				if (Math::IsPointInRectangle(Math::Vector2(enemyPos.x, enemyPos.y), Math::Vector2(pos.x, pos.y), 0.0f, Math::Vector2(100.0f, 80.0f), -angle * Math::Radian, showDebug))
				{
					auto dir = enemyPos - pos;
					dir.z = 0.0f;
					dir.Normalize();

					orc.Hit(dir, damage);

					return;
				}
			}
		}
	}

	void WorldManager::BoomEnemies(Math::Vector3 pos, float radius, int damage)
	{
		float dist2 = radius * radius;

		for (auto flock : flocks)
		{
			for (auto& orc : flock->orcs)
			{
				if (orc.hp == 0)
				{
					continue;
				}

				auto enemyPos = orc.transform.position;

				if ((enemyPos - pos).Length2() < dist2)
				{
					auto dir = enemyPos - pos;
					dir.z = 0.0f;
					dir.Normalize();

					orc.Hit(dir, damage);
				}
			}
		}
	}

	int WorldManager::FindTargetInSector(Math::Vector3 pos, float dist, float angle)
	{
		if (state != GameState::Gameplay)
		{
			return -1;
		}

		for (auto& decoy : decoys)
		{
			if (decoy.hp > 0 && IsTargetVisibleInSector(pos, decoy.pos, dist, angle))
			{
				return decoy.id;
			}
		}

		if (player->hp > 0 && IsTargetVisibleInSector(pos, player->GetTransform().position, dist, angle))
		{
			return playerID;
		}

		return -1;
	}

	bool WorldManager::IsTargetVisibleInSector(Math::Vector3 pos, Math::Vector3 targetPos, float dist, float angle)
	{
		auto len = (targetPos - pos).Length2();

		if (Math::IsPointInSector(Math::Vector2(targetPos.x, targetPos.y), Math::Vector2(pos.x, pos.y), -angle * Math::Radian, dist, orcViewAngle, showDebug) ||
			(targetPos - pos).Length2() < 200.0f * 200.0f)
		{
			auto dir = targetPos - pos;
			dir.Normalize();

			PhysScene::RaycastDesc rcdesc;

			rcdesc.origin = pos * Sprite::pixelsPerUnitInvert;
			rcdesc.dir = dir;
			rcdesc.length = dist * Sprite::pixelsPerUnitInvert;
			rcdesc.group = 1;

			if (!root.GetPhysScene()->RayCast(rcdesc))
			{
				return true;
			}
		}

		return false;
	}

	Math::Vector3 WorldManager::GetTargetPos(int targetID)
	{
		if (targetID == playerID)
		{
			return player->GetTransform().position;
		}

		for (auto& decoy : decoys)
		{
			if (decoy.id == targetID)
			{
				return decoy.pos;
			}
		}

		return 0.0f;
	}

	bool WorldManager::IsTargetVisibleInSector(Math::Vector3 pos, int targetID, float dist, float angle)
	{
		if (state != GameState::Gameplay)
		{
			return false;
		}

		if (targetID == playerID)
		{
			if (player->hp > 0 && IsTargetVisibleInSector(pos, player->GetTransform().position, dist, angle))
			{
				return true;
			}

			return false;
		}

		for (auto& decoy : decoys)
		{
			if (decoy.id == targetID)
			{
				if (decoy.hp > 0 && IsTargetVisibleInSector(pos, decoy.pos, dist, angle))
				{
					return true;
				}

				return false;
			}
		}

		return false;
	}

	int WorldManager::AddLightInstance(Math::Vector3 position, float radius, Color color)
	{
		auto* entity = dynamic_cast<Lights2DRenderer*>(WorldManager::instance->lightRendererRef.entity);

		if (entity)
		{
			return entity->AddInstance(position, radius, color);
		}

		return -1;
	}

	void WorldManager::UpdateLightInstance(int id, Math::Vector3 position, float radius, Color color)
	{
		auto* entity = dynamic_cast<Lights2DRenderer*>(WorldManager::instance->lightRendererRef.entity);

		if (entity)
		{
			entity->UpdateInstance(id, position, radius, color);
		}
	}

	void WorldManager::ReleaseLightInstance(int id)
	{
		auto* entity = dynamic_cast<Lights2DRenderer*>(WorldManager::instance->lightRendererRef.entity);

		if (entity)
		{
			entity->ReleaseInstance(id);
		}
	}
}