#include "contiki.h"
#include "dev/leds.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include <string.h>
#include <stdio.h>

#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

static linkaddr_t dest_addr = {{ 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }};

#if MAC_CONF_WITH_TSCH
#include "net/mac/tsch/tsch.h"
static linkaddr_t coordinator_addr = {{ 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }};
#endif

// Callbacks prototype declaration
static void green_timer_callback(void *data);
static void red_timer_callback(void *data);

PROCESS(unicast, "Pregunta 2: Comunicacion unicast");
AUTOSTART_PROCESSES(&unicast);

// Callback function for the green timer
static void green_timer_callback(void *data) {
    leds_off(LEDS_GREEN);
}

// Callback function for the red timer
static void red_timer_callback(void *data) {
    leds_off(LEDS_RED);
}

void input_callback(const void *data, uint16_t len, const linkaddr_t *src, const linkaddr_t *dest) {
    if (len == sizeof(unsigned)) {
        unsigned count;
        memcpy(&count, data, sizeof(count));
        leds_on(LEDS_GREEN);

        char node_name[20];
        snprintf(node_name, sizeof(node_name), "Node %d", linkaddr_node_addr.u8[0]);
        LOG_INFO("Hola recibi %u de ", count);
        LOG_INFO_LLADDR(src);
        if (linkaddr_cmp(dest, &linkaddr_node_addr)) {
            LOG_INFO_(" Mi nombre es %s | ademas, yo soy el receptor  \n", node_name);
        } else {
            LOG_INFO_(" | no recibe Node\n");
        }

        static struct ctimer green_timer;
        ctimer_set(&green_timer, CLOCK_SECOND, green_timer_callback, NULL);
    }
}

PROCESS_THREAD(unicast, ev, data) {
    static struct ctimer red_timer;
    static struct etimer periodic_timer;
    static unsigned count = 0;
    static unsigned value = 2;

    PROCESS_BEGIN();

#if MAC_CONF_WITH_TSCH
    tsch_set_coordinator(linkaddr_cmp(&coordinator_addr, &linkaddr_node_addr));
#endif

    nullnet_buf = (uint8_t *)&count;
    nullnet_len = sizeof(count);
    nullnet_set_input_callback(input_callback);

    if (!linkaddr_cmp(&dest_addr, &linkaddr_node_addr)) {
        etimer_set(&periodic_timer, 2 * CLOCK_SECOND);

        while (1) {
            PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
            leds_on(LEDS_RED);
            LOG_INFO("Enviando %u hacia ", count);
            LOG_INFO_LLADDR(&dest_addr);
            LOG_INFO_("\n");

            ctimer_set(&red_timer, CLOCK_SECOND, red_timer_callback, NULL);
            NETSTACK_NETWORK.output(&dest_addr);

            count++;
            if (value != 20) {
                value += 2;
            } else {
                value = 2;
            }
            etimer_set(&periodic_timer, value * CLOCK_SECOND);
        }
    }

    PROCESS_END();
}
