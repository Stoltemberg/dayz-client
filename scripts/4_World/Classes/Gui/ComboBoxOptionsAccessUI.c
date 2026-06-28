class ComboBoxOptionsAccessUI extends OptionsAccessUI
{
	string GetDisplayText(string text)
	{
		if (text == "French")
		{
			return "Portugues";
		}

		return text;
	}

	void Init()
	{
		ListOptionsAccess list = (ListOptionsAccess)m_optionAccess;
		XComboBoxWidget combo = (XComboBoxWidget)m_widget;

		if (list && combo)
		{
			combo.ClearAll();

			string text;
			int c = list.GetItemsCount();
			for (int i = 0; i < c; i++)
			{
				list.GetItemText(i, text);
				combo.AddItem(GetDisplayText(text));
			}
		}
	}

	void Update()
	{
		ListOptionsAccess list = (ListOptionsAccess)m_optionAccess;
		XComboBoxWidget combo = (XComboBoxWidget)m_widget;

		if (list && combo)
		{
			int index = list.GetIndex();
			combo.SetCurrentItem(index);
			string text;
			list.GetItemText(index, text);
			combo.SetItem(index, GetDisplayText(text));
		}
	}

	void SetValue()
	{
		ListOptionsAccess list = (ListOptionsAccess)m_optionAccess;
		XComboBoxWidget combo = (XComboBoxWidget)m_widget;

		if (list && combo)
		{
			list.SetIndex(combo.GetCurrentItem());
			Update();
		}
	}

	bool OnChange(Widget w, int x, int y, bool finished)
	{
		super.OnChange(w, x, y, finished);
		
		SetValue();
		return false;
	}
}
