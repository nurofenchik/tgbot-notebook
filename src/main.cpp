#include <tgbot/tgbot.h>
#include <iostream>
#include <fstream>

int main() {
    std::ifstream tokenfile("../build/token.txt");
    std::string token;
    if (tokenfile.is_open()) {
        std::getline(tokenfile, token);
        tokenfile.close();
    } else {
        std::cerr << "Не удалось открыть файл token.txt" << std::endl;
        return 1;
    }
    TgBot::Bot bot(token);

    bot.getEvents().onCommand("start", [&bot](TgBot::Message::Ptr msg) {
        bot.getApi().sendMessage(msg->chat->id, "Привет! Я твой бот.");
    });

    bot.getEvents().onAnyMessage([&bot](TgBot::Message::Ptr msg) {
        if (!msg->text.empty())
            bot.getApi().sendMessage(msg->chat->id, "Ты написал: " + std::string(msg->text) );
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
