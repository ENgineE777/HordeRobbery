
#include "HordeFlock.h"
#include "WorldManager.h"
#include "Root/Root.h"

namespace Oak
{
	CLASSREG(SceneEntity, HordeFlock, "HordeFlock")

	META_DATA_DESC(HordeFlock)
		BASE_SCENE_ENTITY_PROP(HordeFlock)
		ASSET_ANIM_GRAPH_2D_PROP(HordeFlock, anim, "Visual", "Anim")
		INT_PROP(HordeFlock, orcsCount, 1, "Properties", "orcsCount", "orcsCount")
		INT_PROP(HordeFlock, maxHP, 1, "Properties", "maxHP", "maxHP")
		FLOAT_PROP(HordeFlock, patrolRadius, 300.0f, "Properties", "patrolRadius", "patrolRadius")
	META_DATA_DESC_END()

	void HordeFlock::Orc::Hit(Math::Vector3 dir, int damage)
	{
		hp -= damage;

		if (hp <= 0)
		{
			MakeDead(dir);
		}
	}

	void HordeFlock::Orc::MakeDead(Math::Vector3 dir)
	{
		WorldManager::instance->ReleaseLightInstance(lightID);

		hp = 0;

		flyTime = 0.65f;
		transform.position.z += 0.005f;
		transform.rotation.z = -atan2f(dir.y, dir.x) / Math::Radian;
		moveDir = dir;
		controller->SetActive(false);

		anim.GotoNode("Death");
	}

	void HordeFlock::Orc::OnFrameChangeCallback(int frame, eastl::string& name, eastl::string& param)
	{
		if (StringUtils::IsEqual("KickPlayer", name.c_str()))
		{
			int damage = atoi(param.c_str());

			if (damage < 1)
			{
				damage = 1;
			}

			WorldManager::instance->KickTarget(transform.position, targetID, transform.rotation.z, damage);
		}
	}

	void HordeFlock::Orc::CommandMe(HordeFlock* flock, float dt)
	{
		float viewDist = WorldManager::instance->orcViewDist;

		if (state != State::Patroling)
		{
			viewDist *= 1.25f;
		}

		switch (state)
		{
			case (State::Idle):
			{
				if (WorldManager::instance->IsTargetVisibleInSector(transform.position, targetID, viewDist, transform.rotation.z))
				{
					state = State::Pursuit;
					actionTime = 0.75f;
				}
				else
				{
					state = State::GoingToGhost;
					targetID = -1;
				}

				break;
			}
			case (State::Patroling):
			{
				auto dir = patrolTarget - transform.position;
				if (dir.Normalize() < 50.0f)
				{
					patrolTarget = flock->ChoosePatrolTraget();
				}

				if ((lastPoint - transform.position).Length2() < 10.0f * 10.0f * dt)
				{
					stucked += dt;

					if (stucked > 0.5f)
					{
						patrolTarget = flock->ChoosePatrolTraget();
						stucked = 0.0f;
					}
				}
				else
				{
					stucked = 0.0f;
				}

				float targetAngle = atan2f(dir.y, dir.x);

				transform.rotation.z = -Math::AdvanceAngle(-transform.rotation.z * Math::Radian, targetAngle, dt * Math::PI * 1.5f) / Math::Radian;
				moveDir.x = cosf(-transform.rotation.z * Math::Radian);
				moveDir.y = sinf(-transform.rotation.z * Math::Radian);

				targetID = WorldManager::instance->FindTargetInSector(transform.position, viewDist, transform.rotation.z);

				if (targetID != -1)
				{
					state = State::Idle;
					lastTargetPos = WorldManager::instance->GetTargetPos(targetID);

					moveDir = 0.0f;

					WorldManager::instance->AgroEnemies(transform.position, WorldManager::instance->orcAgroRadius);
				}

				break;
			}
			case (State::Pursuit):
			{
				lastTargetPos = WorldManager::instance->GetTargetPos(targetID);

				auto dir = lastTargetPos - transform.position;
				float dist = dir.Normalize();

				if (dist > viewDist || !WorldManager::instance->IsTargetVisibleInSector(transform.position, targetID, viewDist, transform.rotation.z))
				{
					state = State::GoingToGhost;
					targetID = -1;

					break;
				}

				if (dist < 65.0f)
				{
					state = State::MelleAttack;
					anim.ActivateLink("Kick");
					WorldManager::instance->AgroEnemies(transform.position, WorldManager::instance->orcAgroRadius);
					moveDir = 0.0f;
					actionTime = 1.0f;

					break;
				}

				actionTime -= dt;
				
				if (actionTime < 0.0f)
				{
					WorldManager::instance->AgroEnemies(transform.position, WorldManager::instance->orcAgroRadius);
					actionTime = 0.75f;
				}

				float targetAngle = atan2f(dir.y, dir.x);

				transform.rotation.z = -Math::AdvanceAngle(-transform.rotation.z * Math::Radian, targetAngle, dt * Math::PI * 1.5f) / Math::Radian;
				moveDir.x = cosf(-transform.rotation.z * Math::Radian);
				moveDir.y = sinf(-transform.rotation.z * Math::Radian);

				break;
			}
			case (State::GoingToGhost):
			{
				targetID = WorldManager::instance->FindTargetInSector(transform.position, viewDist, transform.rotation.z);

				if (targetID != -1)
				{
					state = State::Pursuit;
					actionTime = 0.75f;

					break;
				}

				auto dir = lastTargetPos - transform.position;

				if (dir.Normalize() < 50.0f)
				{
					state = State::WaitingForTarget;
					actionTime = 5.0f;
					actionTime2 = 0.65f;
					actionData = transform.rotation.z * Math::Radian;

					moveDir = 0.0f;

					break;
				}

				float targetAngle = atan2f(dir.y, dir.x);

				transform.rotation.z = -Math::AdvanceAngle(-transform.rotation.z * Math::Radian, targetAngle, dt * Math::PI * 1.5f) / Math::Radian;
				moveDir.x = cosf(-transform.rotation.z * Math::Radian);
				moveDir.y = sinf(-transform.rotation.z * Math::Radian);

				break;
			}
			case (State::WaitingForTarget):
			{
				targetID = WorldManager::instance->FindTargetInSector(transform.position, viewDist, transform.rotation.z);

				if (targetID != -1)
				{
					state = State::Idle;

					WorldManager::instance->AgroEnemies(transform.position, WorldManager::instance->orcAgroRadius);

					break;
				}

				actionTime -= dt;

				if (actionTime < 0.0f)
				{
					state = State::Patroling;
				}
				else
				{
					transform.rotation.z = -Math::AdvanceAngle(-transform.rotation.z * Math::Radian, actionData, dt * Math::PI * 1.5f) / Math::Radian;

					actionTime2 -= dt;

					if (actionTime2 < 0.0001f)
					{
						actionTime2 = 0.65f;
						actionData = Math::Rand() * Math::TwoPI;
					}
				}
				break;
			}
			case (State::MelleAttack):
			{
				actionTime -= dt;

				if (actionTime < 0.0f)
				{
					state = State::Idle;
				}

				auto dir = WorldManager::instance->GetTargetPos(targetID) - transform.position;
				float dist = dir.Normalize();

				float targetAngle = atan2f(dir.y, dir.x);

				transform.rotation.z = -Math::AdvanceAngle(-transform.rotation.z * Math::Radian, targetAngle, dt * Math::PI * 1.5f) / Math::Radian;

				if (dist > 65.0f)
				{
					moveDir.x = cosf(-transform.rotation.z * Math::Radian);
					moveDir.y = sinf(-transform.rotation.z * Math::Radian);
				}
				else
				{
					moveDir = 0.0f;
				}

				break;
			}
		}

		lastPoint = transform.position;
	}

	HordeFlock::HordeFlock() : SceneEntity()
	{
		//inst_class_name = "SpriteInst";
	}

	void HordeFlock::Init()
	{
		transform.unitsScale = &Sprite::pixelsPerUnit;
		transform.unitsInvScale = &Sprite::pixelsPerUnitInvert;
		transform.transformFlag = MoveXYZ | TransformFlag::RotateZ | TransformFlag::ScaleX | TransformFlag::ScaleY | RectMoveXY | RectAnchorn;

		Tasks(true)->AddTask(0, this, (Object::Delegate)& HordeFlock::Draw);
	}

	void HordeFlock::Restart()
	{
		for (auto& orc : orcs)
		{
			auto pos = ChoosePatrolTraget();

			orc.transform.position = pos;
			pos *= Sprite::pixelsPerUnitInvert;
			pos.z = -0.5f;
			orc.controller->SetPosition(pos);
			orc.controller->SetActive(true);

			orc.anim.Reset();
			orc.hp = maxHP;
			orc.state = State::Patroling;
			orc.transform.rotation.z = Math::Rand() * Math::TwoPI;
			orc.patrolTarget = ChoosePatrolTraget();
			orc.lastPoint = transform.position;
			orc.moveDir = 0.0f;

			if (orc.lightID == -1)
			{
				orc.lightID = WorldManager::instance->AddLightInstance(pos, 5.0f, Color(0.15f, 0.15f, 0.0f));
			}
		}
	}

	void HordeFlock::Play()
	{
		SceneEntity::Play();

		PhysControllerDesc desc;
		desc.height = 1.0f;

		auto size = anim.GetSize();
		desc.radius = fminf(size.x, size.y) * Sprite::pixelsPerUnitInvert * 0.5f *  0.65f;

		transform.BuildMatrices();
		desc.upVector.Set(0.0f, 0.0f, 1.0f);

		orcs.resize(orcsCount);

		for (int i = 0; i < orcs.size(); i++)
		{
			auto& orc = orcs[i];

			orc.transform.position = ChoosePatrolTraget();
			desc.pos = orc.transform.position * Sprite::pixelsPerUnitInvert;

			orc.controller = GetRoot()->GetPhysScene()->CreateController(desc, 4);
			orc.controller->RestrictZAxis();

			orc.bodyData.controller = orc.controller;
			orc.controller->SetUserData(&orc.bodyData);

			auto fireEvents = [this, i](int frame, eastl::string& name, eastl::string& param)
			{
				orcs[i].OnFrameChangeCallback(frame, name, param);
			};

			orc.anim = anim;
			orc.anim.SetOnFrameChangeCallback(fireEvents);
		}
	}

	Math::Vector3 HordeFlock::ChoosePatrolTraget()
	{
		Math::Vector3 patrolTarget = transform.position;

		float targetAngle = Math::Rand() * Math::TwoPI;
		float radius = Math::Rand() * patrolRadius;

		patrolTarget.x += cosf(targetAngle) * radius;
		patrolTarget.y += sinf(targetAngle) * radius;

		return patrolTarget;
	}

	void HordeFlock::Draw(float dt)
	{
		if (!IsVisible())
		{
			return;
		}

		if (!scene->IsPlaying())
		{
			if (anim.Get())
			{
				transform.BuildMatrices();

				anim.Draw(&transform, COLOR_WHITE, dt);
			}

			return;
		}

		for (auto& orc : orcs)
		{
			if (orc.hp == 0)
			{
				if (orc.flyTime > 0)
				{
					orc.flyTime -= dt;

					if (orc.flyTime < 0.0f)
					{
						orc.flyTime = -1.0f;
						orc.moveDir = 0.0f;
					}
				}
			}
			else
			{
				orc.CommandMe(this, dt);
			}

			if (orc.moveDir.Normalize() > 0.01f)
			{
				orc.anim.ActivateLink("Walk");

				Math::Vector3 pos;
				orc.controller->GetPosition(pos);

				float speed = (orc.state == State::Patroling) ? WorldManager::instance->orcSpeedPatrol : WorldManager::instance->orcSpeedPursuit;

				orc.controller->Move(orc.moveDir * speed * Sprite::pixelsPerUnitInvert * dt, 1, 6);

				orc.controller->GetPosition(pos);

				orc.transform.position.x = pos.x * Sprite::pixelsPerUnit;
				orc.transform.position.y = pos.y * Sprite::pixelsPerUnit;

				WorldManager::instance->UpdateLightInstance(orc.lightID, orc.transform.position, WorldManager::instance->orcFlareRadius, WorldManager::instance->orcFlareColor);
			}
			else
			{
				orc.anim.ActivateLink("Idle");
			}

			orc.transform.BuildMatrices();

			orc.anim.Draw(&orc.transform, COLOR_WHITE, dt);
		}
	}
}