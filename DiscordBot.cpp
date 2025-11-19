#include <iostream>
#include <string>
#include <windows.h>
#include <wininet.h>
#include <string.h>

#pragma comment(lib, "wininet.lib")

class DiscordBot {
private:
    std::string token;

    std::string HttpRequest(const std::string& method, const std::string& url, const std::string& data = "") {
        HINTERNET hInternet = NULL, hConnect = NULL, hRequest = NULL;
        std::string response;

        std::cout << "[DEBUG] Starting HTTP request..." << std::endl;
        std::cout << "[DEBUG] Method: " << method << std::endl;
        std::cout << "[DEBUG] URL: " << url << std::endl;
        std::cout << "[DEBUG] Data: " << data << std::endl;

        try {
            hInternet = InternetOpenA("DiscordBot/1.0", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
            if (!hInternet) {
                DWORD error = GetLastError();
                std::cout << "[DEBUG] InternetOpen failed! Error: " << error << std::endl;
                throw "InternetOpen failed";
            }
            std::cout << "[DEBUG] InternetOpen successful" << std::endl;

            URL_COMPONENTSA urlComp = {0};
            urlComp.dwStructSize = sizeof(urlComp);
            urlComp.dwSchemeLength = -1;
            urlComp.dwHostNameLength = -1;
            urlComp.dwUrlPathLength = -1;
            urlComp.dwExtraInfoLength = -1;

            char hostName[256] = {0};
            char urlPath[1024] = {0};
            urlComp.lpszHostName = hostName;
            urlComp.dwHostNameLength = sizeof(hostName);
            urlComp.lpszUrlPath = urlPath;
            urlComp.dwUrlPathLength = sizeof(urlPath);

            if (!InternetCrackUrlA(url.c_str(), url.length(), 0, &urlComp)) {
                DWORD error = GetLastError();
                std::cout << "[DEBUG] InternetCrackUrl failed! Error: " << error << std::endl;
                throw "InternetCrackUrl failed";
            }
            
            std::cout << "[DEBUG] URL parsed - Host: " << hostName << ", Path: " << urlPath << ", Port: " << urlComp.nPort << std::endl;

            // Connect
            hConnect = InternetConnectA(hInternet, hostName, urlComp.nPort, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
            if (!hConnect) {
                DWORD error = GetLastError();
                std::cout << "[DEBUG] InternetConnect failed! Error: " << error << std::endl;
                throw "InternetConnect failed";
            }
            std::cout << "[DEBUG] InternetConnect successful" << std::endl;

            // Request
            DWORD flags = INTERNET_FLAG_SECURE | INTERNET_FLAG_RELOAD;
            hRequest = HttpOpenRequestA(hConnect, 
                                      method.c_str(), 
                                      urlPath,
                                      NULL, NULL, NULL, flags, 0);
            if (!hRequest) {
                DWORD error = GetLastError();
                std::cout << "[DEBUG] HttpOpenRequest failed! Error: " << error << std::endl;
                throw "HttpOpenRequest failed";
            }
            std::cout << "[DEBUG] HttpOpenRequest successful" << std::endl;

            
            std::string headers = "Content-Type: application/json\r\nAuthorization: Bot " + token;
            std::cout << "[DEBUG] Headers: " << headers << std::endl;
            
            // Send request
            if (!HttpSendRequestA(hRequest, headers.c_str(), headers.length(), 
                                (LPVOID)data.c_str(), data.length())) {
                DWORD error = GetLastError();
                std::cout << "[DEBUG] HttpSendRequest failed! Error: " << error << std::endl;
                throw "HttpSendRequest failed";
            }
            std::cout << "[DEBUG] HttpSendRequest successful" << std::endl;

            DWORD statusCode = 0;
            DWORD statusSize = sizeof(statusCode);
            if (HttpQueryInfoA(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, 
                             &statusCode, &statusSize, NULL)) {
                std::cout << "[DEBUG] HTTP Status Code: " << statusCode << std::endl;
            } else {
                std::cout << "[DEBUG] Could not get HTTP status code" << std::endl;
            }

            // Read response
            char buffer[4096];
            DWORD bytesRead;
            int chunkCount = 0;
            
            while (InternetReadFile(hRequest, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead > 0) {
                buffer[bytesRead] = '\0';
                response += buffer;
                chunkCount++;
                std::cout << "[DEBUG] Read chunk " << chunkCount << ", size: " << bytesRead << " bytes" << std::endl;
                
              
                if (chunkCount == 1) {
                    std::string preview = buffer;
                    if (preview.length() > 200) {
                        preview = preview.substr(0, 200) + "...";
                    }
                    std::cout << "[DEBUG] First chunk preview: " << preview << std::endl;
                }
            }
            
            std::cout << "[DEBUG] Total response size: " << response.length() << " bytes" << std::endl;
            std::cout << "[DEBUG] Total chunks: " << chunkCount << std::endl;

        }
        catch (const char* error) {
            std::cerr << "[ERROR] " << error << " (System Error: " << GetLastError() << ")" << std::endl;
        }

        if (hRequest) {
            InternetCloseHandle(hRequest);
            std::cout << "[DEBUG] hRequest closed" << std::endl;
        }
        if (hConnect) {
            InternetCloseHandle(hConnect);
            std::cout << "[DEBUG] hConnect closed" << std::endl;
        }
        if (hInternet) {
            InternetCloseHandle(hInternet);
            std::cout << "[DEBUG] hInternet closed" << std::endl;
        }

        return response;
    }

public:
    DiscordBot(const std::string& botToken) : token(botToken) {
        std::cout << "[DEBUG] DiscordBot created with token: " << token.substr(0, 10) << "..." << std::endl;
    }

    void testConnection() {
        std::cout << "\n=== Testing Discord API Connection ===" << std::endl;
        std::string response = HttpRequest("GET", "https://discord.com/api/v10/users/@me");
        
        if (!response.empty()) {
            std::cout << "\n Connection successful!" << std::endl;
            std::cout << "Full Response: " << response << std::endl;
        } else {
            std::cout << "\n Connection failed!" << std::endl;
        }
    }

    void sendMessage(const std::string& channelId, const std::string& message) {
        std::cout << "\n=== Sending Message ===" << std::endl;
        std::string url = "https://discord.com/api/v10/channels/" + channelId + "/messages";
        std::string jsonData = "{\"content\":\"" + message + "\"}";
        
        std::cout << "[DEBUG] Channel ID: " << channelId << std::endl;
        std::cout << "[DEBUG] Message: " << message << std::endl;
        std::cout << "[DEBUG] JSON Data: " << jsonData << std::endl;
        
        std::string response = HttpRequest("POST", url, jsonData);
        if (!response.empty()) {
            std::cout << "\nMessage sent successfully!" << std::endl;
            std::cout << " Response: " << response << std::endl;
        } else {
            std::cout << "\n Failed to send message" << std::endl;
        }
    }
};



int main() {
    std::cout << " === C++ Discord Bot (WinINet Debug) ===" << std::endl;

    // ENTER YOUR TOKEN
    std::string token = "token here";



    //Don't change it.
    if (token == "YOUR_BOT_TOKEN_HERE") {
        std::cout << " ERROR: Please enter your bot token" << std::endl;
        std::cout << "Press any key to exit..." << std::endl;
        system("pause");
        return 1;
    }

    std::cout << "[DEBUG] Token length: " << token.length() << " characters" << std::endl;
    std::cout << "[DEBUG] Token preview: " << token.substr(0, 20) << "..." << std::endl;

    DiscordBot bot(token);
    std::string channelId = "channel id here ";
    // Test connection
    bot.testConnection();
    std::string str = "";
    std::cout << "Enter a message: ";
    std::cin.ignore(); 
    std::getline(std::cin, str); 

bot.sendMessage(channelId, str);
    
    bot.sendMessage(channelId, "Hello from C++ WinINet Bot with Debug!");
    
    std::cout << "\n Debug completed. Press any key to exit..." << std::endl;
    system("pause");
    return 0;
}