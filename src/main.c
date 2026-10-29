// CoCIS smart control: ONE ESP32, three logical "terminals" as FreeRTOS tasks.
// Entrance terminal : entrance_task (door + occupancy logic)
// Room 1 / Room 2   : room_task (PIR + light + idle timeout)
// Shared state is guarded by S_mtx; ONLY logger_task touches the SD card.
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdspi_host.h"
#include "driver/spi_common.h"

#define TAG "cocis"
static const int PIR_IN_PIN[2]  = {32, 34};
static const int PIR_OUT_PIN[2] = {33, 35};
static const int LIGHT_PIN[2] = {27, 14};
#define SENSOR_SEQUENCE_TIMEOUT_MS 10000
#define SD_CS_PIN 5
#define SD_MOSI_PIN 23
#define SD_MISO_PIN 19
#define SD_SCK_PIN 18

typedef struct {
    int inside;
    int room_entries[2];
    int room_exits[2];
    bool light[2];
} state_t;
typedef struct { bool in; char uid[16]; } tap_t;

static state_t S;
static SemaphoreHandle_t S_mtx;
static QueueHandle_t tap_q;   /* CLI -> entrance terminal ("RFID reader") */
static QueueHandle_t log_q;   /* everyone -> logger (single SD writer)   */

static esp_err_t sd_mount(sdmmc_card_t **card) {
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = SD_MOSI_PIN,
        .miso_io_num = SD_MISO_PIN,
        .sclk_io_num = SD_SCK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };
    esp_err_t err = spi_bus_initialize(SPI2_HOST, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        return err;
    }

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = SPI2_HOST;
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = SD_CS_PIN;
    slot_config.host_id = host.slot;
    esp_vfs_fat_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 4,
        .allocation_unit_size = 16 * 1024,
    };
    return esp_vfs_fat_sdspi_mount("/sdcard", &host, &slot_config,
                                   &mount_config, card);
}

static void logf_(const char *msg) {
    char line[96];
    strncpy(line, msg, sizeof line - 1); line[sizeof line - 1] = 0;
    xQueueSend(log_q, line, 0);
}

static void logger_task(void *a) {
    char line[96];
    sdmmc_card_t *card = NULL;
    FILE *log_file = NULL;
    esp_err_t err = sd_mount(&card);
    if (err == ESP_OK) {
        log_file = fopen("/sdcard/log.csv", "a+");
        if (log_file == NULL) {
            ESP_LOGE(TAG, "Cannot open /sdcard/log.csv for append");
        } else {
            fseek(log_file, 0, SEEK_END);
            if (ftell(log_file) == 0) {
                fprintf(log_file, "event\n");
                fflush(log_file);
            }
            ESP_LOGI(TAG, "SD card mounted; logging to /sdcard/log.csv");
        }
    } else {
        ESP_LOGE(TAG, "SD card mount failed: %s", esp_err_to_name(err));
    }
    for (;;) {
        if (xQueueReceive(log_q, line, portMAX_DELAY)) {
            ESP_LOGI(TAG, "LOG %s", line);
            if (log_file != NULL) {
                if (fprintf(log_file, "%s\n", line) < 0 || fflush(log_file) != 0) {
                    ESP_LOGE(TAG, "Failed writing /sdcard/log.csv");
                }
            }
        }
    }
}

static void entrance_task(void *a) {
    tap_t t; char m[96];
    for (;;) {
        if (xQueueReceive(tap_q, &t, portMAX_DELAY)) {
            /* TODO: check uid in user table, schedule window, anti-passback */
            xSemaphoreTake(S_mtx, portMAX_DELAY);
            if (t.in) S.inside++; else if (S.inside > 0) S.inside--;
            int n = S.inside;
            xSemaphoreGive(S_mtx);
            /* TODO: sweep door servo (LEDC) */
            snprintf(m, sizeof m, "%s,%s,inside=%d", t.in ? "IN" : "OUT", t.uid, n);
            logf_(m);
        }
    }
}

static void room_task(void *arg) {
    int r = (int)(intptr_t)arg;
    bool previous_in = false;
    bool previous_out = false;
    int sequence = 0;
    TickType_t sequence_started = 0;

    for (;;) {
        TickType_t now = xTaskGetTickCount();
        bool sensor_in = gpio_get_level(PIR_IN_PIN[r]);
        bool sensor_out = gpio_get_level(PIR_OUT_PIN[r]);
        bool rising_in = sensor_in && !previous_in;
        bool rising_out = sensor_out && !previous_out;
        bool entered = false;
        bool exited = false;

        if (sequence != 0 &&
            now - sequence_started > pdMS_TO_TICKS(SENSOR_SEQUENCE_TIMEOUT_MS)) {
            sequence = 0;
        }

        if (sequence == 0) {
            if (rising_in) {
                sequence = 1;
                sequence_started = now;
            } else if (rising_out) {
                sequence = 2;
                sequence_started = now;
            }
        } else if (sequence == 1 && rising_out) {
            entered = true;
            sequence = 0;
        } else if (sequence == 2 && rising_in) {
            exited = true;
            sequence = 0;
        }

        previous_in = sensor_in;
        previous_out = sensor_out;

        if (rising_in) {
            ESP_LOGI(TAG, "ROOM%d entry PIR detected", r + 1);
        }
        if (rising_out) {
            ESP_LOGI(TAG, "ROOM%d exit PIR detected", r + 1);
        }

        if (entered || exited) {
            xSemaphoreTake(S_mtx, portMAX_DELAY);
            if (entered) {
                S.room_entries[r]++;
            } else {
                S.room_exits[r]++;
            }
            bool want = S.room_entries[r] > S.room_exits[r];
            bool changed = (S.light[r] != want);
            S.light[r] = want;
            int occupancy = S.room_entries[r] - S.room_exits[r];
            xSemaphoreGive(S_mtx);

            char m[64];
            snprintf(m, sizeof m, "ROOM%d,%s,occupancy=%d", r + 1,
                     entered ? "IN" : "OUT", occupancy);
            logf_(m);
            if (changed) {
                gpio_set_level(LIGHT_PIN[r], want);
                snprintf(m, sizeof m, "ROOM%d,light=%d", r + 1, want);
                logf_(m);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

static void cli_task(void *a) {
    char buf[64]; int n = 0; uint8_t c;
    printf("\ncommands: tap in <uid> | tap out <uid> | status\n> ");
    for (;;) {
        if (uart_read_bytes(UART_NUM_0, &c, 1, portMAX_DELAY) != 1) continue;
        if (c != '\n' && c != '\r') { if (n < 62) buf[n++] = c; continue; }
        buf[n] = 0; n = 0;
        char d[8], u[16];
        if (sscanf(buf, "tap %7s %15s", d, u) == 2) {
            tap_t t = { .in = strcmp(d, "in") == 0 };
            strncpy(t.uid, u, sizeof t.uid - 1); t.uid[sizeof t.uid - 1] = 0;
            xQueueSend(tap_q, &t, 0);
        } else if (strcmp(buf, "status") == 0) {
            xSemaphoreTake(S_mtx, portMAX_DELAY);
             printf("inside=%d room1=in:%d out:%d(light %d) room2=in:%d out:%d(light %d)\n",
                 S.inside, S.room_entries[0], S.room_exits[0], S.light[0],
                 S.room_entries[1], S.room_exits[1], S.light[1]);
            xSemaphoreGive(S_mtx);
        }
        printf("> ");
    }
}

void app_main(void) {
    uart_driver_install(UART_NUM_0, 256, 0, 0, NULL, 0);
    S_mtx = xSemaphoreCreateMutex();
    tap_q = xQueueCreate(8, sizeof(tap_t));
    log_q = xQueueCreate(16, 96);
    for (int i = 0; i < 2; i++) {
        gpio_set_direction(PIR_IN_PIN[i], GPIO_MODE_INPUT);
        gpio_set_direction(PIR_OUT_PIN[i], GPIO_MODE_INPUT);
        gpio_set_direction(LIGHT_PIN[i], GPIO_MODE_OUTPUT);
        gpio_set_level(LIGHT_PIN[i], 0);
    }
    xTaskCreate(logger_task,   "logger",   4096, NULL, 4, NULL);
    xTaskCreate(entrance_task, "entrance", 4096, NULL, 5, NULL);
    xTaskCreate(room_task,     "room1",    3072, (void *)0, 5, NULL);
    xTaskCreate(room_task,     "room2",    3072, (void *)1, 5, NULL);
    xTaskCreate(cli_task,      "cli",      4096, NULL, 3, NULL);
}
