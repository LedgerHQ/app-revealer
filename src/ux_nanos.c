#include "ux_nanos.h"
#include "error_codes.h"
#include "revealer.h"

#define UX_CALLBACK_INTERVAL_VALUE      200

enum UI_STATE { UI_IDLE, UI_TEXT, UI_APPROVAL };

enum UI_STATE uiState;

extern const ux_menu_entry_t menu_about_nanos[];
extern const ux_menu_entry_t ui_idle_mainmenu_nanos[];
extern const ux_menu_entry_t ui_type_seed_words_menu_nanos[];
extern const bagl_element_t ui_confirm_seed_display_nanos[];

const ux_menu_entry_t menu_about_nanos[] = {
    {NULL, NULL, 0, NULL, "Version", APPVERSION, 0, 0},
    {NULL, ui_idle_init, 1, &C_icon_back, "Back", NULL, 61, 40},
    UX_MENU_END}; 

const ux_menu_entry_t ui_idle_mainmenu_nanos[] = {
  {NULL, ui_type_noise_seed_nanos_init, 0, &C_badge_revealer, "Type your", "revealer code", 32, 10},
  //{NULL, screen_onboarding_3_restore_init, 0, /*&icon_hack*/NULL, "Type your", "seed words", 32, 10},
  {menu_about_nanos, NULL, 0, NULL, "About", NULL, 0, 0},
  {NULL, os_sched_exit, 0, &C_icon_dashboard, "Quit app", NULL, 50, 29},
  UX_MENU_END
};

const ux_menu_entry_t ui_idle_mainmenu_nanos_noise_seed_valid[] = {
  {NULL, screen_onboarding_3_restore_init, 0, &C_badge_revealer, "Type your", "seed words", 32, 10},
  {menu_about_nanos, NULL, 0, NULL, "About", NULL, 0, 0},
  {NULL, os_sched_exit, 0, &C_icon_dashboard, "Quit app", NULL, 50, 29},
  UX_MENU_END
};

const ux_menu_entry_t ui_idle_mainmenu_nanos_words_seed_valid[] = {
  {NULL, ui_type_noise_seed_nanos_init, 0, &C_badge_revealer, "Type your", "revealer code", 32, 10},
  {menu_about_nanos, NULL, 0, NULL, "About", NULL, 0, 0},
  {NULL, os_sched_exit, 0, &C_icon_dashboard, "Quit app", NULL, 50, 29},
  UX_MENU_END
};

const ux_menu_entry_t ui_idle_mainmenu_nanos_all_valid[] = {
  {NULL, NULL, 0, &C_badge_revealer, "Encrypted", "backup ready", 32, 10},
  {menu_about_nanos, NULL, 0, NULL, "About", NULL, 0, 0},
  {NULL, os_sched_exit, 0, &C_icon_dashboard, "Quit app", NULL, 50, 29},
  UX_MENU_END
};

const ux_menu_entry_t ui_type_seed_words_menu_nanos[] = {
  {NULL, screen_onboarding_3_restore_init, 0, /*&icon_hack*/NULL, "Type your", "seed words", 32, 10},
  //{/*menu_settings_nanos*/NULL, NULL, 0, NULL, "Settings", NULL, 0, 0},
  {ui_idle_mainmenu_nanos, NULL, 0, &C_icon_dashboard, "Abort", NULL, 50, 29},
  UX_MENU_END
}; 

/*const bagl_element_t ui_confirm_seed_display_nanos[] = {
    // type                               userid    x    y   w    h  str rad
    // fill      fg        bg      fid iid  txt   touchparams...       ]
    {{BAGL_RECTANGLE, 0x00, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF,
      0, 0},
     NULL,
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},

    {{BAGL_LABELINE, 0x01, 0, 12, 128, 32, 0, 0, 0, 0xFFFFFF, 0x000000,
      BAGL_FONT_OPEN_SANS_EXTRABOLD_11px | BAGL_FONT_ALIGNMENT_CENTER , 0},
     "Confirm seed",
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},
    {{BAGL_LABELINE, 0x01, 1, 26, 128, 32, 0, 0, 0, 0xFFFFFF, 0x000000,
      BAGL_FONT_OPEN_SANS_EXTRABOLD_11px | BAGL_FONT_ALIGNMENT_CENTER , 0},
     "display",
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},

    {{BAGL_ICON, 0x00, 3, 12, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0,
      BAGL_GLYPH_ICON_CROSS},
     NULL,
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},

    {{BAGL_ICON, 0x00, 117, 13, 8, 6, 0, 0, 0, 0xFFFFFF, 0x000000, 0,
      BAGL_GLYPH_ICON_CHECK},
     NULL,
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},

};

void seed_display_cancel(void){
  G_io_apdu_buffer[0] = DENIED_BY_USER >> 8;
  G_io_apdu_buffer[1] = DENIED_BY_USER & 0xff;
  io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, 2);
  ui_idle_init();
}

void seed_display_confirm(void){
  int replysize = 250;
  int offset = 0;
  switch (G_bolos_ux_context.onboarding_kind){
    case 24:
      offset = 128;
      break;
    case 18:
      offset = 64;
      break;
    case 12:
      offset = 64;
      break;
    default:
      offset = 0;
      break;
  }
  memcpy(G_io_apdu_buffer, &G_bolos_ux_context.string_buffer[offset], replysize);
  //THROW(0x6FFA);
  G_io_apdu_buffer[replysize] = SW_OK >> 8;
  G_io_apdu_buffer[replysize+1] = SW_OK & 0xff;
  io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, replysize+2);
  ui_idle_init();
}

unsigned int ui_confirm_seed_display_nanos_button(unsigned int button_mask,unsigned int button_mask_counter) {
    switch (button_mask) {
    case BUTTON_EVT_RELEASED | BUTTON_RIGHT:       
        //confirm
        seed_display_confirm();
        break;

    case BUTTON_EVT_RELEASED | BUTTON_LEFT:
        // deny
        seed_display_cancel();
        break;

    default:
        break;
    }
}*/

void ui_idle_init(void) {
  uiState = UI_IDLE;
  // PRINTF("%d", G_io_apdu_media);
  if ((G_bolos_ux_context.noise_seed_valid == 1)&&(G_bolos_ux_context.words_seed_valid == 1)){
    // //Activate io
    USB_power(0);
    USB_power(1);
    UX_MENU_DISPLAY(0, ui_idle_mainmenu_nanos_all_valid, NULL);
  }
  else if (G_bolos_ux_context.noise_seed_valid == 1){
    UX_MENU_DISPLAY(0, ui_idle_mainmenu_nanos_noise_seed_valid, NULL); 
  }
  else if ((G_bolos_ux_context.noise_seed_valid == 0)&&(G_bolos_ux_context.words_seed_valid == 1)){
    UX_MENU_DISPLAY(0, ui_idle_mainmenu_nanos_words_seed_valid, NULL);
  }
  else{
    //revealer_struct_init();
    UX_MENU_DISPLAY(0, ui_idle_mainmenu_nanos, NULL);    
  }
  UX_MENU_DISPLAY(0, ui_idle_mainmenu_nanos_noise_seed_valid, NULL);
  // setup the first screen changing
  UX_CALLBACK_SET_INTERVAL(UX_CALLBACK_INTERVAL_VALUE);
}

void ui_type_seed_words_init(void){
  uiState = UI_TEXT;

  UX_MENU_DISPLAY(0, ui_type_seed_words_menu_nanos, NULL);
  // setup the first screen changing
  UX_CALLBACK_SET_INTERVAL(UX_CALLBACK_INTERVAL_VALUE);
}

/*void ui_confirm_seed_display_init(void){
  UX_DISPLAY(ui_confirm_seed_display_nanos,NULL);
}*/

/*const bagl_element_t ui_revealer_in_progress[] = {
    // erase
    {{BAGL_RECTANGLE, 0x00, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF,
      0, 0},
     NULL,
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},
    {{BAGL_LABELINE, 0x00, 10, 12, 128, 32, 0, 0, 0, 0xFFFFFF, 0x000000,
      BAGL_FONT_OPEN_SANS_EXTRABOLD_11px | BAGL_FONT_ALIGNMENT_CENTER, 0},
     "Revealer in progress",
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},
    {{BAGL_LABELINE, 0x03, 10, 26, 128, 32, 0, 0, 0, 0xFFFFFF, 0x000000,
      BAGL_FONT_OPEN_SANS_EXTRABOLD_11px | BAGL_FONT_ALIGNMENT_CENTER, 0},
     "0/159",
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},
};

unsigned int ui_revealer_in_progress_prepro(const  bagl_element_t* element){
  return 1;
}

unsigned int ui_revealer_in_progress_button(unsigned int button_mask,unsigned int button_mask_counter) {
  switch (button_mask) {
  case BUTTON_EVT_RELEASED|BUTTON_LEFT|BUTTON_RIGHT:
      //os_sched_exit(0);
      break;
  default:
      break;
  }
  ui_idle_init();
}

const bagl_element_t ui_revealer_final[] = {
    // erase
    {{BAGL_RECTANGLE, 0x00, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF,
      0, 0},
     NULL,
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},

    {{BAGL_ICON, 0x02, 6, 9, 14, 14, 0, 0, 0, 0xFFFFFF, 0x000000, 0,
      BAGL_GLYPH_ICON_CHECK_BADGE},
     NULL,
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},
    {{BAGL_LABELINE, 0x00, 10, 12, 128, 32, 0, 0, 0, 0xFFFFFF, 0x000000,
      BAGL_FONT_OPEN_SANS_EXTRABOLD_11px | BAGL_FONT_ALIGNMENT_CENTER, 0},
     "Revealer successfully",
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},
    {{BAGL_LABELINE, 0x03, 10, 26, 128, 32, 0, 0, 0, 0xFFFFFF, 0x000000,
      BAGL_FONT_OPEN_SANS_EXTRABOLD_11px | BAGL_FONT_ALIGNMENT_CENTER, 0},
     "generated",
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},
};

unsigned int ui_revealer_final_prepro(const  bagl_element_t* element){
  return 1;
}

unsigned int ui_revealer_final_button(unsigned int button_mask,unsigned int button_mask_counter) {
  switch (button_mask) {
  case BUTTON_EVT_RELEASED|BUTTON_LEFT|BUTTON_RIGHT:
      os_sched_exit(0);
      break;
  default:
      break;
  }
}*/

#define PIN_DIGIT_LEN 17
static const char C_pin_digit[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C', 'D', 'E', 'F', '<'};

const bagl_element_t ui_noise_seed_final_compare_nanos[] = {
    // erase
    {{BAGL_RECTANGLE, 0x00, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF,
      0, 0},
     NULL,
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},

    {{BAGL_ICON, 0x01, 6, 9, 14, 14, 0, 0, 0, 0xFFFFFF, 0x000000, 0,
      BAGL_GLYPH_ICON_CROSS_BADGE},
     NULL,
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},
     {{BAGL_ICON, 0x02, 6, 9, 14, 14, 0, 0, 0, 0xFFFFFF, 0x000000, 0,
      BAGL_GLYPH_ICON_CHECK_BADGE},
     NULL,
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},
    {{BAGL_LABELINE, 0x00, 10, 12, 128, 32, 0, 0, 0, 0xFFFFFF, 0x000000,
      BAGL_FONT_OPEN_SANS_EXTRABOLD_11px | BAGL_FONT_ALIGNMENT_CENTER, 0},
     "Noise seed",
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},
    {{BAGL_LABELINE, 0x03, 10, 26, 128, 32, 0, 0, 0, 0xFFFFFF, 0x000000,
      BAGL_FONT_OPEN_SANS_EXTRABOLD_11px | BAGL_FONT_ALIGNMENT_CENTER, 0},
     G_bolos_ux_context.string_buffer,
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},
};

unsigned int ui_noise_seed_final_compare_nanos_prepro(const  bagl_element_t* element){
  if (element->component.userid == 0x03){ 
    if (G_bolos_ux_context.noise_seed_valid == 1){
      SPRINTF(G_bolos_ux_context.string_buffer, "is valid");
    }
    else{
      SPRINTF(G_bolos_ux_context.string_buffer, "is invalid");
    }    
  }
  else if (element->component.userid == 0x02){ //check
    if (G_bolos_ux_context.noise_seed_valid == 0){
      return 0;
    }
  }
  return 1;
}

unsigned int ui_noise_seed_final_compare_nanos_button(unsigned int button_mask,unsigned int button_mask_counter) {
  switch (button_mask) {
  case BUTTON_EVT_RELEASED|BUTTON_LEFT|BUTTON_RIGHT:
      if (G_bolos_ux_context.noise_seed_valid){
        G_bolos_ux_context.processing = 2;
        display_processing_screen();
      }
      else {
        ui_idle_init();                
      }
      break;
  default:
      break;
  }
  return 0;
}

const bagl_element_t ui_type_noise_seed_nanos[] = {
  // type             userid    x    y    w    h    str   rad  fill              fg        bg     font_id                   icon_id  
  { {BAGL_RECTANGLE,  0x00,     0,   0, 128,  32,    0,    0,  BAGL_FILL,  0x000000, 0xFFFFFF,    0,                         0}, 
    NULL, 
    0, 
    0, 0, 
    NULL, NULL, NULL},

  { {BAGL_ICON,       0x00,    3,   12,   7,   7,    0,    0,         0,   0xFFFFFF, 0x000000,    0,                          BAGL_GLYPH_ICON_DOWN  }, 
    NULL, 
    0, 
    0, 0, 
    NULL, NULL, NULL },
    
  { {BAGL_ICON,       0x00,  117,   13,   8,   6,    0,    0,         0,   0xFFFFFF, 0x000000,    0,                          BAGL_GLYPH_ICON_UP  }, 
     NULL, 
     0, 
     0, 0, 
     NULL, NULL, NULL },

  /*{ {BAGL_LABELINE,   0x01,    0,   12, 128,  32,    0,    0,         0,   0xFFFFFF, 0x000000,    BAGL_FONT_OPEN_SANS_EXTRABOLD_11px|BAGL_FONT_ALIGNMENT_CENTER, 0  }, 
    "Enter noise seed",  
    0, 
    0, 0, 
    NULL, NULL, NULL },*/
  { {BAGL_LABELINE,   0x01,    0,   12, 128,  32,    0,    0,         0,   0xFFFFFF, 0x000000,    BAGL_FONT_OPEN_SANS_REGULAR_11px|BAGL_FONT_ALIGNMENT_CENTER, 0  }, 
    "Enter revealer code",  
    0, 
    0, 0, 
    NULL, NULL, NULL },
  { {BAGL_LABELINE,   0x02,    0,   26, 128,  32,    0,    0,         0,   0xFFFFFF, 0x000000,    BAGL_FONT_OPEN_SANS_REGULAR_11px|BAGL_FONT_ALIGNMENT_CENTER, 0  }, 
    G_bolos_ux_context.string_buffer,
    //noise_seed_display,
    0, 
    0, 0, 
    NULL, NULL, NULL },
};

#define MAX_CHAR_PER_LINE 16

unsigned int ui_type_noise_seed_nanos_prepro(const  bagl_element_t* element){
  //36 noise seed len
  //8 number of spaces (digits are grouped 4by4 separated by spaces)
  //1 for EOL
  uint8_t intermediaryBuffer[36+8+1];
  uint8_t digitsAndSpaceCounter = 0;
  uint8_t idx = 0;
  uint8_t printIdx = 0;
  uint8_t processed = 1;
  memset(intermediaryBuffer, 0x00, 36+8+1);
  if (element->component.userid == 0x02){ // ie noise_seed_display
    while(idx<G_bolos_ux_context.typedDigitLen){
      // insert space between 4 digit groups processed to skip first digit
      if ((idx%4==0)&&(!processed)){
        intermediaryBuffer[digitsAndSpaceCounter] = ' ';
        processed = 1;
      }
      // insert digit
      else {
        intermediaryBuffer[digitsAndSpaceCounter] = G_bolos_ux_context.noise_seed[idx];
        idx ++;
        processed = 0;          
      }
      digitsAndSpaceCounter++;
    }    
    if ((digitsAndSpaceCounter+1)%5==0){
      intermediaryBuffer[digitsAndSpaceCounter] = ' ';
      digitsAndSpaceCounter++;
    }
    // Append current typed digit to intermediary buffer
    intermediaryBuffer[digitsAndSpaceCounter] = C_pin_digit[G_bolos_ux_context.offset];      
    // Copy last MAX_CHAR_PER_LINE from intermediary buffer to string_buffer for display
    printIdx = strlen(intermediaryBuffer)>MAX_CHAR_PER_LINE ? strlen(intermediaryBuffer)-MAX_CHAR_PER_LINE:0;
    strcpy(G_bolos_ux_context.string_buffer, &intermediaryBuffer[printIdx]);
  }
  // UX_CALLBACK_SET_INTERVAL(200);
  return 1;
}

// max shift = 12
const bagl_element_t ui_processing[] = {
    // erase
    {{BAGL_RECTANGLE, 0x00, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF,
      0, 0},
     NULL,
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},

    {{BAGL_ICON, 0x01, 11, 9, 14, 14, 0, 0, 0, 0xFFFFFF, 0x000000, 0,
      BAGL_GLYPH_ICON_LOADING_BADGE},
     NULL,
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},

    {{BAGL_LABELINE, 0x00, 10, 20, 128, 32, 0, 0, 0, 0xFFFFFF, 0x000000,
      BAGL_FONT_OPEN_SANS_EXTRABOLD_11px | BAGL_FONT_ALIGNMENT_CENTER | BAGL_FONT_ALIGNMENT_MIDDLE, 0},
     "Processing...",
     0,
     0,
     0,
     NULL,
     NULL,
     NULL}
};

const bagl_element_t * 
ui_processing_before_element_display_callback(const bagl_element_t *element) 
{
    // copy element to be displayed
    os_memmove(&G_bolos_ux_context.tmp_element, PIC(element),
               sizeof(G_bolos_ux_context.tmp_element));

    if(element->component.userid == 0x01)
    {
        G_bolos_ux_context.tmp_element.component.width =
            C_badge_loading_v2.width;
        
        G_bolos_ux_context.tmp_element.component.y = 9;
        G_bolos_ux_context.tmp_element.component.height =
            C_badge_loading_v2.height;
        G_bolos_ux_context.tmp_element.component.type = BAGL_ICON;
        G_bolos_ux_context.tmp_element.component.icon_id = 0;
        G_bolos_ux_context.tmp_element.text =
            (const char *)&C_badge_loading_v2;
    }
    // update element display
    return &G_bolos_ux_context.tmp_element;
}


unsigned int ui_type_noise_seed_nanos_button(unsigned int button_mask,unsigned int button_mask_counter) {
  switch (button_mask) {
  case BUTTON_EVT_RELEASED | BUTTON_LEFT: //decrement digit
      if (G_bolos_ux_context.offset == 0){
        G_bolos_ux_context.offset = G_bolos_ux_context.typedDigitLen == 0 ? PIN_DIGIT_LEN-2:PIN_DIGIT_LEN-1;
      }
      else {
        G_bolos_ux_context.offset = G_bolos_ux_context.offset-1;
      }
      break;

  case BUTTON_EVT_RELEASED | BUTTON_RIGHT:  //increment digit
        G_bolos_ux_context.offset = G_bolos_ux_context.typedDigitLen == 0 ? (G_bolos_ux_context.offset+1)%(PIN_DIGIT_LEN-1):(G_bolos_ux_context.offset+1)%PIN_DIGIT_LEN;
      break;

  case BUTTON_EVT_RELEASED|BUTTON_LEFT|BUTTON_RIGHT:
      switch (C_pin_digit[G_bolos_ux_context.offset]){
        case '<': //Erase
          if (G_bolos_ux_context.typedDigitLen > 0){
            G_bolos_ux_context.string_buffer[G_bolos_ux_context.typedDigitLen] = '\0';
            G_bolos_ux_context.typedDigitLen--;
          }
          G_bolos_ux_context.offset = G_bolos_ux_context.typedDigitLen == 0 ? 7:PIN_DIGIT_LEN-1;
          break;
        default:  //Confirm digit value               
          G_bolos_ux_context.noise_seed[G_bolos_ux_context.typedDigitLen] = C_pin_digit[G_bolos_ux_context.offset];
          G_bolos_ux_context.typedDigitLen++;
          G_bolos_ux_context.offset = 7;
          if (G_bolos_ux_context.typedDigitLen == 36){
            if (isNoise(G_bolos_ux_context.noise_seed,33)){
              G_bolos_ux_context.noise_seed_valid = 1;
            }
            else {
              G_bolos_ux_context.noise_seed_valid = 0;             
            }
            UX_DISPLAY(ui_noise_seed_final_compare_nanos, ui_noise_seed_final_compare_nanos_prepro);
          }
          break;
      }        
      break;
  default:
      break;
  }
  UX_CALLBACK_SET_INTERVAL(UX_CALLBACK_INTERVAL_VALUE);
  return 0;
}

void revealer_struct_init(void){
  os_memset(G_bolos_ux_context.noise_seed, NULL, 36);
  os_memset(G_bolos_ux_context.string_buffer, NULL, MAX(64, sizeof(bagl_icon_details_t) +
                                   BOLOS_APP_ICON_SIZE_B -
                                   1));
  os_memset(G_bolos_ux_context.words, NULL, SEED_SIZE);
  G_bolos_ux_context.typedDigitLen = 0;
  G_bolos_ux_context.offset = 7;
  #ifndef WORDS_IMG_DBG
    G_bolos_ux_context.noise_seed_valid = 0;
  #else
    G_bolos_ux_context.noise_seed_valid = 1;
  #endif
  G_bolos_ux_context.words_seed_valid = 0;
}

void ui_type_noise_seed_nanos_init(void){
  revealer_struct_init();
  UX_DISPLAY(ui_type_noise_seed_nanos,ui_type_noise_seed_nanos_prepro);
  UX_CALLBACK_SET_INTERVAL(UX_CALLBACK_INTERVAL_VALUE);
}

void initPrngAndWriteNoise_Cb(void){
  G_bolos_ux_context.processing = 0;
  io_seproxyhal_general_status();
  drbg_hmac_init();
  drbg_reseed(G_bolos_ux_context.noise_seed_bytearray, 1);
  drbg_write_noise();
  ui_idle_init();
}
