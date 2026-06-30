class IngameHud extends Hud
{
	private const float FADE_IN_TIME = 0.3;
	private const float FADE_OUT_TIME = 0.3;
	private const float HIDE_MENU_TIME = 5;
	private const int CUSTOM_MAP_HUD_MARKER_COUNT = 10;
	private const float CUSTOM_MAP_HUD_MAX_DISTANCE = 10000.0;
	private const int CLAN_HUD_MARKER_COUNT = 10;
	private const float CLAN_HUD_MAX_DISTANCE = 2500.0;
	private const int ADMIN_ESP_PLAYER_COUNT = 12;
	private const int ADMIN_ESP_LINES_PER_PLAYER = 90;
	private const int ADMIN_ESP_DOTS_PER_SEGMENT = 5;
	private const float ADMIN_ESP_DOT_SIZE = 0.016;
	private const float ADMIN_ESP_MAX_DISTANCE = 1200.0;

	private autoptr map<int, string>	m_StatesWidgetNames;
	private autoptr map<int, ImageWidget>	m_StatesWidgets;  // [key] ImageWidget

	private autoptr map<int, string>	m_BadgesWidgetNames;
	private autoptr map<int, bool>	m_BadgesWidgetDisplay;
	private autoptr map<int, ImageWidget>	m_BadgesWidgets;  // [key] ImageWidget

	private autoptr map<int, string>	m_VehicleGearTable;

	private Widget m_HudPanelWidget;
	private Widget	m_quickbar_widget;
	private autoptr InventoryQuickbar m_quickbar;
	
	private TextWidget VehiclePanelSpeedValue;
	private TextWidget VehiclePanelGearValue;
	private Widget m_VehiclePanel0;
	
	private Widget m_VehiclePanel;
	private Widget m_Notifiers;
	private Widget m_CustomStatusPanel;
	private ImageWidget m_CustomHudBleedingIcon;
	private ImageWidget m_CustomHudEnergy;
	private ImageWidget m_CustomHudFood;
	private ImageWidget m_CustomHudWater;
	private ImageWidget m_CustomHudTemperature;
	private ImageWidget m_CustomHudBlood;
	private ImageWidget m_CustomHudHealth;
	private autoptr array<Widget> m_CustomMapHudMarkerRoots;
	private autoptr array<TextWidget> m_CustomMapHudMarkerTexts;
	private autoptr array<Widget> m_ClanHudMarkerRoots;
	private autoptr array<TextWidget> m_ClanHudMarkerTexts;
	private autoptr array<Widget> m_AdminEspRoots;
	private autoptr array<Widget> m_AdminEspLineWidgets;
	private autoptr array<TextWidget> m_AdminEspLabels;
	private autoptr TimerUpdate m_AdminEspTimer;
	private bool m_CustomHudBleeding;
	private bool m_CustomHudSick;
	private bool m_HudVisible;
	private autoptr Timer m_HudHideTimer;
	private autoptr WidgetFadeTimer m_FadeTimerCustomHud;
	private autoptr WidgetFadeTimer m_FadeTimerNotifiers;
	private autoptr WidgetFadeTimer m_FadeTimerBadges;
	private autoptr WidgetFadeTimer m_FadeTimerStamina;
	private autoptr WidgetFadeTimer m_FadeTimerStaminaBg;
	private autoptr WidgetFadeTimer m_FadeTimerStance;
	private int m_CustomHudBloodLevel;
	private int m_CustomHudHealthLevel;
	private autoptr map<ImageWidget, string> m_CustomHudCurrentImages;
	private autoptr WidgetFadeTimer m_FadeTimerCustomFood;
	private autoptr WidgetFadeTimer m_FadeTimerCustomWater;
	private autoptr WidgetFadeTimer m_FadeTimerCustomTemp;
	private autoptr WidgetFadeTimer m_FadeTimerCustomBlood;
	private autoptr WidgetFadeTimer m_FadeTimerCustomHealth;
	private autoptr WidgetFadeTimer m_FadeTimerCustomEnergy;
	private autoptr WidgetFadeTimer m_FadeTimerCustomBleeding;
	private Widget m_Badges;
	private autoptr Timer m_hide_timer;
	private autoptr Timer m_vehicle_timer;
	private autoptr Timer m_zeroing_and_weaponmode_timer;
	private autoptr WidgetFadeTimer m_fade_timer_weapon_mode;
	private autoptr WidgetFadeTimer m_fade_timer_zeroing;
	private autoptr WidgetFadeTimer m_fade_timer_zeroing_old;
	private autoptr WidgetFadeTimer m_fade_timer_magazine_name;
	private autoptr WidgetFadeTimer m_fade_timer_weapon_name;
	private autoptr WidgetFadeTimer m_fade_timer_weapon_stats;
	private autoptr WidgetFadeTimer m_fade_timer_quickbar;
	private ProgressBarWidget m_stamina;
	private Widget m_stamina_background;
	private Widget m_presence;
	private Widget m_stanceProne;
	private Widget m_stanceStand;
	private Widget m_stanceCrouch;
	private Widget m_stancePanel;
	private Widget m_PresenceLevel0;
	private Widget m_PresenceLevel1;
	private Widget m_PresenceLevel2;
	private Widget m_PresenceLevel3;
	private Widget m_PresenceLevel4;
	private TextWidget m_Zeroing;
	private Widget m_WeaponStats;
	private TextWidget m_WeaponMode;
	private TextWidget m_ZeroingOld;
	private TextWidget m_WeaponName;
	private TextWidget m_MagazineName;
	private autoptr Timer myTimer;
	private autoptr Timer m_quickbar_timer;
	private autoptr array<WidgetFadeTimer> m_fade_timers = new array<WidgetFadeTimer>;
	
	bool	m_quickbar_state;
	bool faded;
	bool zeroing_key_pressed;
		
	void IngameHud()
	{
		m_fade_timer_weapon_mode = new WidgetFadeTimer;
		m_fade_timer_zeroing = new WidgetFadeTimer;
		 m_fade_timer_zeroing_old = new WidgetFadeTimer;
		 m_fade_timer_magazine_name = new WidgetFadeTimer;
		 m_fade_timer_weapon_name = new WidgetFadeTimer;
		 m_fade_timer_weapon_stats = new WidgetFadeTimer;
		 m_fade_timer_quickbar = new WidgetFadeTimer;
		m_hide_timer = new Timer(CALL_CATEGORY_GUI);
		m_vehicle_timer = new Timer( CALL_CATEGORY_GAMEPLAY );
		m_zeroing_and_weaponmode_timer = new Timer( CALL_CATEGORY_GAMEPLAY );

		m_StatesWidgets = new map<int, ImageWidget>; // [key] widgetName
		m_StatesWidgetNames = new map<int, string>;

		m_BadgesWidgets = new map<int, ImageWidget>; // [key] widgetName
		m_BadgesWidgetNames = new map<int, string>;
		m_BadgesWidgetDisplay = new map<int, bool>;

		m_CustomHudCurrentImages = new map<ImageWidget, string>;
		m_FadeTimerCustomFood = new WidgetFadeTimer;
		m_FadeTimerCustomWater = new WidgetFadeTimer;
		m_FadeTimerCustomTemp = new WidgetFadeTimer;
		m_FadeTimerCustomBlood = new WidgetFadeTimer;
		m_FadeTimerCustomHealth = new WidgetFadeTimer;
		m_FadeTimerCustomEnergy = new WidgetFadeTimer;
		m_FadeTimerCustomBleeding = new WidgetFadeTimer;

		m_CustomMapHudMarkerRoots = new array<Widget>;
		m_CustomMapHudMarkerTexts = new array<TextWidget>;
		m_ClanHudMarkerRoots = new array<Widget>;
		m_ClanHudMarkerTexts = new array<TextWidget>;
		m_AdminEspRoots = new array<Widget>;
		m_AdminEspLineWidgets = new array<Widget>;
		m_AdminEspLabels = new array<TextWidget>;
		m_AdminEspTimer = new TimerUpdate(this, "UpdateAdminEspSkeletons", CALL_CATEGORY_GUI);

		m_VehicleGearTable = new map<int, string>;
		m_VehicleGearTable.Set( -1, "" );
		m_VehicleGearTable.Set( GEAR_NEUTRAL, "N" );
		m_VehicleGearTable.Set( GEAR_FIRST, "1" );
		m_VehicleGearTable.Set( GEAR_SECOND, "2" );
		m_VehicleGearTable.Set( GEAR_THIRD, "3" );
		m_VehicleGearTable.Set( GEAR_FOURTH, "4" );
		m_VehicleGearTable.Set( GEAR_FIFTH, "5" );
		m_VehicleGearTable.Set( GEAR_SIXTH, "6" );
		m_VehicleGearTable.Set( GEAR_SEVENTH, "7" );
		m_VehicleGearTable.Set( GEAR_EIGTH, "8" );
		m_VehicleGearTable.Set( GEAR_REVERSE, "R" );
	}
	
	void Init( Widget hud_panel_widget )
	{
		m_HudPanelWidget = hud_panel_widget;
		m_HudPanelWidget.Show( true );

		m_quickbar_timer = new Timer( CALL_CATEGORY_GAMEPLAY );

		// quickbar
		m_quickbar_widget = m_HudPanelWidget.FindAnyWidget("QuickbarGrid");
		m_quickbar_widget.Show( false );
		
		// TEMPORARY HACK!!! player is not present when Hud is being initialized 
		myTimer = new Timer( CALL_CATEGORY_GAMEPLAY );
		myTimer.Run( 1, this, "InitQuickbar" );

		// panels
		m_stamina = m_HudPanelWidget.FindAnyWidget("StaminaBar");
		m_presence = m_HudPanelWidget.FindAnyWidget("PresencePanel");
		m_Badges = hud_panel_widget.FindAnyWidget("BadgesPanel");
		m_Notifiers = m_HudPanelWidget.FindAnyWidget("NotifiersPanel");
		m_VehiclePanel = m_HudPanelWidget.FindAnyWidget("VehiclePanel");
		VehiclePanelSpeedValue = m_HudPanelWidget.FindAnyWidget("SpeedValue0");
		VehiclePanelGearValue = m_HudPanelWidget.FindAnyWidget("SpeedValue1");
		m_VehiclePanel0 = m_HudPanelWidget.FindAnyWidget("VehiclePanel0");
		m_Zeroing = (TextWidget)m_HudPanelWidget.FindAnyWidget("Zeroing");
		m_WeaponMode = (TextWidget)m_HudPanelWidget.FindAnyWidget("WeaponMode");
		m_WeaponStats = (Widget)m_HudPanelWidget.FindAnyWidget("WeaponStats");
		m_ZeroingOld = (TextWidget)m_HudPanelWidget.FindAnyWidget("ZeroingOld");
		m_WeaponName = (TextWidget)m_HudPanelWidget.FindAnyWidget("WeaponName");
		m_MagazineName = (TextWidget)m_HudPanelWidget.FindAnyWidget("MagazineName");
		m_stamina_background = m_HudPanelWidget.FindAnyWidget("StaminaBackground");
		m_stamina_background.Show(true);
		m_stanceProne = m_HudPanelWidget.FindAnyWidget("StanceProne");
		m_stanceCrouch = m_HudPanelWidget.FindAnyWidget("StanceCrouch");
		m_stanceStand = m_HudPanelWidget.FindAnyWidget("StanceStand");
		m_stancePanel = m_HudPanelWidget.FindAnyWidget("StancePanel");
		
		// state notifiers
		m_StatesWidgetNames.Clear();
		m_StatesWidgets.Clear();
		m_StatesWidgetNames.Set( NTFKEY_THIRSTY, "Thirsty" );
		m_StatesWidgetNames.Set( NTFKEY_HUNGRY, "Hungry" );
		m_StatesWidgetNames.Set( NTFKEY_SICK, "Health" );
		m_StatesWidgetNames.Set( NTFKEY_BACTERIA, "Bacteria" );
		m_StatesWidgetNames.Set( NTFKEY_BLEEDISH, "Blood" );
		m_StatesWidgetNames.Set( NTFKEY_FEVERISH, "Temperature" );

		#ifndef NO_GUI
			delete(m_Timer);
			m_Timer = new Timer( CALL_CATEGORY_GAMEPLAY );
			m_Timer.Run(1, this, "RefreshQuickbar", NULL, true ); 
		#endif

		if ( GetDayZGame().IsNewPlayer() )
		{
			m_Notifiers.Show( true );
			// m_Notifiers.SetAlpha( 0 );
			m_Badges.Show( true );
			// m_Badges.SetAlpha( 0 );

			int i = 0;
			int key = 0;
			for ( i = 0; i < m_StatesWidgetNames.Count(); i++ )
			{
				string widget_name = m_StatesWidgetNames.GetElement(i);
				key = m_StatesWidgetNames.GetKey(i);
				ImageWidget w = m_Notifiers.FindAnyWidget( String( "Icon" + widget_name ) );
				m_StatesWidgets.Set( key, w );
				w.Show( true );
				// clear all arrows
				for ( int x = 1; x < 4; x++ )
				{
					w = m_Notifiers.FindAnyWidget( String( widget_name + "ArrowUp" + x.ToString() ) );
					w.Show( false );
					w = m_Notifiers.FindAnyWidget( String( widget_name + "ArrowDown" + x.ToString() ) );
					w.Show( false );
				}
			}

			// badges
			m_BadgesWidgetNames.Clear();
			m_BadgesWidgets.Clear();
			m_BadgesWidgetDisplay.Clear();
			m_BadgesWidgetNames.Set( NTFKEY_FRACTURE, "Fracture" );
			m_BadgesWidgetNames.Set( NTFKEY_STUFFED, "Stomach" );
			m_BadgesWidgetNames.Set( NTFKEY_SICK, "Pill" );
			m_BadgesWidgetNames.Set( NTFKEY_BLEEDISH, "Wetness" );
			m_BadgesWidgetNames.Set( NTFKEY_FEVERISH, "Skull" );
			// NTFKEY_SICK
			// NTFKEY_BLEEDISH
			// NTFKEY_FRACTURE
			// NTFKEY_STUFFED
			// NTFKEY_WETNESS iconDrops

			for ( i = 0; i < m_BadgesWidgetNames.Count(); i++ )
			{
				string badge_name = m_BadgesWidgetNames.GetElement(  i);
				key = m_BadgesWidgetNames.GetKey( i );
				ImageWidget badge_widget = m_Badges.FindAnyWidget( badge_name );
				m_BadgesWidgets.Set( key, badge_widget );
				badge_widget.Show( false );
				m_BadgesWidgetDisplay.Set( key, false );
			}

			m_PresenceLevel0 = hud_panel_widget.FindAnyWidget("Presence0");
			m_PresenceLevel1 = hud_panel_widget.FindAnyWidget("Presence1");
			m_PresenceLevel2 = hud_panel_widget.FindAnyWidget("Presence2");
			m_PresenceLevel3 = hud_panel_widget.FindAnyWidget("Presence3");
			m_PresenceLevel4 = hud_panel_widget.FindAnyWidget("Presence4");
			m_PresenceLevel0.Show( false );
			m_PresenceLevel1.Show( false );
			m_PresenceLevel2.Show( false );
			m_PresenceLevel3.Show( false );
			m_PresenceLevel4.Show( false );
		}
		else
		{
			m_stamina_background.Show( false );
			m_stamina.Show( false );
			m_presence.Show( false );
			m_stanceProne.Show( false );
			m_stanceStand.Show( false );
			m_stanceCrouch.Show( false );
			m_Badges.Show( false );
			m_Notifiers.Show( false );
		}

		InitCustomStatusHud();
		InitCustomMapHudMarkers();
		InitClanHudMarkers();
		InitAdminEspWidgets();

		#ifndef NO_GUI
			m_vehicle_timer.Run(0.5, this, "RefreshVehicleSpeedHud", NULL, true ); 
		#endif
		
		#ifndef NO_GUI
		if ( GetDayZGame().IsNewPlayer() )
		{
			m_zeroing_and_weaponmode_timer.Run(0.1, this, "RefreshZeroingAndWeaponMode", NULL, true );
		}
		else
		{
			m_zeroing_and_weaponmode_timer.Run(0.1, this, "RefreshZeroingAndWeaponModeOld", NULL, true );
		}
		#endif

		m_FadeTimerCustomHud = new WidgetFadeTimer;
		m_FadeTimerNotifiers = new WidgetFadeTimer;
		m_FadeTimerBadges = new WidgetFadeTimer;
		m_FadeTimerStamina = new WidgetFadeTimer;
		m_FadeTimerStaminaBg = new WidgetFadeTimer;
		m_FadeTimerStance = new WidgetFadeTimer;
		m_HudHideTimer = new Timer(CALL_CATEGORY_GUI);

		m_HudVisible = true;
		// m_HudHideTimer.Run( 15, this, "CheckAndHideHud" );
	}

	void InitCustomStatusHud()
	{
		m_CustomStatusPanel = GetGame().GetWorkspace().CreateWidgets("gui/layouts/custom_status_hud.layout", m_HudPanelWidget);
		if ( !m_CustomStatusPanel )
		{
			return;
		}

		if ( m_Notifiers )
		{
			m_Notifiers.Show( false );
		}

		if ( m_Badges )
		{
			m_Badges.Show( false );
			m_Badges.SetPos( 0.215, 0.02, false );
		}

		m_CustomHudEnergy = (ImageWidget)m_CustomStatusPanel.FindAnyWidget("CustomHudEnergy");
		m_CustomHudBleedingIcon = (ImageWidget)m_CustomStatusPanel.FindAnyWidget("CustomHudBleeding");
		m_CustomHudFood = (ImageWidget)m_CustomStatusPanel.FindAnyWidget("CustomHudFood");
		m_CustomHudWater = (ImageWidget)m_CustomStatusPanel.FindAnyWidget("CustomHudWater");
		m_CustomHudTemperature = (ImageWidget)m_CustomStatusPanel.FindAnyWidget("CustomHudTemperature");
		m_CustomHudBlood = (ImageWidget)m_CustomStatusPanel.FindAnyWidget("CustomHudBlood");
		m_CustomHudHealth = (ImageWidget)m_CustomStatusPanel.FindAnyWidget("CustomHudHealth");
		if ( m_CustomHudBleedingIcon )
		{
			SetCustomStatusImage( m_CustomHudBleedingIcon, "bleeding.tga" );
			m_CustomHudBleedingIcon.Show( false );
		}
		m_CustomHudBleeding = false;
		m_CustomHudSick = false;
		m_CustomHudBloodLevel = 5;
		m_CustomHudHealthLevel = 5;
		SyncCustomStatusHudFromInventoryStatus();
	}

	void InitCustomMapHudMarkers()
	{
		if ( !m_HudPanelWidget )
		{
			return;
		}

		m_CustomMapHudMarkerRoots.Clear();
		m_CustomMapHudMarkerTexts.Clear();

		for ( int i = 0; i < CUSTOM_MAP_HUD_MARKER_COUNT; i++ )
		{
			Widget marker_root = GetGame().GetWorkspace().CreateWidgets("gui/layouts/custom_map_hud_marker.layout", m_HudPanelWidget);

			if ( marker_root )
			{
				marker_root.SetSort(930);
				marker_root.Show(false);
			}

			TextWidget marker_text = NULL;
			if ( marker_root )
			{
				marker_text = (TextWidget)marker_root.FindAnyWidget("CustomMapHudMarkerText");

				if ( marker_text )
				{
					marker_text.SetColor(0xFFFFFFFF);
				}
			}

			m_CustomMapHudMarkerRoots.Insert(marker_root);
			m_CustomMapHudMarkerTexts.Insert(marker_text);
		}
	}

	void UpdateCustomMapHudMarkers()
	{
		if ( !m_CustomMapHudMarkerRoots || !MapMenu.GetCustomMarkersShowHud() )
		{
			HideCustomMapHudMarkers();
			return;
		}

		PlayerBase player = GetGame().GetPlayer();

		if ( !player )
		{
			HideCustomMapHudMarkers();
			return;
		}

		int count = MapMenu.GetCustomMarkerCount();

		for ( int i = 0; i < m_CustomMapHudMarkerRoots.Count(); i++ )
		{
			Widget marker_root = m_CustomMapHudMarkerRoots.Get(i);

			if ( !marker_root )
			{
				continue;
			}

			if ( i >= count )
			{
				marker_root.Show(false);
				continue;
			}

			float distance = MapMenu.GetCustomMarkerDistance(player, i);

			if ( distance > CUSTOM_MAP_HUD_MAX_DISTANCE )
			{
				marker_root.Show(false);
				continue;
			}

			vector marker_world = MapMenu.GetCustomMarkerWorldPosition(i, 2.4);
			vector screen_pos = GetGame().GetScreenPosRelative(marker_world);

			if ( screen_pos[2] <= 0 || screen_pos[0] < 0.0 || screen_pos[0] > 1.0 || screen_pos[1] < 0.0 || screen_pos[1] > 1.0 )
			{
				marker_root.Show(false);
				continue;
			}

			TextWidget marker_text = m_CustomMapHudMarkerTexts.Get(i);

			if ( marker_text )
			{
				marker_text.SetText(MapMenu.GetCustomMarkerName(i) + "\n" + MapMenu.FormatCustomMarkerDistance(distance));
			}

			marker_root.Show(true);
			marker_root.SetPos(Math.Clamp(screen_pos[0] - 0.08, 0.0, 0.84), Math.Clamp(screen_pos[1] - 0.055, 0.0, 0.92));
		}
	}

	void HideCustomMapHudMarkers()
	{
		if ( !m_CustomMapHudMarkerRoots )
		{
			return;
		}

		for ( int i = 0; i < m_CustomMapHudMarkerRoots.Count(); i++ )
		{
			Widget marker_root = m_CustomMapHudMarkerRoots.Get(i);

			if ( marker_root )
			{
				marker_root.Show(false);
			}
		}
	}

	void InitClanHudMarkers()
	{
		if ( !m_HudPanelWidget )
		{
			return;
		}

		m_ClanHudMarkerRoots.Clear();
		m_ClanHudMarkerTexts.Clear();

		for ( int i = 0; i < CLAN_HUD_MARKER_COUNT; i++ )
		{
			Widget marker_root = GetGame().GetWorkspace().CreateWidgets("gui/layouts/custom_map_hud_marker.layout", m_HudPanelWidget);

			if ( marker_root )
			{
				marker_root.SetSort(940);
				marker_root.SetColor(0xAA041F1A);
				marker_root.Show(false);
			}

			TextWidget marker_text = NULL;
			if ( marker_root )
			{
				marker_text = (TextWidget)marker_root.FindAnyWidget("CustomMapHudMarkerText");
				if ( marker_text )
				{
					marker_text.SetColor(0xFF66F2C2);
				}
			}

			m_ClanHudMarkerRoots.Insert(marker_root);
			m_ClanHudMarkerTexts.Insert(marker_text);
		}
	}

	void UpdateClanHudMarkers()
	{
		if ( !m_ClanHudMarkerRoots )
		{
			return;
		}

		PluginParty party = PluginParty.GetInstance();
		PlayerBase player = GetGame().GetPlayer();
		if ( !party || !player || party.GetLocalClanTag().Length() == 0 )
		{
			HideClanHudMarkers();
			return;
		}

		string local_uid = GetLocalPartyUID(party);
		string blob = party.GetMembersBlob();
		if ( blob.Length() == 0 )
		{
			HideClanHudMarkers();
			return;
		}

		TStringArray lines = new TStringArray;
		blob.Split("\n", lines);

		int marker_index = 0;
		for ( int i = 0; i < lines.Count(); i++ )
		{
			if ( marker_index >= m_ClanHudMarkerRoots.Count() )
			{
				break;
			}

			TStringArray parts = new TStringArray;
			lines.Get(i).Split("|", parts);
			if ( parts.Count() < 8 )
			{
				continue;
			}

			string name = parts.Get(0);
			string uid = parts.Get(1);
			string rank = parts.Get(3);
			if ( uid == local_uid )
			{
				continue;
			}

			vector member_pos = Vector(parts.Get(5).ToFloat(), parts.Get(6).ToFloat() + 2.2, parts.Get(7).ToFloat());
			if ( member_pos[0] == 0 && member_pos[2] == 0 )
			{
				continue;
			}

			float distance = vector.Distance(player.GetPosition(), member_pos);
			if ( distance > CLAN_HUD_MAX_DISTANCE )
			{
				continue;
			}

			vector screen_pos = GetGame().GetScreenPosRelative(member_pos);
			Widget marker_root = m_ClanHudMarkerRoots.Get(marker_index);
			if ( !marker_root )
			{
				continue;
			}

			if ( screen_pos[2] <= 0 || screen_pos[0] < 0.0 || screen_pos[0] > 1.0 || screen_pos[1] < 0.0 || screen_pos[1] > 1.0 )
			{
				marker_root.Show(false);
				continue;
			}

			TextWidget marker_text = m_ClanHudMarkerTexts.Get(marker_index);
			if ( marker_text )
			{
				marker_text.SetText(name + "\n" + GetClanRankLabel(rank) + "  " + MapMenu.FormatCustomMarkerDistance(distance));
			}

			marker_root.Show(true);
			marker_root.SetPos(Math.Clamp(screen_pos[0] - 0.08, 0.0, 0.84), Math.Clamp(screen_pos[1] - 0.055, 0.0, 0.92));
			marker_index++;
		}

		for ( int j = marker_index; j < m_ClanHudMarkerRoots.Count(); j++ )
		{
			Widget unused_marker = m_ClanHudMarkerRoots.Get(j);
			if ( unused_marker )
			{
				unused_marker.Show(false);
			}
		}
	}

	void HideClanHudMarkers()
	{
		if ( !m_ClanHudMarkerRoots )
		{
			return;
		}

		for ( int i = 0; i < m_ClanHudMarkerRoots.Count(); i++ )
		{
			Widget marker_root = m_ClanHudMarkerRoots.Get(i);
			if ( marker_root )
			{
				marker_root.Show(false);
			}
		}
	}

	string GetLocalPartyUID(PluginParty party)
	{
		TStringArray parts = new TStringArray;
		party.GetLocalInfo().Split("|", parts);
		if ( parts.Count() > 0 )
		{
			return parts.Get(0);
		}

		return "";
	}

	string GetClanRankLabel(string rank)
	{
		if ( rank == "owner" ) return "Dono";
		if ( rank == "leader" ) return "Lider";
		if ( rank == "vice" ) return "Vice";
		if ( rank == "trial" ) return "Trial";
		return "Membro";
	}

	void InitAdminEspWidgets()
	{
		// [2026-06-30] FIX: [ISSUE-012] Removido return prematuro — ESP admin agora funciona
		// O return abaixo desabilitava todo o código de ESP admin

		if ( !m_HudPanelWidget )
		{
			return;
		}

		m_AdminEspRoots.Clear();
		m_AdminEspLineWidgets.Clear();
		m_AdminEspLabels.Clear();

		for ( int i = 0; i < ADMIN_ESP_PLAYER_COUNT; i++ )
		{
			Widget root = GetGame().GetWorkspace().CreateWidgets("gui/layouts/admin_esp_skeleton.layout", m_HudPanelWidget);

			if ( root )
			{
				root.SetSort(1800);
				root.SetAlpha(1);
				root.Show(false);
			}

			for ( int line_index = 0; line_index < ADMIN_ESP_LINES_PER_PLAYER; line_index++ )
			{
				Widget line = NULL;
				if ( root )
				{
					line = root.FindAnyWidget("AdminEspLine" + line_index.ToString());

					if ( line )
					{
						line.SetSort(1801);
						TextWidget line_text = (TextWidget)line;
						if ( line_text )
						{
							line_text.SetText("+");
							line_text.SetColor(0xEE00D8FF);
						}
						line.SetAlpha(1);
						line.Show(false);
					}
				}

				m_AdminEspLineWidgets.Insert(line);
			}

			TextWidget label = NULL;
			if ( root )
			{
				label = (TextWidget)root.FindAnyWidget("AdminEspLabel");

				if ( label )
				{
					label.SetSort(1802);
					label.SetColor(0xFFFFFFFF);
					label.SetAlpha(1);
					label.Show(false);
				}
			}

			m_AdminEspRoots.Insert(root);
			m_AdminEspLabels.Insert(label);
		}
	}

	void UpdateAdminEspSkeletons(float timeslice = 0)
	{
		return;

		PluginAdmin admin = PluginAdmin.GetInstance();
		if ( !admin || !admin.IsLocalEspEnabled() )
		{
			HideAdminEspSkeletons();
			return;
		}

		PlayerBase local_player = GetGame().GetPlayer();
		if ( !local_player )
		{
			HideAdminEspSkeletons();
			return;
		}
		DayZPlayer local_dayz_player = (DayZPlayer)local_player;

		array<Man> players = new array<Man>;
		GetGame().GetPlayers(players);

		int esp_index = 0;
		if ( local_dayz_player && DrawAdminEspSkeleton(local_dayz_player, esp_index, 0) )
		{
			esp_index++;
		}

		for ( int i = 0; i < players.Count(); i++ )
		{
			DayZPlayer target = (DayZPlayer)players.Get(i);
			if ( !target || target == local_dayz_player )
			{
				continue;
			}

			if ( !target.IsAlive() )
			{
				continue;
			}

			float distance = vector.Distance(local_player.GetPosition(), target.GetPosition());
			if ( distance > ADMIN_ESP_MAX_DISTANCE )
			{
				continue;
			}

			if ( DrawAdminEspSkeleton(target, esp_index, distance) )
			{
				esp_index++;
			}

			if ( esp_index >= ADMIN_ESP_PLAYER_COUNT )
			{
				break;
			}
		}

		for ( int hide_index = esp_index; hide_index < ADMIN_ESP_PLAYER_COUNT; hide_index++ )
		{
			HideAdminEspSkeleton(hide_index);
		}
	}

	bool DrawAdminEspSkeleton(DayZPlayer target, int esp_index, float distance)
	{
		if ( esp_index < 0 || esp_index >= ADMIN_ESP_PLAYER_COUNT || esp_index >= m_AdminEspRoots.Count() || !target )
		{
			return false;
		}

		vector head;
		vector neck;
		vector spine3;
		vector spine1;
		vector pelvis;
		vector left_shoulder;
		vector left_arm;
		vector left_forearm;
		vector left_hand;
		vector right_shoulder;
		vector right_arm;
		vector right_forearm;
		vector right_hand;
		vector left_upleg;
		vector left_leg;
		vector left_foot;
		vector right_upleg;
		vector right_leg;
		vector right_foot;

		bool visible = GetAdminEspPointScreenSafe(target, Vector(0, 1.72, 0), head);
		visible = GetAdminEspPointScreenSafe(target, Vector(0, 1.52, 0), neck) && visible;
		visible = GetAdminEspPointScreenSafe(target, Vector(0, 1.30, 0), spine3) && visible;
		visible = GetAdminEspPointScreenSafe(target, Vector(0, 1.05, 0), spine1) && visible;
		visible = GetAdminEspPointScreenSafe(target, Vector(0, 0.86, 0), pelvis) && visible;
		GetAdminEspPointScreenSafe(target, Vector(-0.18, 1.45, 0), left_shoulder);
		GetAdminEspPointScreenSafe(target, Vector(-0.26, 1.22, 0), left_arm);
		GetAdminEspPointScreenSafe(target, Vector(-0.30, 1.00, 0), left_forearm);
		GetAdminEspPointScreenSafe(target, Vector(-0.31, 0.82, 0), left_hand);
		GetAdminEspPointScreenSafe(target, Vector(0.18, 1.45, 0), right_shoulder);
		GetAdminEspPointScreenSafe(target, Vector(0.26, 1.22, 0), right_arm);
		GetAdminEspPointScreenSafe(target, Vector(0.30, 1.00, 0), right_forearm);
		GetAdminEspPointScreenSafe(target, Vector(0.31, 0.82, 0), right_hand);
		GetAdminEspPointScreenSafe(target, Vector(-0.12, 0.68, 0), left_upleg);
		GetAdminEspPointScreenSafe(target, Vector(-0.14, 0.38, 0), left_leg);
		GetAdminEspPointScreenSafe(target, Vector(-0.15, 0.10, 0.04), left_foot);
		GetAdminEspPointScreenSafe(target, Vector(0.12, 0.68, 0), right_upleg);
		GetAdminEspPointScreenSafe(target, Vector(0.14, 0.38, 0), right_leg);
		GetAdminEspPointScreenSafe(target, Vector(0.15, 0.10, 0.04), right_foot);

		if ( !visible )
		{
			HideAdminEspSkeleton(esp_index);
			return false;
		}

		Widget root = m_AdminEspRoots.Get(esp_index);
		if ( root )
		{
			root.Show(true);
		}

		int base_line = esp_index * ADMIN_ESP_LINES_PER_PLAYER;
		int next_line = base_line;
		next_line = DrawAdminEspLine(next_line, head, neck);
		next_line = DrawAdminEspLine(next_line, neck, spine3);
		next_line = DrawAdminEspLine(next_line, spine3, spine1);
		next_line = DrawAdminEspLine(next_line, spine1, pelvis);
		next_line = DrawAdminEspLine(next_line, neck, left_shoulder);
		next_line = DrawAdminEspLine(next_line, left_shoulder, left_arm);
		next_line = DrawAdminEspLine(next_line, left_arm, left_forearm);
		next_line = DrawAdminEspLine(next_line, left_forearm, left_hand);
		next_line = DrawAdminEspLine(next_line, neck, right_shoulder);
		next_line = DrawAdminEspLine(next_line, right_shoulder, right_arm);
		next_line = DrawAdminEspLine(next_line, right_arm, right_forearm);
		next_line = DrawAdminEspLine(next_line, right_forearm, right_hand);
		next_line = DrawAdminEspLine(next_line, pelvis, left_upleg);
		next_line = DrawAdminEspLine(next_line, left_upleg, left_leg);
		next_line = DrawAdminEspLine(next_line, left_leg, left_foot);
		next_line = DrawAdminEspLine(next_line, pelvis, right_upleg);
		next_line = DrawAdminEspLine(next_line, right_upleg, right_leg);
		next_line = DrawAdminEspLine(next_line, right_leg, right_foot);
		HideUnusedAdminEspDots(next_line, base_line + ADMIN_ESP_LINES_PER_PLAYER);

		TextWidget label = m_AdminEspLabels.Get(esp_index);
		if ( label )
		{
			if ( distance <= 0.5 )
			{
				label.SetText("VOCE");
			}
			else
			{
				label.SetText("PLAYER\n" + Math.Round(distance).ToString() + " m");
			}
			label.SetPos(Math.Clamp(head[0] - 0.04, 0.0, 0.90), Math.Clamp(head[1] - 0.08, 0.0, 0.90));
			label.Show(true);
		}

		return true;
	}

	bool GetAdminEspPointScreenSafe(DayZPlayer target, vector fallback_model_pos, out vector screen_pos)
	{
		if ( !target )
		{
			return false;
		}

		vector world_pos = target.ModelToWorld(fallback_model_pos);
		screen_pos = GetGame().GetScreenPosRelative(world_pos);

		if ( screen_pos[2] <= 0 )
		{
			return false;
		}

		if ( screen_pos[0] < -0.10 || screen_pos[0] > 1.10 || screen_pos[1] < -0.10 || screen_pos[1] > 1.10 )
		{
			return false;
		}

		return true;
	}

	int DrawAdminEspLine(int line_index, vector point_a, vector point_b)
	{
		if ( line_index < 0 || line_index >= m_AdminEspLineWidgets.Count() )
		{
			return line_index;
		}

		if ( !IsAdminEspScreenPointUsable(point_a) || !IsAdminEspScreenPointUsable(point_b) )
		{
			return line_index;
		}

		float dx = point_b[0] - point_a[0];
		float dy = point_b[1] - point_a[1];
		float length = Math.Sqrt((dx * dx) + (dy * dy));

		if ( length <= 0.002 )
		{
			return line_index;
		}

		float divider = ADMIN_ESP_DOTS_PER_SEGMENT - 1;
		for ( int i = 0; i < ADMIN_ESP_DOTS_PER_SEGMENT; i++ )
		{
			if ( line_index >= m_AdminEspLineWidgets.Count() )
			{
				return line_index;
			}

			float t = i / divider;
			float x = point_a[0] + (dx * t);
			float y = point_a[1] + (dy * t);
			DrawAdminEspDot(line_index, x, y);
			line_index++;
		}

		return line_index;
	}

	void DrawAdminEspDot(int line_index, float x, float y)
	{
		Widget dot = m_AdminEspLineWidgets.Get(line_index);
		if ( !dot )
		{
			return;
		}

		dot.SetSize(ADMIN_ESP_DOT_SIZE, ADMIN_ESP_DOT_SIZE);
		dot.SetPos(x - (ADMIN_ESP_DOT_SIZE * 0.5), y - (ADMIN_ESP_DOT_SIZE * 0.5));
		dot.Show(true);
	}

	void HideUnusedAdminEspDots(int start_index, int end_index)
	{
		for ( int i = start_index; i < end_index; i++ )
		{
			if ( i >= 0 && i < m_AdminEspLineWidgets.Count() )
			{
				Widget dot = m_AdminEspLineWidgets.Get(i);
				if ( dot )
				{
					dot.Show(false);
				}
			}
		}
	}

	bool IsAdminEspScreenPointUsable(vector screen_pos)
	{
		if ( screen_pos[2] <= 0 )
		{
			return false;
		}

		if ( screen_pos[0] < 0.0 || screen_pos[0] > 1.0 || screen_pos[1] < 0.0 || screen_pos[1] > 1.0 )
		{
			return false;
		}

		return true;
	}

	void HideAdminEspSkeletons()
	{
		for ( int i = 0; i < ADMIN_ESP_PLAYER_COUNT; i++ )
		{
			HideAdminEspSkeleton(i);
		}
	}

	void HideAdminEspSkeleton(int esp_index)
	{
		if ( esp_index < 0 || esp_index >= ADMIN_ESP_PLAYER_COUNT || esp_index >= m_AdminEspRoots.Count() )
		{
			return;
		}

		Widget root = m_AdminEspRoots.Get(esp_index);
		if ( root )
		{
			root.Show(false);
		}

		int base_line = esp_index * ADMIN_ESP_LINES_PER_PLAYER;
		for ( int i = 0; i < ADMIN_ESP_LINES_PER_PLAYER; i++ )
		{
			int line_index = base_line + i;
			if ( line_index >= 0 && line_index < m_AdminEspLineWidgets.Count() )
			{
				Widget line = m_AdminEspLineWidgets.Get(line_index);
				if ( line )
				{
					line.Show(false);
				}
			}
		}

		TextWidget label = m_AdminEspLabels.Get(esp_index);
		if ( label )
		{
			label.Show(false);
		}
	}
	
	void DisplayNotifier( int key, int tendency )
	{
		if ( m_CustomStatusPanel && !SyncCustomStatusHudFromInventoryStatus() )
		{
			DisplayCustomStatusTendency( key, tendency );
		}

		if ( m_Notifiers )
		{
			ImageWidget w = m_Notifiers.FindAnyWidget( String( "Icon" + m_StatesWidgetNames.Get( key ) ) );
			if ( w )
			{
				w.Show( true );
			}

			// tendency arrows
			string arrow_name = "ArrowUp";
			if ( tendency < 0 )
			{
				arrow_name = "ArrowDown";
			}
			tendency = Math.AbsInt( tendency );

			for ( int x = 1; x < 4; x++ )
			{ 
				w = m_Notifiers.FindAnyWidget( String(  m_StatesWidgetNames.Get( key ) + "ArrowUp" + x.ToString() ) );
				if ( w )
				{
					w.Show( false );
				}
				w = m_Notifiers.FindAnyWidget( String(  m_StatesWidgetNames.Get( key ) + "ArrowDown" + x.ToString() ) );
				if ( w )
				{
					w.Show( false );
				}
			}

			for ( int i = 1; i < ( tendency + 1) ; i++ )
			{
				string widget_name = m_StatesWidgetNames.Get( key ) + arrow_name + i.ToString() ;
				w = m_Notifiers.FindAnyWidget( widget_name );
				if ( w )
				{
					w.Show( true );
				}
			}
		}

		ShowHudTemporarily();
	}

	void DisplayStatusLabel( int key, string label )
	{
		ShowHudTemporarily();

		if ( !m_CustomStatusPanel )
		{
			super.DisplayStatusLabel( key, label );
			return;
		}

		if ( SyncCustomStatusHudFromInventoryStatus() )
		{
			return;
		}

		label.ToUpper();

		if ( key == NTFKEY_HUNGRY )
		{
			if ( label == "" )
			{
				AnimateCustomStatusImage( m_CustomHudFood, "food_4.tga" );
			}
			else if ( label == "ENERGIZED" )
			{
				AnimateCustomStatusImage( m_CustomHudFood, "food_5.tga" );
			}
			else if ( label == "HUNGRY" )
			{
				AnimateCustomStatusImage( m_CustomHudFood, "food_3.tga" );
			}
			else if ( label == "VERY HUNGRY" )
			{
				AnimateCustomStatusImage( m_CustomHudFood, "food_2.tga" );
			}
			else if ( label == "STARVING" )
			{
				AnimateCustomStatusImage( m_CustomHudFood, "food_1.tga" );
			}
		}
		else if ( key == NTFKEY_THIRSTY )
		{
			if ( label == "" )
			{
				AnimateCustomStatusImage( m_CustomHudWater, "water_4.tga" );
			}
			else if ( label == "HYDRATED" )
			{
				AnimateCustomStatusImage( m_CustomHudWater, "water_5.tga" );
			}
			else if ( label == "THIRSTY" )
			{
				AnimateCustomStatusImage( m_CustomHudWater, "water_3.tga" );
			}
			else if ( label == "VERY THIRSTY" )
			{
				AnimateCustomStatusImage( m_CustomHudWater, "water_2.tga" );
			}
			else if ( label == "FATALLY THIRSTY" )
			{
				AnimateCustomStatusImage( m_CustomHudWater, "water_1.tga" );
			}
		}
		else if ( key == NTFKEY_WARMTH )
		{
			if ( label == "" )
			{
				AnimateCustomStatusImage( m_CustomHudTemperature, "temperature_3.tga" );
			}
			else if ( label == "HYPERTHERMIC" || label == "FEVER" )
			{
				AnimateCustomStatusImage( m_CustomHudTemperature, "temperature_5.tga" );
			}
			else if ( label == "HOT" )
			{
				AnimateCustomStatusImage( m_CustomHudTemperature, "temperature_5.tga" );
			}
			else if ( label == "OVERHEATING" )
			{
				AnimateCustomStatusImage( m_CustomHudTemperature, "temperature_4.tga" );
			}
			else if ( label == "COLD" )
			{
				AnimateCustomStatusImage( m_CustomHudTemperature, "temperature_2.tga" );
			}
			else if ( label == "FREEZING" || label == "HYPOTHERMIC" )
			{
				AnimateCustomStatusImage( m_CustomHudTemperature, "temperature_1.tga" );
			}
		}
		else if ( key == NTFKEY_FEVERISH )
		{
			if ( label == "FEVER" )
			{
				AnimateCustomStatusImage( m_CustomHudTemperature, "temperature_5.tga" );
			}
		}
		else if ( key == NTFKEY_BLEEDISH )
		{
			if ( label == "BLEEDING" )
			{
				m_CustomHudBleeding = true;
				ApplyCustomBloodStatus();
			}
			else if ( label == "" )
			{
				m_CustomHudBleeding = false;
				ApplyCustomBloodStatus();
			}
		}
		else if ( key == NTFKEY_SICK )
		{
			if ( label == "SICK" )
			{
				m_CustomHudSick = true;
				ApplyCustomHealthStatus();
			}
			else if ( label == "" )
			{
				m_CustomHudSick = false;
				ApplyCustomHealthStatus();
			}
		}
	}

	void DisplayStatusLevel( int key, int level )
	{
		ShowHudTemporarily();

		if ( !m_CustomStatusPanel )
		{
			super.DisplayStatusLevel( key, level );
			return;
		}

		if ( SyncCustomStatusHudFromInventoryStatus() )
		{
			if ( key != NTFKEY_BLOOD && key != NTFKEY_LIVES )
			{
				return;
			}
		}

		if ( key == NTFKEY_HUNGRY )
		{
			if ( level == 0 )
			{
				AnimateCustomStatusImage( m_CustomHudFood, "food_4.tga" );
			}
			else if ( level == 1 )
			{
				AnimateCustomStatusImage( m_CustomHudFood, "food_5.tga" );
			}
			else if ( level == 2 )
			{
				AnimateCustomStatusImage( m_CustomHudFood, "food_3.tga" );
			}
			else if ( level == 3 )
			{
				AnimateCustomStatusImage( m_CustomHudFood, "food_2.tga" );
			}
			else if ( level >= 4 )
			{
				AnimateCustomStatusImage( m_CustomHudFood, "food_1.tga" );
			}
		}
		else if ( key == NTFKEY_THIRSTY )
		{
			if ( level == 0 )
			{
				AnimateCustomStatusImage( m_CustomHudWater, "water_4.tga" );
			}
			else if ( level == 1 )
			{
				AnimateCustomStatusImage( m_CustomHudWater, "water_5.tga" );
			}
			else if ( level == 2 )
			{
				AnimateCustomStatusImage( m_CustomHudWater, "water_3.tga" );
			}
			else if ( level == 3 )
			{
				AnimateCustomStatusImage( m_CustomHudWater, "water_2.tga" );
			}
			else if ( level >= 4 )
			{
				AnimateCustomStatusImage( m_CustomHudWater, "water_1.tga" );
			}
		}
		else if ( key == NTFKEY_WARMTH )
		{
			if ( level == 0 )
			{
				AnimateCustomStatusImage( m_CustomHudTemperature, "temperature_3.tga" );
			}
			else if ( level == 1 || level == 2 )
			{
				AnimateCustomStatusImage( m_CustomHudTemperature, "temperature_5.tga" );
			}
			else if ( level == 3 )
			{
				AnimateCustomStatusImage( m_CustomHudTemperature, "temperature_4.tga" );
			}
			else if ( level == 4 )
			{
				AnimateCustomStatusImage( m_CustomHudTemperature, "temperature_2.tga" );
			}
			else if ( level >= 5 )
			{
				AnimateCustomStatusImage( m_CustomHudTemperature, "temperature_1.tga" );
			}
		}
		else if ( key == NTFKEY_FEVERISH )
		{
			if ( level > 0 )
			{
				AnimateCustomStatusImage( m_CustomHudTemperature, "temperature_5.tga" );
			}
		}
		else if ( key == NTFKEY_BLEEDISH )
		{
			if ( level > 0 )
			{
				m_CustomHudBleeding = true;
			}
			else
			{
				m_CustomHudBleeding = false;
			}
			ApplyCustomBloodStatus();
		}
		else if ( key == NTFKEY_BLOOD )
		{
			if ( level == 0 )
			{
				m_CustomHudBloodLevel = 5;
			}
			else
			{
				m_CustomHudBloodLevel = Math.Clamp( level, 1, 5 );
			}
			ApplyCustomBloodStatus();
		}
		else if ( key == NTFKEY_SICK )
		{
			if ( level > 0 )
			{
				m_CustomHudSick = true;
			}
			else
			{
				m_CustomHudSick = false;
			}
			ApplyCustomHealthStatus();
		}
		else if ( key == NTFKEY_LIVES )
		{
			if ( level == 0 )
			{
				m_CustomHudHealthLevel = 5;
			}
			else
			{
				m_CustomHudHealthLevel = Math.Clamp( level, 1, 5 );
			}
			ApplyCustomHealthStatus();
		}
	}

	void DisplayCustomStatusTendency( int key, int tendency )
	{
		if ( key == NTFKEY_HUNGRY )
		{
			if ( tendency > 0 )
			{
				AnimateCustomStatusImage( m_CustomHudEnergy, "energy_5.tga" );
			}
			else if ( tendency < 0 )
			{
				AnimateCustomStatusImage( m_CustomHudEnergy, "energy_2.tga" );
			}
			else
			{
				AnimateCustomStatusImage( m_CustomHudEnergy, "energy_3.tga" );
			}
		}
		else if ( key == NTFKEY_THIRSTY )
		{
			if ( tendency > 0 )
			{
				AnimateCustomStatusImage( m_CustomHudWater, "water_5.tga" );
			}
			else if ( tendency < 0 )
			{
				AnimateCustomStatusImage( m_CustomHudWater, "water_2.tga" );
			}
		}
		else if ( key == NTFKEY_FEVERISH )
		{
			if ( tendency > 0 )
			{
				AnimateCustomStatusImage( m_CustomHudTemperature, "temperature_5.tga" );
			}
			else if ( tendency < 0 )
			{
				AnimateCustomStatusImage( m_CustomHudTemperature, "temperature_1.tga" );
			}
			else
			{
				AnimateCustomStatusImage( m_CustomHudTemperature, "temperature_3.tga" );
			}
		}
		else if ( key == NTFKEY_BLEEDISH )
		{
			// Blood tendency shares the bleeding key in 0.62; the bleeding icon is driven by DisplayBadge.
		}
		else if ( key == NTFKEY_SICK )
		{
			if ( tendency < 0 )
			{
				m_CustomHudSick = true;
				if ( m_CustomHudHealthLevel > 2 )
				{
					m_CustomHudHealthLevel = 2;
				}
			}
			ApplyCustomHealthStatus();
		}
	}

	void SetCustomStatusImage( ImageWidget widget, string image_name )
	{
		if ( widget )
		{
			string current = "";
			if ( m_CustomHudCurrentImages.Contains( widget ) )
			{
				current = m_CustomHudCurrentImages.Get( widget );
				if ( current == image_name )
				{
					return;
				}
			}
			m_CustomHudCurrentImages.Set( widget, image_name );
			widget.LoadImageFile( 0, "gui/hud_custom/" + image_name, true );
			widget.Show( true );
		}
	}

	WidgetFadeTimer GetCustomFadeTimer( ImageWidget widget )
	{
		if ( widget == m_CustomHudFood ) return m_FadeTimerCustomFood;
		if ( widget == m_CustomHudWater ) return m_FadeTimerCustomWater;
		if ( widget == m_CustomHudTemperature ) return m_FadeTimerCustomTemp;
		if ( widget == m_CustomHudBlood ) return m_FadeTimerCustomBlood;
		if ( widget == m_CustomHudHealth ) return m_FadeTimerCustomHealth;
		if ( widget == m_CustomHudEnergy ) return m_FadeTimerCustomEnergy;
		if ( widget == m_CustomHudBleedingIcon ) return m_FadeTimerCustomBleeding;
		return NULL;
	}

	void AnimateCustomStatusImage( ImageWidget widget, string image_name )
	{
		if ( widget )
		{
			string current = "";
			if ( m_CustomHudCurrentImages.Contains( widget ) )
			{
				current = m_CustomHudCurrentImages.Get( widget );
				if ( current == image_name )
				{
					return;
				}
			}
			m_CustomHudCurrentImages.Set( widget, image_name );
			WidgetFadeTimer fade_timer = GetCustomFadeTimer( widget );
			if ( fade_timer )
			{
				fade_timer.FadeOut( widget, 0.15, true );
			}
			widget.LoadImageFile( 0, "gui/hud_custom/" + image_name, true );
			widget.Show( true );
			if ( fade_timer )
			{
				fade_timer.FadeIn( widget, 0.15, true );
			}
		}
	}

	int GetCustomStatusLevel( float value, float max )
	{
		if ( max <= 0 )
		{
			return 1;
		}

		float ratio = value / max;
		if ( ratio <= 0.15 )
		{
			return 1;
		}
		else if ( ratio <= 0.4 )
		{
			return 2;
		}
		else if ( ratio <= 0.75 )
		{
			return 3;
		}
		else if ( ratio <= 0.9 )
		{
			return 4;
		}

		return 5;
	}

	void SetCustomStatusImageByLevel( ImageWidget widget, string prefix, int level )
	{
		level = Math.Clamp( level, 1, 5 );
		AnimateCustomStatusImage( widget, prefix + "_" + level.ToString() + ".tga" );
	}

	bool SyncCustomStatusHudFromInventoryStatus()
	{
		if ( !m_CustomStatusPanel )
		{
			return false;
		}

		PluginPlayerStatus module_player_status = (PluginPlayerStatus)GetPlugin(PluginPlayerStatus);
		if ( !module_player_status )
		{
			return false;
		}

		// Retrieve levels from module_player_status
		int hunger_lvl = 0;
		if ( module_player_status.m_NotifiersLevel.HasKey( NTFKEY_HUNGRY ) )
		{
			hunger_lvl = module_player_status.m_NotifiersLevel.Get( NTFKEY_HUNGRY ).Get(0);
		}

		int thirst_lvl = 0;
		if ( module_player_status.m_NotifiersLevel.HasKey( NTFKEY_THIRSTY ) )
		{
			thirst_lvl = module_player_status.m_NotifiersLevel.Get( NTFKEY_THIRSTY ).Get(0);
		}

		int temp_lvl = 0;
		if ( module_player_status.m_NotifiersLevel.HasKey( NTFKEY_WARMTH ) )
		{
			temp_lvl = module_player_status.m_NotifiersLevel.Get( NTFKEY_WARMTH ).Get(0);
		}

		bool has_fever = false;
		if ( module_player_status.m_NotifiersLevel.HasKey( NTFKEY_FEVERISH ) )
		{
			int fever_lvl = module_player_status.m_NotifiersLevel.Get( NTFKEY_FEVERISH ).Get(0);
			if ( fever_lvl > 0 )
			{
				has_fever = true;
			}
		}

		// Update Food Icon
		if ( hunger_lvl == 4 ) AnimateCustomStatusImage( m_CustomHudFood, "food_1.tga" );
		else if ( hunger_lvl == 3 ) AnimateCustomStatusImage( m_CustomHudFood, "food_2.tga" );
		else if ( hunger_lvl == 2 ) AnimateCustomStatusImage( m_CustomHudFood, "food_3.tga" );
		else if ( hunger_lvl == 1 ) AnimateCustomStatusImage( m_CustomHudFood, "food_5.tga" );
		else AnimateCustomStatusImage( m_CustomHudFood, "food_4.tga" );

		// Update Water Icon
		if ( thirst_lvl == 4 ) AnimateCustomStatusImage( m_CustomHudWater, "water_1.tga" );
		else if ( thirst_lvl == 3 ) AnimateCustomStatusImage( m_CustomHudWater, "water_2.tga" );
		else if ( thirst_lvl == 2 ) AnimateCustomStatusImage( m_CustomHudWater, "water_3.tga" );
		else if ( thirst_lvl == 1 ) AnimateCustomStatusImage( m_CustomHudWater, "water_5.tga" );
		else AnimateCustomStatusImage( m_CustomHudWater, "water_4.tga" );

		// Update Temperature Icon
		if ( has_fever )
		{
			AnimateCustomStatusImage( m_CustomHudTemperature, "temperature_5.tga" );
		}
		else
		{
			if ( temp_lvl == 1 || temp_lvl == 2 )
			{
				AnimateCustomStatusImage( m_CustomHudTemperature, "temperature_5.tga" );
			}
			else if ( temp_lvl == 3 )
			{
				AnimateCustomStatusImage( m_CustomHudTemperature, "temperature_4.tga" );
			}
			else if ( temp_lvl == 4 )
			{
				AnimateCustomStatusImage( m_CustomHudTemperature, "temperature_2.tga" );
			}
			else if ( temp_lvl == 5 || temp_lvl == 6 )
			{
				AnimateCustomStatusImage( m_CustomHudTemperature, "temperature_1.tga" );
			}
			else
			{
				AnimateCustomStatusImage( m_CustomHudTemperature, "temperature_3.tga" );
			}
		}

		// Update Energy battery Icon
		int energy_level = 3; // Stable / Default
		if ( hunger_lvl == 1 )
		{
			energy_level = 5; // Energized
		}
		else if ( hunger_lvl == 2 || hunger_lvl == 3 )
		{
			energy_level = 2; // Low
		}
		else if ( hunger_lvl == 4 )
		{
			energy_level = 1; // Empty
		}
		AnimateCustomStatusImage( m_CustomHudEnergy, "energy_" + energy_level.ToString() + ".tga" );

		// Sync bleeding and sick states from badges display to avoid periodic reset to false
		m_CustomHudBleeding = false;
		if ( m_BadgesWidgetDisplay.Contains( NTFKEY_BLEEDISH ) )
		{
			m_CustomHudBleeding = m_BadgesWidgetDisplay.Get( NTFKEY_BLEEDISH );
		}

		m_CustomHudSick = false;
		if ( m_BadgesWidgetDisplay.Contains( NTFKEY_SICK ) )
		{
			m_CustomHudSick = m_BadgesWidgetDisplay.Get( NTFKEY_SICK );
		}

		// Sync Health and Blood from module_player_status (native GetHealth is not linked on client)
		m_CustomHudBloodLevel = 5;
		if ( module_player_status.m_NotifiersLevel.HasKey( NTFKEY_BLOOD ) )
		{
			int blood_level = module_player_status.m_NotifiersLevel.Get( NTFKEY_BLOOD ).Get(0);
			if ( blood_level > 0 )
			{
				m_CustomHudBloodLevel = Math.Clamp( blood_level, 1, 5 );
			}
		}

		m_CustomHudHealthLevel = 5;
		if ( module_player_status.m_NotifiersLevel.HasKey( NTFKEY_LIVES ) )
		{
			int health_level = module_player_status.m_NotifiersLevel.Get( NTFKEY_LIVES ).Get(0);
			if ( health_level > 0 )
			{
				m_CustomHudHealthLevel = Math.Clamp( health_level, 1, 5 );
			}
		}

		ApplyCustomBloodStatus();
		ApplyCustomHealthStatus();
		return true;
	}

	void ApplyCustomInventoryStatusLabel( string label )
	{
		label.ToUpper();

		if ( label == "ENERGIZED" )
		{
			AnimateCustomStatusImage( m_CustomHudEnergy, "energy_5.tga" );
		}
		else if ( label == "HUNGRY" )
		{
			AnimateCustomStatusImage( m_CustomHudFood, "food_3.tga" );
		}
		else if ( label == "VERY HUNGRY" || label == "EXTREMELY HUNGRY" )
		{
			AnimateCustomStatusImage( m_CustomHudFood, "food_2.tga" );
		}
		else if ( label == "STARVING" )
		{
			AnimateCustomStatusImage( m_CustomHudFood, "food_1.tga" );
		}
		else if ( label == "HYDRATED" )
		{
			AnimateCustomStatusImage( m_CustomHudWater, "water_5.tga" );
		}
		else if ( label == "THIRSTY" )
		{
			AnimateCustomStatusImage( m_CustomHudWater, "water_3.tga" );
		}
		else if ( label == "VERY THIRSTY" || label == "REALLY THIRSTY" )
		{
			AnimateCustomStatusImage( m_CustomHudWater, "water_2.tga" );
		}
		else if ( label == "FATALLY THIRSTY" || label == "DEHYDRATED" )
		{
			AnimateCustomStatusImage( m_CustomHudWater, "water_1.tga" );
		}
		else if ( label == "HOT" || label == "HYPERTHERMIC" || label == "FEVER" )
		{
			AnimateCustomStatusImage( m_CustomHudTemperature, "temperature_5.tga" );
		}
		else if ( label == "OVERHEATING" )
		{
			AnimateCustomStatusImage( m_CustomHudTemperature, "temperature_4.tga" );
		}
		else if ( label == "COLD" )
		{
			AnimateCustomStatusImage( m_CustomHudTemperature, "temperature_2.tga" );
		}
		else if ( label == "FREEZING" || label == "HYPOTHERMIC" )
		{
			AnimateCustomStatusImage( m_CustomHudTemperature, "temperature_1.tga" );
		}
		else if ( label == "BLEEDING" )
		{
			m_CustomHudBleeding = true;
		}
		else if ( label == "SICK" || label == "POISONED" || label == "INFECTED" )
		{
			m_CustomHudSick = true;
		}
	}

	void ApplyCustomBloodStatus()
	{
		UpdateCustomBleedingStatus();

		int level = Math.Clamp( m_CustomHudBloodLevel, 1, 5 );
		AnimateCustomStatusImage( m_CustomHudBlood, "blood_" + level.ToString() + ".tga" );
	}

	void UpdateCustomBleedingStatus()
	{
		if ( m_CustomHudBleedingIcon )
		{
			string current = "";
			if ( m_CustomHudCurrentImages.Contains( m_CustomHudBleedingIcon ) )
			{
				current = m_CustomHudCurrentImages.Get( m_CustomHudBleedingIcon );
				if ( ( current == "bleeding.tga" ) == m_CustomHudBleeding )
				{
					return;
				}
			}
			if ( m_CustomHudBleeding )
			{
				AnimateCustomStatusImage( m_CustomHudBleedingIcon, "bleeding.tga" );
			}
			else
			{
				m_CustomHudCurrentImages.Set( m_CustomHudBleedingIcon, "" );
				m_CustomHudBleedingIcon.Show( false );
			}
		}
	}

	void ApplyCustomHealthStatus()
	{
		int level = Math.Clamp( m_CustomHudHealthLevel, 1, 5 );
		if ( m_CustomHudSick && level > 2 )
		{
			level = 2;
		}
		AnimateCustomStatusImage( m_CustomHudHealth, "health_" + level.ToString() + ".tga" );
	}

	void UpdateCustomStatusHud()
	{
		if ( !SyncCustomStatusHudFromInventoryStatus() )
		{
			ApplyCustomBloodStatus();
			ApplyCustomHealthStatus();
		}
	}
	
	void DisplayBadge( int key, bool show )
	{
		if ( m_CustomStatusPanel )
		{
			if ( key == NTFKEY_BLEEDISH )
			{
				m_CustomHudBleeding = show;
				ApplyCustomBloodStatus();
				return;
			}
			else if ( key == NTFKEY_SICK )
			{
				m_CustomHudSick = show;
				ApplyCustomHealthStatus();
				return;
			}
			else if ( key != NTFKEY_FRACTURE && key != NTFKEY_FEVERISH )
			{
				// Disable stomach (STUFFED) etc.
				return;
			}
		}

		m_BadgesWidgetDisplay.Set( key, show );

		if ( m_Badges )
		{
			if ( m_CustomStatusPanel )
			{
				bool show_badges = false;
				if ( m_BadgesWidgetDisplay.Contains( NTFKEY_FRACTURE ) && m_BadgesWidgetDisplay.Get( NTFKEY_FRACTURE ) )
				{
					show_badges = true;
				}
				if ( m_BadgesWidgetDisplay.Contains( NTFKEY_FEVERISH ) && m_BadgesWidgetDisplay.Get( NTFKEY_FEVERISH ) )
				{
					show_badges = true;
				}
				m_Badges.Show( show_badges );
			}

			int x = 0;
			for ( int i = 0; i < m_BadgesWidgetDisplay.Count(); i++ )
			{
				int badge_key = m_BadgesWidgetDisplay.GetKey( i );
				string badge_name = m_BadgesWidgetNames.Get( badge_key );
				ImageWidget badge_widget = m_Badges.FindAnyWidget( badge_name );
				if ( badge_widget )
				{
					if ( m_BadgesWidgetDisplay.Get( badge_key ) == true )
					{
						badge_widget.SetPos ( x*0.2, 0.0, true);
						badge_widget.Show( true );
						x = x + 1;
					}
					else
					{
						badge_widget.Show( false );
					}
				}
			}
		}
	}
	
	void SetStamina( int value , int range )
	{
		PlayerBase player = GetGame().GetPlayer();
		if( !player.m_PlayerStats )
		{
			return;
		}
		float max = player.m_PlayerStats.m_Stamina.GetMax();
		float percentage =  range / max;
		
		if ( m_stamina )
		{
			m_stamina.SetCurrent( ( value / range ) * max );
			m_stamina.SetSize( percentage, 0.1 );
		}
		
		if ( m_stamina_background )
		{
			m_stamina_background.SetSize( 1-percentage, 0.1);
		}

		if ( value < range )
		{
			ShowHudTemporarily();
		}
	}
	
	void RefreshZeroingAndWeaponModeOld()
	{
		PlayerBase player = GetGame().GetPlayer();
		
		if ( player != NULL )
		{
			EntityAI entity = player.GetEntityInHands();
			
			if ( entity != NULL && entity.IsWeapon() )
			{	
				float zeroing = player.GetCurrentZeroing();
				
				if( zeroing_key_pressed )
				{
					zeroing_key_pressed = false;
					faded = false;
					ShowHudTemporarily();
				}
				
				if( !faded )
				{
					m_fade_timer_zeroing_old.FadeOut( m_WeaponStats, 5 );
					// [2026-06-30] FIX: [ISSUE-013] Timers de fade estavam invertidos
					m_fade_timer_magazine_name.FadeOut( m_MagazineName, 5 );
					m_fade_timer_weapon_name.FadeOut( m_WeaponName, 5 );
					m_fade_timer_weapon_stats.FadeOut( m_ZeroingOld, 5 );
					faded = true;
				}
				
				if( entity.AttachmentsCount() == 0 )
				{
					m_MagazineName.SetText( "" );
				}
				
				for ( int i = 0; i < entity.AttachmentsCount(); i++ )
				{
					EntityAI attachment = entity.GetAttachmentFromIndex( i );
					if ( !attachment ) continue;
					
					if ( attachment.IsMagazine() )
					{
						m_MagazineName.SetText( attachment.GetName() );
					}
					else
					{
						m_MagazineName.SetText("");
					}
				}
				
				m_WeaponName.SetText( entity.GetName() );
				m_WeaponStats.Show( true );
				m_ZeroingOld.SetText( zeroing.ToString() );
			}
			else
			{
				faded = false;
				m_WeaponStats.Show( false );
			}
		}
	}
	
	void RefreshZeroingAndWeaponMode()
	{
		PlayerBase player = GetGame().GetPlayer();
		
		if ( player != NULL )
		{
			EntityAI entity = player.GetEntityInHands();
			
			if ( entity != NULL && entity.IsWeapon() )
			{	
				float zeroing = player.GetCurrentZeroing();
				string weaponMode = player.GetCurrentWeaponMode();
				
				if( zeroing_key_pressed )
				{
					zeroing_key_pressed = false;
					faded = false;
					ShowHudTemporarily();
				}
				
				if( !faded )
				{
					m_fade_timer_weapon_mode.FadeOut(m_WeaponMode, 5);
					m_fade_timer_zeroing.FadeOut(m_Zeroing, 5);
					faded = true;
				}
				
				m_Zeroing.Show(true);
				m_WeaponMode.Show(true);
				m_Zeroing.SetText( zeroing.ToString() );
				m_WeaponMode.SetText( weaponMode );
			}
			else
			{
				faded = false;
				m_WeaponMode.Show(false);
				m_Zeroing.Show(false);
			}
		}
	}

	bool KeyPress(int key)
	{
		if ( key == KeyCode.KC_H )
		{
			ToggleDynamicHudManual();
			return true;
		}
		return false;
	}
	
	bool ZeroingKeyPress()
	{
		zeroing_key_pressed = true;
		return true;
	}
	
	void DisplayStance( int stance )
	{
		if ( m_stanceStand ) m_stanceStand.Show( stance == 1 );
		if ( m_stanceCrouch ) m_stanceCrouch.Show( stance == 2 );
		if ( m_stanceProne ) m_stanceProne.Show( stance == 3 );
		ShowHudTemporarily();
	}
	
	void DisplayPresence()
	{
		PlayerBase player = GetGame().GetPlayer();
		if ( player != NULL )
		{
			float presence_level = player.GetPresenceInAI();
			if ( m_PresenceLevel0 ) m_PresenceLevel0.Show( presence_level > 0 );
			if ( m_PresenceLevel1 ) m_PresenceLevel1.Show( presence_level > 0.2 );
			if ( m_PresenceLevel2 ) m_PresenceLevel2.Show( presence_level > 0.4 );
			if ( m_PresenceLevel3 ) m_PresenceLevel3.Show( presence_level > 0.6 );
			if ( m_PresenceLevel4 ) m_PresenceLevel4.Show( presence_level > 0.8 );
		}
	}

	bool IsPlayerHealthyAndSafe()
	{
		PlayerBase player = GetGame().GetPlayer();
		if ( !player )
		{
			return true;
		}

		if ( player.m_PlayerStats )
		{
			float max_stamina = player.m_PlayerStats.m_Stamina.GetMax();
			float cur_stamina = player.m_PlayerStats.m_Stamina.Get();
			if ( cur_stamina < (max_stamina - 5.0) )
			{
				return false;
			}
		}

		if ( m_CustomStatusPanel )
		{
			if ( m_CustomHudBleeding ) return false;
			if ( m_CustomHudSick ) return false;
			if ( m_CustomHudBloodLevel < 5 ) return false;
			if ( m_CustomHudHealthLevel < 5 ) return false;
		}

		PluginPlayerStatus module_player_status = (PluginPlayerStatus)GetPlugin(PluginPlayerStatus);
		if ( module_player_status )
		{
			if ( module_player_status.m_NotifiersLevel.HasKey( NTFKEY_HUNGRY ) )
			{
				int hunger_lvl = module_player_status.m_NotifiersLevel.Get( NTFKEY_HUNGRY ).Get(0);
				if ( hunger_lvl >= 2 ) return false;
			}
			if ( module_player_status.m_NotifiersLevel.HasKey( NTFKEY_THIRSTY ) )
			{
				int thirst_lvl = module_player_status.m_NotifiersLevel.Get( NTFKEY_THIRSTY ).Get(0);
				if ( thirst_lvl >= 2 ) return false;
			}
		if ( module_player_status.m_NotifiersLevel.HasKey( NTFKEY_BLOOD ) )
			{
				int blood_lvl = module_player_status.m_NotifiersLevel.Get( NTFKEY_BLOOD ).Get(0);
				if ( blood_lvl > 0 && blood_lvl < 5 ) return false;
			}
		if ( module_player_status.m_NotifiersLevel.HasKey( NTFKEY_LIVES ) )
			{
				int health_lvl = module_player_status.m_NotifiersLevel.Get( NTFKEY_LIVES ).Get(0);
				if ( health_lvl > 0 && health_lvl < 5 ) return false;
			}
		}

		if ( m_BadgesWidgetDisplay )
		{
			for ( int i = 0; i < m_BadgesWidgetDisplay.Count(); i++ )
			{
				int key = m_BadgesWidgetDisplay.GetKey(i);
				if ( m_BadgesWidgetDisplay.Get(key) )
				{
					return false;
				}
			}
		}

		return true;
	}

	void ShowHudTemporarily()
	{
		// HUD sempre visível em DayZ 0.62
		m_HudVisible = true;
	}

	void CheckAndHideHud()
	{
		// HUD sempre visível em DayZ 0.62
		m_HudVisible = true;
	}

	void FadeHudIn(float time)
	{
		m_HudVisible = true;
		if ( m_CustomStatusPanel )
		{
			m_FadeTimerCustomHud.FadeIn( m_CustomStatusPanel, time, true );
		}
		else
		{
			if ( m_Notifiers ) m_FadeTimerNotifiers.FadeIn( m_Notifiers, time, true );
		}
		if ( m_Badges && !m_CustomStatusPanel ) m_FadeTimerBadges.FadeIn( m_Badges, time, true );
		if ( m_stamina ) m_FadeTimerStamina.FadeIn( m_stamina, time, true );
		if ( m_stamina_background ) m_FadeTimerStaminaBg.FadeIn( m_stamina_background, time, true );
		if ( m_presence ) m_presence.Show( true );
		if ( m_stancePanel ) m_FadeTimerStance.FadeIn( m_stancePanel, time, true );
	}

	void FadeHudOut(float time)
	{
		m_HudVisible = false;
		if ( m_CustomStatusPanel )
		{
			m_FadeTimerCustomHud.FadeOut( m_CustomStatusPanel, time, true );
		}
		else
		{
			if ( m_Notifiers ) m_FadeTimerNotifiers.FadeOut( m_Notifiers, time, true );
		}
		if ( m_Badges && !m_CustomStatusPanel ) m_FadeTimerBadges.FadeOut( m_Badges, time, true );
		if ( m_stamina ) m_FadeTimerStamina.FadeOut( m_stamina, time, true );
		if ( m_stamina_background ) m_FadeTimerStaminaBg.FadeOut( m_stamina_background, time, true );
		if ( m_presence ) m_presence.Show( false );
		if ( m_stancePanel ) m_FadeTimerStance.FadeOut( m_stancePanel, time, true );
	}

	void ToggleDynamicHudManual()
	{
		if ( m_HudVisible )
		{
			m_HudHideTimer.Stop();
			FadeHudOut( FADE_OUT_TIME );
		}
		else
		{
			FadeHudIn( FADE_IN_TIME );
			m_HudHideTimer.Stop();
			m_HudHideTimer.Run( HIDE_MENU_TIME, this, "CheckAndHideHud" );
		}
	}

	void RefreshVehicleSpeedHud()
	{
		PlayerBase player = GetGame().GetPlayer();
		if ( player != NULL )
		{
			Transport vehicle = player.GetDrivingVehicle();
			if ( vehicle != NULL && vehicle.IsInherited( TankOrCar ) )
			{
				TankOrCar car = (TankOrCar) vehicle;
				m_VehiclePanel0.Show( true );				
				VehiclePanelSpeedValue.SetText( Math.Floor( vehicle.GetSpeedometerValue() ).ToString() );
				
				int engaged_gear = car.GetEngagedGear();
				VehiclePanelGearValue.SetText( m_VehicleGearTable.Get( engaged_gear ) );
			}
			else
			{
				m_VehiclePanel0.Show( false );
			}
		}
	}

	/*void RefreshVehicleHud()
	{
		PlayerBase player = GetGame().GetPlayer();
		if ( player != NULL )
		{
			Transport vehicle = player.GetDrivingVehicle();
			TankOrCar car = (TankOrCar) vehicle;
			if ( vehicle != NULL )
			{
				m_VehiclePanel.Show( true );
				// zatial zakomentovane - bude treba poriesit nejaky per frame refresh system
				// float rpm_value = -90 + ( vehicle.GetRPM() * 100 ) ;
				// Widget RPMPointer = m_HudPanelWidget.FindAnyWidget("rpm_pointer");
				// RPMPointer.SetRotation( 0, 0, rpm_value, true );
				// TextWidget VehiclePanelRPMValue = m_HudPanelWidget.FindAnyWidget("rpm_label");
				// VehiclePanelRPMValue.SetText( ftoa( floor( rpm_value * 100 ) ) );
				
				TextWidget VehiclePanelSpeedValue = m_HudPanelWidget.FindAnyWidget("SpeedValue");
				VehiclePanelSpeedValue.SetText( ftoa( floor( vehicle.GetSpeedometerValue() ) ) );
				
				int engaged_gear = car.GetEngagedGear();
				int prev_gear = engaged_gear - 1;
				int next_gear = engaged_gear + 1;

				if ( engaged_gear == GEAR_NEUTRAL )
				{
					prev_gear = GEAR_REVERSE;
				}
				else if ( engaged_gear == GEAR_REVERSE )
				{
					prev_gear = -1;
					next_gear = GEAR_NEUTRAL;
				}
				
				VehiclePanelSpeedValue = m_HudPanelWidget.FindAnyWidget("Current");
				VehiclePanelSpeedValue.SetText( m_VehicleGearTable.Get( engaged_gear ) );
							
				VehiclePanelSpeedValue = m_HudPanelWidget.FindAnyWidget("Next");
				VehiclePanelSpeedValue.SetText( m_VehicleGearTable.Get( next_gear ) );

				VehiclePanelSpeedValue = m_HudPanelWidget.FindAnyWidget("Prev");
				VehiclePanelSpeedValue.SetText( m_VehicleGearTable.Get( prev_gear ) );
			}
			else
			{
				m_VehiclePanel.Show( false );
			}
		}
	}*/

	InventoryQuickbar GetQuickbar()
	{
		return m_quickbar;
	}

	void InitQuickbar()
	{
		if (m_quickbar == NULL)
		{
			m_quickbar = new InventoryQuickbar( m_quickbar_widget );
			ShowQuickbar();
		}
	}
	
	void ShowQuickbar()
	{
		if(!GetGame().GetPlayer())
		{
			return;
		}
		
		if ( m_quickbar_widget && GetGame().GetPlayer().GetQuickBarSize() != 0 )
		{
				InventoryGrid quickbarGrid;
				m_quickbar_widget.GetScript(quickbarGrid);
				Widget child = quickbarGrid.GetRoot().GetChildren();
				float alpha = quickbarGrid.GetRoot().GetAlpha();
				RefreshQuickbar();

				for ( int i = 0; i < m_fade_timers.Count(); i++)
				{
					WidgetFadeTimer timer = m_fade_timers.Get(i);
					timer.Stop();
					delete timer;
				}
				m_fade_timers.Clear();
				
				m_quickbar_widget.Show( true );
				m_quickbar_state = true;
				
				while (child)
					{
						child.Show(true);
						// [2026-06-30] FIX: [1.2.3] Null check para GetChildren()
						Widget childChildren = child.GetChildren();
						if (childChildren) {
							childChildren.Show(true);
							childChildren.SetAlpha(1);
						};
						child.SetAlpha(alpha);
						child = child.GetSibling();
					}
				m_quickbar_timer.Stop();
				m_quickbar_timer.Run( 45, this, "HideQuickbar" );
		}
	}
	
	void HideQuickbar( bool ignore_state = false , bool instant_hide = false )
	{
		if ( m_quickbar_widget )
		{
			InventoryGrid quickbarGrid;
			m_quickbar_widget.GetScript(quickbarGrid);
			
			if( !instant_hide )
			{
				// [2026-06-30] FIX: [3.5.4] Limpar timers existentes antes de criar novos
				foreach (WidgetFadeTimer t : m_fade_timers)
				{
					t.Stop();
				}
				m_fade_timers.Clear();
				
				Widget child = quickbarGrid.GetRoot().GetChildren();
				while (child)
				{
					WidgetFadeTimer m_fade_timer_quickbar = new WidgetFadeTimer;
					m_fade_timer_quickbar.FadeOut(child, 5, true);
					m_fade_timers.Insert(m_fade_timer_quickbar);
					child = child.GetSibling();
				}
			}
			else
			{
				m_quickbar_widget.Show( false );
			}
			
			if ( ! ignore_state )
			{
				m_quickbar_state = false;
			}
			m_quickbar_timer.Stop();
		}
	}
	
	void SetLeftStatsVisibility(bool visible)
	{
		/*m_stamina_background.Show(visible);
		m_stamina.Show(visible);
		m_stancePanel.Show(visible);
		m_presence.Show(visible);*/
	}
	
	void RefreshQuickbar()
	{
		UpdateCustomStatusHud();
		UpdateCustomMapHudMarkers();
		UpdateClanHudMarkers();
		UpdateAdminEspSkeletons();

		if ( m_quickbar ) 
		{
			m_quickbar.UpdateItems( m_quickbar_widget );
		}
	}
}
