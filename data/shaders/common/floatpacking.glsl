#ifndef FLOAT_PACKING
#define FLOAT_PACKING

// based on http://diaryofagraphicsprogrammer.blogspot.de/2009/10/bitmasks-packing-data-into-fp-render.html
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
