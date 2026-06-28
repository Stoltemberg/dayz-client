class Health: ModifierBase
{
	private float	m_LastHealthLevel;
	private float	m_LastBloodLevel;
	
	
	void Init()
	{
		m_TrackActivatedTime				= false;
		m_ID 							= MDF_HEALTH;
		float	m_TickIntervalInactive 	= DEFAULT_TICK_TIME_INACTIVE;
		float	m_TickIntervalActive 	= DEFAULT_TICK_TIME_ACTIVE;
	}
	
	private bool ActivateCondition(PlayerBase player)
	{
		return true;
	}


	private bool DeactivateCondition(PlayerBase player)
	{
		return false;
	}

	private bool OnTick(PlayerBase player, float deltaT)
	{	
		
		float blood =  player.GetHealth("GlobalHealth", "Blood");
		float health = player.GetHealth("GlobalHealth", "Health");	

		float healthdelta  = Math.AbsInt(health - m_LastHealthLevel);
		if (health <  m_LastHealthLevel) healthdelta = -healthdelta;
		//if( player.m_NotifiersManager ) player.m_NotifiersManager.FindNotifier(NTF_LIVES).DisplayTendency(healthdelta);
		m_LastHealthLevel = health;
		
		float blooddelta  = Math.AbsInt(blood - m_LastBloodLevel);
		if (blood <  m_LastBloodLevel) blooddelta = -blooddelta;
		//if( player.m_NotifiersManager ) player.m_NotifiersManager.FindNotifier(NTF_BLOOD).DisplayTendency(blooddelta);
		m_LastBloodLevel = blood;

		DisplayVitalStatus( player, health, blood, healthdelta, blooddelta );
		
	}

	private void DisplayVitalStatus(PlayerBase player, float health, float blood, float healthdelta, float blooddelta)
	{
		DisplayStatus display_status = player.GetDisplayStatus();
		if ( !display_status )
		{
			return;
		}

		display_status.SetStatus(DELM_NTFR_BLOOD, GetVitalStatusLevel( blood, player.GetMaxHealth("GlobalHealth","Blood") ) );
		display_status.SetStatus(DELM_NTFR_HEALTHY, GetVitalStatusLevel( health, player.GetMaxHealth("GlobalHealth","Health") ) );
		display_status.SetStatus(DELM_TDCY_BLOOD, GetVitalTendency( blooddelta ) );
		display_status.SetStatus(DELM_TDCY_HEALTH, GetVitalTendency( healthdelta ) );
	}

	private int GetVitalStatusLevel(float value, float max)
	{
		if ( max <= 0 )
		{
			return DELM_LVL_1;
		}

		float ratio = value / max;
		if ( ratio <= 0.15 )
		{
			return DELM_LVL_1;
		}
		else if ( ratio <= 0.4 )
		{
			return DELM_LVL_2;
		}
		else if ( ratio <= 0.75 )
		{
			return DELM_LVL_3;
		}
		else if ( ratio <= 0.9 )
		{
			return DELM_LVL_4;
		}

		return DELM_LVL_5;
	}

	private int GetVitalTendency(float delta)
	{
		if ( delta < -2 )
		{
			return TENDENCY_DEC_HIGH;
		}
		else if ( delta < -1 )
		{
			return TENDENCY_DEC_MED;
		}
		else if ( delta < 0 )
		{
			return TENDENCY_DEC_LOW;
		}
		else if ( delta > 2 )
		{
			return TENDENCY_INC_HIGH;
		}
		else if ( delta > 1 )
		{
			return TENDENCY_INC_MED;
		}
		else if ( delta > 0 )
		{
			return TENDENCY_INC_LOW;
		}

		return TENDENCY_STABLE;
	}
};
