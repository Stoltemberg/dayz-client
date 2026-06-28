class AdminMenu extends UIScriptedMenu
{
	protected Widget m_AdminRoot;
	protected Widget m_AdminShade;
	protected Widget m_AdminMainPanel;
	protected Widget m_AdminTopBar;
	protected Widget m_AdminPlayersPanel;
	protected Widget m_AdminActionsPanel;
	protected TextWidget m_TitleText;
	protected TextWidget m_StatusText;
	protected TextListboxWidget m_PlayerList;
	protected ButtonWidget m_RefreshButton;
	protected ButtonWidget m_CloseButton;
	protected ButtonWidget m_GodModeButton;
	protected ButtonWidget m_TeleportButton;
	protected ButtonWidget m_PullButton;
	protected ButtonWidget m_ItemsButton;
	protected ButtonWidget m_KickButton;
	protected ButtonWidget m_BanButton;
	protected ButtonWidget m_EspButton;
	protected ButtonWidget m_TeleportCityButton;
	protected ButtonWidget m_SpawnSelectedItemButton;
	protected TextListboxWidget m_CityList;
	protected TextListboxWidget m_ItemList;
	protected int m_LastPlayerListVersion;
	protected autoptr TStringArray m_PlayerUIDs;
	protected autoptr TStringArray m_PlayerNames;
	protected autoptr TStringArray m_CityNames;
	protected autoptr TStringArray m_ItemNames;

	void AdminMenu()
	{
		m_PlayerUIDs = new TStringArray;
		m_PlayerNames = new TStringArray;
		m_CityNames = new TStringArray;
		m_ItemNames = new TStringArray;
	}

	Widget Init()
	{
		layoutRoot = GetGame().GetWorkspace().CreateWidgets("gui/layouts/day_z_admin_menu.layout");

		m_AdminRoot = layoutRoot.FindAnyWidget("AdminRoot");
		m_AdminShade = layoutRoot.FindAnyWidget("AdminShade");
		m_AdminMainPanel = layoutRoot.FindAnyWidget("AdminMainPanel");
		m_AdminTopBar = layoutRoot.FindAnyWidget("AdminTopBar");
		m_AdminPlayersPanel = layoutRoot.FindAnyWidget("AdminPlayersPanel");
		m_AdminActionsPanel = layoutRoot.FindAnyWidget("AdminActionsPanel");
		m_TitleText = (TextWidget)layoutRoot.FindAnyWidget("AdminTitle");
		m_StatusText = (TextWidget)layoutRoot.FindAnyWidget("AdminStatus");
		m_PlayerList = (TextListboxWidget)layoutRoot.FindAnyWidget("AdminPlayerList");
		m_RefreshButton = (ButtonWidget)layoutRoot.FindAnyWidget("AdminRefresh");
		m_CloseButton = (ButtonWidget)layoutRoot.FindAnyWidget("AdminClose");
		m_GodModeButton = (ButtonWidget)layoutRoot.FindAnyWidget("AdminGodMode");
		m_TeleportButton = (ButtonWidget)layoutRoot.FindAnyWidget("AdminTeleportPlayer");
		m_PullButton = (ButtonWidget)layoutRoot.FindAnyWidget("AdminPullPlayer");
		m_ItemsButton = (ButtonWidget)layoutRoot.FindAnyWidget("AdminItems");
		m_KickButton = (ButtonWidget)layoutRoot.FindAnyWidget("AdminKick");
		m_BanButton = (ButtonWidget)layoutRoot.FindAnyWidget("AdminBan");
		m_EspButton = (ButtonWidget)layoutRoot.FindAnyWidget("AdminEsp");
		m_TeleportCityButton = (ButtonWidget)layoutRoot.FindAnyWidget("AdminTeleportCity");
		m_SpawnSelectedItemButton = (ButtonWidget)layoutRoot.FindAnyWidget("AdminSpawnSelectedItem");
		m_CityList = (TextListboxWidget)layoutRoot.FindAnyWidget("AdminCityList");
		m_ItemList = (TextListboxWidget)layoutRoot.FindAnyWidget("AdminItemList");
		m_LastPlayerListVersion = -1;

		ApplyAdminMenuStyle();
		LoadCityList();
		LoadItemList();

		PluginAdmin admin = PluginAdmin.GetInstance();
		if ( admin && m_StatusText )
		{
			m_StatusText.SetText("Cargo: " + admin.GetLocalRoleName() + " | UID: " + admin.GetLocalUID());
			admin.RequestPlayerList();
		}

		SetActionsEnabled(true);
		RefreshPlayerList();

		return layoutRoot;
	}

	void Update(float timeslice)
	{
		PluginAdmin admin = PluginAdmin.GetInstance();
		if ( admin && admin.GetPlayerListVersion() != m_LastPlayerListVersion )
		{
			RefreshPlayerList();
		}
	}

	void RefreshPlayerList()
	{
		if ( !m_PlayerList )
		{
			return;
		}

		m_PlayerList.ClearItems();
		m_PlayerUIDs.Clear();
		m_PlayerNames.Clear();

		PluginAdmin admin = PluginAdmin.GetInstance();
		if ( !admin )
		{
			return;
		}

		m_LastPlayerListVersion = admin.GetPlayerListVersion();

		string blob = admin.GetLastPlayerListBlob();
		if ( blob.Length() == 0 )
		{
			m_PlayerList.AddItem("Aguardando lista do servidor...", NULL, 0);
			return;
		}

		TStringArray lines = new TStringArray;
		blob.Split("\n", lines);

		for ( int i = 0; i < lines.Count(); i++ )
		{
			string line = lines.Get(i);
			TStringArray parts = new TStringArray;
			line.Split("|", parts);

			if ( parts.Count() < 4 )
			{
				continue;
			}

			string display = parts.Get(0) + "  [" + parts.Get(2) + "]  UID: " + parts.Get(1) + "  Pos: " + parts.Get(3);
			m_PlayerList.AddItem(display, NULL, 0);
			m_PlayerNames.Insert(parts.Get(0));
			m_PlayerUIDs.Insert(parts.Get(1));
		}
	}

	bool OnClick(Widget w, int x, int y, int button)
	{
		super.OnClick(w, x, y, button);

		if ( w == m_CloseButton )
		{
			GetGame().GetUIManager().Back();
			return true;
		}

		if ( w == m_RefreshButton )
		{
			PluginAdmin admin = PluginAdmin.GetInstance();
			if ( admin )
			{
				admin.RequestPlayerList();
			}
			return true;
		}

		PluginAdmin plugin_admin = PluginAdmin.GetInstance();
		if ( !plugin_admin )
		{
			return false;
		}

		if ( w == m_GodModeButton )
		{
			plugin_admin.SendCommand("godmode", "");
			SetStatus("Solicitando godmode...");
			return true;
		}

		if ( w == m_ItemsButton )
		{
			plugin_admin.SendCommand("spawn_items", "");
			SetStatus("Solicitando spawn de itens...");
			return true;
		}

		if ( w == m_SpawnSelectedItemButton )
		{
			string item_name = GetSelectedItemName();
			if ( item_name.Length() == 0 )
			{
				SetStatus("Selecione um item na lista.");
				return true;
			}

			plugin_admin.SendCommand("spawn_item", item_name);
			SetStatus("Spawnando " + item_name + "...");
			return true;
		}

		if ( w == m_EspButton )
		{
			plugin_admin.ToggleLocalEsp();
			plugin_admin.SendCommand("esp", "");
			SetStatus("ESP admin alternado.");
			return true;
		}

		if ( w == m_TeleportButton )
		{
			string tp_uid = GetSelectedPlayerUID();
			if ( tp_uid.Length() == 0 )
			{
				SetStatus("Selecione um player na lista.");
				return true;
			}

			plugin_admin.SendCommand("tp_to_player", tp_uid);
			SetStatus("Teleportando ate " + GetSelectedPlayerName() + "...");
			return true;
		}

		if ( w == m_TeleportCityButton )
		{
			string city_name = GetSelectedCityName();
			if ( city_name.Length() == 0 )
			{
				SetStatus("Selecione uma cidade na lista.");
				return true;
			}

			plugin_admin.SendCommand("tp_city", city_name);
			SetStatus("Teleportando para " + city_name + "...");
			return true;
		}

		if ( w == m_PullButton )
		{
			string pull_uid = GetSelectedPlayerUID();
			if ( pull_uid.Length() == 0 )
			{
				SetStatus("Selecione um player na lista.");
				return true;
			}

			plugin_admin.SendCommand("pull_player", pull_uid);
			SetStatus("Puxando " + GetSelectedPlayerName() + "...");
			return true;
		}

		if ( w == m_KickButton )
		{
			string kick_uid = GetSelectedPlayerUID();
			if ( kick_uid.Length() == 0 )
			{
				SetStatus("Selecione um player na lista.");
				return true;
			}

			plugin_admin.SendCommand("kick_player", kick_uid);
			SetStatus("Solicitando kick de " + GetSelectedPlayerName() + "...");
			return true;
		}

		if ( w == m_BanButton )
		{
			string ban_uid = GetSelectedPlayerUID();
			if ( ban_uid.Length() == 0 )
			{
				SetStatus("Selecione um player na lista.");
				return true;
			}

			plugin_admin.SendCommand("ban_uid", ban_uid);
			SetStatus("Registrando ban UID de " + GetSelectedPlayerName() + "...");
			return true;
		}

		return false;
	}

	bool OnKeyPress(Widget w, int x, int y, int key)
	{
		super.OnKeyPress(w, x, y, key);

		if ( key == KeyCode.KC_DELETE || key == KeyCode.KC_ESCAPE )
		{
			GetGame().GetUIManager().Back();
			return true;
		}

		return false;
	}

	protected void ApplyAdminMenuStyle()
	{
		if ( layoutRoot )
		{
			layoutRoot.SetSort(1600);
			layoutRoot.Show(true);
		}

		ForcePanel(m_AdminRoot, 1600);
		ForcePanel(m_AdminShade, 1601);
		ForcePanel(m_AdminMainPanel, 1602);
		ForcePanel(m_AdminTopBar, 1603);
		ForcePanel(m_AdminPlayersPanel, 1604);
		ForcePanel(m_AdminActionsPanel, 1604);

		if ( m_PlayerList )
		{
			m_PlayerList.SetSort(1605);
		}

		if ( m_CityList )
		{
			m_CityList.SetSort(1605);
		}

		if ( m_ItemList )
		{
			m_ItemList.SetSort(1605);
		}
	}

	protected void ForcePanel(Widget panel, int sort)
	{
		if ( !panel )
		{
			return;
		}

		panel.SetSort(sort);
		panel.SetAlpha(1);
		panel.Show(true);
	}

	protected void SetActionsEnabled(bool enabled)
	{
		SetButtonEnabled(m_GodModeButton, enabled);
		SetButtonEnabled(m_TeleportButton, enabled);
		SetButtonEnabled(m_PullButton, enabled);
		SetButtonEnabled(m_ItemsButton, enabled);
		SetButtonEnabled(m_KickButton, enabled);
		SetButtonEnabled(m_BanButton, enabled);
		SetButtonEnabled(m_EspButton, enabled);
		SetButtonEnabled(m_TeleportCityButton, enabled);
		SetButtonEnabled(m_SpawnSelectedItemButton, enabled);
	}

	protected void SetButtonEnabled(ButtonWidget button, bool enabled)
	{
		if ( !button )
		{
			return;
		}

		button.Enable(enabled);
		if ( enabled )
		{
			button.SetColor(0xCC455A64);
		}
		else
		{
			button.SetColor(0xAA30343A);
		}
	}

	protected void LoadCityList()
	{
		m_CityNames.Clear();
		if ( !m_CityList )
		{
			return;
		}

		m_CityList.ClearItems();

		PluginConfigDebugProfileFixed fixed_profile = (PluginConfigDebugProfileFixed)GetPlugin(PluginConfigDebugProfileFixed);
		TStringArray positions = new TStringArray;
		if ( fixed_profile )
		{
			fixed_profile.GetAllPositionsNames(positions);
		}

		if ( positions.Count() == 0 )
		{
			AddCity("Severograd");
			AddCity("Krasnostav");
			AddCity("Stary Sobor");
			AddCity("Vybor");
			AddCity("Airport Vybor");
			AddCity("Chernogorsk Center");
			AddCity("Elektrozavodsk Center");
		}
		else
		{
			for ( int i = 0; i < positions.Count(); i++ )
			{
				AddCity(positions.Get(i));
			}
		}
	}

	protected void AddCity(string city_name)
	{
		if ( city_name.Length() == 0 || !m_CityList )
		{
			return;
		}

		m_CityList.AddItem(city_name, NULL, 0);
		m_CityNames.Insert(city_name);
	}

	protected void LoadItemList()
	{
		m_ItemNames.Clear();
		if ( !m_ItemList )
		{
			return;
		}

		m_ItemList.ClearItems();
		AddSpawnItem("EN5C_Rice");
		AddSpawnItem("EN5C_WaterBottle");
		AddSpawnItem("EN5C_BandageDressing");
		AddSpawnItem("EN5C_Rag");
		AddSpawnItem("EN5C_Roadflare");
		AddSpawnItem("EN5C_TacticalBaconCan");
		AddSpawnItem("EN5C_SardinesCan");
		AddSpawnItem("Drink_Canteen");
		AddSpawnItem("BagMountain_Green");
		AddSpawnItem("M4A1_Black");
		AddSpawnItem("M_STANAG_30Rnd_Coupled");
		AddSpawnItem("Mosin9130");
		AddSpawnItem("CLIP_762_5Rnd");
		AddSpawnItem("magnum");
		AddSpawnItem("M_357_Speedloader");
		AddSpawnItem("FirefighterAxe");
		AddSpawnItem("CombatKnife");
	}

	protected void AddSpawnItem(string item_name)
	{
		if ( item_name.Length() == 0 || !m_ItemList )
		{
			return;
		}

		m_ItemList.AddItem(item_name, NULL, 0);
		m_ItemNames.Insert(item_name);
	}

	protected string GetSelectedPlayerUID()
	{
		if ( !m_PlayerList || m_PlayerUIDs.Count() == 0 )
		{
			return "";
		}

		int row = m_PlayerList.GetSelectedRow();
		if ( row < 0 && m_PlayerUIDs.Count() == 1 )
		{
			row = 0;
		}

		if ( row < 0 || row >= m_PlayerUIDs.Count() )
		{
			return "";
		}

		return m_PlayerUIDs.Get(row);
	}

	protected string GetSelectedPlayerName()
	{
		if ( !m_PlayerList || m_PlayerNames.Count() == 0 )
		{
			return "player";
		}

		int row = m_PlayerList.GetSelectedRow();
		if ( row < 0 && m_PlayerNames.Count() == 1 )
		{
			row = 0;
		}

		if ( row < 0 || row >= m_PlayerNames.Count() )
		{
			return "player";
		}

		return m_PlayerNames.Get(row);
	}

	protected string GetSelectedCityName()
	{
		if ( !m_CityList || m_CityNames.Count() == 0 )
		{
			return "";
		}

		int row = m_CityList.GetSelectedRow();
		if ( row < 0 && m_CityNames.Count() == 1 )
		{
			row = 0;
		}

		if ( row < 0 || row >= m_CityNames.Count() )
		{
			return "";
		}

		return m_CityNames.Get(row);
	}

	protected string GetSelectedItemName()
	{
		if ( !m_ItemList || m_ItemNames.Count() == 0 )
		{
			return "";
		}

		int row = m_ItemList.GetSelectedRow();
		if ( row < 0 && m_ItemNames.Count() == 1 )
		{
			row = 0;
		}

		if ( row < 0 || row >= m_ItemNames.Count() )
		{
			return "";
		}

		return m_ItemNames.Get(row);
	}

	protected void SetStatus(string text)
	{
		if ( m_StatusText )
		{
			m_StatusText.SetText(text);
		}

		PlayerBase player = GetGame().GetPlayer();
		if ( player )
		{
			player.MessageStatus(text);
		}
	}
}
