# Prova Finale di Algoritmi e Strutture Dati 
## AA. 2019-2020

Implement a simple text editor with undo/redo functionality, in C99 language.

Ingegneria Informatica, [Politecnico di Milano](https://www.polimi.it/)

## Short commands description
 - **`ind1,ind2c`** changes the text for the lines in between ind1 and ind2. The text following the command must be ind2-ind1+1 lines long, plus one more line with a dot only

- **`ind1,ind2d`** deletes the lines in between ind1 and ind2 (boundaries included)

- **`ind1,ind2p`** prints the lines in between ind1 and ind2

- **`nu`** undos the last n change or delete commands

- **`nr`** redos the last n undone commands 

- **`q`** quit the program
  
## Software
- [CLion](https://www.jetbrains.com/clion/)
- [Valgrind](https://valgrind.org/)
- [GDB](https://www.gnu.org/software/gdb/)

## License & Copyright
Licensed with [MIT License](LICENSE)