#include "uart_config.h"
#include "gps.h"

#include "FreeRTOS.h"
#include "task.h"

uint8_t rx_buffer_1[160];
uint8_t rx_buffer_2[160];
bool pointerBuffer = true;
PIO pio;
uint sm;
queue_t fifo;

static void async_worker_func(async_context_t *async_context, async_when_pending_worker_t *worker);
static void pio_irq_func(void);
static async_context_freertos_t async_context_instance;
static async_when_pending_worker_t worker = {.do_work = async_worker_func};

// Create an async context
static async_context_t *get_async_context(void)
{
    async_context_freertos_config_t config = async_context_freertos_default_config();
    config.task_priority = UART_TASK_PRIORITY;     // defaults to ASYNC_CONTEXT_DEFAULT_FREERTOS_TASK_PRIORITY
    config.task_stack_size = UART_TASK_STACK_SIZE; // defaults to ASYNC_CONTEXT_DEFAULT_FREERTOS_TASK_STACK_SIZE
    if (!async_context_freertos_init(&async_context_instance, &config))
        return NULL;
    return &async_context_instance.core;
}

static bool init_pio(const pio_program_t *program, PIO *pio_hw, uint *sm, uint *offset)
{
    // Find a free pio
    *pio_hw = pio1;
    if (!pio_can_add_program(*pio_hw, program))
    {
        *pio_hw = pio0;
        if (!pio_can_add_program(*pio_hw, program))
        {
            *offset = -1;
            return false;
        }
    }
    *offset = pio_add_program(*pio_hw, program);
    // Find a state machine
    *sm = (int8_t)pio_claim_unused_sm(*pio_hw, false);
    if (*sm < 0)
    {
        return false;
    }
    return true;
}

void uart_configure()
{
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    queue_init(&fifo, 1, FIFO_SIZE);

    async_context_t *context = get_async_context();
    uint offset;
    async_context_add_when_pending_worker(context, &worker);
    if (!init_pio(&uart_rx_program, &pio, &sm, &offset))
    {
        //  printf("failed to setup pio\n");
    }
    uart_rx_program_init(pio, sm, offset, UART_RX_PIN, BAUD_RATE);

    static_assert(PIO0_IRQ_1 == PIO0_IRQ_0 + 1 && PIO1_IRQ_1 == PIO1_IRQ_0 + 1, "");
    int8_t pio_irq;
    pio_irq = (pio == pio0) ? PIO0_IRQ_0 : PIO1_IRQ_0;
    if (irq_get_exclusive_handler(pio_irq))
    {
        pio_irq++;
        if (irq_get_exclusive_handler(pio_irq))
        {
            //    printf("All IRQs are in use\n");
        }
    }
    irq_add_shared_handler(pio_irq, pio_irq_func, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY); // Add a shared IRQ handler
    irq_set_enabled(pio_irq, true);                                                                // Enable the IRQ
    const uint irq_index = pio_irq - ((pio == pio0) ? PIO0_IRQ_0 : PIO1_IRQ_0);                    // Get index of the IRQ
    pio_set_irqn_source_enabled(pio, irq_index, pis_sm0_rx_fifo_not_empty, true);                  // Set pio to tell us when the FIFO is NOT empty
}

void pio_irq_func(void)
{
    static uint8_t counter = 0;

    while (!pio_sm_is_rx_fifo_empty(pio, sm))
    {
        char c = uart_rx_program_getc(pio, sm);

        if (pointerBuffer)
            rx_buffer_1[counter] = c;
        else
            rx_buffer_2[counter] = c;
        counter++;

        if (c == '\n')
        {
            if (pointerBuffer)
                rx_buffer_1[counter] = '\0';
            else
                rx_buffer_2[counter] = '\0';

            counter = 0;
            pointerBuffer = !pointerBuffer;
            async_context_set_work_pending(&async_context_instance.core, &worker);
        }
    }
}

static void async_worker_func(async_context_t *async_context, async_when_pending_worker_t *worker)
{
    gpio_put(14, 1);
    if (pointerBuffer)
        nmea_parcer(rx_buffer_1);
    else
        nmea_parcer(rx_buffer_2);
    gpio_put(14, 0);
}
