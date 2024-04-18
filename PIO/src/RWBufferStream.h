#ifndef __RWBufferStream_H__
#define __RWBufferStream_H__

#include <Arduino.h>

class RWBufferStream : public Stream {
	public:
		explicit RWBufferStream(size_t rSize, size_t wSize);

	public:
		size_t write(uint8_t value);
		int available();
		int read();
		int peek();
        virtual void flush() {}

        boolean setInput(char * data);
		char* getOutput();
        void reset();
		void dumpState();

		using Print::write;

	protected:
		uint8_t *rBuff;
        size_t _rSize;
        size_t _rp;
		uint8_t *wBuff;
        size_t _wSize;
        size_t _wp;
};

#endif