#include "config.h"

const char default_config[] = DEFAULT_CONFIG;

File config_file;
char *config_content = (char *)malloc(sizeof(default_config));
uint8_t *threshold = (uint8_t *)malloc(32);

bool is_setting_threshold = false;
CY8CMBR3116 *_left;
CY8CMBR3116 *_right;

void write_config() {
  config_file = LittleFS.open(CONFIG_FILE, "w");
  config_file.write(config_content, sizeof(default_config));
  config_file.close();
}

void init_config(CY8CMBR3116 *left, CY8CMBR3116 *right) {
  _left = left;
  _right = right;
  memset(config_content, 0, sizeof(default_config));

  LittleFS.begin();
  if (!LittleFS.exists(CONFIG_FILE)) {
    config_file = LittleFS.open(CONFIG_FILE, "w");

    config_file.write(default_config, sizeof(default_config));
    config_file.close();
  }

  config_file = LittleFS.open(CONFIG_FILE, "r");
  if (config_file.size() != sizeof(default_config)) {
    config_file.close();
    LittleFS.remove(CONFIG_FILE);
    config_file = LittleFS.open(CONFIG_FILE, "w");

    config_file.write(default_config, sizeof(default_config));
    config_file.close();

    config_file = LittleFS.open(CONFIG_FILE, "r");
  }

  config_file.readBytes(config_content, sizeof(default_config));

  if (config_content[0] != CONFIG_VERSION) {
    // TODO: 升级CONFIG VERSION
  }

  config_file.close();

  memcpy(threshold, config_content + 1, 32);

  for (uint8_t i = 0; i < 32; i++) {
    Serial.print(threshold[i]);
    Serial.print(' ');
  }
  Serial.println();

  write_threshold_config();
}

uint8_t *threshold_buf;
uint8_t *threshold_debounce;

void threshold_setting_start() {
  is_setting_threshold = true;
  threshold_buf = (uint8_t *)malloc(32);
  threshold_debounce = (uint8_t *)malloc(32);
  memcpy(threshold_buf, threshold, 32);
  memset(threshold_debounce, 0, 32);
}

void threshold_setting_loop() {
  if (is_setting_threshold) {
    for (int i = 0; i < 16; i++) {
      uint8_t value = (uint8_t)(_right->read_touched_data(i));
      if (threshold_buf[i] < value) {
        if (threshold_debounce[i] < 3) {
          threshold_debounce++;
        } else {
          threshold_buf[i] = value;
          threshold_debounce[i] = 0;
        }
      }
    }
    for (int i = 0; i < 16; i++) {
      uint8_t value = (uint8_t)(_left->read_touched_data(i));
      if (threshold_buf[i + 16] < value) {
        if (threshold_debounce[i + 16] < 3) {
          threshold_debounce++;
        } else {
          threshold_buf[i + 16] = value;
          threshold_debounce[i + 16] = 0;
        }
      }
    }
  }
}

void threshold_setting_end() {
  if (is_setting_threshold) {
    is_setting_threshold = false;
    memcpy(threshold, threshold_buf, 32);
    free(threshold_buf);
    free(threshold_debounce);

    for (uint8_t i = 0; i < 32; i++) {
      Serial.print(threshold[i]);
      Serial.print(' ');
    }
    Serial.println();

    memcpy(config_content + 1, threshold, 32);
    write_config();
  }
}

void write_threshold_config() {
  _right->_config_data[BASE_THRESHOLD0] = threshold[0];
  _right->_config_data[BASE_THRESHOLD1] = threshold[1];
  _right->_config_data[FINGER_THRESHOLD2] = threshold[2];
  _right->_config_data[FINGER_THRESHOLD3] = threshold[3];
  _right->_config_data[FINGER_THRESHOLD4] = threshold[4];
  _right->_config_data[FINGER_THRESHOLD5] = threshold[5];
  _right->_config_data[FINGER_THRESHOLD6] = threshold[6];
  _right->_config_data[FINGER_THRESHOLD7] = threshold[7];
  _right->_config_data[FINGER_THRESHOLD8] = threshold[8];
  _right->_config_data[FINGER_THRESHOLD9] = threshold[9];
  _right->_config_data[FINGER_THRESHOLD10] = threshold[10];
  _right->_config_data[FINGER_THRESHOLD11] = threshold[11];
  _right->_config_data[FINGER_THRESHOLD12] = threshold[12];
  _right->_config_data[FINGER_THRESHOLD13] = threshold[13];
  _right->_config_data[FINGER_THRESHOLD14] = threshold[14];
  _right->_config_data[FINGER_THRESHOLD15] = threshold[15];

  _left->_config_data[BASE_THRESHOLD0] = threshold[16];
  _left->_config_data[BASE_THRESHOLD1] = threshold[17];
  _left->_config_data[FINGER_THRESHOLD2] = threshold[18];
  _left->_config_data[FINGER_THRESHOLD3] = threshold[19];
  _left->_config_data[FINGER_THRESHOLD4] = threshold[20];
  _left->_config_data[FINGER_THRESHOLD5] = threshold[21];
  _left->_config_data[FINGER_THRESHOLD6] = threshold[22];
  _left->_config_data[FINGER_THRESHOLD7] = threshold[23];
  _left->_config_data[FINGER_THRESHOLD8] = threshold[24];
  _left->_config_data[FINGER_THRESHOLD9] = threshold[25];
  _left->_config_data[FINGER_THRESHOLD10] = threshold[26];
  _left->_config_data[FINGER_THRESHOLD11] = threshold[27];
  _left->_config_data[FINGER_THRESHOLD12] = threshold[28];
  _left->_config_data[FINGER_THRESHOLD13] = threshold[29];
  _left->_config_data[FINGER_THRESHOLD14] = threshold[30];
  _left->_config_data[FINGER_THRESHOLD15] = threshold[31];
}