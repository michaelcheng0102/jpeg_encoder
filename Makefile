TARGET = jpeg_encoder

OBJ = obj
SRC = src
INC = inc

CXX = g++
CXX_LIBS =
CXX_FLAGS = -O2 --std=c++11 -I$(INC)

DEPS = constants.h

ENCODER_OBJS = bmp.o jpeg.o yuv.o quantable.o huffman.o block.o rle.o
OBJS = $(foreach n, $(ENCODER_OBJS), $(OBJ)/$(n))

.PHONY: all clean

all: $(TARGET)

jpeg_encoder: $(OBJS) $(SRC)/main.cpp
	$(CXX) $(CXX_FLAGS) $^ -o $@ $(CXX_LIBS)

$(OBJS): $(OBJ)/%.o: $(SRC)/%.cpp $(INC)/%.h $(foreach n, $(DEPS), $(INC)/$(n))
	$(CXX) -c $< $(CXX_FLAGS) -o $@ $(CXX_LIBS)

clean:
	rm -f $(OBJ)/* $(TARGET)
