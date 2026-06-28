
static string DataBase_GetDate()
{
	int year, month, day, hour, minute, second;
	
	GetYearMonthDay(year, month, day);
	
	GetHourMinuteSecond(hour, minute, second);

//	string date = itoal(month, 2) + "-" + itoal(day, 2) + "-" + itoal(hour, 2) + itoal(minute, 2);   (not worced code 0.61)
	string date = month.ToStringLen(2) + "-" + day.ToStringLen(2) + "-" + hour.ToStringLen(2) + minute.ToStringLen(2);
	
	return date;
}


static void DataBase_CreateDir()
{
	Print("Database CreateDir");
	
	MakeDirectory(DataBase_BASE_DIR);

	MakeDirectory(DataBase_BASE_DIR + DataBase_ALIVE_DIR);

	MakeDirectory(DataBase_BASE_DIR + DataBase_DEAD_DIR);
	
}



static void DataBaseDelete(string in1, string in2)
{
	string player_alive, player_dead, file_name, date;
	
	MakeDirectory(DataBase_BASE_DIR + DataBase_DEAD_DIR + in2);
	
	file_name = "$FILENAME$.sqf";
	
	//strrep(file_name, "$FILENAME$", in1);  (not worced code 0.61)
	file_name.Replace("$FILENAME$", in1);
	
	date = DataBase_GetDate();
	
	player_alive = DataBase_BASE_DIR + DataBase_ALIVE_DIR + in2 + "\\" + file_name;
	
	player_dead = DataBase_BASE_DIR + DataBase_DEAD_DIR + in2 + "\\" + file_name;
	
	//FileHandle file = OpenFile(player_alive, FILEMODE_READ);  (not worced code 0.61)
	FileHandle file = OpenFile(player_alive, FileMode.READ);
	
	if (file != 0)  {   
		
		CloseFile(file);

		CopyFile(player_alive, player_dead); 	
		
		DeleteFile(player_alive);
	} 
}



static void DataBaseWrite(string in1, string in2, string in3)
{	
	string player_alive, file_name;

	file_name = "$FILENAME$.sqf";
	
	//strrep(file_name, "$FILENAME$", in1);    (not worced code 0.61)
	file_name.Replace("$FILENAME$", in1);
	
	player_alive = DataBase_BASE_DIR + DataBase_ALIVE_DIR + in2 + "\\" + file_name;
			
	//FileHandle file = OpenFile(player_alive, FILEMODE_WRITE);     (not worced code 0.61)
	FileHandle file = OpenFile(player_alive, FileMode.WRITE);

	if (file != 0) {
		
	/*	// strrep(in3, "<null>", "[]");
		
		strrep(in3, "<null>", "\'" + "\'");
		
		// strrep(in3, "\"","\'");*/ //  (not worced code 0.61)
		
		in3.Replace("<null>", "\'" + "\'");
		
		FPrintln(file, in3);
		
		CloseFile(file);

	}  else {
		
		MakeDirectory(DataBase_BASE_DIR + DataBase_ALIVE_DIR + in2 );
		
	}
	
}



static string DataBaseRead(string in1, string in2)
{
	string player_alive, file_content, file_name;
	
	file_content = "";
	
	file_name = "$FILENAME$.sqf";
	
	//strrep(file_name, "$FILENAME$", in1);    (not worced code 0.61)
	file_name.Replace("$FILENAME$", in1);
	
	player_alive = DataBase_BASE_DIR + DataBase_ALIVE_DIR + in2 + "\\" + file_name;
		
	//FileHandle file = OpenFile(player_alive, FILEMODE_READ);     (not worced code 0.61)
	FileHandle file = OpenFile(player_alive, FileMode.READ);
	
	if (file != 0)  {  
	
		FGets(file, file_content);
		
		CloseFile(file);
	}
	
	return file_content;
}





