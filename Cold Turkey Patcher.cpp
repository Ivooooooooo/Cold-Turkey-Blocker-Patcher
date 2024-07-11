#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include <shobjidl_core.h>
#include "sqlite3.h"
#include "json.hpp"

using json = nlohmann::json;

class Database {
public:
    Database(const std::string& databaseFile) : db(nullptr) {
        int result = sqlite3_open(databaseFile.c_str(), &db);
        if (result != SQLITE_OK) {
            showError("Error opening database: " + std::string(sqlite3_errmsg(db)));
            sqlite3_close(db);
            db = nullptr;
        }
    }

    ~Database() {
        if (db) {
            sqlite3_close(db);
            db = nullptr;
        }
    }

    bool isOpen() const {
        return db != nullptr;
    }

    bool executeQuery(const std::string& query) const {
        char* errMsg = nullptr;
        int result = sqlite3_exec(db, query.c_str(), nullptr, nullptr, &errMsg);
        if (result != SQLITE_OK) {
            showError("Error executing query: " + std::string(errMsg));
            sqlite3_free(errMsg);
            return false;
        }
        return true;
    }

    bool executeSelectQuery(const std::string& query, std::vector<std::vector<std::string>>& results) const {
        char* errMsg = nullptr;
        results.clear();
        int result = sqlite3_exec(db, query.c_str(), selectCallback, &results, &errMsg);
        if (result != SQLITE_OK) {
            showError("Error executing select query: " + std::string(errMsg));
            sqlite3_free(errMsg);
            return false;
        }
        return true;
    }

private:
    sqlite3* db;

    static int selectCallback(void* data, int argc, char** argv, char** azColName) {
        auto results = static_cast<std::vector<std::vector<std::string>>*>(data);
        std::vector<std::string> row;
        for (int i = 0; i < argc; i++) {
            row.push_back(argv[i] ? argv[i] : "NULL");
        }
        results->push_back(row);
        return 0;
    }

    static void showError(const std::string& errorMessage) {
        std::cerr << errorMessage << std::endl;
        MessageBoxA(nullptr, errorMessage.c_str(), "Error", MB_OK | MB_ICONERROR);
    }
};

class Application {
public:
    void run() {
        std::string databasePath = "C:\\ProgramData\\Cold Turkey\\data-app.db";
        HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
        if (FAILED(hr)) {
            showError("Error initializing COM library: " + std::to_string(hr));
            return;
        }

        if (fileExists(databasePath)) {
            processDatabase(databasePath);
        }
        else {
            std::string selectedFile = openFileDialog();
            if (selectedFile.empty()) {
                showError("No database file selected.");
                return;
            }

            processDatabase(selectedFile);
        }

        CoUninitialize();
    }

private:
    bool fileExists(const std::string& filePath) {
        std::wstring wFilePath(filePath.begin(), filePath.end());
        DWORD fileAttributes = GetFileAttributesW(wFilePath.c_str());
        return (fileAttributes != INVALID_FILE_ATTRIBUTES && !(fileAttributes & FILE_ATTRIBUTE_DIRECTORY));
    }

    void processDatabase(const std::string& databaseFile) {
        Database database(databaseFile);
        if (!database.isOpen()) {
            showError("Failed to open the database.");
            return;
        }

        if (modifyDatabase(database)) {
            showSuccess("Database modification successful.");
        }
        else {
            showError("Failed to modify the database.");
        }
    }

    bool modifyDatabase(Database& database) {
        std::string selectQuery = "SELECT value FROM settings WHERE key = 'settings';";
        std::vector<std::vector<std::string>> results;
        if (database.executeSelectQuery(selectQuery, results)) {
            if (results.size() == 1 && results[0].size() == 1) {
                std::string jsonValue = results[0][0];

                try {
                    json jsonData = json::parse(jsonValue);
                    if (jsonData.contains("additional") && jsonData["additional"].contains("proStatus")) {
                        std::string currentProStatus = jsonData["additional"]["proStatus"];

                        if (isValidProStatus(currentProStatus)) {
                            std::string newProStatus = "pro";
                            jsonData["additional"]["proStatus"] = newProStatus;

                            std::string updatedJson = jsonData.dump();

                            std::string updateQuery = "UPDATE settings SET value = '" + updatedJson + "' WHERE key = 'settings';";
                            if (!database.executeQuery(updateQuery)) {
                                showError("Failed to execute the update query for 'settings' table.");
                                return false;
                            }
                        }
                        else {
                            showError("Invalid current 'proStatus' value: " + currentProStatus);
                            return false;
                        }
                    }
                    else {
                        showError("Invalid data structure in the 'settings' table.");
                        return false;
                    }
                }
                catch (const std::exception& e) {
                    showError("Error parsing JSON data: " + std::string(e.what()));
                    return false;
                }
            }
            else {
                showError("Invalid data retrieved from the settings table. Try starting CTB at least one time.");
                return false;
            }
        }
        else {
            showError("Failed to execute the select query for 'settings' table.");
            return false;
        }

        return true;
    }

    bool isValidProStatus(const std::string& proStatus) {
        return (proStatus == "pro" || proStatus == "free" || proStatus == "test");
    }

    std::string openFileDialog() {
        std::string filePath;

        IFileOpenDialog* pFileOpen;
        if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL, IID_PPV_ARGS(&pFileOpen)))) {
            DWORD dwOptions;
            if (SUCCEEDED(pFileOpen->GetOptions(&dwOptions))) {
                pFileOpen->SetOptions(dwOptions | FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST);

                pFileOpen->SetTitle(L"Select Database File");

                if (SUCCEEDED(pFileOpen->Show(nullptr))) {
                    IShellItem* pItem;
                    if (SUCCEEDED(pFileOpen->GetResult(&pItem))) {
                        PWSTR pszFilePath;
                        if (SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath))) {
                            int bufferSize = WideCharToMultiByte(CP_UTF8, 0, pszFilePath, -1, nullptr, 0, nullptr, nullptr);
                            if (bufferSize > 0) {
                                std::vector<char> buffer(bufferSize);
                                WideCharToMultiByte(CP_UTF8, 0, pszFilePath, -1, buffer.data(), bufferSize, nullptr, nullptr);
                                filePath = buffer.data();
                            }
                            CoTaskMemFree(pszFilePath);
                        }
                        pItem->Release();
                    }
                }
            }
            pFileOpen->Release();
        }

        return filePath;
    }

    static void showError(const std::string& errorMessage) {
        std::cerr << errorMessage << std::endl;
        MessageBoxA(nullptr, errorMessage.c_str(), "Error", MB_OK | MB_ICONERROR);
    }

    void showSuccess(const std::string& successMessage) const {
        MessageBoxA(nullptr, successMessage.c_str(), "Success", MB_OK | MB_ICONINFORMATION);
        Sleep(2000);
        exit(0);
    }
};

int main() {
    Application app;
    app.run();

    return 0;
}
