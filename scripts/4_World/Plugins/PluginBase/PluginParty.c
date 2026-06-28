class PluginParty extends PluginBase
{
	protected const string PARTY_CLAN_FILE = "$profile:party_clans.txt";
	protected const string PARTY_CMD_STATE = "state";
	protected const string PARTY_CMD_CREATE = "create";
	protected const string PARTY_CMD_INVITE = "invite";
	protected const string PARTY_CMD_ACCEPT = "accept";
	protected const string PARTY_CMD_DENY = "deny";
	protected const string PARTY_CMD_LEAVE = "leave";
	protected const string PARTY_CMD_KICK = "kick";
	protected const string PARTY_CMD_ROLE = "role";
	protected const string PARTY_RANK_TRIAL = "trial";
	protected const string PARTY_RANK_MEMBER = "member";
	protected const string PARTY_RANK_VICE = "vice";
	protected const string PARTY_RANK_LEADER = "leader";
	protected const string PARTY_RANK_OWNER = "owner";

	protected autoptr array<PlayerBase> m_OnlinePlayers;
	protected autoptr TStringArray m_OnlineUIDs;
	protected autoptr TStringArray m_OnlineNames;

	protected autoptr TStringArray m_ClanTags;
	protected autoptr TStringArray m_ClanNames;
	protected autoptr TStringArray m_ClanOwnerUIDs;
	protected autoptr TStringArray m_MemberUIDs;
	protected autoptr TStringArray m_MemberNames;
	protected autoptr TStringArray m_MemberRanks;

	protected autoptr TStringArray m_InviteTargetUIDs;
	protected autoptr TStringArray m_InviteSenderUIDs;
	protected autoptr TStringArray m_InviteClanTags;
	protected autoptr TStringArray m_InviteClanNames;
	protected autoptr TStringArray m_InviteSenderNames;

	protected string m_LocalInfo;
	protected string m_OnlineBlob;
	protected string m_MembersBlob;
	protected string m_InvitesBlob;
	protected string m_ClanListBlob;
	protected int m_StateVersion;

	static PluginParty GetInstance()
	{
		return GetPlugin(PluginParty);
	}

	void PluginParty()
	{
		m_OnlinePlayers = new array<PlayerBase>;
		m_OnlineUIDs = new TStringArray;
		m_OnlineNames = new TStringArray;

		m_ClanTags = new TStringArray;
		m_ClanNames = new TStringArray;
		m_ClanOwnerUIDs = new TStringArray;
		m_MemberUIDs = new TStringArray;
		m_MemberNames = new TStringArray;
		m_MemberRanks = new TStringArray;

		m_InviteTargetUIDs = new TStringArray;
		m_InviteSenderUIDs = new TStringArray;
		m_InviteClanTags = new TStringArray;
		m_InviteClanNames = new TStringArray;
		m_InviteSenderNames = new TStringArray;

		m_LocalInfo = "";
		m_OnlineBlob = "";
		m_MembersBlob = "";
		m_InvitesBlob = "";
		m_ClanListBlob = "";
		m_StateVersion = 0;
	}

	void OnInit()
	{
		super.OnInit();

		if ( GetGame().IsServer() )
		{
			LoadClans();
		}
	}

	void TogglePartyMenu()
	{
		UIManager manager = GetGame().GetUIManager();
		if ( !manager )
		{
			return;
		}

		if ( manager.IsMenuOpen(MENU_PARTY) )
		{
			manager.Back();
			return;
		}

		RequestState();
		manager.EnterScriptedMenu(MENU_PARTY, NULL);
	}

	void RegisterPlayer(PlayerIdentity identity, PlayerBase player)
	{
		if ( !GetGame().IsServer() || !identity || !player )
		{
			return;
		}

		string uid = identity.GetId();
		string name = SanitizeToken(identity.GetName());
		RegisterOnlinePlayer(player, uid, name, true);
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
			if ( rpc_type == PARTY_RPC_COMMAND )
			{
				HandleServerCommand(player, ctx);
			}
		}
		else if ( GetGame().IsClient() )
		{
			if ( rpc_type == PARTY_RPC_STATE )
			{
				HandleClientState(ctx);
			}
			else if ( rpc_type == PARTY_RPC_MESSAGE )
			{
				HandleClientMessage(ctx);
			}
		}
	}

	void RequestState()
	{
		SendClientCommand(PARTY_CMD_STATE, "");
	}

	void ClearClientState()
	{
		if ( !GetGame().IsClient() )
		{
			return;
		}

		m_LocalInfo = "";
		m_OnlineBlob = "";
		m_MembersBlob = "";
		m_InvitesBlob = "";
		m_ClanListBlob = "";
		m_StateVersion++;
	}

	void CreateClan(string tag, string clan_name)
	{
		tag = SanitizeTag(tag);
		clan_name = SanitizeToken(clan_name.Trim());

		if ( tag == "TAG" )
		{
			tag = "";
		}

		if ( clan_name == "Nome do clan" )
		{
			clan_name = "";
		}

		if ( clan_name.Length() == 0 )
		{
			clan_name = tag;
		}

		SendClientCommand(PARTY_CMD_CREATE, tag + "|" + clan_name);
	}

	void InvitePlayer(string uid)
	{
		if ( uid.Length() == 0 )
		{
			ShowLocalMessage("Selecione um player online.");
			return;
		}

		SendClientCommand(PARTY_CMD_INVITE, uid);
	}

	void AcceptInvite(string tag)
	{
		if ( tag.Length() == 0 )
		{
			ShowLocalMessage("Selecione um convite.");
			return;
		}

		SendClientCommand(PARTY_CMD_ACCEPT, tag);
	}

	void DenyInvite(string tag)
	{
		if ( tag.Length() == 0 )
		{
			ShowLocalMessage("Selecione um convite.");
			return;
		}

		SendClientCommand(PARTY_CMD_DENY, tag);
	}

	void LeaveClan()
	{
		SendClientCommand(PARTY_CMD_LEAVE, "");
	}

	void KickMember(string uid)
	{
		if ( uid.Length() == 0 )
		{
			ShowLocalMessage("Selecione um membro do clan.");
			return;
		}

		SendClientCommand(PARTY_CMD_KICK, uid);
	}

	void SetMemberRank(string uid, string rank)
	{
		uid = uid.Trim();
		rank = SanitizeRank(rank);

		if ( uid.Length() == 0 )
		{
			ShowLocalMessage("Selecione um membro do clan.");
			return;
		}

		SendClientCommand(PARTY_CMD_ROLE, uid + "|" + rank);
	}

	string GetLocalInfo()
	{
		return m_LocalInfo;
	}

	string GetOnlineBlob()
	{
		return m_OnlineBlob;
	}

	string GetMembersBlob()
	{
		return m_MembersBlob;
	}

	string GetInvitesBlob()
	{
		return m_InvitesBlob;
	}

	string GetClanListBlob()
	{
		return m_ClanListBlob;
	}

	int GetStateVersion()
	{
		return m_StateVersion;
	}

	string GetLocalClanTag()
	{
		TStringArray parts = new TStringArray;
		m_LocalInfo.Split("|", parts);
		if ( parts.Count() > 2 )
		{
			return parts.Get(2);
		}

		return "";
	}

	string GetLocalClanName()
	{
		TStringArray parts = new TStringArray;
		m_LocalInfo.Split("|", parts);
		if ( parts.Count() > 3 )
		{
			return parts.Get(3);
		}

		return "";
	}

	bool IsLocalOwner()
	{
		TStringArray parts = new TStringArray;
		m_LocalInfo.Split("|", parts);
		if ( parts.Count() > 4 && parts.Get(4) == "1" )
		{
			return true;
		}

		return false;
	}

	string GetLocalRank()
	{
		TStringArray parts = new TStringArray;
		m_LocalInfo.Split("|", parts);
		if ( parts.Count() > 5 )
		{
			return parts.Get(5);
		}

		if ( IsLocalOwner() )
		{
			return PARTY_RANK_OWNER;
		}

		return PARTY_RANK_MEMBER;
	}

	bool CanLocalInvite()
	{
		TStringArray parts = new TStringArray;
		m_LocalInfo.Split("|", parts);
		if ( parts.Count() > 6 && parts.Get(6) == "1" )
		{
			return true;
		}

		return IsLocalOwner();
	}

	protected void SendClientCommand(string command, string data)
	{
		PlayerBase player = GetGame().GetPlayer();
		if ( !player )
		{
			return;
		}

		Param2<string, string> request = new Param2<string, string>(command, data);
		player.RPCSingleParam(PARTY_RPC_COMMAND, request);
	}

	protected void HandleServerCommand(PlayerBase player, ParamsReadContext ctx)
	{
		if ( !player )
		{
			return;
		}

		Param2<string, string> request = new Param2<string, string>("", "");
		if ( !ctx.Read(request) )
		{
			SendMessage(player, "Comando de party invalido.");
			return;
		}

		LoadClans();
		RefreshOnlinePlayersFromGame();

		if ( request.param1 == PARTY_CMD_STATE )
		{
			SendState(player);
		}
		else if ( request.param1 == PARTY_CMD_CREATE )
		{
			HandleCreateClan(player, request.param2);
		}
		else if ( request.param1 == PARTY_CMD_INVITE )
		{
			HandleInvite(player, request.param2);
		}
		else if ( request.param1 == PARTY_CMD_ACCEPT )
		{
			HandleAccept(player, request.param2);
		}
		else if ( request.param1 == PARTY_CMD_DENY )
		{
			HandleDeny(player, request.param2);
		}
		else if ( request.param1 == PARTY_CMD_LEAVE )
		{
			HandleLeave(player);
		}
		else if ( request.param1 == PARTY_CMD_KICK )
		{
			HandleKick(player, request.param2);
		}
		else if ( request.param1 == PARTY_CMD_ROLE )
		{
			HandleSetRank(player, request.param2);
		}
		else
		{
			SendMessage(player, "Comando de party desconhecido.");
		}
	}

	protected void HandleCreateClan(PlayerBase player, string data)
	{
		string uid = "";
		string name = "";
		ResolvePlayerIdentity(player, uid, name);

		if ( uid.Length() == 0 )
		{
			SendMessage(player, "Nao foi possivel identificar seu UID.");
			Print("[Party] Falha ao criar clan: UID vazio.");
			return;
		}

		if ( GetClanTagByUID(uid).Length() > 0 )
		{
			SendMessage(player, "Voce ja esta em um clan.");
			SendState(player);
			return;
		}

		TStringArray parts = new TStringArray;
		data.Split("|", parts);
		if ( parts.Count() < 1 )
		{
			SendMessage(player, "Tag invalida.");
			return;
		}

		string tag = SanitizeTag(parts.Get(0).Trim());
		string clan_name = tag;
		if ( parts.Count() > 1 )
		{
			clan_name = SanitizeToken(parts.Get(1).Trim());
		}

		if ( tag == "TAG" )
		{
			tag = "";
		}

		if ( clan_name == "Nome do clan" )
		{
			clan_name = "";
		}

		if ( tag.Length() == 0 )
		{
			tag = BuildDefaultClanTagFromIdentity(uid, name);
		}

		if ( tag.Length() == 0 )
		{
			SendMessage(player, "Tag invalida.");
			return;
		}

		if ( clan_name.Length() == 0 )
		{
			clan_name = tag;
		}

		if ( ClanExists(tag) )
		{
			SendMessage(player, "Essa tag de clan ja existe.");
			return;
		}

		AddClanMember(tag, clan_name, uid, uid, name, PARTY_RANK_OWNER);
		SaveClans();
		Print("[Party] Clan criado: tag=" + tag + " nome=" + clan_name + " dono=" + uid);
		SendMessage(player, "Clan criado: [" + tag + "] " + clan_name);
		SendState(player);
	}

	protected void HandleInvite(PlayerBase player, string target_uid)
	{
		string sender_uid = "";
		string sender_name = "";
		ResolvePlayerIdentity(player, sender_uid, sender_name);

		string clan_tag = GetClanTagByUID(sender_uid);
		if ( clan_tag.Length() == 0 )
		{
			SendMessage(player, "Crie ou entre em um clan antes de convidar.");
			SendState(player);
			return;
		}

		if ( !CanInvite(sender_uid) )
		{
			SendMessage(player, "Seu cargo nao permite convidar.");
			return;
		}

		target_uid = target_uid.Trim();
		if ( target_uid == sender_uid )
		{
			SendMessage(player, "Voce nao pode convidar a si mesmo.");
			return;
		}

		PlayerBase target = FindOnlinePlayerByUID(target_uid);
		if ( !target )
		{
			SendMessage(player, "Player selecionado nao esta online.");
			SendState(player);
			return;
		}

		if ( GetClanTagByUID(target_uid).Length() > 0 )
		{
			SendMessage(player, "Esse player ja esta em um clan.");
			return;
		}

		string clan_name = GetClanNameByTag(clan_tag);
		string target_name = GetNameByUID(target_uid);
		RemoveInvite(target_uid, clan_tag);

		m_InviteTargetUIDs.Insert(target_uid);
		m_InviteSenderUIDs.Insert(sender_uid);
		m_InviteClanTags.Insert(clan_tag);
		m_InviteClanNames.Insert(clan_name);
		m_InviteSenderNames.Insert(sender_name);

		SendMessage(player, "Convite enviado para " + target_name + ".");
		SendMessage(target, "Voce recebeu convite do clan [" + clan_tag + "] " + clan_name + ".");
		SendState(player);
		SendState(target);
	}

	protected void HandleAccept(PlayerBase player, string clan_tag)
	{
		string uid = "";
		string name = "";
		ResolvePlayerIdentity(player, uid, name);

		if ( GetClanTagByUID(uid).Length() > 0 )
		{
			SendMessage(player, "Voce ja esta em um clan.");
			SendState(player);
			return;
		}

		int invite_index = FindInvite(uid, clan_tag);
		if ( invite_index < 0 )
		{
			SendMessage(player, "Convite nao encontrado.");
			SendState(player);
			return;
		}

		string tag = m_InviteClanTags.Get(invite_index);
		string clan_name = m_InviteClanNames.Get(invite_index);
		string owner_uid = GetOwnerUIDByTag(tag);

		AddClanMember(tag, clan_name, owner_uid, uid, name, PARTY_RANK_TRIAL);
		RemoveInvitesForTarget(uid);
		SaveClans();

		SendMessage(player, "Voce entrou no clan [" + tag + "].");
		SendState(player);
		RefreshClanMembers(tag);
	}

	protected void HandleDeny(PlayerBase player, string clan_tag)
	{
		string uid = "";
		string name = "";
		ResolvePlayerIdentity(player, uid, name);

		int invite_index = FindInvite(uid, clan_tag);
		if ( invite_index >= 0 )
		{
			string sender_uid = m_InviteSenderUIDs.Get(invite_index);
			string tag = m_InviteClanTags.Get(invite_index);
			RemoveInviteAt(invite_index);

			PlayerBase sender = FindOnlinePlayerByUID(sender_uid);
			if ( sender )
			{
				SendMessage(sender, name + " recusou o convite do clan [" + tag + "].");
				SendState(sender);
			}
		}

		SendMessage(player, "Convite recusado.");
		SendState(player);
	}

	protected void HandleLeave(PlayerBase player)
	{
		string uid = "";
		string name = "";
		ResolvePlayerIdentity(player, uid, name);

		string clan_tag = GetClanTagByUID(uid);
		if ( clan_tag.Length() == 0 )
		{
			SendMessage(player, "Voce nao esta em um clan.");
			SendState(player);
			return;
		}

		bool was_owner = IsOwner(uid);
		if ( was_owner )
		{
			RemoveClan(clan_tag);
			RemoveInvitesForClan(clan_tag);
			SaveClans();
			SendMessage(player, "Clan [" + clan_tag + "] foi removido.");
			SendState(player);
			return;
		}

		RemoveMember(uid);
		SaveClans();
		SendMessage(player, "Voce saiu do clan [" + clan_tag + "].");
		SendState(player);
		RefreshClanMembers(clan_tag);
	}

	protected void HandleSetRank(PlayerBase player, string data)
	{
		string uid = "";
		string name = "";
		ResolvePlayerIdentity(player, uid, name);

		string clan_tag = GetClanTagByUID(uid);
		if ( clan_tag.Length() == 0 || !IsOwner(uid) )
		{
			SendMessage(player, "Apenas o dono do clan pode alterar patentes.");
			return;
		}

		TStringArray parts = new TStringArray;
		data.Split("|", parts);
		if ( parts.Count() < 2 )
		{
			SendMessage(player, "Patente invalida.");
			return;
		}

		string target_uid = parts.Get(0).Trim();
		string rank = SanitizeRank(parts.Get(1).Trim());

		if ( target_uid == uid )
		{
			SendMessage(player, "O dono permanece como Dono.");
			return;
		}

		if ( rank == PARTY_RANK_OWNER )
		{
			SendMessage(player, "A patente Dono nao pode ser atribuida por aqui.");
			return;
		}

		if ( GetClanTagByUID(target_uid) != clan_tag )
		{
			SendMessage(player, "Esse player nao esta no seu clan.");
			return;
		}

		SetRankByUID(target_uid, rank);
		SaveClans();
		SendMessage(player, "Patente atualizada para " + GetMemberNameByUID(target_uid) + ": " + GetRankLabel(rank) + ".");

		PlayerBase target = FindOnlinePlayerByUID(target_uid);
		if ( target )
		{
			SendMessage(target, "Sua patente no clan agora e " + GetRankLabel(rank) + ".");
		}

		RefreshClanMembers(clan_tag);
	}

	protected void HandleKick(PlayerBase player, string target_uid)
	{
		string uid = "";
		string name = "";
		ResolvePlayerIdentity(player, uid, name);

		string clan_tag = GetClanTagByUID(uid);
		if ( clan_tag.Length() == 0 || !IsOwner(uid) )
		{
			SendMessage(player, "Apenas o dono do clan pode expulsar.");
			return;
		}

		target_uid = target_uid.Trim();
		if ( target_uid == uid )
		{
			SendMessage(player, "Use Sair para remover o clan.");
			return;
		}

		if ( GetClanTagByUID(target_uid) != clan_tag )
		{
			SendMessage(player, "Esse player nao esta no seu clan.");
			return;
		}

		string target_name = GetMemberNameByUID(target_uid);
		RemoveMember(target_uid);
		SaveClans();

		SendMessage(player, target_name + " foi removido do clan.");

		PlayerBase target = FindOnlinePlayerByUID(target_uid);
		if ( target )
		{
			SendMessage(target, "Voce foi removido do clan [" + clan_tag + "].");
			SendState(target);
		}

		RefreshClanMembers(clan_tag);
	}

	protected void SendState(PlayerBase player)
	{
		if ( !player )
		{
			return;
		}

		LoadClans();
		RefreshOnlinePlayersFromGame();

		string uid = "";
		string name = "";
		ResolvePlayerIdentity(player, uid, name);

		string clan_tag = GetClanTagByUID(uid);
		string clan_name = GetClanNameByTag(clan_tag);
		string owner_flag = "0";
		if ( IsOwner(uid) )
		{
			owner_flag = "1";
		}
		string local_rank = GetRankByUID(uid);
		string invite_flag = "0";
		if ( CanInvite(uid) )
		{
			invite_flag = "1";
		}

		string local_info = uid + "|" + name + "|" + clan_tag + "|" + clan_name + "|" + owner_flag + "|" + local_rank + "|" + invite_flag;
		string online_blob = BuildOnlineBlob(uid);
		string members_blob = BuildMembersBlob(clan_tag);
		string invites_blob = BuildInvitesBlob(uid);
		string clan_list_blob = BuildClanListBlob();

		if ( clan_tag.Length() > 0 && members_blob.Length() == 0 )
		{
			Print("[Party] Estado de clan inconsistente para uid=" + uid + " tag=" + clan_tag + ". Enviando estado sem clan.");
			clan_tag = "";
			clan_name = "";
			owner_flag = "0";
			local_info = uid + "|" + name + "|||0||0";
		}

		Param5<string, string, string, string, string> state = new Param5<string, string, string, string, string>(local_info, online_blob, members_blob, invites_blob, clan_list_blob);
		player.RPCSingleParam(PARTY_RPC_STATE, state, player);
	}

	protected void SendMessage(PlayerBase target, string text)
	{
		if ( !target )
		{
			return;
		}

		Param1<string> message = new Param1<string>(text);
		target.RPCSingleParam(PARTY_RPC_MESSAGE, message, target);
	}

	protected void HandleClientState(ParamsReadContext ctx)
	{
		Param5<string, string, string, string, string> state = new Param5<string, string, string, string, string>("", "", "", "", "");
		if ( !ctx.Read(state) )
		{
			return;
		}

		m_LocalInfo = state.param1;
		m_OnlineBlob = state.param2;
		m_MembersBlob = state.param3;
		m_InvitesBlob = state.param4;
		m_ClanListBlob = state.param5;
		m_StateVersion++;
	}

	protected void HandleClientMessage(ParamsReadContext ctx)
	{
		Param1<string> message = new Param1<string>("");
		if ( !ctx.Read(message) )
		{
			return;
		}

		ShowLocalMessage(message.param1);
	}

	protected void ShowLocalMessage(string text)
	{
		PlayerBase player = GetGame().GetPlayer();
		if ( player )
		{
			player.MessageStatus(text);
		}
		else
		{
			Print("[Party] " + text);
		}
	}

	protected string BuildOnlineBlob(string local_uid)
	{
		string output = "";
		bool first = true;
		for ( int i = 0; i < m_OnlineUIDs.Count(); i++ )
		{
			string uid = m_OnlineUIDs.Get(i);
			if ( uid == local_uid )
			{
				continue;
			}

			if ( !first )
			{
				output = output + "\n";
			}

			output = output + m_OnlineNames.Get(i) + "|" + uid + "|" + GetClanTagByUID(uid);
			first = false;
		}

		return output;
	}

	protected string BuildMembersBlob(string clan_tag)
	{
		if ( clan_tag.Length() == 0 )
		{
			return "";
		}

		string output = "";
		bool first = true;
		for ( int i = 0; i < m_MemberUIDs.Count(); i++ )
		{
			if ( m_ClanTags.Get(i) != clan_tag )
			{
				continue;
			}

			string uid = m_MemberUIDs.Get(i);
			string rank = GetRankByUID(uid);
			string owner_flag = "0";
			if ( rank == PARTY_RANK_OWNER || m_ClanOwnerUIDs.Get(i) == uid )
			{
				owner_flag = "1";
			}

			float health01 = 0.0;
			vector pos = "0 0 0";
			PlayerBase member = FindOnlinePlayerByUID(uid);
			if ( member )
			{
				pos = member.GetPosition();
				health01 = 1.0;
			}

			if ( !first )
			{
				output = output + "\n";
			}

			output = output + m_MemberNames.Get(i) + "|" + uid + "|" + owner_flag + "|" + rank + "|" + health01.ToString() + "|" + Math.Round(pos[0]).ToString() + "|" + Math.Round(pos[1]).ToString() + "|" + Math.Round(pos[2]).ToString();
			first = false;
		}

		return output;
	}

	protected string BuildClanListBlob()
	{
		string output = "";
		bool first = true;

		for ( int i = 0; i < m_ClanTags.Count(); i++ )
		{
			string tag = m_ClanTags.Get(i);
			if ( CountClanMembersBefore(tag, i) > 0 )
			{
				continue;
			}

			if ( !first )
			{
				output = output + "\n";
			}

			output = output + tag + "|" + m_ClanNames.Get(i) + "|" + CountClanMembers(tag).ToString();
			first = false;
		}

		return output;
	}

	protected string BuildInvitesBlob(string uid)
	{
		string output = "";
		bool first = true;
		for ( int i = 0; i < m_InviteTargetUIDs.Count(); i++ )
		{
			if ( m_InviteTargetUIDs.Get(i) != uid )
			{
				continue;
			}

			if ( !first )
			{
				output = output + "\n";
			}

			output = output + m_InviteClanTags.Get(i) + "|" + m_InviteClanNames.Get(i) + "|" + m_InviteSenderUIDs.Get(i) + "|" + m_InviteSenderNames.Get(i);
			first = false;
		}

		return output;
	}

	protected void RefreshClanMembers(string clan_tag)
	{
		for ( int i = 0; i < m_MemberUIDs.Count(); i++ )
		{
			if ( m_ClanTags.Get(i) == clan_tag )
			{
				PlayerBase member = FindOnlinePlayerByUID(m_MemberUIDs.Get(i));
				if ( member )
				{
					SendState(member);
				}
			}
		}
	}

	protected void LoadClans()
	{
		m_ClanTags.Clear();
		m_ClanNames.Clear();
		m_ClanOwnerUIDs.Clear();
		m_MemberUIDs.Clear();
		m_MemberNames.Clear();
		m_MemberRanks.Clear();

		EnsureClanFile();

		FileHandle file = OpenFile(PARTY_CLAN_FILE, FileMode.READ);
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
	}

	protected void SaveClans()
	{
		EnsureClanFile();

		FileHandle file = OpenFile(PARTY_CLAN_FILE, FileMode.WRITE);
		if ( file == 0 )
		{
			return;
		}

		FPrintln(file, "# DayZ 0.62 party/clan data");
		FPrintln(file, "# formato: TAG|ClanName|OwnerUID|MemberUID|MemberName|Rank");

		for ( int i = 0; i < m_MemberUIDs.Count(); i++ )
		{
			FPrintln(file, m_ClanTags.Get(i) + "|" + m_ClanNames.Get(i) + "|" + m_ClanOwnerUIDs.Get(i) + "|" + m_MemberUIDs.Get(i) + "|" + m_MemberNames.Get(i) + "|" + GetRankByIndex(i));
		}

		CloseFile(file);
	}

	protected void EnsureClanFile()
	{
		if ( FileExist(PARTY_CLAN_FILE) )
		{
			return;
		}

		FileHandle file = OpenFile(PARTY_CLAN_FILE, FileMode.WRITE);
		if ( file == 0 )
		{
			Print("[Party] Falha ao criar arquivo de clans em " + PARTY_CLAN_FILE);
			return;
		}

		FPrintln(file, "# DayZ 0.62 party/clan data");
		FPrintln(file, "# formato: TAG|ClanName|OwnerUID|MemberUID|MemberName|Rank");
		CloseFile(file);
	}

	protected string BuildDefaultClanTagFromIdentity(string uid, string name)
	{
		string source = SanitizeTag(name);
		if ( source.Length() >= 3 )
		{
			return source.Substring(0, 3);
		}

		source = SanitizeTag(uid);
		if ( source.Length() >= 3 )
		{
			return source.Substring(source.Length() - 3, 3);
		}

		return "CLAN";
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
		if ( parts.Count() < 5 )
		{
			return;
		}

		string owner_uid = parts.Get(2).Trim();
		string member_uid = parts.Get(3).Trim();
		string rank = PARTY_RANK_MEMBER;
		if ( parts.Count() > 5 )
		{
			rank = SanitizeRank(parts.Get(5).Trim());
		}
		else if ( owner_uid == member_uid )
		{
			rank = PARTY_RANK_OWNER;
		}

		AddClanMember(SanitizeTag(parts.Get(0).Trim()), SanitizeToken(parts.Get(1).Trim()), owner_uid, member_uid, SanitizeToken(parts.Get(4).Trim()), rank);
	}

	protected void AddClanMember(string tag, string clan_name, string owner_uid, string member_uid, string member_name, string rank)
	{
		if ( tag.Length() == 0 || owner_uid.Length() == 0 || member_uid.Length() == 0 )
		{
			return;
		}

		rank = SanitizeRank(rank);
		if ( owner_uid == member_uid )
		{
			rank = PARTY_RANK_OWNER;
		}

		m_ClanTags.Insert(tag);
		m_ClanNames.Insert(clan_name);
		m_ClanOwnerUIDs.Insert(owner_uid);
		m_MemberUIDs.Insert(member_uid);
		m_MemberNames.Insert(member_name);
		m_MemberRanks.Insert(rank);
	}

	protected bool ClanExists(string tag)
	{
		for ( int i = 0; i < m_ClanTags.Count(); i++ )
		{
			if ( m_ClanTags.Get(i) == tag )
			{
				return true;
			}
		}

		return false;
	}

	protected bool IsOwner(string uid)
	{
		for ( int i = 0; i < m_MemberUIDs.Count(); i++ )
		{
			if ( m_MemberUIDs.Get(i) == uid && m_ClanOwnerUIDs.Get(i) == uid )
			{
				return true;
			}
		}

		return false;
	}

	protected bool CanInvite(string uid)
	{
		string rank = GetRankByUID(uid);
		if ( rank == PARTY_RANK_OWNER || rank == PARTY_RANK_LEADER || rank == PARTY_RANK_VICE )
		{
			return true;
		}

		return false;
	}

	protected string GetClanTagByUID(string uid)
	{
		for ( int i = 0; i < m_MemberUIDs.Count(); i++ )
		{
			if ( m_MemberUIDs.Get(i) == uid )
			{
				return m_ClanTags.Get(i);
			}
		}

		return "";
	}

	protected string GetClanNameByTag(string tag)
	{
		for ( int i = 0; i < m_ClanTags.Count(); i++ )
		{
			if ( m_ClanTags.Get(i) == tag )
			{
				return m_ClanNames.Get(i);
			}
		}

		return "";
	}

	protected string GetOwnerUIDByTag(string tag)
	{
		for ( int i = 0; i < m_ClanTags.Count(); i++ )
		{
			if ( m_ClanTags.Get(i) == tag )
			{
				return m_ClanOwnerUIDs.Get(i);
			}
		}

		return "";
	}

	protected string GetMemberNameByUID(string uid)
	{
		for ( int i = 0; i < m_MemberUIDs.Count(); i++ )
		{
			if ( m_MemberUIDs.Get(i) == uid )
			{
				return m_MemberNames.Get(i);
			}
		}

		return uid;
	}

	protected string GetRankByUID(string uid)
	{
		for ( int i = 0; i < m_MemberUIDs.Count(); i++ )
		{
			if ( m_MemberUIDs.Get(i) == uid )
			{
				return GetRankByIndex(i);
			}
		}

		return "";
	}

	protected string GetRankByIndex(int index)
	{
		if ( index < 0 || index >= m_MemberRanks.Count() )
		{
			return PARTY_RANK_MEMBER;
		}

		return SanitizeRank(m_MemberRanks.Get(index));
	}

	protected void SetRankByUID(string uid, string rank)
	{
		rank = SanitizeRank(rank);
		for ( int i = 0; i < m_MemberUIDs.Count(); i++ )
		{
			if ( m_MemberUIDs.Get(i) == uid )
			{
				m_MemberRanks.Set(i, rank);
			}
		}
	}

	protected string GetRankLabel(string rank)
	{
		rank = SanitizeRank(rank);
		if ( rank == PARTY_RANK_OWNER ) return "Dono";
		if ( rank == PARTY_RANK_LEADER ) return "Lider";
		if ( rank == PARTY_RANK_VICE ) return "Vice lider";
		if ( rank == PARTY_RANK_MEMBER ) return "Membro";
		return "Trial";
	}

	protected string SanitizeRank(string rank)
	{
		rank = rank.Trim();
		if ( rank == PARTY_RANK_OWNER || rank == PARTY_RANK_LEADER || rank == PARTY_RANK_VICE || rank == PARTY_RANK_MEMBER || rank == PARTY_RANK_TRIAL )
		{
			return rank;
		}

		return PARTY_RANK_MEMBER;
	}

	protected int CountClanMembers(string tag)
	{
		int count = 0;
		for ( int i = 0; i < m_ClanTags.Count(); i++ )
		{
			if ( m_ClanTags.Get(i) == tag )
			{
				count++;
			}
		}

		return count;
	}

	protected int CountClanMembersBefore(string tag, int before_index)
	{
		int count = 0;
		for ( int i = 0; i < before_index; i++ )
		{
			if ( m_ClanTags.Get(i) == tag )
			{
				count++;
			}
		}

		return count;
	}

	protected void RemoveMember(string uid)
	{
		for ( int i = m_MemberUIDs.Count() - 1; i >= 0; i-- )
		{
			if ( m_MemberUIDs.Get(i) == uid )
			{
				m_ClanTags.Remove(i);
				m_ClanNames.Remove(i);
				m_ClanOwnerUIDs.Remove(i);
				m_MemberUIDs.Remove(i);
				m_MemberNames.Remove(i);
				m_MemberRanks.Remove(i);
			}
		}
	}

	protected void RemoveClan(string clan_tag)
	{
		for ( int i = m_ClanTags.Count() - 1; i >= 0; i-- )
		{
			if ( m_ClanTags.Get(i) == clan_tag )
			{
				m_ClanTags.Remove(i);
				m_ClanNames.Remove(i);
				m_ClanOwnerUIDs.Remove(i);
				m_MemberUIDs.Remove(i);
				m_MemberNames.Remove(i);
				m_MemberRanks.Remove(i);
			}
		}
	}

	protected int FindInvite(string target_uid, string clan_tag)
	{
		for ( int i = 0; i < m_InviteTargetUIDs.Count(); i++ )
		{
			if ( m_InviteTargetUIDs.Get(i) == target_uid && m_InviteClanTags.Get(i) == clan_tag )
			{
				return i;
			}
		}

		return -1;
	}

	protected void RemoveInvite(string target_uid, string clan_tag)
	{
		int index = FindInvite(target_uid, clan_tag);
		if ( index >= 0 )
		{
			RemoveInviteAt(index);
		}
	}

	protected void RemoveInviteAt(int index)
	{
		if ( index < 0 || index >= m_InviteTargetUIDs.Count() )
		{
			return;
		}

		m_InviteTargetUIDs.Remove(index);
		m_InviteSenderUIDs.Remove(index);
		m_InviteClanTags.Remove(index);
		m_InviteClanNames.Remove(index);
		m_InviteSenderNames.Remove(index);
	}

	protected void RemoveInvitesForTarget(string target_uid)
	{
		for ( int i = m_InviteTargetUIDs.Count() - 1; i >= 0; i-- )
		{
			if ( m_InviteTargetUIDs.Get(i) == target_uid )
			{
				RemoveInviteAt(i);
			}
		}
	}

	protected void RemoveInvitesForClan(string clan_tag)
	{
		for ( int i = m_InviteClanTags.Count() - 1; i >= 0; i-- )
		{
			if ( m_InviteClanTags.Get(i) == clan_tag )
			{
				RemoveInviteAt(i);
			}
		}
	}

	protected PlayerBase FindOnlinePlayerByUID(string uid)
	{
		for ( int i = 0; i < m_OnlineUIDs.Count(); i++ )
		{
			if ( m_OnlineUIDs.Get(i) == uid )
			{
				return m_OnlinePlayers.Get(i);
			}
		}

		return NULL;
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

		ResolveFallbackIdentity(uid, name);

		if ( name.Length() == 0 )
		{
			name = "Player";
		}
	}

	protected void ResolveFallbackIdentity(out string uid, out string name)
	{
		if ( uid.Length() > 0 && name.Length() > 0 )
		{
			return;
		}

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
				RegisterOnlinePlayer(player, identity.GetId(), SanitizeToken(identity.GetName()), false);
			}
		}
	}

	protected void RegisterOnlinePlayer(PlayerBase player, string uid, string name, bool force_update)
	{
		if ( !player || uid.Length() == 0 )
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
		m_OnlineUIDs.Insert(uid);
		m_OnlineNames.Insert(name);
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

	protected string SanitizeToken(string value)
	{
		value.Replace("|", " ");
		value.Replace("\n", " ");

		if ( value.Length() > 32 )
		{
			value = value.Substring(0, 32);
		}

		return value;
	}

	protected string SanitizeTag(string value)
	{
		value.Replace("|", "");
		value.Replace("\n", "");
		value.Replace("[", "");
		value.Replace("]", "");
		value.Replace(" ", "");

		if ( value.Length() > 10 )
		{
			value = value.Substring(0, 10);
		}

		return value;
	}
}
