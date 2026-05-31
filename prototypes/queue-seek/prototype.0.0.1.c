#include <stdio.h>
#include "../../aesd-char-driver/aesd-circular-buffer.c"
#include "../../aesd-char-driver/aesd_ioctl.h"


  int main() {

      struct aesd_circular_buffer buffer;

      aesd_circular_buffer_init(&buffer);

      struct aesd_buffer_entry e0 = { "Grass\n", 6 };

      struct aesd_buffer_entry e1 = { "Sentosa\n", 6 };

      struct aesd_buffer_entry e2 = { "Singapore\n",  5 };

      aesd_circular_buffer_add_entry(&buffer, &e0);
      aesd_circular_buffer_add_entry(&buffer, &e1);
      aesd_circular_buffer_add_entry(&buffer, &e2);

      

      return 0;
  }