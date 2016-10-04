#version 330

in float g_normalRadius;
flat in ivec2 g_centerCoord;

layout(location = 0) out float outNormalRadius;

void main()
{
    outNormalRadius = g_normalRadius;

    // don't bleed over ISM borders
    // ivec2 origPixelCoord = g_centerCoord;
    // ivec2 currentPixelCoord = ivec2(gl_FragCoord.xy);
    // // ISMs are 64px wide, so we ignore 6 bits of texture coordinates
    // if (origPixelCoord >> 6 != currentPixelCoord >> 6) {
    //     discard;
    // }
}
