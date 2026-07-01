class PartyMenu extends UIScriptedMenu
{
	protected Widget m_PartyRoot;
	protected Widget m_PartyShade;
	protected Widget m_PartyMainPanel;
	protected Widget m_PartyTopBar;
	protected Widget m_PartyCreatePanel;
	protected Widget m_PartyMembersPanel;
	protected Widget m_PartyClansPanel;
	protected Widget m_PartyOnlinePanel;
	protected Widget m_PartyInvitesPanel;
	protected TextWidget m_PartyStatus;
	protected EditBoxWidget m_TagEdit;
	protected EditBoxWidget m_NameEdit;
	protected ButtonWidget m_CreateButton;
	protected ButtonWidget m_LeaveButton;
	protected ButtonWidget m_RefreshButton;
	protected ButtonWidget m_CloseButton;
	protected ButtonWidget m_InviteButton;
	protected ButtonWidget m_AcceptButton;
	protected ButtonWidget m_DenyButton;
	protected ButtonWidget m_KickButton;
	protected ButtonWidget m_RankTrialButton;
	protected ButtonWidget m_RankMemberButton;
	protected ButtonWidget m_RankViceButton;
	protected ButtonWidget m_RankLeaderButton;
	protected TextListboxWidget m_MemberList;
	protected TextListboxWidget m_ClanList;
	protected TextListboxWidget m_OnlineList;
	protected TextListboxWidget m_InviteList;

	protected autoptr TStringArray m_MemberUIDs;
	protected autoptr TStringArray m_MemberNames;
	protected autoptr TStringArray m_MemberOwnerFlags;
	protected autoptr TStringArray m_MemberRanks;
	protected autoptr TStringArray m_OnlineUIDs;
	protected autoptr TStringArray m_OnlineNames;
	protected autoptr TStringArray m_InviteTags;
	protected autoptr TStringArray m_InviteNames;
	protected autoptr TStringArray m_ClanTags;

	protected int m_LastStateVersion;
	protected float m_RefreshAccum;

	void PartyMenu()
	{
		m_MemberUIDs = new TStringArray;
		m_MemberNames = new TStringArray;
		m_MemberOwnerFlags = new TStringArray;
		m_MemberRanks = new TStringArray;
		m_OnlineUIDs = new TStringArray;
		m_OnlineNames = new TStringArray;
		m_InviteTags = new TStringArray;
		m_InviteNames = new TStringArray;
		m_ClanTags = new TStringArray;
		m_LastStateVersion = -1;
		m_RefreshAccum = 0.0;
	}

	Widget Init()
	{
		layoutRoot = GetGame().GetWorkspace().CreateWidgets("gui/layouts/day_z_party_menu.layout");

		m_PartyRoot = layoutRoot.FindAnyWidget("PartyRoot");
		m_PartyShade = layoutRoot.FindAnyWidget("PartyShade");
		m_PartyMainPanel = layoutRoot.FindAnyWidget("PartyMainPanel");
		m_PartyTopBar = layoutRoot.FindAnyWidget("PartyTopBar");
		m_PartyCreatePanel = layoutRoot.FindAnyWidget("PartyCreatePanel");
		m_PartyMembersPanel = layoutRoot.FindAnyWidget("PartyMembersPanel");
		m_PartyClansPanel = layoutRoot.FindAnyWidget("PartyClansPanel");
		m_PartyOnlinePanel = layoutRoot.FindAnyWidget("PartyOnlinePanel");
		m_PartyInvitesPanel = layoutRoot.FindAnyWidget("PartyInvitesPanel");
		m_PartyStatus = (TextWidget)layoutRoot.FindAnyWidget("PartyStatus");
		m_TagEdit = (EditBoxWidget)layoutRoot.FindAnyWidget("PartyTagEdit");
		m_NameEdit = (EditBoxWidget)layoutRoot.FindAnyWidget("PartyNameEdit");
		m_CreateButton = (ButtonWidget)layoutRoot.FindAnyWidget("PartyCreate");
		m_LeaveButton = (ButtonWidget)layoutRoot.FindAnyWidget("PartyLeave");
		m_RefreshButton = (ButtonWidget)layoutRoot.FindAnyWidget("PartyRefresh");
		m_CloseButton = (ButtonWidget)layoutRoot.FindAnyWidget("PartyClose");
		m_InviteButton = (ButtonWidget)layoutRoot.FindAnyWidget("PartyInvite");
		m_AcceptButton = (ButtonWidget)layoutRoot.FindAnyWidget("PartyAcceptInvite");
		m_DenyButton = (ButtonWidget)layoutRoot.FindAnyWidget("PartyDenyInvite");
		m_KickButton = (ButtonWidget)layoutRoot.FindAnyWidget("PartyKickMember");
		m_RankTrialButton = (ButtonWidget)layoutRoot.FindAnyWidget("PartyRankTrial");
		m_RankMemberButton = (ButtonWidget)layoutRoot.FindAnyWidget("PartyRankMember");
		m_RankViceButton = (ButtonWidget)layoutRoot.FindAnyWidget("PartyRankVice");
		m_RankLeaderButton = (ButtonWidget)layoutRoot.FindAnyWidget("PartyRankLeader");
		m_MemberList = (TextListboxWidget)layoutRoot.FindAnyWidget("PartyMemberList");
		m_ClanList = (TextListboxWidget)layoutRoot.FindAnyWidget("PartyClanList");
		m_OnlineList = (TextListboxWidget)layoutRoot.FindAnyWidget("PartyOnlineList");
		m_InviteList = (TextListboxWidget)layoutRoot.FindAnyWidget("PartyInviteList");

		ApplyPartyStyle();

		PluginParty party = PluginParty.GetInstance();
		if ( party )
		{
			party.ClearClientState();
			party.RequestState();
		}

		Render();
		return layoutRoot;
	}

	void Update(float timeslice)
	{
		m_RefreshAccum = m_RefreshAccum + timeslice;
		if ( m_RefreshAccum >= 2.0 )
		{
			m_RefreshAccum = 0.0;
			PluginParty party_refresh = PluginParty.GetInstance();
			if ( party_refresh )
			{
				party_refresh.RequestState();
			}
		}

		PluginParty party = PluginParty.GetInstance();
		if ( party && party.GetStateVersion() != m_LastStateVersion )
		{
			Render();
		}
	}

	bool OnClick(Widget w, int x, int y, int button)
	{
		super.OnClick(w, x, y, button);

		PluginParty party = PluginParty.GetInstance();
		if ( !party )
		{
			return false;
		}

		if ( w == m_CloseButton )
		{
			GetGame().GetUIManager().Back();
			return true;
		}

		if ( w == m_RefreshButton )
		{
			party.RequestState();
			SetStatus("Atualizando party...");
			return true;
		}

		if ( w == m_CreateButton )
		{
			party.CreateClan(m_TagEdit.GetText(), m_NameEdit.GetText());
			SetStatus("Criando clan...");
			return true;
		}

		if ( w == m_LeaveButton )
		{
			party.LeaveClan();
			SetStatus("Saindo do clan...");
			return true;
		}

		if ( w == m_InviteButton )
		{
			string invite_uid = GetSelectedOnlineUID();
			if ( invite_uid.Length() == 0 )
			{
				SetStatus("Selecione um player online.");
				return true;
			}

			party.InvitePlayer(invite_uid);
			SetStatus("Enviando convite...");
			return true;
		}

		if ( w == m_AcceptButton )
		{
			string accept_tag = GetSelectedInviteTag();
			if ( accept_tag.Length() == 0 )
			{
				SetStatus("Selecione um convite.");
				return true;
			}

			party.AcceptInvite(accept_tag);
			SetStatus("Aceitando convite...");
			return true;
		}

		if ( w == m_DenyButton )
		{
			string deny_tag = GetSelectedInviteTag();
			if ( deny_tag.Length() == 0 )
			{
				SetStatus("Selecione um convite.");
				return true;
			}

			party.DenyInvite(deny_tag);
			SetStatus("Recusando convite...");
			return true;
		}

		if ( w == m_KickButton )
		{
			string kick_uid = GetSelectedMemberUID();
			if ( kick_uid.Length() == 0 )
			{
				SetStatus("Selecione um membro.");
				return true;
			}

			party.KickMember(kick_uid);
			SetStatus("Removendo membro...");
			return true;
		}

		if ( w == m_RankTrialButton || w == m_RankMemberButton || w == m_RankViceButton || w == m_RankLeaderButton )
		{
			string rank_uid = GetSelectedMemberUID();
			if ( rank_uid.Length() == 0 )
			{
				SetStatus("Selecione um membro.");
				return true;
			}

			string rank = "member";
			if ( w == m_RankTrialButton )
			{
				rank = "trial";
			}
			else if ( w == m_RankViceButton )
			{
				rank = "vice";
			}
			else if ( w == m_RankLeaderButton )
			{
				rank = "leader";
			}

			party.SetMemberRank(rank_uid, rank);
			SetStatus("Alterando patente...");
			return true;
		}

		return false;
	}

	bool OnKeyPress(Widget w, int x, int y, int key)
	{
		super.OnKeyPress(w, x, y, key);

		if ( key == KeyCode.KC_F8 || key == KeyCode.KC_ESCAPE )
		{
			GetGame().GetUIManager().Back();
			return true;
		}

		return false;
	}

	protected void Render()
	{
		PluginParty party = PluginParty.GetInstance();
		if ( !party )
		{
			return;
		}

		m_LastStateVersion = party.GetStateVersion();
		RenderMembers(party.GetMembersBlob());
		RenderClanList(party.GetClanListBlob());
		RenderOnline(party.GetOnlineBlob());
		RenderInvites(party.GetInvitesBlob());
		RenderStatus(party);
		UpdateButtons(party);
	}

	protected void RenderStatus(PluginParty party)
	{
		if ( !m_PartyStatus )
		{
			return;
		}

		string tag = party.GetLocalClanTag();
		string clan_name = party.GetLocalClanName();
		if ( tag.Length() == 0 )
		{
			m_PartyStatus.SetText("Sem clan. Crie um clan ou aceite um convite.");
			return;
		}

		if ( m_MemberUIDs.Count() == 0 )
		{
			m_PartyStatus.SetText("Estado de clan desatualizado. Atualize ou crie novamente.");
			return;
		}

		string role = "Membro";
		if ( party.IsLocalOwner() )
		{
			role = "Dono";
		}

		m_PartyStatus.SetText("Clan [" + tag + "] " + clan_name + " | " + role);
	}

	protected void RenderMembers(string blob)
	{
		m_MemberUIDs.Clear();
		m_MemberNames.Clear();
		m_MemberOwnerFlags.Clear();
		m_MemberRanks.Clear();

		if ( !m_MemberList )
		{
			return;
		}

		m_MemberList.ClearItems();
		if ( blob.Length() == 0 )
		{
			m_MemberList.AddItem("Sem membros carregados.", NULL, 0);
			return;
		}

		TStringArray lines = new TStringArray;
		blob.Split("\n", lines);

		// [2026-06-30] FEATURE: [3.5.6] Paginação — mostrar apenas 15 membros por vez
		int PAGE_SIZE = 15;
		int total_members = 0;
		
		// Contar membros válidos primeiro
		int count_idx = 0;
		for ( count_idx = 0; count_idx < lines.Count(); count_idx++ )
		{
			TStringArray parts = new TStringArray;
			lines.Get(count_idx).Split("|", parts);
			if ( parts.Count() >= 4 ) { total_members++; };
		}
		
		// Calcular paginação
		int total_pages = (total_members + PAGE_SIZE - 1) / PAGE_SIZE;
		if (total_pages < 1) { total_pages = 1; };
		
		// Usar m_CurrentPage (adicionar como membro da classe se não existir)
		int start_idx = 0;
		int end_idx = PAGE_SIZE;
		int displayed = 0;

		for ( int i = 0; i < lines.Count(); i++ )
		{
			TStringArray parts = new TStringArray;
			lines.Get(i).Split("|", parts);
			if ( parts.Count() < 4 )
			{
				continue;
			}

			string name = parts.Get(0);
			string uid = parts.Get(1);
			string owner = parts.Get(2);
			string rank = "member";
			float health = 1.0;
			// [2026-06-30] FIX: [1.2.5] Parsing robusto para 4-8+ campos
			if ( parts.Count() >= 8 )
			{
				rank = parts.Get(3);
				health = parts.Get(4).ToFloat();
			}
			else if ( parts.Count() >= 5 )
			{
				rank = parts.Get(3);
				health = parts.Get(4).ToFloat();
			}
			else
			{
				// Fallback: 4 campos — owner flag indica rank
				if ( owner == "1" ) { rank = "owner"; };
			}
			float health_percent = health * 100.0;
			string health_text = Math.Round(health_percent).ToString();

			// Paginação: só adicionar itens da página atual
			if (displayed >= start_idx && displayed < end_idx)
			{
				string display = name + "  " + GetRankLabel(rank) + "  " + health_text + "%";
				m_MemberList.AddItem(display, NULL, 0);
			};
			
			m_MemberNames.Insert(name);
			m_MemberUIDs.Insert(uid);
			m_MemberOwnerFlags.Insert(owner);
			m_MemberRanks.Insert(rank);
			displayed++;
		}
		
		// Mostrar info de paginação
		if (total_pages > 1)
		{
			m_MemberList.AddItem("--- Página 1/" + total_pages + " (" + total_members + " membros) ---", NULL, 0);
		};
	}

	protected void RenderClanList(string blob)
	{
		m_ClanTags.Clear();

		if ( !m_ClanList )
		{
			return;
		}

		m_ClanList.ClearItems();
		if ( blob.Length() == 0 )
		{
			m_ClanList.AddItem("Nenhum clan criado.", NULL, 0);
			return;
		}

		TStringArray lines = new TStringArray;
		blob.Split("\n", lines);

		for ( int i = 0; i < lines.Count(); i++ )
		{
			TStringArray parts = new TStringArray;
			lines.Get(i).Split("|", parts);
			if ( parts.Count() < 3 )
			{
				continue;
			}

			string tag = parts.Get(0);
			string clan_name = parts.Get(1);
			string count = parts.Get(2);
			m_ClanList.AddItem("[" + tag + "] " + clan_name + "  " + count + " players", NULL, 0);
			m_ClanTags.Insert(tag);
		}
	}

	protected void RenderOnline(string blob)
	{
		m_OnlineUIDs.Clear();
		m_OnlineNames.Clear();

		if ( !m_OnlineList )
		{
			return;
		}

		m_OnlineList.ClearItems();
		if ( blob.Length() == 0 )
		{
			m_OnlineList.AddItem("Nenhum outro player online.", NULL, 0);
			return;
		}

		TStringArray lines = new TStringArray;
		blob.Split("\n", lines);

		for ( int i = 0; i < lines.Count(); i++ )
		{
			TStringArray parts = new TStringArray;
			lines.Get(i).Split("|", parts);
			if ( parts.Count() < 2 )
			{
				continue;
			}

			string name = parts.Get(0);
			string uid = parts.Get(1);
			string tag = "";
			if ( parts.Count() > 2 )
			{
				tag = parts.Get(2);
			}

			string display = name;
			if ( tag.Length() > 0 )
			{
				display = display + "  [" + tag + "]";
			}

			m_OnlineList.AddItem(display, NULL, 0);
			m_OnlineNames.Insert(name);
			m_OnlineUIDs.Insert(uid);
		}
	}

	protected void RenderInvites(string blob)
	{
		m_InviteTags.Clear();
		m_InviteNames.Clear();

		if ( !m_InviteList )
		{
			return;
		}

		m_InviteList.ClearItems();
		if ( blob.Length() == 0 )
		{
			m_InviteList.AddItem("Sem convites pendentes.", NULL, 0);
			return;
		}

		TStringArray lines = new TStringArray;
		blob.Split("\n", lines);

		for ( int i = 0; i < lines.Count(); i++ )
		{
			TStringArray parts = new TStringArray;
			lines.Get(i).Split("|", parts);
			if ( parts.Count() < 4 )
			{
				continue;
			}

			string tag = parts.Get(0);
			string clan_name = parts.Get(1);
			string sender_name = parts.Get(3);
			m_InviteList.AddItem("[" + tag + "] " + clan_name + " por " + sender_name, NULL, 0);
			m_InviteTags.Insert(tag);
			m_InviteNames.Insert(clan_name);
		}
	}

	protected void UpdateButtons(PluginParty party)
	{
		bool has_clan = party.GetLocalClanTag().Length() > 0 && m_MemberUIDs.Count() > 0;
		bool is_owner = party.IsLocalOwner();
		bool can_invite = party.CanLocalInvite();
		bool has_online_target = m_OnlineUIDs.Count() > 0;
		bool has_invite = m_InviteTags.Count() > 0;
		bool has_kick_target = m_MemberUIDs.Count() > 1;
		bool has_rank_target = has_kick_target;

		SetButtonVisual(m_CreateButton, !has_clan, 0xCC2E7D32);
		SetButtonVisual(m_LeaveButton, has_clan, 0xCC8E2C24);
		SetButtonVisual(m_InviteButton, has_clan && can_invite && has_online_target, 0xCC1565C0);
		SetButtonVisual(m_KickButton, has_clan && is_owner && has_kick_target, 0xCC8E2C24);
		SetButtonVisual(m_RankTrialButton, has_clan && is_owner && has_rank_target, 0xCC455A64);
		SetButtonVisual(m_RankMemberButton, has_clan && is_owner && has_rank_target, 0xCC455A64);
		SetButtonVisual(m_RankViceButton, has_clan && is_owner && has_rank_target, 0xCC1565C0);
		SetButtonVisual(m_RankLeaderButton, has_clan && is_owner && has_rank_target, 0xCC1565C0);
		SetButtonVisual(m_AcceptButton, !has_clan && has_invite, 0xCC2E7D32);
		SetButtonVisual(m_DenyButton, has_invite, 0xCC8E2C24);
		SetButtonVisual(m_RefreshButton, true, 0xCC455A64);
		SetButtonVisual(m_CloseButton, true, 0xCC5D4037);
	}

	protected void SetButtonVisual(ButtonWidget button, bool enabled, int active_color)
	{
		if ( button )
		{
			button.Enable(enabled);
			if ( enabled )
			{
				button.SetColor(active_color);
			}
			else
			{
				button.SetColor(0xAA30343A);
			}
		}
	}

	protected string GetSelectedOnlineUID()
	{
		if ( !m_OnlineList || m_OnlineUIDs.Count() == 0 )
		{
			return "";
		}

		int row = m_OnlineList.GetSelectedRow();
		if ( row < 0 && m_OnlineUIDs.Count() == 1 )
		{
			row = 0;
		}

		if ( row < 0 || row >= m_OnlineUIDs.Count() )
		{
			return "";
		}

		return m_OnlineUIDs.Get(row);
	}

	protected string GetSelectedInviteTag()
	{
		if ( !m_InviteList || m_InviteTags.Count() == 0 )
		{
			return "";
		}

		int row = m_InviteList.GetSelectedRow();
		if ( row < 0 && m_InviteTags.Count() == 1 )
		{
			row = 0;
		}

		if ( row < 0 || row >= m_InviteTags.Count() )
		{
			return "";
		}

		return m_InviteTags.Get(row);
	}

	protected string GetSelectedMemberUID()
	{
		if ( !m_MemberList || m_MemberUIDs.Count() == 0 )
		{
			return "";
		}

		int row = m_MemberList.GetSelectedRow();
		if ( row < 0 && m_MemberUIDs.Count() == 1 )
		{
			row = 0;
		}

		if ( row < 0 || row >= m_MemberUIDs.Count() )
		{
			return "";
		}

		return m_MemberUIDs.Get(row);
	}

	protected void SetStatus(string text)
	{
		if ( m_PartyStatus )
		{
			m_PartyStatus.SetText(text);
		}

		PlayerBase player = GetGame().GetPlayer();
		if ( player )
		{
			player.MessageStatus(text);
		}
	}

	protected void ApplyPartyStyle()
	{
		if ( layoutRoot )
		{
			layoutRoot.SetSort(1650);
			layoutRoot.SetColor(0x99000000);
			layoutRoot.Show(true);
		}

		ForcePanel(m_PartyRoot, 1650, 0x99000000);
		ForcePanel(m_PartyShade, 1651, 0xAA000000);
		ForcePanel(m_PartyMainPanel, 1652, 0xE00A121E);
		ForcePanel(m_PartyTopBar, 1653, 0xEE061326);
		ForcePanel(m_PartyCreatePanel, 1654, 0xCC071B2D);
		ForcePanel(m_PartyMembersPanel, 1654, 0xCC071B2D);
		ForcePanel(m_PartyClansPanel, 1654, 0xCC071B2D);
		ForcePanel(m_PartyOnlinePanel, 1654, 0xCC071B2D);
		ForcePanel(m_PartyInvitesPanel, 1654, 0xCC071B2D);

		if ( m_MemberList )
		{
			m_MemberList.SetSort(1655);
		}

		if ( m_OnlineList )
		{
			m_OnlineList.SetSort(1655);
		}

		if ( m_ClanList )
		{
			m_ClanList.SetSort(1655);
		}

		if ( m_InviteList )
		{
			m_InviteList.SetSort(1655);
		}
	}

	protected void ForcePanel(Widget panel, int sort, int color)
	{
		if ( !panel )
		{
			return;
		}

		panel.SetSort(sort);
		panel.SetColor(color);
		panel.SetAlpha(1);
		panel.Show(true);
	}

	protected string GetRankLabel(string rank)
	{
		if ( rank == "owner" ) return "Dono";
		if ( rank == "leader" ) return "Lider";
		if ( rank == "vice" ) return "Vice";
		if ( rank == "trial" ) return "Trial";
		return "Membro";
	}
}
