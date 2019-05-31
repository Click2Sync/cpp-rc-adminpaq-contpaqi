# cpp-rc-adminpaq-contpaqi
C++ implementation of the reverse connector API of Click2Sync for AdminPAQ 11 CONTPAQi

## Installing
1. Load in Dev-C++ as C++ project
2. Use MinGW with C++ 11 with 32 bit or 64 bit depending on the DLL of your AdminPAQ installation
3. Missing libraries (check include files to understand which libraries to include)
4. Missing MGW_SDK.dll files from AdminPAQ installation
5. Create folder C:\c2srcsrvc
6. Build project and copy ReverseConnector.exe in c2srcsrvc folder
7. Copy "config.json" to "C:\c2srcsrvc"

## Running as a service
1. Register as a service running "installservice.bat"
2. Change the paremeter "executionmode" to "service" located in "C:\c2srcsrvc\config.json"
3. Start the windows service named "Click2Sync"

## Running as debug
1. Change the paremeter "executionmode" to "debug" located in "C:\c2srcsrvc\config.json"
2. Run C:\c2srcsrvc\ReverseConnector.exe
