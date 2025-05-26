#include <windows.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <mutex>
#include <map>
#include <string>
#include <sstream>
#include <set>
#include "employ.h"
using namespace std;

const string PIPE_BASE_NAME = "\\\\.\\pipe\\EmployeePipe";


string fileName;
mutex lockMutex;
map<int, int> recordLocks; 

void printFile() {
    ifstream file(fileName, ios::binary);
    employee e;
    cout << "\n--- FILE CONTENT ---\n";
    while (file.read((char*)&e, sizeof(e))) {
        cout << "ID: " << e.num << ", Name: " << e.name << ", Hours: " << e.hours << "\n";
    }
    cout << "--------------------\n";
    file.close();
}

bool lockRecord(int id, int mode) {
    lock_guard<mutex> lock(lockMutex);
    if (recordLocks[id] == 2 || (recordLocks[id] == 1 && mode == 2)) return false;
    recordLocks[id] = mode;
    return true;
}

void unlockRecord(int id) {
    lock_guard<mutex> lock(lockMutex);
    recordLocks[id] = 0;
}

employee readRecord(int id) {
    ifstream file(fileName, ios::binary);
    employee e;
    while (file.read((char*)&e, sizeof(e))) {
        if (e.num == id) {
            return e; 
        }
    }
    return employee{ -1 }; 
}

void writeRecord(employee updated) {
    fstream file(fileName, ios::in | ios::out | ios::binary);
    employee e;
    while (file.read((char*)&e, sizeof(e))) {
        if (e.num == updated.num) {
            file.seekp((int)file.tellg() - sizeof(employee));
            file.write((char*)&updated, sizeof(employee));
            break;
        }
    }
    file.close();
}

void clientHandler(HANDLE hPipe) {
    DWORD bytesRead, bytesWritten;
    char command[16];
    while (true) {
        if (!ReadFile(hPipe, command, sizeof(command), &bytesRead, NULL)) break;
        if (strncmp(command, "exit", 4) == 0) break;

        int id;
        ReadFile(hPipe, &id, sizeof(int), &bytesRead, NULL);

        if (strncmp(command, "read", 4) == 0) {
            if (!lockRecord(id, 1)) {
                int fail = -1;
                WriteFile(hPipe, &fail, sizeof(int), &bytesWritten, NULL);
                continue;
            }
            employee e = readRecord(id);
            WriteFile(hPipe, &e, sizeof(e), &bytesWritten, NULL);
            unlockRecord(id);
        }
        else if (strncmp(command, "write", 5) == 0) {
            if (!lockRecord(id, 2)) {
                int fail = -1;
                WriteFile(hPipe, &fail, sizeof(int), &bytesWritten, NULL);
                continue;
            }
            employee e = readRecord(id);
            if (e.num == -1) {
                int fail = -1;
                WriteFile(hPipe, &fail, sizeof(int), &bytesWritten, NULL);
                unlockRecord(id);
                continue; 
            }
            WriteFile(hPipe, &e, sizeof(e), &bytesWritten, NULL);
            ReadFile(hPipe, &e, sizeof(e), &bytesRead, NULL);
            writeRecord(e);
            unlockRecord(id);
        }
    }
    DisconnectNamedPipe(hPipe);
    CloseHandle(hPipe);
}



int main() {
    cout << "Enter filename: ";
    cin >> fileName;

    int count;
    cout << "Number of employees: ";

    while (true) {
        cin >> count;
        if (cin.fail()) {
            cin.clear(); 
            cin.ignore(10000, '\n');
            cout << "Invalid input. Please enter a number.\n";
            continue;
        }
        break;
    }

    ofstream file(fileName, ios::binary);
    set<int> existingIDs; 

    for (int i = 0; i < count; ++i) {
        employee e;
        while (true) {
            cout << "ID: ";
            cin >> e.num;

            if (cin.fail() || !isUniqueID(existingIDs, e.num)) {
                cin.clear();
                cin.ignore(10000, '\n');
                cout << "Invalid ID or ID already exists. Try again.\n";
                continue;
            }
            existingIDs.insert(e.num); 
            break;
        }

        cout << "Name: ";
        cin >> e.name;

        while (true) {
            cout << "Hours: ";
            cin >> e.hours;

            if (cin.fail()) {
                cin.clear(); 
                cin.ignore(10000, '\n'); 
                cout << "Invalid input for hours. Please enter a number.\n";
                continue;
            }
            break;
        }

        file.write((char*)&e, sizeof(employee));
    }
    file.close();

    printFile();

    int clientCount;
    cout << "Number of clients: ";

    while (true) {
        cin >> clientCount;
        if (cin.fail()) {
            cin.clear(); 
            cin.ignore(10000, '\n'); 
            cout << "Invalid input. Please enter a number.\n";
            continue;
        }
        break;
    }


    vector<thread> clientThreads;

    for (int i = 0; i < clientCount; ++i) {
        stringstream pipeName;
        pipeName << PIPE_BASE_NAME << i;

        HANDLE hPipe = CreateNamedPipeA(
            pipeName.str().c_str(),
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
            1,
            0, 0,
            0, NULL
        );

        if (hPipe == INVALID_HANDLE_VALUE) {
            cerr << "Failed to create pipe\n";
            return 1;
        }

        STARTUPINFOA si = { sizeof(si) };
        PROCESS_INFORMATION pi;
        string cmd = "client.exe " + to_string(i);
        if (!CreateProcessA(NULL, &cmd[0], NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
            cerr << "CreateProcess failed\n";
            return 1;
        }

        cout << "Waiting for client " << i << "...\n";
        ConnectNamedPipe(hPipe, NULL);
        clientThreads.emplace_back(clientHandler, hPipe);
    }

    for (auto& t : clientThreads) t.join();

    printFile();

    cout << "Press ENTER to exit...\n";
    cin.ignore(); cin.get();
    return 0;
}