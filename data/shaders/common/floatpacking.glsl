#ifndef FLOAT_PACKING
#define FLOAT_PACKING


uint pack4UNToUint(vec4 channel)
{
    return packUnorm4x8(channel);
}

float pack4UNToFloat(vec4 channel)
{
    return uintBitsToFloat(pack4UNToUint(channel));
}

vec4 unpack4UNFromUint(uint input)
{
    return unpackUnorm4x8(input);
}

vec4 unpack4UNFromFloat(float input)
{
    return unpack4UNFromUint(floatBitsToUint(input));
}

// based on http://diaryofagraphicsprogrammer.blogspot.de/2009/10/bitmasks-packing-data-into-fp-render.html
float pack3UNToFloat(vec3 channel)
{
    uint uValue;

    uValue = (uint(channel.x * 1023.0 + 0.5));

    uValue |= (uint(channel.y * 1023.0 + 0.5)) << 10;

    // to prevent an exponent that is 0 we add 1.0
    uValue |= (uint(channel.z * 1021.0 + 1.5)) << 20;

    return uintBitsToFloat(uValue);
}

float pack3SNToFloat(vec3 channel)
{
    vec3 channel2 = channel * 0.5 + 0.5;
    return pack3UNToFloat(channel2);
}

vec3 unpack3UNFromFloat(float fFloatFromFP32)
{
    float a, b, c;
    uint uValue;

    uint uInputFloat = floatBitsToUint(fFloatFromFP32);

    a = float((uInputFloat) & 0x3FFu) / 1023.0;

    b = float((uInputFloat >> 10) & 0x3FFu) / 1023.0;

    c = (float((uInputFloat >> 20) & 0x3FFu) - 1.5) / 1021.0;

    return vec3(a, b, c);
}

vec3 unpack3SNFromFloat(float fFloatFromFP32)
{

 return unpack3UNFromFloat(fFloatFromFP32) * 2.0 - 1.0;
}

float pack2FloatsToFloat(vec2 channel)
{
  return uintBitsToFloat(packHalf2x16(channel));
}

vec2 unpack2FloatsFromFloat(float fFloatFromFP32)
{
  return unpackHalf2x16(floatBitsToUint(fFloatFromFP32));
}

float pack2UNToFloat(vec2 channel)
{
  return uintBitsToFloat(packUnorm2x16(channel));
}

vec2 unpack2UNFromFloat(float fFloatFromFP32)
{
  return unpackUnorm2x16(floatBitsToUint(fFloatFromFP32));
}

#endif
