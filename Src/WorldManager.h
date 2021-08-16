
#pragma once

#include "Root/Scenes/SceneEntity.h"
#include "Support/MetaData.h"
#include "Support/Sprite.h"
#include "root/Assets/AssetTexture.h"
#include "HordePlayer.h"
#include "HordeFlock.h"
#include "SceneEntities/2D/Lights2DRenderer.h"

namespace Oak
{
	class WorldManager : public SceneEntity
	{
	public:

		static WorldManager* instance;

		META_DATA_DECL_BASE(WorldManager)

		HordePlayer* player = nullptr;

		SceneEntityRef titleScreenRef;
		SceneEntityRef helpScreenRef;
		SceneEntityRef themeMusicRef;
		SceneEntityRef levelRef;
		SceneEntityRef playerRef;
		SceneEntityRef escapeRef;
		SceneEntityRef wastedRef;
		SceneEntityRef victoryRef;
		SceneEntityRef treasureRef;
		SceneEntityRef startRef;
		SceneEntityRef lightRendererRef;
		SceneEntityRef hordesRootRef;

		AssetTextureRef escapeArrowTex;
		AssetTextureRef landMineBoomTex;

		enum class GameState
		{
			TittleScreen,
			HelpScreen,
			Gameplay,
			Victory,
			Wasted
		};

		GameState state = GameState::TittleScreen;

		int hpAmountForPlayer = 1;
		float hpIconPosX = 0.0f;

		AssetTextureRef landMineTexture;
		float landMineActivateRadius = 50.0f;
		float landMineBoomRadius = 400.0f;
		int landMineAmountForPlayer = 1;
		float landMineIconPosX = 0.0f;
		Color landMineBoomFlareColor;

		struct LandMine
		{
			Math::Vector3 pos;
			bool detonated = false;
			int lightID = -1;
			float fxTimer = 0.0f;
		};

		eastl::vector<LandMine> landMines;

		int decoyID = 0;

		struct Decoy
		{
			int id;
			Math::Vector3 pos;
			int lightID = -1;
			int hp;
		};

		AssetTextureRef decoyTexture;
		int decoyAmountForPlayer = 1;
		int decoyMaxHP = 5;
		float decoyIconPosX = 0.0f;
		Color decoyFlareColor;
		float decoyFlareRadius = 10.0f;
		eastl::vector<Decoy> decoys;
		
		eastl::vector<HordeFlock*> flocks;

		float orcViewDist = 500.0f;
		float orcViewAngle = 70.0f;
		float orcSpeedPatrol = 180.0f;
		float orcSpeedPursuit = 300.0f;
		float orcAgroRadius = 700.0f;
		Color orcFlareColor;
		float orcFlareRadius = 10.0f;

		bool treasureGrabbed = false;

		bool showDebug = false;

		int playerID = 1000;

	#ifndef DOXYGEN_SKIP

		WorldManager();
		virtual ~WorldManager() = default;

		void Init() override;

		void Play() override;

		void Reset();

		void Work(float dt);
		void UpdateGameplay(float dt);
		void Draw(float dt);
		void DrawOverlay(float dt);

		void KickTarget(Math::Vector3 pos, int targetID, float angle, int damage);

		void AgroEnemies(Math::Vector3 pos, float radius);
		void KickEnemies(Math::Vector3 pos, float angle, int damage);
		void BoomEnemies(Math::Vector3 pos, float radius, int damage);

		int FindTargetInSector(Math::Vector3 pos, float dist, float angle);
		bool IsTargetVisibleInSector(Math::Vector3 pos, Math::Vector3 targetPos, float dist, float angle);
		bool IsTargetVisibleInSector(Math::Vector3 pos, int targetID, float dist, float angle);
		Math::Vector3 GetTargetPos(int targetID);

		void PlaceLandMine(Math::Vector3 pos);
		void PlaceDecoy(Math::Vector3 pos);
		void SetState(GameState state);

		int AddLightInstance(Math::Vector3 position, float radius, Color color);
		void UpdateLightInstance(int id, Math::Vector3 position, float radius, Color color);
		void ReleaseLightInstance(int id);
	#endif
	};
}