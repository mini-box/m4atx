#include <stdio.h>
#include "m4api.h"

int main (int argc, char **argv) {
  float fval;
  int ival; /* m4api will handle conversion to 8- or 16-bit integer */
  struct m4Diagnostics diag;

  struct usb_dev_handle *dev;
  char user_buf[64];

  printf("This example will set your VIN_MIN_ON to 10 V, and it will\n"
         "increase IGN_DBC by 10 ms. OK? Type 'yes' or 'no': ");

  if (NULL == fgets(user_buf, sizeof(user_buf), stdin)
      || user_buf[0] != 'y') {
    printf("Exiting.\n");
    return -1;
  }

  dev = m4Init();

  if (!dev) {
    perror("m4Init");
    return -1;
  }

  /* Read in diagnostics */
  if (m4GetDiag(dev, &diag))
    return -1;

  printf("VIN = %0.2f\n", diag.vin);
  printf("V33 = %0.2f\n", diag.v33);

  /* Get and update a voltage level */
  if (m4GetFloat(dev, M4_VIN_MIN_ON, &fval))
    return -1;

  printf("VIN_MIN_ON was %0.2f", fval);

  fval = 10.0;
  if (m4SetFloat(dev, M4_VIN_MIN_ON, fval))
    return -1;

  if (m4GetFloat(dev, M4_VIN_MIN_ON, &fval))
    return -1;

  printf(", now %0.2f\n", fval);

  /* Update an integer */
  if (m4GetInteger(dev, M4_IGN_DBC, &ival))
    return -1;

  printf("IGN_DBC was %d", ival);

  ival += 10;
  if (m4SetInteger(dev, M4_IGN_DBC, ival))
    return -1;

  if (m4GetInteger(dev, M4_IGN_DBC, &ival))
    return -1;

  printf (", now %d\n", ival);

  return 0;
}
