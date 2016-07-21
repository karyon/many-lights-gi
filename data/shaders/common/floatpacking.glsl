#ifndef FLOAT_PACKING
#define FLOAT_PACKING


// based on http://diaryofagraphicsprogrammer.blogspot.de/2009/10/bitmasks-packing-data-into-fp-render.html
uint Pack4PNToUint(vec4 channel)
{
 uint uValue;

 uValue = (uint(channel.x * 255.0 + 0.5));

 uValue |= (uint(channel.y * 255.0 + 0.5)) << 8;

 uValue |= (uint(channel.z * 255.0 + 0.5)) << 16;

 uValue |= (uint(channel.w * 253.0 + 1.5)) << 24;

 return uValue;
}

float Pack4PNToFP32(vec4 channel)
{
    return uintBitsToFloat(Pack4PNToUint(channel));
}

vec4 Unpack4PNFromUint(uint input)
{
 float a, b, c, d;

 a = float((input) & 0xFFu) / 255.0;

 b = float((input >> 8) & 0xFFu) / 255.0;

 c = float((input >> 16) & 0xFFu) / 255.0;

 d = (float((input >> 24) & 0xFFu) - 1.0) / 253.0;

 return vec4(a, b, c, d);
}

vec4 Unpack4PNFromFP32(float input)
{
    return Unpack4PNFromUint(floatBitsToUint(input));
}

float Pack3PNForFP32(vec3 channel)
{
 uint uValue;

 uValue = (uint(channel.x * 1023.0 + 0.5));

 uValue |= (uint(channel.y * 1023.0 + 0.5)) << 10;

 // to prevent an exponent that is 0 we add 1.0
 uValue |= (uint(channel.z * 1021.0 + 1.5)) << 20;

 return uintBitsToFloat(uValue);
}

vec3 Unpack3PNFromFP32(float fFloatFromFP32)
{
 float a, b, c;
 uint uValue;

 uint uInputFloat = floatBitsToUint(fFloatFromFP32);

 a = float((uInputFloat) & 0x3FFu) / 1023.0;

 b = float((uInputFloat >> 10) & 0x3FFu) / 1023.0;

 c = (float((uInputFloat >> 20) & 0x3FFu) - 1.5) / 1021.0;

 return vec3(a, b, c);
}

// don't use [un]packHalf2x16 as that wastes exponent and sign on normalized values
float Pack2PNForFP32(vec2 channel)
{
 uint uValue;

 uValue = (uint(channel.x * 65535.0 + 0.5));

 // to prevent an exponent that is 0 we add 1.0
 uValue |= (uint(channel.y * 65533.0 + 1.5)) << 16;

 return uintBitsToFloat(uValue);
}

vec2 Unpack2PNFromFP32(float fFloatFromFP32)
{
 float a, b;
 uint uValue;

 uint uInputFloat = floatBitsToUint(fFloatFromFP32);

 a = float((uInputFloat) & 0xFFFFu) / 65535.0;

 b = float((uInputFloat >> 16) & 0xFFFFu) / 65535.0;

 return vec2(a, b);
}

float Pack2ForFP32(vec2 channel)
{
  return uintBitsToFloat(packHalf2x16(channel));
}

vec2 Unpack2FromFP32(float fFloatFromFP32)
{
  return unpackHalf2x16(floatBitsToUint(fFloatFromFP32));
}

#endif
