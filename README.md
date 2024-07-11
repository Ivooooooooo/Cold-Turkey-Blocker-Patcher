
# Cold Turkey Blocker Database Patcher

## Description

This project is a Cold Turkey Blocker database patcher written in C++. It modifies specific settings within the Cold Turkey Blocker database, particularly the `proStatus` setting, changing it to `pro`. This enables premium features within the Cold Turkey Blocker application.

## Features

- Changes the `proStatus` setting to `pro`.
- Handles errors and invalid data gracefully.
- Provides a user-friendly file selection dialog.

## Requirements

- Cold Turkey Blocker installed
- SQLite3 library
- nlohmann/json library

## Installation

1. **Clone the Repository:**
    ```sh
    git clone https://github.com/Ivooooooooo/cold-turkey-patcher.git
    cd cold-turkey-patcher
    ```

2. **Install Dependencies:**
    Make sure you have the SQLite3 and nlohmann/json libraries installed on your system.

## Usage

1. **Run the Executable:**

2. **Select Database:**
    - The program will attempt to open the default database path `C:\ProgramData\Cold Turkey\data-app.db`.
    - If the default database is not found, a file dialog will prompt you to select the database file manually.

3. **Modify Database:**
    - The program will modify the `proStatus` setting to `pro`.
    - Upon successful modification, a success message will be displayed.

## Code Overview

### `Database` Class

- Manages the connection to the SQLite database.
- Provides methods to execute queries and handle errors.

### `Application` Class

- Handles the main application logic.
- Manages COM initialization and file dialog interactions.
- Processes the database to modify the `proStatus` setting.

### `main` Function

- Entry point of the application.
- Instantiates the `Application` class and runs the patcher.

## Error Handling

- The application uses message boxes to display errors and success messages.
- Errors during database operations are logged to the console and shown to the user.

## Contributing

Contributions are welcome! Please fork the repository and submit a pull request with your changes.

---

By using this patcher, you agree to use it responsibly and at your own risk. The developers are not responsible for any potential issues that may arise from modifying the Cold Turkey Blocker database.
