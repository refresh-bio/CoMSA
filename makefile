all: msac

MSAC_ROOT_DIR = .
MSAC_MAIN_DIR = src
MSAC_LIBS_DIR = src/libs

CC 	= g++
CFLAGS	= -Wall -O3 -m64 -std=c++14 -pthread -I $(MSAC_LIBS_DIR)
CLINK	= -Wall -O3 -m64 -static -Wl,--whole-archive -lpthread -Wl,--no-whole-archive -std=c++14

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@

msac: $(MSAC_MAIN_DIR)/MSAC.o \
	$(MSAC_MAIN_DIR)/entropy.o \
	$(MSAC_MAIN_DIR)/fasta_file.o \
	$(MSAC_MAIN_DIR)/lzma_wrapper.o \
	$(MSAC_MAIN_DIR)/msa.o \
	$(MSAC_MAIN_DIR)/pbwt.o \
	$(MSAC_MAIN_DIR)/queue.o \
	$(MSAC_MAIN_DIR)/rle.o \
	$(MSAC_MAIN_DIR)/stockholm.o \
	$(MSAC_MAIN_DIR)/transpose.o \
	$(MSAC_MAIN_DIR)/mtf.o \
	$(MSAC_MAIN_DIR)/wfc.o
	$(CC) $(CLINK) -o $(MSAC_ROOT_DIR)/$@  \
	$(MSAC_MAIN_DIR)/MSAC.o \
	$(MSAC_MAIN_DIR)/entropy.o \
	$(MSAC_MAIN_DIR)/fasta_file.o \
	$(MSAC_MAIN_DIR)/lzma_wrapper.o \
	$(MSAC_MAIN_DIR)/msa.o \
	$(MSAC_MAIN_DIR)/pbwt.o \
	$(MSAC_MAIN_DIR)/queue.o \
	$(MSAC_MAIN_DIR)/rle.o \
	$(MSAC_MAIN_DIR)/stockholm.o \
	$(MSAC_MAIN_DIR)/transpose.o \
	$(MSAC_MAIN_DIR)/mtf.o \
	$(MSAC_MAIN_DIR)/wfc.o \
	$(MSAC_LIBS_DIR)/liblzma.a
clean:
	-rm $(MSAC_MAIN_DIR)/*.o
	-rm msac
	
