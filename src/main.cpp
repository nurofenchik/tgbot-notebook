#include <tgbot/tgbot.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <sqlite3.h>

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






    bot.getEvents().onCommand("start", [&bot , &response](TgBot::Message::Ptr msg) {
        awaitingText[msg->chat->id] = false;
        UserStates[msg->chat->id] = UserState::none;
        bot.getApi().sendMessage( msg->chat->id, "Привет,  "+ msg->chat->firstName +"!\nЯ бот-хранитель твоих заметок!");
        bot.getApi().sendMessage(msg->chat->id , response.str() );
    });

    bot.getEvents().onCommand("add" , [&bot](TgBot::Message::Ptr msg)
    {
        awaitingText[msg->chat->id] = true;
        bot.getApi().sendMessage(msg->chat->id, "Напиши заголовок заметки");
        UserStates[msg->chat->id] = UserState::WAITING_TITLE;
    });

    bot.getEvents().onCommand("list" , [&bot](TgBot::Message::Ptr msg){
        bot.getApi().sendMessage(msg->chat->id , "test txt");
    });

    bot.getEvents().onCommand("delete" , [&bot](TgBot::Message::Ptr msg){
        bot.getApi().sendMessage(msg->chat->id , "Выбери заметку, которую хочешь удалить");
    });

    bot.getEvents().onCommand("clear" , [&bot](TgBot::Message::Ptr msg){
        bot.getApi().sendMessage(msg->chat->id , "test txt");
    });

    bot.getEvents().onCommand("help" , [&bot , &response](TgBot::Message::Ptr msg){
        bot.getApi().sendMessage(msg->chat->id , response.str() );
    });

    bot.getEvents().onAnyMessage([&bot , &commands](TgBot::Message::Ptr msg) {
        for( const auto& cmd : commands)
        {
            if( "/" + cmd->command == msg->text)
            {
                return;
            }
        }
        if( awaitingText[msg->chat->id ] == true && in(UserStates[msg->chat->id] , {UserState::WAITING_TITLE , UserState::WAITING_BODY} ) )
        {
            std::string text;
            if( UserStates[msg->chat->id] == UserState::WAITING_TITLE )
            {
                text = msg->text;
                bot.getApi().sendMessage(msg->chat->id , "Введи текст заметки");
                UserStates[msg->chat->id] = UserState::WAITING_BODY;
            }
            else if( UserStates[msg->chat->id] == UserState::WAITING_BODY)
            {
                text = msg->text;
                bot.getApi().sendMessage(msg->chat->id , "Заметка сохранена");
                UserStates[msg->chat->id] = UserState::NONE;
                awaitingText[msg->chat->id] = false;
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