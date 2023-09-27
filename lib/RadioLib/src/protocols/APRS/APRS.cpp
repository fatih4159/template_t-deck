#include "APRS.h"

APRSClient::APRSClient(AX25Client* ax) {
  _ax = ax;
}

int16_t APRSClient::begin(char symbol, bool alt) {
  RADIOLIB_CHECK_RANGE(symbol, ' ', '}', RADIOLIB_ERR_INVALID_SYMBOL);
  _symbol = symbol;

  if(alt) {
    _table = '\\';
  } else {
    _table = '/';
  }

  return(RADIOLIB_ERR_NONE);
}

int16_t APRSClient::sendPosition(char* destCallsign, uint8_t destSSID, char* lat, char* lon, char* msg, char* time) {
  #if !defined(RADIOLIB_STATIC_ONLY)
    size_t len = 1 + strlen(lat) + 1 + strlen(lon);
    if(msg != NULL) {
      len += 1 + strlen(msg);
    }
    if(time != NULL) {
      len += strlen(time);
    }
    char* info = new char[len];
  #else
    char info[RADIOLIB_STATIC_ARRAY_SIZE];
  #endif

  // build the info field
  if((msg == NULL) && (time == NULL)) {
    // no message, no timestamp
    sprintf(info, RADIOLIB_APRS_DATA_TYPE_POSITION_NO_TIME_NO_MSG "%s%c%s%c", lat, _table, lon, _symbol);
  } else if((msg != NULL) && (time == NULL)) {
    // message, no timestamp
    sprintf(info, RADIOLIB_APRS_DATA_TYPE_POSITION_NO_TIME_MSG "%s%c%s%c%s", lat, _table, lon, _symbol, msg);
  } else if((msg == NULL) && (time != NULL)) {
    // timestamp, no message
    sprintf(info, RADIOLIB_APRS_DATA_TYPE_POSITION_TIME_NO_MSG "%s%s%c%s%c", time, lat, _table, lon, _symbol);
  } else {
    // timestamp and message
    sprintf(info, RADIOLIB_APRS_DATA_TYPE_POSITION_TIME_MSG "%s%s%c%s%c%s", time, lat, _table, lon, _symbol, msg);
  }

  // send the frame
  int16_t state = sendFrame(destCallsign, destSSID, info);
  #if !defined(RADIOLIB_STATIC_ONLY)
    delete[] info;
  #endif
  return(state);
}

int16_t APRSClient::sendMicE(float lat, float lon, uint16_t heading, uint16_t speed, uint8_t type, uint8_t* telem, size_t telemLen, char* grid, char* status, int32_t alt) {
  // sanity checks first
  if(((telemLen == 0) && (telem != NULL)) || ((telemLen != 0) && (telem == NULL))) {
    return(RADIOLIB_ERR_INVALID_MIC_E_TELEMETRY);
  }

  if((telemLen != 0) && (telemLen != 2) && (telemLen != 5)) {
    return(RADIOLIB_ERR_INVALID_MIC_E_TELEMETRY_LENGTH);
  }

  if((telemLen > 0) && ((grid != NULL) || (status != NULL) || (alt != RADIOLIB_APRS_MIC_E_ALTITUDE_UNUSED))) {
    // can't have both telemetry and status
    return(RADIOLIB_ERR_MIC_E_TELEMETRY_STATUS);
  }

  // prepare buffers
  char destCallsign[7];
  #if !defined(RADIOLIB_STATIC_ONLY)
    size_t infoLen = 10;
    if(telemLen > 0) {
      infoLen += 1 + telemLen;
    } else {
      if(grid != NULL) {
        infoLen += strlen(grid) + 2;
      }
      if(status != NULL) {
        infoLen += strlen(status);
      }
      if(alt > RADIOLIB_APRS_MIC_E_ALTITUDE_UNUSED) {
        infoLen += 4;
      }
    }
    char* info = new char[infoLen];
  #else
    char info[RADIOLIB_STATIC_ARRAY_SIZE];
  #endif
  size_t infoPos = 0;

  // the following is based on APRS Mic-E implementation by https://github.com/omegat
  // as discussed in https://github.com/jgromes/RadioLib/issues/430

  // latitude first, because that is in the destination field
  float lat_abs = abs(lat);
  int lat_deg = (int)lat_abs;
  int lat_min = (lat_abs - (float)lat_deg) * 60.0f;
  int lat_hun = (((lat_abs - (float)lat_deg) * 60.0f) - lat_min) * 100.0f;
  destCallsign[0] = lat_deg/10;
  destCallsign[1] = lat_deg%10;
  destCallsign[2] = lat_min/10;
  destCallsign[3] = lat_min%10;
  destCallsign[4] = lat_hun/10;
  destCallsign[5] = lat_hun%10;

  // next, add the extra bits
  if(type & 0x04) { destCallsign[0] += RADIOLIB_APRS_MIC_E_DEST_BIT_OFFSET; }
  if(type & 0x02) { destCallsign[1] += RADIOLIB_APRS_MIC_E_DEST_BIT_OFFSET; }
  if(type & 0x01) { destCallsign[2] += RADIOLIB_APRS_MIC_E_DEST_BIT_OFFSET; }
  if(lat >= 0) { destCallsign[3] += RADIOLIB_APRS_MIC_E_DEST_BIT_OFFSET; }
  if(lon >= 100 || lon <= -100) { destCallsign[4] += RADIOLIB_APRS_MIC_E_DEST_BIT_OFFSET; }
  if(lon < 0) { destCallsign[5] += RADIOLIB_APRS_MIC_E_DEST_BIT_OFFSET; }
  destCallsign[6] = '\0';

  // now convert to Mic-E characters to get the "callsign"
  for(uint8_t i = 0; i < 6; i++) {
    if(destCallsign[i] <= 9) {
      destCallsign[i] += '0';
    } else {
      destCallsign[i] += ('A' - 10);
    }
  }

  // setup the information field
  info[infoPos++] = RADIOLIB_APRS_MIC_E_GPS_DATA_CURRENT;

  // encode the longtitude
  float lon_abs = abs(lon);
  int32_t lon_deg = (int32_t)lon_abs;
  int32_t lon_min = (lon_abs - (float)lon_deg) * 60.0f;
  int32_t lon_hun = (((lon_abs - (float)lon_deg) * 60.0f) - lon_min) * 100.0f;

  if(lon_deg <= 9) {
    info[infoPos++] = lon_deg + 118;
  } else if(lon_deg <= 99) {
    info[infoPos++] = lon_deg + 28;
  } else if(lon_deg <= 109) {
    info[infoPos++] = lon_deg + 8;
  } else {
    info[infoPos++] = lon_deg - 72;
  }

  if(lon_min <= 9){
    info[infoPos++] = lon_min + 88;
  } else {
    info[infoPos++] = lon_min + 28;
  }

  info[infoPos++] = lon_hun + 28;

  // now the speed and heading - this gets really weird
  int32_t speed_hun_ten = speed/10;
  int32_t speed_uni = speed%10;
  int32_t head_hun = heading/100;
  int32_t head_ten_uni = heading%100;

  if(speed <= 199) {
    info[infoPos++] = speed_hun_ten + 'l';
  } else {
    info[infoPos++] = speed_hun_ten + '0';
  }

  info[infoPos++] = speed_uni*10 + head_hun + 32;
  info[infoPos++] = head_ten_uni + 28;
  info[infoPos++] = _symbol;
  info[infoPos++] = _table;

  // onto the optional stuff - check telemetry first
  if(telemLen > 0) {
    if(telemLen == 2) {
      info[infoPos++] = RADIOLIB_APRS_MIC_E_TELEMETRY_LEN_2;
    } else {
      info[infoPos++] = RADIOLIB_APRS_MIC_E_TELEMETRY_LEN_5;
    }
    for(uint8_t i = 0; i < telemLen; i++) {
      sprintf(&(info[infoPos]), "%02X", telem[i]);
      infoPos += 2;
    }

  } else {
    if(grid != NULL) {
      memcpy(&(info[infoPos]), grid, strlen(grid));
      infoPos += strlen(grid);
      info[infoPos++] = '/';
      info[infoPos++] = 'G';
    }
    if(status != NULL) {
      info[infoPos++] = ' ';
      memcpy(&(info[infoPos]), status, strlen(status));
      infoPos += strlen(status);
    }
    if(alt > RADIOLIB_APRS_MIC_E_ALTITUDE_UNUSED) {
      // altitude is offset by -10 km
      int32_t alt_val = alt + 10000;

      // ... and encoded in base 91 for some reason
      info[infoPos++] = (alt_val / 8281) + 33;
      info[infoPos++] = ((alt_val % 8281) / 91) + 33;
      info[infoPos++] = ((alt_val % 8281) % 91) + 33;
      info[infoPos++] = '}';
    }
  }
  info[infoPos++] = '\0';

  // send the frame
  int16_t state = sendFrame(destCallsign, 0, info);
  #if !defined(RADIOLIB_STATIC_ONLY)
    delete[] info;
  #endif
  return(state);
}

int16_t APRSClient::sendFrame(char* destCallsign, uint8_t destSSID, char* info) {
  // get AX.25 callsign
  char srcCallsign[RADIOLIB_AX25_MAX_CALLSIGN_LEN + 1];
  _ax->getCallsign(srcCallsign);

  AX25Frame frameUI(destCallsign, destSSID, srcCallsign, _ax->getSSID(), RADIOLIB_AX25_CONTROL_U_UNNUMBERED_INFORMATION |
                    RADIOLIB_AX25_CONTROL_POLL_FINAL_DISABLED | RADIOLIB_AX25_CONTROL_UNNUMBERED_FRAME,
                    RADIOLIB_AX25_PID_NO_LAYER_3, (const char*)info);

  return(_ax->sendFrame(&frameUI));
}
