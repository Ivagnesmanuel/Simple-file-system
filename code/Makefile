CCOPTS = -g -std=gnu99 -Wstrict-prototypes
LIBS = 
CC = gcc
AR = ar

HEADERS = bitmap.h\
		disk_driver.h\
		simplefs.h
 
shell: shell.c bitmap.c disk_driver.c simplefs.c $(HEADERS)
	$(CC) $(CCOPTS) shell.c bitmap.c disk_driver.c simplefs.c -o shell
	
simplefs_test: bitmap.c disk_driver.c simplefs.c simplefs_test.c $(HEADERS)
	$(CC) $(CCOPTS) bitmap.c disk_driver.c simplefs.c simplefs_test.c -o simplefs_test
	
test_disk_driver: bitmap.c disk_driver.c test_disk_driver.c $(HEADERS)
	$(CC) $(CCOPTS) bitmap.c disk_driver.c test_disk_driver.c -o test_disk_driver

test_bitmap: bitmap.c test_bitmap.c $(HEADERS)
	$(CC) $(CCOPTS) bitmap.c test_bitmap.c -o test_bitmap
	
.phony: clean

clean:
	rm -f *.o shell test_bitmap test_disk_driver simplefs_test disk_file.txt disco.disk
