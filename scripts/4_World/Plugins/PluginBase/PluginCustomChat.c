class PluginCustomChat extends PluginBase
{
	protected const string CHAT_CONFIG_DIR = "!ConfigServer";
	protected const string CHAT_CLAN_FILE = "!ConfigServer/chat_clans.txt";
	protected const string CHAT_PARTY_CLAN_FILE = "$profile:party_clans.txt";
	protected const string CHAT_ADMIN_FILE = "!ConfigServer/admin_permissions.txt";
	protected const float LOCAL_CHAT_DISTANCE = 60.0;

	protected const int CUSTOM_CHAT_GLOBAL = 0;
	protected const int CUSTOM_CHAT_LOCAL = 1;
	protected const int CUSTOM_CHAT_CLAN = 2;
	protected const int CUSTOM_CHAT_ADMIN = 3;

	protected const int CHAT_ROLE_NONE = 0;
	protected const int CHAT_ROLE_MODERATOR = 1;
	protected const int CHAT_ROLE_ADMIN = 2;

	protected autoptr TStringArray m_AdminUIDs;
	protected autoptr TStringArray m_ModeratorUIDs;
	protected autoptr TStringArray m_ClanUIDs;
	protected autoptr TStringArray m_ClanTags;
	protected autoptr TStringArray m_ClanNames;
	protected autoptr array<PlayerBase> m_OnlinePlayers;
	protected autoptr TStringArray m_OnlineUIDs;
	protected autoptr TStringArray m_OnlineNames;

	static PluginCustomChat GetInstance()
	{
		return GetPlugin(PluginCustomChat);
	}

	void PluginCustomChat()
	{
		m_AdminUIDs = new TStringArray;
		m_ModeratorUIDs = new TStringArray;
		m_ClanUIDs = new TStringArray;
		m_ClanTags = new TStringArray;
		m_ClanNames = new TStringArray;
		m_OnlinePlayers = new array<PlayerBase>;
		m_OnlineUIDs = new TStringArray;
		m_OnlineNames = new TStringArray;
	}

	void OnInit()
	{
		super.OnInit();

		if ( GetGame().IsServer() )
		{
			LoadRoleTags();
			LoadClanTags();
		}
	}

	void RegisterPlayer(PlayerIdentity identity, PlayerBase player)
	{
		if ( !GetGame().IsServer() || !identity || !player )
		{
			return;
		}

		string uid = identity.GetId();
		string name = identity.GetName();
		RemoveOnlinePlayer(player, uid);

		m_OnlinePlayers.Insert(player);
		m_OnlineUIDs.Insert(uid);
		m_OnlineNames.Insert(SanitizeToken(name));
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

	void SendLocalMessage(string text)
	{
		PlayerBase player = GetGame().GetPlayer();
		if ( !player )
		{
			GetGame().ChatPlayer(GetGame().ChatGetChannel(), text);
			return;
		}

		string payload_text = text.Trim();
		if ( payload_text.Length() == 0 )
		{
			return;
		}

		int channel = ResolveOutgoingChannel(payload_text);
		if ( channel < 0 )
		{
			return;
		}

		Param2<int, string> payload = new Param2<int, string>(channel, payload_text);
		player.RPCSingleParam(CHAT_RPC_SEND, payload);
	}

	string GetLocalChannelName()
	{
		return GetChannelDisplayName(MapNativeChannel(GetGame().ChatGetChannel()));
	}

	void OnRPC(PlayerBase player, int rpc_type, ParamsReadContext ctx)
	{
		if ( GetGame().IsServer() )
		{
			if ( rpc_type == CHAT_RPC_SEND )
			{
				HandleServerMessage(player, ctx);
			}
		}
		else if ( GetGame().IsClient() )
		{
			if ( rpc_type == CHAT_RPC_DELIVER )
			{
				HandleClientMessage(ctx);
			}
		}
	}

	protected void HandleServerMessage(PlayerBase sender, ParamsReadContext ctx)
	{
		if ( !sender )
		{
			return;
		}

		Param2<int, string> request = new Param2<int, string>(CUSTOM_CHAT_GLOBAL, "");
		if ( !ctx.Read(request) )
		{
			return;
		}

		string text = SanitizeMessage(request.param2);
		if ( text.Length() == 0 )
		{
			return;
		}

		LoadRoleTags();
		LoadClanTags();
		RebuildOnlinePlayersFromGameIfNeeded();

		string uid = GetUIDByPlayer(sender);
		string name = GetNameByUID(uid);
		if ( uid.Length() == 0 )
		{
			TryResolveSingleIdentity(uid, name);
		}

		if ( name.Length() == 0 )
		{
			name = "Player";
		}

		int channel = ClampChannel(request.param1);
		string sender_name = BuildSenderName(uid, name);

		if ( channel == CUSTOM_CHAT_GLOBAL )
		{
			BroadcastGlobal(sender, sender_name, text);
		}
		else if ( channel == CUSTOM_CHAT_LOCAL )
		{
			BroadcastLocal(sender, sender_name, text);
		}
		else if ( channel == CUSTOM_CHAT_CLAN )
		{
			BroadcastClan(sender, uid, sender_name, text);
		}
		else if ( channel == CUSTOM_CHAT_ADMIN )
		{
			BroadcastAdminContact(sender, sender_name, text);
		}
	}

	protected void HandleClientMessage(ParamsReadContext ctx)
	{
		ChatMessageEventParams message = new ChatMessageEventParams(CCGlobal, "", "", "");
		if ( !ctx.Read(message) )
		{
			return;
		}

		Mission mission = GetGame().GetMission();
		if ( mission )
		{
			mission.OnEvent(ChatMessageEventTypeID, message);
		}
		else
		{
			if ( message.param2.Length() > 0 )
			{
				GetGame().Chat(message.param2 + ": " + message.param3, "");
			}
			else
			{
				GetGame().Chat(message.param3, "");
			}
		}
	}

	protected void BroadcastGlobal(PlayerBase sender, string sender_name, string text)
	{
		RebuildOnlinePlayersFromGameIfNeeded();

		bool sent = false;
		for ( int i = 0; i < m_OnlinePlayers.Count(); i++ )
		{
			SendChatMessage(m_OnlinePlayers.Get(i), CUSTOM_CHAT_GLOBAL, sender_name, text);
			sent = true;
		}

		if ( !sent )
		{
			SendChatMessage(sender, CUSTOM_CHAT_GLOBAL, sender_name, text);
		}
	}

	protected void BroadcastLocal(PlayerBase sender, string sender_name, string text)
	{
		RebuildOnlinePlayersFromGameIfNeeded();

		vector sender_pos = sender.GetPosition();
		bool sent = false;
		for ( int i = 0; i < m_OnlinePlayers.Count(); i++ )
		{
			PlayerBase target = m_OnlinePlayers.Get(i);
			if ( target && vector.Distance(sender_pos, target.GetPosition()) <= LOCAL_CHAT_DISTANCE )
			{
				SendChatMessage(target, CUSTOM_CHAT_LOCAL, sender_name, text);
				sent = true;
			}
		}

		if ( !sent )
		{
			SendChatMessage(sender, CUSTOM_CHAT_LOCAL, sender_name, text);
		}
	}

	protected void BroadcastClan(PlayerBase sender, string sender_uid, string sender_name, string text)
	{
		string clan_tag = GetClanTag(sender_uid);
		if ( clan_tag.Length() == 0 )
		{
			SendSystemMessage(sender, "Voce nao possui clan configurado em " + CHAT_CLAN_FILE + ".");
			return;
		}

		RebuildOnlinePlayersFromGameIfNeeded();

		bool sent = false;
		for ( int i = 0; i < m_OnlinePlayers.Count(); i++ )
		{
			string target_uid = m_OnlineUIDs.Get(i);
			if ( GetClanTag(target_uid) == clan_tag )
			{
				SendChatMessage(m_OnlinePlayers.Get(i), CUSTOM_CHAT_CLAN, sender_name, text);
				sent = true;
			}
		}

		if ( !sent )
		{
			SendChatMessage(sender, CUSTOM_CHAT_CLAN, sender_name, text);
		}
	}

	protected void BroadcastAdminContact(PlayerBase sender, string sender_name, string text)
	{
		RebuildOnlinePlayersFromGameIfNeeded();

		bool found_staff = false;
		bool sent_sender = false;
		for ( int i = 0; i < m_OnlinePlayers.Count(); i++ )
		{
			PlayerBase target = m_OnlinePlayers.Get(i);
			string uid = m_OnlineUIDs.Get(i);
			int role = GetRoleByUID(uid);

			if ( target == sender )
			{
				SendChatMessage(target, CUSTOM_CHAT_ADMIN, sender_name, text);
				sent_sender = true;
			}
			else if ( role > CHAT_ROLE_NONE )
			{
				SendChatMessage(target, CUSTOM_CHAT_ADMIN, sender_name, text);
				found_staff = true;
			}
		}

		if ( !sent_sender )
		{
			SendChatMessage(sender, CUSTOM_CHAT_ADMIN, sender_name, text);
		}

		if ( !found_staff )
		{
			SendSystemMessage(sender, "Nenhum admin/moderador online recebeu o chamado.");
		}
	}

	protected void SendChatMessage(PlayerBase target, int custom_channel, string sender_name, string text)
	{
		if ( !target )
		{
			return;
		}

		string display_name = sender_name;
		if ( custom_channel == CUSTOM_CHAT_GLOBAL )
		{
			display_name = "[GLOBAL] " + sender_name;
		}

		ChatMessageEventParams message = new ChatMessageEventParams(ToEngineChannel(custom_channel), display_name, text, "");
		target.RPCSingleParam(CHAT_RPC_DELIVER, message, target);
	}

	protected void SendSystemMessage(PlayerBase target, string text)
	{
		if ( !target )
		{
			return;
		}

		ChatMessageEventParams message = new ChatMessageEventParams(CCSystem, "", text, "");
		target.RPCSingleParam(CHAT_RPC_DELIVER, message, target);
	}

	protected int ResolveOutgoingChannel(out string text)
	{
		if ( StartsWith(text, "/g ") )
		{
			text = text.Substring(3, text.Length() - 3).Trim();
			return CUSTOM_CHAT_GLOBAL;
		}

		if ( StartsWith(text, "/global ") )
		{
			text = text.Substring(8, text.Length() - 8).Trim();
			return CUSTOM_CHAT_GLOBAL;
		}

		if ( StartsWith(text, "/l ") )
		{
			text = text.Substring(3, text.Length() - 3).Trim();
			return CUSTOM_CHAT_LOCAL;
		}

		if ( StartsWith(text, "/local ") )
		{
			text = text.Substring(7, text.Length() - 7).Trim();
			return CUSTOM_CHAT_LOCAL;
		}

		if ( StartsWith(text, "/c ") )
		{
			text = text.Substring(3, text.Length() - 3).Trim();
			return CUSTOM_CHAT_CLAN;
		}

		if ( StartsWith(text, "/clan ") )
		{
			text = text.Substring(6, text.Length() - 6).Trim();
			return CUSTOM_CHAT_CLAN;
		}

		if ( StartsWith(text, "/a ") )
		{
			text = text.Substring(3, text.Length() - 3).Trim();
			return CUSTOM_CHAT_ADMIN;
		}

		if ( StartsWith(text, "/admin ") )
		{
			text = text.Substring(7, text.Length() - 7).Trim();
			return CUSTOM_CHAT_ADMIN;
		}

		return MapNativeChannel(GetGame().ChatGetChannel());
	}

	protected int MapNativeChannel(ChatChannel channel)
	{
		if ( channel == CCDirect )
		{
			return CUSTOM_CHAT_LOCAL;
		}

		if ( channel == CCCustom1 )
		{
			return CUSTOM_CHAT_CLAN;
		}

		if ( channel == CCCustom2 )
		{
			return CUSTOM_CHAT_ADMIN;
		}

		return CUSTOM_CHAT_GLOBAL;
	}

	protected int ClampChannel(int channel)
	{
		if ( channel < CUSTOM_CHAT_GLOBAL || channel > CUSTOM_CHAT_ADMIN )
		{
			return CUSTOM_CHAT_GLOBAL;
		}

		return channel;
	}

	protected int ToEngineChannel(int channel)
	{
		if ( channel == CUSTOM_CHAT_LOCAL )
		{
			return CCDirect;
		}

		if ( channel == CUSTOM_CHAT_CLAN )
		{
			return CCCustom1;
		}

		if ( channel == CUSTOM_CHAT_ADMIN )
		{
			return CCCustom2;
		}

		return CCGlobal;
	}

	protected string GetChannelDisplayName(int channel)
	{
		if ( channel == CUSTOM_CHAT_LOCAL )
		{
			return "Local";
		}

		if ( channel == CUSTOM_CHAT_CLAN )
		{
			return "Clan";
		}

		if ( channel == CUSTOM_CHAT_ADMIN )
		{
			return "Admin";
		}

		return "Global";
	}

	protected string BuildSenderName(string uid, string fallback_name)
	{
		string output = "";
		int role = GetRoleByUID(uid);

		if ( role == CHAT_ROLE_ADMIN )
		{
			output = output + "[Admin] ";
		}
		else if ( role == CHAT_ROLE_MODERATOR )
		{
			output = output + "[Moderador] ";
		}

		string clan_tag = GetClanTag(uid);
		if ( clan_tag.Length() > 0 )
		{
			output = output + "[" + clan_tag + "] ";
		}

		return output + fallback_name;
	}

	protected void LoadRoleTags()
	{
		m_AdminUIDs.Clear();
		m_ModeratorUIDs.Clear();

		if ( !FileExist(CHAT_ADMIN_FILE) )
		{
			return;
		}

		FileHandle file = OpenFile(CHAT_ADMIN_FILE, FileMode.READ);
		if ( file == 0 )
		{
			return;
		}

		string line;
		int chars = FGets(file, line);
		while ( chars >= 0 )
		{
			ParseRoleLine(line);
			chars = FGets(file, line);
		}

		CloseFile(file);
	}

	protected void ParseRoleLine(string line)
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
		if ( uid.Length() == 0 )
		{
			return;
		}

		if ( role == "admin" )
		{
			m_AdminUIDs.Insert(uid);
		}
		else if ( role == "moderator" || role == "moderador" || role == "mod" )
		{
			m_ModeratorUIDs.Insert(uid);
		}
	}

	protected void LoadClanTags()
	{
		m_ClanUIDs.Clear();
		m_ClanTags.Clear();
		m_ClanNames.Clear();

		EnsureClanFile();

		FileHandle file = OpenFile(CHAT_CLAN_FILE, FileMode.READ);
		if ( file == 0 )
		{
			return;
		}

		string line;
		int chars = FGets(file, line);
		while ( chars >= 0 )
		{
			ParseClanLine(line);
			chars = FGets(file, line);
		}

		CloseFile(file);

		LoadPartyClanTags();
	}

	protected void LoadPartyClanTags()
	{
		if ( !FileExist(CHAT_PARTY_CLAN_FILE) )
		{
			return;
		}

		FileHandle file = OpenFile(CHAT_PARTY_CLAN_FILE, FileMode.READ);
		if ( file == 0 )
		{
			return;
		}

		string line;
		int chars = FGets(file, line);
		while ( chars >= 0 )
		{
			ParsePartyClanLine(line);
			chars = FGets(file, line);
		}

		CloseFile(file);
	}

	protected void EnsureClanFile()
	{
		if ( !FileExist(CHAT_CONFIG_DIR) )
		{
			MakeDirectory(CHAT_CONFIG_DIR);
		}

		if ( FileExist(CHAT_CLAN_FILE) )
		{
			return;
		}

		FileHandle file = OpenFile(CHAT_CLAN_FILE, FileMode.WRITE);
		if ( file == 0 )
		{
			return;
		}

		FPrintln(file, "# DayZ 0.62 Custom Chat clans");
		FPrintln(file, "# formato: TAG|uid|nome");
		FPrintln(file, "# exemplo: ALFA|76561198000000000|Arthur");
		CloseFile(file);
	}

	protected void ParseClanLine(string line)
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

		string tag = SanitizeTag(parts.Get(0).Trim());
		string uid = parts.Get(1).Trim();
		string name = "";
		if ( parts.Count() > 2 )
		{
			name = SanitizeToken(parts.Get(2).Trim());
		}

		if ( tag.Length() == 0 || uid.Length() == 0 )
		{
			return;
		}

		m_ClanTags.Insert(tag);
		m_ClanUIDs.Insert(uid);
		m_ClanNames.Insert(name);
	}

	protected void ParsePartyClanLine(string line)
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
		if ( parts.Count() < 5 )
		{
			return;
		}

		string tag = SanitizeTag(parts.Get(0).Trim());
		string uid = parts.Get(3).Trim();
		string name = SanitizeToken(parts.Get(4).Trim());

		if ( tag.Length() == 0 || uid.Length() == 0 )
		{
			return;
		}

		m_ClanTags.Insert(tag);
		m_ClanUIDs.Insert(uid);
		m_ClanNames.Insert(name);
	}

	protected string GetClanTag(string uid)
	{
		for ( int i = 0; i < m_ClanUIDs.Count(); i++ )
		{
			if ( m_ClanUIDs.Get(i) == uid )
			{
				return m_ClanTags.Get(i);
			}
		}

		return "";
	}

	protected int GetRoleByUID(string uid)
	{
		for ( int i = 0; i < m_AdminUIDs.Count(); i++ )
		{
			if ( m_AdminUIDs.Get(i) == uid )
			{
				return CHAT_ROLE_ADMIN;
			}
		}

		for ( int j = 0; j < m_ModeratorUIDs.Count(); j++ )
		{
			if ( m_ModeratorUIDs.Get(j) == uid )
			{
				return CHAT_ROLE_MODERATOR;
			}
		}

		return CHAT_ROLE_NONE;
	}

	protected void RebuildOnlinePlayersFromGameIfNeeded()
	{
		if ( m_OnlinePlayers.Count() > 0 )
		{
			return;
		}

		RebuildOnlinePlayersFromGame();
	}

	protected void RebuildOnlinePlayersFromGame()
	{
		if ( !GetGame().IsServer() )
		{
			return;
		}

		m_OnlinePlayers.Clear();
		m_OnlineUIDs.Clear();
		m_OnlineNames.Clear();

		array<Man> players = new array<Man>;
		array<PlayerIdentity> identities = new array<PlayerIdentity>;
		GetGame().GetPlayers(players);
		GetGame().GetPlayerIndentities(identities);

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
				m_OnlinePlayers.Insert(player);
				m_OnlineUIDs.Insert(identity.GetId());
				m_OnlineNames.Insert(SanitizeToken(identity.GetName()));
			}
		}
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
			}
		}
	}

	protected void RemoveOnlinePlayer(PlayerBase player, string uid)
	{
		for ( int i = m_OnlinePlayers.Count() - 1; i >= 0; i-- )
		{
			if ( m_OnlinePlayers.Get(i) == player || ( uid.Length() > 0 && m_OnlineUIDs.Get(i) == uid ) )
			{
				m_OnlinePlayers.Remove(i);
				m_OnlineUIDs.Remove(i);
				m_OnlineNames.Remove(i);
			}
		}
	}

	protected bool StartsWith(string text, string prefix)
	{
		if ( text.Length() < prefix.Length() )
		{
			return false;
		}

		return text.Substring(0, prefix.Length()) == prefix;
	}

	protected string SanitizeMessage(string value)
	{
		value = value.Trim();
		value.Replace("\n", " ");
		value.Replace("|", " ");

		if ( value.Length() > 180 )
		{
			value = value.Substring(0, 180);
		}

		return value;
	}

	protected string SanitizeToken(string value)
	{
		value.Replace("|", " ");
		value.Replace("\n", " ");
		return value;
	}

	protected string SanitizeTag(string value)
	{
		value.Replace("|", "");
		value.Replace("\n", "");
		value.Replace("[", "");
		value.Replace("]", "");

		if ( value.Length() > 12 )
		{
			value = value.Substring(0, 12);
		}

		return value;
	}
}
