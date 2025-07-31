#include <tgbot/tgbot.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <sqlite3.h>
#include <sqlite_modern_cpp.h>
#include "database.hpp"


void CommandAddition(TgBot::Bot& mybot , std::vector<TgBot::BotCommand::Ptr>& cmdstorage, const std::string title , const std::string descrp);
void FileReader(std::ostringstream& buffer , const std::string path);

template<typename T>
bool in(const T& value , std::initializer_list<T> list)
{
    return std::find( list.begin() , list.end() , value) != list.end()  ;
}

enum class UserState {
    NONE,
    WAITING_TITLE,
    WAITING_BODY,
};
std::unordered_map<int64_t , bool> awaitingText;
std::unordered_map<int64_t , UserState> UserStates;
std::unordered_map<int64_t, Note> TempNote;
std::unordered_map<int64_t, std::vector<Note>> UsersNotes;

int main() {
    Database db("../db/notes.db");

    std::ostringstream response;
    FileReader(response , "help_response.txt");
    std::ostringstream token;
    FileReader(token , "token.txt");
    

    TgBot::Bot bot(token.str());
    std::vector<TgBot::BotCommand::Ptr> commands;
    CommandAddition(bot , commands ,"start" , "Запустить бота");
    CommandAddition(bot , commands ,"add" , "Добавить новую заметку");
    CommandAddition(bot , commands , "list" , "Показать все заметки");
    CommandAddition(bot , commands , "delete" , "Удалить заметку");
    CommandAddition(bot , commands , "clear" , "Удалить все заметки");
    CommandAddition(bot , commands , "help" , "Просмотр справки");
    bot.getApi().setMyCommands(commands);





    bot.getEvents().onCommand("start", [&bot , &response](TgBot::Message::Ptr msg) {
        awaitingText[msg->chat->id] = false;
        UserStates[msg->chat->id] = UserState::NONE;
        bot.getApi().sendMessage(msg->chat->id, "Привет, "+ msg->chat->firstName +"!\nЯ бот-хранитель твоих заметок!");
        bot.getApi().sendMessage(msg->chat->id, response.str());
    });

    bot.getEvents().onCommand("add" , [&bot](TgBot::Message::Ptr msg)
    {
        awaitingText[msg->chat->id] = true;
        bot.getApi().sendMessage(msg->chat->id, "Напиши заголовок заметки");
        UserStates[msg->chat->id] = UserState::WAITING_TITLE;
    });

    bot.getEvents().onCommand("list" , [&bot , &db](TgBot::Message::Ptr msg){
        awaitingText[msg->chat->id] = false;
        UserStates[msg->chat->id] = UserState::NONE;
        db.getNotes( msg->chat->id , UsersNotes[msg->chat->id]);
        int len = UsersNotes[msg->chat->id].size();
        if( len > 0 )
        {
            std::string meganote;
            meganote += "Список твои заметок:\n\n";
            for(int i = 0 ; i < len ; i++ )
            {
                meganote += std::to_string(i+1) + ". " + UsersNotes[msg->chat->id][i].title + " - " + UsersNotes[msg->chat->id][i].body+"\n";
            }
            bot.getApi().sendMessage(msg->chat->id , meganote);
            meganote.clear();
        }
        else
        {
            bot.getApi().sendMessage(msg->chat->id , "Твой список заметок пуст");
        }
        UsersNotes[msg->chat->id].clear();
        
    });

    bot.getEvents().onCommand("delete" , [&bot , &db ](TgBot::Message::Ptr msg){
        awaitingText[msg->chat->id] = false;
        UserStates[msg->chat->id] = UserState::NONE;
        db.getNotes( msg->chat->id , UsersNotes[msg->chat->id]);
        int len = UsersNotes[msg->chat->id].size();
        if( len > 0)
        {
            TgBot::InlineKeyboardMarkup::Ptr keyboard(new TgBot::InlineKeyboardMarkup);
            for( const auto& note : UsersNotes[msg->chat->id])
            {
                TgBot::InlineKeyboardButton::Ptr button( new TgBot::InlineKeyboardButton);
                button->text = note.title;
                button->callbackData = note.title;
                keyboard->inlineKeyboard.push_back( {button} );
            }
            bot.getApi().sendMessage(msg->chat->id, "Выбери заголовок заметки для удаления", nullptr, nullptr, keyboard);
        }
        else
        {
            bot.getApi().sendMessage(msg->chat->id , "Твой список заметок пуст");
        }
        UsersNotes[msg->chat->id].clear();
    });

    bot.getEvents().onCallbackQuery([&bot , &db](TgBot::CallbackQuery::Ptr query) {
        awaitingText[query->message->chat->id] = false;
        UserStates[query->message->chat->id] = UserState::NONE;
        db.getNotes( query->message->chat->id , UsersNotes[query->message->chat->id]);
        bool deleted = false;
        for( auto& note : UsersNotes[query->message->chat->id] )
        {
            if( note.title == query->data )
            {
                db.clearNoteByTitle(query->message->chat->id , note.title);
                bot.getApi().sendMessage(query->message->chat->id , "Заметка успешно удалена!");
                deleted = true;
            }
        }
        if( !deleted )
        {
            bot.getApi().sendMessage(query->message->chat->id , "Такой заметки больше не существует");
        }
        deleted = false;
        UsersNotes[query->message->chat->id].clear();
    });

    bot.getEvents().onCommand("clear" , [&bot , &db](TgBot::Message::Ptr msg){
        awaitingText[msg->chat->id] = false;
        UserStates[msg->chat->id] = UserState::NONE;
        db.getNotes( msg->chat->id , UsersNotes[msg->chat->id]);
        if( UsersNotes[msg->chat->id].size() > 0)
        {
            bot.getApi().sendMessage(msg->chat->id , "Все заметки успешно удалены!");
            db.clearNotes(msg->chat->id);
        }
        else
        {
            bot.getApi().sendMessage(msg->chat->id , "Нет заметок для удаления");
        }

    });

    bot.getEvents().onCommand("help" , [&bot , &response](TgBot::Message::Ptr msg){
        awaitingText[msg->chat->id] = false;
        UserStates[msg->chat->id] = UserState::NONE;
        bot.getApi().sendMessage(msg->chat->id , response.str() );
    });

    bot.getEvents().onAnyMessage([&bot , &commands  , &db](TgBot::Message::Ptr msg) {
        for( const auto& cmd : commands)
        {
            if( "/" + cmd->command == msg->text)
            {
                return;
            }
        }
        if( awaitingText[msg->chat->id ] == true && in(UserStates[msg->chat->id] , {UserState::WAITING_TITLE , UserState::WAITING_BODY} ) )
        {
            if( UserStates[msg->chat->id] == UserState::WAITING_TITLE )
            {
                TempNote[msg->chat->id].title = msg->text;
                bot.getApi().sendMessage(msg->chat->id , "Введи текст заметки");
                UserStates[msg->chat->id] = UserState::WAITING_BODY;
                return;
            }
            else if( UserStates[msg->chat->id] == UserState::WAITING_BODY)
            {
                TempNote[msg->chat->id].body= msg->text;
                db.addNote(msg->chat->id , TempNote[msg->chat->id].title , TempNote[msg->chat->id].body);
                bot.getApi().sendMessage(msg->chat->id , "Заметка сохранена");
                UserStates[msg->chat->id] = UserState::NONE;
                awaitingText[msg->chat->id] = false;
                TempNote[msg->chat->id].title.clear();
                TempNote[msg->chat->id].body.clear();
                return;
            }
        }
        bot.getApi().sendMessage( msg->chat->id , "Неизвестная команда, используйте /help ");
    });
        

    try {
        TgBot::TgLongPoll longPoll(bot);
        std::cout << "Бот запущен!" << std::endl;
        while (true) longPoll.start();
    } catch (std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
    }
    return 0;
}

void CommandAddition(TgBot::Bot& mybot , std::vector<TgBot::BotCommand::Ptr>& cmdstorage, const std::string title , const std::string descrp)
{
    auto command = std::make_shared<TgBot::BotCommand>();
    command->command = title;
    command->description = descrp;
    cmdstorage.push_back(command);
}

void FileReader(std::ostringstream& buffer , const std::string path)
{
    std::ifstream file;
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        file.open(path);
        buffer << file.rdbuf();
        file.close();
    }
    catch( const std::exception& e)
    {
        std::cerr << "Ошибка: " << e.what() << std::endl;
    }
}