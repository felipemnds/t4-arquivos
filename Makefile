all:
	gcc -g programaTrab4.c programaTrab4.h main.c funcoesMonitor.c funcoesMonitor.h -o programaTrab4

run:
	./programaTrab4
valgrind:
	gcc -g programaTrab4.c programaTrab4.h main.c funcoesMonitor.c funcoesMonitor.h -o programaTrab4
	valgrind --leak-check=full --track-origins=yes ./programaTrab4
test:
	gcc -g programaTrab4.c programaTrab4.h main.c funcoesMonitor.c funcoesMonitor.h -o programaTrab4
	./programaTrab4 < 14.in > saida.out
unzip: 
	unzip -j 2-casos-teste.zip < unzip.in
	unzip -j 1-arquivos-binarios.zip < unzip.in
meld: 
	gcc -g programaTrab4.c programaTrab4.h main.c funcoesMonitor.c funcoesMonitor.h -o programaTrab4	
	./programaTrab4 < 13.in

	hexdump -Cv binario-13.index > meu.txt
	hexdump -Cv binario-13-depois.index > gab.txt	
	meld meu.txt gab.txt
vizualiza:
	gcc -g programaTrab4.c programaTrab4.h main.c funcoesMonitor.c funcoesMonitor.h -o programaTrab4
	./programaTrab4 < vizu1.in > vis1.txtx	
	./programaTrab4 < vizu2.in > vis2.txt
	
