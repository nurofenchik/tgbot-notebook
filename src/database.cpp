#include "database.hpp"
#include <sqlite_modern_cpp.h>
#include <iostream>

Database::Database(const std::string& path) : dbPath(path) {
    try {
        sqlite::database db(dbPath);
        db <<
            "CREATE TABLE IF NOT EXISTS notes ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "user_id INTEGER,"
            "title TEXT,"
            "body TEXT);";
    } catch (const std::exception& e) {
        std::cerr << "DB Init Error: " << e.what() << std::endl;
    }
}

void Database::addNote(int64_t userId, const std::string& title, const std::string& body) {
    sqlite::database db(dbPath);
    db << "INSERT INTO notes (user_id, title, body) VALUES (?, ?, ?);"
       << userId << title << body;
}

void Database::getNotes(int64_t userId , std::vector<Note>& notes) {
    sqlite::database db(dbPath);

    db << "SELECT id, title, body FROM notes WHERE user_id = ?;"
       << userId
       >> [&](int id, std::string title, std::string body) {
            notes.push_back({id, std::move(title), std::move(body)});
       };
}

void Database::clearNoteByTitle(int64_t userId , const std::string& title) {
    sqlite::database db(dbPath);
    db << "DELETE FROM notes WHERE user_id = ? AND title = ?;" << userId << title;
}


void Database::clearNotes(int64_t userId) {
    sqlite::database db(dbPath);
    db << "DELETE FROM notes WHERE user_id = ?;" << userId;
}
