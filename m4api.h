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

#ifndef M4API_H
#define M4API_H

enum m4Type {
  M4_VLT_12_11,
  M4_VLT_12_07,
  M4_VLT_5_03,
  M4_VLT_33_01,
  M4_DEG,
  M4_SEC,
  M4_MSC_1_1, // 1 byte
  M4_MSC_1_2, // 2 bytes
  M4_MSC_10_1, // 1 byte
  M4_MSC_10_2, // 2 bytes
  M4_TIM, // 2 bytes
  M4_TRY,
  M4_BYT
};

enum m4FieldID {
  M4_IGN_HIGH,
  M4_IGN_LOW,
  M4_IGN_DBC,
  M4_PSU_DELAY,
  M4_VIN_MIN_START,
  M4_VIN_MIN_ON,
  M4_VIN_MIN_5V,
  M4_VIN_MAX,
  M4_12V_MAX,
  M4_12V_MIN,
  M4_5V_MAX,
  M4_5V_MIN,
  M4_33V_MAX,
  M4_33V_MIN,
  M4_12V_TIME,
  M4_33V_TIME,
  M4_PWRSW,
  M4_PSU_ON_TIME,
  M4_ON_DELAY,
  M4_PSU_OFF_TIME,
  M4_OFF_DELAY,
  M4_EMG_TIME_5VSB,
  M4_EMG_TIMER,
  M4_PS_ON_0,
  M4_PS_ON_1,
  M4_THUMP,
  M4_TEMP_MAX,
  M4_TEMP_MIN,
  M4_EMG_OFF_MODE,
  M4_5V_SBY_DLY,
  M4_OFF_DELAY_0,
  M4_OFF_HARD_0,
  M4_OFF_DELAY_1,
  M4_OFF_HARD_1,
  M4_OFF_DELAY_2,
  M4_OFF_HARD_2,
  M4_OFF_DELAY_3,
  M4_OFF_HARD_3,
  M4_OFF_DELAY_4,
  M4_OFF_HARD_4,
  M4_OFF_DELAY_5,
  M4_OFF_HARD_5,
  M4_OFF_DELAY_6,
  M4_OFF_HARD_6,
  M4_OFF_DELAY_7,
  M4_OFF_HARD_7,
  M4_RESET,
  M4_NUM_CONFIG_FIELDS
};

enum m4Repr {
  M4_INTEG,
  M4_FLOAT,
  M4_TIMER
};

struct m4DiagField {
  enum m4Type type;
  unsigned int index;
  char *name;
  char *desc;
};

struct m4ConfigField {
  enum m4Type type;
  unsigned int index;
  char *name;
  char *desc;
};

struct m4Diagnostics {
  float vin;
  float vign;
  float v33;
  float v5;
  float v12;
  float temp;
};

extern struct m4DiagField m4DiagFields[];
extern struct m4ConfigField m4ConfigFields[];
extern char* m4TypeDescs[];

/* Find and open the PSU */
struct usb_dev_handle *m4Init();

/* User-friendly-ish routines for setting/getting config values */
int m4GetFloat(struct usb_dev_handle *dev, enum m4FieldID fid, float *out);
int m4SetFloat(struct usb_dev_handle *dev, enum m4FieldID fid, float val);
int m4GetInteger(struct usb_dev_handle *dev, enum m4FieldID fid, int *out);
int m4SetInteger(struct usb_dev_handle *dev, enum m4FieldID fid, int val);

/* Get the diagnostic string in its raw form (<0 = error) */
int m4FetchDiag (struct usb_dev_handle *dev, char *buf);

/* Get the diagnostic values in processed form (<0 = error) */
int m4GetDiag (struct usb_dev_handle *dev, struct m4Diagnostics *diag);

/* Get a floating-point representation of the value stored at specified memory location */
float m4GetVal(enum m4Type type, char *posn);

/* Print the formatted value of the element of type `type' and value `val' */
void m4PrintVal(enum m4Type type, float val);

/* Load the field's value from the PSU into buf */
int m4GetConfig(struct usb_dev_handle *dev, struct m4ConfigField *field, char *buf);

/* Parse a value (123, 1.23 or 00:00:10) into buffer, encoding it */
int m4ParseValue(enum m4Type type, char const *strval, char *buf);

/* Write the human-readable value to the field on the PSU */
int m4SetConfig(struct usb_dev_handle *dev, struct m4ConfigField *field, char const *strval);

/* Print the status variables in the (raw) buffer */
void m4PrintDiag(char *buf);

/* Find the m4ConfigField of the given name */
int m4ConfigField(char const *name);

#endif
