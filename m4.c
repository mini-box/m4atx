#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <asm/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/hiddev.h>

#include "m4api.h"

int main (int argc, char **argv) {
  int fd = -1;
  int field_id;
  float val;

  if (argc >= 2) {
    if ((fd = open(argv[1], O_RDONLY)) < 0) {
      perror("hiddev open");
      return -1;
    }

    /* we will print a USAGE block later */
  }

  /* by default, print diagnostics */
  if (argc == 2 || (argc >= 3 && !strcmp(argv[2], "-diag"))) {
    char diagBuf[23];
    char repeat = argc >= 4;

    if (repeat) {
      for (;;) {
        if (m4FetchDiag(fd, diagBuf) < 0) {
          puts("ERROR: cannot fetch diagnostics");
	  return -1;
	}

	printf("-- %d\n", time(NULL));	
	m4PrintDiag(diagBuf);

	sleep(1);
      }
    } else {
      if (m4FetchDiag(fd, diagBuf) < 0) {
        puts("ERROR: cannot fetch diagnostics");
        return -1;
      }

      m4PrintDiag(diagBuf);
    }
  } else if (argc >= 3 && !strcmp(argv[2], "-config")) {
    /* with no further args after -config, print them all */
    char configBuf[23];
    if (argc == 3) {
      for (field_id = 0; field_id < m4NumConfigFields; ++field_id) {
	struct m4ConfigField *field = &m4ConfigFields[field_id];
	
	printf("%s:\t", field->name);
	  
	m4GetConfig(fd, field, configBuf);
	  
	m4PrintVal(field->type, &configBuf[3]);
	  
	if (*m4TypeDescs[field->type] != 0)
	  printf(" %s\n", m4TypeDescs[field->type]);
	else
	  puts("");

      }
    } else {
      /* we got a field name; first verify it */
      int field_id = m4ConfigField(argv[3]);

      if (field_id < 0) {
	fprintf(stderr, "%s: Invalid configuration field\n", argv[3]);
	return -1;
      }

      struct m4ConfigField *field = &m4ConfigFields[field_id];

      if (argc >= 5) {
	return m4SetConfig(fd, field, argv[4]);
      } else {
	m4GetConfig(fd, field, configBuf);

	m4PrintVal(field->type, &configBuf[3]);

	if (&m4TypeDescs[field->type] != 0)
	  printf(" %s\n", m4TypeDescs[field->type]);
	else
	  puts("");
      }
    }
  } else {
    printf("USAGE: %s /dev/usb/hiddevX -diag   # Print diagnostic values\n" \
	   "       %s /dev/usb/hiddevX -config # Print all config values\n" \
	   "       %s /dev/usb/hiddevX -config FIELD # Print one field\n" \
	   "       %s /dev/usb/hiddevX -config FIELD VALUE # Set one value\n",
	   argv[0], argv[0], argv[0], argv[0]
	   );

    return -1;
  }

  close(fd);

  return 0;
}
