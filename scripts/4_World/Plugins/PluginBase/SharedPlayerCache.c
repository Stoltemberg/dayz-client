// [2026-06-30] FEATURE: [3.5.2] Cache unificado de jogadores online
// Módulo compartilhado para evitar duplicação entre PluginParty, PluginAdmin, PluginCustomChat

class SharedPlayerCache
{
	private static autoptr array<string> s_UIDs = new array<string>;
	private static autoptr array<string> s_Names = new array<string>;
	private static autoptr array<PlayerIdentity> s_Identities = new array<PlayerIdentity>;
	private static float s_LastRefresh = 0;
	private static const float CACHE_TTL = 5.0; // 5 segundos
	
	static void Refresh()
	{
		float now = GetGame().GetTime();
		if (now - s_LastRefresh < CACHE_TTL * 1000)
		{
			return; // Cache ainda válido
		}
		
		s_UIDs.Clear();
		s_Names.Clear();
		s_Identities.Clear();
		
		array<PlayerIdentity> identities = new array<PlayerIdentity>;
		GetGame().GetPlayerIndentities(identities);
		
		for (int i = 0; i < identities.Count(); i++)
		{
			PlayerIdentity identity = identities.Get(i);
			if (identity)
			{
				s_Identities.Insert(identity);
				s_UIDs.Insert(identity.GetId());
				s_Names.Insert(identity.GetName());
			}
		}
		
		s_LastRefresh = now;
	}
	
	static int GetCount()
	{
		Refresh();
		return s_UIDs.Count();
	}
	
	static string GetUID(int index)
	{
		Refresh();
		if (index >= 0 && index < s_UIDs.Count())
		{
			return s_UIDs.Get(index);
		}
		return "";
	}
	
	static string GetName(int index)
	{
		Refresh();
		if (index >= 0 && index < s_Names.Count())
		{
			return s_Names.Get(index);
		}
		return "";
	}
	
	static PlayerIdentity GetIdentity(int index)
	{
		Refresh();
		if (index >= 0 && index < s_Identities.Count())
		{
			return s_Identities.Get(index);
		}
		return NULL;
	}
	
	static int FindIndexByUID(string uid)
	{
		Refresh();
		for (int i = 0; i < s_UIDs.Count(); i++)
		{
			if (s_UIDs.Get(i) == uid)
			{
				return i;
			}
		}
		return -1;
	}
	
	static string GetNameByUID(string uid)
	{
		int idx = FindIndexByUID(uid);
		if (idx >= 0)
		{
			return s_Names.Get(idx);
		}
		return "";
	}
	
	static PlayerIdentity GetIdentityByUID(string uid)
	{
		int idx = FindIndexByUID(uid);
		if (idx >= 0)
		{
			return s_Identities.Get(idx);
		}
		return NULL;
	}
	
	static void Invalidate()
	{
		s_LastRefresh = 0;
	}
}
