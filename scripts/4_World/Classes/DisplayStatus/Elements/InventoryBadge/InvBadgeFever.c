class InvBadgeFever extends DisplayElement
{
	void InvBadgeFever()
	{
		m_Colors[1] = 0xFFFF3030;
		
		m_Labels[1] = "FEVER";

		m_Key 	= 	NTFKEY_FEVERISH;
		m_Pos 	= 	NTFP_FEVERISH;
		m_Type	=	TYPE_NTFR;
	}
}

class InvBadgeTemperature extends DisplayElement
{
	void InvBadgeTemperature()
	{
		m_Colors[1] = 0xFFFF3030;
		m_Colors[2] = 0xFFFF6630;
		m_Colors[3] = 0xFFFFAA30;
		m_Colors[4] = 0xFF6090FF;
		m_Colors[5] = 0xFF3060FF;
		m_Colors[6] = 0xFF3030FF;

		m_Labels[1] = "HYPERTHERMIC";
		m_Labels[2] = "HOT";
		m_Labels[3] = "OVERHEATING";
		m_Labels[4] = "COLD";
		m_Labels[5] = "FREEZING";
		m_Labels[6] = "HYPOTHERMIC";

		m_Key 	= 	NTFKEY_WARMTH;
		m_Pos 	= 	NTFP_WARMTH;
		m_Type	=	TYPE_NTFR;
	}
}
