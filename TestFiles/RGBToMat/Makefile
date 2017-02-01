
DEPENDENCIES = 
SOURCEFILES = $(wildcard *.cpp)
OBJECTS = $(SOURCEFILES:.cpp=.o)
OUTPUTNAME = RGBMat
FLAGS = -Wall -Wextra -std=c++11
LIBS = `pkg-config opencv librealsense --cflags --libs`

#If any of the matching .cpp files changed or any of the deps changed,
#Remake the object files
%.o : %.cpp $(DEPENDENCIES)
	g++ $(FLAGS) -c -o $@ $< 

#If any of the objects was changed, see above, remake exe
make : $(OBJECTS)
	g++  $(FLAGS) $(SOURCEFILES) -o $(OUTPUTNAME) $(LIBS)

#Remove the executables
clean : 
	rm $(OUTPUTNAME) $(OBJECTS)
