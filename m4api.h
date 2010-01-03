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

#define M4_VLT_12_11 0
#define M4_VLT_12_07 1
#define M4_VLT_5_03 2
#define M4_VLT_33_01 3
#define M4_DEG 4
#define M4_SEC 5
#define M4_MSC_1_1 6 // 1 byte
#define M4_MSC_1_2 7 // 2 bytes
#define M4_MSC_10_1 8 // 1 byte
#define M4_MSC_10_2 9 // 2 bytes
#define M4_TIM 10 // 2 bytes
#define M4_TRY 11
#define M4_BYT 12

#define M4_INTEG 1
#define M4_FLOAT 2
#define M4_TIMER 3

struct m4DiagField {
  int type;
  int index;
  char *name;
  char *desc;
};

struct m4ConfigField {
  int type;
  size_t index;
  char *name;
  char *desc;
};

extern struct m4DiagField m4DiagFields[];
extern size_t m4NumDiagFields;
extern struct m4ConfigField m4ConfigFields[];
extern size_t m4NumConfigFields;
extern char* m4TypeDescs[];

/* Get the diagnostic string in its raw form (<0 = error) */
int m4FetchDiag (int fd, char *buf);

/* Print the formatted value of the element stored at posn[0] */
void m4PrintVal(int type, char *posn);

/* Load the field's value from the PSU into buf */
int m4GetConfig(int fd, struct m4ConfigField *field, char *buf);

/* Parse a value (123, 1.23 or 00:00:10) into buffer, encoding it */
int m4ParseValue(int type, char const *strval, char *buf);

/* Write the human-readable value to the field on the PSU */
int m4SetConfig(int fd, struct m4ConfigField *field, char const *strval);

/* Print the status variables in the (raw) buffer */
void m4PrintDiag(char *buf);

/* Find the m4ConfigField of the given name */
int m4ConfigField(char const *name);

#endif
