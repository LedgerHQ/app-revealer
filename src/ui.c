#include "ui.h"

enum UI_STATE { UI_IDLE, UI_TEXT, UI_APPROVAL };

enum UI_STATE uiState;

#if defined(TARGET_NANOS)
#define UX_CALLBACK_INTERVAL_VALUE 200
void ui_type_noise_seed_nanos_init(void);
const ux_menu_entry_t menu_about_nanos[] = {
    {NULL, NULL, 0, NULL, "Version", APPVERSION, 0, 0},
    {NULL, ui_idle_init, 1, &C_icon_back, "Back", NULL, 61, 40},
    UX_MENU_END}; 

const ux_menu_entry_t ui_idle_mainmenu_nanos[] = {
  {NULL, ui_type_noise_seed_nanos_init, 0, &C_badge, "Type your", "revealer code", 32, 10},
  //{NULL, screen_onboarding_3_restore_init, 0, /*&icon_hack*/NULL, "Type your", "seed words", 32, 10},
  {menu_about_nanos, NULL, 0, NULL, "About", NULL, 0, 0},
  {NULL, os_sched_exit, 0, &C_icon_dashboard, "Quit app", NULL, 50, 29},
  UX_MENU_END
};

const ux_menu_entry_t ui_idle_mainmenu_nanos_noise_seed_valid[] = {
  {NULL, screen_onboarding_3_restore_init, 0, &C_badge, "Type your", "seed words", 32, 10},
  {menu_about_nanos, NULL, 0, NULL, "About", NULL, 0, 0},
  {NULL, os_sched_exit, 0, &C_icon_dashboard, "Quit app", NULL, 50, 29},
  UX_MENU_END
};

const ux_menu_entry_t ui_idle_mainmenu_nanos_words_seed_valid[] = {
  {NULL, ui_type_noise_seed_nanos_init, 0, &C_badge, "Type your", "revealer code", 32, 10},
  {menu_about_nanos, NULL, 0, NULL, "About", NULL, 0, 0},
  {NULL, os_sched_exit, 0, &C_icon_dashboard, "Quit app", NULL, 50, 29},
  UX_MENU_END
};

const ux_menu_entry_t ui_idle_mainmenu_nanos_all_valid[] = {
  {NULL, NULL, 0, &C_badge, "Encrypted", "backup ready", 32, 10},
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

const bagl_element_t * 
screen_onboarding_4_processing_before_element_display_callback(const bagl_element_t *element) 
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

unsigned int screen_onboarding_4_processing_button(
    unsigned int button_mask, unsigned int button_mask_counter){
        // No interaction here
    }

const bagl_element_t screen_onboarding_4_processing[] = {
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

void display_processing_screen(void){
    UX_DISPLAY(screen_onboarding_4_processing, screen_onboarding_4_processing_before_element_display_callback);
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
  UX_CALLBACK_SET_INTERVAL(UX_CALLBACK_INTERVAL_VALUE);
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

UX_STEP_VALID(
    ux_idle_flow_1_step, 
    pbb,
    // screen_onboarding_3_restore_init();,
    ui_type_noise_seed_nanos_init(),
    {
      &C_badge,
      "Type your",
      "revelaer code",
    });
UX_STEP_NOCB(
    ux_idle_flow_3_step, 
    bn, 
    {
      "Version",
      APPVERSION,
    });
UX_STEP_VALID(
    ux_idle_flow_4_step,
    pb,
    os_sched_exit(-1),
    {
      &C_icon_dashboard_x,
      "Quit",
    });
UX_FLOW(ux_idle_flow,
  &ux_idle_flow_1_step,
  &ux_idle_flow_3_step,
  &ux_idle_flow_4_step
);

UX_STEP_VALID(
    ux_noise_seed_valid_flow_1_step, 
    pbb,
    screen_onboarding_3_restore_init();,
    // ui_type_noise_seed_nanos_init(),
    {
      &C_badge,
      "Type your",
      "seed words",
    });
UX_STEP_NOCB(
    ux_noise_seed_valid_flow_3_step, 
    bn, 
    {
      "Version",
      APPVERSION,
    });
UX_STEP_VALID(
    ux_noise_seed_valid_flow_4_step,
    pb,
    os_sched_exit(-1),
    {
      &C_icon_dashboard_x,
      "Quit",
    });
UX_FLOW(ux_noise_seed_valid_flow,
  &ux_noise_seed_valid_flow_1_step,
  &ux_noise_seed_valid_flow_3_step,
  &ux_noise_seed_valid_flow_4_step
);

UX_STEP_VALID(
    ux_all_valid_flow_1_step, 
    pbb,
    NULL,
    {
      &C_badge,
      "Revealer",
      "Ready",
    });
UX_STEP_NOCB(
    ux_all_valid_flow_3_step, 
    bn, 
    {
      "Version",
      APPVERSION,
    });
UX_STEP_VALID(
    ux_all_valid_flow_4_step,
    pb,
    os_sched_exit(-1),
    {
      &C_icon_dashboard_x,
      "Quit",
    });
UX_FLOW(ux_all_valid_flow,
  &ux_all_valid_flow_1_step,
  &ux_all_valid_flow_3_step,
  &ux_all_valid_flow_4_step
);

#elif defined(TARGET_NANOX)

//////////////////////////////////////////////////////////////////////

const char* const number_of_words_getter_values[] = {
  "12 words",
  "18 words",
  "24 words",
  "Back",
};

const char* number_of_words_getter(unsigned int idx) {
  if (idx < ARRAYLEN(number_of_words_getter_values)) {
    return number_of_words_getter_values[idx];
  }
  return NULL;
}

void number_of_words_selector(unsigned int idx) {
  switch(idx) {
    case 0:
      G_bolos_ux_context.onboarding_kind = 12;
      screen_onboarding_4_restore_word_init(1 /*entering the first word*/);
      break;
    case 1:
      G_bolos_ux_context.onboarding_kind = 18;
      screen_onboarding_4_restore_word_init(1 /*entering the first word*/);
      break;
    case 2:
      G_bolos_ux_context.onboarding_kind = 24;
      screen_onboarding_4_restore_word_init(1 /*entering the first word*/);
      break;
    default:
      ui_idle_init();
  }
}

//////////////////////////////////////////////////////////////////////

UX_STEP_VALID(
    ux_instruction_step, 
    nnn,
    ux_menulist_init(0, number_of_words_getter, number_of_words_selector),
    {
      "Select the number",
      "of words written on",
      "your Recovery Sheet",
    });

UX_FLOW(ux_instruction_flow,
  &ux_instruction_step
);

//////////////////////////////////////////////////////////////////////

UX_STEP_VALID(
    ux_idle_flow_1_step, 
    pbb,
    ux_flow_init(0, ux_instruction_flow, NULL),
    {
      &C_badge,
      "Check your",
      "recovery phrase",
    });
UX_STEP_NOCB(
    ux_idle_flow_3_step, 
    bn, 
    {
      "Version",
      APPVERSION,
    });
UX_STEP_VALID(
    ux_idle_flow_4_step,
    pb,
    os_sched_exit(-1),
    {
      &C_icon_dashboard_x,
      "Quit",
    });
UX_FLOW(ux_idle_flow,
  &ux_idle_flow_1_step,
  &ux_idle_flow_3_step,
  &ux_idle_flow_4_step
);

#endif


void ui_idle_init(void) {
  uiState = UI_IDLE;

  // reserve a display stack slot if none yet
  if(G_ux.stack_count == 0) {
      ux_stack_push();
  }
  if ((G_bolos_ux_context.noise_seed_valid == 1)&&(G_bolos_ux_context.words_seed_valid == 1)){
    // UX_MENU_DISPLAY(0, ui_idle_mainmenu_nanos_all_valid, NULL);
    PRINTF("ALL VALID\n");
    ux_flow_init(0, ux_all_valid_flow, NULL);
  }
  else if (G_bolos_ux_context.noise_seed_valid == 1){
    // UX_MENU_DISPLAY(0, ui_idle_mainmenu_nanos_noise_seed_valid, NULL);
    ux_flow_init(0, ux_noise_seed_valid_flow, NULL);
  }
  else if ((G_bolos_ux_context.noise_seed_valid == 0)&&(G_bolos_ux_context.words_seed_valid == 1)){
    // UX_MENU_DISPLAY(0, ui_idle_mainmenu_nanos_words_seed_valid, NULL);
  }
  else{
    // UX_MENU_DISPLAY(0, ui_idle_mainmenu_nanos, NULL);
    ux_flow_init(0, ux_idle_flow, NULL);
  }
  // ux_flow_init(0, ux_idle_flow, NULL);
  // ux_flow_init(0, ux_noise_seed_valid_flow, NULL);
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
