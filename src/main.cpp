#include <tgbot/tgbot.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
enum class UserState {
    None,
    WAITING_TITLE,
    WAITING_BODY
};


std::unordered_map<int64_t , bool> awaitingNote;
std::unordered_map<int64_t , UserState> UserStates;
int main() {

    std::ostringstream response;
    std::ifstream helpfile;
    try
    {
        helpfile.open( "../src/help_response.txt");
        response << helpfile.rdbuf();
        helpfile.close();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }
    
    std::string token;
    std::ifstream tokenfile;
    try
    {
        tokenfile.open("../build/token.txt");
        tokenfile >> token;
        tokenfile.close();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }
    

    std::vector<TgBot::BotCommand::Ptr> commands;

    auto cmd1 = std::make_shared<TgBot::BotCommand>();
    cmd1->command = "start";
    cmd1->description = "Запустить бота";
    commands.push_back(cmd1);

    auto cmd2 = std::make_shared<TgBot::BotCommand>();
    cmd2->command = "add";
    cmd2->description = "Добавить заметку";
    commands.push_back(cmd2);

    // ... добавь остальные команды

    TgBot::Bot bot(token);
    
    bot.getApi().setMyCommands(commands);
    bot.getEvents().onCommand("start", [&bot , &response](TgBot::Message::Ptr msg) {
        bot.getApi().sendMessage( msg->chat->id, "Привет,  "+ msg->chat->firstName +"!\nЯ бот-хранитель твоих заметок!");
        awaitingNote.erase(msg->chat->id);
        UserStates.erase(msg->chat->id);
        bot.getApi().sendMessage(msg->chat->id , response.str() );
    });

    bot.getEvents().onCommand("add" , [&bot](TgBot::Message::Ptr msg)
    {
        awaitingNote[msg->chat->id] = true;
        bot.getApi().sendMessage(msg->chat->id, "Напиши заголовок заметки");
        UserStates[msg->chat->id] = UserState::WAITING_TITLE;
    });

    bot.getEvents().onAnyMessage([&bot , &commands](TgBot::Message::Ptr msg) {
        if( awaitingNote[msg->chat->id ] )
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
                UserStates.erase(msg->chat->id);
                awaitingNote.erase(msg->chat->id);
            }
        }
        else
        {
            for( const auto& cmd : commands)
            {
            if( "/" + cmd->command == msg->text)
            {
                return;
            }
            }
            bot.getApi().sendMessage( msg->chat->id , "Invalid command , use /help ");
        }
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
