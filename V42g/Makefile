# makefile pour MinGW

CCOPT = -Wall -O2
C_SRC = avi_nc.c quant.c decode.c encode.c draw.c appli.c
CPP_SRC =
EXE = v42.exe

OBJS = $(C_SRC:.c=.o) $(CPP_SRC:.cpp=.o)

# linkage
$(EXE) : $(OBJS)
	g++ -o $(EXE) $(OBJS)

# compilage
.c.o :
	gcc $(CCOPT) -c $<

.cpp.o :
	g++ $(CCOPT) -c $<
# other

clean :
	del *.o; del *.exe

# dependances : 
avi_nc.o : avi_nc.h
quant.o : v42.h
decode.o : decode.h v42.h avi_nc.h draw.h
encode.o : encode.h decode.h v42.h avi_nc.h
draw.o : draw.h
appli.o : encode.h decode.h v42.h

