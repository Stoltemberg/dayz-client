class SpawnSelectorMenu extends UIScriptedMenu
{
	protected Widget m_Root;
	protected Widget m_MainPanel;
	protected Widget m_MapPanel;
	protected TextWidget m_StatusText;
	protected TextWidget m_SelectedText;
	protected ButtonWidget m_SpawnButton0;
	protected ButtonWidget m_SpawnButton1;
	protected ButtonWidget m_SpawnButton2;
	protected ButtonWidget m_SpawnButton3;
	protected ButtonWidget m_SpawnButton4;
	protected ButtonWidget m_SpawnButton5;
	protected ButtonWidget m_SpawnButton6;
	protected ButtonWidget m_SpawnButton7;
	protected ButtonWidget m_UseButton;
	protected ButtonWidget m_RandomButton;
	protected ButtonWidget m_CloseButton;

	protected autoptr TStringArray m_SpawnNames;
	protected int m_LastStateVersion;
	protected int m_SelectedSpawnIndex;

	void SpawnSelectorMenu()
	{
		m_SpawnNames = new TStringArray;
		m_LastStateVersion = -1;
		m_SelectedSpawnIndex = 0;
	}

	Widget Init()
	{
		layoutRoot = GetGame().GetWorkspace().CreateWidgets("gui/layouts/day_z_spawn_selector.layout");
		if ( !layoutRoot )
		{
			return NULL;
		}

		m_Root = layoutRoot.FindAnyWidget("SpawnSelectorRoot");
		m_MainPanel = layoutRoot.FindAnyWidget("SpawnSelectorMainPanel");
		m_MapPanel = layoutRoot.FindAnyWidget("SpawnSelectorMapPanel");
		m_StatusText = (TextWidget)layoutRoot.FindAnyWidget("SpawnSelectorStatus");
		m_SelectedText = (TextWidget)layoutRoot.FindAnyWidget("SpawnSelectorSelected");
		m_SpawnButton0 = (ButtonWidget)layoutRoot.FindAnyWidget("SpawnSelectorCity0");
		m_SpawnButton1 = (ButtonWidget)layoutRoot.FindAnyWidget("SpawnSelectorCity1");
		m_SpawnButton2 = (ButtonWidget)layoutRoot.FindAnyWidget("SpawnSelectorCity2");
		m_SpawnButton3 = (ButtonWidget)layoutRoot.FindAnyWidget("SpawnSelectorCity3");
		m_SpawnButton4 = (ButtonWidget)layoutRoot.FindAnyWidget("SpawnSelectorCity4");
		m_SpawnButton5 = (ButtonWidget)layoutRoot.FindAnyWidget("SpawnSelectorCity5");
		m_SpawnButton6 = (ButtonWidget)layoutRoot.FindAnyWidget("SpawnSelectorCity6");
		m_SpawnButton7 = (ButtonWidget)layoutRoot.FindAnyWidget("SpawnSelectorCity7");
		m_UseButton = (ButtonWidget)layoutRoot.FindAnyWidget("SpawnSelectorUse");
		m_RandomButton = (ButtonWidget)layoutRoot.FindAnyWidget("SpawnSelectorRandom");
		m_CloseButton = (ButtonWidget)layoutRoot.FindAnyWidget("SpawnSelectorClose");

		ApplyStyle();
		Render();
		return layoutRoot;
	}

	void Update(float timeslice)
	{
		PluginSpawnSelector selector = PluginSpawnSelector.GetInstance();
		if ( selector && selector.GetStateVersion() != m_LastStateVersion )
		{
			Render();
		}
	}

	bool OnClick(Widget w, int x, int y, int button)
	{
		super.OnClick(w, x, y, button);

		PluginSpawnSelector selector = PluginSpawnSelector.GetInstance();
		if ( !selector )
		{
			return false;
		}

		int city_index = GetSpawnButtonIndex(w);
		if ( city_index >= 0 && city_index < m_SpawnNames.Count() )
		{
			m_SelectedSpawnIndex = city_index;
			UpdateSelectedText();
			UpdateSpawnButtonVisuals();
			return true;
		}

		if ( w == m_CloseButton )
		{
			CloseSelectorMenu();
			return true;
		}

		if ( w == m_UseButton )
		{
			string spawn_name = GetSelectedSpawnName();
			if ( spawn_name.Length() == 0 )
			{
				SetStatus("Selecione uma cidade.");
				return true;
			}

			selector.RequestTeleport(spawn_name);
			SetStatus("Solicitando spawn em " + spawn_name + "...");
			CloseSelectorMenu();
			return true;
		}

		if ( w == m_RandomButton )
		{
			if ( m_SpawnNames.Count() == 0 )
			{
				SetStatus("Lista de spawns vazia.");
				return true;
			}

			int random_index = Math.RandomInt(0, m_SpawnNames.Count() - 1);
			string random_spawn = m_SpawnNames.Get(random_index);
			selector.RequestTeleport(random_spawn);
			SetStatus("Solicitando spawn aleatorio...");
			CloseSelectorMenu();
			return true;
		}

		return false;
	}

	bool OnKeyPress(Widget w, int x, int y, int key)
	{
		super.OnKeyPress(w, x, y, key);

		if ( key == KeyCode.KC_ESCAPE )
		{
			CloseSelectorMenu();
			return true;
		}

		return false;
	}

	protected void ApplyStyle()
	{
		ForcePanel(m_Root, 0xB0000000);
		ForcePanel(m_MainPanel, 0xD6000000);
		ForcePanel(m_MapPanel, 0x55132232);
		SetButtonVisual(m_UseButton, 0xDDD01414);
		SetButtonVisual(m_RandomButton, 0xDDD01414);
		SetButtonVisual(m_CloseButton, 0xCC455A64);
		UpdateSpawnButtonVisuals();
	}

	protected void ForcePanel(Widget widget, int color)
	{
		if ( widget )
		{
			widget.SetColor(color);
			widget.SetAlpha(1.0);
		}
	}

	protected void SetButtonVisual(ButtonWidget button, int color)
	{
		if ( button )
		{
			button.Enable(true);
			button.SetColor(color);
		}
	}

	protected void Render()
	{
		PluginSpawnSelector selector = PluginSpawnSelector.GetInstance();
		if ( !selector )
		{
			return;
		}

		m_LastStateVersion = selector.GetStateVersion();
		SetStatus(selector.GetLocalMessage());
		RenderSpawnList(selector.GetLocalSpawnBlob());
		UpdateSelectedText();
		UpdateSpawnButtonVisuals();
	}

	protected void RenderSpawnList(string blob)
	{
		m_SpawnNames.Clear();
		ClearSpawnButtons();
		if ( blob.Length() == 0 )
		{
			m_SelectedSpawnIndex = -1;
			return;
		}

		TStringArray lines = new TStringArray;
		blob.Split("\n", lines);

		for ( int i = 0; i < lines.Count(); i++ )
		{
			TStringArray parts = new TStringArray;
			lines.Get(i).Split("|", parts);
			if ( parts.Count() < 1 )
			{
				continue;
			}

			string name = parts.Get(0);
			m_SpawnNames.Insert(name);
			ButtonWidget city_button = GetSpawnButton(m_SpawnNames.Count() - 1);
			if ( city_button )
			{
				city_button.Show(true);
				city_button.Enable(true);
				city_button.SetText(name);
			}
		}

		if ( m_SpawnNames.Count() > 0 )
		{
			if ( m_SelectedSpawnIndex < 0 || m_SelectedSpawnIndex >= m_SpawnNames.Count() )
			{
				m_SelectedSpawnIndex = 0;
			}
		}
	}

	protected string GetSelectedSpawnName()
	{
		if ( m_SpawnNames.Count() == 0 )
		{
			return "";
		}

		if ( m_SelectedSpawnIndex < 0 || m_SelectedSpawnIndex >= m_SpawnNames.Count() )
		{
			return "";
		}

		return m_SpawnNames.Get(m_SelectedSpawnIndex);
	}

	protected void ClearSpawnButtons()
	{
		for ( int i = 0; i < 8; i++ )
		{
			ButtonWidget button = GetSpawnButton(i);
			if ( button )
			{
				button.SetText("");
				button.Enable(false);
				button.Show(false);
				button.SetColor(0xAA30343A);
			}
		}
	}

	protected void UpdateSpawnButtonVisuals()
	{
		for ( int i = 0; i < 8; i++ )
		{
			ButtonWidget button = GetSpawnButton(i);
			if ( button )
			{
				if ( i < m_SpawnNames.Count() )
				{
					button.Show(true);
					button.Enable(true);
					if ( i == m_SelectedSpawnIndex )
					{
						button.SetColor(0xFFE00000);
					}
					else
					{
						button.SetColor(0xCC1A2635);
					}
				}
				else
				{
					button.Show(false);
					button.Enable(false);
				}
			}
		}
	}

	protected ButtonWidget GetSpawnButton(int index)
	{
		if ( index == 0 )
		{
			return m_SpawnButton0;
		}
		if ( index == 1 )
		{
			return m_SpawnButton1;
		}
		if ( index == 2 )
		{
			return m_SpawnButton2;
		}
		if ( index == 3 )
		{
			return m_SpawnButton3;
		}
		if ( index == 4 )
		{
			return m_SpawnButton4;
		}
		if ( index == 5 )
		{
			return m_SpawnButton5;
		}
		if ( index == 6 )
		{
			return m_SpawnButton6;
		}
		if ( index == 7 )
		{
			return m_SpawnButton7;
		}

		return NULL;
	}

	protected int GetSpawnButtonIndex(Widget widget)
	{
		if ( widget == m_SpawnButton0 )
		{
			return 0;
		}
		if ( widget == m_SpawnButton1 )
		{
			return 1;
		}
		if ( widget == m_SpawnButton2 )
		{
			return 2;
		}
		if ( widget == m_SpawnButton3 )
		{
			return 3;
		}
		if ( widget == m_SpawnButton4 )
		{
			return 4;
		}
		if ( widget == m_SpawnButton5 )
		{
			return 5;
		}
		if ( widget == m_SpawnButton6 )
		{
			return 6;
		}
		if ( widget == m_SpawnButton7 )
		{
			return 7;
		}

		return -1;
	}

	protected void UpdateSelectedText()
	{
		if ( !m_SelectedText )
		{
			return;
		}

		string selected = GetSelectedSpawnName();
		if ( selected.Length() == 0 )
		{
			m_SelectedText.SetText("Escolha uma cidade na lista.");
		}
		else
		{
			m_SelectedText.SetText("Selecionado: " + selected);
		}
	}

	protected void SetStatus(string text)
	{
		if ( !m_StatusText )
		{
			return;
		}

		if ( text.Length() == 0 )
		{
			text = "Aguardando escolha de spawn.";
		}

		m_StatusText.SetText(text);
	}

	protected void CloseSelectorMenu()
	{
		UnlockControls();

		UIManager manager = GetGame().GetUIManager();
		if ( !manager )
		{
			return;
		}

		if ( manager.IsMenuOpen(MENU_SPAWN_SELECTOR) )
		{
			manager.CloseMenu(MENU_SPAWN_SELECTOR);
		}
		else
		{
			manager.Back();
		}
	}
}
