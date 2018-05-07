CXX = g++
CFLAGS =`pkg-config --cflags opencv` -Wall -g -O3
LIBS = `pkg-config --libs opencv` -L./common/lib/
INCLUDE = -I./ -I./common/include


OBJS_SERVER = capture.o vcompress.o sender.o server.o vshow.o
OBJS_SHOWER = vshow.o recver.o shower.o

all: cam_server cam_client 
cam_server: $(OBJS_SERVER)
	$(CXX) $^ $(LIBS) -o $@ $(LIBS) $(INCLUDE) -lavcodec -lswscale -lavutil -lx264 -lX11 -lXext -pthread -lm

cam_client: $(OBJS_SHOWER)
	$(CXX) $^ $(LIBS) -o $@ $(LIBS) $(INCLUDE) -lavcodec -lswscale -lavutil -lx264 -lX11 -lXext -pthread -lm

%.o:%.cpp
	$(CXX) -c $(CFLAGS) $< $(LIBS) $(INCLUDE) -lavcodec -lswscale -lavutil -lx264 -lX11 -lXext -pthread -lm

clean:
	rm -f *.o 
	rm -f cam_* 
