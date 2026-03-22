#include "player_logic.h"
#include <SDL2/SDL_mixer.h>

float GetSongProgress() {
    // Проверяем, играет ли музыка вообще
    if (!Mix_PlayingMusic()) return 0.0f;

    // В SDL_mixer нет прямой функции "прогресс в %", 
    // но мы можем получить время в секундах
    double totalTime = Mix_MusicDuration(nullptr); // Длительность трека
    double currentTime = Mix_GetMusicPosition(nullptr); // Текущая позиция

    if (totalTime <= 0) return 0.0f;

    float p = (float)(currentTime / totalTime);
    if (p > 1.0f) p = 1.0f;
    return p;
}

void DrawProgressBar(SDL_Renderer* r, SDL_Rect area, float progress, bool isDark) {
    // Отрисовка остается прежней — она работает отлично!

    // 1. Фон полоски
    SDL_SetRenderDrawColor(r, isDark ? 60 : 200, isDark ? 60 : 200, isDark ? 60 : 200, 255);
    SDL_RenderFillRect(r, &area);

    // 2. Активная часть (синяя)
    int fillWidth = (int)(area.w * progress);
    SDL_Rect fill = { area.x, area.y, fillWidth, area.h };
    SDL_SetRenderDrawColor(r, 0, 120, 215, 255);
    SDL_RenderFillRect(r, &fill);

    // 3. Бегунок
    SDL_Rect thumb = { area.x + fillWidth - 2, area.y - 2, 5, area.h + 4 };
    SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
    SDL_RenderFillRect(r, &thumb);
}