class CustomMapMarkerData
{
	string m_Name;
	float m_X;
	float m_Y;
}

class MapMenu extends UIScriptedMenu
{
	private static const float MAP_WORLD_SIZE = 15360.0;
	private const float ZOOM_MIN = 1.0;
	private const float ZOOM_MAX = 8.0;
	private const float ZOOM_STEP = 0.25;
	const int MAX_CUSTOM_MARKERS = 10;
	const int MARKER_USER_ID_BASE = 7000;

	static private autoptr array<string> s_MarkerNames;
	static private autoptr array<float> s_MarkerXs;
	static private autoptr array<float> s_MarkerYs;
	static private bool s_ShowHud = false;
	static private bool s_ShowMap = true;

	private Widget m_MapViewport;
	private Widget m_MapContent;
	private ImageWidget m_MapImage;
	private Widget m_PlayerMarker;
	private Widget m_PlayerMarkerLabel;
	private ImageWidget m_PlayerMarkerOverlay;
	private Widget m_PlayerMarkerNameBg;
	private TextWidget m_PlayerMarkerName;

	private TextListboxWidget m_MarkersListbox;
	private CheckBoxWidget m_ShowHudCheckbox;
	private CheckBoxWidget m_ShowMapCheckbox;
	private Widget m_MarkerEditor;
	private EditBoxWidget m_MarkerNameEdit;
	private ButtonWidget m_MarkerSaveButton;
	private ButtonWidget m_MarkerDeleteButton;
	private ButtonWidget m_MarkerCancelButton;

	private ButtonWidget m_CloseButton;
	private ButtonWidget m_ZoomInButton;
	private ButtonWidget m_ZoomOutButton;
	private ButtonWidget m_CenterButton;

	private TextWidget m_ZoomText;
	private TextWidget m_PositionText;

	private autoptr array<CustomMapMarkerData> m_CustomMarkers;
	private autoptr array<Widget> m_MarkerPins;
	private int m_SelectedMarkerIndex;

	private float m_Zoom;
	private float m_OffsetX;
	private float m_OffsetY;

	private bool m_Dragging;
	private int m_DragStartMouseX;
	private int m_DragStartMouseY;
	private float m_DragStartOffsetX;
	private float m_DragStartOffsetY;

	void MapMenu()
	{
		m_CustomMarkers = new array<CustomMapMarkerData>;
		m_MarkerPins = new array<Widget>;
		LoadMarkersFromStorage();
		m_SelectedMarkerIndex = -1;
		m_Zoom = 1.0;
		m_OffsetX = 0.0;
		m_OffsetY = 0.0;
		m_Dragging = false;
	}

	void ~MapMenu()
	{
		StopDragging();
	}

	Widget Init()
	{
		layoutRoot = GetGame().GetWorkspace().CreateWidgets("gui/layouts/day_z_map.layout");
		if ( !layoutRoot )
		{
			return NULL;
		}

		m_MapViewport = layoutRoot.FindAnyWidget("CustomMapViewport");
		m_MapContent = layoutRoot.FindAnyWidget("CustomMapContent");
		m_MapImage = (ImageWidget)layoutRoot.FindAnyWidget("CustomMapImage");
		m_PlayerMarker = layoutRoot.FindAnyWidget("CustomMapPlayerMarker");
		m_PlayerMarkerLabel = layoutRoot.FindAnyWidget("CustomMapPlayerLabel");
		m_PlayerMarkerOverlay = (ImageWidget)layoutRoot.FindAnyWidget("CustomMapPlayerMarkerOverlay");
		m_PlayerMarkerNameBg = layoutRoot.FindAnyWidget("CustomMapPlayerMarkerNameBg");
		m_PlayerMarkerName = (TextWidget)layoutRoot.FindAnyWidget("CustomMapPlayerMarkerName");

		m_MarkersListbox = (TextListboxWidget)layoutRoot.FindAnyWidget("CustomMapMarkersListbox");
		m_ShowHudCheckbox = (CheckBoxWidget)layoutRoot.FindAnyWidget("CustomMapShowHud");
		m_ShowMapCheckbox = (CheckBoxWidget)layoutRoot.FindAnyWidget("CustomMapShowMap");
		m_MarkerEditor = layoutRoot.FindAnyWidget("CustomMapMarkerEditor");
		m_MarkerNameEdit = (EditBoxWidget)layoutRoot.FindAnyWidget("CustomMapMarkerNameEdit");
		m_MarkerSaveButton = (ButtonWidget)layoutRoot.FindAnyWidget("CustomMapMarkerSave");
		m_MarkerDeleteButton = (ButtonWidget)layoutRoot.FindAnyWidget("CustomMapMarkerDelete");
		m_MarkerCancelButton = (ButtonWidget)layoutRoot.FindAnyWidget("CustomMapMarkerCancel");

		m_CloseButton = (ButtonWidget)layoutRoot.FindAnyWidget("CustomMapClose");
		m_ZoomInButton = (ButtonWidget)layoutRoot.FindAnyWidget("CustomMapZoomIn");
		m_ZoomOutButton = (ButtonWidget)layoutRoot.FindAnyWidget("CustomMapZoomOut");
		m_CenterButton = (ButtonWidget)layoutRoot.FindAnyWidget("CustomMapCenter");

		m_ZoomText = (TextWidget)layoutRoot.FindAnyWidget("CustomMapZoomText");
		m_PositionText = (TextWidget)layoutRoot.FindAnyWidget("CustomMapPositionText");

		if ( m_MapContent )
		{
			m_MapContent.SetSort(20);
		}

		if ( m_ZoomInButton )
		{
			m_ZoomInButton.SetSort(950);
		}

		if ( m_ZoomOutButton )
		{
			m_ZoomOutButton.SetSort(950);
		}

		if ( m_MarkerEditor )
		{
			m_MarkerEditor.SetSort(980);
			m_MarkerEditor.Show(false);
		}

		if ( m_ShowHudCheckbox )
		{
			m_ShowHudCheckbox.SetChecked(s_ShowHud);
		}

		if ( m_ShowMapCheckbox )
		{
			m_ShowMapCheckbox.SetChecked(s_ShowMap);
		}

		if ( m_MapImage )
		{
			m_MapImage.SetFlags(WidgetFlags.STRETCH | WidgetFlags.NOWRAP | WidgetFlags.VISIBLE);
			m_MapImage.SetSort(30);
			bool map_loaded = m_MapImage.LoadImageFile(0, "gui/maps/BALOTA.tga", true);
		}
		else
		{
			Print("[CustomMap] CustomMapImage widget not found.");
		}

		if ( m_PlayerMarker )
		{
			m_PlayerMarker.Show(false);
		}

		if ( m_PlayerMarkerLabel )
		{
			m_PlayerMarkerLabel.Show(false);
		}

		if ( m_PlayerMarkerOverlay )
		{
			m_PlayerMarkerOverlay.SetFlags(WidgetFlags.STRETCH | WidgetFlags.NOWRAP | WidgetFlags.SOURCEALPHA | WidgetFlags.BLEND | WidgetFlags.VISIBLE);
			bool marker_loaded = m_PlayerMarkerOverlay.LoadImageFile(0, "gui/maps/player_marker.tga", true);
			Print("[CustomMap] player_marker.tga loaded=" + marker_loaded.ToString());
			m_PlayerMarkerOverlay.SetSort(990);
			m_PlayerMarkerOverlay.Show(true);
		}

		if ( m_PlayerMarkerNameBg )
		{
			m_PlayerMarkerNameBg.SetSort(991);
			m_PlayerMarkerNameBg.SetColor(0xCC061326);
			m_PlayerMarkerNameBg.Show(false);
		}

		if ( m_PlayerMarkerName )
		{
			m_PlayerMarkerName.SetSort(992);
			m_PlayerMarkerName.SetColor(0xFFFFFFFF);
			m_PlayerMarkerName.SetText("ME");
			m_PlayerMarkerName.Show(false);
		}

		InitMarkerPins();
		RenderMarkerList();
		CenterOnPlayer();
		UpdatePlayerMarker();

		return layoutRoot;
	}

	void Cleanup()
	{
		StopDragging();
	}

	void Update(float timeslice)
	{
		UpdatePlayerMarker();
		UpdateMarkerPinPositions();
	}

	bool OnClick(Widget w, int x, int y, int button)
	{
		super.OnClick(w, x, y, button);

		int marker_index = MarkerIndexFromWidget(w);
		if ( marker_index >= 0 )
		{
			SelectMarker(marker_index, true);
			return true;
		}

		if ( w == m_ShowHudCheckbox )
		{
			s_ShowHud = m_ShowHudCheckbox.IsChecked();
			return true;
		}

		if ( w == m_ShowMapCheckbox )
		{
			s_ShowMap = m_ShowMapCheckbox.IsChecked();
			UpdateMarkerPinPositions();
			return true;
		}

		if ( w == m_MarkerSaveButton )
		{
			SaveSelectedMarkerName();
			return true;
		}

		if ( w == m_MarkerDeleteButton )
		{
			DeleteSelectedMarker();
			return true;
		}

		if ( w == m_MarkerCancelButton )
		{
			HideMarkerEditor();
			return true;
		}

		if ( w == m_CloseButton )
		{
			Close();
			return true;
		}

		if ( w == m_ZoomInButton )
		{
			SetZoom(m_Zoom + ZOOM_STEP);
			return true;
		}

		if ( w == m_ZoomOutButton )
		{
			SetZoom(m_Zoom - ZOOM_STEP);
			return true;
		}

		if ( w == m_CenterButton )
		{
			CenterOnPlayer();
			return true;
		}

		return false;
	}

	bool OnDoubleClick(Widget w, int x, int y, int button)
	{
		super.OnDoubleClick(w, x, y, button);

		if ( button != MouseState.LEFT )
		{
			return false;
		}

		if ( MarkerIndexFromWidget(w) >= 0 )
		{
			return true;
		}

		if ( IsMapWidget(w) )
		{
			StopDragging();
			AddMarkerAtMouse();
			return true;
		}

		return false;
	}

	bool OnItemSelected(Widget w, int x, int y, int row, int column, int oldRow, int oldColumn)
	{
		super.OnItemSelected(w, x, y, row, column, oldRow, oldColumn);

		if ( w == m_MarkersListbox )
		{
			SelectMarker(row, true);
			return true;
		}

		return false;
	}

	bool OnMouseButtonDown(Widget w, int x, int y, int button)
	{
		super.OnMouseButtonDown(w, x, y, button);

		if ( button == MouseState.LEFT && IsMapWidget(w) && MarkerIndexFromWidget(w) < 0 )
		{
			m_Dragging = true;
			m_DragStartOffsetX = m_OffsetX;
			m_DragStartOffsetY = m_OffsetY;
			GetMousePos(m_DragStartMouseX, m_DragStartMouseY);
			GetGame().GetDragQueue().Call(this, "UpdateDragging");
			return true;
		}

		return false;
	}

	bool OnMouseButtonUp(Widget w, int x, int y, int button)
	{
		super.OnMouseButtonUp(w, x, y, button);
		StopDragging();
		return false;
	}

	bool OnMouseWheel(Widget w, int x, int y, int wheel)
	{
		super.OnMouseWheel(w, x, y, wheel);

		if ( IsMapWidget(w) )
		{
			if ( wheel > 0 )
			{
				SetZoom(m_Zoom + ZOOM_STEP);
			}
			else
			{
				SetZoom(m_Zoom - ZOOM_STEP);
			}

			return true;
		}

		return false;
	}

	bool OnKeyPress(Widget w, int x, int y, int key)
	{
		super.OnKeyPress(w, x, y, key);

		if ( key == KeyCode.KC_M || key == KeyCode.KC_ESCAPE )
		{
			Close();
			return true;
		}

		if ( key == KeyCode.KC_EQUALS || key == KeyCode.KC_ADD )
		{
			SetZoom(m_Zoom + ZOOM_STEP);
			return true;
		}

		if ( key == KeyCode.KC_MINUS || key == KeyCode.KC_SUBTRACT )
		{
			SetZoom(m_Zoom - ZOOM_STEP);
			return true;
		}

		if ( key == KeyCode.KC_C )
		{
			CenterOnPlayer();
			return true;
		}

		return false;
	}

	private bool IsMapWidget(Widget w)
	{
		Widget current = w;

		while ( current )
		{
			if ( current == m_MapViewport )
			{
				return true;
			}

			if ( current == layoutRoot )
			{
				return false;
			}

			current = current.GetParent();
		}

		return false;
	}

	private int MarkerIndexFromWidget(Widget w)
	{
		Widget current = w;

		while ( current )
		{
			int user_id = current.GetUserID();

			if ( user_id >= MARKER_USER_ID_BASE )
			{
				int index = user_id - MARKER_USER_ID_BASE;

				if ( index >= 0 && index < m_CustomMarkers.Count() )
				{
					return index;
				}
			}

			if ( current == m_MapViewport || current == layoutRoot )
			{
				return -1;
			}

			current = current.GetParent();
		}

		return -1;
	}

	private void AddMarkerAtMouse()
	{
		if ( !m_MapViewport || m_CustomMarkers.Count() >= MAX_CUSTOM_MARKERS )
		{
			return;
		}

		int mouse_x;
		int mouse_y;
		GetMousePos(mouse_x, mouse_y);

		float screen_x;
		float screen_y;
		float width;
		float height;
		m_MapViewport.GetScreenPos(screen_x, screen_y);
		m_MapViewport.GetScreenSize(width, height);

		if ( width <= 0 || height <= 0 || m_Zoom <= 0 )
		{
			return;
		}

		float view_x = (mouse_x - screen_x) / width;
		float view_y = (mouse_y - screen_y) / height;
		float map_x = Math.Clamp((view_x - m_OffsetX) / m_Zoom, 0.0, 1.0);
		float map_y = Math.Clamp((view_y - m_OffsetY) / m_Zoom, 0.0, 1.0);

		CustomMapMarkerData marker = new CustomMapMarkerData;
		marker.m_X = map_x;
		marker.m_Y = map_y;
		marker.m_Name = "Marcador " + (m_CustomMarkers.Count() + 1).ToString();

		m_CustomMarkers.Insert(marker);
		SaveMarkersToStorage();
		RenderMarkerList();
		SelectMarker(m_CustomMarkers.Count() - 1, true);
		UpdateMarkerPinPositions();
	}

	static private void EnsureMarkerStorage()
	{
		if ( !s_MarkerNames || !s_MarkerXs || !s_MarkerYs )
		{
			s_MarkerNames = new array<string>;
			s_MarkerXs = new array<float>;
			s_MarkerYs = new array<float>;
		}
	}

	static int GetCustomMarkerCount()
	{
		EnsureMarkerStorage();
		return s_MarkerNames.Count();
	}

	static bool GetCustomMarkersShowHud()
	{
		return s_ShowHud;
	}

	static bool GetCustomMarkersShowMap()
	{
		return s_ShowMap;
	}

	static string GetCustomMarkerName(int index)
	{
		EnsureMarkerStorage();

		if ( index < 0 || index >= s_MarkerNames.Count() )
		{
			return "";
		}

		return s_MarkerNames.Get(index);
	}

	static vector GetCustomMarkerWorldPosition(int index, float y_offset = 0.0)
	{
		EnsureMarkerStorage();

		if ( index < 0 || index >= s_MarkerXs.Count() || index >= s_MarkerYs.Count() )
		{
			return Vector(0, 0, 0);
		}

		float world_x = s_MarkerXs.Get(index) * MAP_WORLD_SIZE;
		float world_z = (1.0 - s_MarkerYs.Get(index)) * MAP_WORLD_SIZE;
		float world_y = GetGame().SurfaceY(world_x, world_z) + y_offset;
		return Vector(world_x, world_y, world_z);
	}

	static float GetCustomMarkerDistance(PlayerBase player, int index)
	{
		if ( !player )
		{
			return 0.0;
		}

		vector marker_pos = GetCustomMarkerWorldPosition(index);
		vector player_pos = player.GetPosition();
		marker_pos[1] = player_pos[1];
		return vector.Distance(player_pos, marker_pos);
	}

	static string FormatCustomMarkerDistance(float distance)
	{
		float meters = Math.Round(distance);
		return meters.ToString() + "m";
	}

	private void LoadMarkersFromStorage()
	{
		EnsureMarkerStorage();
		m_CustomMarkers.Clear();

		int count = s_MarkerNames.Count();

		if ( s_MarkerXs.Count() < count )
		{
			count = s_MarkerXs.Count();
		}

		if ( s_MarkerYs.Count() < count )
		{
			count = s_MarkerYs.Count();
		}

		if ( count > MAX_CUSTOM_MARKERS )
		{
			count = MAX_CUSTOM_MARKERS;
		}

		for ( int i = 0; i < count; i++ )
		{
			CustomMapMarkerData marker = new CustomMapMarkerData;
			marker.m_Name = s_MarkerNames.Get(i);
			marker.m_X = s_MarkerXs.Get(i);
			marker.m_Y = s_MarkerYs.Get(i);
			m_CustomMarkers.Insert(marker);
		}
	}

	private void SaveMarkersToStorage()
	{
		EnsureMarkerStorage();
		s_MarkerNames.Clear();
		s_MarkerXs.Clear();
		s_MarkerYs.Clear();

		for ( int i = 0; i < m_CustomMarkers.Count(); i++ )
		{
			CustomMapMarkerData marker = m_CustomMarkers.Get(i);
			s_MarkerNames.Insert(marker.m_Name);
			s_MarkerXs.Insert(marker.m_X);
			s_MarkerYs.Insert(marker.m_Y);
		}
	}

	private void InitMarkerPins()
	{
		m_MarkerPins.Clear();

		for ( int i = 0; i < MAX_CUSTOM_MARKERS; i++ )
		{
			Widget pin = layoutRoot.FindAnyWidget("CustomMapMarkerPin" + i.ToString());

			if ( pin )
			{
				pin.SetUserID(MARKER_USER_ID_BASE + i);
				pin.SetSort(930);
				pin.Show(false);

				ImageWidget icon = (ImageWidget)pin.FindAnyWidget("CustomMapMarkerIcon");
				if ( icon )
				{
					icon.SetFlags(WidgetFlags.STRETCH | WidgetFlags.NOWRAP | WidgetFlags.SOURCEALPHA | WidgetFlags.BLEND | WidgetFlags.VISIBLE);
					bool custom_marker_loaded = icon.LoadImageFile(0, "gui/maps/custom_marker_icon.tga", true);
					Print("[CustomMap] custom_marker_icon.tga loaded=" + custom_marker_loaded.ToString());
				}

				Widget label_bg = pin.FindAnyWidget("CustomMapMarkerLabelBg");
				if ( label_bg )
				{
					label_bg.SetColor(0xCC061326);
					label_bg.SetSort(935);
				}

				TextWidget label = GetMarkerLabel(pin);
				if ( label )
				{
					label.SetColor(0xFFFFFFFF);
					label.SetSort(936);
					label.SetText("");
				}
			}

			m_MarkerPins.Insert(pin);
		}
	}

	private TextWidget GetMarkerLabel(Widget pin)
	{
		if ( !pin )
		{
			return NULL;
		}

		Widget label_bg = pin.FindAnyWidget("CustomMapMarkerLabelBg");

		if ( label_bg )
		{
			return (TextWidget)label_bg.GetChildren();
		}

		return (TextWidget)pin.FindAnyWidget("CustomMapMarkerLabel");
	}

	private void RenderMarkerList()
	{
		if ( !m_MarkersListbox )
		{
			return;
		}

		m_MarkersListbox.ClearItems();

		if ( m_CustomMarkers.Count() == 0 )
		{
			m_MarkersListbox.AddItem("Duplo clique no mapa", NULL, 0);
			return;
		}

		for ( int i = 0; i < m_CustomMarkers.Count(); i++ )
		{
			CustomMapMarkerData marker = m_CustomMarkers.Get(i);
			m_MarkersListbox.AddItem((i + 1).ToString() + ". " + GetMarkerLabelText(marker), NULL, 0);
		}

	}

	private void SelectMarker(int index, bool show_editor)
	{
		if ( index < 0 || index >= m_CustomMarkers.Count() )
		{
			return;
		}

		m_SelectedMarkerIndex = index;

		if ( show_editor )
		{
			ShowMarkerEditor(index);
		}
	}

	private void ShowMarkerEditor(int index)
	{
		if ( !m_MarkerEditor || !m_MarkerNameEdit || index < 0 || index >= m_CustomMarkers.Count() )
		{
			return;
		}

		CustomMapMarkerData marker = m_CustomMarkers.Get(index);
		m_MarkerNameEdit.SetText(marker.m_Name);
		PlaceMarkerEditor(marker);
		m_MarkerEditor.Show(true);
	}

	private void HideMarkerEditor()
	{
		m_SelectedMarkerIndex = -1;

		if ( m_MarkerEditor )
		{
			m_MarkerEditor.Show(false);
		}
	}

	private void SaveSelectedMarkerName()
	{
		if ( !m_MarkerNameEdit || m_SelectedMarkerIndex < 0 || m_SelectedMarkerIndex >= m_CustomMarkers.Count() )
		{
			return;
		}

		string name = m_MarkerNameEdit.GetText().Trim();

		if ( name.Length() == 0 )
		{
			name = "Marcador " + (m_SelectedMarkerIndex + 1).ToString();
		}

		CustomMapMarkerData marker = m_CustomMarkers.Get(m_SelectedMarkerIndex);
		marker.m_Name = name;
		SaveMarkersToStorage();
		RenderMarkerList();
		ShowMarkerEditor(m_SelectedMarkerIndex);
	}

	private void DeleteSelectedMarker()
	{
		if ( m_SelectedMarkerIndex < 0 || m_SelectedMarkerIndex >= m_CustomMarkers.Count() )
		{
			return;
		}

		m_CustomMarkers.Remove(m_SelectedMarkerIndex);
		SaveMarkersToStorage();
		m_SelectedMarkerIndex = -1;
		HideMarkerEditor();
		RenderMarkerList();
		UpdateMarkerPinPositions();
	}

	private void PlaceMarkerEditor(CustomMapMarkerData marker)
	{
		if ( !m_MarkerEditor || !marker )
		{
			return;
		}

		float editor_x = m_OffsetX + (marker.m_X * m_Zoom) + 0.025;
		float editor_y = m_OffsetY + (marker.m_Y * m_Zoom) - 0.05;

		editor_x = Math.Clamp(editor_x, 0.02, 0.62);
		editor_y = Math.Clamp(editor_y, 0.02, 0.82);
		m_MarkerEditor.SetPos(editor_x, editor_y);
	}

	private void UpdateDragging(int mouse_x, int mouse_y, bool is_dragging)
	{
		if ( !m_Dragging )
		{
			return;
		}

		if ( !is_dragging )
		{
			StopDragging();
			return;
		}

		float width;
		float height;
		m_MapViewport.GetScreenSize(width, height);

		if ( width <= 0 || height <= 0 )
		{
			return;
		}

		m_OffsetX = m_DragStartOffsetX + ((mouse_x - m_DragStartMouseX) / width);
		m_OffsetY = m_DragStartOffsetY + ((mouse_y - m_DragStartMouseY) / height);

		ClampPan();
		ApplyMapTransform();
	}

	private void StopDragging()
	{
		if ( m_Dragging )
		{
			GetGame().GetDragQueue().RemoveCalls(this);
			m_Dragging = false;
		}
	}

	private void SetZoom(float zoom)
	{
		float old_zoom = m_Zoom;
		float center_x = (0.5 - m_OffsetX) / old_zoom;
		float center_y = (0.5 - m_OffsetY) / old_zoom;

		m_Zoom = Math.Clamp(zoom, ZOOM_MIN, ZOOM_MAX);
		m_OffsetX = 0.5 - (center_x * m_Zoom);
		m_OffsetY = 0.5 - (center_y * m_Zoom);

		ClampPan();
		ApplyMapTransform();
	}

	private void CenterOnPlayer()
	{
		PlayerBase player = GetGame().GetPlayer();

		if ( !player )
		{
			ApplyMapTransform();
			return;
		}

		vector pos = player.GetPosition();
		float map_x = Math.Clamp(pos[0] / MAP_WORLD_SIZE, 0.0, 1.0);
		float map_y = Math.Clamp(1.0 - (pos[2] / MAP_WORLD_SIZE), 0.0, 1.0);

		m_OffsetX = 0.5 - (map_x * m_Zoom);
		m_OffsetY = 0.5 - (map_y * m_Zoom);

		ClampPan();
		ApplyMapTransform();
	}

	private void ClampPan()
	{
		if ( m_Zoom <= 1.0 )
		{
			m_OffsetX = 0.0;
			m_OffsetY = 0.0;
			return;
		}

		float min_offset = 1.0 - m_Zoom;
		m_OffsetX = Math.Clamp(m_OffsetX, min_offset, 0.0);
		m_OffsetY = Math.Clamp(m_OffsetY, min_offset, 0.0);
	}

	private void ApplyMapTransform()
	{
		if ( m_MapContent )
		{
			m_MapContent.SetSize(m_Zoom, m_Zoom);
			m_MapContent.SetPos(m_OffsetX, m_OffsetY);
		}

		if ( m_ZoomText )
		{
			float percent = Math.Round(m_Zoom * 100.0);
			m_ZoomText.SetText("Zoom: " + percent.ToString() + "%");
		}

		UpdateMarkerPinPositions();

		if ( m_SelectedMarkerIndex >= 0 && m_SelectedMarkerIndex < m_CustomMarkers.Count() && m_MarkerEditor && m_MarkerEditor.IsVisible() )
		{
			PlaceMarkerEditor(m_CustomMarkers.Get(m_SelectedMarkerIndex));
		}
	}

	private void UpdateMarkerPinPositions()
	{
		for ( int i = 0; i < m_MarkerPins.Count(); i++ )
		{
			if ( i < m_CustomMarkers.Count() )
			{
				UpdateMarkerPinPosition(i);
			}
			else
			{
				Widget hidden_pin = m_MarkerPins.Get(i);

				if ( hidden_pin )
				{
					hidden_pin.Show(false);
				}
			}
		}
	}

	private void UpdateMarkerPinPosition(int index)
	{
		if ( index < 0 || index >= m_CustomMarkers.Count() )
		{
			return;
		}

		if ( index >= m_MarkerPins.Count() )
		{
			return;
		}

		Widget pin = m_MarkerPins.Get(index);

		if ( !pin )
		{
			return;
		}

		if ( !s_ShowMap )
		{
			pin.Show(false);
			return;
		}

		CustomMapMarkerData marker = m_CustomMarkers.Get(index);
		Widget label_bg = pin.FindAnyWidget("CustomMapMarkerLabelBg");
		if ( label_bg )
		{
			label_bg.Show(true);
			label_bg.SetColor(0xCC061326);
		}

		TextWidget label = GetMarkerLabel(pin);
		if ( label )
		{
			label.Show(true);
			label.SetColor(0xFFFFFFFF);
			label.SetText(GetMarkerLabelText(marker));
		}

		float pin_x = m_OffsetX + (marker.m_X * m_Zoom) - 0.02;
		float pin_y = m_OffsetY + (marker.m_Y * m_Zoom) - 0.025;

		if ( pin_x < -0.04 || pin_y < -0.05 || pin_x > 1.0 || pin_y > 1.0 )
		{
			pin.Show(false);
			return;
		}

		pin.Show(true);
		pin.SetPos(pin_x, pin_y);
	}

	private vector GetMarkerWorldPosition(CustomMapMarkerData marker)
	{
		if ( !marker )
		{
			return Vector(0, 0, 0);
		}

		float world_x = marker.m_X * MAP_WORLD_SIZE;
		float world_z = (1.0 - marker.m_Y) * MAP_WORLD_SIZE;
		float world_y = GetGame().SurfaceY(world_x, world_z);
		return Vector(world_x, world_y, world_z);
	}

	private string GetMarkerDistanceText(CustomMapMarkerData marker)
	{
		PlayerBase player = GetGame().GetPlayer();

		if ( !player || !marker )
		{
			return "";
		}

		vector marker_pos = GetMarkerWorldPosition(marker);
		vector player_pos = player.GetPosition();
		marker_pos[1] = player_pos[1];
		return FormatCustomMarkerDistance(vector.Distance(player_pos, marker_pos));
	}

	private string GetMarkerLabelText(CustomMapMarkerData marker)
	{
		if ( !marker )
		{
			return "";
		}

		string distance_text = GetMarkerDistanceText(marker);

		if ( distance_text.Length() == 0 )
		{
			return marker.m_Name;
		}

		return marker.m_Name + "  " + distance_text;
	}

	private void UpdatePlayerMarker()
	{
		PlayerBase player = GetGame().GetPlayer();

		if ( !player )
		{
			if ( m_PlayerMarker )
			{
				m_PlayerMarker.Show(false);
			}

			if ( m_PlayerMarkerOverlay )
			{
				m_PlayerMarkerOverlay.Show(false);
			}

			if ( m_PlayerMarkerNameBg )
			{
				m_PlayerMarkerNameBg.Show(false);
			}

			if ( m_PlayerMarkerName )
			{
				m_PlayerMarkerName.Show(false);
			}

			return;
		}

		vector pos = player.GetPosition();
		float map_x = Math.Clamp(pos[0] / MAP_WORLD_SIZE, 0.0, 1.0);
		float map_y = Math.Clamp(1.0 - (pos[2] / MAP_WORLD_SIZE), 0.0, 1.0);

		if ( m_PlayerMarkerOverlay )
		{
			float marker_x = m_OffsetX + (map_x * m_Zoom) - 0.02;
			float marker_y = m_OffsetY + (map_y * m_Zoom) - 0.0275;

			marker_x = Math.Clamp(marker_x, 0.0, 0.96);
			marker_y = Math.Clamp(marker_y, 0.0, 0.945);

			m_PlayerMarkerOverlay.Show(true);
			m_PlayerMarkerOverlay.SetPos(marker_x, marker_y);

			if ( m_PlayerMarkerNameBg )
			{
				float name_x = Math.Clamp(marker_x + 0.04, 0.0, 0.93);
				float name_y = Math.Clamp(marker_y + 0.016, 0.0, 0.96);
				m_PlayerMarkerNameBg.Show(true);
				m_PlayerMarkerNameBg.SetPos(name_x, name_y);
			}

			if ( m_PlayerMarkerName )
			{
				m_PlayerMarkerName.Show(true);
				m_PlayerMarkerName.SetText("ME");
			}
		}

		if ( m_PositionText )
		{
			float pos_x = Math.Round(pos[0]);
			float pos_z = Math.Round(pos[2]);
			m_PositionText.SetText("Pos: " + pos_x.ToString() + " / " + pos_z.ToString());
		}
	}
}
