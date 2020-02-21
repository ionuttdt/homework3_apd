// Duta Viorel-Ionut, 331CB
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include "tema3.h"

#define  MASTER		0
#define  TAG0       0          
#define  TAG1       1
#define  TAG2       2

int main(int argc, char * argv[]) {

    int numtasks;               // numarul de procese
    int rank;                   // procesul curent
    int len;
    char hostname[MPI_MAX_PROCESSOR_NAME];
    int i, j;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Get_processor_name(hostname, &len);
  
    IMG *image = NULL;          // imaginea fiecarui proces

    /*  Procesul master se ocupa de citirea si distribuirea pozei catre 
    celelalte procese si de reconstruirea si scrierea pozei.*/
    if(rank == MASTER) {
        // mpirun -np N ./tema3 i_in.pnm i_out.pnm filter1 ... filterX
 		if(argc < 3) {                     
		  printf("too few arguments\n");
		  exit(-1);
	    }

	    image = readIMG(argv[1]);           // citesc imaginea din fisier
    	if(image == NULL) {                 // daca citirea a esuat 
    		printf("null image\n");
            return -1;
        }

        int aux_width = image->width;       // imagine alb-negru
        if(image->type == 6)                // imagine color         
            aux_width = aux_width * 3;

        // partajarea imaginii cu celelalte procese 
    	for(i = 1; i < numtasks; i++) {

            // noul start si end al imaginii procesului i
    		int s = (i * image->height / numtasks) - 1;	
    		int e = (i+1) * image->height / numtasks;
    		if(i != numtasks - 1)
    			e++;
    		int aux_height = e - s;                   // noua inaltime a imaginii

            // noua imagine:
    		MPI_Send(&(image->type), 1, MPI_INT, i, TAG0, MPI_COMM_WORLD);	// type
    		MPI_Send(&(image->width), 1, MPI_INT, i, TAG0, MPI_COMM_WORLD);	// width
    		MPI_Send(&aux_height, 1, MPI_INT, i, TAG0, MPI_COMM_WORLD);		//height
    		MPI_Send(&(image->maxval), 1, MPI_INT, i, TAG0, MPI_COMM_WORLD);// maxval

 			for(j = s; j < e; j++) {
                MPI_Send(image->image[j], aux_width, MPI_CHAR, i, 
                    TAG0, MPI_COMM_WORLD);
            }
     	}

        // calcularea sfarsitului imaginii pentru procesul 0
        int e = image->height /numtasks;
        if(e != image->height )
            e++;

        // aplic filtrele pe imaginea procesului MASTER
        for (int i = 3; i < argc; i++) {
            Filter(image, 0, e, argv[i]);
            if( numtasks > 1) {
                // trimit muchia end - 2 catre urmatorul proces
                MPI_Send(image->image[e - 2], aux_width, MPI_CHAR, 1, 
                    TAG1, MPI_COMM_WORLD);
                // actualizez muchia necesara pentru aplicarea filtrelor
                MPI_Recv(image->image[e - 1], aux_width, MPI_CHAR, 1, 
                    TAG1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }

        // construierea imagii finale
        // primesc liniile finale de la fiecare proces cu tagul 2
        for(i = 1; i < numtasks; i++) {  
            AIMG img_aux = malloc(sizeof(IMG));

            MPI_Recv(&(img_aux->type), 1, MPI_INT, i, TAG2, 
                MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&(img_aux->width), 1, MPI_INT, i, TAG2, 
                MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&(img_aux->height), 1, MPI_INT, i, TAG2, 
                MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&(img_aux->maxval), 1, MPI_INT, i, TAG2, 
                MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // calculez de unde incepe si unde se termina imaginea primita
            int s = (i * image->height) / numtasks;
            int e = (i + 1) * image->height / numtasks;

            // daca am putut sa aloc spatiu pentru noua imagine:
            if(allocIMG(img_aux)) {
                for(j = 0; j < img_aux->height; j++) {
                   MPI_Recv(img_aux->image[j], aux_width, MPI_INT, i, TAG2, 
                    MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                }
            }
            else {
                printf("error alloc image\n");
                return -1;
            }

            // imaginea finala
            for(j = s; j < e; j++) {
                free(image->image[j]);
                image->image[j] = img_aux->image[j + 1 - s];
            }
        }

        // scriu in fisier rezultatul
        writeIMG(argv[2], *image);

        // EROARE LINUX:
    	// freeIMG(image);
	}
	else {
        image = malloc(sizeof(IMG));

        MPI_Recv(&(image->type), 1, MPI_INT, 0, TAG0, 
            MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&(image->width), 1, MPI_INT, 0, TAG0, 
            MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&(image->height), 1, MPI_INT, 0, TAG0, 
            MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&(image->maxval), 1, MPI_INT, 0, TAG0, 
            MPI_COMM_WORLD, MPI_STATUS_IGNORE);
         
        // alocare memorie pentru matricea imaginii
        if(!allocIMG(image)) {
            printf("error alloc image\n");
            return -1;
        }

        int h = image->height;

        int aux_w = image->width;       // imagine alb-negru
        if(image->type == 6)            // imagine color
            aux_w *= 3;

        // imaginea corespunzatoare procesului
        for(i = 0; i < h; i++) {
            MPI_Recv(image->image[i], aux_w, MPI_CHAR, 0, TAG0, 
                MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        for(i = 3; i < argc; i++) {
            // aplic filtru pe imaginea procesului
            if(rank == numtasks - 1)
                Filter(image, 0, h , argv[i]);
            else
                Filter2(image, 0, h , argv[i]);

            // actualizare margini superioare si inferioare:
            MPI_Recv(image->image[0], aux_w, MPI_CHAR, rank - 1, 
                TAG1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Send(image->image[1], aux_w, MPI_CHAR, rank - 1, 
                TAG1, MPI_COMM_WORLD);

            // ultimul proces partajeaza doar partea superioara 
            if(rank != numtasks - 1) {
                MPI_Send(image->image[h - 2], aux_w, MPI_CHAR, rank + 1, 
                    TAG1, MPI_COMM_WORLD);
                MPI_Recv(image->image[h - 1], aux_w, MPI_CHAR, rank + 1, 
                    TAG1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }

        // trimitere imagine catre MASTER pentru reconstructie
        MPI_Send(&(image->type), 1, MPI_INT, 0, TAG2, MPI_COMM_WORLD);   // type
        MPI_Send(&(image->width), 1, MPI_INT, 0, TAG2, MPI_COMM_WORLD);  // width
        MPI_Send(&(image->height), 1, MPI_INT, 0, TAG2, MPI_COMM_WORLD); // height
        MPI_Send(&(image->maxval), 1, MPI_INT, 0, TAG2, MPI_COMM_WORLD); // maxval
      
        for(j = 0; j < h; j++) {
            MPI_Send(image->image[j], aux_w, MPI_CHAR, 0, TAG2, MPI_COMM_WORLD);
        }

        // eliberare parte memorie proces
        freeIMG(image);
	}
	
	MPI_Finalize();
	return 0;
}