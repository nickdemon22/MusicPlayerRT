#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>
#include <windows.h>
#include <vector>
#include <string>
#include "player_logic.h"

// Автоматическое подключение библиотек для Visual Studio
#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "SDL2_mixer.lib")

// Глобальная переменная для музыки
Mix_Music* currentMusic = nullptr;

// 1. Реализация воспроизведения
void PlayTrack(const std::string& filename) {
    Mix_HaltMusic(); // Останавливаем текущий трек
    if (currentMusic) {
        Mix_FreeMusic(currentMusic);
        currentMusic = nullptr;
    }

    currentMusic = Mix_LoadMUS(filename.c_str());
    if (currentMusic) {
        // Играем 1 раз (важно для автопереключения в menu.cpp)
        Mix_PlayMusic(currentMusic, 1);
    }
    else {
        OutputDebugStringA(("Ошибка загрузки: " + std::string(Mix_GetError()) + "\n").c_str());
    }
}

// 2. Управление паузой
void TogglePause() {
    if (Mix_PausedMusic()) {
        Mix_ResumeMusic();
    }
    else {
        Mix_PauseMusic();
    }
}

// 3. Поиск файлов
std::vector<std::string> GetWavFiles() {
    std::vector<std::string> files;
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA("./*", &findData);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            std::string name = findData.cFileName;
            if (name.length() > 4) {
                std::string ext = name.substr(name.find_last_of(".") + 1);
                if (ext == "wav" || ext == "mp3" || ext == "ogg" || ext == "flac") {
                    files.push_back(name);
                }
            }
        } while (FindNextFileA(hFind, &findData));
        FindClose(hFind);
    }
    return files;
}

// Указываем компилятору, что RunMenu реализована в menu.cpp
extern void RunMenu();

// Главная точка входа Windows
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nShow) {
    SDL_SetMainReady();

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) return 1;

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        SDL_Quit();
        return 1;
    }

    Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG);

    // ЗАПУСК ИНТЕРФЕЙСА (из menu.cpp)
    RunMenu();

    // Очистка при выходе
    if (currentMusic) Mix_FreeMusic(currentMusic);
    Mix_CloseAudio();
    Mix_Quit();
    SDL_Quit();

    return 0;
}