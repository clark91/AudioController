#include <iostream>
#include <audiopolicy.h>
#include <Audioclient.h>
#include <mmdeviceapi.h>
#include <comdef.h>
#include <tlhelp32.h>
#include <Windows.h>
#include <shellapi.h>
#include <conio.h>

#pragma comment(lib, "ole32.lib")
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")


DWORD GetProcessIdByName(const std::wstring& processName) {
    DWORD processId = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(hSnapshot, &pe32)) {
            do {
                if (processName == pe32.szExeFile) {
                    processId = pe32.th32ProcessID;
                    break;
                }
            } while (Process32Next(hSnapshot, &pe32));
        }
        CloseHandle(hSnapshot);
    }
    return processId;
}

ISimpleAudioVolume* GetAudioVolumeObj() {
    
    // Initialize COM library
    HRESULT hr = CoInitialize(nullptr);
    if (FAILED(hr)) {
        std::cerr << "Failed to initialize COM library. Error: " << std::hex << hr << "\n";
        return NULL;
    }

    // Get the IMMDeviceEnumerator
    IMMDeviceEnumerator* deviceEnumerator = nullptr;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&deviceEnumerator);
    if (FAILED(hr)) {
        std::cerr << "Failed to create device enumerator. Error: " << std::hex << hr << "\n";
        CoUninitialize();
        return NULL;
    }

    // Get the default audio endpoint
    IMMDevice* device = nullptr;
    hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
    deviceEnumerator->Release();
    if (FAILED(hr)) {
        std::cerr << "Failed to get default audio endpoint. Error: " << std::hex << hr << "\n";
        CoUninitialize();
        return NULL;
    }

    // Get the IAudioSessionManager
    IAudioSessionManager2* sessionManager = nullptr;
    hr = device->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, NULL, (void**)&sessionManager);
    device->Release();
    if (FAILED(hr)) {
        std::cerr << "Failed to activate IAudioSessionManager2. Error: " << std::hex << hr << "\n";
        CoUninitialize();
        return NULL;
    }

    // Get the IAudioSessionEnumerator
    IAudioSessionEnumerator* sessionEnumerator = nullptr;
    hr = sessionManager->GetSessionEnumerator(&sessionEnumerator);
    if (FAILED(hr)) {
        std::cerr << "Failed to get session enumerator. Error: " << std::hex << hr << "\n";
        sessionManager->Release();
        CoUninitialize();
        return NULL;
    }

    int sessionCount = 0;
    sessionEnumerator->GetCount(&sessionCount);
    // Finds the Process ID of Spotify.exe
    DWORD targetPID = GetProcessIdByName(L"Spotify.exe");
    if (targetPID == 0) {
        std::cerr << "Failed to find process\n";
        sessionEnumerator->Release();
        sessionManager->Release();
        CoUninitialize();
        return NULL;
    }

    // Gets the ISimpleAudioVolume object
    ISimpleAudioVolume* audioVolume = nullptr;

    for (int i = 0; i < sessionCount; i++) {
        IAudioSessionControl* sessionControl = nullptr;
        hr = sessionEnumerator->GetSession(i, &sessionControl);

        if (SUCCEEDED(hr)) {
            IAudioSessionControl2* sessionControl2 = nullptr;
            hr = sessionControl->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&sessionControl2);
            sessionControl->Release();

            if (SUCCEEDED(hr)) {
                DWORD processId;
                hr = sessionControl2->GetProcessId(&processId);
                if (SUCCEEDED(hr) && processId == targetPID) {
                    
                    hr = sessionControl2->QueryInterface(__uuidof(ISimpleAudioVolume), (void**)&audioVolume);
                    if (SUCCEEDED(hr)) {
                        // Not needed but can do things here
                    }
                    else {
                        std::cerr << "Failed to get ISimpleAudioVolume. Error: " << std::hex << hr << "\n";
                    }
                    sessionControl2->Release();
                    break; // Exit loop after finding the correct session
                }
                sessionControl2->Release();
            }
        }
    }

    sessionEnumerator->Release();
    sessionManager->Release();
    return (audioVolume);
}

int main() {

    

    ISimpleAudioVolume* appVol = nullptr;
    BOOL running = TRUE;
    float changeAmount = 0.05f;

    int count = 0;

    while (running) {
        if (appVol == nullptr) {
            appVol = GetAudioVolumeObj(); 
            if (appVol == nullptr) {
                Sleep(5000);
                std::cout << "Failed to GetAudioVolumeObj\n";
            }
        }
        else {
            float currentVol;
            appVol->GetMasterVolume(&currentVol);

            // Volume Down Button
            if (GetAsyncKeyState(VK_F22) != 0) {
                if (currentVol <= 0.05f) {
                    appVol->SetMute(TRUE, NULL);
                }
                appVol->SetMasterVolume(currentVol - changeAmount, NULL);
                //std::cout << "Vol Dn: " << currentVol << std::endl;
            }

            // Volume Up Button
            if (GetAsyncKeyState(VK_F24) != 0) {
                appVol->SetMute(FALSE, NULL);
                appVol->SetMasterVolume(currentVol + changeAmount, NULL);
                //std::cout << "Vol Up: " << currentVol << std::endl;
            }

            // Volume Mute Button
            if (GetAsyncKeyState(VK_F23) != 0) {
                BOOL muted;
                appVol->GetMute(&muted);
                appVol->SetMute(!muted, NULL);
                //std::cout << "Muted: " << currentVol << std::endl;
            }
            Sleep(1000 / 7);
        }
        

        // Resets the appVol variable every 10 seconds to check if the audio stream is no longer valid.
        count++;
        if (count == 70) {
            appVol = nullptr;
            count = 0;
        }
    }

    CoUninitialize();
    return 0;
}
