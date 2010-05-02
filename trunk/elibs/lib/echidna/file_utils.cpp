#include <stdio.h>

// Cross platform way to get the size of a file.
long GetSizeOfFile(FILE* file) {
  // get current postion
  long position = fseek(file, 0, SEEK_CUR);

  fseek(file, 0, SEEK_END);

  // Get the postion
  long size = ftell(file);
  if (size < 0) {
    return -1;
  }

  // Go back it's original position
  fseek(file, position, SEEK_SET);
}



