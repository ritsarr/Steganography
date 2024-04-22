//
//  main.c
//  Steganography
//
//  Created by Даниил on 07.04.2024.
//

#include <stdio.h>
#include <stdlib.h>

size_t get_size_image_bmp(FILE* image){
    int size, temp; size = 0; temp = 0;
    
    for(int i = 5; i > 1; i--){
        size *= 16*16;
        fseek(image, i, SEEK_SET);
        fread(&temp, 1, 1, image);
        size += temp;
    }
    fseek(image, 0, SEEK_SET);
    return size;
}

size_t get_size_text(FILE* text){
    size_t size = 0;
    fseek(text, 0, SEEK_END);
    size = ftell(text);
    fseek(text, 0, SEEK_SET);
    return size;

}

unsigned char make_text_mask(int degree){
    unsigned char text_mask = 0b11111111;
    text_mask <<= (8 - degree);
    
    return text_mask;
}

unsigned char make_image_mask(int degree){
    unsigned char image_mask = 0b11111111;
    image_mask <<= degree;
    
    return image_mask;
}


void hide(){
    FILE *text = fopen("text.txt", "r");
    FILE *image = fopen("image.bmp", "rb");
    FILE *image_s = fopen("image_s.bmp", "wb");
    
    printf("Введите степень упаковки, кратную двум:\n");
    int degree = 0; scanf("%d", &degree);
    
    fseek(image, 0, SEEK_SET);
    size_t size_image, size_text = 0;
    size_image = get_size_image_bmp(image);
    size_text = get_size_text(text);
    
    if(size_text > size_image/8*degree-54){
        printf("Слишко много букв, больше трёх не перевариваю\n");
        return;
    }
    
    unsigned char *bytes = (unsigned char*)malloc(size_image); if (bytes == NULL) return;
    fread(bytes, 1, size_image, image);

    unsigned char *data = (unsigned char*)malloc(size_text); if (data == NULL) return;
    fread(data, 1, size_text, text);
    
    unsigned char text_mask = make_text_mask(degree);
    unsigned char image_mask = make_image_mask(degree);
    
    
    int index = 54;
    for(size_t current = 0; current < size_text; current++){
        for(int i = 0; i < 8; i += degree){
            
            unsigned char image_byte = bytes[index] & image_mask;
            unsigned char bits = data[current] & text_mask;
            bits >>= 8-degree;
            image_byte |= bits;
            
            bytes[index] = image_byte;
                        
            data[current] <<= degree;
            index++;
        }
    }
    
    fwrite(bytes, 1, size_image, image_s);

    free(bytes);
    free(data);
    fclose(text);
    fclose(image);
    fclose(image_s);

}

void unhide(){
    FILE *image_s = fopen("image_s.bmp", "rb");
    FILE *text_s = fopen("text_s.txt", "wb");
    
    printf("Введите степень запаковки:\n");
    int degree = 0; scanf("%d", &degree);
    printf("Введите длину сообщения в байтах:\n");
    int word_amount = 0; scanf("%d", &word_amount);
    
    size_t size_image_s, size_text_s = 0;
    size_image_s = get_size_image_bmp(image_s);
    size_text_s = word_amount;
    
    unsigned char *bytes = (unsigned char*)malloc(size_image_s); if (bytes == NULL) return;
    fread(bytes, 1, size_image_s, image_s);
    
    unsigned char *data = (unsigned char*)malloc(size_text_s); if (data == NULL) return;
    fread(data, 1, size_text_s, text_s);

    if(word_amount > size_image_s/8*degree-54){
        printf("Полегче, сталкер, так много букв сюда не влезет\n");
        return;
    }
    
    unsigned char image_mask = make_image_mask(degree);
    image_mask = ~image_mask;
    
    int index = 54;
    for(size_t current = 0; current < size_text_s; current++){
        unsigned char symbol = 0;
        for(int i = 0; i < 8; i += degree){
            unsigned char image_byte = bytes[index] & image_mask;
            
            symbol <<= degree;
            symbol |= image_byte;
            index++;
        }
        data[current] = symbol;
    }

    fwrite(data, 1, size_text_s, text_s);

    fclose(text_s);
    fclose(image_s);
}

int main(int argc, char* argv[]){
    hide();
    unhide();
    return 0;
}

