
#include "HordePlayer.h"
#include "WorldManager.h"
#include "Root/Root.h"

namespace Oak
{
	CLASSREG(SceneEntity, HordePlayer, "HordePlayer")

	META_DATA_DESC(HordePlayer)
		BASE_SCENE_ENTITY_PROP(HordePlayer)
		ASSET_ANIM_GRAPH_2D_PROP(HordePlayer, anim, "Visual", "Anim")
		SCENEOBJECT_PROP(HordePlayer, cursorRef, "Visual", "Cursor")
		SCENEOBJECT_PROP(HordePlayer, healthBarRef, "Visual", "Health Bar")
		SCENEOBJECT_PROP(HordePlayer, selIconRef, "Visual", "Select Icon")
		SCENEOBJECT_PROP(HordePlayer, useIconRef, "Visual", "Use Icon")
		SCENEOBJECT_PROP(HordePlayer, amountHealthRef, "Visual", "Health amount label")
		SCENEOBJECT_PROP(HordePlayer, amountLandMineRef, "Visual", "Land mine amount label")
		SCENEOBJECT_PROP(HordePlayer, amountDecoyRef, "Visual", "Decoy amount label")
		INT_PROP(HordePlayer, maxHP, 1, "Properties", "maxHP", "maxHP")
		FLOAT_PROP(HordePlayer, speed, 280.0f, "Properties", "speed", "speed")
		FLOAT_PROP(HordePlayer, timeToUseItem, 2.0f, "Properties", "timeToUseItem", "timeToUseItem")
	META_DATA_DESC_END()

	HordePlayer::HordePlayer() : SceneEntity()
	{
		//inst_class_name = "SpriteInst";
	}

	void HordePlayer::Init()
	{
		transform.unitsScale = &Sprite::pixelsPerUnit;
		transform.unitsInvScale = &Sprite::pixelsPerUnitInvert;
		transform.transformFlag = MoveXYZ | TransformFlag::RotateZ | TransformFlag::ScaleX | TransformFlag::ScaleY | RectMoveXY | RectAnchorn;

		Tasks(true)->AddTask(0, this, (Object::Delegate)& HordePlayer::Draw);
	}

	void HordePlayer::Restart(Math::Vector3 pos)
	{
		anim.Reset();

		moveDir = 0.0f;
		transform.position = pos;
		pos *= Sprite::pixelsPerUnitInvert;
		pos.z = -0.5f;
		controller->SetPosition(pos);
		controller->SetActive(true);

		hp = maxHP;
		inHit = -1.0f;

		curTimeToUseItem = 0.0f;
		itemWasUse = false;

		curItem = 0;

		for (auto& item : items)
		{
			item.amount = item.maxAmount;

			if (item.amountLabel)
			{
				item.amountLabel->text = StringUtils::PrintTemp("x%i", item.amount);
			}
		}
	}

	void HordePlayer::Play()
	{
		SceneEntity::Play();

		{
			ItemDesc item;
			item.name = "Health";
			item.action = [this]()
			{
				this->hp = this->maxHP;
			};

			item.maxAmount = WorldManager::instance->hpAmountForPlayer;
			item.iconPosX = WorldManager::instance->hpIconPosX;
			item.amountLabel = amountHealthRef.entity ? dynamic_cast<LabelWidget*>(amountHealthRef.entity) : nullptr;

			items.push_back(item);
		}

		{
			ItemDesc item;
			item.name = "Mine";
			item.action = [this]()
			{
				WorldManager::instance->PlaceLandMine(this->transform.position);
			};

			item.maxAmount = WorldManager::instance->landMineAmountForPlayer;
			item.iconPosX = WorldManager::instance->landMineIconPosX;
			item.amountLabel = amountLandMineRef.entity ? dynamic_cast<LabelWidget*>(amountLandMineRef.entity) : nullptr;

			items.push_back(item);
		}

		{
			ItemDesc item;
			item.name = "Decoy";
			item.action = [this]()
			{
				WorldManager::instance->PlaceDecoy(this->transform.position);
			};

			item.maxAmount = WorldManager::instance->decoyAmountForPlayer;
			item.iconPosX = WorldManager::instance->decoyIconPosX;
			item.amountLabel = amountDecoyRef.entity ? dynamic_cast<LabelWidget*>(amountDecoyRef.entity) : nullptr;

			items.push_back(item);
		}

		transform.BuildMatrices();

		PhysControllerDesc desc;
		desc.height = 1.0f;

		auto size = anim.GetSize();
		desc.radius = fminf(size.x, size.y) * Sprite::pixelsPerUnitInvert * 0.5f *  0.65f;
		desc.upVector.Set(0.0f, 0.0f, 1.0f);
		desc.pos = transform.global.Pos() * Sprite::pixelsPerUnitInvert;

		controller = root.GetPhysScene()->CreateController(desc, 2);
		controller->RestrictZAxis();

		bodyData.object = this;
		bodyData.controller = controller;

		controller->SetUserData(&bodyData);

		auto fireEvents = [this](int frame, eastl::string& name, eastl::string& param)
		{
			this->OnFrameChangeCallback(frame, name, param);
		};

		anim.SetOnFrameChangeCallback(fireEvents);

		heroCURSOR_X = root.controls.GetAlias("Hero.CURSOR_X");
		heroCURSOR_Y = root.controls.GetAlias("Hero.CURSOR_Y");

		heroMoveHorz = root.controls.GetAlias("Hero.MOVE_HORZ");
		heroMoveVert = root.controls.GetAlias("Hero.MOVE_VERT");

		heroFire = root.controls.GetAlias("Hero.Fire");

		heroPrevItem = root.controls.GetAlias("Hero.PrevItem");
		heroNextItem = root.controls.GetAlias("Hero.NextItem");

		heroUseItem = root.controls.GetAlias("Hero.UseItem");
	}

	void HordePlayer::OnFrameChangeCallback(int frame, eastl::string& name, eastl::string& param)
	{
		if (StringUtils::IsEqual("Kick", name.c_str()))
		{
			int damage = atoi(param.c_str());

			if (damage < 1)
			{
				damage = 1;
			}

			WorldManager::instance->KickEnemies(transform.position, transform.rotation.z, damage);
		}
	}

	void HordePlayer::Hit(Math::Vector3 dir, int damage)
	{
		hp -= damage;

		if (inHit < 0.0f)
		{
			inHit = 0.25f;
		}

		if (hp <= 0)
		{
			MakeDead(dir);
		}
	}

	void HordePlayer::MakeDead(Math::Vector3 dir)
	{
		hp = 0;
		inHit = -1.0f;

		flyTime = 0.65f;
		transform.position.z += 0.005f;
		transform.rotation.z = -atan2f(dir.y, dir.x) / Math::Radian;
		moveDir = dir;
		controller->SetActive(false);

		anim.GotoNode("Death");
	}

	void HordePlayer::UpdateIconItem()
	{
		auto& item = items[curItem];

		if (selIconRef.entity)
		{
			selIconRef.entity->GetTransform().position.x = item.iconPosX;
		}

		if (item.amountLabel)
		{
			item.amountLabel->text = StringUtils::PrintTemp("x%i", item.amount);
		}

	}

	void HordePlayer::ControlPlayer(float dt)
	{
		float msX = root.controls.GetAliasValue(heroCURSOR_X, false);
		float msY = root.controls.GetAliasValue(heroCURSOR_Y, false);

		if (cursorRef.entity)
		{
			float k = Sprite::pixelsHeight / root.render.GetDevice()->GetHeight();

			auto& trans = cursorRef.entity->GetTransform();
			trans.position.x = msX * k;
			trans.position.y = Sprite::pixelsHeight - msY * k;
		}

		auto screenPos = root.render.TransformToScreen(Math::Vector3(transform.position.x * Sprite::pixelsPerUnitInvert, transform.position.y * Sprite::pixelsPerUnitInvert, 0.0f), 2);

		Math::Vector2 dir((msX - screenPos.x), (msY - screenPos.y));
		dir.Normalize();

		transform.rotation.z = atan2f(dir.y, dir.x) / Math::Radian;

		moveDir.x = root.controls.GetAliasValue(heroMoveHorz, false);
		moveDir.y = root.controls.GetAliasValue(heroMoveVert, false);

		if (root.controls.GetAliasState(heroPrevItem))
		{
			curItem--;

			if (curItem < 0)
			{
				curItem = (int)items.size() - 1;
			}
		}

		if (root.controls.GetAliasState(heroNextItem))
		{
			curItem = (curItem + 1) % items.size();
		}

		UpdateIconItem();

		if (root.controls.GetAliasState(heroUseItem, AliasAction::Pressed))
		{
			if (items[curItem].amount > 0)
			{
				curTimeToUseItem += dt;

				if (curTimeToUseItem > timeToUseItem)
				{
					curTimeToUseItem = timeToUseItem;

					if (!itemWasUse)
					{
						itemWasUse = true;

						if (items[curItem].action)
						{
							items[curItem].action();
						}

						items[curItem].amount--;
					}
				}
			}
		}
		else
		{
			curTimeToUseItem = 0.0f;
			itemWasUse = false;
		}

		if (useIconRef.entity)
		{
			useIconRef.entity->GetTransform().scale.y = curTimeToUseItem / timeToUseItem;
		}

		if (root.controls.GetAliasState(heroFire, AliasAction::JustPressed))
		{
			anim.ActivateLink("Kick");
		}
	}

	void HordePlayer::Draw(float dt)
	{
		if (!IsVisible())
		{
			return;
		}

		if (scene->IsPlaying())
		{
			if (healthBarRef.entity)
			{
				healthBarRef.entity->GetTransform().scale.x = (float)hp / (float)maxHP;
			}

			if (hp == 0)
			{
				if (flyTime > 0)
				{
					flyTime -= dt;

					if (flyTime < 0.0f)
					{
						flyTime = -1.0f;
						moveDir = 0.0f;

						WorldManager::instance->SetState(WorldManager::GameState::Wasted);
					}
				}
			}
			else
			{
				ControlPlayer(dt);
			}

			if (inHit > 0.0f)
			{
				inHit -= dt;
			}

			if (moveDir.Normalize() > 0.01f)
			{
				anim.ActivateLink("Walk");

				controller->Move(moveDir * speed * Sprite::pixelsPerUnitInvert * dt, 1, 6);

				Math::Vector3 pos;
				controller->GetPosition(pos);

				transform.position.x = pos.x * Sprite::pixelsPerUnit;
				transform.position.y = pos.y * Sprite::pixelsPerUnit;
			}
			else
			{
				anim.ActivateLink("Idle");
			}
		}

		if (anim.Get())
		{
			transform.BuildMatrices();

			anim.Draw(&transform, inHit > 0.0f ? COLOR_RED : COLOR_WHITE, dt);
		}
	}
}