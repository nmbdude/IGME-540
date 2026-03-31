#include <iostream>
#include <stddef.h>
#include "BufferStructs.h"

int main() {
    std::cout << "ambientColor: " << offsetof(PixelShaderData, ambientColor) << "\n";
    std::cout << "padding3: " << offsetof(PixelShaderData, padding3) << "\n";
    std::cout << "directionalLight: " << offsetof(PixelShaderData, directionalLight) << "\n";
    std::cout << "sizeof: " << sizeof(PixelShaderData) << "\n";
    return 0;
}
