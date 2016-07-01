#ifndef FLOAT_PACKING
#define FLOAT_PACKING

// based on http://diaryofagraphicsprogrammer.blogspot.de/2009/10/bitmasks-packing-data-into-fp-render.html
float Pack3PNForFP32(vec3 channel)
{
 // layout of a 32-bit fp register
 // SEEEEEEEEMMMMMMMMMMMMMMMMMMMMMMM
 // 1 sign bit; 8 bits for the exponent and 23 bits for the mantissa
 uint uValue;

 // pack x
 uValue = (uint(channel.x * 1023.0 + 0.5)); // goes from bit 0 to 15

 // pack y in EMMMMMMM
 uValue |= (uint(channel.y * 1023.0 + 0.5)) << 10;

 // pack z in SEEEEEEE
 // the last E will never be 1b because the upper value is 254
 // max value is 11111110 == 254
 // this prevents the bits of the exponents to become all 1
 // range is 1.. 254
 // to prevent an exponent that is 0 we add 1.0
 uValue |= (uint(channel.z * 1021.0 + 1.5)) << 20;

 return uintBitsToFloat(uValue);
}

// unpack three positive normalized values from a 32-bit float
vec3 Unpack3PNFromFP32(float fFloatFromFP32)
{
 float a, b, c, d;
 uint uValue;

 uint uInputFloat = floatBitsToUint(fFloatFromFP32);

 // unpack a
 // mask out all the stuff above 16-bit with 0xFFFF
 a = float((uInputFloat) & 0x3FFu) / 1023.0;

 b = float((uInputFloat >> 10) & 0x3FFu) / 1023.0;

 // extract the 1..254 value range and subtract 1
 // ending up with 0..253
 c = (float((uInputFloat >> 20) & 0x3FFu) - 1.5) / 1021.0;

 return vec3(a, b, c);
}

#endif
