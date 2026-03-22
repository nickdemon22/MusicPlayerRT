#ifndef PLAYER_LOGIC_H
#define PLAYER_LOGIC_H

#include <SDL.h>
#include <vector>
#include <string>

void PlayTrack(const std::string& filename);
void TogglePause();
std::vector<std::string> GetWavFiles();

// Новые функции для времени
float GetSongProgress(); 
void DrawProgressBar(SDL_Renderer* r, SDL_Rect area, float progress, bool isDark);

#endif