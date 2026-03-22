#include <SDL.h>
#include <SDL_mixer.h>
#include "player_logic.h"
#include "font.h"
#include <vector>
#include <string>
#include <cstdio>
#include <algorithm>

// Константы для кнопок
enum Buttons { BTN_NONE = -1, BTN_PREV, BTN_PLAY, BTN_PAUSE, BTN_NEXT };

// 1. Сначала объявляем DrawText, чтобы её видели все функции ниже
void DrawText(SDL_Renderer* r, const char* text, int x, int y, int scale, bool highlight) {
    if (highlight) SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
    else SDL_SetRenderDrawColor(r, 180, 180, 180, 255);

    for (int i = 0; text[i] != '\0'; i++) {
        char c = text[i]; if (c >= 'a' && c <= 'z') c -= 32;
        if (c < 32 || (c > 90 && c != ':') && c != '/') continue; // Добавили поддержку '/'

        int idx;
        if (c == ':') idx = 26;
        else if (c == '/') idx = 27; // Если в font.h есть символ под номером 27
        else idx = (c - 32);

        for (int row = 0; row < 8; row++) {
            for (int col = 0; col < 8; col++) {
                if ((font8x8[idx][row] >> (7 - col)) & 0x01) {
                    SDL_Rect pixel = { x + (i * 8 * scale) + (col * scale), y + (row * scale), scale, scale };
                    SDL_RenderFillRect(r, &pixel);
                }
            }
        }
    }
}

// 2. Затем DrawSymbol
void DrawSymbol(SDL_Renderer* r, int type, SDL_Rect rect, bool isActive) {
    if (isActive) SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
    else SDL_SetRenderDrawColor(r, 230, 230, 230, 255);

    int x = rect.x + 15, y = rect.y + 15, h = rect.h - 30;
    switch (type) {
    case BTN_PREV:
        for (int i = 0; i < h; i++) {
            int lineW = (i < h / 2) ? i : (h - i);
            SDL_RenderDrawLine(r, x + 15 - lineW, y + i, x + 15, y + i);
            SDL_RenderDrawLine(r, x + 30 - lineW, y + i, x + 30, y + i);
        } break;
    case BTN_PLAY:
        for (int i = 0; i < h; i++) {
            int lineW = (i < h / 2) ? i : (h - i);
            SDL_RenderDrawLine(r, x, y + i, x + lineW * 2, y + i);
        } break;
    case BTN_PAUSE:
    {
        SDL_Rect p1 = { x + 5, y, 8, h }, p2 = { x + 20, y, 8, h };
        SDL_RenderFillRect(r, &p1); SDL_RenderFillRect(r, &p2);
    } break;
    case BTN_NEXT:
        for (int i = 0; i < h; i++) {
            int lineW = (i < h / 2) ? i : (h - i);
            SDL_RenderDrawLine(r, x + lineW, y + i, x, y + i);
            SDL_RenderDrawLine(r, x + 15 + lineW, y + i, x + 15, y + i);
        } break;
    }
}

// Вспомогательная функция перемотки
void SeekToProgress(float progress) {
    if (!Mix_PlayingMusic()) return;
    double duration = Mix_MusicDuration(nullptr);
    if (duration > 0) Mix_SetMusicPosition(duration * progress);
}

// 3. Основная функция меню
void RunMenu() {
    SDL_Window* win = SDL_CreateWindow("MusicPlayer RT", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 400, 500, 0);
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    std::vector<std::string> songs;
    bool loading = true, running = true;
    int selected = -1;
    int scrollOffset = 0;
    const int maxVisible = 8;

    int activeBtn = BTN_NONE;
    Uint32 lastClickTime = 0;
    const Uint32 clickDuration = 100;
    bool isDraggingSeek = false;
    float dragProgress = 0.0f;

    SDL_Rect rPrev = { 20, 20, 60, 60 }, rPlay = { 87, 20, 60, 60 },
        rPause = { 154, 20, 60, 60 }, rNext = { 221, 20, 60, 60 },
        rSeek = { 20, 115, 310, 10 };

    SDL_Event ev;
    while (running) {
        Uint32 currentTime = SDL_GetTicks();
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) running = false;

            if (ev.type == SDL_MOUSEWHEEL) {
                if (ev.wheel.y > 0 && scrollOffset > 0) scrollOffset--;
                if (ev.wheel.y < 0 && scrollOffset < (int)songs.size() - maxVisible) scrollOffset++;
            }

            if (!loading && ev.type == SDL_MOUSEBUTTONDOWN) {
                SDL_Point p = { ev.button.x, ev.button.y };
                if (SDL_PointInRect(&p, &rSeek)) {
                    isDraggingSeek = true;
                    dragProgress = (float)(p.x - rSeek.x) / rSeek.w;
                    SeekToProgress(dragProgress);
                }
                else if (SDL_PointInRect(&p, &rPlay)) { if (selected != -1) PlayTrack(songs[selected]); activeBtn = BTN_PLAY; lastClickTime = currentTime; }
                else if (SDL_PointInRect(&p, &rPause)) { TogglePause(); activeBtn = BTN_PAUSE; lastClickTime = currentTime; }
                else if (SDL_PointInRect(&p, &rNext)) {
                    if (selected < (int)songs.size() - 1) { selected++; PlayTrack(songs[selected]); if (selected >= scrollOffset + maxVisible) scrollOffset++; }
                    activeBtn = BTN_NEXT; lastClickTime = currentTime;
                }
                else if (SDL_PointInRect(&p, &rPrev)) {
                    if (selected > 0) { selected--; PlayTrack(songs[selected]); if (selected < scrollOffset) scrollOffset--; }
                    activeBtn = BTN_PREV; lastClickTime = currentTime;
                }
                else {
                    for (int i = 0; i < maxVisible; i++) {
                        int songIdx = i + scrollOffset;
                        if (songIdx >= (int)songs.size()) break;
                        SDL_Rect item = { 20, 150 + (i * 40), 360, 35 };
                        if (SDL_PointInRect(&p, &item)) { selected = songIdx; if (ev.button.clicks == 2) PlayTrack(songs[selected]); }
                    }
                }
            }
            if (ev.type == SDL_MOUSEBUTTONUP) isDraggingSeek = false;
            if (ev.type == SDL_MOUSEMOTION && isDraggingSeek) {
                int mouseX = std::max(rSeek.x, std::min(ev.motion.x, rSeek.x + rSeek.w));
                dragProgress = (float)(mouseX - rSeek.x) / rSeek.w;
                SeekToProgress(dragProgress);
            }
        }

        if (activeBtn != BTN_NONE && (currentTime - lastClickTime > clickDuration)) activeBtn = BTN_NONE;

        SDL_SetRenderDrawColor(ren, 30, 30, 30, 255);
        SDL_RenderClear(ren);

        if (loading) {
            DrawText(ren, "LOADING...", 140, 230, 1, true);
            SDL_RenderPresent(ren);
            songs = GetWavFiles();
            loading = false;
            continue;
        }

        // Кнопки
        auto drawBtnFunc = [&](SDL_Rect r, int type) {
            bool isPressed = (activeBtn == type);
            SDL_SetRenderDrawColor(ren, isPressed ? 0 : 55, isPressed ? 120 : 55, isPressed ? 215 : 55, 255);
            SDL_RenderFillRect(ren, &r);
            DrawSymbol(ren, type, r, isPressed);
            };
        drawBtnFunc(rPrev, BTN_PREV); drawBtnFunc(rPlay, BTN_PLAY);
        drawBtnFunc(rPause, BTN_PAUSE); drawBtnFunc(rNext, BTN_NEXT);

        DrawText(ren, "PREV", 34, 85, 1, false); DrawText(ren, "PLAY", 101, 85, 1, false);
        DrawText(ren, "PAUSE", 164, 85, 1, false); DrawText(ren, "NEXT", 235, 85, 1, false);

        float currentProg = isDraggingSeek ? dragProgress : GetSongProgress();
        DrawProgressBar(ren, rSeek, currentProg, true);

        double duration = Mix_MusicDuration(nullptr);
        int totalSec = (duration > 0) ? (int)duration : 0;
        int curSec = (int)(currentProg * totalSec);
        char timeStr[32];
        // Оставляем только два числа, разделенных двоеточием
        sprintf_s(timeStr, "%02d:%02d", curSec / 60, curSec % 60);
        DrawText(ren, timeStr, 335, 115, 1, false);

        for (int i = 0; i < maxVisible; i++) {
            int songIdx = i + scrollOffset;
            if (songIdx >= (int)songs.size()) break;
            SDL_Rect item = { 20, 150 + (i * 40), 360, 35 };
            if (songIdx == selected) SDL_SetRenderDrawColor(ren, 0, 120, 215, 255);
            else SDL_SetRenderDrawColor(ren, 45, 45, 45, 255);
            SDL_RenderFillRect(ren, &item);
            std::string name = songs[songIdx];
            if (name.length() > 42) name = name.substr(0, 39) + "...";
            DrawText(ren, name.c_str(), 30, 162 + (i * 40), 1, (songIdx == selected));
        }

        if (songs.size() > maxVisible) {
            SDL_SetRenderDrawColor(ren, 70, 70, 70, 255);
            int trackH = maxVisible * 40 - 5;
            int barH = (int)(trackH * ((float)maxVisible / songs.size()));
            int barY = 150 + (int)(trackH * ((float)scrollOffset / songs.size()));
            SDL_Rect scrollBar = { 385, barY, 5, barH };
            SDL_RenderFillRect(ren, &scrollBar);
        }

        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }
    SDL_DestroyRenderer(ren); SDL_DestroyWindow(win);
}