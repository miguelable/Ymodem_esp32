/* YModem Example

   Copyright (C) LoBo 2017

   Author: Boris Lovosevic (loboris@gmail.com)

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <time.h>

#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "esp_system.h"
#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "soc/uart_struct.h"
#include "ymodem.h"

// === Maximal file size we can receive in bytes ===
#define MAX_FILE_SIZE (CONFIG_SPIFFS_SIZE - 0x2000)
#define CONFIG_EXAMPLE_BAUDRATE 115200

static const char *TAG = "[Ymodem example]";

#ifdef CONFIG_EXAMPLE_USE_WIFI

#include "apps/sntp/sntp.h"
#include "esp_attr.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/event_groups.h"
#include "lwip/err.h"
#include <sys/time.h>
#include <unistd.h>

static struct tm *tm_info;
static time_t time_now;

/* FreeRTOS event group to signal when we are connected & ready to make a
 * request */
static EventGroupHandle_t wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
const int CONNECTED_BIT = 0x00000001;

//------------------------------------------------------------
static esp_err_t event_handler(void *ctx, system_event_t *event) {
  switch (event->event_id) {
  case SYSTEM_EVENT_STA_START:
    esp_wifi_connect();
    break;
  case SYSTEM_EVENT_STA_GOT_IP:
    xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    /* This is a workaround as ESP32 WiFi libs don't currently
       auto-reassociate. */
    esp_wifi_connect();
    xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
    break;
  default:
    break;
  }
  return ESP_OK;
}

//-------------------------------
static void initialise_wifi(void) {
  tcpip_adapter_init();
  wifi_event_group = xEventGroupCreate();
  ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
  wifi_config_t wifi_config = {
      .sta =
          {
              .ssid = CONFIG_WIFI_SSID,
              .password = CONFIG_WIFI_PASSWORD,
          },
  };
  ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());
}

//-------------------------------
static void initialize_sntp(void) {
  ESP_LOGI(TAG, "Initializing SNTP");
  sntp_setoperatingmode(SNTP_OPMODE_POLL);
  sntp_setservername(0, "pool.ntp.org");
  sntp_init();
}

//--------------------------
static int obtain_time(void) {
  int res = 1;
  initialise_wifi();
  xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true,
                      portMAX_DELAY);

  initialize_sntp();

  // wait for time to be set
  int retry = 0;
  const int retry_count = 20;

  time(&time_now);
  tm_info = localtime(&time_now);

  while (tm_info->tm_year < (2016 - 1900) && ++retry < retry_count) {
    // ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry,
    // retry_count);
    vTaskDelay(500 / portTICK_RATE_MS);
    time(&time_now);
    tm_info = localtime(&time_now);
  }
  if (tm_info->tm_year < (2016 - 1900)) {
    ESP_LOGI(TAG, "System time NOT set.");
    res = 0;
  } else {
    ESP_LOGI(TAG, "System time is set.");
  }

  ESP_ERROR_CHECK(esp_wifi_stop());
  return res;
}

#endif

static QueueHandle_t uart0_queue;

//---------------------------------------------
static void uart_event_task(void *pvParameters) {
  uart_event_t event;
  for (;;) {
    // Waiting for UART event.
    if (xQueueReceive(uart0_queue, (void *)&event,
                      (portTickType)portMAX_DELAY)) {
      // ESP_LOGI(TAG, "uart[%d] event:", EX_UART_NUM);
      switch (event.type) {
      // Event of UART receving data
      case UART_DATA:
        break;
      // Event of HW FIFO overflow detected
      case UART_FIFO_OVF:
        ESP_LOGI(TAG, "hw fifo overflow\n");
        uart_flush(EX_UART_NUM);
        break;
      // Event of UART ring buffer full
      case UART_BUFFER_FULL:
        ESP_LOGI(TAG, "ring buffer full\n");
        uart_flush(EX_UART_NUM);
        break;
      // Event of UART RX break detected
      case UART_BREAK:
        ESP_LOGI(TAG, "uart rx break\n");
        break;
      // Event of UART parity check error
      case UART_PARITY_ERR:
        ESP_LOGI(TAG, "uart parity error\n");
        break;
      // Event of UART frame error
      case UART_FRAME_ERR:
        ESP_LOGI(TAG, "uart frame error\n");
        break;
      // UART_PATTERN_DET
      case UART_PATTERN_DET:
        ESP_LOGI(TAG, "uart pattern detected\n");
        break;
      // Others
      default:
        ESP_LOGI(TAG, "uart event type: %d\n", event.type);
        break;
      }
    }
  }
  vTaskDelete(NULL);
}

//----------------------------------
static int check_file(char *fname) {
  if (!spiffs_is_mounted)
    return -1;

  struct stat sb;
  if (stat(fname, &sb) == 0)
    return 1;
  return 0;
}

// Check free space on file system
//-----------------------
static uint32_t fs_free() {
  uint32_t tot, used;
  spiffs_fs_stat(&tot, &used);
  return (tot - used - 16384);
}

// ============================================================================
#include <ctype.h>

// fnmatch defines
#define FNM_NOMATCH 1 // Match failed.
#define FNM_NOESCAPE 0x01 // Disable backslash escaping.
#define FNM_PATHNAME 0x02 // Slash must be matched by slash.
#define FNM_PERIOD 0x04 // Period must be matched by period.
#define FNM_LEADING_DIR 0x08 // Ignore /<tail> after Imatch.
#define FNM_CASEFOLD 0x10 // Case insensitive search.
#define FNM_PREFIX_DIRS 0x20 // Directory prefixes of pattern match too.
#define EOS '\0'

//-----------------------------------------------------------------------
static const char *rangematch(const char *pattern, char test, int flags) {
  int negate, ok;
  char c, c2;

  /*
   * A bracket expression starting with an unquoted circumflex
   * character produces unspecified results (IEEE 1003.2-1992,
   * 3.13.2).  This implementation treats it like '!', for
   * consistency with the regular expression syntax.
   * J.T. Conklin (conklin@ngai.kaleida.com)
   */
  if ((negate = (*pattern == '!' || *pattern == '^')))
    ++pattern;

  if (flags & FNM_CASEFOLD)
    test = tolower((unsigned char)test);

  for (ok = 0; (c = *pattern++) != ']';) {
    if (c == '\\' && !(flags & FNM_NOESCAPE))
      c = *pattern++;
    if (c == EOS)
      return (NULL);

    if (flags & FNM_CASEFOLD)
      c = tolower((unsigned char)c);

    if (*pattern == '-' && (c2 = *(pattern + 1)) != EOS && c2 != ']') {
      pattern += 2;
      if (c2 == '\\' && !(flags & FNM_NOESCAPE))
        c2 = *pattern++;
      if (c2 == EOS)
        return (NULL);

      if (flags & FNM_CASEFOLD)
        c2 = tolower((unsigned char)c2);

      if ((unsigned char)c <= (unsigned char)test &&
          (unsigned char)test <= (unsigned char)c2)
        ok = 1;
    } else if (c == test)
      ok = 1;
  }
  return (ok == negate ? NULL : pattern);
}

//--------------------------------------------------------------------
static int fnmatch(const char *pattern, const char *string, int flags) {
  const char *stringstart;
  char c, test;

  for (stringstart = string;;)
    switch (c = *pattern++) {
    case EOS:
      if ((flags & FNM_LEADING_DIR) && *string == '/')
        return (0);
      return (*string == EOS ? 0 : FNM_NOMATCH);
    case '?':
      if (*string == EOS)
        return (FNM_NOMATCH);
      if (*string == '/' && (flags & FNM_PATHNAME))
        return (FNM_NOMATCH);
      if (*string == '.' && (flags & FNM_PERIOD) &&
          (string == stringstart ||
           ((flags & FNM_PATHNAME) && *(string - 1) == '/')))
        return (FNM_NOMATCH);
      ++string;
      break;
    case '*':
      c = *pattern;
      // Collapse multiple stars.
      while (c == '*')
        c = *++pattern;

      if (*string == '.' && (flags & FNM_PERIOD) &&
          (string == stringstart ||
           ((flags & FNM_PATHNAME) && *(string - 1) == '/')))
        return (FNM_NOMATCH);

      // Optimize for pattern with * at end or before /.
      if (c == EOS)
        if (flags & FNM_PATHNAME)
          return ((flags & FNM_LEADING_DIR) || strchr(string, '/') == NULL
                      ? 0
                      : FNM_NOMATCH);
        else
          return (0);
      else if ((c == '/') && (flags & FNM_PATHNAME)) {
        if ((string = strchr(string, '/')) == NULL)
          return (FNM_NOMATCH);
        break;
      }

      // General case, use recursion.
      while ((test = *string) != EOS) {
        if (!fnmatch(pattern, string, flags & ~FNM_PERIOD))
          return (0);
        if ((test == '/') && (flags & FNM_PATHNAME))
          break;
        ++string;
      }
      return (FNM_NOMATCH);
    case '[':
      if (*string == EOS)
        return (FNM_NOMATCH);
      if ((*string == '/') && (flags & FNM_PATHNAME))
        return (FNM_NOMATCH);
      if ((pattern = rangematch(pattern, *string, flags)) == NULL)
        return (FNM_NOMATCH);
      ++string;
      break;
    case '\\':
      if (!(flags & FNM_NOESCAPE)) {
        if ((c = *pattern++) == EOS) {
          c = '\\';
          --pattern;
        }
      }
      break;
      // FALLTHROUGH
    default:
      if (c == *string) {
      } else if ((flags & FNM_CASEFOLD) && (tolower((unsigned char)c) ==
                                            tolower((unsigned char)*string))) {
      } else if ((flags & FNM_PREFIX_DIRS) && *string == EOS &&
                 ((c == '/' && string != stringstart) ||
                  (string == stringstart + 1 && *stringstart == '/')))
        return (0);
      else
        return (FNM_NOMATCH);
      string++;
      break;
    }
  // NOTREACHED
  return 0;
}

// ============================================================================
//-----------------------------------------
static void list(char *path, char *match) {

  DIR *dir = NULL;
  struct dirent *ent;
  char type;
  char size[9];
  char tpath[255];
  char tbuffer[80];
  struct stat sb;
  struct tm *tm_info;
  char *lpath = NULL;
  int statok;

  printf("LIST of DIR [%s]\r\n", path);
  // Open directory
  dir = opendir(path);
  if (!dir) {
    printf("Error opening directory\r\n");
    return;
  }

  // Read directory entries
  uint64_t total = 0;
  int nfiles = 0;
  printf("T  Size      Date/Time         Name\r\n");
  printf("-----------------------------------\r\n");
  while ((ent = readdir(dir)) != NULL) {
    sprintf(tpath, path);
    if (path[strlen(path) - 1] != '/')
      strcat(tpath, "/");
    strcat(tpath, ent->d_name);
    tbuffer[0] = '\0';

    if ((match == NULL) || (fnmatch(match, tpath, (FNM_PERIOD)) == 0)) {
      // Get file stat
      statok = stat(tpath, &sb);

      if (statok == 0) {
        tm_info = localtime(&sb.st_mtime);
        strftime(tbuffer, 80, "%d/%m/%Y %R", tm_info);
      } else
        sprintf(tbuffer, "                ");

      if (ent->d_type == DT_REG) {
        type = 'f';
        nfiles++;
        if (statok)
          strcpy(size, "       ?");
        else {
          total += sb.st_size;
          if (sb.st_size < (1024 * 1024))
            sprintf(size, "%8d", (int)sb.st_size);
          else if ((sb.st_size / 1024) < (1024 * 1024))
            sprintf(size, "%6dKB", (int)(sb.st_size / 1024));
          else
            sprintf(size, "%6dMB", (int)(sb.st_size / (1024 * 1024)));
        }
      } else {
        type = 'd';
        strcpy(size, "       -");
      }

      printf("%c  %s  %s  %s\r\n", type, size, tbuffer, ent->d_name);
    }
  }
  if (total) {
    printf("-----------------------------------\r\n");
    if (total < (1024 * 1024))
      printf("   %8d", (int)total);
    else if ((total / 1024) < (1024 * 1024))
      printf("   %6dKB", (int)(total / 1024));
    else
      printf("   %6dMB", (int)(total / (1024 * 1024)));
    printf(" in %d file(s)\r\n", nfiles);
  }
  printf("-----------------------------------\r\n");

  closedir(dir);

  free(lpath);

  uint32_t tot, used;
  spiffs_fs_stat(&tot, &used);
  printf("SPIFFS: free %d KB of %d KB\r\n", (tot - used) / 1024, tot / 1024);
}

//=============
void app_main() {
#if YMODEM_LED_ACT
  gpio_set_direction(YMODEM_LED_ACT, GPIO_MODE_OUTPUT);
  gpio_set_level(YMODEM_LED_ACT, YMODEM_LED_ACT_ON ^ 1);
#endif

  uart_config_t uart_config = {
      .baud_rate = CONFIG_EXAMPLE_BAUDRATE,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .rx_flow_ctrl_thresh = 122,
  };
  // Set UART parameters
  uart_param_config(EX_UART_NUM, &uart_config);
  // Set UART log level
  esp_log_level_set(TAG, ESP_LOG_VERBOSE);
  // Install UART driver, and get the queue.
  uart_driver_install(EX_UART_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 10, &uart0_queue,
                      0);

  // Set UART pins (using UART0 default pins, ie no changes.)
  uart_set_pin(EX_UART_NUM, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE,
               UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

  // Set uart pattern detect function.
  // uart_enable_pattern_det_intr(EX_UART_NUM, '+', 3, 10000, 10, 10);

  // Create a task to handler UART event from ISR
  xTaskCreate(uart_event_task, "uart_event_task", 1024, NULL, 12, NULL);
  ESP_LOGW(TAG, "UART task created, baudrate=%d.", uart_config.baud_rate);

#ifdef CONFIG_EXAMPLE_USE_WIFI

  ESP_ERROR_CHECK(nvs_flash_init());

  // ===== Set time zone ======
  setenv("TZ", "CET-1CEST", 0);
  tzset();
  // ==========================

  time(&time_now);
  tm_info = localtime(&time_now);

  // Is time set? If not, tm_year will be (1970 - 1900).
  if (tm_info->tm_year < (2016 - 1900)) {
    ESP_LOGI(
        TAG,
        "Time is not set yet. Connecting to WiFi and getting time over NTP.");
    if (obtain_time()) {
    } else {
    }
    time(&time_now);
  }
#endif

  char fname[128] = {'\0'};

  // ==== Initialize the file system ====
  printf("\r\n\n");
  vfs_spiffs_register();
  if (spiffs_is_mounted) {
    ESP_LOGI(TAG, "File system mounted.");
    // Remove old received files
    for (int i = 1; i < 10; i++) {
      sprintf(fname, "%s/yfile-%d.bin", SPIFFS_BASE_PATH, i);
      if (check_file(fname) == 1) {
        remove(fname);
        printf("Removed \"%s\"\r\n", fname);
      }
    }
    list(SPIFFS_BASE_PATH "/", NULL);
    printf("\r\n\n");
  } else {
    ESP_LOGE(TAG, "Error mounting file system, HALTED");
    while (1) {
      vTaskDelay(1000 / portTICK_RATE_MS);
    }
  }

  // === YMODEM TEST ===

  FILE *ffd = NULL;
  int rec_res = -1, trans_res = -1;
  char orig_name[256] = {'\0'};
  char send_name[128] = {'\0'};
  int nfile = 1;
  uint32_t max_fsize;
  while (1) {
    // ==== Receive file ====
    max_fsize = fs_free();

    if (max_fsize > 16 * 1024) {
      if (max_fsize > MAX_FILE_SIZE)
        max_fsize = MAX_FILE_SIZE;

      sprintf(fname, "%s/yfile-%d.bin", SPIFFS_BASE_PATH, nfile);
      // Open the file
      ffd = fopen(fname, "wb");
      if (ffd) {
        printf(
            "\r\nReceiving file, please start YModem transfer on host ...\r\n");
        rec_res = Ymodem_Receive(ffd, max_fsize, orig_name);
        fclose(ffd);
        printf("\r\n");
        if (rec_res > 0) {
          ESP_LOGI(TAG, "Transfer complete, Size=%d, orig name: \"%s\"",
                   rec_res, fname);
          list(SPIFFS_BASE_PATH "/", SPIFFS_BASE_PATH "/yfile-*.bin");
        } else {
          ESP_LOGE(TAG, "Transfer complete, Error=%d", rec_res);
          remove(fname);
        }
      } else {
        ESP_LOGE(TAG, "Error opening file \"%s\" for receive.", fname);
      }

      vTaskDelay(5000 / portTICK_RATE_MS);

      if (rec_res > 0) {
        // ==== Send file back ====
        sprintf(send_name, "yfile-%d.bin", nfile);
        // Open the file
        ffd = fopen(fname, "rb");
        if (ffd) {
          printf("\r\nSending file \"%s\", please start YModem receive on host "
                 "...\r\n",
                 fname);
          trans_res = Ymodem_Transmit(send_name, rec_res, ffd);
          fclose(ffd);
          printf("\r\n");
          if (trans_res == 0) {
            ESP_LOGI(TAG, "Transfer complete.");
          } else
            ESP_LOGE(TAG, "Transfer complete, Error=%d", trans_res);
        } else {
          ESP_LOGE(TAG, "Error opening file \"%s\" for sending.", fname);
        }
      }

      nfile++;
    } else
      ESP_LOGE(TAG, "File system full, %u left", max_fsize);

    vTaskDelay(30000 / portTICK_RATE_MS);
  }
}
