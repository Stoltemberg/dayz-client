/**
 * \defgroup Mouse Events
 * \desc constants for mouse events in PluginKeyBinding
 * @{
 */

const int MB_EVENT_PRESS			= 0;
const int MB_EVENT_CLICK			= 1;
const int MB_EVENT_RELEASE			= 2;
const int MB_EVENT_DOUBLECLICK		= 3;
const int MB_EVENT_DRAG				= 4;
/** @}*/

class PluginKeyBinding extends PluginBase
{
	static PluginKeyBinding instance;
	
	// System Public Events
	void PluginKeyBinding()
	{
		if ( instance == NULL )
		{
			instance = this;
		}
	}

	//============================================
	// OnInit
	//============================================
	void OnInit()
	{
		super.OnInit();
		
		m_KeyBindings	= new array<KeyBinding>;
		m_MouseBindings	= new array<MouseBinding>;
		m_MouseButtons	= new array<MouseButtonInfo>;
		m_MouseButtons.Insert ( new MouseButtonInfo( MouseState.LEFT ) );
		m_MouseButtons.Insert ( new MouseButtonInfo( MouseState.RIGHT ) );
		m_MouseButtons.Insert ( new MouseButtonInfo( MouseState.MIDDLE ) );
		m_TimerUpdate	= new TimerUpdate(this, "OnFrame", CALL_CATEGORY_SYSTEM);

		// Habilita as hotkeys do Script Debug Console e navegação de histórico
		RegisterKeyBind(	 MENU_NONE|MENU_SCRIPTCONSOLE	,KeyCode.KC_INSERT		,-1					,"PluginDeveloper"		,"ToggleScriptConsole" 			,"[Insert]"						,"Mostrar/ocultar console de script");
		RegisterKeyBind(	 MENU_SCRIPTCONSOLE				,KeyCode.KC_PRIOR		,-1					,"PluginDeveloper"		,"ScriptHistoryBack"			,"[Page Up]"					,"Console Debug => Histórico anterior");
		RegisterKeyBind(	 MENU_SCRIPTCONSOLE				,KeyCode.KC_NEXT		,-1					,"PluginDeveloper"		,"ScriptHistoryNext"			,"[Page Down]"					,"Console Debug => Próximo histórico");
		RegisterKeyBind(	 MENU_NONE|MENU_MAP				,KeyCode.KC_M			,-1					,"PluginDeveloper"		,"ToggleMapMenu" 				,"[M]"							,"Mostrar/ocultar mapa custom");
		RegisterKeyBind(	 MENU_NONE|MENU_ADMIN			,KeyCode.KC_DELETE		,-1					,"PluginAdmin"			,"ToggleAdminMenu"				,"[Delete]"						,"Mostrar/ocultar painel admin");
		RegisterKeyBind(	 MENU_NONE|MENU_PARTY			,KeyCode.KC_F8			,-1					,"PluginParty"			,"TogglePartyMenu"				,"[F8]"							,"Mostrar/ocultar party e clan");
		RegisterKeyBind(	 MENU_NONE|MENU_SCENE_EDITOR	,-1						,KeyCode.KC_P		,"PluginSceneManager"	,"EditorToggle"					,"[P]"							,"Mostrar/ocultar editor de cena");

		return;
		
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//Keyboard Binds	|UI_ID							|Key1					|Key2				|Callback Plugin		|Callback Function				|Info Shrtcut					|Info Description
		//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		//					|constants.h					|constants.h			|constants.h		|only plugin name		|only function					|								|
		//					|MENU_***						|KeyCode.KC_***			|KeyCode.KC_***		|						|in plugin						|								|
		//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		RegisterKeyBind(	 MENU_ANY						,KeyCode.KC_F1			,-1					,"PluginDeveloper"		,"ToggleHelpScreen" 			,"[F1]"							,"Mostrar/ocultar ajuda");
		RegisterKeyBind(	 MENU_NONE|MENU_SCRIPTCONSOLE	,KeyCode.KC_INSERT		,-1					,"PluginDeveloper"		,"ToggleScriptConsole" 			,"[Insert]"						,"Mostrar/ocultar console de script");
		RegisterKeyBind(	 MENU_NONE|MENU_MAP				,KeyCode.KC_M			,-1					,"PluginDeveloper"		,"ToggleMapMenu" 				,"[M]"							,"Mostrar/ocultar mapa custom");
		RegisterKeyBind(	 MENU_MAIN						,KeyCode.KC_LWIN 		,KeyCode.KC_E		,"PluginDeveloper"		,"ToggleMissionLoader"			,"[Win]+[E]"					,"Mostrar/ocultar carregador de missão");
		RegisterKeyBind(	 MENU_NONE						,KeyCode.KC_RSHIFT		,-1					,"PluginDeveloper"		,"SpawnFromClipboard"			,"[Right Shift]"				,"Jogo => Spawnar item do clipboard");
		RegisterKeyBind(	 MENU_NONE						,KeyCode.KC_LCONTROL	,KeyCode.KC_INSERT	,"PluginDeveloper"		,"TeleportAtCursor"				,"[LCtrl]+[Insert]"		 		,"Jogo => Teleportar jogador para o cursor");
		RegisterKeyBind(	 MENU_NONE						,KeyCode.KC_NEXT		,-1					,"PluginDeveloper"		,"ToggleGodModeThisPlayer"		,"[Page Down]"					,"Jogo => Ativar/desativar modo deus");
		RegisterKeyBind(	 MENU_NONE						,KeyCode.KC_HOME		,-1					,"PluginDeveloper"		,"ToggleFreeCamera"				,"[Home]"						,"Jogo => Ativar/desativar freecam e teleportar ao sair");
		RegisterKeyBind(	 MENU_NONE						,KeyCode.KC_DIVIDE		,-1					,"PluginDeveloper"		,"ToggleFreeCameraBackPos"		,"[Divide numeric]"				,"Jogo => Ativar/desativar freecam sem mover jogador ao sair");
		RegisterKeyBind(	 MENU_NONE|MENU_ADMIN			,KeyCode.KC_DELETE		,-1					,"PluginAdmin"			,"ToggleAdminMenu"				,"[Delete]"						,"Mostrar/ocultar painel admin");
		RegisterKeyBind(	 MENU_NONE|MENU_PARTY			,KeyCode.KC_F8			,-1					,"PluginParty"			,"TogglePartyMenu"				,"[F8]"							,"Mostrar/ocultar party e clan");
		RegisterKeyBind(	 MENU_SCRIPTCONSOLE				,KeyCode.KC_PRIOR		,-1					,"PluginDeveloper"		,"ScriptHistoryBack"			,"[Page Up]"					,"Console Debug => Histórico anterior");
		RegisterKeyBind(	 MENU_SCRIPTCONSOLE				,KeyCode.KC_NEXT		,-1					,"PluginDeveloper"		,"ScriptHistoryNext"			,"[Page Down]"					,"Console Debug => Próximo histórico");	
		RegisterKeyBind(	 MENU_SCENE_EDITOR				,KeyCode.KC_DELETE		,-1					,"PluginSceneManager"	,"DeleteSelectedObject"			,"[Delete]"						,"Editor de cena => Excluir objeto selecionado");	
		RegisterKeyBind(	 MENU_SCENE_EDITOR				,KeyCode.KC_LCONTROL	,KeyCode.KC_S		,"PluginSceneManager"	,"SceneSave"					,"[LCtrl]+[S]"					,"Editor de cena => Salvar cena atual");
		RegisterKeyBind(	 MENU_SCENE_EDITOR				,KeyCode.KC_LCONTROL	,KeyCode.KC_D		,"PluginSceneManager"	,"SelectedObjectDuplicate"		,"[LCtrl]+[D]"					,"Editor de cena => Duplicar objeto selecionado");
		RegisterKeyBind(	 MENU_SCENE_EDITOR				,KeyCode.KC_LCONTROL	,KeyCode.KC_F		,"PluginSceneManager"	,"SelectedObjectFocus"			,"[LCtrl]+[F]"					,"Editor de cena => Focar câmera no objeto selecionado");
		RegisterKeyBind(	 MENU_SCENE_EDITOR				,KeyCode.KC_LCONTROL	,KeyCode.KC_E		,"PluginSceneManager"	,"RulerToggle"					,"[LCtrl]+[E]"					,"Editor de cena => Ativar/desativar régua");
		RegisterKeyBind(	 MENU_ANY						,KeyCode.KC_LCONTROL	,KeyCode.KC_BACK	,"PluginRecipesManager"	,"CallbackGenerateCache"		,"[LCtrl]+[BACKSPACE]"			,"Receitas => Gerar cache de receitas e salvar em arquivo");
		RegisterKeyBind(	 MENU_ANY						,KeyCode.KC_LCONTROL	,KeyCode.KC_DECIMAL	,"PluginItemDiagnostic"	,"ToggleDebugWindowEvent"		,"[LCtrl]+[KeyCode.KC_DECIMAL]"	,"Fechar janela de debug de item");
		RegisterKeyBind(	 MENU_ANY						,KeyCode.KC_LWIN		,KeyCode.KC_NUMPAD0	,"PluginDayzPlayerDebug","ToggleDebugWindowEvent"		,"[LWin]+[KeyCode.KC_NUMPAD0]"	,"Abrir/fechar janela de debug do jogador");
		RegisterKeyBind(	 MENU_ANY						,KeyCode.KC_LWIN		,KeyCode.KC_DECIMAL	,"PluginDayzPlayerDebug","ToggleDebugWindowEventP"		,"[LWin]+[KeyCode.KC_NUMPAD0]"	,"Abrir/fechar janela de debug do jogador");
		RegisterKeyBind(	 MENU_NONE						,KeyCode.KC_LCONTROL	,KeyCode.KC_P		,"PluginHologramDebug"	,"TogglePlacingDebug"			,"[LCtrl]+[P]"					,"Iniciar/finalizar posicionamento preciso");
		RegisterKeyBind(	 MENU_NONE|MENU_SCENE_EDITOR	,-1						,KeyCode.KC_P		,"PluginSceneManager"	,"EditorToggle"					,"[P]"							,"Mostrar/ocultar editor de cena");
		//--------------------------------------------------------------------------------------------------------------------------------------------------------------
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Mouses Binds		|UI_ID				|Mouse Button	|Mouse Event			|Callback Plugin		|Callback Function			|Info Shrtcut			|Info Description
		//----------------------------------------------------------------------------------------------------------------------------------------------------------------
		//					|constants.h		|MouseState.LEFT		|MB_EVENT_CLICK			|only plugin name		|only function				|						|
		//					|MENU_***			|MouseState.RIGHT		|MB_EVENT_DOUBLECLICK   |						|in plugin					|						|
		//										|MouseState.MIDDLE		|MB_EVENT_DRAG			|						|							|						|
		//										|						|MB_EVENT_RELEASE		|						|							|						|
		//--------------------------------------------------------------------------------------------------------------------------------------------------------------
		RegisterMouseBind( 	MENU_SCENE_EDITOR	,MouseState.LEFT		,MB_EVENT_CLICK			,"PluginSceneManager"	,"Event_OnClick"			,"[LMB] Clique"			,"Editor de cena => Selecionar objeto na cena");
		RegisterMouseBind( 	MENU_SCENE_EDITOR	,MouseState.LEFT		,MB_EVENT_DOUBLECLICK	,"PluginSceneManager"	,"Event_OnDoubleClick" 		,"[LMB] Duplo clique"	,"Editor de cena => Criar novo objeto na cena");
		RegisterMouseBind( 	MENU_SCENE_EDITOR	,MouseState.LEFT		,MB_EVENT_DRAG			,"PluginSceneManager"	,"Event_OnDrag" 			,"[LMB] Segurar"		,"Editor de cena => Mover objeto selecionado");
		RegisterMouseBind( 	MENU_SCENE_EDITOR	,MouseState.MIDDLE		,MB_EVENT_CLICK			,"PluginSceneManager"	,"Event_OnClickMiddle"		,"[MMB] Clique"			,"Editor de cena => Vincular objeto clicado ao selecionado");
		RegisterMouseBind( 	MENU_SCENE_EDITOR	,MouseState.RIGHT		,MB_EVENT_PRESS			,"PluginSceneManager"	,"FreeCameraControlEnable"	,"[RMB] Segurar"		,"Editor de cena => Ativar movimento da câmera");
		RegisterMouseBind( 	MENU_SCENE_EDITOR	,MouseState.RIGHT		,MB_EVENT_RELEASE		,"PluginSceneManager"	,"FreeCameraControlDisable"	,"[RMB] Soltar"			,"Editor de cena => Desativar movimento da câmera");
		//--------------------------------------------------------------------------------------------------------------------------------------------------------------
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	}
	
	//==========================================
	// OnInit (System Event)
	//==========================================
	void OnDestroy()
	{
		m_TimerUpdate.Stop();
		int i;
		
		for ( i = 0; i < m_KeyBindings.Count(); ++i )
		{
			delete m_KeyBindings.Get(i);
		}
		delete m_KeyBindings;
		
		for ( i = 0; i < m_MouseButtons.Count(); ++i )
		{
			delete m_MouseButtons.Get(i);
		}
		delete m_MouseButtons;
		
		for ( i = 0; i < m_MouseBindings.Count(); ++i )
		{
			delete m_MouseBindings.Get(i);
		}
		delete m_MouseBindings;
	}
	
	//============================================
	// OnKeyPress
	//============================================
	void OnKeyPress(int key)
	{
//Print("OnKeyPress "+ key);
		if ( m_KeyBindings == NULL )
		{
			return;
		}

		for ( int i = 0; i < m_KeyBindings.Count(); ++i )
		{
			KeyBinding key_binding = m_KeyBindings.Get(i);
			
			if ( !IsCurrentUIPageValid( key_binding.GetUIMenuID() ) )
			{
				continue;
			}
			
			bool key1_match = false;
			bool key2_match = false;
			
			if ( key_binding.GetKey1() > -1 )
			{
				if( key == key_binding.GetKey1() || IsKeyPressed(key_binding.GetKey1()) )
				{
					// Key1 is down
					key1_match = true;
				}
			}
			else
			{
				// Key1 is not set
				key1_match = true;
			}
			
			if ( key_binding.GetKey2() > -1 )
			{
				if( key == key_binding.GetKey2() || IsKeyPressed(key_binding.GetKey2()) )
				{
					// Key2 is down
					key2_match = true;
				}
			}
			else
			{
				// Key2 is not set
				key2_match = true;
			}
			
			if ( key1_match && key2_match )
			{
//Print( "Call Method -> key press: " + key + " " + key_binding.GetInfoDescription() );
				
				PluginBase m = GetPlugin(key_binding.GetCallbackTarget().ToType());
				if ( m )
				{
					GetGame().GameScript.CallFunction(m, key_binding.GetCallbackFunction(), NULL, 0);
				}
				
				break;
			}
		}
	}
	
	//============================================
	// OnKeyRelease
	//============================================
	void OnKeyRelease(int key)
	{
	}

	void ClearMouseButtonStates()
	{
		if ( m_MouseButtons == NULL )
		{
			return;
		}

		for ( int i = 0; i < m_MouseButtons.Count(); ++i )
		{
			MouseButtonInfo info = m_MouseButtons.Get(i);
			if ( info )
			{
				info.ForceRelease();
			}
		}
	}
	
	//============================================
	// OnMouseButtonPress
	//============================================
	void OnMouseButtonPress(int button)
	{
		MouseButtonInfo button_info = GetMouseButtonInfo( button );
		if (button_info == NULL) return;
		button_info.Press();
		
	//Log("****************************");
	//Log(" MB_EVENT_PRESS");
		
		OnMouse( MB_EVENT_PRESS, button );
	}
	
	//============================================
	// OnMouseButtonRelease
	//============================================
	void OnMouseButtonRelease(int button)
	{
		MouseButtonInfo button_info = GetMouseButtonInfo( button );
		if (button_info == NULL) return;
		int time_curr			= GetGame().GetTime();
		int time_last_press		= button_info.GetTimeLastPress();
		int time_last_release	= button_info.GetTimeLastRelease();	
		int time_delta_press	= time_curr - time_last_press;
		int time_delta_relase	= time_curr - time_last_release;
		
		if ( time_delta_relase < DOUBLE_CLICK_TIME )
		{
	//Log(" DOUBLE_CLICK_TIME ");
			
			OnMouse( MB_EVENT_DOUBLECLICK, button );
		}
		else if ( time_delta_press < CLICK_TIME )
		{
	//Log(" MB_EVENT_CLICK ");
			
			OnMouse( MB_EVENT_CLICK, button );
		}
		
	//Log(" MB_EVENT_RELEASE");
		
		OnMouse( MB_EVENT_RELEASE, button );
		
		button_info.Release();
	}
	
	//============================================
	// OnFrame
	//============================================
	void OnFrame()
	{
		if ( m_MouseButtons == NULL )
		{
			return;
		}

		for ( int i = 0; i < m_MouseButtons.Count(); ++i )
		{
			MouseButtonInfo info = m_MouseButtons.Get(i);
			
			if ( info.IsButtonDown() )
			{		
				int time_curr = GetGame().GetTime();
				int time_hold = info.GetTimeLastPress() + HOLD_CLICK_TIME_MIN;
				
				if ( time_hold < time_curr )
				{
	//Log("MB_EVENT_DRAG");
					
					OnMouse( MB_EVENT_DRAG, info.GetButtonID() );
				}
			}
		}
	}
	
	//--------------------------------------------
	// GetKeyBindings
	//--------------------------------------------
	array<KeyBinding> GetKeyBindings()
	{
		return m_KeyBindings;
	}

	//--------------------------------------------
	// GetMouseBindings
	//--------------------------------------------
	array<MouseBinding> GetMouseBindings()
	{
		return m_MouseBindings;
	}
	
	protected const int CLICK_TIME			= 200; //ms
	protected const int DOUBLE_CLICK_TIME	= 300; //ms
	protected const int HOLD_CLICK_TIME_MIN	= 300; //ms
	
	protected autoptr TimerUpdate		m_TimerUpdate;
	protected array<KeyBinding>			m_KeyBindings;
	protected array<MouseButtonInfo>	m_MouseButtons;
	protected array<MouseBinding>		m_MouseBindings;
	
	//--------------------------------------------
	// RegisterKeyBind
	//--------------------------------------------
	protected void RegisterKeyBind( int ui_id, int key_code1, int key_code2, string plugin_name, string fnc_name, string info_shortcut, string info_description )
	{
		if ( m_KeyBindings == NULL )
		{
			m_KeyBindings = new array<KeyBinding>;
		}

		m_KeyBindings.Insert( new KeyBinding(ui_id, key_code1, key_code2, plugin_name, fnc_name, info_shortcut, info_description) );
	}

	//--------------------------------------------
	// RegisterMouseBind
	//--------------------------------------------
	protected void RegisterMouseBind( int ui_id, int mouse_button, int mouse_event, string plugin_name, string fnc_name, string info_shortcut, string info_description )
	{
		if ( m_MouseBindings == NULL )
		{
			m_MouseBindings = new array<MouseBinding>;
		}

		m_MouseBindings.Insert( new MouseBinding(ui_id, mouse_button, mouse_event, plugin_name, fnc_name, info_shortcut, info_description) );
	}

	//--------------------------------------------
	// IsCurrentUIPageValid
	//--------------------------------------------
	protected bool IsCurrentUIPageValid( int ui_page_request )
	{
		int ui_page_current = MENU_NONE;
		
		if ( GetGame().GetUIManager().GetMenu() )
		{
			ui_page_current = GetGame().GetUIManager().GetMenu().GetID();
		}
		
		if( !CheckMask( ui_page_request, ui_page_current ) )
		{			
			if ( CheckMask( ui_page_request, MENU_NONE ) || !CheckMask( ui_page_request, MENU_ANY ) )
			{
				return false;
			}
		}
		
		return true;
	}

	bool CheckMask( int source_mask, int target_mask )
	{
		if ( ( source_mask & target_mask ) == target_mask )
		{
			return true;
		}
		
		return false;
	}

	//--------------------------------------------
	// OnMouse
	//--------------------------------------------
	protected void OnMouse( int event_id, int button )
	{	
		if ( m_MouseBindings == NULL )
		{
			return;
		}

		for ( int i = 0; i < m_MouseBindings.Count(); ++i )
		{
			MouseBinding mouse_binding = m_MouseBindings.Get(i);
			
			if ( mouse_binding && mouse_binding.GetButtonID() == button && mouse_binding.GetMouseEvent() == event_id )
			{
				if ( IsCurrentUIPageValid( mouse_binding.GetUIMenuID() ) )
				{
					PluginBase m = GetPlugin(mouse_binding.GetCallbackTarget().ToType());
					if ( m )
					{
						GetGame().GameScript.CallFunction(m, mouse_binding.GetCallbackFunction(), NULL, 0);
					}
				}
			}			
		}
	}

	//--------------------------------------------
	// OnMouse
	//--------------------------------------------
	protected MouseButtonInfo GetMouseButtonInfo( int button )
	{	
		if ( m_MouseButtons == NULL )
		{
			return NULL;
		}

		for ( int i = 0; i < m_MouseButtons.Count(); ++i )
		{
			MouseButtonInfo info = m_MouseButtons.Get(i);
			
			if ( info && info.GetButtonID() == button )
			{
				return info;
			}
		}

		return NULL;
	}

	//--------------------------------------------
	// IsKeyPressed
	//--------------------------------------------
	protected bool IsKeyPressed(int key)
	{	
		return ( KeyState(key) == 1 );
	}
}
