#include "Sprite.inc"

float4 main( PS_Input frag ) : SV_TARGET {
    return colorMap_.Sample( colorSampler_, frag.tex0 ) * frag.color;    
}