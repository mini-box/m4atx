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
  {M4_VLT_12_11, 1, "VIN", "Input voltage"},
  {M4_VLT_12_11, 2, "IGN", "Ignition voltage"},
  {M4_VLT_33_01, 3, "33V", "Voltage on 3.3V rail"},
  {M4_VLT_5_03, 4, "5V", "Voltage on 5V rail"},
  {M4_VLT_12_07, 5, "12V", "Voltage on 12V rail"},
  {M4_DEG, 11, "TEMP", "Temperature"},
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
  {M4_TIM, 0x22, "OFF_HARD_1", "Hard-off MODE 0"},
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

size_t m4NumConfigFields = sizeof(m4ConfigFields) / sizeof(m4ConfigFields[0]);

int m4FetchDiag (int fd, char *buf) {
  struct hiddev_report_info rinfo;
  struct hiddev_field_info finfo;
  struct hiddev_usage_ref uref;
  int i, j, ret;

  /* load 0x21 report info from system cache */
  rinfo.report_type = HID_REPORT_TYPE_INPUT;
  rinfo.report_id = 0x21;
  ret = ioctl(fd, HIDIOCGREPORTINFO, &rinfo);

  /* send an 0x81 report to request an 0x21 */
  rinfo.report_type = HID_REPORT_TYPE_OUTPUT;
  rinfo.report_id = 0x81;
  rinfo.num_fields = 1;
  ret = ioctl(fd, HIDIOCSREPORT, &rinfo);
  if (ret < 0) {
    return -1;
  }

  /* read in the 0x21 response */
  rinfo.report_type = HID_REPORT_TYPE_INPUT;
  rinfo.report_id = 0x21;
  rinfo.num_fields = 1;
  ret = ioctl(fd, HIDIOCGREPORT, &rinfo);
  if (ret < 0) {
    return -1;
  }


  struct hiddev_usage_ref_multi muref;
  muref.uref.report_type = HID_REPORT_TYPE_INPUT;
  muref.uref.report_id = 0x21;
  muref.uref.field_index = 0;
  muref.uref.usage_index = 0;
  muref.num_values = 23;
  ret = ioctl(fd, HIDIOCGUSAGES, &muref);
  if (ret < 0) {
    return -1;
  }

  for (i = 0; i < 23; ++i) {
    buf[i] = muref.values[i];
  }

  return 0;
}

void m4PrintVal(int type, char *posn) {
  float val;
  short tmp_sh;
  int tmp_i, sec, min, hr;

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

int m4GetConfig(int fd, struct m4ConfigField *field, char *buf) {
  struct hiddev_report_info rinfo;
  struct hiddev_field_info finfo;
  struct hiddev_usage_ref_multi muref;
  int i, j, ret;
  
  /* prepare an 0xa4 report */
  rinfo.report_type = HID_REPORT_TYPE_OUTPUT;
  rinfo.report_id = 0xa4;
  rinfo.num_fields = 1;
  muref.uref.report_type = HID_REPORT_TYPE_OUTPUT;
  muref.uref.report_id = 0xa4;
  muref.uref.field_index = 0;
  muref.uref.usage_index = 0;
  muref.num_values = 3;

  muref.values[0] = 0xa1;
  muref.values[1] = field->index;
  muref.values[2] = m4TypeLengths[field->type];

  /* post the usage values to the system's buffer */
  ret = ioctl(fd, HIDIOCSUSAGES, &muref);
  if (ret < 0) {
    return -1;
  }

  /* send the 0xa4 report, with usages */
  ret = ioctl(fd, HIDIOCSREPORT, &rinfo);
  if (ret < 0) {
    return -1;
  }
  
  /* read in the 0x31 response */
  rinfo.report_type = HID_REPORT_TYPE_INPUT;
  rinfo.report_id = 0x31;
  rinfo.num_fields = 1;
  ret = ioctl(fd, HIDIOCGREPORT, &rinfo);
  if (ret < 0) {
    return -1;
  }

  muref.values[1] = 0xff;

  /* now pull the usages from the kernel */
  muref.uref.report_type = HID_REPORT_TYPE_INPUT;
  muref.uref.report_id = 0x31;
  muref.uref.field_index = 0;
  muref.uref.usage_index = 0;
  muref.num_values = 23;

  do {
    ret = ioctl(fd, HIDIOCGUSAGES, &muref);
    if (ret < 0) {
      return -1;
    }
  } while (muref.values[1] != field->index && !usleep(5000));

  for (i = 0; i < 23; ++i) {
    buf[i] = muref.values[i];
  }
  
  return 0;
  
}

int m4ParseValue(int type, char const *strval, char *buf) {
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

int m4SetConfig(int fd, struct m4ConfigField *field, char const *strval) {
  char buf[2];

  if (m4ParseValue(field->type, strval, buf) < 0) {
    fprintf(stderr, "%s: Invalid value for %s\n", strval, field->name);
    return -1;
  }

  struct hiddev_usage_ref_multi muref;
  struct hiddev_report_info rinfo;
  struct hiddev_field_info finfo;
  int ret;

  /* prepare an 0xa4 report */
  rinfo.report_type = HID_REPORT_TYPE_OUTPUT;
  rinfo.report_id = 0xa4;
  rinfo.num_fields = 1;
  muref.uref.report_type = HID_REPORT_TYPE_OUTPUT;
  muref.uref.report_id = 0xa4;
  muref.uref.field_index = 0;
  muref.uref.usage_index = 0;
  muref.num_values = 5;

  muref.values[0] = 0xa0;
  muref.values[1] = field->index;
  muref.values[2] = m4TypeLengths[field->type];
  muref.values[3] = 0xff & buf[0];
  muref.values[4] = 0xff & buf[1];

  /* post the usage values to the system's buffer */
  ret = ioctl(fd, HIDIOCSUSAGES, &muref);
  if (ret < 0) {
    return -1;
  }

  /* send the 0xa4 report, with usages */
  ret = ioctl(fd, HIDIOCSREPORT, &rinfo);
  if (ret < 0) {
    return -1;
  }

  /* TODO: Wait for and confirm the 0x31 report to make sure it worked. */

  return 0;
}

void m4PrintDiag(char *buf) {
  int field_id;
  size_t config_offset = 0;

  for (field_id = 0; field_id < m4NumDiagFields; ++field_id) {
    printf("%s:\t", m4DiagFields[field_id].name);
    m4PrintVal(m4DiagFields[field_id].type, buf + m4DiagFields[field_id].index);
    puts("");
  }
}

int m4ConfigField(char const *name) {
  int field_id;

  for (field_id = 0; field_id < m4NumConfigFields; ++field_id) {
    if (!strcasecmp(m4ConfigFields[field_id].name, name))
      return field_id;
  }

  return -1;
}

