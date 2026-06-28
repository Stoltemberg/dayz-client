class MissionServer extends MissionBase
{
	Timer m_UpdatePlayerStatsTimer;
	autoptr array<Man> m_Players;
	int m_currentPlayer;
	
	void MissionServer()
	{
		m_UpdatePlayerStatsTimer = new Timer( CALL_CATEGORY_GAMEPLAY );
		m_UpdatePlayerStatsTimer.Run( 30, this, "UpdatePlayersStats", NULL, true );
		UpdatePlayersStats();
		m_Players = new array<Man>;
		m_currentPlayer = 0;
	}

	void ~MissionServer()
	{
		delete m_UpdatePlayerStatsTimer;
	}

	void TickScheduler(float timeslice)
	{
		GetGame().GetWorld().GetPlayerList(m_Players);//REWORK.V: this is kind of bad, as we construct this array on every single tick: programmers are fine with this, but lets see if we can construct our own static array, will also fix duplicate ticks when players disconnect

		if(m_currentPlayer >= m_Players.Count() )
		{
			m_currentPlayer = 0;
		} 
		//actual code for player ticks
		if( m_Players.Count() != 0 )
		{

			PlayerBase currentPlayer = m_Players.Get(m_currentPlayer);
			currentPlayer.OnTick();
			m_currentPlayer++;
			
		}
		
	}

	void OnInit()
	{
		/*
		// example
		Print("This is Server mission");
		
		Print("vytvaram triggeraaaaaa");
		vector player_pos = "2558 16 2854";
		g_Game.CreateObject("ItemTrigger", player_pos, false);
		//SpawnItems();*/
	}
	
	void OnUpdate(float timeslice)
	{
		TickScheduler(timeslice);//tick scheduler
	}

	bool IsServer()
	{
		return true;
	}	
	
	void UpdatePlayersStats()
	{
		autoptr array<Man> players = new array<Man>;
		g_Game.GetWorld().GetPlayerList( players );
			
		for ( int i = 0; i < players.Count(); i++ )
		{
			PlayerBase player = players.Get(i);

			// NEW STATS API
			player.StatUpdateByTime("playtime");
			player.StatUpdateByPosition("dist");

			PluginLifespan module_lifespan = (PluginLifespan)GetPlugin( PluginLifespan );
			module_lifespan.UpdateLifespanLevelAuto( player );
		}
	}

	void OnEvent(EventType eventTypeId, Param params) 
	{
		PlayerIdentity identity;
		PlayerBase player;
		
		switch(eventTypeId)
		{
		case ClientNewEventTypeID:
			Print("MissionServer::OnEvent::NEW event");
			ClientNewEventParams newParams = params;
			//SurvivorPartsMaleAsian is just temporary until something like DZ_SkinsArray exists

/*
inventory_tops = [
	"EN5C_Hoodie_Blue","EN5C_Hoodie_Black","EN5C_Hoodie_Brown","EN5C_Hoodie_Green","EN5C_Hoodie_Grey","EN5C_Hoodie_Red",
	"EN5C_Raincoat_Green","EN5C_Raincoat_Yellow","EN5C_Raincoat_Orange",
	"EN5C_TacticalShirt_Grey","EN5C_TacticalShirt_Black","EN5C_TacticalShirt_Olive","TacticalShirtTan"
];
inventory_pants = [
	"EN5C_Jeans_Black","EN5C_Jeans_BlueDark","EN5C_Jeans_Blue","EN5C_Jeans_Brown","EN5C_Jeans_Grey","EN5C_Jeans_Green",
	"EN5C_CargoPants_Beige"
];
inventory_shoes = [
	"EN5C_HikingBoots_Brown","EN5C_AthleticShoes_Blue"
];
*/
			Entity playerEnt = GetGame().CreateEntity(newParams.param1, "SurvivorENFANIMSYS", newParams.param2, 0, "NONE");

			identity = newParams.param1;
			player = (PlayerBase)playerEnt;

/*
			ItemBase item = player.CreateInInventory( "EN5C_Hoodie_Blue" );
			ItemBase item = player.CreateInInventory( "EN5C_Jeans_Black" );
			ItemBase item = player.CreateInInventory( "EN5C_HikingBoots_Brown" );
*/
			ItemBase item = player.CreateInInventory( "Shirt_CheckWhiteNewAnimSystem" ); 
			ItemBase item2 = player.CreateInInventory( "JeansAbove" );
			ItemBase item3 = player.CreateInInventory( "AthleticShoesAbove" );
			ItemBase item4 = player.CreateInInventory( "BagTortillaAbove" );
			
			
			ItemBase item5 = player.CreateInInventory( "EN5C_Rag" );
			item.SetQuantity2( 1 );
			item = player.CreateInInventory( "EN5C_Roadflare" );

			GetGame().SelectPlayer(identity, player);

			/*
			PluginSpawnSelector module_spawn_new = PluginSpawnSelector.GetInstance();
			if ( module_spawn_new )
			{
				module_spawn_new.OnPlayerReady(identity, player);
			}
			*/

			break;
			
		case ClientRespawnEventTypeID:
			Print("MissionServer::OnEvent::RESPANW event");
			ClientRespawnEventParams respawnParams = params;
			/*
			PluginSpawnSelector module_spawn_respawn = PluginSpawnSelector.GetInstance();
			if ( module_spawn_respawn )
			{
				module_spawn_respawn.MarkRespawnPending(respawnParams.param1);
			}
			*/
			break;
			
		case ClientReadyEventTypeID:
			Print("MissionServer::OnEvent::READY event");
			ClientReadyEventParams readyParams = params;
			InvokeOnConnect(params);
			
			identity = readyParams.param1;
			player = (PlayerBase)readyParams.param2;
			
			GetGame().SelectPlayer(identity, player);

			PluginAdmin module_admin_ready = PluginAdmin.GetInstance();
			if ( module_admin_ready )
			{
				module_admin_ready.RegisterPlayer(identity, player);
				if ( module_admin_ready.EnforceBan(identity, player) )
				{
					break;
				}
			}

			PluginCustomChat module_chat_ready = PluginCustomChat.GetInstance();
			if ( module_chat_ready )
			{
				module_chat_ready.RegisterPlayer(identity, player);
			}

			PluginParty module_party_ready = PluginParty.GetInstance();
			if ( module_party_ready )
			{
				module_party_ready.RegisterPlayer(identity, player);
			}

			/*
			PluginSpawnSelector module_spawn_ready = PluginSpawnSelector.GetInstance();
			if ( module_spawn_ready )
			{
				module_spawn_ready.OnPlayerReady(identity, player);
			}
			*/
			break;
		
		case ClientDisconnectedEventTypeID:
			Print("MissionServer::OnEvent::DISCONECTED event");
			ClientDisconnectedEventParams discoParams = params;
			InvokeOnDisconnect(params);
			int queueTime = discoParams.param3;

			PluginAdmin module_admin_disco = PluginAdmin.GetInstance();
			if ( module_admin_disco )
			{
				module_admin_disco.UnregisterPlayer(discoParams.param1, (PlayerBase)discoParams.param2);
			}

			PluginCustomChat module_chat_disco = PluginCustomChat.GetInstance();
			if ( module_chat_disco )
			{
				module_chat_disco.UnregisterPlayer(discoParams.param1, (PlayerBase)discoParams.param2);
			}

			PluginParty module_party_disco = PluginParty.GetInstance();
			if ( module_party_disco )
			{
				module_party_disco.UnregisterPlayer(discoParams.param1, (PlayerBase)discoParams.param2);
			}
			
			// wait for queueTime seconds and then call: 
			GetGame().SetPlayerDisconnected(discoParams.param1);
			break;
			
		case ClientQueuedEventTypeID:
			break;
		}
	}
	
	void InvokeOnConnect(Param params)
	{
		Debug.Log("InvokeOnConnect:"+this.ToString(),"Connect");
		ClientReadyEventParams prms = params;
		PlayerBase player = prms.param2;
		if( player ) player.OnConnect();
	}

	void InvokeOnDisconnect(Param params)
	{
		Debug.Log("InvokeOnDisconnect:"+this.ToString(),"Connect");
		ClientDisconnectedEventParams prms = params;
		PlayerBase player = prms.param2;
		if( player ) player.OnDisconnect();
	}
}
