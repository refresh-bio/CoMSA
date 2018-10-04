all: CoMSA

CoMSA_ROOT_DIR = .
CoMSA_MAIN_DIR = src
CoMSA_LIBS_DIR = src/libs

CC 	= g++
CFLAGS	= -Wall -O3 -m64 -std=c++14 -pthread -I $(CoMSA_LIBS_DIR)
CLINK	= -Wall -O3 -m64 -static -Wl,--whole-archive -lpthread -Wl,--no-whole-archive -std=c++14

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@

CoMSA: $(CoMSA_MAIN_DIR)/CoMSA.o \
	$(CoMSA_MAIN_DIR)/entropy.o \
	$(CoMSA_MAIN_DIR)/fasta_file.o \
	$(CoMSA_MAIN_DIR)/lzma_wrapper.o \
	$(CoMSA_MAIN_DIR)/msa.o \
	$(CoMSA_MAIN_DIR)/pbwt.o \
	$(CoMSA_MAIN_DIR)/queue.o \
	$(CoMSA_MAIN_DIR)/rle.o \
	$(CoMSA_MAIN_DIR)/stockholm.o \
	$(CoMSA_MAIN_DIR)/transpose.o \
	$(CoMSA_MAIN_DIR)/mtf.o \
	$(CoMSA_MAIN_DIR)/wfc.o
	$(CC) $(CLINK) -o $(CoMSA_ROOT_DIR)/$@  \
	$(CoMSA_MAIN_DIR)/CoMSA.o \
	$(CoMSA_MAIN_DIR)/entropy.o \
	$(CoMSA_MAIN_DIR)/fasta_file.o \
	$(CoMSA_MAIN_DIR)/lzma_wrapper.o \
	$(CoMSA_MAIN_DIR)/msa.o \
	$(CoMSA_MAIN_DIR)/pbwt.o \
	$(CoMSA_MAIN_DIR)/queue.o \
	$(CoMSA_MAIN_DIR)/rle.o \
	$(CoMSA_MAIN_DIR)/stockholm.o \
	$(CoMSA_MAIN_DIR)/transpose.o \
	$(CoMSA_MAIN_DIR)/mtf.o \
	$(CoMSA_MAIN_DIR)/wfc.o \
	$(CoMSA_LIBS_DIR)/liblzma.a \
	$(CoMSA_LIBS_DIR)/libz.a
clean:
	-rm $(CoMSA_MAIN_DIR)/*.o
	-rm CoMSA
	
