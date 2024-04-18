#include "RWBufferStream.h"

/*
uint8_t *rBuff;
size_t _rSize;
size_t _rp;
uint8_t *wBuff;
size_t _wSize;
size_t _wp;
*/

////////////////////////////////////////////////////////////////////////////////////////////////////
RWBufferStream::RWBufferStream(size_t rSize, size_t wSize) : Stream() {
    _rSize = rSize;
    rBuff = (uint8_t*) malloc(rSize);
    _wSize = wSize;
    wBuff = (uint8_t*) malloc(wSize);
    reset();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void RWBufferStream::reset() {
    _rp = 0;
    rBuff[_rp] = '\0';
    _wp = 0;
    wBuff[_wp] = '\0';
}

boolean RWBufferStream::setInput(char * data) {
    reset();
    if(strlen(data) >= _rSize) {
        return false;
    }
    strcpy((char*) rBuff, data);
    return true;
}

char* RWBufferStream::getOutput() {
    // null terminate
    wBuff[_wp] = '\0';
    return (char*) wBuff;
}

void RWBufferStream::dumpState() {
    Serial.printf("rBuff: '%s'\r\n", rBuff);
    Serial.printf("_rp: %d\r\n", _rp);
    Serial.print("wBuff: '");
    for(int i=0; i < _wp; i++) {
        Serial.write(wBuff[i]);
    }
    Serial.print("'\r\n");
    Serial.printf("_wp: %d\r\n", _wp);
    Serial.println();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
size_t RWBufferStream::write(uint8_t value) {
	if (_wp == _wSize) {
		return 0;
	}
	wBuff[_wp++] = value;
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
int RWBufferStream::available() {
	return (_rp < _rSize && rBuff[_rp] != '\0');
}

////////////////////////////////////////////////////////////////////////////////////////////////////
int RWBufferStream::read() {
	if (_rp == _rSize) {
		return -1;
	}
	return rBuff[_rp++];
}

////////////////////////////////////////////////////////////////////////////////////////////////////
int RWBufferStream::peek() {
	if (_rp == _rSize) {
		return -1;
	}
	return rBuff[_rp];
}
