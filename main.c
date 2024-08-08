#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>


int a_mat_rows, a_mat_cols, b_mat_rows, b_mat_cols;
//pointers to files
FILE *a_file;
FILE *b_file;
FILE *c_file ;

//Struct to be passed to threads
struct mat_elements {
	//a is 1st input matrix , b is second & c is the output matrix
    int **a_mat, **_b_mat, **c_mat;
    //i & to store the next true row or column to be calculated
    int i, j;
};


void read_row_col(FILE *file ,int *row,int *col){
	//get character by character
	char c = fgetc(file);
	int temp = 0;
	//loop over file's first line
	while(1){
		//if = found
		if(c == '='){
			//if 2nd time so its "col="
			if(temp){
				//fscanf by filtering an integer & put it in row & col
				fscanf(file, "%d",col);
				return ;
			}
			else{
				fscanf(file, "%d",row);
				temp++;
			}
		}
		c=fgetc(file);
	}

}
//function to copy file contents to 2d array
void read_mat(int **arr,FILE *file,int r,int col){
	for(int i=0 ;i<r ;i++){
		for(int j=0 ;j<col;j++){
			//int num ;
			fscanf(file, "%d",&arr[i][j]);
			//array[i][j] =  num ;
		}
	}

}
//Function to solve in one thread
void Solve_Mat(int **mat_a,int **mat_b,int **mat_c){

	for(int i = 0 ;i<a_mat_rows ; i++){
		for(int j=0 ;j<b_mat_cols;j++){
			mat_c[i][j] = 0;
			for(int r =0 ;r<a_mat_cols;r++){
				mat_c[i][j] += mat_a[i][r] * mat_b[r][j] ;
			}
		}
	}


}
//Function to solve matrix by rows , a thread for each row
void *Solve_Row(void *obj){
	//initialize new struct (object)
	struct mat_elements *Sys_data_row   ;
	Sys_data_row = (struct mat_elements *) obj ;
	//current i (row) in matrix a to be multiplied by columns in matrix b
	int i = Sys_data_row->i ;
	//get input and output arrays
	int **a = Sys_data_row->a_mat , **b = Sys_data_row->_b_mat ,**c = Sys_data_row->c_mat ;

		for(int j=0 ;j<b_mat_cols;j++){
			c[i][j] = 0;
			for(int r =0 ;r<a_mat_cols;r++){
				c[i][j] += a[i][r] * b[r][j] ;
			}
		}


//exit after finishing
pthread_exit(NULL) ;

}

//Function to solve matrix a thread for each element
void *Solve_Element(void *obj){
	//initialize new struct (object)
	struct mat_elements *Sys_data_col   ;
	Sys_data_col = (struct mat_elements *) obj ;

	//current i (row) & j (column) in matrix a & b respectively
	int i = Sys_data_col->i , j =Sys_data_col->j;
	//get input and output arrays
	int **a = Sys_data_col->a_mat , **b = Sys_data_col->_b_mat ,**c = Sys_data_col->c_mat ;

		//calculate element c[i][j]
		c[i][j] = 0;
		for(int r =0 ;r<a_mat_cols;r++){
			c[i][j] += a[i][r] * b[r][j] ;
		}

//exit after finishing
pthread_exit(NULL) ;

}
//creating Threads for solving by rows
void Row_Thread(int **mat_a,int **mat_b,int **mat_c){
	//Array of threads has #row threads
	 pthread_t thread[a_mat_rows];
	 int rc ;
	 //generate threads
	 for(int i=0;i<a_mat_rows;i++){
		 //set struct to be passed to the function
		 struct mat_elements *objj = malloc(sizeof(struct mat_elements));
		 objj->a_mat = mat_a, objj->_b_mat = mat_b, objj->c_mat = mat_c;
		 objj->i = i;
		 //create pthread
		 rc = pthread_create(&thread[i], NULL, Solve_Row, (void *) objj);
		 //Check if thread is not created
		 if(rc){
			 fprintf(stderr, "ERROR!!! The thread can't be created \n");
			 exit(-1);
		 }
	 }

	 //join threads
	 for (int i = 0; i < a_mat_rows; ++i) {

	         pthread_join(thread[i], NULL);

	         if (rc) {   // Check if thread is not joined
	             fprintf(stderr, "ERROR!!!");
	             exit(-1);
	         }
	     }
}
//Creating Threads for solving by each element
void Threads_Elements(int **mat_a,int **mat_b,int **mat_c){
	//number of threads
	int thread_num =  a_mat_rows * b_mat_cols ;
	//Array of threads has #element threads
	 pthread_t threads[thread_num];
	 int not_created ;
	 //generate threads
	 for(int i=0;i<a_mat_rows;i++){
		 for(int j=0; j<b_mat_cols;j++){
			 //set struct to be passed to the function
			 struct mat_elements *object = malloc(sizeof(struct mat_elements));
			 object->a_mat = mat_a, object->_b_mat = mat_b, object->c_mat = mat_c;
			 object->i = i;
			 object->j = j;
			 //create pthread
			 not_created = pthread_create(&threads[i*b_mat_cols + j], NULL, Solve_Element, (void *) object);
			 //Check if thread is not created
			 if(not_created){
				 fprintf(stderr, "ERROR!!! thread can't be created \n");
				 exit(-1);
		 }
	 }
}

	 //join threads
	 for (int i = 0; i < a_mat_rows; ++i) {
		 for(int j =0 ; j< b_mat_cols;j++){

	         pthread_join(threads[i*b_mat_cols+j], NULL);

	         if (not_created) {   // Check if thread is not joined
	             fprintf(stderr, "ERROR!!1");
	             exit(-1);
	         }
	     }
}

}
// Writes output array to a distinct file
void output_res(int **mat_c, char *out_file_path) {
    FILE *out_file = fopen(out_file_path, "w");
    if (out_file == NULL) {     // Check if file doesn't exist
          fprintf(stderr, "Can not open the file in this path 3\n");
          exit(1);
      }
    fprintf(out_file, "row=%d col=%d\n", a_mat_rows,b_mat_cols);

    for (int i = 0; i < a_mat_rows; ++i) {
        for (int j = 0; j < b_mat_cols; ++j) {
            if (j != b_mat_cols - 1) {
                fprintf(out_file, "%d\t", mat_c[i][j]);
            } else {
                fprintf(out_file, "%d\n", mat_c[i][j]);
            }
        }

}
    fclose(out_file);
}
int main(int argc, char *argv[]) {

	// Default files' names , if there are not arguments
	    char *mat_a_path = "a.txt", *mat_b_path = "b.txt", *mat_c_path = "c.out";

	    // Taking arguments if exist
	    if (argc == 2) {
	        mat_a_path = argv[1];
	    } else if (argc == 3) {
	        mat_a_path = argv[1];
	        mat_b_path = argv[2];
	    } else if (argc == 4) {
	        mat_a_path = argv[1];
	        mat_b_path = argv[2];
	        mat_c_path = argv[3];
	    } else if (argc > 4) {      // Print error if too many arguments
	        fprintf(stderr, "Too many arguments \n");
	        exit(1);
	    }

	    a_file = fopen(mat_a_path,"r");
	    b_file = fopen(mat_b_path,"r");

	//read row and column dimensions from a and b files

	  if (a_file== NULL) {
	        fprintf(stderr, "Can't open the files 1\n");
	        exit(1);
	    }
	  if (b_file == NULL) {     // Check if file doesn't exist
	  	        fprintf(stderr, "Can't open the files 2\n");
	  	        exit(1);
	  	    }
	read_row_col(a_file, &a_mat_rows, &a_mat_cols);



	read_row_col(b_file, &b_mat_rows, &b_mat_cols);

	//allocate 2d arrays to store matrices from file
	 int **arr_a = (int **) malloc(a_mat_rows * sizeof(int *));
	 for (int i = 0; i < a_mat_rows; ++i) {
	     arr_a[i] = (int *) malloc(a_mat_cols * sizeof(int));
	   }
	 int **arr_b = (int **) malloc(b_mat_rows * sizeof(int *));
	 	 for (int i = 0; i < b_mat_rows; ++i) {
	 	     arr_b[i] = (int *) malloc(b_mat_cols * sizeof(int));
	 	   }

	 //store input matrices in arrays
	 read_mat(arr_a, a_file, a_mat_rows, a_mat_cols);
	 read_mat(arr_b, b_file, b_mat_rows, b_mat_cols);



	 //allocate output memory
	 int **arr_c = (int **) malloc(a_mat_rows * sizeof(int *));
	 	 	 for (int i = 0; i < a_mat_rows; ++i) {
	 	 	     arr_c[i] = (int *) malloc(b_mat_cols * sizeof(int));
	 	 	   }

	 //if matrices dimensions not suitable return
	 if(!(a_mat_cols == b_mat_rows)){
		 printf("Multiplication can not be done (false dimensions) " );
		 return 0 ;
	 }

	 struct timeval stop, start;

	 //measure time for ( one thread )
	 gettimeofday(&start, NULL); 
	 printf("Method 1 threads (Solving 1 thread) : 1 \n");
	 Solve_Mat(arr_a, arr_b, arr_c) ;

	 gettimeofday(&stop, NULL); 

	 printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
	 printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
	 printf("---------------------------------------\n");

	 //measure time for  ( thread per row )
	 gettimeofday(&start, NULL); 

	 printf("Method 2 threads (Solving through rows ): %d \n", a_mat_rows );
	 Row_Thread(arr_a, arr_b, arr_c);

	 gettimeofday(&stop, NULL); 

	 printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
	 printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
	 printf("---------------------------------------\n");
	 //measure time for ( thread per element )
	 gettimeofday(&start, NULL); 
	 printf("Method 3 threads (Solving through elements) : %d \n", a_mat_rows*b_mat_cols );
	 Threads_Elements(arr_a, arr_b, arr_c) ;

	 gettimeofday(&stop, NULL); 

	 printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
	 printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);

	 //the output file
	 output_res(arr_c, mat_c_path);

	 fclose(a_file) ;
	 fclose(b_file) ;

	 //free allocate
	 free(arr_a);
	 free(arr_b);
	 free(arr_c);

	return 0;
}
