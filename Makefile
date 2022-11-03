default:
	gcc -Werror -Wall -pthread -O -o psort psort.c

clean: 
	rm -f *.tex *.dvi *.idx *.aux *.log *.ind *.ilg \
	*.o *.d *.asm *.sym vectors.S bootblock entryother \
	initcode initcode.out kernel xv6.img fs.img kernelmemfs \
	xv6memfs.img mkfs .gdbinit \
	
generate:
	gcc -Werror -Wall -pthread -O -o testcase_generator testcase_generator.c