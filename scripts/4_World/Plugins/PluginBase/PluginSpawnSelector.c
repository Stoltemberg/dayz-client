class PluginSpawnSelector extends PluginBase
{
	protected const string SPAWN_CMD_SELECT = "select";

	protected autoptr TStringArray m_PendingUIDs;
	protected autoptr TStringArray m_SpawnNames;
	protected autoptr array<vector> m_SpawnPositions;
	protected autoptr array<PlayerBase> m_TeleportPlayers;
	protected autoptr array<vector> m_TeleportPositions;
	protected autoptr TStringArray m_TeleportNames;
	protected autoptr Timer m_ClientOpenTimer;
	protected autoptr Timer m_ServerTeleportTimer;

	protected string m_LocalSpawnBlob;
	protected string m_LocalMessage;
	protected int m_StateVersion;
	protected int m_ClientOpenAttempts;

	static PluginSpawnSelector GetInstance()
	{
		return GetPlugin(PluginSpawnSelector);
	}

	void PluginSpawnSelector()
	{
		m_PendingUIDs = new TStringArray;
		m_SpawnNames = new TStringArray;
		m_SpawnPositions = new array<vector>;
		m_TeleportPlayers = new array<PlayerBase>;
		m_TeleportPositions = new array<vector>;
		m_TeleportNames = new TStringArray;
		m_ClientOpenTimer = new Timer(CALL_CATEGORY_GUI);
		m_ServerTeleportTimer = new Timer();

		m_LocalSpawnBlob = "";
		m_LocalMessage = "";
		m_StateVersion = 0;
		m_ClientOpenAttempts = 0;

		BuildSpawnList();
	}

	void OnDestroy()
	{
		if ( m_ClientOpenTimer )
		{
			m_ClientOpenTimer.Stop();
		}

		if ( m_ServerTeleportTimer )
		{
			m_ServerTeleportTimer.Stop();
		}
	}

	void MarkRespawnPending(PlayerIdentity identity)
	{
		if ( !GetGame().IsServer() || !identity )
		{
			return;
		}

		string uid = identity.GetId();
		if ( uid.Length() == 0 )
		{
			return;
		}

		if ( m_PendingUIDs.Find(uid) < 0 )
		{
			m_PendingUIDs.Insert(uid);
		}

		Print("[SpawnSelector] Respawn pendente para UID " + uid);
	}

	void OnPlayerReady(PlayerIdentity identity, PlayerBase player)
	{
		if ( !GetGame().IsServer() || !identity || !player )
		{
			return;
		}

		string uid = identity.GetId();
		int index = m_PendingUIDs.Find(uid);
		if ( index < 0 )
		{
			return;
		}

		m_PendingUIDs.Remove(index);
		SendSpawnMenu(player);
	}

	void RequestTeleport(string spawn_name)
	{
		PlayerBase player = GetGame().GetPlayer();
		if ( !player || spawn_name.Length() == 0 )
		{
			return;
		}

		Param1<string> request = new Param1<string>(spawn_name);
		player.RPCSingleParam(SPAWN_SELECTOR_RPC_SELECT, request);
	}

	void ClientRespawnRequested()
	{
		if ( !GetGame().IsClient() )
		{
			return;
		}

		m_LocalSpawnBlob = BuildSpawnBlob();
		m_LocalMessage = "Aguardando novo personagem para escolher spawn.";
		m_StateVersion++;
		m_ClientOpenAttempts = 0;

		if ( m_ClientOpenTimer )
		{
			m_ClientOpenTimer.Stop();
			m_ClientOpenTimer.Run(1.0, this, "TryOpenLocalSelectorAfterRespawn", NULL, true);
		}
	}

	string GetLocalSpawnBlob()
	{
		return m_LocalSpawnBlob;
	}

	string GetLocalMessage()
	{
		return m_LocalMessage;
	}

	int GetStateVersion()
	{
		return m_StateVersion;
	}

	void OnRPC(PlayerBase player, int rpc_type, ParamsReadContext ctx)
	{
		if ( GetGame().IsServer() )
		{
			if ( rpc_type == SPAWN_SELECTOR_RPC_SELECT )
			{
				HandleServerSelect(player, ctx);
			}
		}
		else if ( GetGame().IsClient() )
		{
			if ( rpc_type == SPAWN_SELECTOR_RPC_SHOW )
			{
				HandleClientShow(ctx);
			}
			else if ( rpc_type == SPAWN_SELECTOR_RPC_MESSAGE )
			{
				HandleClientMessage(ctx);
			}
		}
	}

	protected void BuildSpawnList()
	{
		m_SpawnNames.Clear();
		m_SpawnPositions.Clear();

		AddSpawn("Kamenka", Vector(1900.0, 20.0, 2200.0));
		AddSpawn("West Balota", Vector(4200.0, 20.0, 2500.0));
		AddSpawn("Chernogorsk", Vector(6893.07, 20.0, 2618.13));
		AddSpawn("Eletrozavodsk", Vector(10459.8, 20.0, 2322.72));
		AddSpawn("Kamyshovo", Vector(12100.0, 20.0, 3450.0));
		AddSpawn("Solnechny", Vector(13393.1, 20.0, 6159.8));
		AddSpawn("Berezino", Vector(12296.9, 20.0, 9470.51));
		AddSpawn("Svetloyarsk", Vector(13835.3, 20.0, 13202.3));
	}

	protected void AddSpawn(string name, vector position)
	{
		m_SpawnNames.Insert(name);
		m_SpawnPositions.Insert(position);
	}

	protected string BuildSpawnBlob()
	{
		string blob = "";

		for ( int i = 0; i < m_SpawnNames.Count(); i++ )
		{
			if ( blob.Length() > 0 )
			{
				blob = blob + "\n";
			}

			blob = blob + m_SpawnNames.Get(i) + "|" + m_SpawnPositions.Get(i).ToString();
		}

		return blob;
	}

	protected void SendSpawnMenu(PlayerBase player)
	{
		if ( !player )
		{
			return;
		}

		Param1<string> payload = new Param1<string>(BuildSpawnBlob());
		player.RPCSingleParam(SPAWN_SELECTOR_RPC_SHOW, payload, player);
		Print("[SpawnSelector] Menu enviado ao player.");
	}

	protected void HandleClientShow(ParamsReadContext ctx)
	{
		Param1<string> payload = new Param1<string>("");
		if ( !ctx.Read(payload) )
		{
			return;
		}

		m_LocalSpawnBlob = payload.param1;
		m_LocalMessage = "Selecione uma cidade e clique em usar spawn.";
		m_StateVersion++;
		OpenLocalSelectorMenu();
	}

	protected void HandleClientMessage(ParamsReadContext ctx)
	{
		Param1<string> message = new Param1<string>("");
		if ( !ctx.Read(message) )
		{
			return;
		}

		m_LocalMessage = message.param1;
		m_StateVersion++;
		Print("[SpawnSelector] " + m_LocalMessage);
	}

	protected void HandleServerSelect(PlayerBase player, ParamsReadContext ctx)
	{
		if ( !player )
		{
			return;
		}

		Param1<string> request = new Param1<string>("");
		if ( !ctx.Read(request) )
		{
			SendMessage(player, "Pedido de spawn invalido.");
			return;
		}

		string spawn_name = request.param1.Trim();
		int spawn_index = m_SpawnNames.Find(spawn_name);
		if ( spawn_index < 0 )
		{
			SendMessage(player, "Cidade de spawn invalida.");
			return;
		}

		vector position = m_SpawnPositions.Get(spawn_index);
		position[0] = position[0] + Math.RandomFloat(-35.0, 35.0);
		position[2] = position[2] + Math.RandomFloat(-35.0, 35.0);
		position[1] = 20.0;

		if ( IsInvalidWorldPosition(position) )
		{
			SendMessage(player, "Posicao de spawn invalida para " + spawn_name + ".");
			Print("[SpawnSelector] Posicao recusada para " + spawn_name + ": " + position.ToString());
			return;
		}

		QueueServerTeleport(player, position, spawn_name);
		Print("[SpawnSelector] Teleporte agendado para " + spawn_name + " em " + position.ToString());
	}

	protected void QueueServerTeleport(PlayerBase player, vector position, string spawn_name)
	{
		if ( !player )
		{
			return;
		}

		int existing_index = m_TeleportPlayers.Find(player);
		if ( existing_index >= 0 )
		{
			m_TeleportPlayers.Remove(existing_index);
			m_TeleportPositions.Remove(existing_index);
			m_TeleportNames.Remove(existing_index);
		}

		m_TeleportPlayers.Insert(player);
		m_TeleportPositions.Insert(position);
		m_TeleportNames.Insert(spawn_name);

		if ( m_ServerTeleportTimer )
		{
			m_ServerTeleportTimer.Stop();
			m_ServerTeleportTimer.Run(0.75, this, "FlushServerTeleports", NULL, false);
		}
	}

	protected void FlushServerTeleports()
	{
		for ( int i = 0; i < m_TeleportPlayers.Count(); i++ )
		{
			PlayerBase player = m_TeleportPlayers.Get(i);
			if ( !player || !player.IsAlive() )
			{
				continue;
			}

			vector position = m_TeleportPositions.Get(i);
			if ( IsInvalidWorldPosition(position) )
			{
				Print("[SpawnSelector] Teleporte cancelado por posicao invalida: " + position.ToString());
				continue;
			}
			DeveloperTeleport.SetPlayerPosition(player, position);
			Print("[SpawnSelector] Player enviado para " + m_TeleportNames.Get(i) + " em " + position.ToString());
		}

		m_TeleportPlayers.Clear();
		m_TeleportPositions.Clear();
		m_TeleportNames.Clear();
	}

	protected bool IsInvalidWorldPosition(vector position)
	{
		if ( position[0] < 100.0 || position[2] < 100.0 )
		{
			return true;
		}

		if ( position[0] > 16000.0 || position[2] > 16000.0 )
		{
			return true;
		}

		return false;
	}

	protected void SendMessage(PlayerBase player, string text)
	{
		if ( !player )
		{
			return;
		}

		Param1<string> message = new Param1<string>(text);
		player.RPCSingleParam(SPAWN_SELECTOR_RPC_MESSAGE, message, player);
	}

	protected void TryOpenLocalSelectorAfterRespawn()
	{
		m_ClientOpenAttempts++;

		PlayerBase player = GetGame().GetPlayer();
		if ( player && player.IsAlive() )
		{
			m_LocalMessage = "Selecione uma cidade e clique em usar spawn.";
			m_StateVersion++;
			if ( m_ClientOpenTimer )
			{
				m_ClientOpenTimer.Stop();
			}
			OpenLocalSelectorMenu();
			return;
		}

		if ( m_ClientOpenAttempts >= 10 )
		{
			if ( m_ClientOpenTimer )
			{
				m_ClientOpenTimer.Stop();
			}
			Print("[SpawnSelector] Respawn solicitado, mas o player vivo nao foi detectado.");
		}
	}

	protected void OpenLocalSelectorMenu()
	{
		UIManager manager = GetGame().GetUIManager();
		if ( !manager )
		{
			return;
		}

		if ( manager.IsMenuOpen(MENU_INGAME) )
		{
			manager.CloseMenu(MENU_INGAME);
		}

		if ( !manager.IsMenuOpen(MENU_SPAWN_SELECTOR) )
		{
			manager.EnterScriptedMenu(MENU_SPAWN_SELECTOR, NULL);
		}
	}
}
