class HudDebugHudTests extends ScriptedWidgetEventHandler
{
	private const int HUD_TEST_FOOD = 1;
	private const int HUD_TEST_WATER = 2;
	private const int HUD_TEST_TEMPERATURE = 3;
	private const int HUD_TEST_HEALTH = 4;
	private const int HUD_TEST_BLOOD = 5;

	private Widget m_WgtRoot;
	private CheckBoxWidget m_TestEnabled;
	private TextWidget m_StatusText;
	private TextWidget m_FoodLabel;
	private TextWidget m_WaterLabel;
	private TextWidget m_TemperatureLabel;
	private TextWidget m_BloodLabel;
	private TextWidget m_HealthLabel;

	private ButtonWidget m_FoodStage1;
	private ButtonWidget m_FoodStage2;
	private ButtonWidget m_FoodStage3;
	private ButtonWidget m_FoodStage4;
	private ButtonWidget m_FoodStage5;

	private ButtonWidget m_WaterStage1;
	private ButtonWidget m_WaterStage2;
	private ButtonWidget m_WaterStage3;
	private ButtonWidget m_WaterStage4;
	private ButtonWidget m_WaterStage5;

	private ButtonWidget m_TemperatureStage1;
	private ButtonWidget m_TemperatureStage2;
	private ButtonWidget m_TemperatureStage3;
	private ButtonWidget m_TemperatureStage4;
	private ButtonWidget m_TemperatureStage5;

	private ButtonWidget m_BloodStage1;
	private ButtonWidget m_BloodStage2;
	private ButtonWidget m_BloodStage3;
	private ButtonWidget m_BloodStage4;
	private ButtonWidget m_BloodStage5;

	private ButtonWidget m_HealthStage1;
	private ButtonWidget m_HealthStage2;
	private ButtonWidget m_HealthStage3;
	private ButtonWidget m_HealthStage4;
	private ButtonWidget m_HealthStage5;

	void HudDebugHudTests(Widget parent)
	{
		m_WgtRoot = GetGame().GetWorkspace().CreateWidgets("gui/layouts/custom_hud_tests.layout", parent);
		if ( !m_WgtRoot )
		{
			return;
		}

		m_WgtRoot.Show( true );
		m_WgtRoot.SetHandler( this );

		m_TestEnabled = (CheckBoxWidget)m_WgtRoot.FindAnyWidget("HudTestEnabled");
		m_StatusText = (TextWidget)m_WgtRoot.FindAnyWidget("HudTestStatus");
		m_FoodLabel = (TextWidget)m_WgtRoot.FindAnyWidget("HudTestFoodLabel");
		m_WaterLabel = (TextWidget)m_WgtRoot.FindAnyWidget("HudTestWaterLabel");
		m_TemperatureLabel = (TextWidget)m_WgtRoot.FindAnyWidget("HudTestTemperatureLabel");
		m_BloodLabel = (TextWidget)m_WgtRoot.FindAnyWidget("HudTestBloodLabel");
		m_HealthLabel = (TextWidget)m_WgtRoot.FindAnyWidget("HudTestHealthLabel");

		m_FoodStage1 = (ButtonWidget)m_WgtRoot.FindAnyWidget("HudTestFood1");
		m_FoodStage2 = (ButtonWidget)m_WgtRoot.FindAnyWidget("HudTestFood2");
		m_FoodStage3 = (ButtonWidget)m_WgtRoot.FindAnyWidget("HudTestFood3");
		m_FoodStage4 = (ButtonWidget)m_WgtRoot.FindAnyWidget("HudTestFood4");
		m_FoodStage5 = (ButtonWidget)m_WgtRoot.FindAnyWidget("HudTestFood5");

		m_WaterStage1 = (ButtonWidget)m_WgtRoot.FindAnyWidget("HudTestWater1");
		m_WaterStage2 = (ButtonWidget)m_WgtRoot.FindAnyWidget("HudTestWater2");
		m_WaterStage3 = (ButtonWidget)m_WgtRoot.FindAnyWidget("HudTestWater3");
		m_WaterStage4 = (ButtonWidget)m_WgtRoot.FindAnyWidget("HudTestWater4");
		m_WaterStage5 = (ButtonWidget)m_WgtRoot.FindAnyWidget("HudTestWater5");

		m_TemperatureStage1 = (ButtonWidget)m_WgtRoot.FindAnyWidget("HudTestTemperature1");
		m_TemperatureStage2 = (ButtonWidget)m_WgtRoot.FindAnyWidget("HudTestTemperature2");
		m_TemperatureStage3 = (ButtonWidget)m_WgtRoot.FindAnyWidget("HudTestTemperature3");
		m_TemperatureStage4 = (ButtonWidget)m_WgtRoot.FindAnyWidget("HudTestTemperature4");
		m_TemperatureStage5 = (ButtonWidget)m_WgtRoot.FindAnyWidget("HudTestTemperature5");

		m_BloodStage1 = (ButtonWidget)m_WgtRoot.FindAnyWidget("HudTestBlood1");
		m_BloodStage2 = (ButtonWidget)m_WgtRoot.FindAnyWidget("HudTestBlood2");
		m_BloodStage3 = (ButtonWidget)m_WgtRoot.FindAnyWidget("HudTestBlood3");
		m_BloodStage4 = (ButtonWidget)m_WgtRoot.FindAnyWidget("HudTestBlood4");
		m_BloodStage5 = (ButtonWidget)m_WgtRoot.FindAnyWidget("HudTestBlood5");

		m_HealthStage1 = (ButtonWidget)m_WgtRoot.FindAnyWidget("HudTestHealth1");
		m_HealthStage2 = (ButtonWidget)m_WgtRoot.FindAnyWidget("HudTestHealth2");
		m_HealthStage3 = (ButtonWidget)m_WgtRoot.FindAnyWidget("HudTestHealth3");
		m_HealthStage4 = (ButtonWidget)m_WgtRoot.FindAnyWidget("HudTestHealth4");
		m_HealthStage5 = (ButtonWidget)m_WgtRoot.FindAnyWidget("HudTestHealth5");

		RegisterHandler( m_TestEnabled );
		RegisterStageHandlers();

		if ( m_TestEnabled )
		{
			m_TestEnabled.SetText("Ativar teste");
			m_TestEnabled.SetChecked(false);
		}

		SetLabelText( m_FoodLabel, "Fome" );
		SetLabelText( m_WaterLabel, "Sede" );
		SetLabelText( m_TemperatureLabel, "Temp" );
		SetLabelText( m_BloodLabel, "Sangue" );
		SetLabelText( m_HealthLabel, "Vida" );

		SetStageButtonTexts();
		SetStatus("Pronto");
	}

	void ~HudDebugHudTests()
	{
		if ( m_WgtRoot )
		{
			m_WgtRoot.Destroy();
		}
	}

	private void RegisterStageHandlers()
	{
		RegisterHandler( m_FoodStage1 );
		RegisterHandler( m_FoodStage2 );
		RegisterHandler( m_FoodStage3 );
		RegisterHandler( m_FoodStage4 );
		RegisterHandler( m_FoodStage5 );

		RegisterHandler( m_WaterStage1 );
		RegisterHandler( m_WaterStage2 );
		RegisterHandler( m_WaterStage3 );
		RegisterHandler( m_WaterStage4 );
		RegisterHandler( m_WaterStage5 );

		RegisterHandler( m_TemperatureStage1 );
		RegisterHandler( m_TemperatureStage2 );
		RegisterHandler( m_TemperatureStage3 );
		RegisterHandler( m_TemperatureStage4 );
		RegisterHandler( m_TemperatureStage5 );

		RegisterHandler( m_BloodStage1 );
		RegisterHandler( m_BloodStage2 );
		RegisterHandler( m_BloodStage3 );
		RegisterHandler( m_BloodStage4 );
		RegisterHandler( m_BloodStage5 );

		RegisterHandler( m_HealthStage1 );
		RegisterHandler( m_HealthStage2 );
		RegisterHandler( m_HealthStage3 );
		RegisterHandler( m_HealthStage4 );
		RegisterHandler( m_HealthStage5 );
	}

	private void RegisterHandler(Widget widget)
	{
		if ( widget )
		{
			widget.SetHandler( this );
		}
	}

	private void SetStageButtonTexts()
	{
		SetButtonText( m_FoodStage1, "1" );
		SetButtonText( m_FoodStage2, "2" );
		SetButtonText( m_FoodStage3, "3" );
		SetButtonText( m_FoodStage4, "4" );
		SetButtonText( m_FoodStage5, "5" );

		SetButtonText( m_WaterStage1, "1" );
		SetButtonText( m_WaterStage2, "2" );
		SetButtonText( m_WaterStage3, "3" );
		SetButtonText( m_WaterStage4, "4" );
		SetButtonText( m_WaterStage5, "5" );

		SetButtonText( m_TemperatureStage1, "1" );
		SetButtonText( m_TemperatureStage2, "2" );
		SetButtonText( m_TemperatureStage3, "3" );
		SetButtonText( m_TemperatureStage4, "4" );
		SetButtonText( m_TemperatureStage5, "5" );

		SetButtonText( m_BloodStage1, "1" );
		SetButtonText( m_BloodStage2, "2" );
		SetButtonText( m_BloodStage3, "3" );
		SetButtonText( m_BloodStage4, "4" );
		SetButtonText( m_BloodStage5, "5" );

		SetButtonText( m_HealthStage1, "1" );
		SetButtonText( m_HealthStage2, "2" );
		SetButtonText( m_HealthStage3, "3" );
		SetButtonText( m_HealthStage4, "4" );
		SetButtonText( m_HealthStage5, "5" );
	}

	private void SetButtonText(ButtonWidget button_widget, string text)
	{
		if ( button_widget )
		{
			button_widget.SetText( text );
		}
	}

	private void SetLabelText(TextWidget label_widget, string text)
	{
		if ( label_widget )
		{
			label_widget.SetText( text );
		}
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if ( !w )
		{
			return false;
		}

		string name = w.GetName();
		if ( name == "HudTestEnabled" )
		{
			if ( IsTestEnabled() )
			{
				SetStatus("Escolha 1-5");
			}
			else
			{
				SetStatus("Pronto");
			}
			return false;
		}

		if ( HandleStageClick( name ) )
		{
			return true;
		}

		return false;
	}

	private bool HandleStageClick(string name)
	{
		if ( name == "HudTestFood1" )
		{
			RunStatusStageTest( HUD_TEST_FOOD, 1 );
			return true;
		}
		else if ( name == "HudTestFood2" )
		{
			RunStatusStageTest( HUD_TEST_FOOD, 2 );
			return true;
		}
		else if ( name == "HudTestFood3" )
		{
			RunStatusStageTest( HUD_TEST_FOOD, 3 );
			return true;
		}
		else if ( name == "HudTestFood4" )
		{
			RunStatusStageTest( HUD_TEST_FOOD, 4 );
			return true;
		}
		else if ( name == "HudTestFood5" )
		{
			RunStatusStageTest( HUD_TEST_FOOD, 5 );
			return true;
		}
		else if ( name == "HudTestWater1" )
		{
			RunStatusStageTest( HUD_TEST_WATER, 1 );
			return true;
		}
		else if ( name == "HudTestWater2" )
		{
			RunStatusStageTest( HUD_TEST_WATER, 2 );
			return true;
		}
		else if ( name == "HudTestWater3" )
		{
			RunStatusStageTest( HUD_TEST_WATER, 3 );
			return true;
		}
		else if ( name == "HudTestWater4" )
		{
			RunStatusStageTest( HUD_TEST_WATER, 4 );
			return true;
		}
		else if ( name == "HudTestWater5" )
		{
			RunStatusStageTest( HUD_TEST_WATER, 5 );
			return true;
		}
		else if ( name == "HudTestTemperature1" )
		{
			RunStatusStageTest( HUD_TEST_TEMPERATURE, 1 );
			return true;
		}
		else if ( name == "HudTestTemperature2" )
		{
			RunStatusStageTest( HUD_TEST_TEMPERATURE, 2 );
			return true;
		}
		else if ( name == "HudTestTemperature3" )
		{
			RunStatusStageTest( HUD_TEST_TEMPERATURE, 3 );
			return true;
		}
		else if ( name == "HudTestTemperature4" )
		{
			RunStatusStageTest( HUD_TEST_TEMPERATURE, 4 );
			return true;
		}
		else if ( name == "HudTestTemperature5" )
		{
			RunStatusStageTest( HUD_TEST_TEMPERATURE, 5 );
			return true;
		}
		else if ( name == "HudTestBlood1" )
		{
			RunStatusStageTest( HUD_TEST_BLOOD, 1 );
			return true;
		}
		else if ( name == "HudTestBlood2" )
		{
			RunStatusStageTest( HUD_TEST_BLOOD, 2 );
			return true;
		}
		else if ( name == "HudTestBlood3" )
		{
			RunStatusStageTest( HUD_TEST_BLOOD, 3 );
			return true;
		}
		else if ( name == "HudTestBlood4" )
		{
			RunStatusStageTest( HUD_TEST_BLOOD, 4 );
			return true;
		}
		else if ( name == "HudTestBlood5" )
		{
			RunStatusStageTest( HUD_TEST_BLOOD, 5 );
			return true;
		}
		else if ( name == "HudTestHealth1" )
		{
			RunStatusStageTest( HUD_TEST_HEALTH, 1 );
			return true;
		}
		else if ( name == "HudTestHealth2" )
		{
			RunStatusStageTest( HUD_TEST_HEALTH, 2 );
			return true;
		}
		else if ( name == "HudTestHealth3" )
		{
			RunStatusStageTest( HUD_TEST_HEALTH, 3 );
			return true;
		}
		else if ( name == "HudTestHealth4" )
		{
			RunStatusStageTest( HUD_TEST_HEALTH, 4 );
			return true;
		}
		else if ( name == "HudTestHealth5" )
		{
			RunStatusStageTest( HUD_TEST_HEALTH, 5 );
			return true;
		}

		return false;
	}

	private bool IsTestEnabled()
	{
		if ( m_TestEnabled && m_TestEnabled.IsChecked() )
		{
			return true;
		}

		return false;
	}

	private void RunStatusStageTest(int status_type, int stage)
	{
		if ( !IsTestEnabled() )
		{
			SetStatus("Marque ativar");
			return;
		}

		ApplyStatusStageToHud( status_type, stage );

		PluginDeveloper developer = PluginDeveloper.GetInstance();
		PlayerBase player = GetGame().GetPlayer();
		if ( developer && player )
		{
			developer.SetHudTestStatusStage( player, status_type, stage );
		}

		SetStatus( GetStatusLabel( status_type ) + ": " + stage.ToString() );
	}

	private void ApplyStatusStageToHud(int status_type, int stage)
	{
		Mission mission = GetGame().GetMission();
		if ( !mission )
		{
			return;
		}

		Hud hud = mission.GetHud();
		if ( !hud )
		{
			return;
		}

		if ( status_type == HUD_TEST_FOOD )
		{
			hud.DisplayStatusLevel( NTFKEY_HUNGRY, GetFoodWaterDisplayLevel( stage ) );
		}
		else if ( status_type == HUD_TEST_WATER )
		{
			hud.DisplayStatusLevel( NTFKEY_THIRSTY, GetFoodWaterDisplayLevel( stage ) );
		}
		else if ( status_type == HUD_TEST_TEMPERATURE )
		{
			hud.DisplayStatusLevel( NTFKEY_WARMTH, GetTemperatureDisplayLevel( stage ) );
		}
		else if ( status_type == HUD_TEST_BLOOD )
		{
			hud.DisplayStatusLevel( NTFKEY_BLOOD, stage );
		}
		else if ( status_type == HUD_TEST_HEALTH )
		{
			hud.DisplayStatusLevel( NTFKEY_LIVES, stage );
		}
	}

	private int GetFoodWaterDisplayLevel(int stage)
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

	private int GetTemperatureDisplayLevel(int stage)
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

	private string GetStatusLabel(int status_type)
	{
		if ( status_type == HUD_TEST_FOOD )
		{
			return "Fome";
		}
		else if ( status_type == HUD_TEST_WATER )
		{
			return "Sede";
		}
		else if ( status_type == HUD_TEST_TEMPERATURE )
		{
			return "Temp";
		}
		else if ( status_type == HUD_TEST_BLOOD )
		{
			return "Sangue";
		}
		else if ( status_type == HUD_TEST_HEALTH )
		{
			return "Vida";
		}

		return "Teste";
	}

	private void SetStatus(string text)
	{
		if ( m_StatusText )
		{
			m_StatusText.SetText( text );
		}
	}
}
