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
    void getNotes(int64_t userId , std::vector<Note>& notes);
    void clearNotes(int64_t userId);
    void clearNoteByTitle(int64_t userId , const std::string& title);

private:
    std::string dbPath;
};
#endif