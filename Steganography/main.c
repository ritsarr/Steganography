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

void write_info_into_image_bmp(size_t size, size_t size_image, unsigned char* bytes, FILE* image, int degree){
    int temp = 0; int i = 3;
    
    for(; i > 0; i--){
        if(size < 16*16) break;
        temp = (int)size/(16*16);
        bytes[6+i] = temp;
        size %= 16*16;
    }
    temp = (int)size;
    bytes[6+i] = temp;
    bytes[46] = degree;
}

size_t get_size_secret_image_bmp(FILE* image){
    size_t size, temp; size = 0; temp = 0;
    
    for(int i = 3; i > 0; i--){
        fseek(image, i+6, SEEK_SET);
        fread(&temp, 1, 1, image);
        if(temp == 0) break;            //Очень нехорошо если будет ноль между цифрами вообще все полетит
        size *= 16*16;
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


void hide(char* file){
    FILE *text = fopen(file, "rb");
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
    
    write_info_into_image_bmp(size_text, size_image, bytes, image, degree);
    
    int index = 54;
    for(size_t current = 0; current < size_text; current++){
        for(int i = 0; i < 8; i += degree){
            int degree_new = 8-i > degree ? degree : 8-i;
            
            image_mask = make_image_mask(degree_new);
            text_mask = make_text_mask(degree_new);

            unsigned char image_byte = bytes[index] & image_mask;       //очищенный последние
            unsigned char bits = data[current] & text_mask;             //очищенные первые
            bits >>= 8-degree_new;                                      //сдвигаем первые в конец
            image_byte |= bits;                                         //плюсуем с последними
            
            bytes[index] = image_byte;                                  //записываем измененнынй
                        
            data[current] <<= degree_new;                               //убираем записанное теперь первые новые
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
    FILE *text_s = fopen("text_s", "wb");
    
    int degree = 0;
    size_t word_amount = get_size_secret_image_bmp(image_s);
    
    size_t size_image_s, size_text_s = 0;
    size_image_s = get_size_image_bmp(image_s);
    size_text_s = word_amount;
    
    unsigned char *bytes = (unsigned char*)malloc(size_image_s); if (bytes == NULL) return;
    fread(bytes, 1, size_image_s, image_s);
    degree = bytes[46];

    
    unsigned char *data = (unsigned char*)malloc(size_text_s); if (data == NULL) return;
    fread(data, 1, size_text_s, text_s);

    if(word_amount > size_image_s/8*degree-54){
        printf("Cтолько букв сюда не поместилось бы >:\n");
        return;
    }
    
    unsigned char image_mask = make_image_mask(degree);
    image_mask = ~image_mask;
    
    int index = 54;
    for(size_t current = 0; current < size_text_s; current++){
        unsigned char symbol = 0;
        for(int i = 0; i < 8; i += degree){
            
            int degree_new = 8-i > degree ? degree : 8-i;
            
            image_mask = make_image_mask(degree_new);
            image_mask = ~image_mask;

            
            unsigned char image_byte = bytes[index] & image_mask;
            
            symbol <<= degree_new;
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
    if (argc == 1) argv[1] = "text.txt";
    hide(argv[1]);
    unhide();
    return 0;
}

