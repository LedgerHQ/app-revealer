/*******************************************************************************
*   Ledger Blue
*   (c) 2016 Ledger
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
********************************************************************************/

#include "os.h"
#include "cx.h"

#include "os_io_seproxyhal.h"
#include "ui.h"
#include "error_codes.h"

unsigned char G_io_seproxyhal_spi_buffer[IO_SEPROXYHAL_BUFFER_SIZE_B];

static unsigned int current_text_pos; // parsing cursor in the text to display
static unsigned int text_y;           // current location of the displayed text

// UI currently displayed
extern enum UI_STATE { UI_IDLE, UI_TEXT, UI_APPROVAL };

extern enum UI_STATE uiState;

#ifdef TARGET_NANOX
#include "ux.h"
#include "bolos_ux_nanox.h"
#else
#include "bolos_ux_nanos.h"
#endif

ux_state_t G_ux;
bolos_ux_params_t G_ux_params;

// bolos_ux_context_t G_bolos_ux_context;

static unsigned char display_text_part(void);

#define MAX_CHARS_PER_LINE 49

static char lineBuffer[50];

unsigned short io_exchange_al(unsigned char channel, unsigned short tx_len) {
    switch (channel & ~(IO_FLAGS)) {
    case CHANNEL_KEYBOARD:
        break;

    // multiplexed io exchange over a SPI channel and TLV encapsulated protocol
    case CHANNEL_SPI:
        if (tx_len) {
            io_seproxyhal_spi_send(G_io_apdu_buffer, tx_len);

            if (channel & IO_RESET_AFTER_REPLIED) {
                reset();
            }
            return 0; // nothing received from the master so far (it's a tx
                      // transaction)
        } else {
            return io_seproxyhal_spi_recv(G_io_apdu_buffer,
                                          sizeof(G_io_apdu_buffer), 0);
        }

    default:
        THROW(INVALID_PARAMETER);
    }
    return 0;
}

uint8_t row_nb;
static void sample_main(void) {

    volatile unsigned int rx = 0;
    volatile unsigned int tx = 0;
    volatile uint8_t flags = 0;
    revealer_struct_init();
    for (;;) {
        volatile unsigned short sw = 0;

        BEGIN_TRY {
          TRY {
            rx = tx;
            tx = 0; // ensure no race in catch_other if io_exchange throws an error
            rx = io_exchange(CHANNEL_APDU|flags, rx);
            flags = 0;
            // no apdu received, well, reset the session, and reset the bootloader configuration
            if (rx == 0) {
              for(;;);
              THROW(0x6982);
            }

            if (G_io_apdu_buffer[0] != 0x80) {
              THROW(0x6E00);
            }

            switch (G_io_apdu_buffer[1]) {
                /*case 0xCA: // get hash
                    for (int i=0; i<4; i++){
                        G_io_apdu_buffer[i] = hashDBG[i];
                    }
                    tx += 4;
                    THROW(SW_OK);
                    break;*/
                case 0xAA: //get words debug
                    PRINTF("%.*H",sizeof(G_bolos_ux_context.string_buffer), G_bolos_ux_context.string_buffer);
                    PRINTF("%s", G_bolos_ux_context.string_buffer);
                    THROW(SW_OK);
                    break;
                case 0xCB: // Send img row chunk
                    #ifndef WORDS_IMG_DBG
                    if ((G_bolos_ux_context.words_seed_valid)&&(G_bolos_ux_context.noise_seed_valid)){
                    #else
                    if (1){
                    #endif
                        row_nb = G_io_apdu_buffer[3];
                        if (row_nb >= IMG_WIDTH){
                            THROW(ROW_OUT_OF_RANGE);
                        }
                        else {
                            tx += send_row(row_nb);
                            THROW(SW_OK);                            
                        }
                    }
                    else {
                        THROW(REVEALER_UNSET);
                    }
                    break;
                default:
                THROW(0x6D00);
                break;
            }
          }
          CATCH_OTHER(e) {
            PRINTF("%d", e);
            switch(e & 0xF000) {
              case 0x6000:
                sw = e;
                break;
              case SW_OK:
                sw = e;
                break;
              default:
                sw = 0x6800 | (e&0x7FF);
                break;
            }
            // Unexpected exception => report 
            G_io_apdu_buffer[tx] = sw>>8;
            G_io_apdu_buffer[tx+1] = sw;
            tx += 2;
            // test to avoid INVALID_STATE exception when USB is not yet powered
            // if (G_io_apdu_media != IO_APDU_MEDIA_NONE){
                io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, tx);
            // }
            flags |= IO_ASYNCH_REPLY;
          }
          FINALLY {
          }
        }
        END_TRY;
      }

return_to_dashboard:
    return;
}


// Pick the text elements to display
static unsigned char display_text_part() {
    unsigned int i;
    WIDE char *text = (char*) G_io_apdu_buffer + 5;
    if (text[current_text_pos] == '\0') {
        return 0;
    }
    i = 0;
    while ((text[current_text_pos] != 0) && (text[current_text_pos] != '\n') &&
           (i < MAX_CHARS_PER_LINE)) {
        lineBuffer[i++] = text[current_text_pos];
        current_text_pos++;
    }
    if (text[current_text_pos] == '\n') {
        current_text_pos++;
    }
    lineBuffer[i] = '\0';

    return 1;
}


unsigned char io_event(unsigned char channel) {
    // nothing done with the event, throw an error on the transport layer if
    // needed

    // can't have more than one tag in the reply, not supported yet.
    switch (G_io_seproxyhal_spi_buffer[0]) {
    case SEPROXYHAL_TAG_FINGER_EVENT:
        UX_FINGER_EVENT(G_io_seproxyhal_spi_buffer);
        break;

    case SEPROXYHAL_TAG_BUTTON_PUSH_EVENT: // for Nano S
        UX_BUTTON_PUSH_EVENT(G_io_seproxyhal_spi_buffer);
        break;

    case SEPROXYHAL_TAG_DISPLAY_PROCESSED_EVENT:
#if defined(TARGET_NANOS)
        if ((uiState == UI_TEXT) &&
            (os_seph_features() & SEPROXYHAL_TAG_SESSION_START_EVENT_FEATURE_SCREEN_BIG)) {
                UX_REDISPLAY();
            }
        else {
            if(G_bolos_ux_context.processing == 1)
            {
                // UX_DISPLAYED_EVENT(check_and_write_words_Cb(););
                // UX_DISPLAYED_EVENT(write_words(););
                UX_DISPLAYED_EVENT(compare_recovery_phrase(););
            }
            else if (G_bolos_ux_context.processing == 2){
                UX_DISPLAYED_EVENT(initPrngAndWriteNoise_Cb(););
            }
            else
            {
                UX_DISPLAYED_EVENT({});
                // ui_idle_init();
            }
        }
#elif defined(TARGET_NANOX)
        UX_DISPLAYED_EVENT({});
#endif
        break;

    case SEPROXYHAL_TAG_TICKER_EVENT:
            UX_TICKER_EVENT(G_io_seproxyhal_spi_buffer, {
                // defaulty retrig very soon (will be overriden during
                // stepper_prepro)
                UX_CALLBACK_SET_INTERVAL(500);
                UX_REDISPLAY();
            });
        break;

    // unknown events are acknowledged
    default:
        UX_DEFAULT_EVENT();
        break;
    }

    // close the event if not done previously (by a display or whatever)
    if (!io_seproxyhal_spi_is_status_sent()) {
        io_seproxyhal_general_status();
    }

    // command has been processed, DO NOT reset the current APDU transport
    return 1;
}

__attribute__((section(".boot"))) int main(void) {
    // exit critical section
    __asm volatile("cpsie i");

    current_text_pos = 0;
    text_y = 60;
    uiState = UI_IDLE;

    // ensure exception will work as planned
    os_boot();

    UX_INIT();

    BEGIN_TRY {
        TRY {
            io_seproxyhal_init();
            USB_power(0);
            USB_power(1);
            revealer_struct_init();
            #ifdef WORDS_IMG_DBG
                // SPRINTF(G_bolos_ux_context.words, "fiscal price law neutral script buyer desert join load venue crucial cloth"); // bug last line 18px font
                // SPRINTF(G_bolos_ux_context.words, "sadness they ceiling trash size skull critic shy toddler never man drastic");
                SPRINTF(G_bolos_ux_context.words, "version section faint federal load term cattle first success sun rent immune");
                // SPRINTF(G_bolos_ux_context.words, "feel miracle entry dust love drink kit what insane river blush pitch"); // bug last line 18px font
                // SPRINTF(G_bolos_ux_context.words, "toto tata titi tutu tete toto tata titi tutu tete");
                G_bolos_ux_context.words_length = strlen(G_bolos_ux_context.words);
                write_words();
                USB_power(1);
            #endif
            ui_idle_init();
            sample_main();
        }
        CATCH_OTHER(e) {
        }
        FINALLY {
        }
    }
    END_TRY;
}
