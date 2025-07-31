#include <tgbot/tgbot.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
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

// TODO Состояние ожидания у каждой команды

int main() {
    Database db("../db/notes.db");

    std::ostringstream response;
    FileReader(response , "../src/help_response.txt");
    std::ostringstream token;
    FileReader(token , "../build/token.txt");
    

    TgBot::Bot bot(token.str());
    std::vector<TgBot::BotCommand::Ptr> commands;
    CommandAddition(bot , commands ,"start" , "Запустить бота");
    CommandAddition(bot , commands ,"add" , "Добавить новую заметку");
    CommandAddition(bot , commands , "list" , "Показать все заметки");
    CommandAddition(bot , commands , "delete" , "Удалить заметку");
    CommandAddition(bot , commands , "clear" , "Удалить все заметки");
    CommandAddition(bot , commands , "help" , "Просмотр справки");
    bot.getApi().setMyCommands(commands);


    std::string title;
    std::string body;
    std::vector<Note> usernotes;



    bot.getEvents().onCommand("start", [&bot , &response](TgBot::Message::Ptr msg) {
        awaitingText[msg->chat->id] = false;
        UserStates[msg->chat->id] = UserState::NONE;
        bot.getApi().sendMessage( msg->chat->id, "Привет, "+ msg->chat->firstName +"!\nЯ бот-хранитель твоих заметок!");
        bot.getApi().sendMessage(msg->chat->id , response.str() );
    });

    bot.getEvents().onCommand("add" , [&bot](TgBot::Message::Ptr msg)
    {
        awaitingText[msg->chat->id] = true;
        bot.getApi().sendMessage(msg->chat->id, "Напиши заголовок заметки");
        UserStates[msg->chat->id] = UserState::WAITING_TITLE;
    });

    bot.getEvents().onCommand("list" , [&bot , &db , &usernotes](TgBot::Message::Ptr msg){
        db.getNotes( msg->chat->id , usernotes);
        int len = usernotes.size();
        if( len > 0 )
        {
            std::string meganote;
            meganote += "Список твои заметок:\n\n";
            for(int i = 0 ; i < len ; i++ )
            {
                meganote += std::to_string(i+1) + ". " + usernotes[i].title + " - " + usernotes[i].body+"\n";
            }
            bot.getApi().sendMessage(msg->chat->id , meganote);
            meganote.clear();
        }
        else
        {
            bot.getApi().sendMessage(msg->chat->id , "Твой список заметок пуст");
        }
        usernotes.clear();
        
    });

    bot.getEvents().onCommand("delete" , [&bot , &db , &usernotes](TgBot::Message::Ptr msg){
        awaitingText[msg->chat->id] = false;
        UserStates[msg->chat->id] = UserState::NONE;
        db.getNotes( msg->chat->id , usernotes);
        int len = usernotes.size();
        if( len > 0)
        {
            TgBot::InlineKeyboardMarkup::Ptr keyboard(new TgBot::InlineKeyboardMarkup);
            for( const auto& note : usernotes)
            {
                TgBot::InlineKeyboardButton::Ptr button( new TgBot::InlineKeyboardButton);
                button->text = note.title;
                button->callbackData = note.title;
                keyboard->inlineKeyboard.push_back( {button} );
            }
            bot.getApi().sendMessage(msg->chat->id , "Выбери заголовок заметки для удаления" , 0 , 0 , keyboard);
        }
        else
        {
            bot.getApi().sendMessage(msg->chat->id , "Твой список заметок пуст");
        }
        usernotes.clear();
    });

    bot.getEvents().onCallbackQuery([&bot , &db , &usernotes](TgBot::CallbackQuery::Ptr query) {
        awaitingText[query->message->chat->id] = false;
        UserStates[query->message->chat->id] = UserState::NONE;
        db.getNotes(query->message->chat->id , usernotes);
        bool deleted = false;
        for( auto& note : usernotes )
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
        usernotes.clear();
    });

    bot.getEvents().onCommand("clear" , [&bot , &db](TgBot::Message::Ptr msg){
        bot.getApi().sendMessage(msg->chat->id , "Все заметки успешно удалены!");
        db.clearNotes(msg->chat->id);
    });

    bot.getEvents().onCommand("help" , [&bot , &response](TgBot::Message::Ptr msg){
        bot.getApi().sendMessage(msg->chat->id , response.str() );
    });

    bot.getEvents().onAnyMessage([&bot , &commands , &title , &body , &db](TgBot::Message::Ptr msg) {
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
                title = msg->text;
                bot.getApi().sendMessage(msg->chat->id , "Введи текст заметки");
                UserStates[msg->chat->id] = UserState::WAITING_BODY;
                return;
            }
            else if( UserStates[msg->chat->id] == UserState::WAITING_BODY)
            {
                body = msg->text;
                db.addNote(msg->chat->id , title , body);
                bot.getApi().sendMessage(msg->chat->id , "Заметка сохранена");
                UserStates[msg->chat->id] = UserState::NONE;
                awaitingText[msg->chat->id] = false;
                title.clear();
                body.clear();
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