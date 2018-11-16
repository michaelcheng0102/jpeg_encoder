TARGET = jpeg_encoder
CXX = g++
CXX_LIBS = -lpthread
CXX_FLAGS = -O2 --std=c++11 -DDEBUG
DEPS = constants.h

ENCODER_OBJS = bmp.o jpeg.o yuv.o quantable.o huffman.o

.PHONY: all clean

all: $(TARGET)

jpeg_encoder: $(ENCODER_OBJS) main.cpp
	$(CXX) $(CXX_FLAGS) $^ -o $@ $(CXX_LIBS)

%.o: %.cpp %.h $(DEPS)
	$(CXX) -c $< $(CXX_FLAGS)

clean:
	rm -f *.o $(TARGET)
