#ifndef LORELEI_H
#define LORELEI_H

#include <cstring>
#include <functional>
#include <sqlite3.h>
#include <cstdlib>
#include "common.h"

class Lorelei //Лора, класс
{
    sqlite3 *_lorelei_db;

    string _name_short = "лора";
    string _name_long = "лорелей";

    string _db_path;

    string _version = "indev-2";

    string _request; //Строка запроса

    vector<command_t> _current_commands; //Вектор текущих команд

    function<void (const string&)> dp; //Функциональный обект - обертка над лямбдой - указатель на отладочную ф-цию вывода на экран
    function<void (const string&)> err;

public:
    Lorelei();
    ~Lorelei();

    void Init();

    void ClearCommands(); //Очистка вектора текущих команд

    string GetShortName();

    string GetLongName();

    string GetDBPath();

    string GetVersion();

    void SetDBPath(string db_path);

    void Parse(string request); //Парсинг запроса

    void SetDebugPrinter(function<void (const string&)> dp); //Устанавливаем ф-цию вывода на экран
    void SetErrorPrinter(function<void (const string&)> err);

    void PrintResults(); //Вывод результатов обработки запроса

private:
    sentence_t tokenize(); //Перевод строкового запроса в вектор лексем

    void analyze(sentence_t& sentence); //Вход в анализатор

    void eval(sentence_t &sentence, int current_id, bool arg_search=false, command_t *last_com=nullptr); //Рекурсивная ф-ция анализа лексем

    bool isName(token_t &current);
    bool isWeakword(token_t &current);
    bool isKeyword(token_t &current);

    command_t getCommand(token_t &current); //Возвращаем команду по лексеме

    void printCommand(command_t com); //Печать вектора команд

    bool isCyrillicLetter(char ch); //Проверяем, является ли символ кириллической буквой

    char toLowerCyrillic(char ch); //Занижаем

    /* callback'и для SQL */
    static int checkCallback(void *found, int argc, char **argv, char **columnName);
    static int commandCallback(void *com_ptr, int argc, char **argv, char **columnName);
};


#endif // LORELEI_H
