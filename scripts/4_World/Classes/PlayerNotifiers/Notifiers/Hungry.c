class Hungry: NotifierBase
{
	private const float	 	ENERGIZED_TRESHOLD			= 2000;
	private const float	 	HUNGRY_TRESHOLD 	 		= 1000;
	private const float	 	VERY_HUNGRY_TRESHOLD	 	= 500;
	private const float	 	FATALLY_HUNGRY_TRESHOLD 	= 0;
	private const float 	DEC_TRESHOLD_LOW 			= -0.3;
	private const float 	DEC_TRESHOLD_MED 			= -1.2;
	private const float 	DEC_TRESHOLD_HIGH			= -2;
	private const float 	INC_TRESHOLD_LOW 			= 0.1;
	private const float 	INC_TRESHOLD_MED 			= 1.2;
	private const float 	INC_TRESHOLD_HIGH			= 2;
	
	void Hungry(NotifiersManager manager)
	{
		m_MinPauseBetweenMessages 	= 3;
		m_MaxPauseBetweenMessages 	= 5;
		m_MinPauseBetweenSounds 	= 10;
		m_MaxPauseBetweenSounds 	= 12;
		m_MinPauseBetweenAnimations	= 10;
		m_MaxPauseBetweenAnimations	= 12;
	}

	int GetNotifierType()
	{
		return NTF_HUNGRY;
	}

	void DisplayTendency(float delta)
	{
		int tendency = CalculateTendency(delta, INC_TRESHOLD_LOW, INC_TRESHOLD_MED, INC_TRESHOLD_HIGH, DEC_TRESHOLD_LOW, DEC_TRESHOLD_MED, DEC_TRESHOLD_HIGH);
		GetDisplayStatus().SetStatus(DELM_TDCY_ENERGY,tendency);
	}

	private void DisplayBadge()
	{
		float energy = m_Player.GetStatEnergy().Get();
		if (energy <= FATALLY_HUNGRY_TRESHOLD)
		{
			GetDisplayStatus().SetStatus(DELM_NTFR_HUNGER,DELM_LVL_4);
		}
		else if (energy <= VERY_HUNGRY_TRESHOLD)
		{
			GetDisplayStatus().SetStatus(DELM_NTFR_HUNGER,DELM_LVL_3);
		}
		else if (energy <= HUNGRY_TRESHOLD)
		{
			GetDisplayStatus().SetStatus(DELM_NTFR_HUNGER,DELM_LVL_2);
		}
		else if (energy >= ENERGIZED_TRESHOLD) 
		{
			GetDisplayStatus().SetStatus(DELM_NTFR_HUNGER,DELM_LVL_1);
		}
		else
		{
			GetDisplayStatus().SetStatus(DELM_NTFR_HUNGER,DELM_LVL_0);
		}
	}

	private void HideBadge()
	{
		GetDisplayStatus().SetStatus(DELM_NTFR_HUNGER,DELM_LVL_0);
	}

	private void DisplayMessage()
	{
		float energy = m_Player.GetStatEnergy().Get();
		if ( energy <= FATALLY_HUNGRY_TRESHOLD )
		{
			m_Player.MessageImportant("I'm dying of starvation");
		}
		else if ( energy <= VERY_HUNGRY_TRESHOLD )
		{
			m_Player.MessageStatus("I'm extremely hungry");
		}
		else if ( energy <= HUNGRY_TRESHOLD )
		{
			m_Player.MessageStatus("My stomach grumbles");
		}
	}

	private void PlayAnimation()
	{
		float energy = m_Player.GetStatEnergy().Get();
		if ( energy <= FATALLY_HUNGRY_TRESHOLD )
		{
			//PrintString(GetName() + " hunger dying animation");
		}
		else if ( energy <= VERY_HUNGRY_TRESHOLD )
		{
			//PrintString(GetName() + " grumbly animation");
		}
	}

	private void PlaySound()
	{
		//GetGame().CreateSoundOnObject(m_Player, "Character_Mad", 3, false);
	}
	
	protected float GetObservedValue()
	{
		return m_Player.GetStatEnergy().Get();
	}
};
