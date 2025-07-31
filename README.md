# MyTelegramBot - Personal Notes Manager

A Telegram bot for managing personal notes with SQLite database storage. Users can create, view, and delete notes through simple Telegram commands.

## Features

- 📝 **Add Notes**: Create notes with custom titles and content
- 📋 **List Notes**: View all your saved notes
- 🗑️ **Delete Notes**: Remove specific notes by title
- 🧹 **Clear All**: Delete all notes at once
- 💾 **Persistent Storage**: Notes are stored in SQLite database
- 🔒 **User Isolation**: Each user has their own private notes

## Commands

- `/start` - Start the bot
- `/add` - Add a new note (interactive process)
- `/list` - Show all your notes
- `/delete` - Delete a specific note
- `/clear` - Delete all your notes
- `/help` - Show help information

## Prerequisites

Before building and running the bot, make sure you have the following installed:

- **CMake** (version 3.10 or higher)
- **C++17** compatible compiler (GCC, Clang, or MSVC)
- **Boost** libraries (system component)
- **OpenSSL** development libraries
- **SQLite3** development libraries
- **pkg-config**

### Ubuntu/Debian Installation:
```bash
sudo apt update
sudo apt install cmake g++ libboost-system-dev libssl-dev libsqlite3-dev pkg-config
```

### CentOS/RHEL/Fedora Installation:
```bash
# For Fedora/newer versions
sudo dnf install cmake gcc-c++ boost-devel openssl-devel sqlite-devel pkgconfig

# For CentOS/RHEL
sudo yum install cmake gcc-c++ boost-devel openssl-devel sqlite-devel pkgconfig
```

## Setup

1. **Clone the repository:**
   ```bash
   git clone <repository-url>
   cd MyTelegramBot
   ```

2. **Create Telegram Bot:**
   - Contact [@BotFather](https://t.me/botfather) on Telegram
   - Create a new bot using `/newbot` command
   - Copy the bot token

3. **Configure the bot:**
   - Open `files/token.txt`
   - Replace the content with your bot token
   ```bash
   echo "YOUR_BOT_TOKEN_HERE" > files/token.txt
   ```

4. **Initialize submodules:**
   ```bash
   git submodule update --init --recursive
   ```

## Building

1. **Create build directory:**
   ```bash
   mkdir build
   cd build
   ```

2. **Configure with CMake:**
   ```bash
   cmake ..
   ```

3. **Build the project:**
   ```bash
   make -j$(nproc)
   ```

## Running

1. **From the build directory:**
   ```bash
   ./bot
   ```

2. **The bot will:**
   - Read the token from `token.txt`
   - Initialize SQLite database in `../db/notes.db`
   - Start listening for Telegram messages

## Project Structure

```
MyTelegramBot/
├── CMakeLists.txt          # Build configuration
├── README.md              # This file
├── include/               # Header files
│   └── database.hpp       # Database interface
├── src/                   # Source files
│   ├── main.cpp          # Main bot logic
│   ├── database.cpp      # Database implementation
│   ├── tgbot-cpp/        # Telegram Bot API library (submodule)
│   └── sqlite_modern_cpp/ # Modern C++ SQLite wrapper (submodule)
├── files/                 # Configuration files
│   ├── token.txt         # Bot token (configure this)
│   └── help_response.txt # Help message content
└── db/                   # Database directory
    └── notes.db          # SQLite database (created automatically)
```

## Dependencies

This project uses the following libraries:

- **[tgbot-cpp](https://github.com/reo7sp/tgbot-cpp)**: C++ library for Telegram Bot API
- **[sqlite_modern_cpp](https://github.com/SqliteModernCpp/sqlite_modern_cpp)**: Modern C++ wrapper for SQLite
- **Boost.System**: For networking support
- **OpenSSL**: For HTTPS connections to Telegram API
- **SQLite3**: Database engine

## Database Schema

The bot automatically creates a `notes` table with the following structure:

```sql
CREATE TABLE IF NOT EXISTS notes (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL,
    title TEXT NOT NULL,
    body TEXT NOT NULL
);
```

## Usage Example

1. Start a conversation with your bot on Telegram
2. Send `/start` to initialize
3. Send `/add` to create a new note:
   - Bot will ask for a title
   - Then ask for the note content
4. Send `/list` to see all your notes
5. Send `/delete` to remove a specific note
6. Send `/clear` to remove all notes

## Security Notes

- Keep your bot token secure and never commit it to version control
- The `files/token.txt` should be added to `.gitignore`
- Each user's notes are isolated by their Telegram user ID
- The database file should have appropriate file permissions

## Troubleshooting

**Build Issues:**
- Ensure all dependencies are installed
- Check that submodules are properly initialized
- Verify CMake version is 3.10 or higher

**Runtime Issues:**
- Verify bot token is correct in `files/token.txt`
- Check internet connectivity for Telegram API access
- Ensure database directory `db/` exists and is writable

**Bot Not Responding:**
- Check if the bot is running without errors
- Verify the bot token is valid
- Make sure the bot is not blocked or restricted

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## License

[Add your license information here]