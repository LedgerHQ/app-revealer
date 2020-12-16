#include "revealer.h"
#include "bolos_ux_nanos.h"
#include "font.h"
#include "cx.h"

WIDE internalStorage_t N_storage_real;
#define N_storage (*(WIDE internalStorage_t *)PIC(&N_storage_real)) 

uint8_t isNoise(char * string, uint8_t hashPos){
  uint8_t computedHash[32];
  uint8_t hashIdCMP[4];
  uint8_t stringLowerCase[36];
  int hashId = 0;
  uint8_t digit;

  if (string == NULL){
    return 0;
  }

  //convert ABCDEF to lower case
  for (int i=0; i<36; i++){
    stringLowerCase[i] = ((string[i]>=0x41)&&(string[i]<=0x46))?string[i]+0x20:string[i];
  }
  
  cx_sha256_t context;
  cx_sha256_init(&context);
  cx_hash(&context,CX_LAST,stringLowerCase,hashPos,computedHash, 32);

  for (uint8_t n=0; n<3; n++){
    hashId += (stringLowerCase[n+hashPos] >= 0x30)&&(stringLowerCase[n+hashPos] <= 0x39)?(stringLowerCase[n+hashPos] - 0x30)<<4*(2-n):(stringLowerCase[n+hashPos] - 0x57)<<4*(2-n);
  }
  hashIdCMP[0] = computedHash[30]&0x0F;
  hashIdCMP[1] = computedHash[31];
  hashIdCMP[2] = (hashId>>8)&0x000F;
  hashIdCMP[3] = hashId&0x00FF;
  // Comparison fails if not using intermediary hashIdCMP
  // Invalid cast ?? TODO remove hashIdCMP and replace by valid compare
  if ((hashIdCMP[0] == hashIdCMP[2])&&(hashIdCMP[1] == hashIdCMP[3])){
    // Noise seed is Valid
    return 1;
  }
  return 0;
}

uint8_t hexStringToByteArray(char* string, uint8_t slength, uint8_t* byteArray) {
    if(string == NULL) 
       return 0;
    if((slength % 2) != 0) // must be even
       return 0;

    memset(byteArray, 0x00, 18);
    size_t dlength = slength / 2;
    size_t index = 0;

    while (index < slength) {
        char c = string[index];
        int value = 0;
        if(c >= '0' && c <= '9')
          value = (c - '0');
        else if (c >= 'A' && c <= 'F') 
          value = (10 + (c - 'A'));
        else if (c >= 'a' && c <= 'f')
          value = (10 + (c - 'a'));
        
        byteArray[(index/2)] += value << (((index + 1) % 2) * 4);

        index++;
    }
    return 1;
}

void drbg_reseed(uint8_t * data, uint8_t dataPresent){
  uint8_t key_out[64];
  uint8_t val_out[64];
  uint8_t val_in[64+1];
  uint8_t valData_in[64+1+18];

  if (dataPresent){
    memcpy(valData_in,G_bolos_ux_context.hmac_drbg_val, 64);
    valData_in[64] = 0x00;
    memcpy(&valData_in[65], data, 18);
    cx_hmac_sha512(G_bolos_ux_context.hmac_drbg_key, 64, valData_in, 64+1+18, key_out, 64);
    cx_hmac_sha512(key_out, 64, G_bolos_ux_context.hmac_drbg_val, 64, val_out, 64);
    
    memcpy(valData_in,val_out, 64);
    valData_in[64] = 0x01;
    memcpy(&valData_in[65], data, 18);
    cx_hmac_sha512(key_out, 64, valData_in, 64+1+18, key_out, 64);
    cx_hmac_sha512(key_out, 64, valData_in, 64, val_out, 64);
  }
  else{

  }
  memcpy(G_bolos_ux_context.hmac_drbg_key, key_out, 64);
  memcpy(G_bolos_ux_context.hmac_drbg_val, val_out, 64);
  // PRINTF("%.*h\n", 5, G_bolos_ux_context.hmac_drbg_val);
}

void drbg_hmac_init(void){
  uint8_t noise_seed_processed[36];
  // init key and val arrays
  memset(G_bolos_ux_context.hmac_drbg_key, 0x00, 64);
  memset(G_bolos_ux_context.hmac_drbg_val, 0x01, 64);
  // revealer seed bytes permutation, fist byte (should be 1, for algo version), is inserted at 32th index
  memcpy(noise_seed_processed,      &G_bolos_ux_context.noise_seed[1],  32);
  memcpy(&noise_seed_processed[32], G_bolos_ux_context.noise_seed,      1);
  memcpy(&noise_seed_processed[33], &G_bolos_ux_context.noise_seed[33], 3);
  // Convert noise seed (hexstring) to byte array
  if (hexStringToByteArray(noise_seed_processed, 36, G_bolos_ux_context.noise_seed_bytearray)){
    // PRINTF("%.*h \n", 18, G_bolos_ux_context.noise_seed_bytearray);
  }
  else {
    PRINTF("CONVERT ERROR\n");
    // THROW(0x6900);
  }
}

// Generates 512 (64*8) pixels from a sha 512 hmac
void drbg_generate(uint8_t *sha512_hmac, uint8_t *pixels){
  uint32_t val;
  int p = 0;
  uint8_t dec=0;
  memset(pixels, 0x00, 512);
  int ret = 512, i = 0;

  for (i=0; i<64; i++){
    for (uint8_t j=0; j<8; j++){
      pixels[p] = (sha512_hmac[i]&(1<<(7-j)))>>(7-j);
      p++;
    }
  }
}


// Writes noise in revelaer image
void drbg_write_noise(void){
  int writtenPixels = 0, pixelsRemaining = 0;
  uint8_t firstHmac = 1;
  uint8_t hmac_noise[64];
  uint32_t hmac_noise_32[16];
  uint8_t pixels[512];

  int YX;
  uint8_t x, y, val, cpt;
  cpt = 0;
  val = 0;

  uint8_t zero = 0;

  for (x=0; x<IMG_WIDTH; x++){
    for (y=0; y<IMG_HEIGHT/8; y++){
      YX = y*IMG_WIDTH + x;
      val = 0;
      for (uint8_t j=0; j<8; j++){
        if (pixelsRemaining == 0){
          cx_hmac_sha512(G_bolos_ux_context.hmac_drbg_key, 64, G_bolos_ux_context.hmac_drbg_val, 64, hmac_noise, 64);
          memcpy(G_bolos_ux_context.hmac_drbg_val, hmac_noise, 64);
          drbg_generate(hmac_noise, pixels);
          pixelsRemaining = 512;
          writtenPixels = 0;
          if (zero){
            zero = 0;
            writtenPixels++;
            pixelsRemaining--;            
          }
          if (firstHmac)
          {
            firstHmac = 0;
            while(pixels[writtenPixels]==0){
              writtenPixels++;
              pixelsRemaining--;
            }
          }          
        }
        val += (pixels[writtenPixels])<<j;
        writtenPixels++;
        pixelsRemaining--;
      }
      nvm_write(&N_storage.revealer_image[YX], (uint8_t *)&val, sizeof(uint8_t));
    }
    //IMG_HEIGHT = 97 = 12*8+1 => dump last pixel
    if (pixelsRemaining !=0){
      writtenPixels++;
      pixelsRemaining--;      
    }
    else {
      zero = 1;
    }
  }
}

// Code below is adapted from Nano S MCU screen HAL driver, screen is replaced by an image matrix stored in nvram cf revealer.h
unsigned int screen_changed; // to avoid screen update for nothing
int screen_draw_x;
int screen_draw_y;
unsigned int screen_draw_width;
unsigned int screen_draw_height;
int screen_draw_YX;
int screen_draw_YXlinemax;
int screen_draw_Ybitmask;
unsigned int* screen_draw_colors;

void draw_bitmap_within_rect_internal(unsigned int bit_per_pixel, const unsigned char* bitmap, unsigned int bitmap_length_bits) {
  unsigned int i;
  int xx;  
  //unsigned int pixel_mask = (1<<bit_per_pixel)-1;
  #define pixel_mask 1 // 2 colors only

  int x = screen_draw_x;
  xx = x;
  int y = screen_draw_y;
  unsigned int width = screen_draw_width;
  unsigned int height = screen_draw_height;
  int YX = screen_draw_YX;
  int YXlinemax = screen_draw_YXlinemax;
  int Ybitmask = screen_draw_Ybitmask;
  unsigned int* colors = screen_draw_colors;
  char val, val2;
  screen_changed=1;
  val2 = 0xFFFFFFFF;
  while(bitmap_length_bits) {
    // horizontal scan transformed into vertical
    unsigned int ch = *bitmap++;
    // draw each pixel (at most 256 index color bitmap support)
    
  for (i = 0; i < 8 && bitmap_length_bits; bitmap_length_bits -= bit_per_pixel, i += bit_per_pixel) {
    //for (i = 0; i < 8 ; i++) {
      // grab LSB to MSB bits
      // 2 colors only
      // bit    colorsarray     painted
      // 0      [0]=0               0
      // 1      [1]=0               0
      // 0      [0]=1               1
      // 1      [1]=1               1
      // 
      if (y>=0 && xx>=0) { // else we're out of screen
        val = N_storage.revealer_image[YX];
    #ifdef WORDS_IMG_DBG //Debug macro to write words only in image (no noise)
        if (colors[((ch>>i) & pixel_mask)] != 0) {
          val |= Ybitmask;
        }
        else {
          val &= ~Ybitmask;
        }        
    #else
        if (colors[((ch>>i) & pixel_mask)] != 0) {
          val ^= Ybitmask; 
        }        
    #endif
        nvm_write(&N_storage.revealer_image[YX], (char*)&val, sizeof(char));
      }


      xx++;
      YX++;
      if (YX >= YXlinemax) {
        y++;
        height--;
        // update fast bit operation variables
        YX = (y/8)*IMG_WIDTH + x;
        xx = x;
        YXlinemax = YX + width;
        Ybitmask = 1<<(y%8);
      }
      if (height == 0) {
        goto end;
      }
    } 
  }
  // save for continue
end:
  screen_draw_x = x;
  screen_draw_y = y;
  screen_draw_width = width;
  screen_draw_height = height;
  screen_draw_YX = YX;
  screen_draw_YXlinemax = YXlinemax;
  screen_draw_Ybitmask = Ybitmask;
  return;
}

void draw_bitmap_within_rect(int x, int y, unsigned int width, unsigned int height, unsigned int color_count, const unsigned int *colors, unsigned int bit_per_pixel, const unsigned char* bitmap, unsigned int bitmap_length_bits) {
  // horizontal scan
  if (x>= IMG_WIDTH || y >= IMG_HEIGHT) {
    return;
  }
  if (x+width > IMG_WIDTH) {
    width = IMG_WIDTH-x;
  }

  if (y+height > IMG_HEIGHT) {
    height = IMG_HEIGHT-y;
  }

  int YX = (y/8)*IMG_WIDTH + x;
  int Ybitmask = 1<<(y%8); 
  int YXlinemax = YX + width;

  // run bitmap draw
  screen_draw_x = x;
  screen_draw_y = y;
  screen_draw_width = width;
  screen_draw_height = height;
  screen_draw_YX = YX;
  screen_draw_YXlinemax = YXlinemax;
  screen_draw_Ybitmask = Ybitmask;
  screen_draw_colors = colors;

  draw_bitmap_within_rect_internal(bit_per_pixel, bitmap, bitmap_length_bits);
}

int draw_string(bagl_font_t font_id, unsigned int fgcolor, unsigned int bgcolor, int x, int y, unsigned int width, unsigned int height, const void* text, unsigned int text_length, unsigned char text_encoding) {
  unsigned int xx;
  unsigned int colors[16];
  colors[0] = bgcolor;
  colors[1] = fgcolor;

  bagl_font_t *font = &font_id;

  // always comparing this way, very optimized etc
  width += x;
  height += y;

  // initialize first index
  xx = x;

  //printf("display text: %s\n", text);

  // depending on encoding
  while (text_length--) {
    unsigned int ch = 0;
    // TODO support other encoding than ascii ISO8859 Latin
    switch(text_encoding) {
      default:
      case BAGL_ENCODING_LATIN1:
        ch = *((unsigned char*)text); 
        /*if (ch != ' '){
          ch-=0x20; //uppercase
        }*/
        text = (void*)(((unsigned char*)text)+1);
        break;
    }

    unsigned char ch_height = 0;
    unsigned char ch_kerning = 0;
    unsigned char ch_width = 0;
    const unsigned char * ch_bitmap = NULL;
    int ch_y = y;

    /*if (ch < font->first_char || ch > font->last_char) {
      //printf("invalid char");
      // can't proceed
      if (ch == '\n' || ch == '\r') {
        y += ch_height; // no interleave
        // IGNORED for first line
        if (y + ch_height > height) {
          // we're writing half height of the last line ... probably better to put some dashes
          return (y<<16)|(xx&0xFFFF);
        }
        // newline starts back at first x offset
        xx = x;
        continue;
      }
      // SKIP THE CHAR // return (y<<16)|(xx&0xFFFF);
      ch_width = 0;
      if (ch >= 0xC0) {
        ch_width = ch & 0x3F;
        ch_height = font->char_height;
      }
      else if (ch >= 0x80) {
        // open the glyph font
        //const bagl_font_t *font_symbols = bagl_get_font((ch&0x20)?BAGL_FONT_SYMBOLS_1:BAGL_FONT_SYMBOLS_0);
        const bagl_font_t *font_symbols = BAGL_FONT_SYMBOLS_1;
        if (font_symbols != NULL) {
          ch_bitmap = &font_symbols->bitmap[font_symbols->characters[ch & 0x1F].bitmap_offset];
          ch_width = font_symbols->characters[ch & 0x1F].char_width;
          ch_height = font_symbols->char_height;
          // align baselines
          ch_y = y + font->baseline_height - font_symbols->baseline_height;
        }
      }
    }
    else {*/
      ch -= font->first_char;
      ch_bitmap = &font->bitmap[font->characters[ch].bitmap_offset];
      ch_width = font->characters[ch].char_width;
      ch_kerning = font->char_kerning;
      ch_height = font->char_height;
    //}

    // retrieve the char bitmap

    // go to next line if needed
    if (xx + ch_width > width) {
      y += ch_height; // no interleave

      // IGNORED for first line
      if (y + ch_height > height) {
        // we're writing half height of the last line ... probably better to put some dashes
        return (y<<16)|(xx&0xFFFF);
      }

      // newline starts back at first x offset
      xx = x;
    }

    /* IGNORED for first line
    if (y + ch_height > height) {
        // we're writing half height of the last line ... probably better to put some dashes
        return;
    }
    */

    // chars are storred LSB to MSB in each char, packed chars. horizontal scan
    if (ch_bitmap) {
      draw_bitmap_within_rect(xx, ch_y, ch_width, ch_height, (1<<font->bpp), colors, font->bpp, ch_bitmap, font->bpp*ch_width*ch_height); // note, last parameter is computable could be avoided
    }
    else {
      //draw_rect(bgcolor, xx, ch_y, ch_width, ch_height);
    }
    // prepare for next char
    xx += ch_width + ch_kerning;
  }

  // return newest position, for upcoming printf
  return (y<<16)|(xx&0xFFFF);
}


uint8_t charRemaining;

uint8_t getNextLinePixelWidth(const char* text, bagl_font_t font_id, uint8_t* numChar){
  uint8_t linePixelWidth, linePixelWidthInt, numCharInt, prout;
  linePixelWidth = 0;
  numCharInt = 0;
  *numChar = 0;
  char ch;
  linePixelWidthInt = 0;

  bagl_font_t *font = &font_id;

  while (charRemaining-numCharInt>0){
    ch = *((unsigned char*)text);
    if ((ch==' ')||(ch==NULL)){
      linePixelWidth = linePixelWidthInt;
      *numChar = numCharInt;
    }
    // -10 to add at least 5 pixels on the left and on the right
    if ((linePixelWidthInt > (IMG_WIDTH-10))||(ch==NULL)){
      return linePixelWidth;
    }
    ch -= font->first_char;
    prout = font->characters[ch].char_width;
    linePixelWidthInt += font->characters[ch].char_width;
    numCharInt++;    
    *text++;
  }
  return linePixelWidthInt;
}

void write_words(void){
  int bgcolor = 0x000000;
  int fgcolor = 0xFFFFFF;
  //uint8_t charRemainingDBG;
  uint8_t nextLineIdx, linePixCnt, rowPixCnt, numLine, line;
  uint8_t line_h;
  bagl_font_t font;
  uint8_t fontId;
   
  //First check if words can fit in image with 16px font
  line = 0;
  numLine = 0;
  charRemaining = G_bolos_ux_context.words_length + 1; //include '\0'
  PRINTF("len = %d\n", G_bolos_ux_context.words_length);
  PRINTF("words = %s\n", G_bolos_ux_context.words);
  fontId = BAGL_FONT_FONT_SEMIBOLD_18PX;
  line_h = fontFONT_SEMIBOLD_18PX.char_height;
  strcpy(G_bolos_ux_context.string_buffer, G_bolos_ux_context.words);
  while ((charRemaining>0)&&(line+line_h < IMG_HEIGHT)){
    linePixCnt = getNextLinePixelWidth(G_bolos_ux_context.string_buffer, fontFONT_SEMIBOLD_18PX, &nextLineIdx);
    strcpy(G_bolos_ux_context.string_buffer, &G_bolos_ux_context.string_buffer[nextLineIdx+1]);
    charRemaining -= (nextLineIdx+1);
    numLine++;
    line+=line_h;
  }
  line = 0;
  //Chars remaining, words don't fit in image with 16px font > use 11px font
  if (charRemaining !=0){
    charRemaining = G_bolos_ux_context.words_length+1; // include \0 
    numLine = 0;
    fontId = BAGL_FONT_FONT_EXTRABOLD_11PX;
    line_h = fontFONT_EXTRABOLD_11PX.char_height;
    strcpy(G_bolos_ux_context.string_buffer, G_bolos_ux_context.words);
    while ((charRemaining>0)&&(line+line_h < IMG_HEIGHT)){
      linePixCnt = getNextLinePixelWidth(G_bolos_ux_context.string_buffer, fontFONT_EXTRABOLD_11PX, &nextLineIdx);
      strcpy(G_bolos_ux_context.string_buffer, &G_bolos_ux_context.string_buffer[nextLineIdx+1]);
      charRemaining -= (nextLineIdx+1);
      numLine++;
      line+=line_h;
    }
    line = 0;
    if (charRemaining !=0){
      charRemaining = G_bolos_ux_context.words_length+1; // include \0 
      numLine = 0;
      fontId = BAGL_FONT_FONT_LIGHT_11PX;
      line_h = fontFONT_LIGHT_11PX.char_height;
      strcpy(G_bolos_ux_context.string_buffer, G_bolos_ux_context.words);
      while ((charRemaining>0)&&(line+line_h < IMG_HEIGHT)){
        linePixCnt = getNextLinePixelWidth(G_bolos_ux_context.string_buffer, fontFONT_LIGHT_11PX, &nextLineIdx);
        strcpy(G_bolos_ux_context.string_buffer, &G_bolos_ux_context.string_buffer[nextLineIdx+1]);
        charRemaining -= (nextLineIdx+1);
        numLine++;
        line+=line_h;
      }
      line = 0;
    }
    PRINTF("WRITE WORDS DONE\n");
    G_bolos_ux_context.processing = 0;
  }

  // write_words_img:
  // Write words in choosen font 
    switch (fontId){
      case BAGL_FONT_FONT_SEMIBOLD_18PX:
        font = fontFONT_SEMIBOLD_18PX;
        break;
      case BAGL_FONT_FONT_EXTRABOLD_11PX:
        font = fontFONT_EXTRABOLD_11PX;
        break;
      case BAGL_FONT_FONT_LIGHT_11PX:
        font = fontFONT_LIGHT_11PX;
        break;
      }
    line+= (IMG_HEIGHT - numLine*line_h)/2;
    charRemaining = G_bolos_ux_context.words_length+1; // include \0
    while ((charRemaining>0)&&(line+line_h < IMG_HEIGHT)){
      linePixCnt = getNextLinePixelWidth(G_bolos_ux_context.words, font, &nextLineIdx);
      draw_string(font, fgcolor, bgcolor, (IMG_WIDTH-linePixCnt)/2/*10*/,  line, IMG_WIDTH, IMG_HEIGHT, G_bolos_ux_context.words, nextLineIdx, BAGL_ENCODING_LATIN1);
      strcpy(G_bolos_ux_context.words, &G_bolos_ux_context.words[nextLineIdx+1]); // Dump space char
      charRemaining -= (nextLineIdx+1);
      line += line_h;
    }
    // wipe sensitive data
    os_memset(G_bolos_ux_context.words, NULL, SEED_SIZE);
    os_memset(&(G_bolos_ux_context.words_length), NULL, sizeof(int));
    os_memset(G_bolos_ux_context.noise_seed, NULL, NOISE_SEED_LEN);
}

int send_row(int x){
  int YX;
  uint8_t idx;
  idx = 0;
  uint8_t val;
  for (int i=0; i<IMG_HEIGHT/8; i++){
    YX = i*IMG_WIDTH+x;
    val = N_storage.revealer_image[YX];
    for (uint8_t j=0; j<8; j++){
      G_io_apdu_buffer[idx] = (val>>j)&1;
      idx++;
      if (idx == IMG_HEIGHT-1){
        break;
      }
    }
  }
  G_io_apdu_buffer[IMG_HEIGHT-1] = 0;
  return IMG_HEIGHT;
}