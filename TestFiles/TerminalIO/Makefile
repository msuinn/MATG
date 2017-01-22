
OUTPUTNAME = Hello
SOURCEFILES = HelloWorld.cpp
DEPENDENCIES = H.hpp
OBJECTS = HelloWorld.o

#If any of the matching .cpp files changed or any of the deps changed,
#Remake the object files
%.o : %.cpp $(DEPENDENCIES)
	g++ -c -o $@ $< 

#If any of the objects was changed, see above, remake exe
hellomake : $(OBJECTS)
	g++ -Wall -Wextra $(SOURCEFILES) -o $(OUTPUTNAME)

#Remove the executables
clean : 
	rm $(OUTPUTNAME) $(OBJECTS)
