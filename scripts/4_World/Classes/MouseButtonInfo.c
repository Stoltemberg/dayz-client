class MouseButtonInfo
{
	private int		m_ButtonID;
	private int		m_TimeLastPress;
	private int		m_TimeLastRelease;
	
	void MouseButtonInfo(int button)
	{
		m_ButtonID			= button;
		m_TimeLastPress		= -1;
		m_TimeLastRelease	= -1;
	}
	
	int GetButtonID()
	{
		return m_ButtonID;
	}
	
	int GetTimeLastPress()
	{
		return m_TimeLastPress;
	}
	
	int GetTimeLastRelease()
	{
		return m_TimeLastRelease;
	}
	
	void Press()
	{
		m_TimeLastPress = GetGame().GetTime();
	}
	
	void Release()
	{
		m_TimeLastRelease = GetGame().GetTime();
	}

	void ForceRelease()
	{
		int time_now = GetGame().GetTime();
		m_TimeLastPress = time_now;
		m_TimeLastRelease = time_now;
	}
	
	bool IsButtonDown()
	{
		if ( m_TimeLastRelease < m_TimeLastPress )
		{
			return true;
		}
		
		return false;
	}
}
