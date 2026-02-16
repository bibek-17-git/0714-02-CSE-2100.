💾 Smart Backup Utility
A modular C-based file backup system built for Advanced Programming Laboratory.
Designed using clean project structure, header abstraction, and Makefile automation.
________________________________________
📌 Overview
Smart Backup Utility is a command-line based backup management system written in C.
It allows users to configure and perform file/directory backups in a structured and maintainable way.
This project demonstrates:
•	Modular Programming in C
•	Multi-file project organization
•	Header file abstraction
•	Makefile automation
•	Clean coding style for GitHub projects
________________________________________
📂 Project Structure
smart-backup-utility/
│
├── src/
│   ├── main.c
│   ├── ui.c
│   ├── backup.c
│
├── include/
│   ├── ui.h
│   ├── backup.h
│   └── config.h
│
├── Makefile
├── README.md
├── .gitignore
└── backup_config.txt
________________________________________
🚀 Features
✔ Modular multi-file C project
✔ Separate UI and backup logic
✔ Configurable backup system
✔ Clean folder structure (src/ and include/)
✔ Makefile build automation
✔ Easy to extend
________________________________________
🛠 Technologies Used
•	C Programming Language
•	GCC Compiler
•	GNU Make
•	Linux / WSL / MinGW environment
________________________________________
⚙️ Build Instructions
Make sure GCC and Make are installed.
🔹 Compile the project
From project root directory:
make
This will generate the executable:
smart-backup-utility
________________________________________
▶️ Run the Program
./smart-backup-utility
________________________________________
🧹 Clean Build Files
To remove object files and executable:
make clean
________________________________________
📄 Configuration
The file backup_config.txt stores backup settings such as:
•	Source directory
•	Destination directory
•	Backup preferences
Edit this file before running the program if needed.
________________________________________
🧠 Module Description
🔹 main.c
•	Entry point of the program
•	Initializes the system
•	Connects UI and backup modules
🔹 ui.c / ui.h
•	Handles user interaction
•	Displays menus
•	Takes user input
🔹 backup.c / backup.h
•	Core backup logic
•	File copy operations
•	Directory processing
🔹 config.h
•	Macro definitions
•	Global constants
•	Configuration declarations
________________________________________
🎯 Learning Outcomes
This project demonstrates:
•	Multi-file C programming
•	Header file usage and abstraction
•	Proper GitHub project organization
•	Use of Makefile in real projects
•	Clean coding practices
________________________________________
🔮 Future Improvements
•	Add timestamp-based automatic backup
•	Add restore functionality
•	Add log file system
•	Add compression (ZIP support)
•	Add GTK GUI version
________________________________________
📸 Example (Optional)
You may add screenshots here later if you create a GUI version.
________________________________________
👨‍💻 Author
Course: Advanced Programming Laboratory
Project: Smart Backup Utility
________________________________________
📜 License
This project is developed for academic purposes.

