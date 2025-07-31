#ifndef DATABASE_HPP
#define DATABASE_HPP
#include <string>
#include <vector>

struct Note {
    int id;
    std::string title;
    std::string body;
};

class Database {
public:
    explicit Database(const std::string& path);

    void addNote(int64_t userId, const std::string& title, const std::string& body);
    std::vector<Note> getNotes(int64_t userId);
    void clearNotes(int64_t userId);

private:
    std::string dbPath;
};
#endif