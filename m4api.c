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

static size_t m4TypeLengths[13] = {
 1, 1, 1, 1, 1, 1, 1, 2,
 1, 2, 2, 1, 1
};

static float m4TypeConversions[13] = {
 0.1123, 0.076625, 0.0375, 0.0188136,
 1.0, 1.0, 1.0, 1.0,
 10.0, 10.0, 1.0, 1.0,
 1.0
};

static int m4TypeForms[13] = {
 M4_FLOAT, M4_FLOAT, M4_FLOAT, M4_FLOAT,
 M4_INTEG, M4_INTEG, M4_INTEG, M4_INTEG,
 M4_INTEG, M4_INTEG, M4_TIMER, M4_INTEG,
 M4_INTEG
};

char* m4TypeDescs[14] = {
  "V", "V", "V", "V",
  "deg. C", "sec", "ms", "ms",
  "ms", "ms", "", "tries",
  ""
};

struct m4DiagField m4DiagFields[6] = {
  {M4_VLT_12_11, 2, "VIN", "Input voltage"},
  {M4_VLT_12_11, 3, "IGN", "Ignition voltage"},
  {M4_VLT_33_01, 4, "33V", "Voltage on 3.3V rail"},
  {M4_VLT_5_03, 5, "5V", "Voltage on 5V rail"},
  {M4_VLT_12_07, 6, "12V", "Voltage on 12V rail"},
  {M4_DEG, 12, "TEMP", "Temperature"},
};

size_t m4NumDiagFields = sizeof(m4DiagFields) / sizeof(m4DiagFields[0]);

struct m4ConfigField m4ConfigFields[47] = {
  {M4_VLT_12_11, 0x00, "IGN_HIGH", "Ignition HIGH limit"},
  {M4_VLT_12_11, 0x01, "IGN_LOW", "Ignition LOW limit"},
  {M4_MSC_10_1, 0x02, "IGN_DBC", "Ignition debounce"},
  {M4_SEC, 0x03, "PSU_DELAY", "Delay before PSU startup"},
  {M4_VLT_12_11, 0x04, "VIN_MIN_START", "Min. VIN voltage at PSU startup"},
  {M4_VLT_12_11, 0x05, "VIN_MIN_ON", "Min. VIN voltage while all rails are on"},
  {M4_VLT_12_11, 0x06, "VIN_MIN_5V", "Min. VIN voltage while 5VSB is on"},
  {M4_VLT_12_11, 0x07, "VIN_MAX", "Max. allowed VIN voltage"},
  {M4_VLT_12_07, 0x08, "12V_MAX", "Max. limit for 12V"},
  {M4_VLT_12_07, 0x09, "12V_MIN", "Min. limit for 12V"},
  {M4_VLT_5_03, 0x0a, "5V_MAX", "Max. limit for 5V"},
  {M4_VLT_5_03, 0x0b, "5V_MIN", "Min. limit for 5V"},
  {M4_VLT_33_01, 0x0c, "33V_MAX", "Max. limit for 3.3V"},
  {M4_VLT_33_01, 0x0d, "33V_MIN", "Min. limit for 3.3V"},
  {M4_MSC_1_1, 0x0e, "12V_TIME", "12V rail startup time"},
  {M4_MSC_1_1, 0x0f, "33V_TIME", "3.3V rail startup time"},
  {M4_MSC_10_2, 0x10, "PWRSW", "PWRSW 'push-down' time"},
  {M4_TRY, 0x12, "PSU_ON_TIME", "Try to turn ON the PSU for N time"},
  {M4_SEC, 0x13, "ON_DELAY", "Delay between ON tries"},
  {M4_TRY, 0x14, "PSU_OFF_TIME", "Try to turn OFF the PSU for N time"},
  {M4_SEC, 0x15, "OFF_DELAY", "Delay between OFF tries"},
  {M4_SEC, 0x16, "EMG_TIME_5VSB", "Emergency shutdown timer for 5VSB"},
  {M4_SEC, 0x17, "EMG_TIMER", "Emergency shutdown timer"},
  {M4_MSC_10_1, 0x18, "PS_ON_0", "PS ON 0 threshold"},
  {M4_MSC_10_1, 0x19, "PS_ON_1", "PS ON 1 threshold"},
  {M4_MSC_10_2, 0x1a, "THUMP", "Thump timeout"},
  {M4_DEG, 0x1c, "TEMP_MAX", "Max. temperature"},
  {M4_DEG, 0x1d, "TEMP_MIN", "Min. temperature"},
  {M4_BYT, 0x1e, "EMG_OFF_MODE", "Emergency OFF mode selector"},
  {M4_MSC_10_1, 0x1f, "5V_SBY_DLY", "5V standby PSW-on delay"},
  {M4_TIM, 0x20, "OFF_DELAY_0", "Off-delay MODE 0"},
  {M4_TIM, 0x22, "OFF_HARD_0", "Hard-off MODE 0"},
  {M4_TIM, 0x24, "OFF_DELAY_1", "Off-delay MODE 1"},
  {M4_TIM, 0x26, "OFF_HARD_1", "Hard-off MODE 1"},
  {M4_TIM, 0x28, "OFF_DELAY_2", "Off-delay MODE 2"},
  {M4_TIM, 0x2a, "OFF_HARD_2", "Hard-off MODE 2"},
  {M4_TIM, 0x2c, "OFF_DELAY_3", "Off-delay MODE 3"},
  {M4_TIM, 0x2e, "OFF_HARD_3", "Hard-off MODE 3"},
  {M4_TIM, 0x30, "OFF_DELAY_4", "Off-delay MODE 4"},
  {M4_TIM, 0x32, "OFF_HARD_4", "Hard-off MODE 4"},
  {M4_TIM, 0x34, "OFF_DELAY_5", "Off-delay MODE 5"},
  {M4_TIM, 0x36, "OFF_HARD_5", "Hard-off MODE 5"},
  {M4_TIM, 0x38, "OFF_DELAY_6", "Off-delay MODE 6"},
  {M4_TIM, 0x3a, "OFF_HARD_6", "Hard-off MODE 6"},
  {M4_TIM, 0x3c, "OFF_DELAY_7", "Off-delay MODE 7"},
  {M4_TIM, 0x3e, "OFF_HARD_7", "Hard-off MODE 7"},
  /* big gap with no (known) fields */
  {M4_BYT, 0xff, "RESET", "Reset to factory defaults"},
};

#define VENDOR 0x04d8
#define PRODUCT 0xd001
#define READ_ENDPOINT 0x81
#define WRITE_ENDPOINT 0x01
#define TIMEOUT 3000

usb_dev_handle *m4Init() {
  struct usb_bus *bus;
  struct usb_device *dev;

  usb_init();

  if (usb_find_busses() < 0) {
    return NULL;
  }

  if (usb_find_devices() < 0) {
    return NULL;
  }

  bus = usb_get_busses();

  while (bus) {
    dev = bus->devices;

    while (dev) {
      if (dev->descriptor.idVendor == VENDOR &&
          dev->descriptor.idProduct == PRODUCT) {
	usb_dev_handle *handle = usb_open(dev);

	if (handle) {
#ifdef LIBUSB_HAS_DETACH_KERNEL_DRIVER_NP
          /* Linux usually claims HID devices for its usbhid driver. */
          usb_detach_kernel_driver_np(handle, 0);
#endif
	  if (usb_set_configuration(handle, 1) >= 0) {
	    if (usb_claim_interface(handle, 0) >= 0) {
	      if (usb_set_altinterface(handle, 0) < 0) {
	        usb_close(handle);
		return NULL;
              }
	    } else {
	      usb_close(handle);
	      return NULL;
	    }
	  } else {
	    usb_close(handle);
	    return NULL;
	  }

	  return handle;
	}
      }

      dev = dev->next;
    }

    bus = bus->next;
  }

  return NULL;
}

int m4Read(usb_dev_handle *dev, unsigned char *buf, unsigned int len, int timeout) {
  return usb_interrupt_read(dev, READ_ENDPOINT, (char*) buf, len, timeout);
}

int m4Write(usb_dev_handle *dev, unsigned char *buf, unsigned int len, int timeout) {
  return usb_interrupt_write(dev, WRITE_ENDPOINT, (char*) buf, len, timeout);
}

int m4FetchDiag (usb_dev_handle *dev, char *buf) {
  unsigned char pollCmd[] = {0x81, 0x00};
  
  if (m4Write(dev, pollCmd, 2, TIMEOUT) != 2)
    return -1;

  if (m4Read(dev, buf, 24, TIMEOUT) != 24)
    return -1;

  if (buf[0] != 0x21)
    return -1;
 
  return 0;
}

int m4GetDiag (usb_dev_handle *dev, struct m4Diagnostics *diag) {
  char buf[24];
  int field_id;

  if (m4FetchDiag(dev, buf) < 0)
    return -1;

  diag->vin = m4GetVal(M4_VLT_12_11, buf + 2);
  diag->vign = m4GetVal(M4_VLT_12_11, buf + 3);
  diag->v33 = m4GetVal(M4_VLT_33_01, buf + 4);
  diag->v5 = m4GetVal(M4_VLT_5_03, buf + 5);
  diag->v12 = m4GetVal(M4_VLT_12_07, buf + 6);
  diag->temp = m4GetVal(M4_DEG, buf + 12);

  return 0;
}

float m4GetVal(enum m4Type type, char *posn) {
  float val;
  short tmp_sh;
  int tmp_i;

  switch (m4TypeLengths[type]) {
    case 1:
      tmp_i = posn[0];
      if (type != M4_DEG) tmp_i &= 0xff;
      break;
    case 2:
      if (type == M4_DEG) {
        tmp_sh = (posn[0] << 8) | (posn[1]);
        tmp_i = (float) tmp_sh;
      } else {
        tmp_i = ((unsigned char) posn[0] << 8) | ((unsigned char) posn[1]);
      }
      break;
    default:
      printf("ERROR: typeLen(%d) == %lu!\n", type, m4TypeLengths[type]);
      exit(-1);
  }

  // printf("%x ", tmp_i);

  val = tmp_i * m4TypeConversions[type];

  return val;
}

void m4PrintVal(enum m4Type type, float val) {
  int tmp_i, sec, min, hr;

  switch (m4TypeForms[type]) {
    case M4_INTEG:
      if (type == M4_DEG)
	printf("%+d", (int) val);
      else
	printf("%d", (int) val);
      break;
    case M4_FLOAT:
      printf("%0.2f", val);
      break;
    case M4_TIMER:
      tmp_i = (int) val;

      if (tmp_i == 0xffff) {
	printf("never");
      } else {
	sec = tmp_i % 60;
	min = (tmp_i % 3600) / 60;
	hr = tmp_i / 3600;
	
	printf("%02d:%02d:%02d", hr, min, sec);
      }
      break;
    default:
      printf("ERROR: typeForm(%d) == %d!\n", type, m4TypeForms[type]);
      break;
  }
}

int m4GetConfig(usb_dev_handle *dev, struct m4ConfigField *field, char *buf) {
  unsigned char cmd[24] = {0xa4, 0xa1};

  cmd[2] = field->index;
  cmd[3] = m4TypeLengths[field->type];

  if (m4Write(dev, cmd, 24, TIMEOUT) != 24)
    return -1;

  if (m4Read(dev, buf, 24, TIMEOUT) != 24)
    return -1;

  if (buf[0] != 0x31)
    return -1;

  return 0;
}

int m4ParseValue(enum m4Type type, char const *strval, char *buf) {
  int intval;
  float fval;

  int hr, min, sec;

  switch(m4TypeForms[type]) {
  case M4_INTEG:
    intval = atoi(strval) / (int) m4TypeConversions[type];
    break;
  case M4_FLOAT:
    fval = atof(strval);
    intval = fval / m4TypeConversions[type];
    break;
  case M4_TIMER:
    if (!strcasecmp("never", strval))
      intval = 0xffff;
    else {
      if (sscanf(strval, "%d:%d:%d", &hr, &min, &sec) != 3)
	return -1;
      else
	intval = 3600 * hr + 60 * min + sec;
    }
  }

  if (m4TypeLengths[type] == 2) {
    buf[0] = intval >> 8;
    buf[1] = intval;
  } else {
    buf[0] = intval;
    buf[1] = 0;
  }

  return 0;
}

int m4GetFloat(usb_dev_handle *dev, enum m4FieldID fid, float *out) {
  char buf[24];
  struct m4ConfigField *field;
  
  field = &m4ConfigFields[fid];

  if (m4GetConfig(dev, field, buf))
    return -1;

  *out = m4GetVal(field->type, &buf[4]);

  return 0;
}

int m4SetFloat(usb_dev_handle *dev, enum m4FieldID fid, float val) {
  char binary[2];
  int ival;
  struct m4ConfigField *field;
  
  field = &m4ConfigFields[fid];

  ival = val / m4TypeConversions[field->type];

  if (m4TypeLengths[field->type] == 2) {
    binary[0] = ival >> 8;
    binary[1] = ival;
  } else {
    binary[0] = ival;
    binary[1] = 0;
  }

  return m4SetBinary(dev, field, binary);
}

int m4GetInteger(usb_dev_handle *dev, enum m4FieldID fid, int *out) {
  char buf[24];
  struct m4ConfigField *field;
    
  field = &m4ConfigFields[fid];

  if (m4GetConfig(dev, field, buf))
    return -1;

  *out = (int) m4GetVal(field->type, &buf[4]);

  return 0;
}

int m4SetInteger(usb_dev_handle *dev, enum m4FieldID fid, int val) {
  char binary[2];
  struct m4ConfigField *field;

  field = &m4ConfigFields[fid];

  val /= m4TypeConversions[field->type];

  if (m4TypeLengths[field->type] == 2) {
    binary[0] = val >> 8;
    binary[1] = val;
  } else {
    binary[0] = val;
    binary[1] = 0;
  }

  return m4SetBinary(dev, field, binary);
}

int m4SetConfig(usb_dev_handle *dev, struct m4ConfigField *field, char const *strval) {
  char binary[] = {0, 0};

  if (m4ParseValue(field->type, strval, binary) < 0)
    return -1;

  return m4SetBinary(dev, field, binary);
}

int m4SetBinary(usb_dev_handle *dev, struct m4ConfigField *field, char const *val) {
  char buf[24];
  unsigned char cmd[24] = {0xa4, 0xa0};

  cmd[2] = field->index;
  cmd[3] = m4TypeLengths[field->type];
  cmd[4] = val[0];
  cmd[5] = val[1];

  if (m4Write(dev, cmd, 24, TIMEOUT) != 24)
    return -1;

  if (m4Read(dev, buf, 24, TIMEOUT) != 24)
    return -1;

  if (buf[0] != 0x31 || buf[2] != cmd[2]
      || buf[4] != cmd[4] || buf[5] != cmd[5])
    return -1;

  return 0;
}

void m4PrintDiag(char *buf) {
  int field_id;
  size_t config_offset = 0;
  float value;

  for (field_id = 0; field_id < m4NumDiagFields; ++field_id) {
    printf("%s:\t", m4DiagFields[field_id].name);
    value = m4GetVal(m4DiagFields[field_id].type, buf + m4DiagFields[field_id].index);
    m4PrintVal(m4DiagFields[field_id].type, value);
    puts("");
  }
}

int m4ConfigField(char const *name) {
  int field_id;

  for (field_id = 0; field_id < M4_NUM_CONFIG_FIELDS; ++field_id) {
    if (!strcasecmp(m4ConfigFields[field_id].name, name))
      return field_id;
  }

  return -1;
}

