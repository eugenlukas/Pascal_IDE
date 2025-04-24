#pragma once

#include <string>

class TextureLoader {
public:
    static bool LoadTexture(const std::string& texturePath, unsigned int* textureID);
};