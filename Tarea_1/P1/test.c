#include "contiki.h"
#include "dev/button-hal.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include <string.h>
#include <stdio.h>

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

/* Configuracion */
#define SEND_INTERVAL (8 * CLOCK_SECOND)

#if MAC_CONF_WITH_TSCH
#include "net/mac/tsch/tsch.h"
static linkaddr_t coordinator_addr =  {{ 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }};
#endif

#if MAC_CONF_WITH_TSCH
    tsch_set_coordinator(linkaddr_cmp(&coordinator_addr, &linkaddr_node_addr));
#endif /* MAC_CONF_WITH_TSCH */


/*---------------------------------------------------------------------------*/
PROCESS(button_hal_p1, "Button hal p1");
AUTOSTART_PROCESSES(&button_hal_p1);
/*---------------------------------------------------------------------------*/




/* Nullnet */
void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{
    if(len == sizeof(unsigned)) {
        unsigned count;
        memcpy(&count, data, sizeof(count));
        LOG_INFO("Received %u from ", count);
        LOG_INFO_LLADDR(src);
        LOG_INFO_("\n");
    }

}

/*---------------------------------------------------------------------------*/



PROCESS_THREAD(button_hal_p1, ev, data)
{
    /* Boton */
    button_hal_button_t *btn;
    
    /* Nullnet */
    static unsigned count = 0;

    PROCESS_BEGIN();

    /* Indexamos */
    btn = button_hal_get_by_index(0);

    printf("Pregunta 1.\n");
    printf("Recuento de boton: %u.\n", button_hal_button_count);

    /* Especificamos el GPIO pin */
    if(btn) {
        printf("%s on pin %u with ID=0, Logic=%s, Pull=%s\n",
        
            /* Descripcion del boton */
            BUTTON_HAL_GET_DESCRIPTION(btn), btn->pin,
            btn->negative_logic ? "Negative" : "Positive",
            btn->pull == GPIO_HAL_PIN_CFG_PULL_UP ? "Pull Up" : "Pull Down");
    }


     /* Inicializamos NullNet */
     
    nullnet_buf = (uint8_t *)&count; /* Point NullNet buffer to 'payload' */
    nullnet_len = sizeof(count); /* The payload is "count" bytes */
    nullnet_set_input_callback(input_callback);

    /* Finalizamos Nullnet */
    
    
    while(1) {
    
        /* Espera hasta que se presione el boton */
        PROCESS_WAIT_EVENT_UNTIL(ev == button_hal_press_event);
        btn = (button_hal_button_t *)data;
        printf("Boton presionado, Enviando Broadcast.....(%s)\n", BUTTON_HAL_GET_DESCRIPTION(btn));

        /* Broadcast */
        LOG_INFO("Enviando %u a ", count);
        LOG_INFO_LLADDR(NULL);
        LOG_INFO_("\n");
        memcpy(nullnet_buf, &count, sizeof(count));
        nullnet_len = sizeof(count);
        NETSTACK_NETWORK.output(NULL);
        count++;


        if(btn == button_hal_get_by_id(BUTTON_HAL_ID_BUTTON_ZERO)) {
            printf("Boton 0 en pin %u\n", btn->pin);
        }

    }

    PROCESS_END();
}