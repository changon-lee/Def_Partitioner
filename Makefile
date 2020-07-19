CPP = g++
CFLAGS =
TARGET = partitioner
OBJDIR = ./obj/
CPPDIR = ./src/
DEBUG = 0

AR = ar
ARFLAGS = rcs
ALIB = lib.a
OBJ = main.o pystring.o def2hgr.o def2verilog.o hgr2verilog.o
OBJS = $(addprefix $(OBJDIR), $(OBJ))

$(TARGET) : $(ALIB)
	$(CPP) $(CFLAGS) $^ -o $@ $(ALIB)
$(ALIB) : $(OBJS)
	$(AR) $(ARFLAGS) $@ $^
$(OBJDIR)%.o : $(CPPDIR)%.cpp
	$(CPP) $(CFLAGS) -c  $< -o $@


clean :
	rm -rf $(TARGET) $(OBJDIR)/* $(ALIB)

