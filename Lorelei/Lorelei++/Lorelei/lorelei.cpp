#include <iostream>
#include <sstream>
#include "lorelei.h"

Lorelei::Lorelei() : _db_path(getenv("HOME") + string("/lorelei.db")), //Инициализируем путь к БД...
    err([](const string& e){cerr << e;}) //...и поток вывода ошибок перекидываем на stderr
{

}

Lorelei::~Lorelei()
{
    sqlite3_close(_lorelei_db); //Закрываем БД
}

void Lorelei::Init()
{
    int error = sqlite3_open(_db_path.c_str(), &_lorelei_db); //Открываем БД

    if(error) //Если не удалось открыть
    {
       err("Can't open database: ");
       err(sqlite3_errmsg(_lorelei_db));
    }
}

void Lorelei::ClearCommands()
{
    _current_commands.clear(); //Отчищаем список текущих команд, иначе будет путаница при следующем надоре предложения
}

string Lorelei::GetShortName()
{
    return _name_short; //Возвращаем короткое имя (Лора)
}

string Lorelei::GetLongName()
{
    return _name_long; //Возвращаем длинное имя (Лорелей)
}

void Lorelei::Parse(string request)
{
    _request = request; //Присваеваем внутреннему полю _request значение аргумента, чтоб дальше оперировать только с вн. полем

    auto request_vect = tokenize(); //Преобразуем запрос в массив (вектор) токенов (слов)
    analyze(request_vect); //Начинаем рекурсивный анализ запроса
}

void Lorelei::SetDebugPrinter(function<void (const string &)> dp)
{
    this->dp = dp; //Сохраняем ф-цию (лямбду, при желании) для вывода отладочных сообщений
}

void Lorelei::SetErrorPrinter(function<void (const string &)> err)
{
    this->err = err; //Устанавливаем поток вывода ошибок, аналогично с dp
}

void Lorelei::PrintResults() {
    for(command_t com : _current_commands)
        printCommand(com); //Рекурсивно печатаем вектор текущих команд
    dp("\n");
}

string Lorelei::GetDBPath()
{
    return _db_path; //Возвращаем путь к БД
}

string Lorelei::GetVersion()
{
    return _version; //Возвращаем версию
}

void Lorelei::SetDBPath(string db_path)
{
    _db_path = db_path; //Устанавливаем путь к БД
}

void Lorelei::printCommand(command_t com)
{
    dp(com.base.handler + " ( "); //Вывод имени ф-ции связанной с ключевым словом
    for(command_t arg : com.args) //Перебираем аргументы ключевого слова
    {
        if(!arg.is_object)
            printCommand(arg); //Если аргумент - другое ключевое слово, то печатаем его рекурсивно
        else
        {
            dp(arg.base.keyword + " "); //Иначе просто печатаем аргумент
        }
    }
    dp(")");
}

sentence_t Lorelei::tokenize()
{
    sentence_t result; //Предложение - вектор токенов - результат обработки
    token_t current_token; //Текущая лексема, используется в цикле анализа

    for(auto symbol = _request.begin(); symbol != _request.end(); ++symbol) //Цикл анализа запроса,
        //С помощью итератора проходимся по каждому символу запроса
    {
        if(*symbol == (char)208 || *symbol == (char)209) //Если код символа 208 или 209,
            //мы понимаем, что перед нами двухбайтный кирилличный символ
        {
            if(isCyrillicLetter(*++symbol)) //Если символ - кириллическая буква, анализируем последовательность как слово
            {
                if(current_token.type == token_t::Punct) //Если предыдущая лексема - знак пунктуации...
                    current_token.term.clear(); //...то очищаем слово

                current_token.type = token_t::Word; //Ставим тип лексемы, как слово

                if((byte)*symbol >= (byte)'А' && (byte)*symbol <= (byte)'Я') //Если буква - заглавная
                {
                    if((byte)*symbol <= (byte)'П') //Предугадываем код будущей заниженной буквы
                        current_token.term += 208;
                    else
                        current_token.term += 209;
                    current_token.term += toLowerCyrillic(*symbol); //Занижаем символ
                }
                else
                {
                    //Дописываем кириллический символ к лексеме (2 байта)
                    current_token.term += *(symbol - 1); //Сначала - код, 208 или 209
                    current_token.term += *symbol; //А потом и сам символ
                }
                continue;
            }
        }

        if(isalpha(*symbol)) //Если символ - латинская буква
        {
            if(current_token.type == token_t::Punct) //Если прошлая лексема - знак пунктуации...
                current_token.term.clear(); //...то очищаем слово

            current_token.type = token_t::Word; //Ставим тип лексемы, как слово
            current_token.term += towlower(*symbol); //Занижаем символ
        }
        else if(ispunct(*symbol)) //Если символ - знак пунктуации, алгоритм аналогичен алгоритму для буквы-символа
        {
            if(current_token.type == token_t::Word)
            {
                result.push_back(current_token);
                current_token.term.clear();
            }

            current_token.type = token_t::Punct;
            current_token.term += *symbol;
        }
        else //Если символ - что-то вообще левое, например, пробел
        {
            if(current_token.type == token_t::Word) //Если текущая лексема - слово
                result.push_back(current_token); //Добавляем лексему к предложению

            current_token.term.clear(); //Очищаем текущую лексему
            current_token.type = token_t::None; //Ставим ее тип, как нифига
        }
    }

    if(current_token.type == token_t::Word) //Если последняя лексема - слово
        result.push_back(current_token); //Добавляем ее к предложению

    return result; //Возвращаем предложение
}

void Lorelei::analyze(sentence_t& sentence)
{
    if(sentence.size() == 0) //Если предложение пустое
        return; //То возвращаемся

    if(isName(sentence[0]) || isWeakword(sentence[0])) //Если слово слабое или имя
        sentence.erase(sentence.begin()); //Просто игнорим его и выкидываем нахрен из предложения, чтоб место не занимало

    eval(sentence, 0); //Анализируем предложение рекурсивно
}

void Lorelei::eval(sentence_t &sentence, int current_id, bool arg_search, command_t* last_com)
{
     /*
        sentence_t &sentence - предложение, вектор лексем
        int current_id       - индекс текущей лексемы в предложении
        bool arg_search      - ищем ли мы аргументы ключевого слова или же просто анализируем. По дефолту false - просто анализируем
        command_t* last_com  - последняя найденая команда, указатель, она нам еще понадобится
    */

    if(current_id == sentence.size()) //Если ИД текущего слова == конец предложения
        return; //Возвращаемся

    token_t &current = sentence[current_id]; //Выбираем лексему по ее ИД

    if(isName(current) || isWeakword(current)) //Если лексема - слабое слово
    {
        sentence.erase(sentence.begin() + current_id); //К черту его
        eval(sentence, current_id, arg_search, last_com); //Рекурсивно анализируем следующую лексему
        if(sentence.size() == 0)
            return;
    }

    if(isKeyword(current))  //Если ключевое слово
    {
        if(arg_search) //И если мы ищем аргументы
        {
            last_com->args.push_back(getCommand(current)); //Кидаем в аргументы последней команды найденную команду
            last_com = &last_com->args.back(); //И устанавливаем указатель последней команды на найденную команду
        }
        else
        {
            _current_commands.push_back(getCommand(current)); //Иначе просто кидаем в список текущих команд найденную
            last_com = &_current_commands.back(); //Устанавливаем указатель последней команды на найденную команду
        }
        sentence.erase(sentence.begin() + current_id); //Выкидываем текущую лексему из предложения

        eval(sentence, current_id, true, last_com); //Рекурсивно анализируем следующую лексему
    }

    if(arg_search) //Если не ключевое слово, но ищем аргументы
    {        
        /* Строка запроса: ищем, связан ли объект с ключевым словом */
        string sql_request = "SELECT * FROM objects WHERE id IN (" + last_com->base.obj_refs_str + ") AND name = '" + current.term + "'";
        char * sql_error_message;

        bool found = false; //Изначально - не связан

        /* Выполняем запрос и передаем callback-ф-цию и указатель на флаг поиска - found */
        int rc = sqlite3_exec(_lorelei_db, sql_request.c_str(), checkCallback, &found, &sql_error_message);
        if(rc != SQLITE_OK) //Если что-то не так пошло...
        {
            err(string("SQL error: \n") + sql_error_message); //Печатаем ошибку
            sqlite3_free(sql_error_message);
        }
        if(found) {
            command_t arg; //Аргумент
            arg.is_object = true; //Не является ключевым словом
            arg.base.keyword = current.term; //Устанавливаем слово аргумента, как объект
            last_com->args.push_back(arg); //Кидаем его в аргументы последнего ключевого слова
            sentence.erase(sentence.begin() + current_id); //Выкидываем лексему из предложения
            eval(sentence, current_id); //И снова искупаемся в рекурсии
        }
    }
}

bool Lorelei::isName(token_t &current)
{
    if(current.term == _name_long || current.term == _name_short) //Если лексема - имя
        return true; //То окей
    return false; //Иначе, не окей
}

bool Lorelei::isWeakword(token_t &current)
{
    /* Проверяем, является ли слово слабым - есть ли оно в таблице слабых слов */
    string sql_request = "SELECT word FROM weakwords WHERE word = '" + current.term + "'";
    char * sql_error_message;

    bool found = false; //Изначально - нет

    /* Выполняем запрос и передаем callback-ф-цию и указатель на флаг поиска - found */
    int rc = sqlite3_exec(_lorelei_db, sql_request.c_str(), checkCallback, &found, &sql_error_message);
    if(rc != SQLITE_OK)
    {
        err(string("SQL error: \n") + sql_error_message);
        sqlite3_free(sql_error_message);
    }

    return found;
}

bool Lorelei::isKeyword(token_t &current)
{
    /* Все аналогично ф-ции выше, но только с ключевыми словами */

    string sql_request = "SELECT keyword FROM commands WHERE keyword = '" + current.term + "'";
    char * sql_error_message;

    bool found = false;

    int rc = sqlite3_exec(_lorelei_db, sql_request.c_str(), checkCallback, &found, &sql_error_message);
    if(rc != SQLITE_OK)
    {
        err(string("SQL error: \n") + sql_error_message);
        sqlite3_free(sql_error_message);
    }

    return found;
}

command_t Lorelei::getCommand(token_t &current)
{
    command_t command; //Команда, вернем ее

    command.is_object = true; //Изначально помечаем команду не найденой

    /* Формируем запрос на поиск и извлечение команды */
    string sql_request = "SELECT * FROM commands WHERE keyword = '" + current.term + "'";
    char * sql_error_message;

    /* Выполняем запрос и передаем callback-ф-цию обработки найденой команды и указатель на команду */
    int rc = sqlite3_exec(_lorelei_db, sql_request.c_str(), commandCallback, &command, &sql_error_message);
    if(rc != SQLITE_OK)
    {
        err(string("SQL error: \n") + sql_error_message);
        sqlite3_free(sql_error_message);
    }

    if(command.is_object) //Не нашли команду
    {
        dp("Can't find connection ");
        dp(current.term);
        dp("\n");
    }
    return command;
}

bool Lorelei::isCyrillicLetter(char ch)
{
    //Тут все ясно - проверяем, буква ли это
    if((byte)ch >= (byte)'а' && (byte)ch <= (byte)'п')
        return true;
    if((byte)ch >= (byte)'р' && (byte)ch <= (byte)'я')
        return true;
    if((byte)ch >= (byte)'А' && (byte)ch <= (byte)'Я')
        return true;
    return false;
}

char Lorelei::toLowerCyrillic(char ch)
{
    char res = (byte)ch <= (byte)'П' ? char((byte)ch + 32) : char((byte)ch - 32); //Занижаем букву
    return res; //И возвращаем ее
}

int Lorelei::checkCallback(void *found, int argc, char **argv, char **columnName)
{
    /* Эта callback-ф-ция вызывается только если что-то было найдено, поэтому смело ставим флаг */

    *(bool*)found = true;

    return 0;
}

int Lorelei::commandCallback(void *com_ptr, int argc, char **argv, char **columnName)
{
    command_t* com = (command_t*)com_ptr; //Выбираем указатель на команду
    com->is_object = false; //Обозначаем, что команда обнаружена

    for(uint i = 0; i < argc; ++i) //Проходимся по столбцам таблицы
    {
        if(!strcmp(columnName[i], "keyword"))
            com->base.keyword = argv[i];
        else if(!strcmp(columnName[i], "handler"))
            com->base.handler = argv[i];
        else if(!strcmp(columnName[i], "refs")) {
            com->base.obj_refs_str = argv[i];
        }
    }

    return 0;
}
