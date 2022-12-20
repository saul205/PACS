__kernel void flip(
	__global unsigned char* img){

	int j = get_global_id(1);
	int cols = get_global_size(0);	
	
	for(int i = 0; i < cols / 2; i++){
		unsigned char aux = img[i][j];
		img[i][j] = img[cols - i][j]
		img[cols - i][j] = aux;
	}
}
