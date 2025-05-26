#include <windows.h>
#include <iostream>
#include <string>
#include "employ.h"

using namespace std;



int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Expected pipe number as argument\n";
        return 1;
    }

    string pipeName = "\\\\.\\pipe\\EmployeePipe" + string(argv[1]);

    HANDLE hPipe = CreateFileA(pipeName.c_str(),
        GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

    if (hPipe == INVALID_HANDLE_VALUE) {
        cerr << "Could not connect to pipe.\n";
        return 1;
    }

    DWORD bytesWritten, bytesRead;

    while (true) {
        string command;
        cout << "Enter command (read/write/exit): ";
        cin >> command;

        if (command != "read" && command != "write" && command != "exit") {
            cout << "Invalid command. Please enter 'read', 'write', or 'exit'.\n";
            continue;
        }

        WriteFile(hPipe, command.c_str(), 16, &bytesWritten, NULL);

        if (command == "exit") break;

        int id;
        while (true) {
            cout << "Enter ID: ";
            cin >> id;

            if (cin.fail()) {
                cin.clear(); 
                cin.ignore(100000, '\n'); 
                cout << "Invalid input for ID. Please enter a valid positive number.\n";
            }
            else {
                cin.ignore(100000, '\n'); 
                break; 
            }
        }

        WriteFile(hPipe, &id, sizeof(id), &bytesWritten, NULL);

        if (command == "read") {
            employee e;
            ReadFile(hPipe, &e, sizeof(e), &bytesRead, NULL);
            if (e.num == -1) {
                cout << "Record not found or locked. Please enter a valid ID.\n";
                continue; 
            }
            else {
                cout << "ID: " << e.num << ", Name: " << e.name << ", Hours: " << e.hours << "\n";
            }
            cout << "Press Enter to finish...\n";
            cin.get(); 
        }
        else if (command == "write") {
            employee e;
            ReadFile(hPipe, &e, sizeof(e), &bytesRead, NULL);
            if (e.num == -1) {
                cout << "Record not found or locked. Please enter a valid ID.\n";
                continue;
            }
            cout << "Current Name: " << e.name << ", Hours: " << e.hours << "\n";
            cout << "New Name: "; cin >> e.name;

            while (true) {
                cout << "New Hours: ";
                cin >> e.hours;

                if (cin.fail()) {
                    cin.clear(); 
                    cin.ignore(100000, '\n'); 
                    cout << "Invalid input for hours. Please enter a number.\n";
                }
                else {
                    cin.ignore(100000, '\n'); 
                    break;
                }
            }
            cout << "Press Enter to finish...\n";
            cin.get(); 
            WriteFile(hPipe, &e, sizeof(e), &bytesWritten, NULL);
            cout << "Record updated successfully.\n";
        }
    }

    CloseHandle(hPipe);
    return 0;
}