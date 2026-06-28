//расположение сохранений. Работают только "$profile:" и "$saves:" 
// "$profile:" - папка с cfg сервера
// "$saves:" - папка с cfg сервера\Users\Server
//	пример: static const string DataBase_BASE_DIR = "$saves:PlayerSaves";
static const string DataBase_BASE_DIR = "$profile:PlayerSaves";
// Имя папки с живыми на сервере персонажами (не забываем про \\ для указания подпапки)
static const string DataBase_ALIVE_DIR = "\\alive\\";
// Имя папки с последними мертвыми на сервере персонажами
static const string DataBase_DEAD_DIR =  "\\dead\\";
