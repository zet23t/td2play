return {
	["push-8bit-literal"] = 0x01;
	["push-16bit-literal"] = 0x02;
	["push-32bit-literal"] = 0x03;
	["push-string-literal"] = 0x04;
	["push-8bit-value"] = 0x05;
	["push-16bit-value"] = 0x06;
	["push-stack-value-uint8"] = 0x07;
	["push-stack-value-int8"] = 0x0a;
	["op-*-uint8-uint8-uint16"] = 0x08;
	["op-*-int8-int8-int8"] = 0x0b;
	["return-uint16"] = 0x09;
	["call"] = 0x10;
	["callNative"] = 0x11;
	["return-void"] = 0x12;
  ["move-function-stack-offset"] = 0x13;
  ["cast-uint16-int16"] = 0x14;
  ["cast-uint16-int32"] = 0x15;
  ["cast-int8-int16"] = 0x16;
}