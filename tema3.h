// Duta Viorel-Ionut, 331CB
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct img{
  int type;				// 5 = PGM, 6 = PNM
  int width;
  int height;
  int maxval;
  unsigned char **image;			// imaginea propriu-zisa
} IMG, *AIMG;

char comment[50];

AIMG readIMG(char *fname) {
	int i, j;

	FILE *in = fopen(fname, "rb");	// poza de intrare
	char aux[50];					// buffer pentru citirea imaginii

	AIMG img = malloc(sizeof(IMG));

	if (in == NULL) {				// daca nu am putut deschide imaginea
		printf("IN\n");
		return NULL;
	}

	// tipul imaginii P5 sau P6
	fscanf(in, "%s\n", aux);
	if(!strcmp(aux, "P5") ) {
		img->type = 5;
	}
	else if(!strcmp(aux, "P6") ) {
		img->type = 6;
	}
	else {
		printf("invalid file type\n");
		return NULL;
	}

	// comentariul: # Created by GIMP version 2.10.14 PNM plug-in
	fscanf(in, "%[^\n]", aux);
	strcpy(comment, aux);

 	fscanf(in, "%s\n", aux);
 	img->width = atoi(aux);

 	fscanf(in, "%s\n", aux);
 	img->height = atoi(aux);

 	fscanf(in, "%s\n", aux);
 	img->maxval = atoi(aux);

 	// alocare spatiu pentru imagine
 	img->image = (unsigned char **) calloc(img->height, sizeof( unsigned char **));
 	if(img->image == NULL) {
   		free(img);
 		return NULL;
 	}

 	// daca imaginea este color:
 	int aux_width = img->width ;
 	if(img->type == 6)
 		aux_width *= 3;

 	for (i = 0; i < aux_width; i++) {
    	img->image[i] = calloc(aux_width, sizeof(unsigned char *));
    	if (img->image[i] == NULL) {
      		for (j = 0; j < i; j++)
        		free(img->image[j]);
      		free(img->image);
      		free(img);
      		return NULL;
    	}
	}

	for (int i = 0; i < img->height; i++)
    	fread(img->image[i], sizeof(unsigned char), aux_width, in);

    // EROARE LINUX cand incerc sa inchid fisierul
	// fclose(in);

	return img;
}

void freeIMG(AIMG img) {
	int i;
	// tipul imaginii
	int aux_width = img->width;
 	if(img->type == 6)
 		aux_width *= 3;

	for (i = 0; i < img->height; i++)
    	free(img->image[i]);
  	free(img->image);
  	free(img);
}

void writeIMG(char *fname, IMG img) {
	int i;

	// fisierul de iesire
	FILE *out = fopen(fname, "w");

	if(out == NULL)
		printf("out file error\n");

	if (img.type == 5) {
    	fprintf(out, "P5\n");
  	}
  	else { 
    	fprintf(out, "P6\n");
    }
	// inca nu stiu daca este nevoie de comentariu in fisierul de iesire
//    fprintf(out, "%s\n", comment );					
    fprintf(out, "%d %d\n", img.width, img.height);
  	fprintf(out, "%d\n", img.maxval);
  	
  	// tipul imaginii
  	int aux_width = img.width;
 	if(img.type == 6)
 		aux_width *= 3;

 	// scriu fiecare linie din imagine
  	for (i = 0; i < img.height; i++) {
    	fwrite(img.image[i], sizeof(unsigned char), aux_width, out);
  	}

	fclose(out);
}

float multiplication(unsigned char m[3][3], float f[9]) {
	float ans = 0.0;
	int i, j;
	// rezultatul aplicarii filtrului
	for(i = 0; i < 3; i++) {
		for(j = 0; j < 3; j++) {
			ans += m[i][j] * f[i*3 + j];
		}
	}

	return ans;
}

void Filter(AIMG img, int s, int e, char *filter) {
	IMG aux = *img;
	int i, j, k = 1;

	float *f = calloc(9, sizeof(float));	// filtrul

	if(strcmp(filter, "smooth") == 0) {
		for(i = 0; i < 9; i++)
			f[i] = 1.0/9;
	}
	else if(strcmp(filter, "blur") == 0) {

        f[0]= 1.0 / 16; f[1] = 2.0 / 16; f[2] = 1.0 / 16;
        f[3]= 2.0 / 16; f[4] = 4.0 / 16; f[5] = 2.0 / 16;
        f[6]= 1.0 / 16; f[7] = 2.0 / 16; f[8] = 1.0 / 16;
	}
	else if(strcmp(filter, "sharpen") == 0) {

		f[0]= 0; f[1] = -2.0 / 3; f[2] = 0;
        f[3]= -2.0 / 3; f[4] = 11.0 / 3; f[5] = -2.0 / 3;
        f[6]= 0; f[7] = -2.0 / 3; f[8] = 0;
	}
	else if(strcmp(filter, "mean") == 0) {
		f[0]= -1; f[1] = -1; f[2] = -1;
        f[3]= -1; f[4] = 9; f[5] = -1;
        f[6]= -1; f[7] = -1; f[8] = -1;
	}
	else if(strcmp(filter, "emboss") == 0){
		f[0]= 0; f[1] = -1; f[2] = 0;
        f[3]= 0; f[4] = 0; f[5] = 0;
        f[6]= 0; f[7] = 1; f[8] = 0;
	}
	else {
		f[0]= 0; f[1] = 0; f[2] = 0;
        f[3]= 0; f[4] = 1; f[5] = 0;
        f[6]= 0; f[7] = 0; f[8] = 0;
	}

	// alocare spatiu de memorie pentru imagine
	aux.image = (unsigned char **) calloc(img->height, sizeof(unsigned char **));
 	if(aux.image == NULL) {
 		return;
 	}

 	int aux_width = img->width;
 	if(aux.type == 6)
 		aux_width *= 3;

 	for (i = 0; i < aux.height; i++) {
    	aux.image[i] = calloc(aux_width, sizeof(unsigned char *));
    	if (aux.image[i] == NULL) {
      		for (j = 0; j < i; j++)
        		free(aux.image[j]);
      		free(aux.image);
    	}
	}

	// tipul imaginii
	if(aux.type == 6)
		k = 3;

	// s si e le folosesc pentru MASTER (are imaginea completa)
	// marginea superioara si inferioara trebuie tartate diferit
	for(i = s; i < e; i++) {
		for(j = 0; j < aux_width; j++ ) {

			if(i == s && j < k) {
				unsigned char m[3][3] = {
					{0, 0, 0},
					{0, img->image[i][j], img->image[i][j+k]},
					{0, img->image[i+1][j], img->image[i+1][j+k]}
				};
				aux.image[i][j] = multiplication(m, f);
			}
			else if(i == s && j > aux_width - k - 1) {
				unsigned char m[3][3] = {
					{0, 0, 0},
					{img->image[i][j-k], img->image[i][j], 0},
					{img->image[i+1][j-k], img->image[i+1][j], 0}
				};

				aux.image[i][j] = multiplication(m, f);
			}


			else if(i == e - 1 && j < k ) {
				unsigned char m[3][3] = {
					{0, img->image[i-1][j], img->image[i-1][j+k]},
					{0, img->image[i][j], img->image[i][j+k]},
					{0, 0, 0}
				};
				aux.image[i][j] = multiplication(m, f);
			}
			else if(i == e - 1 && j > aux_width -k -1 ){
				unsigned char m[3][3] = {
					{img->image[i-1][j-k], img->image[i-1][j], 0},
					{img->image[i][j-k], img->image[i][j], 0},
					{0, 0, 0}
				};
				aux.image[i][j] = multiplication(m, f);
			}
			else if(i == s) {
				unsigned char m[3][3] = {
					{0, 0, 0},
					{img->image[i][j-k], img->image[i][j], img->image[i][j+k]},
					{img->image[i+1][j-k], img->image[i+1][j], img->image[i+1][j+k]}
				};
				aux.image[i][j] = multiplication(m, f);
			}
			else if(i == e-1) {
				unsigned char m[3][3] = {
					{img->image[i-1][j-k], img->image[i-1][j], img->image[i-1][j+k]},
					{img->image[i][j-k], img->image[i][j], img->image[i][j+k]},
					{0, 0, 0}
				};
				aux.image[i][j] = multiplication(m, f);
			}
			else if(j < k) {
				unsigned char m[3][3] = {
					{0, img->image[i-1][j], img->image[i-1][j+k]},
					{0, img->image[i][j], img->image[i][j+k]},
					{0, img->image[i+1][j], img->image[i+1][j+k]}
				};
				aux.image[i][j] = multiplication(m, f);
			}
			else if(j > aux_width - k - 1) {
				unsigned char m[3][3] = {
					{img->image[i-1][j-k], img->image[i-1][j], 0},
					{img->image[i][j-k], img->image[i][j], 0},
					{img->image[i+1][j-k], img->image[i+1][j], 0}
				};
				aux.image[i][j] = multiplication(m, f);
			}
			else {

				unsigned char m[3][3] = {
					{img->image[i-1][j-k], img->image[i-1][j], img->image[i-1][j+k]},
					{img->image[i][j-k], img->image[i][j], img->image[i][j+k]},
					{img->image[i+1][j-k], img->image[i+1][j], img->image[i+1][j+k]}
				};
				aux.image[i][j] = multiplication(m, f);
			}
		}
	}

	// schimbare pointer imagine principala
	for(i = s; i < e; i++) {
		for(j = 0; j < aux_width ; j++ ) {
			img->image[i][j] = aux.image[i][j];
		}
	}

	// eliberare memorie pentru imaginea aux
	for (j = 0; j < img->height; j++)
       	free(aux.image[j]);
    free(f);
}

void Filter2(AIMG img, int s, int e, char *filter) {
	IMG aux = *img;
	int i, j, k = 1;

	float *f = calloc(9, sizeof(float));	// filtrul

	if(strcmp(filter, "smooth") == 0) {
		for(i = 0; i < 9; i++)
			f[i] = 1.0/9;
	}
	else if(strcmp(filter, "blur") == 0) {

        f[0]= 1.0 / 16; f[1] = 2.0 / 16; f[2] = 1.0 / 16;
        f[3]= 2.0 / 16; f[4] = 4.0 / 16; f[5] = 2.0 / 16;
        f[6]= 1.0 / 16; f[7] = 2.0 / 16; f[8] = 1.0 / 16;
	}
	else if(strcmp(filter, "sharpen") == 0) {

		f[0]= 0; f[1] = -2.0 / 3; f[2] = 0;
        f[3]= -2.0 / 3; f[4] = 11.0 / 3; f[5] = -2.0 / 3;
        f[6]= 0; f[7] = -2.0 / 3; f[8] = 0;
	}
	else if(strcmp(filter, "mean") == 0) {
		f[0]= -1; f[1] = -1; f[2] = -1;
        f[3]= -1; f[4] = 9; f[5] = -1;
        f[6]= -1; f[7] = -1; f[8] = -1;
	}
	else if(strcmp(filter, "emboss") == 0){
		f[0]= 0; f[1] = -1; f[2] = 0;
        f[3]= 0; f[4] = 0; f[5] = 0;
        f[6]= 0; f[7] = 1; f[8] = 0;
	}
	else {
		f[0]= 0; f[1] = 0; f[2] = 0;
        f[3]= 0; f[4] = 1; f[5] = 0;
        f[6]= 0; f[7] = 0; f[8] = 0;
	}

	// alocare spatiu de memorie pentru imagine
	aux.image = (unsigned char **) calloc(img->height, sizeof(unsigned char **));
 	if(aux.image == NULL) {
 		return;
 	}

 	int aux_width = img->width;
 	if(aux.type == 6)
 		aux_width *= 3;

 	for (i = 0; i < aux.height; i++) {
    	aux.image[i] = calloc(aux_width, sizeof(unsigned char *));
    	if (aux.image[i] == NULL) {
      		for (j = 0; j < i; j++)
        		free(aux.image[j]);
      		free(aux.image);
    	}
	}

	// tipul imaginii
	if(aux.type == 6)
		k = 3;
	// s si e le folosesc pentru MASTER (are imaginea completa)
	// marginea superioara si inferioara trebuie tartate diferit
	for(i = s + 1; i < e - 1; i++) {
		for(j = 0; j < aux_width  ; j++ ) {
			
			if(j < k) {
				unsigned char m[3][3] = {
					{0, img->image[i-1][j], img->image[i-1][j+k]},
					{0, img->image[i][j], img->image[i][j+k]},
					{0, img->image[i+1][j], img->image[i+1][j+k]}
				};
				aux.image[i][j] = multiplication(m, f);
			}
			else if(j > aux_width - k - 1) {
				unsigned char m[3][3] = {
					{img->image[i-1][j-k], img->image[i-1][j], 0},
					{img->image[i][j-k], img->image[i][j], 0},
					{img->image[i+1][j-k], img->image[i+1][j], 0}
				};
				aux.image[i][j] = multiplication(m, f);
			}
			else {

				unsigned char m[3][3] = {
					{img->image[i-1][j-k], img->image[i-1][j], img->image[i-1][j+k]},
					{img->image[i][j-k], img->image[i][j], img->image[i][j+k]},
					{img->image[i+1][j-k], img->image[i+1][j], img->image[i+1][j+k]}
				};
				aux.image[i][j] = multiplication(m, f);
			}
		}
	}

	// schimbare pointer imagine principala
	for(i = s; i < e; i++) {
		for(j = 0; j < aux_width ; j++ ) {
			img->image[i][j] = aux.image[i][j];
		}
	}

	// eliberare memorie pentru imaginea aux
	for (j = 0; j < img->height; j++)
       	free(aux.image[j]);
    free(f);
}

/* alocare spatiu de memorie pentru reconstruire imagine in MASTER
si pentru receptia iamginii in celelalte procese pentru a putea
reconstrui doar o parte din imagine daca o alocare esueaza
*/
bool allocIMG(AIMG img) {
	int i, j;
	img->image = malloc(img->height * sizeof(unsigned char **));
	
	if(img->image == NULL) {
		return false;
	}

	int aux_width = img->width;
 	if(img->type == 6)
 		aux_width *= 3;
 	for (i = 0; i < img->height; i++) {
    	img->image[i] = (unsigned char *) malloc(aux_width* sizeof(unsigned char *));
    	if (img->image[i] == NULL) {
      		for (j = 0; j < i; j++)
        		free(img->image[j]);
      		free(img->image);
      		return false;
    	}
	}
	return true;
}

