class PluginAdmin extends PluginBase
{
	protected const string ADMIN_CONFIG_FILE = "!ConfigServer/admin_permissions.txt";
	protected const string ADMIN_BAN_FILE = "!ConfigServer/admin_bans.txt";
	protected const string ADMIN_CONFIG_DIR = "!ConfigServer";
	protected const int ADMIN_ROLE_NONE = 0;
	protected const int ADMIN_ROLE_MODERATOR = 1;
	protected const int ADMIN_ROLE_ADMIN = 2;
	protected const string ADMIN_CMD_GODMODE = "godmode";
	protected const string ADMIN_CMD_TP_TO_PLAYER = "tp_to_player";
	protected const string ADMIN_CMD_PULL_PLAYER = "pull_player";
	protected const string ADMIN_CMD_SPAWN_ITEMS = "spawn_items";
	protected const string ADMIN_CMD_SPAWN_ITEM = "spawn_item";
	protected const string ADMIN_CMD_TP_CITY = "tp_city";
	protected const string ADMIN_CMD_KICK_PLAYER = "kick_player";
	protected const string ADMIN_CMD_BAN_UID = "ban_uid";
	protected const string ADMIN_CMD_ESP = "esp";

	protected autoptr TStringArray m_AdminUIDs;
	protected autoptr TStringArray m_AdminNames;
	protected autoptr TStringArray m_ModeratorUIDs;
	protected autoptr TStringArray m_ModeratorNames;

	protected autoptr array<PlayerBase> m_OnlinePlayers;
	protected autoptr array<PlayerIdentity> m_OnlineIdentities;
	protected autoptr TStringArray m_OnlineUIDs;
	protected autoptr TStringArray m_OnlineNames;

	protected int m_LocalRole;
	protected string m_LocalUID;
	protected string m_LocalRoleName;
	protected string m_LastPlayerListBlob;
	protected int m_PlayerListVersion;
	protected bool m_LocalEspEnabled;

	static PluginAdmin GetInstance()
	{
		return GetPlugin( PluginAdmin );
	}

	void PluginAdmin()
	{
		m_AdminUIDs = new TStringArray;
		m_AdminNames = new TStringArray;
		m_ModeratorUIDs = new TStringArray;
		m_ModeratorNames = new TStringArray;
		m_OnlinePlayers = new array<PlayerBase>;
		m_OnlineIdentities = new array<PlayerIdentity>;
		m_OnlineUIDs = new TStringArray;
		m_OnlineNames = new TStringArray;
		m_LocalRole = ADMIN_ROLE_NONE;
		m_LocalUID = "";
		m_LocalRoleName = "Player";
		m_LastPlayerListBlob = "";
		m_PlayerListVersion = 0;
		m_LocalEspEnabled = false;
	}

	void OnInit()
	{
		super.OnInit();

		if ( GetGame().IsServer() )
		{
			LoadPermissions();
		}
	}

	void ToggleAdminMenu()
	{
		UIManager manager = GetGame().GetUIManager();
		if ( !manager )
		{
			Print("[Admin] UIManager indisponivel.");
			return;
		}

		if ( manager.IsMenuOpen(MENU_ADMIN) )
		{
			manager.Back();
			return;
		}

		PlayerBase player = GetGame().GetPlayer();
		if ( player )
		{
			player.MessageStatus("Solicitando painel admin...");
		}

		Print("[Admin] Pedido local de abertura do painel.");
		RequestAuth();
	}

	void RequestAuth()
	{
		PlayerBase player = GetGame().GetPlayer();
		if ( player )
		{
			player.RPCSingleParam(ADMIN_RPC_AUTH_REQUEST, NULL);
			Print("[Admin] RPC de autenticacao enviado.");
		}
		else
		{
			Print("[Admin] Nao foi possivel enviar auth: player local nulo.");
		}
	}

	void RequestPlayerList()
	{
		PlayerBase player = GetGame().GetPlayer();
		if ( player )
		{
			player.RPCSingleParam(ADMIN_RPC_PLAYER_LIST_REQUEST, NULL);
		}
	}

	void SendCommand(string command, string target_uid = "")
	{
		PlayerBase player = GetGame().GetPlayer();
		if ( player )
		{
			Param2<string, string> params = new Param2<string, string>(command, target_uid);
			player.RPCSingleParam(ADMIN_RPC_COMMAND, params);
		}
	}

	void ToggleLocalEsp()
	{
		m_LocalEspEnabled = !m_LocalEspEnabled;

		PlayerBase player = GetGame().GetPlayer();
		if ( player )
		{
			if ( m_LocalEspEnabled )
			{
				player.MessageStatus("ESP admin ligado.");
			}
			else
			{
				player.MessageStatus("ESP admin desligado.");
			}
		}
	}

	bool IsLocalEspEnabled()
	{
		return m_LocalEspEnabled;
	}

	void RegisterPlayer(PlayerIdentity identity, PlayerBase player)
	{
		if ( !GetGame().IsServer() || !identity || !player )
		{
			return;
		}

		string uid = identity.GetId();
		string name = identity.GetName();
		RegisterOnlinePlayer(identity, player, uid, SanitizeToken(name), true);
		Print("[Admin] Player registrado: " + name + " uid=" + uid);
	}

	bool EnforceBan(PlayerIdentity identity, PlayerBase player)
	{
		if ( !GetGame().IsServer() || !identity )
		{
			return false;
		}

		string uid = identity.GetId();
		if ( !IsUIDBanned(uid) )
		{
			return false;
		}

		if ( player )
		{
			SendMessage(player, "Seu UID esta banido deste servidor.");
		}

		Print("[Admin] UID banido desconectado: " + uid);
		GetGame().SetPlayerDisconnected(identity);
		return true;
	}

	void UnregisterPlayer(PlayerIdentity identity, PlayerBase player)
	{
		if ( !GetGame().IsServer() )
		{
			return;
		}

		string uid = "";
		if ( identity )
		{
			uid = identity.GetId();
		}

		RemoveOnlinePlayer(player, uid);
	}

	void OnRPC(PlayerBase player, int rpc_type, ParamsReadContext ctx)
	{
		if ( GetGame().IsServer() )
		{
			if ( rpc_type == ADMIN_RPC_AUTH_REQUEST )
			{
				HandleAuthRequest(player);
			}
			else if ( rpc_type == ADMIN_RPC_PLAYER_LIST_REQUEST )
			{
				HandlePlayerListRequest(player);
			}
			else if ( rpc_type == ADMIN_RPC_COMMAND )
			{
				HandleCommand(player, ctx);
			}
		}
		else
		{
			if ( rpc_type == ADMIN_RPC_AUTH_RESPONSE )
			{
				HandleAuthResponse(ctx);
			}
			else if ( rpc_type == ADMIN_RPC_PLAYER_LIST_RESPONSE )
			{
				HandlePlayerListResponse(ctx);
			}
			else if ( rpc_type == ADMIN_RPC_MESSAGE )
			{
				HandleClientMessage(ctx);
			}
		}
	}

	string GetLastPlayerListBlob()
	{
		return m_LastPlayerListBlob;
	}

	int GetPlayerListVersion()
	{
		return m_PlayerListVersion;
	}

	int GetLocalRole()
	{
		return m_LocalRole;
	}

	string GetLocalRoleName()
	{
		return m_LocalRoleName;
	}

	string GetLocalUID()
	{
		return m_LocalUID;
	}

	protected void HandleAuthRequest(PlayerBase player)
	{
		if ( !player )
		{
			return;
		}

		LoadPermissions();
		RefreshOnlinePlayersFromGame();

		string uid = "";
		string name = "";
		ResolvePlayerIdentity(player, uid, name);

		int role = GetRoleByUID(uid);
		string allowed = "0";

		if ( role > ADMIN_ROLE_NONE )
		{
			allowed = "1";
		}

		string payload = allowed + "|" + uid + "|" + role.ToString() + "|" + GetRoleName(role) + "|" + SanitizeToken(name);
		Param1<string> response = new Param1<string>(payload);
		player.RPCSingleParam(ADMIN_RPC_AUTH_RESPONSE, response, player);
		Print("[Admin] Auth request: uid=" + uid + " role=" + GetRoleName(role) + " allowed=" + allowed);

		if ( role <= ADMIN_ROLE_NONE )
		{
			SendMessage(player, "Acesso admin negado. UID: " + uid);
		}
	}

	protected void HandleAuthResponse(ParamsReadContext ctx)
	{
		Param1<string> response = new Param1<string>("");
		if ( !ctx.Read(response) )
		{
			return;
		}

		TStringArray parts = new TStringArray;
		response.param1.Split("|", parts);

		if ( parts.Count() < 4 )
		{
			return;
		}

		m_LocalUID = parts.Get(1);
		m_LocalRole = parts.Get(2).ToInt();
		m_LocalRoleName = parts.Get(3);
		Print("[Admin] Auth response: " + response.param1);

		if ( parts.Get(0) == "1" )
		{
			PlayerBase player_client = GetGame().GetPlayer();
			if ( player_client )
			{
				player_client.MessageStatus("Painel admin autorizado.");
			}

			UIManager manager = GetGame().GetUIManager();
			if ( manager )
			{
				UIScriptedMenu current_menu = manager.GetMenu();
				if ( current_menu && current_menu.GetID() == MENU_INGAME )
				{
					manager.CloseMenu(MENU_INGAME);
				}

				manager.EnterScriptedMenu(MENU_ADMIN, NULL);
			}
		}
		else
		{
			Print("[Admin] Acesso negado para UID " + m_LocalUID);
			PlayerBase denied_player = GetGame().GetPlayer();
			if ( denied_player )
			{
				denied_player.MessageStatus("Acesso admin negado. UID: " + m_LocalUID);
			}
		}
	}

	protected void HandlePlayerListRequest(PlayerBase player)
	{
		if ( !player )
		{
			return;
		}

		LoadPermissions();
		RefreshOnlinePlayersFromGame();

		string uid = "";
		string name = "";
		ResolvePlayerIdentity(player, uid, name);

		if ( GetRoleByUID(uid) <= ADMIN_ROLE_NONE )
		{
			SendMessage(player, "Voce nao tem permissao para listar jogadores. UID: " + uid);
			Print("[Admin] Lista negada: uid=" + uid);
			return;
		}

		string list_blob = BuildPlayerListBlob();
		if ( list_blob.Length() == 0 )
		{
			list_blob = BuildIdentityPlayerListBlob();
		}

		Param1<string> response = new Param1<string>(list_blob);
		player.RPCSingleParam(ADMIN_RPC_PLAYER_LIST_RESPONSE, response, player);
		Print("[Admin] Lista enviada para uid=" + uid + " players=" + CountBlobLines(list_blob).ToString());
	}

	protected void HandleCommand(PlayerBase player, ParamsReadContext ctx)
	{
		if ( !player )
		{
			return;
		}

		Param2<string, string> params = new Param2<string, string>("", "");
		if ( !ctx.Read(params) )
		{
			SendMessage(player, "Comando admin invalido.");
			return;
		}

		LoadPermissions();
		RefreshOnlinePlayersFromGame();

		string uid = "";
		string name = "";
		ResolvePlayerIdentity(player, uid, name);

		int role = GetRoleByUID(uid);
		if ( role <= ADMIN_ROLE_NONE )
		{
			SendMessage(player, "Comando negado. UID sem permissao: " + uid);
			Print("[Admin] Comando negado: cmd=" + params.param1 + " uid=" + uid);
			return;
		}

		string command = params.param1;
		string target_uid = params.param2;
		Print("[Admin] Comando recebido: " + command + " target=" + target_uid + " por uid=" + uid);

		if ( command == ADMIN_CMD_GODMODE )
		{
			HandleGodModeCommand(player);
		}
		else if ( command == ADMIN_CMD_TP_TO_PLAYER )
		{
			HandleTeleportToPlayerCommand(player, target_uid);
		}
		else if ( command == ADMIN_CMD_TP_CITY )
		{
			HandleTeleportCityCommand(player, target_uid);
		}
		else if ( command == ADMIN_CMD_PULL_PLAYER )
		{
			HandlePullPlayerCommand(player, target_uid);
		}
		else if ( command == ADMIN_CMD_SPAWN_ITEMS )
		{
			HandleSpawnItemsCommand(player);
		}
		else if ( command == ADMIN_CMD_SPAWN_ITEM )
		{
			HandleSpawnItemCommand(player, target_uid);
		}
		else if ( command == ADMIN_CMD_KICK_PLAYER )
		{
			HandleKickPlayerCommand(player, target_uid);
		}
		else if ( command == ADMIN_CMD_BAN_UID )
		{
			HandleBanUIDCommand(player, target_uid);
		}
		else if ( command == ADMIN_CMD_ESP )
		{
			SendMessage(player, "ESP esqueleto alternado no client.");
		}
		else
		{
			SendMessage(player, "Comando admin desconhecido: " + command);
		}
	}

	protected void HandleGodModeCommand(PlayerBase player)
	{
		PluginDeveloper developer = PluginDeveloper.GetInstance();
		if ( developer )
		{
			developer.ToggleGodMode(player);
			SendMessage(player, "Godmode alternado.");
		}
		else
		{
			SendMessage(player, "PluginDeveloper indisponivel para godmode.");
		}
	}

	protected void HandleTeleportToPlayerCommand(PlayerBase player, string target_uid)
	{
		PlayerBase target = FindOnlinePlayerByUID(target_uid);
		if ( !target )
		{
			SendMessage(player, "Selecione um player online registrado para teleportar.");
			return;
		}

		DeveloperTeleport.SetPlayerPosition(player, target.GetPosition());
		SendMessage(player, "Teleportado ate " + GetDisplayNameByUID(target_uid) + ".");
	}

	protected void HandleTeleportCityCommand(PlayerBase player, string city_name)
	{
		city_name = city_name.Trim();
		if ( city_name.Length() == 0 )
		{
			SendMessage(player, "Selecione uma cidade para teleportar.");
			return;
		}

		PluginConfigDebugProfileFixed fixed_profile = (PluginConfigDebugProfileFixed)GetPlugin(PluginConfigDebugProfileFixed);
		if ( !fixed_profile )
		{
			SendMessage(player, "Lista fixa de cidades indisponivel.");
			return;
		}

		vector pos = fixed_profile.GetPositionByName(city_name);
		if ( pos[0] == 0 && pos[2] == 0 )
		{
			SendMessage(player, "Cidade nao encontrada: " + city_name);
			return;
		}

		DeveloperTeleport.SetPlayerPosition(player, pos);
		SendMessage(player, "Teleportado para " + city_name + ".");
	}

	protected void HandlePullPlayerCommand(PlayerBase player, string target_uid)
	{
		PlayerBase target = FindOnlinePlayerByUID(target_uid);
		if ( !target )
		{
			SendMessage(player, "Selecione um player online registrado para puxar.");
			return;
		}

		DeveloperTeleport.SetPlayerPosition(target, player.GetPosition());
		SendMessage(player, "Player puxado: " + GetDisplayNameByUID(target_uid) + ".");
		SendMessage(target, "Voce foi puxado por um admin.");
	}

	protected void HandleSpawnItemsCommand(PlayerBase player)
	{
		PluginDeveloper developer = PluginDeveloper.GetInstance();
		if ( !developer )
		{
			SendMessage(player, "PluginDeveloper indisponivel para spawn.");
			return;
		}

		developer.SpawnItem(player, "EN5C_Rice", SPAWNTYPE_INVENTORY, 0, -1);
		developer.SpawnItem(player, "EN5C_WaterBottle", SPAWNTYPE_INVENTORY, 0, -1);
		developer.SpawnItem(player, "EN5C_BandageDressing", SPAWNTYPE_INVENTORY, 0, -1);
		SendMessage(player, "Kit admin spawnado no inventario.");
	}

	protected void HandleSpawnItemCommand(PlayerBase player, string item_name)
	{
		item_name = item_name.Trim();
		if ( item_name.Length() == 0 )
		{
			SendMessage(player, "Selecione um item para spawnar.");
			return;
		}

		PluginDeveloper developer = PluginDeveloper.GetInstance();
		if ( !developer )
		{
			SendMessage(player, "PluginDeveloper indisponivel para spawn.");
			return;
		}

		developer.SpawnItem(player, item_name, SPAWNTYPE_INVENTORY, 0, -1);
		SendMessage(player, "Item spawnado no inventario: " + item_name);
	}

	protected void HandleKickPlayerCommand(PlayerBase player, string target_uid)
	{
		PlayerBase target = FindOnlinePlayerByUID(target_uid);
		if ( !target )
		{
			SendMessage(player, "Selecione um player online registrado para kick.");
			return;
		}

		PlayerIdentity identity = FindOnlineIdentityByUID(target_uid);
		if ( !identity )
		{
			SendMessage(player, "Identity do player nao encontrada para kick.");
			return;
		}

		SendMessage(target, "Voce foi desconectado por um admin.");
		GetGame().SetPlayerDisconnected(identity);
		SendMessage(player, "Kick executado: " + GetDisplayNameByUID(target_uid) + ".");
	}

	protected void HandleBanUIDCommand(PlayerBase player, string target_uid)
	{
		if ( target_uid.Length() == 0 )
		{
			SendMessage(player, "Selecione um player para banir por UID.");
			return;
		}

		if ( !IsUIDBanned(target_uid) )
		{
			AppendBanUID(target_uid);
		}

		SendMessage(player, "UID registrado no ban list: " + target_uid);

		PlayerBase target = FindOnlinePlayerByUID(target_uid);
		if ( target )
		{
			SendMessage(target, "Seu UID foi registrado no ban list do servidor.");
		}

		PlayerIdentity identity = FindOnlineIdentityByUID(target_uid);
		if ( identity )
		{
			GetGame().SetPlayerDisconnected(identity);
			SendMessage(player, "Player banido e desconectado: " + GetDisplayNameByUID(target_uid) + ".");
		}
	}

	protected void HandlePlayerListResponse(ParamsReadContext ctx)
	{
		Param1<string> response = new Param1<string>("");
		if ( !ctx.Read(response) )
		{
			return;
		}

		m_LastPlayerListBlob = response.param1;
		m_PlayerListVersion++;
	}

	protected void HandleClientMessage(ParamsReadContext ctx)
	{
		Param1<string> message = new Param1<string>("");
		if ( ctx.Read(message) )
		{
			Print("[Admin] " + message.param1);
			PlayerBase player = GetGame().GetPlayer();
			if ( player )
			{
				player.MessageStatus(message.param1);
			}
		}
	}

	protected void LoadPermissions()
	{
		m_AdminUIDs.Clear();
		m_AdminNames.Clear();
		m_ModeratorUIDs.Clear();
		m_ModeratorNames.Clear();

		EnsurePermissionFile();

		FileHandle file = OpenFile(ADMIN_CONFIG_FILE, FileMode.READ);
		if ( file == 0 )
		{
			Print("[Admin] Nao foi possivel ler " + ADMIN_CONFIG_FILE);
			return;
		}

		string line;
		int chars = FGets(file, line);
		while ( chars >= 0 )
		{
			ParsePermissionLine(line);
			chars = FGets(file, line);
		}

		CloseFile(file);
		Print("[Admin] Permissoes carregadas: admins=" + m_AdminUIDs.Count().ToString() + " mods=" + m_ModeratorUIDs.Count().ToString());
	}

	protected void EnsurePermissionFile()
	{
		if ( !FileExist(ADMIN_CONFIG_DIR) )
		{
			MakeDirectory(ADMIN_CONFIG_DIR);
		}

		if ( FileExist(ADMIN_CONFIG_FILE) )
		{
			return;
		}

		FileHandle file = OpenFile(ADMIN_CONFIG_FILE, FileMode.WRITE);
		if ( file == 0 )
		{
			Print("[Admin] Nao foi possivel criar " + ADMIN_CONFIG_FILE);
			return;
		}

		FPrintln(file, "# DayZ 0.62 Admin permissions");
		FPrintln(file, "# formato: role|uid|nome");
		FPrintln(file, "# exemplo: admin|76561198000000000|Arthur");
		FPrintln(file, "# exemplo: moderator|76561198000000001|Moderador");
		CloseFile(file);
	}

	protected void ParsePermissionLine(string line)
	{
		line = line.Trim();
		if ( line.Length() == 0 )
		{
			return;
		}

		if ( line.Substring(0, 1) == "#" )
		{
			return;
		}

		TStringArray parts = new TStringArray;
		line.Split("|", parts);

		if ( parts.Count() < 2 )
		{
			return;
		}

		string role = parts.Get(0).Trim();
		role.ToLower();
		string uid = parts.Get(1).Trim();
		string name = "";

		if ( parts.Count() > 2 )
		{
			name = parts.Get(2).Trim();
		}

		if ( uid.Length() == 0 )
		{
			return;
		}

		if ( role == "admin" )
		{
			m_AdminUIDs.Insert(uid);
			m_AdminNames.Insert(SanitizeToken(name));
		}
		else if ( role == "moderator" || role == "moderador" || role == "mod" )
		{
			m_ModeratorUIDs.Insert(uid);
			m_ModeratorNames.Insert(SanitizeToken(name));
		}
	}

	protected string BuildPlayerListBlob()
	{
		RefreshOnlinePlayersFromGame();

		string output = "";

		for ( int i = 0; i < m_OnlinePlayers.Count(); i++ )
		{
			PlayerBase player = m_OnlinePlayers.Get(i);
			if ( !player )
			{
				continue;
			}

			string uid = m_OnlineUIDs.Get(i);
			string name = m_OnlineNames.Get(i);
			int role = GetRoleByUID(uid);
			vector pos = player.GetPosition();

			if ( output.Length() > 0 )
			{
				output = output + "\n";
			}

			output = output + name + "|" + uid + "|" + GetRoleName(role) + "|" + Math.Round(pos[0]).ToString() + " " + Math.Round(pos[1]).ToString() + " " + Math.Round(pos[2]).ToString();
		}

		return output;
	}

	protected string BuildIdentityPlayerListBlob()
	{
		string output = "";
		array<PlayerIdentity> identities = new array<PlayerIdentity>;
		GetGame().GetPlayerIndentities(identities);

		for ( int i = 0; i < identities.Count(); i++ )
		{
			PlayerIdentity identity = identities.Get(i);
			if ( !identity )
			{
				continue;
			}

			string uid = identity.GetId();
			string name = SanitizeToken(identity.GetName());
			if ( name.Length() == 0 )
			{
				name = "Player";
			}

			if ( output.Length() > 0 )
			{
				output = output + "\n";
			}

			output = output + name + "|" + uid + "|" + GetRoleName(GetRoleByUID(uid)) + "|online";
		}

		return output;
	}

	protected void RebuildOnlinePlayersFromGame()
	{
		if ( !GetGame().IsServer() )
		{
			return;
		}

		m_OnlinePlayers.Clear();
		m_OnlineIdentities.Clear();
		m_OnlineUIDs.Clear();
		m_OnlineNames.Clear();
		RefreshOnlinePlayersFromGame();
	}

	protected void RefreshOnlinePlayersFromGame()
	{
		if ( !GetGame().IsServer() )
		{
			return;
		}

		array<Man> players = new array<Man>;
		array<PlayerIdentity> identities = new array<PlayerIdentity>;
		GetGame().GetPlayers(players);
		GetGame().GetPlayerIndentities(identities);

		if ( players.Count() == 0 || identities.Count() == 0 )
		{
			return;
		}

		int count = players.Count();
		if ( identities.Count() < count )
		{
			count = identities.Count();
		}

		for ( int i = 0; i < count; i++ )
		{
			PlayerBase player = (PlayerBase)players.Get(i);
			PlayerIdentity identity = identities.Get(i);
			if ( player && identity )
			{
				RegisterOnlinePlayer(identity, player, identity.GetId(), SanitizeToken(identity.GetName()), false);
			}
		}

		Print("[Admin] Cache de jogadores online: " + m_OnlinePlayers.Count().ToString());
	}

	protected void RegisterOnlinePlayer(PlayerIdentity identity, PlayerBase player, string uid, string name, bool force_update)
	{
		if ( !identity || !player || uid.Length() == 0 )
		{
			return;
		}

		if ( force_update )
		{
			RemoveOnlinePlayer(player, uid);
		}
		else
		{
			int player_index = FindOnlineIndexByPlayer(player);
			int uid_index = FindOnlineIndexByUID(uid);
			if ( player_index >= 0 && uid_index >= 0 )
			{
				return;
			}

			if ( player_index >= 0 || uid_index >= 0 )
			{
				RemoveOnlinePlayer(player, uid);
			}
		}

		if ( name.Length() == 0 )
		{
			name = "Player";
		}

		m_OnlinePlayers.Insert(player);
		m_OnlineIdentities.Insert(identity);
		m_OnlineUIDs.Insert(uid);
		m_OnlineNames.Insert(name);
	}

	protected PlayerBase FindOnlinePlayerByUID(string uid)
	{
		RefreshOnlinePlayersFromGame();

		for ( int i = 0; i < m_OnlinePlayers.Count(); i++ )
		{
			if ( m_OnlineUIDs.Get(i) == uid )
			{
				return m_OnlinePlayers.Get(i);
			}
		}

		return NULL;
	}

	protected PlayerIdentity FindOnlineIdentityByUID(string uid)
	{
		RefreshOnlinePlayersFromGame();

		for ( int i = 0; i < m_OnlineUIDs.Count(); i++ )
		{
			if ( m_OnlineUIDs.Get(i) == uid )
			{
				return m_OnlineIdentities.Get(i);
			}
		}

		return NULL;
	}

	protected string GetDisplayNameByUID(string uid)
	{
		string name = GetNameByUID(uid);
		if ( name.Length() > 0 )
		{
			return name;
		}

		return uid;
	}

	protected void AppendBanUID(string uid)
	{
		if ( !FileExist(ADMIN_CONFIG_DIR) )
		{
			MakeDirectory(ADMIN_CONFIG_DIR);
		}

		FileHandle file = OpenFile(ADMIN_BAN_FILE, FileMode.APPEND);
		if ( file == 0 )
		{
			Print("[Admin] Nao foi possivel escrever " + ADMIN_BAN_FILE);
			return;
		}

		FPrintln(file, uid);
		CloseFile(file);
	}

	protected bool IsUIDBanned(string uid)
	{
		if ( uid.Length() == 0 || !FileExist(ADMIN_BAN_FILE) )
		{
			return false;
		}

		FileHandle file = OpenFile(ADMIN_BAN_FILE, FileMode.READ);
		if ( file == 0 )
		{
			return false;
		}

		string line;
		int chars = FGets(file, line);
		while ( chars >= 0 )
		{
			line = line.Trim();
			if ( line == uid )
			{
				CloseFile(file);
				return true;
			}

			chars = FGets(file, line);
		}

		CloseFile(file);
		return false;
	}

	protected int CountBlobLines(string blob)
	{
		if ( blob.Length() == 0 )
		{
			return 0;
		}

		TStringArray lines = new TStringArray;
		blob.Split("\n", lines);
		return lines.Count();
	}

	protected string GetUIDByPlayer(PlayerBase player)
	{
		for ( int i = 0; i < m_OnlinePlayers.Count(); i++ )
		{
			if ( m_OnlinePlayers.Get(i) == player )
			{
				return m_OnlineUIDs.Get(i);
			}
		}

		return "";
	}

	protected void ResolvePlayerIdentity(PlayerBase player, out string uid, out string name)
	{
		uid = GetUIDByPlayer(player);
		name = GetNameByUID(uid);

		if ( uid.Length() == 0 || name.Length() == 0 )
		{
			RefreshOnlinePlayersFromGame();
			uid = GetUIDByPlayer(player);
			name = GetNameByUID(uid);
		}

		if ( uid.Length() == 0 )
		{
			TryResolveSingleIdentity(uid, name);
		}

		if ( name.Length() == 0 )
		{
			name = "Player";
		}
	}

	protected int FindOnlineIndexByPlayer(PlayerBase player)
	{
		for ( int i = 0; i < m_OnlinePlayers.Count(); i++ )
		{
			if ( m_OnlinePlayers.Get(i) == player )
			{
				return i;
			}
		}

		return -1;
	}

	protected int FindOnlineIndexByUID(string uid)
	{
		for ( int i = 0; i < m_OnlineUIDs.Count(); i++ )
		{
			if ( m_OnlineUIDs.Get(i) == uid )
			{
				return i;
			}
		}

		return -1;
	}

	protected string GetNameByUID(string uid)
	{
		for ( int i = 0; i < m_OnlineUIDs.Count(); i++ )
		{
			if ( m_OnlineUIDs.Get(i) == uid )
			{
				return m_OnlineNames.Get(i);
			}
		}

		return "";
	}

	protected void TryResolveSingleIdentity(out string uid, out string name)
	{
		array<PlayerIdentity> identities = new array<PlayerIdentity>;
		GetGame().GetPlayerIndentities(identities);

		if ( identities.Count() == 1 )
		{
			PlayerIdentity identity = identities.Get(0);
			if ( identity )
			{
				uid = identity.GetId();
				name = SanitizeToken(identity.GetName());
				Print("[Admin] UID resolvido por fallback de identity unica: " + uid);
			}
		}
	}

	protected int GetRoleByUID(string uid)
	{
		for ( int i = 0; i < m_AdminUIDs.Count(); i++ )
		{
			if ( m_AdminUIDs.Get(i) == uid )
			{
				return ADMIN_ROLE_ADMIN;
			}
		}

		for ( int j = 0; j < m_ModeratorUIDs.Count(); j++ )
		{
			if ( m_ModeratorUIDs.Get(j) == uid )
			{
				return ADMIN_ROLE_MODERATOR;
			}
		}

		return ADMIN_ROLE_NONE;
	}

	protected string GetRoleName(int role)
	{
		if ( role == ADMIN_ROLE_ADMIN )
		{
			return "Admin";
		}

		if ( role == ADMIN_ROLE_MODERATOR )
		{
			return "Moderador";
		}

		return "Player";
	}

	protected void RemoveOnlinePlayer(PlayerBase player, string uid)
	{
		for ( int i = m_OnlinePlayers.Count() - 1; i >= 0; i-- )
		{
			if ( m_OnlinePlayers.Get(i) == player || ( uid.Length() > 0 && m_OnlineUIDs.Get(i) == uid ) )
			{
				m_OnlinePlayers.Remove(i);
				m_OnlineIdentities.Remove(i);
				m_OnlineUIDs.Remove(i);
				m_OnlineNames.Remove(i);
			}
		}
	}

	protected string SanitizeToken(string value)
	{
		value.Replace("|", " ");
		value.Replace("\n", " ");
		return value;
	}

	protected void SendMessage(PlayerBase player, string text)
	{
		if ( player )
		{
			Param1<string> message = new Param1<string>(text);
			player.RPCSingleParam(ADMIN_RPC_MESSAGE, message, player);
		}
	}
}
