class PluginDeveloper extends PluginBase
{	
	static const int MIN_SQF_DEBUG_WATCHERS_COUNT = 5;
	static const int MAX_SQF_DEBUG_WATCHERS_COUNT = 12;
	static const int HUD_TEST_STATUS_FOOD = 1;
	static const int HUD_TEST_STATUS_WATER = 2;
	static const int HUD_TEST_STATUS_TEMPERATURE = 3;
	static const int HUD_TEST_STATUS_HEALTH = 4;
	static const int HUD_TEST_STATUS_BLOOD = 5;
	
	protected autoptr array<SqfDebugWatcher>		m_SqfDebugWatchers;
	protected bool									m_IsWinHolding;	
	protected bool 									m_GodModeEnabled;
	protected int									m_FeaturesMask;
	
	static PluginDeveloper GetInstance()
	{
		return GetPlugin( PluginDeveloper );
	}
	
	//! Set Player position at his cursor position in the world
	void TeleportAtCursor()
	{
		DeveloperTeleport.TeleportAtCursor();
	}
	
	//! Teleport player at position
	void Teleport(PlayerBase player, vector position)
	{
		DeveloperTeleport.SetPlayerPosition(player, position);
	}
	
	//! Enable / Disable Free camera (Fly mod)
	void ToggleFreeCameraBackPos()
	{
		DeveloperFreeCamera.FreeCameraToggle( GetGame().GetPlayer(), false );
	}
	
	//! Enable / Disable Free camera (Fly mod) - disable of camera will teleport player at current free camera position.
	void ToggleFreeCamera()
	{
		DeveloperFreeCamera.FreeCameraToggle( GetGame().GetPlayer(), true );
	}
	
	//! Enable / Disable Free camera (Fly mod) - disable of camera will teleport player at current free camera position.
	Camera GetFreeCamera()
	{
		return DeveloperFreeCamera.GetFreeCamera();
	}
	
	bool IsEnabledFreeCamera()
	{
		return DeveloperFreeCamera.IsFreeCameraEnabled();
	}	
	
	// System Public Events
	void PluginDeveloper()
	{
		
	}

	void ~PluginDeveloper()
	{
		int i;
		
		for ( i = 0; i < m_SqfDebugWatchers.Count(); i++ )
		{
			delete m_SqfDebugWatchers.Get( i );
		}
	}

	void OnInit()
	{
		super.OnInit();
		
		m_SqfDebugWatchers = new array<SqfDebugWatcher>;
		m_GodModeEnabled = false;
		
		DeveloperFreeCamera.OnInit();
		
		for ( int i = 1; i <= MIN_SQF_DEBUG_WATCHERS_COUNT; i++ )
		{
			m_SqfDebugWatchers.Insert( new SqfDebugWatcher(i, "") );
		}
	}
	
	void OnRPC(PlayerBase player, int rpc_type, ParamsReadContext ctx)
	{
		if ( !GetGame().IsDebug() )
		{
			if ( rpc_type != DEV_RPC_SPAWN_ITEM && rpc_type != DEV_RPC_CLEAR_INV && rpc_type != DEV_RPC_TELEPORT && rpc_type != DEV_RPC_SET_PLAYER_DIRECTION && rpc_type != DEV_RPC_SEND_SERVER_LOG && rpc_type != DEV_RPC_TOGGLE_GOD_MODE && rpc_type != DEV_RPC_DEBUG_PING && rpc_type != DEV_RPC_SET_HUD_TEST_HUNGER && rpc_type != DEV_RPC_SET_HUD_TEST_STATUS && rpc_type != DEV_RPC_CREATE_SCENE_OBJECT && rpc_type != RPC_SYNC_SCENE_OBJECT )
			{
				return;
			}
		}
		
		switch(rpc_type)
		{
			case DEV_RPC_SPAWN_ITEM:
			{
				OnRPCSpawnItem(player, ctx); break;
			}
			case DEV_RPC_CLEAR_INV:
			{
				OnRPCClearInventory(player); break;
			}
			case DEV_RPC_SEND_SERVER_LOG:
			{
				OnRPCServerLogRecieved( ctx ); break;
			}
			case DEV_RPC_TOGGLE_GOD_MODE:
			{
				OnRPCToggleGodMode( player ); break;
			}
			case DEV_RPC_DEBUG_PING:
			{
				OnRPCDebugPing( player, ctx ); break;
			}
			case DEV_RPC_SET_HUD_TEST_HUNGER:
			{
				OnRPCSetHudTestHunger( player, ctx ); break;
			}
			case DEV_RPC_SET_HUD_TEST_STATUS:
			{
				OnRPCSetHudTestStatus( player, ctx ); break;
			}
			case DEV_RPC_CREATE_SCENE_OBJECT:
			{
				OnRPCCreateSceneObject( player, ctx ); break;
			}
			case RPC_SYNC_SCENE_OBJECT:
			{
				OnRPCSyncSceneObject( ctx ); break;
			}
		}
		
		DeveloperTeleport.OnRPC(player, rpc_type, ctx);
	}
	
	// Public API
	// Send Print to scripted console
	void PrintLogClient(string msg_log)
	{	
		if ( g_Game.GetUIManager().IsMenuOpen(MENU_SCRIPTCONSOLE) )
		{
			Param1<string> msg_p = new Param1<string>(msg_log);
			CallMethod(CALL_ID_SCR_CNSL_ADD_PRINT, msg_p);
		}
	}	
	
	// Server Log Synch: Server Side
	void SendServerLogToClient(string msg)
	{
		autoptr array<Man> players = new array<Man>;
		GetGame().GetPlayers( players );
			
		for ( int i = 0; i < players.Count(); ++i )
		{
			autoptr Param1<string> param = new Param1<string>( msg );
			players.Get(i).RPCSingleParam(DEV_RPC_SEND_SERVER_LOG, param, players.Get(i));
		}
	}
	
	void OnRPCSyncSceneObject( ParamsReadContext ctx )
	{
		autoptr Param4<string, EntityAI, vector, float> par = new Param4<string, EntityAI, vector, float>( "", NULL, "0 0 0", 0 );

		if ( ctx.Read( par ) )
		{		
			PluginSceneManager module_scene_editor = ( PluginSceneManager ) GetPlugin( PluginSceneManager );
			SceneData	scene_data = module_scene_editor.GetLoadedScene();
			
			if ( scene_data == NULL )
			{
				module_scene_editor.InitLoad();
				scene_data = module_scene_editor.GetLoadedScene();
			}
			
			SceneObject scene_object = scene_data.GetSceneObjectByEntityAI( par.param2 );
			
			if ( scene_object )
			{
				string method = par.param1;
				if ( method == "SetPosition" )
				{
					scene_object.SetPosition( par.param3 );
				}
				else if ( method == "SetPositionDirect" )
				{
					scene_object.SetPositionDirect( par.param3 );
				}
				else if ( method == "SetOrientation" )
				{
					scene_object.SetOrientation( par.param3 );
				}
				else if ( method == "SetDamage" )
				{
					scene_object.SetDamage( par.param4 );
				}
				else if ( method == "PlaceOnSurface" )
				{
					scene_object.PlaceOnSurface();
				}
				else if ( method == "DeleteOnServer" )
				{
					scene_object.DeleteOnServer();
				}
			}
		}
	}

	void OnRPCCreateSceneObject(PlayerBase player, ParamsReadContext ctx)
	{
		autoptr Param5<int, string, vector, vector, EntityAI> p = new Param5<int, string, vector, vector, EntityAI>(0, "", "0 0 0", "0 0 0", NULL);
		
		if ( !ctx.Read(p) )
		{
			return;
		}
		
		if ( p.param1 == 0 && GetGame().IsServer() )
		{
			if ( player == NULL )
			{
				return;
			}
			
			EntityAI entity = player.SpawnItem(p.param2, SPAWNTYPE_GROUND, true, 0, true, p.param3, NULL);
			
			if ( entity == NULL )
			{
				SendServerLogToClient("[SCENE EDITOR] Failed: " + p.param2);
				return;
			}
			
			entity.SetOrientation(p.param4);
			
			PluginSceneManager scene_editor = GetPlugin(PluginSceneManager);
			
			if ( scene_editor.GetLoadedScene() == NULL )
			{
				scene_editor.InitLoad();
			}
			
			SceneData scene_data = scene_editor.GetLoadedScene();
			
			if ( scene_data && scene_data.GetSceneObjectByEntityAI(entity) == NULL )
			{
				scene_data.AddObject(entity);
			}
			
			autoptr Param5<int, string, vector, vector, EntityAI> response = new Param5<int, string, vector, vector, EntityAI>(1, p.param2, p.param3, p.param4, entity);
			player.RPCSingleParam(DEV_RPC_CREATE_SCENE_OBJECT, response, player);
			SendServerLogToClient("[SCENE EDITOR] OK: " + p.param2);
		}
		else if ( p.param1 == 1 && !GetGame().IsServer() )
		{
			PluginSceneManager client_scene_editor = GetPlugin(PluginSceneManager);
			
			if ( client_scene_editor )
			{
				client_scene_editor.RegisterSceneObjectFromServer(p.param2, p.param3, p.param4, p.param5);
			}
		}
	}

	// RPC Events
	// Server Log Synch: Client Side
	void OnRPCServerLogRecieved(ParamsReadContext ctx)
	{
		autoptr Param1<string> param = new Param1<string>( "" );
		
		if ( ctx.Read(param) && param.param1 != "" )
		{
			Debug.ReceivedLogMessageFromServer(param.param1);
		}
	}

	void SendDebugPingToServer()
	{
		if ( GetGame().IsServer() )
		{
			PrintLogClient("[DEBUG RPC] Local server debug ping OK.");
			return;
		}
		
		PlayerBase player = GetGame().GetPlayer();
		if ( player )
		{
			autoptr Param1<string> param = new Param1<string>("Script console opened on client.");
			player.RPCSingleParam(DEV_RPC_DEBUG_PING, param);
		}
	}
	
	void OnRPCDebugPing(PlayerBase player, ParamsReadContext ctx)
	{
		autoptr Param1<string> param = new Param1<string>( "" );
		string msg = "debug ping";
		
		if ( ctx.Read(param) && param.param1 != "" )
		{
			msg = param.param1;
		}
		
		if ( GetGame().IsServer() )
		{
			string player_name = "unknown";
			if ( player )
			{
				player_name = player.GetName();
			}
			
			SendServerLogToClient("[DEBUG RPC] Server received client ping from " + player_name + ": " + msg);
		}
	}

	void SetHudTestHungerStage(PlayerBase player, int stage)
	{
		SetHudTestStatusStage( player, HUD_TEST_STATUS_FOOD, stage );
	}

	void OnRPCSetHudTestHunger(PlayerBase player, ParamsReadContext ctx)
	{
		autoptr Param1<int> params = new Param1<int>( 5 );

		if ( ctx.Read( params ) )
		{
			ApplyHudTestStatusStage( player, HUD_TEST_STATUS_FOOD, params.param1 );
		}
	}

	void SetHudTestStatusStage(PlayerBase player, int status_type, int stage)
	{
		if ( !player )
		{
			return;
		}

		stage = ClampHudTestStage( stage );

		if ( GetGame().IsServer() )
		{
			ApplyHudTestStatusStage( player, status_type, stage );
		}
		else
		{
			autoptr Param2<int, int> params = new Param2<int, int>( status_type, stage );
			player.RPCSingleParam( DEV_RPC_SET_HUD_TEST_STATUS, params );
		}
	}

	void OnRPCSetHudTestStatus(PlayerBase player, ParamsReadContext ctx)
	{
		autoptr Param2<int, int> params = new Param2<int, int>( HUD_TEST_STATUS_FOOD, 5 );

		if ( ctx.Read( params ) )
		{
			ApplyHudTestStatusStage( player, params.param1, params.param2 );
		}
	}

	void ApplyHudTestHungerStage(PlayerBase player, int stage)
	{
		ApplyHudTestStatusStage( player, HUD_TEST_STATUS_FOOD, stage );
	}

	void ApplyHudTestStatusStage(PlayerBase player, int status_type, int stage)
	{
		if ( !player || !player.m_PlayerStats )
		{
			SendServerLogToClient("[HUD TEST] Falha: player stats indisponivel.");
			return;
		}

		stage = ClampHudTestStage( stage );

		DisplayStatus display_status = player.GetDisplayStatus();
		float value = 0;
		int hud_level = DELM_LVL_0;

		if ( status_type == HUD_TEST_STATUS_FOOD )
		{
			if ( !player.m_PlayerStats.m_Energy )
			{
				SendServerLogToClient("[HUD TEST] Falha: energia indisponivel.");
				return;
			}

			value = GetHudTestHungerEnergy( stage );
			hud_level = GetHudTestHungerDisplayLevel( stage );
			player.m_PlayerStats.m_Energy.Set( value );

			if ( display_status )
			{
				display_status.SetStatus( DELM_NTFR_HUNGER, hud_level );
				display_status.SendRPC();
			}

			player.MessageStatus( "Teste HUD fome nivel " + stage.ToString() );
			SendServerLogToClient( "[HUD TEST] Fome nivel " + stage.ToString() + " aplicada. Energy=" + value.ToString() );
		}
		else if ( status_type == HUD_TEST_STATUS_WATER )
		{
			if ( !player.m_PlayerStats.m_Water )
			{
				SendServerLogToClient("[HUD TEST] Falha: agua indisponivel.");
				return;
			}

			value = GetHudTestWaterValue( stage );
			hud_level = GetHudTestHungerDisplayLevel( stage );
			player.m_PlayerStats.m_Water.Set( value );

			if ( display_status )
			{
				display_status.SetStatus( DELM_NTFR_THIRST, hud_level );
				display_status.SetStatus( DELM_TDCY_WATER, TENDENCY_STABLE );
				display_status.SendRPC();
			}

			player.MessageStatus( "Teste HUD sede nivel " + stage.ToString() );
			SendServerLogToClient( "[HUD TEST] Sede nivel " + stage.ToString() + " aplicada. Water=" + value.ToString() );
		}
		else if ( status_type == HUD_TEST_STATUS_TEMPERATURE )
		{
			if ( !player.m_PlayerStats.m_Temperature )
			{
				SendServerLogToClient("[HUD TEST] Falha: temperatura indisponivel.");
				return;
			}

			value = GetHudTestTemperatureValue( stage );
			hud_level = GetHudTestTemperatureDisplayLevel( stage );
			player.m_PlayerStats.m_Temperature.Set( value );

			if ( display_status )
			{
				display_status.SetStatus( DELM_NTFR_TEMPERATURE, hud_level );
				display_status.SetStatus( DELM_TDCY_TEMPERATURE, TENDENCY_STABLE );
				display_status.SendRPC();
			}

			player.MessageStatus( "Teste HUD temperatura nivel " + stage.ToString() );
			SendServerLogToClient( "[HUD TEST] Temperatura nivel " + stage.ToString() + " aplicada. Temperature=" + value.ToString() );
		}
		else if ( status_type == HUD_TEST_STATUS_BLOOD )
		{
			float max_blood = player.GetMaxHealth("GlobalHealth", "Blood");
			if ( max_blood <= 0 )
			{
				max_blood = 5000;
			}

			value = GetHudTestBloodValue( stage, max_blood );
			player.SetHealth("GlobalHealth", "Blood", value);

			if ( display_status )
			{
				display_status.SetStatus( DELM_NTFR_BLOOD, stage );
				display_status.SetStatus( DELM_TDCY_BLOOD, TENDENCY_STABLE );
				display_status.SendRPC();
			}

			player.MessageStatus( "Teste HUD sangue nivel " + stage.ToString() );
			SendServerLogToClient( "[HUD TEST] Sangue nivel " + stage.ToString() + " aplicado. Blood=" + value.ToString() );
		}
		else if ( status_type == HUD_TEST_STATUS_HEALTH )
		{
			float max_health = player.GetMaxHealth("GlobalHealth", "Health");
			if ( max_health <= 0 )
			{
				max_health = 5000;
			}

			value = GetHudTestHealthValue( stage, max_health );
			player.SetHealth("GlobalHealth", "Health", value);

			if ( display_status )
			{
				display_status.SetStatus( DELM_NTFR_HEALTHY, stage );
				display_status.SetStatus( DELM_TDCY_HEALTH, TENDENCY_STABLE );
				display_status.SendRPC();
			}

			player.MessageStatus( "Teste HUD vida nivel " + stage.ToString() );
			SendServerLogToClient( "[HUD TEST] Vida nivel " + stage.ToString() + " aplicada. Health=" + value.ToString() );
		}
		else
		{
			SendServerLogToClient( "[HUD TEST] Tipo invalido: " + status_type.ToString() );
		}
	}

	int ClampHudTestStage(int stage)
	{
		if ( stage < 1 )
		{
			return 1;
		}
		else if ( stage > 5 )
		{
			return 5;
		}

		return stage;
	}

	float GetHudTestHungerEnergy(int stage)
	{
		if ( stage == 1 )
		{
			return 0;
		}
		else if ( stage == 2 )
		{
			return 400;
		}
		else if ( stage == 3 )
		{
			return 900;
		}
		else if ( stage == 4 )
		{
			return 1500;
		}

		return 20000;
	}

	float GetHudTestWaterValue(int stage)
	{
		if ( stage == 1 )
		{
			return 0;
		}
		else if ( stage == 2 )
		{
			return 1000;
		}
		else if ( stage == 3 )
		{
			return 2000;
		}
		else if ( stage == 4 )
		{
			return 3000;
		}

		return 5000;
	}

	float GetHudTestTemperatureValue(int stage)
	{
		if ( stage == 1 )
		{
			return 34.4;
		}
		else if ( stage == 2 )
		{
			return 35.4;
		}
		else if ( stage == 3 )
		{
			return 36.7;
		}
		else if ( stage == 4 )
		{
			return 37.5;
		}

		return 40.1;
	}

	float GetHudTestHealthValue(int stage, float max_health)
	{
		if ( stage == 1 )
		{
			return max_health * 0.15;
		}
		else if ( stage == 2 )
		{
			return max_health * 0.35;
		}
		else if ( stage == 3 )
		{
			return max_health * 0.65;
		}
		else if ( stage == 4 )
		{
			return max_health * 0.85;
		}

		return max_health;
	}

	float GetHudTestBloodValue(int stage, float max_blood)
	{
		if ( stage == 1 )
		{
			return max_blood * 0.15;
		}
		else if ( stage == 2 )
		{
			return max_blood * 0.35;
		}
		else if ( stage == 3 )
		{
			return max_blood * 0.65;
		}
		else if ( stage == 4 )
		{
			return max_blood * 0.85;
		}

		return max_blood;
	}

	int GetHudTestHungerDisplayLevel(int stage)
	{
		if ( stage == 1 )
		{
			return DELM_LVL_4;
		}
		else if ( stage == 2 )
		{
			return DELM_LVL_3;
		}
		else if ( stage == 3 )
		{
			return DELM_LVL_2;
		}
		else if ( stage == 4 )
		{
			return DELM_LVL_0;
		}

		return DELM_LVL_1;
	}

	int GetHudTestTemperatureDisplayLevel(int stage)
	{
		if ( stage == 1 )
		{
			return DELM_LVL_5;
		}
		else if ( stage == 2 )
		{
			return DELM_LVL_4;
		}
		else if ( stage == 3 )
		{
			return DELM_LVL_0;
		}
		else if ( stage == 4 )
		{
			return DELM_LVL_3;
		}

		return DELM_LVL_1;
	}
	
	void OnRPCToggleGodMode( PlayerBase player )
	{
		ToggleGodMode( player );
	}
	
	void OnRPCClearInventory(PlayerBase player)
	{
		ClearInventory(player);
	}
	
	// Client -> Server Spawning: Server Side
	void OnRPCSpawnItem(PlayerBase player, ParamsReadContext ctx)
	{
		autoptr Param5<string, int, float, float, float> p = new Param5<string, int, float, float, float>("", 0, 0, 0, 0);
		if ( ctx.Read(p) )
		{
			SpawnItem(player, p.param1, p.param2,  p.param3,  p.param4, p.param5);
		}
	}
	
	// God mode
	bool IsGodMode()
	{
		return m_GodModeEnabled;
	}

	void ToggleGodModeThisPlayer()
	{
		ToggleGodMode( GetGame().GetPlayer() );
	}

	void ToggleGodMode( PlayerBase player )
	{
		if ( GetGame().IsServer() )
		{
			if ( !m_GodModeEnabled )
			{
				player.SetAllowDamage( false );
				player.MessageStatus( "God mode ENABLED" );
				m_GodModeEnabled = true;
			}
			else
			{
				player.SetAllowDamage( true );
				player.MessageStatus( "God mode DISABLED" );
				m_GodModeEnabled = false;
			}
		}
		else
		{
			if ( !m_GodModeEnabled )
			{
				m_GodModeEnabled = true;
			}
			else
			{
				m_GodModeEnabled = false;
			}
			player.RPCSingleParam( DEV_RPC_TOGGLE_GOD_MODE, NULL );
		}
	}
	
	// Spawn item in palyer
	EntityAI SpawnItem(PlayerBase player, string item_name, int spawn_type, float damage, float quantity, float distance = 0, bool usePosition = false, vector pos = "0 0 0", EntityAI attachmentObject = NULL)
	{
		if ( player == NULL )
		{
			if ( GetGame().IsServer() )
			{
				SendServerLogToClient("[DEV SPAWN] Failed: player is NULL for " + item_name);
			}
			return NULL;
		}
		
		if ( quantity == -1 && !GetDayZGame().IsNewPlayer() )
		{
			quantity = GetGame().ConfigGetInt( CFG_VEHICLESPATH + " " + item_name + " stackedMax");
		}

		if ( GetGame().IsServer() )
		{		
			// Client -> Server Spawning: Server Side
			bool full_quantity = true;
			if ( IsAdminSpawnWeaponClass(item_name) )
			{
				full_quantity = false;
			}

			EntityAI entity = player.SpawnItem(item_name, spawn_type, full_quantity, distance, usePosition, pos, attachmentObject);
			
			if ( entity == NULL )
			{
				SendServerLogToClient("[DEV SPAWN] Failed: " + item_name);
				return NULL;
			}
		
			if ( entity != NULL && entity.IsInherited(ItemBase) )
			{
				ItemBase item = entity;
				
				if ( damage > 0 )
				{
					item.SetDamage(damage);
				}
				if ( quantity > 0 )
				{
					if ( item.IsMagazine() )
					{
						Magazine mag = (Magazine)item;
						mag.SetAmmoCount(quantity);
					}
					else
					{
						item.SetQuantity(quantity);
					}
				}
				else if ( item.IsMagazine() )
				{
					Magazine full_mag = (Magazine)item;
					full_mag.SetAmmoMax();
				}
			}

			if ( entity.IsWeapon() )
			{
				SpawnAdminWeaponSupport(player, item_name);
			}
			
			SendServerLogToClient("[DEV SPAWN] OK: " + item_name);
			
			return entity;
		}
		else
		{		
			// Client -> Server Spawning: Client Side
			autoptr Param5<string, int, float, float, float> params = new Param5<string, int, float, float, float>(item_name, spawn_type, damage, quantity, distance);
			player.RPCSingleParam(DEV_RPC_SPAWN_ITEM, params);
		}
		
		return NULL;
	}

	protected bool IsAdminSpawnWeaponClass(string item_name)
	{
		string lower = item_name;
		lower.ToLower();

		if ( lower.Contains("m4a1") || lower.Contains("akm") || lower.Contains("ak101") || lower.Contains("ak74") || lower.Contains("aks74u") )
		{
			return true;
		}
		if ( lower.Contains("cz527") || lower.Contains("izh18") || lower.Contains("mosin") || lower.Contains("sks") || lower.Contains("svd") )
		{
			return true;
		}
		if ( lower.Contains("mp5") || lower.Contains("pm73") || lower.Contains("ump45") || lower.Contains("vss") || lower.Contains("fal") )
		{
			return true;
		}
		if ( lower.Contains("fnx45") || lower.Contains("glock") || lower.Contains("magnum") || lower.Contains("red9") || lower.Contains("cz75") )
		{
			return true;
		}
		if ( lower.Contains("ij70") || lower.Contains("p1") || lower.Contains("1911") || lower.Contains("ruger1022") )
		{
			return true;
		}
		if ( lower.Contains("izh43") || lower.Contains("mp133") || lower.Contains("saiga") || lower.Contains("winchester") )
		{
			return true;
		}

		return false;
	}

	protected void SpawnAdminWeaponSupport(PlayerBase player, string weapon_name)
	{
		string magazine = GetAdminWeaponMagazine(weapon_name);
		string ammo = GetAdminWeaponAmmo(weapon_name);

		if ( magazine.Length() > 0 )
		{
			SpawnAdminSupportItem(player, magazine);
			SpawnAdminSupportItem(player, magazine);
		}

		if ( ammo.Length() > 0 )
		{
			SpawnAdminSupportItem(player, ammo);
		}
	}

	protected void SpawnAdminSupportItem(PlayerBase player, string item_name)
	{
		EntityAI entity = player.SpawnItem(item_name, SPAWNTYPE_INVENTORY, false, 0);
		if ( entity && entity.IsInherited(ItemBase) )
		{
			ItemBase item = (ItemBase)entity;
			if ( item.IsMagazine() )
			{
				Magazine mag = (Magazine)item;
				mag.SetAmmoMax();
			}
			else
			{
				item.SetQuantity(item.GetQuantityMax());
			}
		}
	}

	protected string GetAdminWeaponMagazine(string weapon_name)
	{
		string lower = weapon_name;
		lower.ToLower();

		if ( lower.Contains("ak101") )
		{
			return "EN5C_Mag_AK101_30Rnd";
		}
		if ( lower.Contains("ak74") || lower.Contains("aks74u") )
		{
			return "EN5C_Mag_AK74_30Rnd";
		}
		if ( lower.Contains("akm") )
		{
			return "EN5C_Mag_AKM_30Rnd";
		}
		if ( lower.Contains("m4a1") )
		{
			return "EN5C_Mag_CMAG_30Rnd";
		}
		if ( lower.Contains("cz527") )
		{
			return "EN5C_Mag_CZ527_5rnd";
		}
		if ( lower.Contains("mp5") )
		{
			return "EN5C_Mag_MP5_30Rnd";
		}
		if ( lower.Contains("pm73") )
		{
			return "EN5C_Mag_PM73_25Rnd";
		}
		if ( lower.Contains("ump45") )
		{
			return "EN5C_Mag_UMP_25Rnd";
		}
		if ( lower.Contains("svd") )
		{
			return "EN5C_Mag_SVD_10Rnd";
		}
		if ( lower.Contains("vss") )
		{
			return "EN5C_Mag_VSS_10Rnd";
		}
		if ( lower.Contains("fal") )
		{
			return "EN5C_Mag_FAL_20Rnd";
		}
		if ( lower.Contains("fnx45") )
		{
			return "EN5C_Mag_FNX45_15Rnd";
		}
		if ( lower.Contains("glock") )
		{
			return "EN5C_Mag_Glock_15Rnd";
		}
		if ( lower.Contains("cz75") )
		{
			return "EN5C_Mag_CZ75_15Rnd";
		}
		if ( lower.Contains("ij70") )
		{
			return "EN5C_Mag_IJ70_8Rnd";
		}
		if ( lower.Contains("izh43") || lower.Contains("mp133") )
		{
			return "EN5C_Mag_12gaSnaploader_2Rnd";
		}
		if ( lower.Contains("saiga") )
		{
			return "EN5C_Mag_Saiga_8Rnd";
		}
		if ( lower.Contains("p1") )
		{
			return "EN5C_Mag_P1_8Rnd";
		}
		if ( lower.Contains("1911") )
		{
			return "EN5C_Mag_1911_7Rnd";
		}
		if ( lower.Contains("magnum") )
		{
			return "EN5C_Mag_357Speedloader_6Rnd";
		}
		if ( lower.Contains("ruger1022") )
		{
			return "EN5C_Mag_Ruger1022_10Rnd";
		}
		if ( lower.Contains("mosin") )
		{
			return "EN5C_Mag_CLIP762x54_5Rnd";
		}
		if ( lower.Contains("sks") )
		{
			return "EN5C_Mag_CLIP762x39_10Rnd";
		}
		if ( lower.Contains("winchester") )
		{
			return "EN5C_Mag_308WinSnaploader_2Rnd";
		}
		return "";
	}

	protected string GetAdminWeaponAmmo(string weapon_name)
	{
		string lower = weapon_name;
		lower.ToLower();

		if ( lower.Contains("m4a1") || lower.Contains("ak101") )
		{
			return "EN5C_Ammo_556x45";
		}
		if ( lower.Contains("ak74") || lower.Contains("aks74u") )
		{
			return "EN5C_Ammo_545x39";
		}
		if ( lower.Contains("akm") || lower.Contains("cz527") || lower.Contains("izh18") || lower.Contains("sks") )
		{
			return "EN5C_Ammo_762x39";
		}
		if ( lower.Contains("mosin") || lower.Contains("svd") )
		{
			return "EN5C_Ammo_762x54";
		}
		if ( lower.Contains("winchester") || lower.Contains("fal") )
		{
			return "EN5C_Ammo_308Win";
		}
		if ( lower.Contains("mp5") || lower.Contains("glock") || lower.Contains("cz75") || lower.Contains("p1") || lower.Contains("red9") )
		{
			return "EN5C_Ammo_9x19";
		}
		if ( lower.Contains("fnx45") || lower.Contains("ump45") || lower.Contains("1911") )
		{
			return "EN5C_Ammo_45ACP";
		}
		if ( lower.Contains("izh43") || lower.Contains("mp133") || lower.Contains("saiga") )
		{
			return "EN5C_Ammo_12gaPellets";
		}
		if ( lower.Contains("pm73") || lower.Contains("ij70") )
		{
			return "EN5C_Ammo_380";
		}
		if ( lower.Contains("magnum") )
		{
			return "EN5C_Ammo_357";
		}
		if ( lower.Contains("ruger1022") )
		{
			return "EN5C_Ammo_22";
		}
		if ( lower.Contains("vss") )
		{
			return "EN5C_Ammo_9x39";
		}
		return "";
	}
	
	EntityAI SpawnFromClipboard()
	{
		UIScriptedMenu menu_curr = GetGame().GetUIManager().GetMenu();

		if ( menu_curr == NULL )
		{			
			PlayerBase player = GetGame().GetPlayer();
			if ( player )
			{
				vector pos_player = player.GetPosition();
				
				// Get item from clipboard
				string		item_name;
				GetGame().CopyFromClipboard(item_name);
				
				// Find cursor position in the world
				vector pos_cursor = GetGame().GetCursorPos();
				
				// Spawn & Synch Item
				EntityAI object_spawned_from_clipboard = SpawnItem(player, item_name, SPAWNTYPE_GROUND, 0, -1, vector.Distance (pos_cursor, pos_player ));
				Print(object_spawned_from_clipboard);
				return object_spawned_from_clipboard;
			}
		}
		
		return NULL;
	}
	
	// Clear Player Inventory
	void ClearInventory(PlayerBase player)
	{
		if ( GetGame().IsServer() )
		{
			player.ClearInventory();
		}
		else
		{
			Param1<int> params = new Param1<int>(0);
			player.RPCSingleParam(DEV_RPC_CLEAR_INV, params);
		}
	}	
	
	// Debug Console: SQF Debug Watchers
	void ToggleHelpScreen()
	{
		if ( g_Game.GetUIManager().GetMenu() == NULL )
		{
			g_Game.GetUIManager().EnterScriptedMenu(MENU_HELP_SCREEN, NULL);
		}
		else if ( g_Game.GetUIManager().IsMenuOpen(MENU_HELP_SCREEN) )
		{
			g_Game.GetUIManager().Back();
		}
	}

	void ToggleScriptConsole()
	{
		if ( g_Game.GetUIManager().GetMenu() == NULL )
		{
			g_Game.GetUIManager().EnterScriptedMenu(MENU_SCRIPTCONSOLE, NULL);
			SendDebugPingToServer();
		}
		else if ( g_Game.GetUIManager().IsMenuOpen(MENU_SCRIPTCONSOLE) )
		{
			g_Game.GetUIManager().Back();
		}
	}

	void ToggleMapMenu()
	{
		if ( g_Game.GetUIManager().GetMenu() == NULL )
		{
			g_Game.GetUIManager().EnterScriptedMenu(MENU_MAP, NULL);
		}
		else if ( g_Game.GetUIManager().IsMenuOpen(MENU_MAP) )
		{
			g_Game.GetUIManager().Back();
		}
	}

	void AddSqfDebugWatcher( int id )
	{
		m_SqfDebugWatchers.Insert( new SqfDebugWatcher(id, "") );
	}

	bool RemoveLastSqfDebugWatcher()
	{
		if ( m_SqfDebugWatchers.Count() > MIN_SQF_DEBUG_WATCHERS_COUNT )
		{
			int last_sqf_debug_watcher = m_SqfDebugWatchers.Count() - 1;
			
			SqfDebugWatcher sqf_debug_watcher = m_SqfDebugWatchers.Get( last_sqf_debug_watcher );
			delete sqf_debug_watcher;
			
			m_SqfDebugWatchers.Remove( last_sqf_debug_watcher );
			
			return true;
		}
		
		return false;
	}

	void UpdateSqfDebugWatchers()
	{
		for ( int i = 0; i < m_SqfDebugWatchers.Count(); i++ )
		{
			SqfDebugWatcher sqf_debug_watcher = m_SqfDebugWatchers.Get( i );
				
			if ( sqf_debug_watcher != NULL && sqf_debug_watcher.IsRunning() )
			{
				if ( GetGame().GetUIManager().IsMenuOpen(MENU_SCRIPTCONSOLE) || GetGame().GetUIManager().IsMenuOpen(MENU_SCENE_EDITOR))
				{
						string sqf_command = GetSqfCommandForSqfDebugWatcher( m_SqfDebugWatchers.Get( i ).GetId() );
						m_SqfDebugWatchers.Get( i ).SetSqfCommand( sqf_command );
				}
					
				m_SqfDebugWatchers.Get( i ).Execute();
			}
		}
	}

	int GetSqfDebugWatchersCount()
	{
		return m_SqfDebugWatchers.Count();
	}

	SqfDebugWatcher GetSqfDebugWatcherById( int id )
	{
		for ( int i = 0; i < m_SqfDebugWatchers.Count(); i++ )
		{
			SqfDebugWatcher sqf_debug_watcher = m_SqfDebugWatchers.Get( i );
			if ( id == sqf_debug_watcher.GetId() )
			{
				return sqf_debug_watcher;
			}
		}
		
		return NULL;
	}

	string GetSqfCommandForSqfDebugWatcher( int id )
	{
		autoptr Param1<int> param = new Param1<int>(id);
		Param1<string> p = CallMethod( CALL_ID_SCR_CNSL_GET_SQF_WATCHER, param);
		string s = p.param1;
		delete p;
		return s;
	}

	void OnSqfDebugWatcherResult( int id, string result )
	{
		autoptr Param2<int, string> param = new Param2<int, string>(id, result);
		CallMethod( CALL_ID_SCR_CNSL_SET_SQF_WATCHER_RESULT, param);
	}
	
	// Mission Editor
	void ToggleMissionLoader()
	{
		g_Game.GetUIManager().OpenWindow( GUI_WINDOW_MISSION_LOADER );
	}
	
	// Script Editor History
	private void ScriptHistoryNext()
	{
		// Console key press
		if ( g_Game.GetUIManager().IsMenuOpen(MENU_SCRIPTCONSOLE) )
		{
			CallMethod(CALL_ID_SCR_CNSL_HISTORY_NEXT, NULL);
		}
	}

	private	void ScriptHistoryBack()
	{
		// Console key press
		if ( g_Game.GetUIManager().IsMenuOpen(MENU_SCRIPTCONSOLE) )
		{
			CallMethod(CALL_ID_SCR_CNSL_HISTORY_BACK, NULL);
		}
	}
	
	private bool IsIngame()
	{
		UIScriptedMenu menu_curr = GetGame().GetUIManager().GetMenu();

		if ( menu_curr == NULL )
		{			
			return true;
		}
		
		return false;
	}

	private bool IsInConsole()
	{
		UIScriptedMenu menu_curr = GetGame().GetUIManager().GetMenu();

		if ( menu_curr != NULL && menu_curr.GetID() == MENU_SCRIPTCONSOLE )
		{			
			return true;
		}
		
		return false;
	}
	
	// Tools
	int QuickSortPartition( out TStringArray arr, local int left, local int right )
	{
		local string pivot = arr.Get( left );
		local int i = left;
		local int j = right + 1;
		local string temp;
			
		while ( true )
		{
			while ( true )
			{ 
				i++; 
				if ( i > right || arr.Get(i) > pivot )
				{
					break;
				}
			}
			
			while ( true )
			{
				j--; 
				if ( arr.Get(j) <= pivot )
				{
					break;
				}
			}
			
			if ( i >= j ) 
			{
				break;
			}
			
			temp = arr.Get( i ); 
			arr.Set( i, arr.Get(j) ); 
			arr.Set( j, temp );
		}
		
		temp = arr.Get( left ); 
		arr.Set( left, arr.Get(j) );
		arr.Set( j, temp );
	   
		return j;
	}

	void QuickSort( out TStringArray arr, local int left, local int right )
	{
		local int j;

		if ( left < right ) 
		{
			j = QuickSortPartition( arr, left, right );
			QuickSort( arr, left, j - 1 );
			QuickSort( arr, j + 1, right );
		}
	}

	void SortStringArray( out TStringArray arr )
	{
		QuickSort( arr, 0, arr.Count() - 1 );
	}
}

void OnSqfDebugWatcherResult( int id, string result )
{
	PluginDeveloper.GetInstance().OnSqfDebugWatcherResult( id, result );
}
