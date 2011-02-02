/*
 * m4api, a tool for M4-ATX DC-DC power supplies
 * (c) 2009-2010 Ken Tossell <ktossell@umd.edu>
 * 
 * This is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License, version 2.1,
 * as published by the Free Software Foundation.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this software; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <stdio.h>
#include <usb.h>

#include "m4api.h"

int main (int argc, char **argv) {
  int field_id;
  float val;
  usb_dev_handle *dev;

  dev = m4Init();

  if (!dev) {
    perror("m4Init");
    return -1;
  }

  /* by default, print diagnostics */
  if (argc == 1 || (argc >= 2 && !strcmp(argv[1], "-diag"))) {
    char diagBuf[24];
    char repeat = argc >= 3;

    if (repeat) {
      for (;;) {
        if (m4FetchDiag(dev, diagBuf) < 0) {
          puts("ERROR: cannot fetch diagnostics");
	  return -1;
	}

	printf("-- %d\n", time(NULL));	
	m4PrintDiag(diagBuf);

	sleep(1);
      }
    } else {
      if (m4FetchDiag(dev, diagBuf) < 0) {
        puts("ERROR: cannot fetch diagnostics");
        return -1;
      }

      m4PrintDiag(diagBuf);
    }
  } else if (argc >= 2 && !strcmp(argv[1], "-config")) {
    /* with no further args after -config, print them all */
    char configBuf[24];
    if (argc == 2) {
      for (field_id = 0; field_id < M4_NUM_CONFIG_FIELDS; ++field_id) {
	struct m4ConfigField *field = &m4ConfigFields[field_id];
	
	printf("%s:\t", field->name);
	  
	m4GetConfig(dev, field, configBuf);
	  
	m4PrintVal(field->type, m4GetVal(field->type, &configBuf[4]));
	  
	if (*m4TypeDescs[field->type] != 0)
	  printf(" %s\n", m4TypeDescs[field->type]);
	else
	  puts("");

      }
    } else {
      /* we got a field name; first verify it */
      int field_id = m4ConfigField(argv[2]);

      if (field_id < 0) {
	fprintf(stderr, "%s: Invalid configuration field\n", argv[2]);
	return -1;
      }

      struct m4ConfigField *field = &m4ConfigFields[field_id];

      if (argc >= 4) {
	return m4SetConfig(dev, field, argv[3]);
      } else {
	m4GetConfig(dev, field, configBuf);

	m4PrintVal(field->type, m4GetVal(field->type, &configBuf[4]));

	if (&m4TypeDescs[field->type] != 0)
	  printf(" %s\n", m4TypeDescs[field->type]);
	else
	  puts("");
      }
    }
  } else {
    printf("m4ctl version %d.%d\n",
            M4API_VERSION_MAJOR, M4API_VERSION_MINOR);
    printf("USAGE: %s -diag [loop]   # Print diagnostic values\n" \
	   "       %s -config   # Print all config values\n" \
	   "       %s -config FIELD   # Print one field\n" \
	   "       %s -config FIELD VALUE   # Set one value\n",
	   argv[0], argv[0], argv[0], argv[0]
	   );

    return -1;
  }

  return 0;
}

