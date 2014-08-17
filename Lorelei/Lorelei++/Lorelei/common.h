#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <vector>

using namespace std;

class Connection //Класс связей
{
public:
    string keyword, handler; //Ключевое слово и название обработчика
    string obj_refs_str; //Вектор связей ключевого слова с объектами

    Connection ()
    {   }

    Connection (string kw) : keyword(kw) //Инициализируем ключевое слово
    {   }

    Connection* AddConnection(int id)
    {
        //obj_refs.push_back(id); //Добавляем связь в вектор связей
        return this; //Возвращаем указатель для текущий объект, все для того, чтоб выстроить цепочку запросов
    }

    Connection* SetHandler(string fun)
    {
        handler = fun; //Устанавливаем обработчик ключевого слова
        return this; //Тоже указатель возвращаем, для цепочки, удобно ведь
    }
};

struct token_t //Структура лексемы
{
    enum toktype {Word, Punct, None}; //Перечисление, определяющее тип лексемы

    string term; //Терм - строковое представление лексемы
    toktype type; //Тип
};

struct command_t //Структура команды
{
    bool is_object = false; //Объект ли это
    Connection base; //Объект связи

    vector<command_t> args; //Аргументы связи
};

typedef vector<token_t> sentence_t; //Создаем алиас, чтоб писать проще было

typedef unsigned char byte; //Аналогично

#endif // COMMON_H
